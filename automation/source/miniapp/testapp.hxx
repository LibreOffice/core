/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _TESTAPP_HXX
#define _TESTAPP_HXX

#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/menu.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/poolitem.hxx>


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
    virtual sal_Bool Close(); // derived

    void FileExit();
    void FileOpen();
    void TestGross();
    void Tree(GHEditWindow *aEditWin, Window *pBase, sal_uInt16 Indent);
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
    virtual sal_uInt16 ExecuteFunction( sal_uInt16 nSID, SfxPoolItem** ppArgs = 0, sal_uInt16 nMode = 0);
    virtual void SetExecuteMode( sal_uInt16 nMode )
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
    int Main();

    virtual void                Property( ApplicationProperty& );
    virtual PlugInDispatcher*   GetDispatcher();

    MainWindow  *pMainWin;
};

// -----------------------------------------------------------------------

extern MyApp aApp;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
