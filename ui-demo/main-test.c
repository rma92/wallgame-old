#include <windows.h>
#include <time.h> 

LPCSTR AppName = "TestApp";
#define IDT_TIMER1 501
//60 fps = 1000/60 = 16
#define SETTING_INTERVAL_FLASH 16
//#define SETTING_INTERVAL_FLASH 200
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define MAX_PEOPLE 1024
#define PERSON_WIDTH 16
#define PERSON_HEIGHT 32

#define G_BORDER_Y 250

RECT rect1 = { 10, 20, 80, 100 };
int num_people = 0;
BOOL mouse_down = FALSE;
int person_under_mouse = -1;
int mouse_x_offset = -1;//for dragging
int mouse_y_offset = -1;//for dragging

int mouse_cur_x = 0;
int mouse_cur_y = 0;
char label[255];

#define MALE 0
#define FEMALE 1

struct person
{
  unsigned char x;//top-left corner
  unsigned char y;
  unsigned char race:3;
  unsigned char us_citizen:1;
  unsigned char gender:1; //0 male 1 female
  unsigned char draw_frame:4;
  unsigned char pad2;
};

struct person * peeps;

//check if all of the people are above the wall
// ( y < (G_BORDER_Y >> 1)
void check_win()
{
  int i;
  int max_y = G_BORDER_Y >> 1;
  BOOL win = TRUE;
  for( i = 0; i < num_people; ++i )
  {
    if( peeps[ i ].y > max_y )
    {
      win = FALSE;
      break;
    }
  }
  if( win )
  {
    MessageBoxA(0, "you win", 0, 0);
  }
}

void add_person( int xc, int yc)
{
  peeps[ num_people ].x = xc;
  peeps[ num_people ].y = yc;
  peeps[ num_people ].race = 0;
  peeps[ num_people ].us_citizen = FALSE;
  peeps[ num_people ].gender = FALSE;
  peeps[ num_people ].draw_frame = 0;
  ++num_people;
  printf("person[%d] x=%d y=%d\n", num_people, xc, yc );
}

void paint( HWND hWnd, HDC hdc )
{
  int i;
  RECT clientRect, temp;
  HBRUSH hBackBrush = CreateSolidBrush( RGB(255, 0, 0) );
  HBRUSH hRandBrush = CreateSolidBrush( RGB(rand() & 0xFF, rand() & 0xFF, rand & 0xFF) );
  GetClientRect( hWnd, &clientRect );
  FillRect( hdc, &clientRect, hBackBrush );

  //draw the text.
  temp.top = 0;
  temp.left = 0;
  temp.right = WINDOW_WIDTH;
  temp.bottom = 18;
  DrawText( hdc, "Move the boxes above the line.", -1, &temp, 0);

  //FillRect( hdc, &rect1, hRandBrush );
  //draw the people.
  sprintf( label, "" );
  MoveToEx( hdc, 0, G_BORDER_Y, NULL );
  LineTo( hdc, clientRect.right, G_BORDER_Y);
  for( i = 0; i < num_people; ++i )
  {
    temp.top = ((int)peeps[ i ].y) << 1;
    temp.bottom = temp.top + PERSON_HEIGHT;
    temp.left = ((int)peeps[ i ].x) << 1;
    temp.right = temp.left + PERSON_WIDTH;
    //printf( "top: %d right: %d\n", temp.top, temp.right );
    Rectangle( hdc, temp.left-1, temp.top-1, temp.right + 1, temp.bottom + 1);
    FillRect( hdc, &temp, hRandBrush );
    if( mouse_cur_x > temp.left & mouse_cur_x < temp.right
      & mouse_cur_y > temp.top & mouse_cur_y < temp.bottom
    )
    {
      sprintf( label, "[%d] loc: %d %d", i, temp.top, temp.left );
      /*
      if( !mouse_down )
      {
        person_under_mouse = i;
        mouse_x_offset = mouse_cur_x - temp.left;
        mouse_y_offset = mouse_cur_y - temp.top;
      }
      */
    }
  }
  //printf("\n\n\n");
  Ellipse( hdc, mouse_cur_x - 4, mouse_cur_y - 4, mouse_cur_x + 4, mouse_cur_y + 4);
  temp.top = 0;
  temp.left = 0;
  temp.right = WINDOW_WIDTH;
  temp.bottom = 18;
  DrawText( hdc, label, -1, &temp, DT_CENTER);
  DeleteObject( hBackBrush );
  DeleteObject( hRandBrush );
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_CREATE: 
      {
        srand( time(NULL) );
        SetTimer( hWnd, IDT_TIMER1, SETTING_INTERVAL_FLASH, (TIMERPROC) NULL );
        peeps = (struct person *) calloc(MAX_PEOPLE * 1);

        add_person( rand() & 0xF0 - 16, (rand() & 0x1F) +127);
        add_person( rand() & 0xF0 - 16, (rand() & 0x1F) +127);
        add_person( rand() & 0xF0 - 16, (rand() & 0x1F) +127);

        ShowCursor( FALSE );
      }
      break;
    case WM_TIMER:
      switch( wParam )
      {
        case IDT_TIMER1:
        {
          InvalidateRect( hWnd, 0, TRUE );
        }
      }
      break;
    case WM_MOUSEMOVE:
      {
        //mouse_cur_x = GET_X_PARAM(lParam);
        //mouse_cur_y = GET_Y_PARAM(lParam);
        int temp_x; 
        int temp_y;
        mouse_cur_x = LOWORD(lParam);
        mouse_cur_y = HIWORD(lParam);
        InvalidateRect( hWnd, 0, TRUE );
        if( mouse_down && person_under_mouse > -1 )
        {
          //peeps[ person_under_mouse ].x = ((mouse_cur_x - mouse_x_offset) >> 1);
          temp_x = ((mouse_cur_x - mouse_x_offset) >> 1);
          if( temp_x > 0 && temp_x < 255 )
          {
            peeps[ person_under_mouse ].x = temp_x;
          }
          temp_y = ((mouse_cur_y - mouse_y_offset) >> 1);
          if( temp_y > 0 && temp_y < 255 )
          {
            peeps[ person_under_mouse ].y = temp_y;
          }
        }
      }
      break;
    case WM_LBUTTONDOWN:
      {
        int i;
        RECT temp;
        mouse_down = TRUE;
        printf("Down");
        for( i = 0; i < num_people; ++i )
        {
          temp.top = ((int)peeps[ i ].y) << 1;
          temp.bottom = temp.top + PERSON_HEIGHT;
          temp.left = ((int)peeps[ i ].x) << 1;
          temp.right = temp.left + PERSON_WIDTH;
          if( mouse_cur_x > temp.left & mouse_cur_x < temp.right
            & mouse_cur_y > temp.top & mouse_cur_y < temp.bottom
            )
          { 
            person_under_mouse = i;
            mouse_x_offset = mouse_cur_x - temp.left;
            mouse_y_offset = mouse_cur_y - temp.top;
            break;
          }
        }
      }
      break;
    case WM_LBUTTONUP:
      mouse_down = FALSE;
      person_under_mouse = -1;
      printf("UP");
      check_win();
      break;
    case WM_KEYDOWN:
    {
      if( wParam == VK_DOWN ) 
      {
        ++rect1.top;
        ++rect1.bottom;
      }
      if( wParam == VK_UP ) 
      {
        --rect1.top;
        --rect1.bottom;
      }
      if( wParam == VK_LEFT ) 
      {
        --rect1.left;
        --rect1.right;
      }
      if( wParam == VK_RIGHT ) 
      {
        ++rect1.left;
        ++rect1.right;
      }
      if( wParam == VK_HOME )
      {
        add_person( rand() & 0xF0 - 16, (rand() & 0x1F) +127);
      }
      //printf("%d", wParam);
      break;
    }
    case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      hdc = BeginPaint( hWnd, &ps );
      paint( hWnd, hdc );
      EndPaint( hWnd, &ps );
      break;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
  MSG msg1;
  WNDCLASS wc1;
  HWND hWnd1;
  ZeroMemory(&wc1, sizeof wc1);
  wc1.hInstance = hInst;
  wc1.lpszClassName = AppName;
  wc1.lpfnWndProc = (WNDPROC)WndProc;
  wc1.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
  wc1.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
  wc1.hIcon = LoadIcon(NULL, IDI_INFORMATION);
  wc1.hCursor = LoadCursor(NULL, IDC_ARROW);
  if(RegisterClass(&wc1) == FALSE) return 0;
  hWnd1 = CreateWindow(AppName, AppName, 
    WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
    10, 10, WINDOW_WIDTH + 32, WINDOW_HEIGHT + 32, 0, 0, hInst, 0);
  if(hWnd1 == NULL) return 0;
  while(GetMessage(&msg1,NULL,0,0) > 0){
    TranslateMessage(&msg1);
    DispatchMessage(&msg1);
  }
  return msg1.wParam;
}

