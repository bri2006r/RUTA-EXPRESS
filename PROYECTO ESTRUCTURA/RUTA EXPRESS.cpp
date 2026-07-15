#include <iostream>
#include <cmath>
#include <string>
#include <graphics.h>
#include <conio.h>
#include <fstream>


#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

using namespace std;

// ==================== ESTRUCTURAS Y CONSTANTES ====================
#define MAX_NODOS 7
#define MAX_USUARIOS 50
#define INF 999999999.0 // Usar un número grande para representar infinito (double)

struct NodoGeo {
    int id;
    string nombre;
    double latitud, longitud;
};

struct Arista {
    int destino;
    double distancia;
};

// ==================== VARIABLES GLOBALES ====================
NodoGeo nodos[MAX_NODOS] = {
    {0, "Base Central", 8.3000, -62.7300},
    {1, "Zona Norte", 8.3100, -62.7200},
    {2, "Zona Sur", 8.2900, -62.7400},
    {3, "Calle 123", 8.3050, -62.7250},
    {4, "Carrera 89", 8.2950, -62.7350},
    {5, "Urbanizacion Las Palmas", 8.3080, -62.7220},
    {6, "Centro Comercial Orinokia", 8.3020, -62.7380}
};

Arista grafo[MAX_NODOS][MAX_NODOS]; // Matriz de adyacencia del grafo
int numNodos = MAX_NODOS; // Número total de nodos
int posX[MAX_NODOS]; // Coordenadas X en pantalla para cada nodo
int posY[MAX_NODOS]; // Coordenadas Y en pantalla para cada nodo

// ==================== FUNCIONES DE UTILIDAD ====================
double toRadians(double deg) {
    return deg * M_PI / 180.0;
}

double calcularDistancia(double lat1, double lon1, double lat2, double lon2) {
    double R = 6371000; // Radio de la Tierra en metros
    double phi1 = toRadians(lat1);
    double phi2 = toRadians(lat2);
    double deltaPhi = toRadians(lat2 - lat1);
    double deltaLambda = toRadians(lon2 - lon1);

    double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
               cos(phi1) * cos(phi2) *
               sin(deltaLambda / 2) * sin(deltaLambda / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return R * c; // Distancia en metros
}

// Función para construir el grafo con distancias entre nodos
void construirGrafo() {
    for(int i = 0; i < numNodos; i++) {
        for(int j = 0; j < numNodos; j++) {
            grafo[i][j].distancia = INF;
            grafo[i][j].destino = j;
        }
    }


    grafo[0][1].distancia = calcularDistancia(nodos[0].latitud, nodos[0].longitud, nodos[1].latitud, nodos[1].longitud); // Base Central <-> Zona Norte
    grafo[1][0].distancia = grafo[0][1].distancia;

    grafo[0][2].distancia = calcularDistancia(nodos[0].latitud, nodos[0].longitud, nodos[2].latitud, nodos[2].longitud); // Base Central <-> Zona Sur
    grafo[2][0].distancia = grafo[0][2].distancia;

    grafo[1][3].distancia = calcularDistancia(nodos[1].latitud, nodos[1].longitud, nodos[3].latitud, nodos[3].longitud); // Zona Norte <-> Calle 123
    grafo[3][1].distancia = grafo[1][3].distancia;

    grafo[1][5].distancia = calcularDistancia(nodos[1].latitud, nodos[1].longitud, nodos[5].latitud, nodos[5].longitud); // Zona Norte <-> Urbanizacion Las Palmas
    grafo[5][1].distancia = grafo[1][5].distancia;

    grafo[2][4].distancia = calcularDistancia(nodos[2].latitud, nodos[2].longitud, nodos[4].latitud, nodos[4].longitud); // Zona Sur <-> Carrera 89
    grafo[4][2].distancia = grafo[2][4].distancia;

    grafo[2][6].distancia = calcularDistancia(nodos[2].latitud, nodos[2].longitud, nodos[6].latitud, nodos[6].longitud); // Zona Sur <-> Centro Comercial Orinokia
    grafo[6][2].distancia = grafo[2][6].distancia;

    grafo[3][5].distancia = calcularDistancia(nodos[3].latitud, nodos[3].longitud, nodos[5].latitud, nodos[5].longitud); // Calle 123 <-> Urbanizacion Las Palmas
    grafo[5][3].distancia = grafo[3][5].distancia;

    grafo[4][6].distancia = calcularDistancia(nodos[4].latitud, nodos[4].longitud, nodos[6].latitud, nodos[6].longitud); // Carrera 89 <-> Centro Comercial Orinokia
    grafo[6][4].distancia = grafo[4][6].distancia;

    grafo[0][3].distancia = calcularDistancia(nodos[0].latitud, nodos[0].longitud, nodos[3].latitud, nodos[3].longitud); // Base Central <-> Calle 123
    grafo[3][0].distancia = grafo[0][3].distancia;

    grafo[0][4].distancia = calcularDistancia(nodos[0].latitud, nodos[0].longitud, nodos[4].latitud, nodos[4].longitud); // Base Central <-> Carrera 89
    grafo[4][0].distancia = grafo[0][4].distancia;

    grafo[0][5].distancia = calcularDistancia(nodos[0].latitud, nodos[0].longitud, nodos[5].latitud, nodos[5].longitud); // Base Central <-> Urbanizacion Las Palmas
    grafo[5][0].distancia = grafo[0][5].distancia;

}

// ==================== CLASE PEDIDO ====================
class Pedido {
private:
    int idP;
    long idCliente; 
    int idRepartidor;
    string estado;
    string contenido;
    string instruccionesEspeciales;
    string metodoPago;
    double latitudDestino, longitudDestino;
    int nodoDestino; // ID del nodo más cercano al destino

public:
    Pedido(int idP, long idC, int idR, string e, string c, string iE, string mP, double latD, double lonD) :
        idP(idP), idCliente(idC), idRepartidor(idR), estado(e), contenido(c), instruccionesEspeciales(iE), metodoPago(mP), latitudDestino(latD), longitudDestino(lonD) {
        
        // Calcular el nodo más cercano al destino del pedido
        double minDist = INF;
        int nodoCercano = -1;
        for (int i = 0; i < numNodos; ++i) {
            double dist = calcularDistancia(latitudDestino, longitudDestino, nodos[i].latitud, nodos[i].longitud);
            if (dist < minDist) {
                minDist = dist;
                nodoCercano = i;
            }
        }
        this->nodoDestino = nodoCercano;
    }

    int get_idP() const { return idP; }
    long get_idCliente() const { return idCliente; }
    int get_idRepartidor() const { return idRepartidor; }
    string get_estado() const { return estado; }
    string get_contenido() const { return contenido; }
    string get_instruccionesEspeciales() const { return instruccionesEspeciales; }
    string get_metodoPago() const { return metodoPago; }
    double get_latitudDestino() const { return latitudDestino; }
    double get_longitudDestino() const { return longitudDestino; }
    int obtenerNodoDestino() const { return nodoDestino; }

    void set_idRepartidor(int idR) { idRepartidor = idR; }
    void set_estado(string e) { estado = e; }

    void mostrarInfo() const {
        cout << "  ID Pedido: " << idP << ", Cliente: " << idCliente << ", Estado: " << estado << ", Nodo Destino: " << nodos[nodoDestino].nombre << " (ID " << nodoDestino << ")" << endl;
    }
};

// ==================== CLASES BASE Y DERIVADAS (USUARIOS) ====================
class Usuario {
protected:
    long cedula;
    string nombre;
    string tipo; // "Cliente" o "Repartidor"

public:
    Usuario(long c, string n, string t) : cedula(c), nombre(n), tipo(t) {}
    virtual ~Usuario() {}

    long get_cedula() const { return cedula; }
    string get_nombre() const { return nombre; }
    string get_tipo() const { return tipo; }

    virtual void mostrarInfo() const {
        cout << "Cedula: " << cedula << ", Nombre: " << nombre << ", Tipo: " << tipo << endl;
    }

    bool esCliente() const { return tipo == "Cliente"; }
    bool esRepartidor() const { return tipo == "Repartidor"; }
};

class Cliente : public Usuario {
private:
    string direccion;
    string metodoPagoPreferido;
    double latitud, longitud; // Coordenadas del cliente (inicio de pedidos)
    int nodoCercano; // Nodo más cercano al cliente

public:
    Cliente(long c, string n, long tel, string dir, string mpp, double lat, double lon) :
        Usuario(c, n, "Cliente"), direccion(dir), metodoPagoPreferido(mpp), latitud(lat), longitud(lon) {
        
        // Calcular el nodo más cercano al cliente
        double minDist = INF;
        int nc = -1;
        for (int i = 0; i < numNodos; ++i) {
            double dist = calcularDistancia(lat, lon, nodos[i].latitud, nodos[i].longitud);
            if (dist < minDist) {
                minDist = dist;
                nc = i;
            }
        }
        this->nodoCercano = nc;
    }

    string get_direccion() const { return direccion; }
    string get_metodoPagoPreferido() const { return metodoPagoPreferido; }
    double get_latitud() const { return latitud; }
    double get_longitud() const { return longitud; }
    int obtenerNodoCercano() const { return nodoCercano; }

    void mostrarInfo() const override {
        cout << "Cliente - Cedula: " << cedula << ", Nombre: " << nombre << ", Direccion: " << direccion << ", Nodo Cercano: " << nodos[nodoCercano].nombre << " (ID " << nodoCercano << ")" << endl;
    }
};

class Repartidor : public Usuario {
private:
    string vehiculo;
    string zonaAsignada;
    double latitudActual, longitudActual; // Posición actual del repartidor
    bool disponible;
    Pedido* pedidos[4]; // Arreglo de pedidos que el repartidor lleva (máximo 4)
    int cantidadPedidos;
    int nodoActual; // Nodo más cercano a la posición actual del repartidor

    // Para Dijkstra
    int rutaActual[MAX_NODOS]; // Almacena la ruta 
    int numNodosRuta; // Número de nodos en la ruta

public:
    Repartidor(long c, string n, long tel, string ubi, string v, string zA, double lat, double lon, bool disp) :
        Usuario(c, n, "Repartidor"), vehiculo(v), zonaAsignada(zA), latitudActual(lat), longitudActual(lon), disponible(disp), cantidadPedidos(0), numNodosRuta(0) {
        
        // Calcular el nodo más cercano a la posición inicial del repartidor
        double minDist = INF;
        int nc = -1;
        for (int i = 0; i < numNodos; ++i) {
            double dist = calcularDistancia(lat, lon, nodos[i].latitud, nodos[i].longitud);
            if (dist < minDist) {
                minDist = dist;
                nc = i;
            }
        }
        this->nodoActual = nc;
    }

    string get_vehiculo() const { return vehiculo; }
    string get_zonaAsignada() const { return zonaAsignada; }
    bool get_disponible() const { return disponible; }
    void set_disponible(bool d) { disponible = d; }
    int obtenerNodoCercano() const { return nodoActual; }
    const int* getRutaActual() const { return rutaActual; }
    int getNumNodosRuta() const { return numNodosRuta; }
    
    // Método para obtener el primer pedido (para calcular la ruta a su destino)
    Pedido* getPrimerPedido() const {
        if (cantidadPedidos > 0) {
            return pedidos[0];
        }
        return nullptr;
    }

    bool puedetomarPedido() const {
        // Un repartidor puede tomar un pedido si está disponible y tiene capacidad
        return disponible && (cantidadPedidos < 4); 
    }

    bool asignarPedido(Pedido* p) {
        if (cantidadPedidos < 4) {
            pedidos[cantidadPedidos] = p;
            cantidadPedidos++;
            p->set_estado("Asignado");
            p->set_idRepartidor(this->get_cedula());
            // Una vez que el repartidor toma un pedido, pasa a no estar disponible para nuevas asignaciones
            // (hasta que se "libere" o termine sus entregas)
            set_disponible(false); //ESTO ES CRUCIAL para que el sistema asigne a otros

            cout << "Repartidor " << get_cedula() << " (Origen: " << nodos[obtenerNodoCercano()].nombre << ") - Iniciando Dijkstra." << endl;
            // Despues de asignar un pedido, calcular la ruta a su destino
            calcularRuta(); 
            return true;
        }
        return false;
    }

    void entregarPedido(int idP) {
        for (int i = 0; i < cantidadPedidos; ++i) {
            if (pedidos[i]->get_idP() == idP) {
                pedidos[i]->set_estado("Entregado");
                // Mover los pedidos restantes para llenar el espacio
                for (int j = i; j < cantidadPedidos - 1; ++j) {
                    pedidos[j] = pedidos[j + 1];
                }
                cantidadPedidos--;
                cout << "Repartidor " << cedula << " entrego pedido " << idP << ". Quedan " << cantidadPedidos << " pedidos." << endl;
                
                if (cantidadPedidos == 0) {
                    set_disponible(true); // Repartidor disponible de nuevo si no tiene más pedidos
                }
                // Si el repartidor tiene más pedidos, debería recalcular su ruta al siguiente destino
                
                return;
            }
        }
        cout << "Pedido " << idP << " no encontrado para el repartidor " << cedula << endl;
    }

    void mostrarInfo() const override {
        cout << "Repartidor - Cedula: " << cedula << ", Nombre: " << nombre << ", Vehiculo: " << vehiculo << ", Disponible: " << (disponible ? "Si" : "No") << ", Pedidos: " << cantidadPedidos << ", Nodo Actual: " << nodos[nodoActual].nombre << " (ID " << nodoActual << ")" << endl;
        for(int i = 0; i < cantidadPedidos; ++i) {
            pedidos[i]->mostrarInfo();
        }
    }
    
    void calcularRuta() {
    if (cantidadPedidos == 0) {
        numNodosRuta = 0;
        cout << "No hay pedidos asignados para el Repartidor " << get_cedula() << ". No se calculo ruta." << endl;
        return;
    }

    int inicio = obtenerNodoCercano();
    int destino = pedidos[0]->obtenerNodoDestino();

    double dist[MAX_NODOS];
    int prev[MAX_NODOS];
    bool visitado[MAX_NODOS];

    // Inicialización
    for (int i = 0; i < numNodos; ++i) {
        dist[i] = INF;
        prev[i] = -1;
        visitado[i] = false;
    }

    dist[inicio] = 0;

    // Bucle principal de Dijkstra
    for (int count = 0; count < numNodos; ++count) {
        // Encontrar el nodo no visitado con la distancia mínima
        double minDist = INF;
        int u = -1;

        for (int v = 0; v < numNodos; ++v) {
            if (!visitado[v] && dist[v] < minDist) {
                minDist = dist[v];
                u = v;
            }
        }

        // Si no se encontró un nodo alcanzable o ya llegamos al destino
        if (u == -1 || u == destino) break;

        visitado[u] = true;

        // Actualizar las distancias de los vecinos del nodo u
        for (int v = 0; v < numNodos; ++v) {
            // Si hay una arista de u a v y v no ha sido visitado
            if (grafo[u][v].distancia != INF && !visitado[v]) {
                // Relajación de la arista (u, v)
                if (dist[u] + grafo[u][v].distancia < dist[v]) {
                    dist[v] = dist[u] + grafo[u][v].distancia;
                    prev[v] = u;
                }
            }
        }
    }

    // Reconstruir la ruta
    numNodosRuta = 0;
    if (dist[destino] == INF) {
        cout << "Repartidor " << get_cedula() << ": No se pudo encontrar una ruta al destino " << nodos[destino].nombre << " (ID " << destino << "). Distancia es INF." << endl;
        return;
    }

    int actual = destino;
    int tempRuta[MAX_NODOS];
    int tempRutaSize = 0;

    while (actual != -1) {
        tempRuta[tempRutaSize++] = actual;
        actual = prev[actual];
    }

    // Invertir la ruta
    for (int i = 0; i < tempRutaSize; ++i) {
        rutaActual[i] = tempRuta[tempRutaSize - 1 - i];
    }
    numNodosRuta = tempRutaSize;

    cout << "Repartidor " << get_cedula() << ": Ruta calculada desde " << nodos[inicio].nombre << " a " << nodos[destino].nombre << ": ";
    for (int i = 0; i < numNodosRuta; ++i) {
        cout << nodos[rutaActual[i]].nombre << " ";
    }
    cout << "(Distancia: " << dist[destino] << " metros)" << endl;
}
};

// ==================== CLASE SISTEMA ====================
class Sistema {
private:
    Usuario* usuarios[MAX_USUARIOS];
    int totalUsuarios;

public:
    Sistema() : totalUsuarios(0) {}
    ~Sistema() {
        for (int i = 0; i < totalUsuarios; ++i) {
            delete usuarios[i];
        }
    }

    void registrarUsuario(Usuario* u) {
        if (totalUsuarios < MAX_USUARIOS) {
            usuarios[totalUsuarios++] = u;
            cout << "Usuario " << u->get_nombre() << " registrado." << endl;
        } else {
            cout << "Limite de usuarios alcanzado." << endl;
        }
    }

    Usuario* buscarUsuario(long cedula) {
        for (int i = 0; i < totalUsuarios; ++i) {
            if (usuarios[i]->get_cedula() == cedula) {
                return usuarios[i];
            }
        }
        return nullptr;
    }
    
    // Función de asignación de pedidos
    bool asignarPedido(Pedido* pedido) {
        Repartidor* mejorRepartidor = nullptr; // Inicializar a nullptr
        double mejorDistancia = INF;
        int nodoDestino = pedido->obtenerNodoDestino();
        
        cout << "\nIntentando asignar Pedido " << pedido->get_idP() << " (Destino: " << nodos[nodoDestino].nombre << " (ID " << nodoDestino << "))" << endl;

        for(int i = 0; i < totalUsuarios; i++) {
            if(usuarios[i]->esRepartidor()) {
                Repartidor* rep = (Repartidor*)usuarios[i];
                cout << "  Evaluando Repartidor " << rep->get_cedula() << " (Disponibilidad: " << (rep->puedetomarPedido() ? "Sí" : "No") << ")" << endl;
                
                if(rep->puedetomarPedido()) {
                    int nodoRep = rep->obtenerNodoCercano();
                    // Importante: Aquí se usa la distancia DIRECTA para la asignación inicial.
                    // Si el grafo no tiene conexiones directas a los destinos, siempre será INF.
                    double dist = grafo[nodoRep][nodoDestino].distancia;
                    
                    cout << "    Repartidor " << rep->get_cedula() << " en nodo " << nodos[nodoRep].nombre << " (ID " << nodoRep << "). Distancia DIRECTA a destino " << nodos[nodoDestino].nombre << ": " << (dist == INF ? "INF" : to_string(dist)) << endl;

                    if(dist < mejorDistancia) {
                        mejorDistancia = dist;
                        mejorRepartidor = rep;
                    }
                }
            }
        }
        
        if(mejorRepartidor) {
            cout << "  Encontrado mejor repartidor: " << mejorRepartidor->get_cedula() << " con distancia " << mejorDistancia << endl;
            // Asigna el pedido al mejor repartidor encontrado
            return mejorRepartidor->asignarPedido(pedido);
        } else {
            cout << "  No se encontró ningún repartidor disponible o con ruta directa válida para este pedido." << endl;
        }
        return false;
    }

    void mostrarUsuarios() const {
        cout << "--- Listado de Usuarios ---" << endl;
        for (int i = 0; i < totalUsuarios; ++i) {
            usuarios[i]->mostrarInfo();
        }
    }

    void mostrarRepartidores() const {
        cout << "--- Listado de Repartidores ---" << endl;
        for (int i = 0; i < totalUsuarios; ++i) {
            if (usuarios[i]->esRepartidor()) {
                Repartidor* rep = (Repartidor*)usuarios[i];
                rep->mostrarInfo();
            }
        }
    }
};

// ==================== FUNCIONES DE INTERFAZ GRÁFICA ====================

// Función para obtener la posición en pantalla de un nodo geográfico
void obtenerPosicionesPantalla(int& minX, int& maxX, int& minY, int& maxY, int& screenWidth, int& screenHeight) {
    double minLat = nodos[0].latitud, maxLat = nodos[0].latitud;
    double minLon = nodos[0].longitud, maxLon = nodos[0].longitud;

    for (int i = 0; i < numNodos; ++i) {
        if (nodos[i].latitud < minLat) minLat = nodos[i].latitud;
        if (nodos[i].latitud > maxLat) maxLat = nodos[i].latitud;
        if (nodos[i].longitud < minLon) minLon = nodos[i].longitud;
        if (nodos[i].longitud > maxLon) maxLon = nodos[i].longitud;
    }

    screenWidth = getmaxx();
    screenHeight = getmaxy();

    // Márgenes para el mapa dentro de la ventana
    int margin = 50;
    minX = margin;
    maxX = screenWidth - margin;
    minY = margin;
    maxY = screenHeight - margin;

    double latRange = maxLat - minLat;
    double lonRange = maxLon - minLon;
    
    // Ajustar escala para que el mapa ocupe el mayor espacio posible manteniendo proporciones
    double scaleX = (maxX - minX) / lonRange;
    double scaleY = (maxY - minY) / latRange;
    double scale = min(scaleX, scaleY); // Usar la escala más pequeña para no desbordar

    for (int i = 0; i < numNodos; ++i) {
        // Ajustar el centrado si se usa una escala menor
        posX[i] = minX + (int)((nodos[i].longitud - minLon) * scale + (maxX - minX - lonRange * scale) / 2);
        posY[i] = maxY - (int)((nodos[i].latitud - minLat) * scale + (maxY - minY - latRange * scale) / 2); // Invertir Y para pantalla
    }
}


void dibujarMapa() {
    int minX, maxX, minY, maxY, screenWidth, screenHeight;
    obtenerPosicionesPantalla(minX, maxX, minY, maxY, screenWidth, screenHeight);

    setcolor(DARKGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);

    // Dibujar aristas (rutas del grafo)
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH); // Líneas más gruesas
    setcolor(LIGHTMAGENTA); // Color para las aristas del grafo
    for (int i = 0; i < numNodos; ++i) {
        for (int j = i + 1; j < numNodos; ++j) { // Para no dibujar duplicados
            if (grafo[i][j].distancia != INF) {
                line(posX[i], posY[i], posX[j], posY[j]);
            }
        }
    }

    // Dibujar nodos
    for (int i = 0; i < numNodos; ++i) {
        setcolor(WHITE); 
        setfillstyle(SOLID_FILL, CYAN); 
        fillellipse(posX[i], posY[i], 12, 12); 
        circle(posX[i], posY[i], 12); 

        setcolor(YELLOW); 
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        char idStr[5];
        sprintf(idStr, "%d", nodos[i].id);
        outtextxy(posX[i] - 5, posY[i] - 25, idStr);
        
        
        setcolor(WHITE); 
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 0); 
        
        outtextxy(posX[i] - textwidth(const_cast<char*>(nodos[i].nombre.c_str())) / 2, posY[i] + 15, const_cast<char*>(nodos[i].nombre.c_str())); 
        
    }
}


void animarDelivery(const int ruta1[], int tam1,
                    const int ruta2[], int tam2,
                    const int ruta3[], int tam3,
					const int ruta4[], int tam4) {
    
    int minX, maxX, minY, maxY, screenWidth, screenHeight; 
    obtenerPosicionesPantalla(minX, maxX, minY, maxY, screenWidth, screenHeight); 

    int color1 = LIGHTRED;   // Color para repartidor 1
    int color2 = LIGHTBLUE;  // Color para repartidor 2
    int color3 = LIGHTGREEN; // Color para repartidor 3
	int color4 = LIGHTMAGENTA;
	
    int pasos = 50; 

    // Calcular la duración máxima de la animación: la ruta más larga en términos de "pasos"
    int maxPasosAnimacion = 0;
    // Si la ruta tiene más de 1 nodo (origen y destino), entonces hay al menos 1 segmento para animar
    if (tam1 > 1) maxPasosAnimacion = max(maxPasosAnimacion, (tam1 - 1) * pasos);
    if (tam2 > 1) maxPasosAnimacion = max(maxPasosAnimacion, (tam2 - 1) * pasos);
    if (tam3 > 1) maxPasosAnimacion = max(maxPasosAnimacion, (tam3 - 1) * pasos);
    if (tam4 > 1) maxPasosAnimacion = max(maxPasosAnimacion, (tam4 - 1) * pasos);
    
    // Si ninguna ruta tiene más de 1 nodo (es decir, todas son vacías o solo origen), no animamos nada.
    if (maxPasosAnimacion == 0) {
        cout << "No hay rutas validas (mas de 1 nodo) para animar. Asegurese de que calcularRuta() funciona." << endl;
        delay(2000); // Esperar un momento antes de terminar
        return;
    }

    // Bucle principal de la animación: se ejecuta por la duración de la ruta más larga
    for (int pGlobal = 0; pGlobal <= maxPasosAnimacion; pGlobal++) { 
        cleardevice(); // Limpia toda la pantalla en cada "frame"
        dibujarMapa(); // Vuelve a dibujar el mapa estático (nodos y líneas) en cada "frame"

        
        // DIBUJAR REPARTIDOR 1
        // Solo dibuja si tiene una ruta válida (más de 1 nodo)
        if (tam1 > 1) { 
            int k = pGlobal / pasos; 
            int pLocal = pGlobal % pasos; 

            // Si aún no hemos llegado al final de la ruta para este repartidor
            if (k < tam1 - 1) { 
                
                int x1 = posX[ruta1[k]] + (posX[ruta1[k + 1]] - posX[ruta1[k]]) * pLocal / pasos;
                int y1 = posY[ruta1[k]] + (posY[ruta1[k + 1]] - posY[ruta1[k]]) * pLocal / pasos;
                setcolor(color1);
                setfillstyle(SOLID_FILL, color1);
                fillellipse(x1, y1, 12, 12); 
                setcolor(WHITE); 
                circle(x1, y1, 12);
            } else { 
               
                setcolor(color1);
                setfillstyle(SOLID_FILL, color1);
                fillellipse(posX[ruta1[tam1 - 1]], posY[ruta1[tam1 - 1]], 12, 12);
                setcolor(WHITE);
                circle(posX[ruta1[tam1 - 1]], posY[ruta1[tam1 - 1]], 12);
            }
            
    
        }
        
        
        // DIBUJAR REPARTIDOR 2
        // Exactamente la misma lógica que para el repartidor 1, pero con ruta2 y tam2
        if (tam2 > 1) { 
            int k = pGlobal / pasos;
            int pLocal = pGlobal % pasos;

            if (k < tam2 - 1) {
                int x2 = posX[ruta2[k]] + (posX[ruta2[k + 1]] - posX[ruta2[k]]) * pLocal / pasos;
                int y2 = posY[ruta2[k]] + (posY[ruta2[k + 1]] - posY[ruta2[k]]) * pLocal / pasos;
                setcolor(color2);
                setfillstyle(SOLID_FILL, color2);
                fillellipse(x2, y2, 12, 12);
                setcolor(WHITE);
                circle(x2, y2, 12);
            } else { 
                setcolor(color2);
                setfillstyle(SOLID_FILL, color2);
                fillellipse(posX[ruta2[tam2 - 1]], posY[ruta2[tam2 - 1]], 12, 12);
                setcolor(WHITE);
                circle(posX[ruta2[tam2 - 1]], posY[ruta2[tam2 - 1]], 12);
            }
        }

        
        // DIBUJAR REPARTIDOR 3
        // Exactamente la misma lógica que para el repartidor 1, pero con ruta3 y tam3
        if (tam3 > 1) { 
            int k = pGlobal / pasos;
            int pLocal = pGlobal % pasos;

            if (k < tam3 - 1) {
                int x3 = posX[ruta3[k]] + (posX[ruta3[k + 1]] - posX[ruta3[k]]) * pLocal / pasos;
                int y3 = posY[ruta3[k]] + (posY[ruta3[k + 1]] - posY[ruta3[k]]) * pLocal / pasos;
                setcolor(color3);
                setfillstyle(SOLID_FILL, color3);
                fillellipse(x3, y3, 12, 12);
                setcolor(WHITE);
                circle(x3, y3, 12);
            } else { 
                setcolor(color3);
                setfillstyle(SOLID_FILL, color3);
                fillellipse(posX[ruta3[tam3 - 1]], posY[ruta3[tam3 - 1]], 12, 12);
                setcolor(WHITE);
                circle(posX[ruta3[tam3 - 1]], posY[ruta3[tam3 - 1]], 12);
            }
        }
        
         if (tam4 > 1) { 
            int k = pGlobal / pasos;
            int pLocal = pGlobal % pasos;

            if (k < tam4 - 1) {
                int x4 = posX[ruta3[k]] + (posX[ruta3[k + 1]] - posX[ruta3[k]]) * pLocal / pasos;
                int y4 = posY[ruta3[k]] + (posY[ruta3[k + 1]] - posY[ruta3[k]]) * pLocal / pasos;
                setcolor(color4);
                setfillstyle(SOLID_FILL, color3);
                fillellipse(x4, y4, 12, 12);
                setcolor(WHITE);
                circle(x4, y4, 12);
            } else { 
                setcolor(color4);
                setfillstyle(SOLID_FILL, color4);
                fillellipse(posX[ruta4[tam4 - 1]], posY[ruta4[tam4 - 1]], 12, 12);
                setcolor(WHITE);
                circle(posX[ruta4[tam4 - 1]], posY[ruta4[tam4 - 1]], 12);
            }
        }

        delay(30); // Controla la velocidad de la animación.
    }

    // Al finalizar el bucle de animación, dibuja a todos los repartidores en sus posiciones finales
    
    cleardevice();
    dibujarMapa();
    
    // Repartidor 1
    if (tam1 > 1) {
        setcolor(color1);
        setfillstyle(SOLID_FILL, color1);
        fillellipse(posX[ruta1[tam1 - 1]], posY[ruta1[tam1 - 1]], 12, 12);
        setcolor(WHITE);
        circle(posX[ruta1[tam1 - 1]], posY[ruta1[tam1 - 1]], 12);
    }
    // Repartidor 2
    if (tam2 > 1) {
        setcolor(color2);
        setfillstyle(SOLID_FILL, color2);
        fillellipse(posX[ruta2[tam2 - 1]], posY[ruta2[tam2 - 1]], 12, 12);
        setcolor(WHITE);
        circle(posX[ruta2[tam2 - 1]], posY[ruta2[tam2 - 1]], 12);
    }
    // Repartidor 3
    if (tam3 > 1) {
        setcolor(color3);
        setfillstyle(SOLID_FILL, color3);
        fillellipse(posX[ruta3[tam3 - 1]], posY[ruta3[tam3 - 1]], 12, 12);
        setcolor(WHITE);
        circle(posX[ruta3[tam3 - 1]], posY[ruta3[tam3 - 1]], 12);
    }

    delay(3000); // Esperar 3 segundos para que el usuario vea el resultado final

    // Imprimir los tamaños finales de las rutas 
    cout << "Tamano ruta1: " << tam1 << endl;
    cout << "Tamano ruta2: " << tam2 << endl;
    cout << "Tamao ruta3: " << tam3 << endl;
}

// ==================== MENÚ ====================
int menuSeleccion() {
    int opcion;
    int screenWidth = getmaxx();
    int screenHeight = getmaxy();

    // Cargar la imagen de fondo primero
    readimagefile("Menu.jpg", 0, 0, screenWidth, screenHeight); 

    // Dimensiones del panel del menú
    int panelWidth = 600;
    int panelHeight = 400;
    int panelX = (screenWidth - panelWidth) / 2;
    int panelY = (screenHeight - panelHeight) / 2;

    // Dibujar panel de fondo del menú
    setcolor(DARKGRAY); 
    setfillstyle(SOLID_FILL, BLUE); 
    rectangle(panelX, panelY, panelX + panelWidth, panelY + panelHeight);
    bar(panelX + 1, panelY + 1, panelX + panelWidth - 1, panelY + panelHeight - 1);

    setcolor(LIGHTCYAN); 
    settextstyle(EUROPEAN_FONT, HORIZ_DIR, 4); 
    outtextxy(panelX + (panelWidth - textwidth("RUTA EXPRESS - MENU")) / 2, panelY + 30, "RUTA EXPRESS - MENU");

    setcolor(WHITE); // Color de texto para las opciones
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3); 

    // Posiciones de las opciones
    int startY = panelY + 100;
    int lineHeight = 40;

    outtextxy(panelX + (panelWidth - textwidth("1. Mostrar Datos de Clientes")) / 2, startY, "1. Mostrar Datos de Clientes");
    outtextxy(panelX + (panelWidth - textwidth("2. Mostrar Datos de Repartidores")) / 2, startY + lineHeight, "2. Mostrar Datos de Repartidores ");
    outtextxy(panelX + (panelWidth - textwidth("3. Animacion Ruta")) / 2, startY + 2 * lineHeight, "3. Animacion Ruta");
    outtextxy(panelX + (panelWidth - textwidth("4. Registrar Repartidor")) / 2, startY + 3 * lineHeight, "4. Registrar Repartidor");
	outtextxy(panelX + (panelWidth - textwidth("5. Salir")) / 2, startY + 4 * lineHeight, "5. Salir");
	

    // Mensaje de selección
    setcolor(WHITE); 
    settextstyle(SOLID_FILL, HORIZ_DIR, 1); 
    outtextxy(panelX + (panelWidth - textwidth("Seleccione una opcion:")) / 2, startY + 5 * lineHeight + 20, "Seleccione una opcion:");

    // Esperar entrada
    char c = getch();
    opcion = c - '0'; 
    
    cleardevice(); // Limpiar el menú después de la selección
    return opcion;
}

void mostrarDatosCliente(Cliente* cliente) {
    cleardevice();
    int screenWidth = getmaxx();
    int screenHeight = getmaxy();

    // Dibujar un panel de información para el cliente
    int panelWidth = 800;
    int panelHeight = 500;
    int panelX = (screenWidth - panelWidth) / 2;
    int panelY = (screenHeight - panelHeight) / 2;

    setcolor(LIGHTGRAY); // Borde
    setfillstyle(SOLID_FILL, BLUE); // Relleno oscuro
    rectangle(panelX, panelY, panelX + panelWidth, panelY + panelHeight);
    bar(panelX + 1, panelY + 1, panelX + panelWidth - 1, panelY + panelHeight - 1);

    setcolor(WHITE);
    settextstyle(COMPLEX_FONT, HORIZ_DIR, 4); 
    outtextxy(panelX + (panelWidth - textwidth("Datos del Cliente:")) / 2, panelY + 30, "Datos del Cliente:");
    
    settextstyle(COMPLEX_FONT, HORIZ_DIR, 3); 
    setcolor(WHITE); // Color diferente para los datos

    char buffer[256];
    int startInfoY = panelY + 100;
    int infoLineHeight = 30;

    sprintf(buffer, "Cedula: %ld", cliente->get_cedula());
    outtextxy(panelX + 50, startInfoY, buffer);

    sprintf(buffer, "Nombre: %s", cliente->get_nombre().c_str());
    outtextxy(panelX + 50, startInfoY + infoLineHeight, buffer);

    sprintf(buffer, "Direccion: %s", cliente->get_direccion().c_str());
    outtextxy(panelX + 50, startInfoY + 2 * infoLineHeight, buffer);

    sprintf(buffer, "Metodo de Pago Preferido: %s", cliente->get_metodoPagoPreferido().c_str());
    outtextxy(panelX + 50, startInfoY + 3 * infoLineHeight, buffer);
    
    setcolor(WHITE); 
    settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
    outtextxy(panelX + (panelWidth - textwidth("Presione cualquier tecla para volver al menu...")) / 2, panelY + panelHeight - 50, "Presione cualquier tecla para volver al menu...");
    getch();
    cleardevice();
}

void mostrarDatosRepartidor(Repartidor* repartidor) {
    cleardevice();
    int screenWidth = getmaxx();
    int screenHeight = getmaxy();

    // Dibujar un panel de información para el repartidor
    int panelWidth = 800;
    int panelHeight = 500;
    int panelX = (screenWidth - panelWidth) / 2;
    int panelY = (screenHeight - panelHeight) / 2;

    setcolor(LIGHTGRAY); // Borde
    setfillstyle(SOLID_FILL, MAGENTA); // Relleno oscuro
    rectangle(panelX, panelY, panelX + panelWidth, panelY + panelHeight);
    bar(panelX + 1, panelY + 1, panelX + panelWidth - 1, panelY + panelHeight - 1);

    setcolor(WHITE);
    settextstyle(COMPLEX_FONT, HORIZ_DIR, 4); 
    outtextxy(panelX + (panelWidth - textwidth("Datos del Repartidor:")) / 2, panelY + 30, "Datos del Repartidor:");
    
    settextstyle(COMPLEX_FONT, HORIZ_DIR, 3); 
    setcolor(WHITE); 

    char buffer[256];
    int startInfoY = panelY + 100;
    int infoLineHeight = 30;

    sprintf(buffer, "Cedula: %ld", repartidor->get_cedula());
    outtextxy(panelX + 50, startInfoY, buffer);

    sprintf(buffer, "Nombre: %s", repartidor->get_nombre().c_str());
    outtextxy(panelX + 50, startInfoY + infoLineHeight, buffer);

    sprintf(buffer, "Zona Asignada: %s", repartidor->get_zonaAsignada().c_str());
    outtextxy(panelX + 50, startInfoY + 2 * infoLineHeight, buffer);

    sprintf(buffer, "Vehiculo: %s", repartidor->get_vehiculo().c_str());
    outtextxy(panelX + 50, startInfoY + 3 * infoLineHeight, buffer);
    
    setcolor(WHITE); 
    settextstyle(COMPLEX_FONT, HORIZ_DIR, 2);
    outtextxy(panelX + (panelWidth - textwidth("Presione cualquier tecla para volver al menu...")) / 2, panelY + panelHeight - 50, "Presione cualquier tecla para volver al menu...");
    getch();
    cleardevice();
}

string leerTexto(int x, int y, int longitudMaxima = 30) {
    char texto[100] = "";
    int pos = 0;
    char c;

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    while (true) {
        c = getch();
        if (c == 13) break; // Enter
        else if (c == 8 && pos > 0) { // Backspace
            pos--;
            texto[pos] = '\0';
            setcolor(LIGHTGRAY);
            bar(x, y, x + 400, y + 25); // Borrar texto anterior
            setcolor(BLACK);
            outtextxy(x, y, texto);
        }
        else if (pos < longitudMaxima && c >= 32 && c <= 126) { // Caracter visible
            texto[pos++] = c;
            texto[pos] = '\0';
            setcolor(LIGHTGRAY);
            bar(x, y, x + 400, y + 25); // Borrar texto anterior
            setcolor(BLACK);
            outtextxy(x, y, texto);
        }
    }
    return string(texto);
}

void registrarRepartidorDesdeGrafica(Sistema& sistema) {
    cleardevice();
    setbkcolor(LIGHTBLUE);
    cleardevice();

    int x = 100, y = 100;
    string cedulaStr, nombre, vehiculo, zona;
    long cedula;
    double lat = 8.3000, lon = -62.7300; // Base Central

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(x, y, "Registro de nuevo Repartidor:");

    y += 40;
     settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(x, y, "Cedula:");
    cedulaStr = leerTexto(x + 200, y); // Lee como string
    cedula = stol(cedulaStr); // Convertir a long

    y += 40;
     settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(x, y, "Nombre:");
    nombre = leerTexto(x + 200, y);

    y += 40;
     settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(x, y, "Vehiculo:");
    vehiculo = leerTexto(x + 200, y);

    y += 40;
     settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(x, y, "Zona Asignada:");
    zona = leerTexto(x + 200, y);

    Repartidor* nuevo = new Repartidor(cedula, nombre, 0, "Base Central", vehiculo, zona, lat, lon, true);
    sistema.registrarUsuario(nuevo);

    ofstream archivo("repartidores.txt", ios::app);
    if (archivo.is_open()) {
        archivo << cedula << "," << nombre << "," << vehiculo << "," << zona << "," << lat << "," << lon << "\n";
        archivo.close();
    }

    y += 60;
    setcolor(GREEN);
    outtextxy(x, y, "Repartidor registrado correctamente.");
    getch();
}

int main() {
    // Inicializa el sistema, usuarios, pedidos, etc.
    construirGrafo();
    cout << "--- Grafo Construido ---" << endl;
    for (int i = 0; i < numNodos; ++i) {
        for (int j = 0; j < numNodos; ++j) {
            if (grafo[i][j].distancia == INF) {
                cout << "INF\t";
            } else {
                cout << grafo[i][j].distancia << "\t";
            }
        }
        cout << endl;
    }
    cout << "------------------------" << endl;

    Sistema sistema;

    // Registrar clientes con coordenadas geográficas
    Cliente* cliente1 = new Cliente(1001, "Maria Gonzalez", 3101111111, 
                                   "Calle 123 #45-67", "Tarjeta", 
                                   8.3050, -62.7250);
    Cliente* cliente2 = new Cliente(1002, "Carlos Perez", 3102222222, 
                                   "Carrera 89 #12-34", "Efectivo", 
                                   8.2950, -62.7350);
    Cliente* cliente3 = new Cliente(1003, "Ximena Ponch", 3103333333,
                                    "Urbanizacion Las Palmas", "PagoMovil",
	                                8.3080, -62.7220);
	Cliente* cliente4 = new Cliente(1004, "Juan Perez", 3103333334,
                                    "Centro Comercial Orinokia", "Efectivo",
	                                8.3020, -62.7380);
	                                
    
    // Registrar repartidores en la base central
    Repartidor* repartidor1 = new Repartidor(2001, "Juan Rodriguez", 3103333333, 
                                           "Base Central", "Moto", "Zona Norte", 
                                           8.3000, -62.7300, true);
    Repartidor* repartidor2 = new Repartidor(2002, "Ana Lopez", 3104444444, 
                                           "Base Central", "Camioneta", "Zona Sur", 
                                           8.3000, -62.7300, true);
    Repartidor* repartidor3 = new Repartidor(2003, "Carolina Mendez", 3105555555,
											"Base Central", "Moto", "Zona Sur",
											8.3000, -62.7300, true);
	Repartidor* repartidor4 = new Repartidor(2004, "Maria Garcia", 2222333445,
											"Base Central", "Camioneta", "Zona Norte",
											8.3000, -62.7300,true );
    
    sistema.registrarUsuario(cliente1);
    sistema.registrarUsuario(cliente2);
    sistema.registrarUsuario(cliente3);
    sistema.registrarUsuario(cliente4); 
    sistema.registrarUsuario(repartidor1);
    sistema.registrarUsuario(repartidor2);
    sistema.registrarUsuario(repartidor3);
    sistema.registrarUsuario(repartidor4);
    
	//CREACION DE PEDIDOS
    Pedido* pedido1 = new Pedido(0, cliente1->get_cedula(), 0, "Recibido", 
                                "Laptop x1, Mouse x2", "Empaque original", 
                                "Tarjeta", 8.3050, -62.7250);
    Pedido* pedido2 = new Pedido(1, cliente2->get_cedula(), 0, "Recibido", 
                                "Paquete 3kg, Documentos", "Firmar recibido", 
                                "Efectivo", 8.2950, -62.7350);
    Pedido* pedido3 = new Pedido(2, cliente3->get_cedula(), 0, "Recibido", 
                                "herramienta", "martillo",
								"PagoMovil", 8.3080, -62.7220);
	 Pedido* pedido4 = new Pedido(3, cliente3->get_cedula(), 0, "Recibido", 
                                "herramienta", "martillo",
								"PagoMovil", 8.3020, -62.7380 );
								
    
    //ASIGNACION DE PEDIDOS
    if(sistema.asignarPedido(pedido1)) {
        cout << "Pedido 1 asignado correctamente" << endl;
    } else {
        cout << "Error: No se pudo asignar el Pedido 1." << endl;
    }

    if(sistema.asignarPedido(pedido2)) {
        cout << "Pedido 2 asignado correctamente" << endl;
    } else {
        cout << "Error: No se pudo asignar el Pedido 2." << endl;
    }

    if(sistema.asignarPedido(pedido3)) {
        cout << "Pedido 3 asignado correctamente" << endl;
    } else {
        cout << "Error: No se pudo asignar el Pedido 3." << endl;
    }
    
     if(sistema.asignarPedido(pedido4)) {
        cout << "Pedido 4 asignado correctamente" << endl;
    } else {
        cout << "Error: No se pudo asignar el Pedido 4." << endl;
    }
    
    // Mostrar estado del sistema
    cout << "\n=== ESTADO DE REPARTIDORES DESPUES DE ASIGNACION ===" << endl;
    sistema.mostrarRepartidores();
    
    // Depuración de rutas calculadas después de la asignación
    cout << "\n--- Rutas Calculadas ---" << endl;
    cout << "Nodos en ruta de Repartidor 1: " << repartidor1->getNumNodosRuta() << endl;
    cout << "Nodos en ruta de Repartidor 2: " << repartidor2->getNumNodosRuta() << endl;
    cout << "Nodos en ruta de Repartidor 3: " << repartidor3->getNumNodosRuta() << endl;
    cout << "Nodos en ruta de Repartidor 4: " << repartidor4->getNumNodosRuta() << endl;

    initwindow (900, 600, "RUTA EXPRESS");
     setbkcolor(LIGHTBLUE); 
    cleardevice();        
    
    int opcion = 0;
    while(opcion != 5) {
        cleardevice(); // Limpiar la pantalla antes de mostrar el menú
        opcion = menuSeleccion();

        switch(opcion){
            case 1 :
                mostrarDatosCliente(cliente1);
                mostrarDatosCliente(cliente2);
                mostrarDatosCliente(cliente3);
				mostrarDatosCliente(cliente4);
                break;
                
            	case 2 :
            	mostrarDatosRepartidor(repartidor1);
            	mostrarDatosRepartidor(repartidor2);
            	mostrarDatosRepartidor(repartidor3);
            	mostrarDatosRepartidor(repartidor4);
            	break;
                
                case 3 :
                cleardevice();
                dibujarMapa();
                animarDelivery(repartidor1->getRutaActual(), repartidor1->getNumNodosRuta(),
                               repartidor2->getRutaActual(), repartidor2->getNumNodosRuta(),
                               repartidor3->getRutaActual(), repartidor3->getNumNodosRuta(),
                               repartidor4->getRutaActual(), repartidor4->getNumNodosRuta());
                break;
    
            	case 4:
    			registrarRepartidorDesdeGrafica(sistema);
    			break;
    		
				case 5 :
                cout << "Saliendo del programa..." << endl;
                break;	
    		
            	default :
                cout << "Opcion invalida. Intente de nuevo." << endl;
                delay(1000);
                break;
        }
    }
    
    getch();
    closegraph(); 
    
   
    delete pedido1;
    delete pedido2;
    delete pedido3;
   
    
    return 0;
}