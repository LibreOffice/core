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

#ifndef _BASICAPP_HXX
#define _BASICAPP_HXX

#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/menu.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/timer.hxx>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>

class BasicFrame;
#include <basic/mybasic.hxx>
#include <vector>

class AppWin;
class AppEdit;
class AppBasEd;
class MsgEdit;
class AppError;
class StatusLine;
class BasicPrinter;
struct TTLogMsg;

typedef ::std::vector< AppWin* > EditList;

class BasicApp : public Application {
    short       nWait;              // Wait-Zaehler
public:
    BasicFrame* pFrame;             // Frame Window
    Accelerator*    pMainAccel;     // Acceleratoren

    int Main( );

    void  LoadIniFile();
    void  SetFocus();
    void  Wait( sal_Bool );
    DECL_LINK( LateInit, void * );

#ifdef DBG_UTIL
    DbgPrintLine DbgPrintMsgBox;
#endif
};


typedef sal_uInt16 FileType;

#define FT_NO_FILE              (FileType)0x00  // An error has occurred ...
#define FT_BASIC_SOURCE         (FileType)0x01
#define FT_BASIC_INCLUDE    (FileType)0x02
#define FT_RESULT_FILE      (FileType)0x04
#define FT_RESULT_FILE_TXT  (FileType)0x08
#define FT_BASIC_LIBRARY    (FileType)0x10

struct WinInfoRec;
class DisplayHidDlg;

class FloatingExecutionStatus;

class BasicFrame : public WorkWindow, public SfxBroadcaster, public SfxListener
{
using SystemWindow::Notify;
using Window::Command;

    virtual sal_Bool Close();
    sal_Bool CloseAll();          // Close all windows
    sal_Bool CompileAll();        // Compile all texts
    AutoTimer aLineNum;       // Show the line numbers
    virtual void Resize();
    virtual void Move();
    virtual void GetFocus();
    void LoadLibrary();
    void SaveLibrary();
    sal_Bool bIsAutoRun;
    DisplayHidDlg* pDisplayHidDlg;


    SbxVariable *pEditVar;



    Timer aCheckFiles;      // Checks the files for changes
    sal_Bool bAutoReload;
    sal_Bool bAutoSave;
    DECL_LINK( CheckAllFiles, Timer* );

    MyBasicRef  pBasic;             // BASIC-Engine

    String aAppName;                // Title bar content
    String aAppFile;                // AppName AppFile [AppMode]
    String aAppMode;
    void UpdateTitle();
    DECL_LINK( CloseButtonClick, void* );
    DECL_LINK( FloatButtonClick, void* );
    DECL_LINK( HideButtonClick, void* );

    FloatingExecutionStatus *pExecutionStatus;

public:
    sal_Bool IsAutoRun();
    void SetAutoRun( sal_Bool bAuto );
    sal_Bool bInBreak;                  // sal_True if in Break-Handler
    StatusLine* pStatus;            // Status line
    EditList*   pList;              // List of edit windows
    AppWin*     pWork;              // Current edit window
    BasicPrinter* pPrn;             // Printer
    sal_Bool bDisas;                    // sal_True: disassemble
    sal_uInt16 nFlags;                  // Debugging-Flags
    sal_uInt16 nMaximizedWindows;       // Number of maximized windows
    void FocusWindow( AppWin *pWin );
    void WinMax_Restore();
    void WinShow_Hide();
    void RemoveWindow( AppWin *pWin );
    void AddWindow( AppWin *pWin );
    void WindowRenamed( AppWin *pWin );

    BasicFrame();
   ~BasicFrame();
    MyBasic& Basic()                { return *pBasic; }
    void AddToLRU(String const& aFile);
    void LoadLRU();
    DECL_LINK( InitMenu, Menu * );
    DECL_LINK( DeInitMenu, Menu * );
    DECL_LINK( HighlightMenu, Menu * );
    DECL_LINK( MenuCommand, Menu * );
    DECL_LINK( Accel, Accelerator * );
    DECL_LINK( ShowLineNr, AutoTimer * );
    MsgEdit* GetMsgTree( String aLogFileName );
    DECL_LINK( Log, TTLogMsg * );
    DECL_LINK( WinInfo, WinInfoRec * );
    sal_Bool LoadFile( String aFilename );
    long Command( short,sal_Bool=sal_False );  // Command handler
    virtual void Command( const CommandEvent& rCEvt );      // Command handler
    sal_Bool SaveAll();                 // Save all windows
    sal_Bool QueryFileName( String& rName, FileType nFileType, sal_Bool bSave ); // Query for filename
    DECL_LINK( ModuleWinExists, String* );
    DECL_LINK( WriteString, String* );
    AppBasEd* CreateModuleWin( SbModule* pMod );
    AppBasEd* FindModuleWin( const String& );
    AppError* FindErrorWin( const String& );
    AppWin* FindWin( const String& );
    AppWin* FindWin( sal_uInt16 nWinId );
    AppWin* IsWinValid( AppWin* pMaybeWin );
    sal_uInt16 BreakHandler();          // Break-Handler-Callback

    void SetEditVar( SbxVariable *pVar ){ pEditVar = pVar;}
    SbxVariable* GetEditVar(){ return pEditVar;}
    sal_Bool IsAutoReload() { return bAutoReload; }
    sal_Bool IsAutoSave() { return bAutoSave; }
    void LoadIniFile();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void SetAppMode( const String &aNewMode ){ aAppMode = aNewMode; UpdateTitle(); }

    String GenRealString( const String &aResString );
    Rectangle GetInnerRect() const;

};

extern BasicApp aBasicApp;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
