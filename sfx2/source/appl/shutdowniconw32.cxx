/*************************************************************************
 *
 *  $RCSfile: shutdowniconw32.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ssa $ $Date: 2001-05-11 16:41:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef WNT

//
// the systray icon is only available on windows
//

#include <shutdownicon.hxx>
#include <app.hrc>
#include <systools/win32/user9x.h>

using namespace ::rtl;
using namespace ::com::sun::star::uno;

#define LISTENER_WINDOWCLASS    "SO Listener Class"
#define LISTENER_WINDOWNAME     "SO Listener Window"

#define ID_QUICKSTART               1
#define IDM_EXIT                    2
#define IDM_OPEN                    3
#define IDM_WRITER                  4
#define IDM_CALC                    5
#define IDM_IMPRESS                 6
#define IDM_DRAW                    7
#define IDM_TEMPLATE                8

#define SFX_TASKBAR_NOTIFICATION    WM_USER+1

static HWND aListenerWindow = NULL;

static void addMenuItem( HMENU hMenu, UINT id, LPSTR text, int& pos )
{
    MENUITEMINFOA mi;

    mi.cbSize = sizeof(MENUITEMINFO);
    if( id == -1 )
    {
        mi.fMask=MIIM_TYPE;
        mi.fType=MFT_SEPARATOR;
    }
    else
    {
        mi.fMask=MIIM_TYPE|MIIM_STATE|MIIM_ID;
        mi.fType=MFT_STRING;
        mi.dwTypeData = text;
        mi.cch = strlen(mi.dwTypeData);
        mi.fState=MFS_ENABLED;
        mi.wID = id;
    }
    mi.hSubMenu=NULL;
    mi.hbmpChecked=NULL;
    mi.hbmpUnchecked=NULL;
    mi.dwItemData=NULL;
    InsertMenuItemA( hMenu, pos++, TRUE, &mi );
}

static HMENU createSystrayMenu( )
{
    HMENU hMenu = CreatePopupMenu();
    int pos=0;

    OUString strWriter, strCalc, strImpress, strDraw, strFileopen, strExit;

    if( ShutdownIcon::getInstance() )
    {
        /*
        strWriter   = ShutdownIcon::getInstance()->GetResString(  );
        strCalc     = ShutdownIcon::getInstance()->GetResString(  );
        strImpress  = ShutdownIcon::getInstance()->GetResString(  );
        strDraw     = ShutdownIcon::getInstance()->GetResString(  );
        */
        strFileopen = ShutdownIcon::getInstance()->GetResString( STR_QUICKSTART_FILEOPEN );
        strExit     = ShutdownIcon::getInstance()->GetResString( STR_QUICKSTART_EXIT );
    }

    addMenuItem( hMenu, IDM_WRITER, "Text Document", pos );
    addMenuItem( hMenu, IDM_CALC,   "Spreadsheet", pos );
    addMenuItem( hMenu, IDM_IMPRESS,"Presentation", pos );
    addMenuItem( hMenu, IDM_DRAW,   "Draw", pos );
    addMenuItem( hMenu, -1,   NULL, pos );
    addMenuItem( hMenu, IDM_OPEN, "File open...", pos );
    addMenuItem( hMenu, IDM_EXIT, "Exit", pos );

    return hMenu;
}

LRESULT CALLBACK listenerWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HMENU popupMenu = NULL;

    switch (uMsg)
    {
        case WM_NCCREATE:
            return TRUE;
        case WM_CREATE:
            {
                if( !popupMenu )
                    popupMenu = createSystrayMenu( );

                OUString strTip;
                if( ShutdownIcon::getInstance() )
                    strTip = ShutdownIcon::getInstance()->GetResString( STR_QUICKSTART_TIP );

                // add taskbar icon
                NOTIFYICONDATA nid;
                nid.hIcon = (HICON)LoadImageA( GetModuleHandle( NULL ), MAKEINTRESOURCE( 1 ),
                    IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ),
                    LR_DEFAULTCOLOR | LR_SHARED );
                strcpy(nid.szTip, "StarOffice 6.0 QuickStart");
                nid.cbSize              = sizeof(NOTIFYICONDATA);
                nid.hWnd                = hWnd;
                nid.uID                 = ID_QUICKSTART;
                nid.uCallbackMessage    = SFX_TASKBAR_NOTIFICATION;
                nid.uFlags              = NIF_MESSAGE|NIF_TIP|NIF_ICON;

                Shell_NotifyIcon(NIM_ADD, &nid);

                // disable shutdown
                ShutdownIcon::getInstance()->SetVeto( true );
            }
            return 0;

        case SFX_TASKBAR_NOTIFICATION:
            switch( lParam )
            {
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    SetForegroundWindow( hWnd );
                    int m = TrackPopupMenuEx( popupMenu, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,
                                              pt.x, pt.y, hWnd, NULL );
                    // BUGFIX: See Q135788 (PRB: Menus for Notification Icons Don't Work Correctly)
                    PostMessage( hWnd, NULL, 0, 0 );
                    switch( m )
                    {
                        case IDM_OPEN:
                            ShutdownIcon::FileOpen();
                        break;
                        case IDM_WRITER:
                            ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/swriter" ) ) );
                        break;
                        case IDM_CALC:
                            ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/scalc" ) ) );
                        break;
                        case IDM_IMPRESS:
                            ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/simpress" ) ) );
                        break;
                        case IDM_DRAW:
                            ShutdownIcon::OpenURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/sdraw" ) ) );
                        break;

                        case IDM_EXIT:
                            // delete taskbar icon
                            NOTIFYICONDATA nid;
                            nid.cbSize=sizeof(NOTIFYICONDATA);
                            nid.hWnd = hWnd;
                            nid.uID = ID_QUICKSTART;
                            Shell_NotifyIcon(NIM_DELETE, &nid);

                            // remove listener and
                            //  erminate office if running in background
                            ShutdownIcon::terminateDesktop();
                            break;
                    }
                }
                break;
            }
            break;
        case WM_DESTROY:
            return 0;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void ShutdownIcon::initSystray()
{
    WNDCLASSEXA listenerClass;
    listenerClass.cbSize        = sizeof(WNDCLASSEX);
    listenerClass.style         = 0;
    listenerClass.lpfnWndProc   = listenerWndProc;
    listenerClass.cbClsExtra    = 0;
    listenerClass.cbWndExtra    = 0;
    listenerClass.hInstance     = (HINSTANCE) GetModuleHandle( NULL );
    listenerClass.hIcon         = NULL;
    listenerClass.hCursor       = NULL;
    listenerClass.hbrBackground = NULL;
    listenerClass.lpszMenuName  = NULL;
    listenerClass.lpszClassName = LISTENER_WINDOWCLASS;
    listenerClass.hIconSm       = NULL;

    RegisterClassExA(&listenerClass);

    aListenerWindow = CreateWindowExA(0,
        LISTENER_WINDOWCLASS,       // registered class name
        LISTENER_WINDOWNAME,        // window name
        0,                          // window style
        CW_USEDEFAULT,              // horizontal position of window
        CW_USEDEFAULT,              // vertical position of window
        CW_USEDEFAULT,              // window width
        CW_USEDEFAULT,              // window height
        (HWND) NULL,                // handle to parent or owner window
        NULL,                       // menu handle or child identifier
        (HINSTANCE) GetModuleHandle( NULL ),    // handle to application instance
        NULL                        // window-creation data
        );
}

void ShutdownIcon::deInitSystray()
{
    if( IsWindow( aListenerWindow ) )
    {
        DestroyWindow( aListenerWindow );
        aListenerWindow = NULL;
    }
    UnregisterClassA( LISTENER_WINDOWCLASS, GetModuleHandle( NULL ) );
}

#endif // WNT


