/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DEPAPP_HXX
#define _DEPAPP_HXX


#define EVENT_RESIZE                0x00000001


#include <svl/solar.hrc>
#include <tools/resmgr.hxx>
#include <soldep/sstring.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <soldep/soldep.hxx>
//#include <soldep/soldlg.hrc>
//#include <soldep/tbox.hxx>
// -----------------------------------------------------------------------
#include <sal/main.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/weak.hxx>
#include <soldep/soldep.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
class MyWin;

class MyApp : public Application
{
    ResMgr*     pResMgr;
    SolDep*     pSolDep;
    Application* pMyApp;
    MyWin*      pAppWindow;

public:
//                MyApp();
//               ~MyApp() {};
#if defined(WNT)
        FILE*       pDebugFile;
#endif

    void        Main();
    ResMgr*     GetResMgr() const { return pResMgr; }
    Application* GetMyApp() { return pMyApp; }
    MyWin*      GetAppWin() { return pAppWindow; }
    SolDep*     GetSolDep() { return pSolDep; }
};

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
protected:
    MenuBar     aMenuBar;
//    SoldepToolBox     aToolBox; //->depper.hxx
    Wallpaper   aWallpaper;
//    FloatingWindow aTaskBarFrame; // For ToolBox

public:
                MyWin( Window* pParent, WinBits nWinStyle );

    void        MouseMove( const MouseEvent& rMEvt );
    void        MouseButtonDown( const MouseEvent& rMEvt );
    void        MouseButtonUp( const MouseEvent& rMEvt );
    void        KeyInput( const KeyEvent& rKEvt );
    void        KeyUp( const KeyEvent& rKEvt );
    void        Paint( const Rectangle& rRect );
    void        Resize();
//    FloatingWindow* GetTaskBarFrame() { return &aTaskBarFrame; }
//    SoldepToolBox* GetSoldepToolBox() { return &aToolBox; }

};
static MyApp aMyApp;

#endif

