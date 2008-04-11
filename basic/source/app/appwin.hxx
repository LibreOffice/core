/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: appwin.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _APPWIN_HXX
#define _APPWIN_HXX

//#include <sb.hxx>
#include <vcl/dockwin.hxx>
#include <tools/fsys.hxx>

#include "app.hxx"
#include "dataedit.hxx"

typedef USHORT QueryBits;
#define QUERY_NONE              ( QueryBits ( 0x00 ) )
#define QUERY_DIRTY             ( QueryBits ( 0x01 ) )
#define QUERY_DISK_CHANGED      ( QueryBits ( 0x02 ) )
#define QUERY_ALL               ( QUERY_DIRTY | QUERY_DISK_CHANGED )
#define SAVE_NOT_DIRTY          ( QueryBits ( 0x04 ) )

#define SAVE_RES_SAVED          TRUE
#define SAVE_RES_NOT_SAVED      FALSE
#define SAVE_RES_ERROR          3
#define SAVE_RES_CANCEL         4


#define SINCE_LAST_LOAD         1
#define SINCE_LAST_ASK_RELOAD   2

#define HAS_BEEN_LOADED         1       // includes ASKED_RELOAD
#define ASKED_RELOAD            2


#define TT_WIN_STATE_MAX        0x01
#define TT_WIN_STATE_FLOAT      0x02
#define TT_WIN_STATE_HIDE       0x04

class BasicFrame;

class AppWin : public DockingWindow, public SfxListener     // Dokumentfenster
{
    friend class MsgEdit;
protected:
    static short nNumber;           // fortlaufende Nummer
    static short nCount;            // Anzahl Editfenster
    static String *pNoName;         // "Untitled"
    FileStat aLastAccess;           // Wann wurde die geladene Dateiversion verändert
    USHORT nSkipReload;             // Manchmal darf kein Reload erfolgen
    BOOL bHasFile;                  // Ansonsten hat reload auch keinen Sinn
    BOOL bReloadAborted;            // Wird gesetzt, wenn reload abgelehnt wurde, so daß beim Schließen nochmal gefragt werden kann

    short nId;                      // ID-Nummer( "Unbenannt n" )
    BasicFrame* pFrame;             // Parent-Window
//  Icon* pIcon;                    // Dokument-Icon
    String aFind;                   // Suchstring
    String aReplace;                // Ersetze-String
    BOOL bFind;                     // TRUE, wenn Suchen und nicht Ersetzen
    void RequestHelp( const HelpEvent& );// Hilfe-Handler
    void GetFocus();                // aktivieren
    virtual USHORT ImplSave();      // Datei speichern
    USHORT nWinState;               // Maximized, Iconized oder Normal
    Point nNormalPos;               // Position wenn Normal
    Size nNormalSize;               // Größe wenn Normal
    virtual long    PreNotify( NotifyEvent& rNEvt );
    USHORT nWinId;

public:
    TYPEINFO();
    AppWin( BasicFrame* );
    ~AppWin();
    DataEdit* pDataEdit;                // Daten-Flaeche
    virtual USHORT GetLineNr()=0;       // Aktuelle Zeilennummer
    virtual long InitMenu( Menu* );     // Initialisierung des Menues
    virtual long DeInitMenu( Menu* );   // rücksetzen, so daß wieder alle Shortcuts enabled sind
    virtual void Command( const CommandEvent& rCEvt );  // Kommando-Handler
    virtual void Resize();              // Aenderung Fenstergroesse
    virtual void Help();                // Hilfe aktivieren
    virtual BOOL Load( const String& ); // Datei laden
    virtual void PostLoad(){}           // Nachbearbeiten des geladenen (Source am Modul setzen)
    virtual USHORT SaveAs();                // Datei unter neuem Namen speichern
    virtual void PostSaveAs(){}         // Nachbearbeiten des Moduls ...
    virtual void Find();                // Text suchen
    virtual void Replace();             // Text ersetzen
    virtual void Repeat();              // Suche wiederholen
    virtual BOOL Close();               // Fenster schliessen
    virtual void Activate();            // Fenster wurde aktiviert
    virtual FileType GetFileType()=0;   // Liefert den Filetype
    virtual BOOL ReloadAllowed(){ return TRUE; } // Ermöglicht dem Dok temporär NEIN zu sagen
    virtual void Reload();              // Reload nach änderung auf Platte
    virtual void LoadIniFile(){;}       // (re)load ini file after change
    void CheckReload();                 // Prüft und Fragt ob reloaded werden soll
    BOOL DiskFileChanged( USHORT nWhat );   // Prüft ob die Datei sich verändert hat
    void UpdateFileInfo( USHORT nWhat );    // Merkt sich den aktuellen Zustand der Datei
    BOOL IsSkipReload();                // Soll reload getestet werden
    void SkipReload( BOOL bSkip = TRUE );
    USHORT GetWinState(){ return nWinState; }
    void Maximize();
    void Restore();
    void Minimize( BOOL bMinimize );
    void Cascade( USHORT nNr );

    USHORT QuerySave( QueryBits nBits = QUERY_ALL );        // Speichern
    BOOL IsModified()               { return pDataEdit->IsModified(); }
    BasicFrame* GetBasicFrame() { return pFrame; }
    virtual void    TitleButtonClick( USHORT nButton );
    virtual void    SetText( const XubString& rStr );

    USHORT GetWinId() { return nWinId; }
    void SetWinId( USHORT nWId ) { nWinId = nWId; }
};

DECLARE_LIST( EditList, AppWin* )

#endif
