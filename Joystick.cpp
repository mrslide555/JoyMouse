//***********************************************************************//
//																		 //
//		TFG joystick to mouse - reproduce los movimientos del mouse      //
//          recogiedo los movimientos de un joystick convirtiendolos     //
//          en eventos del mouse.                                   	 //
//                                                                       //
//		$Autor: Daniel Val       danielval69@gmail.com		        	 //																 //
//		$Programa: Joystick				        						 //
//																		 //
//***********************************************************************//

//winmm.lib contiene la API del joystick
#pragma comment(lib,"winmm.lib")


#include <windows.h>										// para jugar con eventos de windows
#include <stdio.h>											// estandar input y output (básico)
#include <iostream>
#include <string>
#include <format>
using namespace std;


#define CURSOR_SPEED 20										// velocidad del mouse

//resolución de pantalla: 1366 x 768
//tamaño ventana: 600x180
// Posicion de pantalla pegada a la derecha
//RECT windowRect = { 100, 100, 600, 500 };
RECT windowRect = {1366-600, 768-180-35, 600, 180};						// se definen los valores de la ventana (posición y tamaño).

///////////////////////////////// INIT JOYSTICK \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	Se inicializa el programa verificando si hay un joystick disponible 
/////                                          o si los drivers son válidos.
/////
///////////////////////////////// INIT JOYSTICK \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

BOOL InitJoystick(HWND hWnd)
{
	DWORD dwResult = 0;

	// La siguiente función devuelve el número de dispositivos disponibles
	if (!joyGetNumDevs())
    {
		// Se muestra un error advirtiendo que no se encuentran los drivers
        MessageBox(NULL, "No se encuentran drivers disponibles o el joystick está desconectado.", "Error", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }


	/**
	* La siguiente función inicializa el joystick. Existen dos opciones, JOYSTICKID1 o JOYSTICKID2.
	* Por lo general, la mayoria de las personas solamente utilizan uno, por ello selecionamos JOYSTICKID1.
	* 
	* hWnd: es el handle que recibirá los mensajes del joystick, los mensajes se comprobaran en WndProc().
	* JOYSTICKID1: es el joystick con el que vamos a jugar.
	* 0: el tiempo en milisegundos en el que se mandarán mensajes (no queremos delay)
	* False: queremos que mande mensajes constantemente, True mandaría mensajes si la posición del jostick cambia.
	**/
	dwResult = joySetCapture(hWnd, JOYSTICKID1, 0, FALSE);

	
	/**
	* Evaluamos el resultado obtenido en dwResult
	* Pueden ocurrir 3 casos, que el joystick este desconectado, que no tenga driver 
	* o que de un error por otro motivo.
	* 
	* En caso contrario, supondremos que todo ha ido bien 
	**/
	switch (dwResult) 
	{
		case JOYERR_UNPLUGGED:								// joystick desconectado
			MessageBox(hWnd, "No se encuentra el joystick, aseguresé de tener el joystick conectado.", NULL, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;									// se devuelve falso
		case MMSYSERR_NODRIVER:								// no hay driver para el joystick
			MessageBox(hWnd, "No se encuentra un driver válido. Por favor, reinstale o actualice los drivers de su joystick.", NULL, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;									// se devuelve falso
		case JOYERR_NOCANDO:								// error desconocido
			MessageBox(hWnd, "No se pueden recoger los datos de su joystick, pruebe a reiniciar la aplicación.", NULL, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;									// se devuelve falso
	}

	return TRUE;											// todo va bien, continuamos!!
}


///////////////////////////////// Movimiento del cursor \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	El movimiento del cursor depende de los movimientos del joystick
/////
///////////////////////////////// Movimiento del cursor \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void MueveCursor(LPARAM lParam)
{
	POINT cursorPos;										// punto que almacena la coordenada del cursor
	int x = 0, y = 0;										// variables donde guardaremos el movimiento del joystick
	GetCursorPos(&cursorPos);								// Recoge la posición actual del cursor


	// Este paso es algo complejo, el rango de valor del joystick va desde 0 hasta 65535, situando el origen o punto (0,0) 
	// en la esquina superior izquierda. La operación que se realiza a continuación es un bit shift que lo que hace es mover los bits
	// de posición, añadiendo ceros para reducir el número. Este mecanismo permite dividir el número a la mitad y en este caso, 
	// la operación se realiza 12 veces.

	// las funciones LOWORD() y HIWORD() recogen la información de los bits de menor peso y de mayor peso respectivamente,
	// en una estructura de 32 bits como WPARAM y LPARAM. En el parámetro lParam se recoge el valor que tiene el joystick,
	// almacena el valor de x en la parte baja y el valor de y en la parte alta.

	std::cout << "lParam: " << lParam << "\n";
	std::cout << "LOWORD(lParam): " << LOWORD(lParam) << "\n";
	std::cout << "HIWORD(lParam): " << HIWORD(lParam) << "\n";


	x = LOWORD(lParam) >> 12;
    y = HIWORD(lParam) >> 12;

	std::cout << "x: " << x << "\n";
	std::cout << "y: " << y << "\n\n";


	// Ahora tenemos escalada la posicion x e y entre 0 y 15, en el siguiente paso habrá que mover la posición del cursor.
	// Los valores 0 y 15 son ambos extremos, se podría utilizar otro valor 7 u 8 para no ener que mover tanto la palanca.
	// Si se quiere mayor precisión, se podria intentar escalar el valor con mayor rango (hacr bit shift 11 veces en lugar de 12
	// daría como resultado valores entre 0 y 31.

	// Si se mueve el joystick a la izquierda, se decrementa el valor de x del cursor con la aceleración
	// sino, si se mueve al a derecha, se incrementa el valor de x del cursor con la aceleración
	if(x == 0)
		cursorPos.x -= CURSOR_SPEED;
	else if(x == 15)
		cursorPos.x += CURSOR_SPEED;

	// Si se mueve el joystick hacia arriba, se decrementa el valor de y del cursor con la aceleración
	// sino, si se mueve hacia abajo, se incrementa el valor de y del cursor con la aceleración
	if(y == 0)
		cursorPos.y -= CURSOR_SPEED;
	else if(y == 15)
		cursorPos.y += CURSOR_SPEED;

	// En caso de no tocar el joystick, el cursor no se moverá.

	// Se coloca el cursor en la nueva posición
	SetCursorPos(cursorPos.x, cursorPos.y);
	
}

// Función que captura los mensajes que se generen en la ventana.
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);		
															// Este es el equivalente al main() en windows, WinMain().	
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)			
{														
    HWND        hwnd;										// Identificador de ventana.
    MSG         msg;										// Variable de mensajes que se lanzan
    WNDCLASSEX  wndclass;									// Información de la ventana (nombre, icono, cursor, color, menu ...)

    wndclass.cbSize        = sizeof (wndclass);				// setea el tamaño de wndclass. 
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;		// Estilo Horizontal-Redraw y Verticle-Redraw 
    wndclass.lpfnWndProc   = WndProc;						// Se le asigna la función CALLBACK. (Que manejará los mensajes)
    wndclass.cbClsExtra    = 0;								// Se ponen 0 bytes extra
    wndclass.cbWndExtra    = 0;								// bytes de ventana
    wndclass.hInstance     = hInstance;						// Se asigna hInstance de la ventana.  
    wndclass.hIcon         = LoadIcon (NULL, IDI_WINLOGO);	// carga de logo de windows
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);	// carga el cursor de flecha.
															// Setea el color de fondo de la ventana
    wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
    wndclass.lpszMenuName  = NULL;							// No queremos menú en la ventana
    wndclass.lpszClassName = "Joystick";					// Nombre que identifica la venana.
    wndclass.hIconSm       = LoadIcon (NULL, IDI_WINLOGO);	// Icono de la parte superior izquierda.
			
	RegisterClassEx (&wndclass);							// Se necesita registrar wndclass con el sistema operativo
															// Ventana de prueba.
    hwnd = CreateWindow ("Joystick",						// Nombre de la ventana 
						 "Joystick",			  			// Título de la ventana    
						 WS_OVERLAPPEDWINDOW,				// estilo de ventana		 
						 windowRect.left,					// posición inicial x
						 windowRect.top,					// posición inicial y
						 windowRect.right,					// tamaño de x	 
						 windowRect.bottom,				    // tamaño de y
						 NULL,								// padre de la ventana
						 NULL,								// menú de la ventana
						 hInstance,						    // hInstance
						 NULL);								// enlace a wndproc

    ShowWindow (hwnd, iCmdShow);							// Mostrar visible la ventana
    UpdateWindow (hwnd);									// refrescar el pintado de vntana
															
															// Bucle del main
	while (GetMessage (&msg, NULL, 0, 0))	{				// para cada mensaje que se recibe, se traduce y se maneja.
		TranslateMessage (&msg);							// Traduce el mensaje recibido del usuario
		DispatchMessage (&msg);								// maneja el mensaje recibido del usuario
    }
    
	UnregisterClass("Joystick",hInstance);					// Se des-registra wndclass para liberar la memoria

	return msg.wParam;										// Salimos de la apicación
}

															// (wndProc).  Este es el manejador de mensajes
LRESULT CALLBACK WndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{	
	PAINTSTRUCT paintStruct;								// Create a paint struct
	HDC hdc;												// Create our HDC (Handle to a Device Context = The graphics card)
	string str = "uu";
	RECT        rc;
	char titulo[80]="";

    switch (iMsg)											// Comparamos qué mensaje se lanza.
    {
		case WM_CREATE:										// Este mensaje se lanza cuando se crea la ventana

			if(!InitJoystick(hWnd))							// llamada la función de inicialización del joystick
				PostQuitMessage(0);							// Si no se pudiera llamar o hubiera algún problema, se cierra la aplicación

			break;											// break del switch.

		case MM_JOY1BUTTONDOWN:								// Este mensaje se envia al pulsar un botón

			// Cuando se pulsa un botón, salta este mensaje.
			// para verificar qué botón se ha pulsado, se compara con el parámetro wParam
			// El parametro wParam contiene el botón pulsado en binario, por tanto la comparación
			// en lugar de ser con AND, se hace con &.

			if (wParam & JOY_BUTTON1)						// Si se pulsa el botón 1
				MessageBox(NULL, "Button 1 was pressed!", "Message", MB_OK);

			if (wParam & JOY_BUTTON2)						// Si se pulsa el botón 2
				MessageBox(NULL, "Button 2 was pressed!", "Message", MB_OK);

			if (wParam & JOY_BUTTON3)						// Si se pulsa el botón 3
				MessageBox(NULL, "Button 3 was pressed!", "Message", MB_OK);

			if (wParam & JOY_BUTTON4)						// Si se pulsa el botón 4
				MessageBox(NULL, "Button 4 was pressed!", "Message", MB_OK);

			// Otro mensaje importante es MM_JOY1BUTTONUP, que salta cuando se deja de pulsar un botón
			
			break;

		case MM_JOY1MOVE:									// Este mensaje salta cuando se mueve el cursor

			// El mensaje MM_JOY1MOVE se envía constantemente, sólamente moveremos el cursor si se mueve el joystick considerablemente.
			// el parámetro lParam nos dice cuanto se está inclinando el joystick.
			
			int xo, yo, x, y;
			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			x = LOWORD(lParam) >> 11;
			y = HIWORD(lParam) >> 11;

			str ="Joystick: ("+ to_string(xo)+","+ to_string(yo)+") (0 y 31): ("+ to_string(x)+","+ to_string(y)+")";
			str.copy(titulo, str.length(), 0);
			SetWindowText(hWnd, titulo);

			MueveCursor(lParam);					// Funcion que mueve el cursor			
			break;

		case WM_PAINT:										
			hdc = BeginPaint(hWnd, &paintStruct);				
			RECT        rc;
			GetClientRect(hWnd, &rc);
			SetTextColor(hdc, 0);
			SetBkMode(hdc, TRANSPARENT);
			DrawText(hdc, "Joystick", -1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			EndPaint(hWnd, &paintStruct);					
			DeleteDC(hdc);									
		    break;											

		case WM_DESTROY:									// Este mensaje se lanza cuando se cierra la aplicación
			PostQuitMessage(0);								// Se envía WM_QUIT para cerrar la aplicacion
			break;											// Break del switch
	}													

	return DefWindowProc (hWnd, iMsg, wParam, lParam);		// Devuelve información para el bucle de mensajes
}															// Fin de WndProc
