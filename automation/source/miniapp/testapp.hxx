/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testapp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:21:37 $
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
#ifndef _TESTAPP_HXX
#define _TESTAPP_HXX

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif


#include "editwin.hxx"


#define EXECUTE_NO                  0
#define EXECUTE_POSSIBLE            1
#define EXECUTE_YES                 2
#define EXECUTEMODE_ASYNCHRON       1
#define EXECUTEMODE_DIALOGASYNCHRON 2


class MyApp;
class MainWindow : public WorkWindow
{
    MyApp *pApp;

public:
    MainWindow(MyApp *pAppl);
    virtual BOOL Close(); // derived

    void FileExit();
    void FileOpen();
    void TestGross();
    void Tree(GHEditWindow *aEditWin, Window *pBase, int Indent);
    void WinTree();
    void SysDlg();
    DECL_LINK(MenuSelectHdl,MenuBar*);

};
#define PlugInDispatcher MyDispatcher
class MyDispatcher
{
    MainWindow  *pMainWin;

public:
    MyDispatcher(MainWindow  *MainWin) : pMainWin(MainWin) {};
    ~MyDispatcher() {};
    virtual USHORT ExecuteFunction( USHORT nSID, SfxPoolItem** ppArgs = 0, USHORT nMode = 0);
    virtual void SetExecuteMode( USHORT nMode ) {}; // Ist hier sowieso egal
};

class MyApp : public Application
{
    PopupMenu  *MyMenu;
    Timer aCommandTimer;
    PlugInDispatcher *pMyDispatcher;

public:
    MyApp();
    void Main();

    virtual void                Property( ApplicationProperty& );
    virtual PlugInDispatcher*   GetDispatcher();

    MainWindow  *pMainWin;
};

// -----------------------------------------------------------------------

extern MyApp aApp;

#endif

