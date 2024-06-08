#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <unordered_map>
#include <fstream>

using namespace std;
using namespace cv;

// Definición de la estructura para un Nodo en el árbol de Huffman
struct Nodo {
    char letra;
    int frecuencia;
    Nodo* der;
    Nodo* izq;
    Nodo* sig;
};

// Función para crear un nuevo nodo
Nodo* Nodos(char letra, int frecuencia, Nodo* izq, Nodo* der) {
    Nodo* nodo = new Nodo();
    nodo->letra = letra;
    nodo->frecuencia = frecuencia;
    nodo->izq = izq;
    nodo->der = der;
    nodo->sig = NULL;
    return nodo;
}

// Raíz global del árbol de Huffman
Nodo* raiz = NULL;

// Función recursiva para generar códigos Huffman
void codificado(Nodo* raiz, string str, unordered_map<char, string>& huffman) {
    if (raiz == NULL) {
        return;
    }
    if (!raiz->izq && !raiz->der) {
        huffman[raiz->letra] = str;
    }
    codificado(raiz->izq, str + "0", huffman);
    codificado(raiz->der, str + "1", huffman);
}

void liberarMemoria(Nodo* nodo) {
    if (nodo != nullptr) {
        liberarMemoria(nodo->izq);
        liberarMemoria(nodo->der);
        delete nodo;
    }
}

// Función recursiva para decodificar un mensaje Huffman
// Función recursiva para decodificar un mensaje Huffman
void decodificar(Nodo* raiz, int& ind, string st) {
    if (raiz == NULL) {
        return;
    }
    if (!raiz->izq && !raiz->der) {
        cout << raiz->letra;
        return;
    }
    ind++;
    if (st[ind] == '0') {
        decodificar(raiz->izq, ind, st);
    }
    else {
        decodificar(raiz->der, ind, st);
    }
}


// Función para insertar un nuevo nodo en la lista ordenadamente
void insertarNodoOrdenado(Nodo*& lista, Nodo* nuevoNodo) {
    if (lista == NULL || lista->frecuencia >= nuevoNodo->frecuencia) {
        nuevoNodo->sig = lista;
        lista = nuevoNodo;
    }
    else {
        Nodo* actual = lista;
        while (actual->sig != NULL && actual->sig->frecuencia < nuevoNodo->frecuencia) {
            actual = actual->sig;
        }
        nuevoNodo->sig = actual->sig;
        actual->sig = nuevoNodo;
    }
}

// Función para construir el árbol de Huffman a partir de las frecuencias
string creararbol(string frase) {
    Nodo* lista = NULL;
    unordered_map<char, int> frecuencia;
    for (char simbolo : frase) {
        frecuencia[simbolo]++;
    }
    // Inicializar la lista de nodos hoja
    for (const auto& par : frecuencia) {
        insertarNodoOrdenado(lista, Nodos(par.first, par.second, NULL, NULL));
    }

    // Construir el árbol de Huffman hasta que sea 1 porque la raíz es uno xd
    while (lista != NULL && lista->sig != NULL) {
        // Tomar los dos primeros nodos de la lista
        Nodo* izquierda = lista;
        Nodo* derecha = lista->sig;

        // Eliminar los dos primeros nodos de la lista
        lista = lista->sig->sig;

        // Crear un nuevo nodo interno
        Nodo* nuevoNodo = Nodos('\0', izquierda->frecuencia + derecha->frecuencia, izquierda, derecha);

        // Insertar el nuevo nodo interno en la lista ordenadamente
        insertarNodoOrdenado(lista, nuevoNodo);
    }
    raiz = lista;

    unordered_map<char, string> huffman;
    codificado(raiz, "", huffman);

    string st = "";
    for (char letra : frase) {
        st += huffman[letra];
    }
    return st;
}

// Función para ocultar un mensaje en los bits menos significativos de una imagen
void ocultarMensajeLSB(Mat& imagen, const string& mensaje) {
    int indiceMensaje = 0;

    if (mensaje.length() > imagen.rows * imagen.cols * 3) {
        cerr << "Error: El mensaje es demasiado largo para ocultarlo en la imagen." << endl;
        return;
    }

    for (int i = 0; i < imagen.rows; ++i) {
        for (int j = 0; j < imagen.cols; ++j) {
            Vec3b& pixel = imagen.at<Vec3b>(i, j);

            // Procesar cada canal (RGB)
            for (int k = 0; k < 3; ++k) {
                if (indiceMensaje < mensaje.length()) {
                    // Limpiar el bit menos significativo
                    pixel[k] &= 0xFE;

                    // Establecer el bit menos significativo con el bit del mensaje
                    pixel[k] |= ((mensaje[indiceMensaje] - '0') & 1);

                    // Mover al siguiente bit del mensaje
                    ++indiceMensaje;
                }
                else {
                    // Marcar el final del mensaje
                    return;
                }
            }
        }
    }
}

// Función para extraer un mensaje oculto de los bits menos significativos de una imagen
string extraerMensajeLSB(const Mat& imagen, int longitudMensaje) {
    string mensajeExtraido = "";
    int bitCount = 0;

    for (int i = 0; i < imagen.rows; ++i) {
        for (int j = 0; j < imagen.cols; ++j) {
            const Vec3b& pixel = imagen.at<Vec3b>(i, j);

            // Procesar cada canal (RGB)
            for (int k = 0; k < 3; ++k) {
                // Extraer el bit menos significativo y agregarlo al mensaje
                mensajeExtraido += ((pixel[k] & 1) + '0');
                ++bitCount;

                // Verificar si hemos recuperado suficientes bits
                if (bitCount == longitudMensaje) {
                    return mensajeExtraido;
                }
            }
        }
    }

    return mensajeExtraido;
}

string ingresar(string filepath) {
    ifstream entr(filepath);

    string frase, linea;
    while (getline(entr, linea)) {
        frase += linea;
    }
    entr.close();
    return frase;
}

// Función para calcular el porcentaje de diferencia entre dos imágenes
double calcularDiferencia(const Mat& imagenOriginal, const Mat& imagenModificada) {
    if (imagenOriginal.size() != imagenModificada.size()) {
        cerr << "Las dimensiones de las imágenes no coinciden." << endl;
        return -1.0;
    }

    Mat grisOriginal, grisModificada;
    cvtColor(imagenOriginal, grisOriginal, COLOR_BGR2GRAY);
    cvtColor(imagenModificada, grisModificada, COLOR_BGR2GRAY);

    Mat diferencia;
    absdiff(grisOriginal, grisModificada, diferencia);

    double sumaDiferencia = sum(diferencia)[0];
    double areaTotal = imagenOriginal.rows * imagenOriginal.cols;
    double porcentajeDiferencia = (sumaDiferencia / areaTotal) * 100.0;

    return porcentajeDiferencia;
}

double calcularNormaL1(const Mat& img1, const Mat& img2) {
    double norma = 0.0;

    for (int i = 0; i < img1.rows; ++i) {
        for (int j = 0; j < img1.cols; ++j) {
            for (int k = 0; k < 3; ++k) {
                norma += abs(static_cast<double>(img1.at<Vec3b>(i, j)[k]) - static_cast<double>(img2.at<Vec3b>(i, j)[k]));
            }
        }
    }

    return norma / (img1.rows * img1.cols * 3);
}

double calcularNormaL2(const Mat& img1, const Mat& img2) {
    double norma = 0.0;

    for (int i = 0; i < img1.rows; ++i) {
        for (int j = 0; j < img1.cols; ++j) {
            for (int k = 0; k < 3; ++k) {
                double diferencia = static_cast<double>(img1.at<Vec3b>(i, j)[k]) - static_cast<double>(img2.at<Vec3b>(i, j)[k]);
                norma += diferencia * diferencia;
            }
        }
    }

    return sqrt(norma / (img1.rows * img1.cols * 3));
}

int main() {
    // Cargar la imagen
    Mat imagen = imread("C:/Users/User/Desktop/TIF/entrada.jpg");
    string filepath = "C:\\entrada\\shrek1.txt";

    if (imagen.empty()) {
        cerr << "Error: No se pudo cargar la imagen." << endl;
        return -1;
    }

    // Solicitar al usuario el mensaje que desea ocultar
    cout << "Ingrese el mensaje que desea ocultar: " << endl;
    string frase = ingresar(filepath);;
    //getline(cin, frase);

    // Crear el árbol de Huffman y obtener la representación en bits del mensaje original
    string mensaje = creararbol(frase);

    // Obtener la longitud del mensaje original para la extracción
    int longitudMensaje = mensaje.length();

    // Ocultar el mensaje en la imagen utilizando los bits menos significativos
    ocultarMensajeLSB(imagen, mensaje);

    // Guardar la imagen con el mensaje oculto
    imwrite("C:/Users/User/Desktop/TIF/imagen_salida.png", imagen);
    cout << "Mensaje oculto exitosamente en la imagen." << endl;
    /*
    // Extraer el mensaje de la imagen modificada
    string mensajeExtraido = extraerMensajeLSB(imagen, longitudMensaje);

    if (mensajeExtraido.empty()) {
        cerr << "Error: No se pudo extraer el mensaje de la imagen." << endl;
    }
    else {
        // Imprimir el mensaje binario extraído
        cout << "Mensaje binario extraido: " << mensajeExtraido << endl;

        // Decodificar y mostrar el mensaje recuperado
        cout << "Mensaje recuperado: ";
        int ind = -1; // Inicializar ind a 0 antes de pasar por referencia
        while (ind < (int)mensajeExtraido.size() - 2) {
            decodificar(raiz, ind, mensajeExtraido);
        }
        cout << endl;
    }
    */
    // Mostrar la imagen con el mensaje oculto
    imshow("Imagen con mensaje oculto", imagen);
    waitKey(0);

    // Calcular y mostrar el porcentaje de diferencia entre la imagen original y la modificada
    const string rutaImagenOriginal = "C:/Users/User/Desktop/TIF/entrada.jpg";
    const string rutaImagenModificada = "C:/Users/User/Desktop/TIF/imagen_salida.png";

    Mat imagenOriginal = imread(rutaImagenOriginal);
    Mat imagenModificada = imread(rutaImagenModificada);

    if (imagenOriginal.empty() || imagenModificada.empty()) {
        cerr << "Error al leer las imágenes." << endl;
        return -1;
    }

    double porcentajeDiferencia = calcularDiferencia(imagenOriginal, imagenModificada);
    double diferenciaCubierta_L1 = calcularNormaL1(imagenOriginal, imagenModificada);
    double diferenciaCubierta_L2 = calcularNormaL2(imagenOriginal, imagenModificada);

    cout << "Norma L1 entre la imagen original de la cubierta y la imagen esteganográfica: " << diferenciaCubierta_L1 << endl;
    cout << "Norma L2 entre la imagen original de la cubierta y la imagen esteganográfica: " << diferenciaCubierta_L2 << endl;

    cout << "Porcentaje de diferencia: " << porcentajeDiferencia << "%" << endl;
    liberarMemoria(raiz);

    return 0;
}