// Simulasi Kain (Cloth simulation)
// created by aji mustofa @pepega90

// Referensi
// https://gamedevelopment.tutsplus.com/tutorials/simulate-tearable-cloth-and-ragdolls-with-simple-verlet-integration--gamedev-519

#include <raylib.h>
#include <math.h>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 640;
// jarak antar constraint
const int SPACING = 15;
// seberapa besar kekuatan mouse untuk narik
const int MOUSEDRAG = 20;
// seberapa besar kekuatan mouse untuk menyobek
const int MOUSECUT = 5;
// gravitasi
const int GRAVITASI = 100;
// tinggi dari cloth
const int CLOTHHEIGHT = 30;
// lebar dari cloth
const int CLOTHWIDTH = 50;
// posisi koordinat y awal mulai cloth
const int start_y = 20;
// jarak antara constraint untuk disobek
const int JARAKSOBEK = 60;

// disini saya melakukan forward declaration untuk struct Point
struct Point;
struct Constraint
{

    // kita menyimpan 2 buah point, yang nantinya akan di sambung
    // dengan constraint
    Point &p1;
    Point &p2;
    float spacing = SPACING;

    Constraint(Point &p1, Point &p2) : p1(p1), p2(p2) {}
};

struct Point
{

    // pos, menyimpan posisi saat ini (current position)
    Vector2 pos;
    // lastPos, menyimpan posisi terakhir dari pos
    Vector2 lastPos;
    // acc, menyimpan nilai akselerasi (bisa juga di ubah menjadi velocity)
    Vector2 acc;

    // isPinned, yang akan membuat clothnya menjadi mengantung, jika bernilai true
    // maka point tersebut akan bertindak sebagai anchor dari cloth
    bool isPinned = false;
    // ini berisikan posisi dari pin point
    float pinX, pinY;
    // berisikan list dari point, yang akan saling terhubung
    std::vector<Constraint *> links;

    Point(float x, float y)
    {
        pos.x = x;
        pos.y = y;

        lastPos.x = x;
        lastPos.y = y;

        acc.x = 0;
        acc.y = 0;
    }

    // menghapus spesifik posisi constraint
    void RemoveConstraint(int index)
    {

        links.erase(links.begin() + index);
    }

    void ResolveConstraint()
    {
        if (isPinned)
        {
            pos.x = pinX;
            pos.y = pinY;
            return;
        }

        // kita update constraint
        for (int i = 0; i < links.size(); i++)
        {
            Vector2 diff = links[i]->p1.pos - links[i]->p2.pos;
            float dist = diff.Magnitude();
            float d = (links[i]->spacing - dist) / dist;

            if (dist > JARAKSOBEK)
            {
                this->RemoveConstraint(i);
            }

            float px = diff.x * d * 0.5;
            float py = diff.y * d * 0.5;

            links[i]->p1.pos.x += px;
            links[i]->p1.pos.y += py;
            links[i]->p2.pos.x -= px;
            links[i]->p2.pos.y -= py;
        }

        // membuat point yang sudah sobek tetap berada di layar
        if (pos.x > GetScreenWidth())
        {
            pos.x = GetScreenWidth();
            lastPos.x = pos.x;
        }
        else if (pos.x < 0)
        {
            pos.x = 0;
            lastPos.x = pos.x;
        }

        if (pos.y > GetScreenHeight())
        {
            pos.y = GetScreenHeight();
            lastPos.y = pos.y;
        }
        else if (pos.y < 0)
        {
            pos.y = 0;
            lastPos.y = pos.y;
        }
    }

    void Update(float &dt, Vector2 &mousePos, Vector2 &prevMouse, bool &leftDown, bool &rightDown)
    {
        Vector2 pm = pos - mousePos;
        float distancePointToMouse = sqrtf((pm.x * pm.x) + (pm.y * pm.y));

        // ketika mouse klik kiri maka kita goyangkan clothnya
        if (leftDown)
        {
            if (distancePointToMouse < MOUSEDRAG)
            {
                lastPos.x = pos.x - (mousePos.x - prevMouse.x) * 1.8;
                lastPos.y = pos.y - (mousePos.y - prevMouse.y) * 1.8;
            }
        }

        // ketika mouse klik kanan, maka kita sobek clothnya
        if (rightDown)
        {
            if (distancePointToMouse < MOUSECUT)
            {
                links.clear();
            }
        }

        // kasih gravitasi
        acc.y += GRAVITASI * dt;

        float velX = pos.x - lastPos.x;
        float velY = pos.y - lastPos.y;

        velX *= 0.99;
        velY *= 0.99;

        float timeStep = dt * dt;

        // kalkulasi posisi baru dengan verlet integration
        // untuk refrensi https://en.wikipedia.org/wiki/Verlet_integration
        float nx = pos.x + velX + 0.5 * acc.x * timeStep;
        float ny = pos.y + velY + 0.5 * acc.y * timeStep;

        lastPos = pos;

        pos.x = nx;
        pos.y = ny;

        acc.x = 0;
        acc.y = 0;
    }

    // gambar garis dari setiap point
    void Draw()
    {
        for (int i = 0; i < links.size(); i++)
        {
            DrawLineEx(links[i]->p1.pos, links[i]->p2.pos, 2.0, GRAY);
        }
    }

    // membuat point menjadi anchor
    void Pin(float x, float y)
    {
        isPinned = true;
        pinX = x;
        pinY = y;
    }

    // kita attach point lain
    void Attach(Point *p)
    {
        Constraint *c = new Constraint(*this, *p);
        links.push_back(c);
    }
};

int main()
{
    InitWindow(WIDTH, HEIGHT, "Cloth Simulation");
    SetTargetFPS(60);

    // menyimpan posisi
    Vector2 mousePos;
    // menyimpan posisi terakhir mouse
    Vector2 prevMouse;
    // untuk mengecek apakah mouse klik kiri ditekan
    bool leftDown;
    // untuk mengecek apakah mouse klik kanan ditekan
    bool rightDown;

    std::vector<Point *> points;
    // kalkulasi posisi awal koordinat x untuk clothnya
    float start_x = GetScreenWidth() / 2 - CLOTHWIDTH * SPACING / 2;

    // kita ingin loop y berada di luar, jadi ini lopp baris demi baris alih-alih kolom demi kolom
    for (int y = 0; y <= CLOTHHEIGHT; y++)
    {
        for (int x = 0; x <= CLOTHWIDTH; x++)
        {
            Point *p = new Point(start_x + x * SPACING, start_y + y * SPACING);

            // pasang point ke kiri
            if (x != 0)
                p->Attach(points[points.size() - 1]);

            // pasang point ke kanan
            if (y != 0)
                p->Attach(points[x + (y - 1) * (CLOTHWIDTH + 1)]);
            // pin point, yang akan membuatnya menjadi anchor dan bikin clothnya jadi mengantung
            if (y == 0)
                p->Pin(p->pos.x, p->pos.y);

            points.push_back(p);
        }
    }

    // delta time, disini saya gak gunain bawaan raylib, karena
    // gak tau kenapa kaya terlalu lambat aja
    float dt = 0.16;

    while (!WindowShouldClose())
    {
        ClearBackground(BLACK);
        BeginDrawing();

        prevMouse = mousePos;
        mousePos = GetMousePosition();

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            leftDown = true;
        }

        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            rightDown = true;
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON) && leftDown)
        {
            leftDown = false;
        }

        if (IsMouseButtonUp(MOUSE_RIGHT_BUTTON) && rightDown)
        {
            rightDown = false;
        }

        for (auto &p : points)
        {
            p->ResolveConstraint();
            p->Update(dt, mousePos, prevMouse, leftDown, rightDown);
            p->Draw();
        }

        EndDrawing();
    }
    CloseWindow();

    return 0;
}