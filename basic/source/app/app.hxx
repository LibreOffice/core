/*************************************************************************
 *
 *  $RCSfile: app.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gh $ $Date: 2001-03-14 11:33:53 $
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

#ifndef _BASICAPP_HXX
#define _BASICAPP_HXX

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

class BasicFrame;
#include "mybasic.hxx"

class EditList;
class AppWin;
class AppEdit;
class AppBasEd;
class MsgEdit;
class AppError;
class StatusLine;
class BasicPrinter;
struct TTLogMsg;

class BasicApp : public Application {
    short       nWait;              // Wait-Zaehler
public:
//  Help*       pHelp;              // Hilfesystem
    BasicFrame* pFrame;             // Frame Window
//  MenuBar*    pMainMenu;          // Hauptmenue
    Accelerator*    pMainAccel;     // Acceleratoren

#ifndef VCL
    void  Main( int, char*[] );
#else
    void  Main( );
#endif

    void  LoadIniFile();
    void  SetFocus();
    void  Wait( BOOL );
    DECL_LINK( LateInit, void * );
};


typedef USHORT FileType;

#define FT_NO_FILE          (FileType)0x00      // Ein Fehler ist aufgetreten ...
#define FT_BASIC_SOURCE     (FileType)0x01
#define FT_BASIC_INCLUDE    (FileType)0x02
#define FT_RESULT_FILE      (FileType)0x04
#define FT_RESULT_FILE_TXT  (FileType)0x08
#define FT_BASIC_LIBRARY    (FileType)0x10

struct WinInfoRec;
class DisplayHidDlg;

class FloatingExecutionStatus;

class BasicFrame : public WorkWindow, public SfxBroadcaster, public SfxListener
{
virtual BOOL Close();               // Schliessen
    BOOL CloseAll();                // Alle Fenster schliessen
    BOOL CompileAll();              // Alle Texte compilieren
    AutoTimer aLineNum;             // Zeigt die Zeilennummer an
virtual void Resize();
virtual void Move();
    void LoadLibrary();
    void SaveLibrary();
    BOOL bIsAutoRun;
    DisplayHidDlg* pDisplayHidDlg;

//  BreakPoint *pRunToCursorBP;

    SbxVariable *pEditVar;



    Timer aCheckFiles;      // Prüfen der Dateien auf Änderungen
    BOOL bAutoReload;
    DECL_LINK( CheckAllFiles, Timer* );

    MyBasicRef  pBasic;             // BASIC-Engine

    String aAppName;                // Inhalt der Titelteile der App:
    String aAppFile;                // AppName AppFile [AppMode]
    String aAppMode;
    void UpdateTitle();
    DECL_LINK( CloseButtonClick, void* );
    DECL_LINK( FloatButtonClick, void* );
    DECL_LINK( HideButtonClick, void* );

    FloatingExecutionStatus *pExecutionStatus;

public:
    BOOL IsAutoRun();
    void SetAutoRun( BOOL bAuto );
    BOOL bInBreak;                  // TRUE, wenn im Break-Handler
    StatusLine* pStatus;            // Statuszeile
    EditList*   pList;              // List der Edit-Fenster
    AppWin*     pWork;              // aktuelles Edit-Fenster
    BasicPrinter* pPrn;             // Drucker
    BOOL bDisas;                    // TRUE: disassemble
    USHORT nFlags;                  // Debugging-Flags
    USHORT nMaximizedWindows;       // Anzahl der Fenster, die maximized sind
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
    BOOL LoadFile( String aFilename );
    long Command( short,BOOL=FALSE );// Kommando-Handler
    BOOL SaveAll();                 // Alle Fenster speichern
    BOOL QueryFileName( String& rName, FileType nFileType, BOOL bSave );// Dateinamen ermitteln
    DECL_LINK( ModuleWinExists, String* );
    AppBasEd* FindModuleWin( const String& );
    AppError* FindErrorWin( const String& );
    AppWin* FindWin( const String& );
    AppWin* FindWin( USHORT nWinId );
    AppWin* IsWinValid( AppWin* pMaybeWin );
    USHORT BreakHandler();          // Break-Handler-Callback

    void SetEditVar( SbxVariable *pVar ){ pEditVar = pVar;}
    SbxVariable* GetEditVar(){ return pEditVar;}
    BOOL IsAutoReload() { return bAutoReload; }
    void LoadIniFile();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void SetAppMode( const String &aNewMode ){ aAppMode = aNewMode; UpdateTitle(); }

    String GenRealString( const String &aResString );

};

extern BasicApp aBasicApp;

#endif
