/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: testapp.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _TESTAPP_HXX
#define _TESTAPP_HXX

#include <vcl/svapp.hxx>
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
#include <svtools/poolitem.hxx>


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
    void Tree(GHEditWindow *aEditWin, Window *pBase, USHORT Indent);
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
    virtual ~MyDispatcher() {};
    virtual USHORT ExecuteFunction( USHORT nSID, SfxPoolItem** ppArgs = 0, USHORT nMode = 0);
    virtual void SetExecuteMode( USHORT nMode )
    {
        (void) nMode; /* avoid warning about unused parameter */
    };  // Ist hier sowieso egal
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

