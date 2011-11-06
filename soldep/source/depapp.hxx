/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

