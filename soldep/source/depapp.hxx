/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: depapp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-27 07:54:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DEPAPP_HXX
#define _DEPAPP_HXX


#define EVENT_RESIZE                0x00000001


#include <svtools/solar.hrc>
#include <tools/resmgr.hxx>
#include <bootstrp/sstring.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include "soldep.hxx"
//#include "soldlg.hrc"
//#include "tbox.hxx"
// -----------------------------------------------------------------------
#include <sal/main.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/weak.hxx>
#include "soldep.hxx"

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

