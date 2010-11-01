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

#ifndef _APPWIN_HXX
#define _APPWIN_HXX

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

class AppWin : public DockingWindow, public SfxListener // Document window
{
    friend class MsgEdit;
protected:
    static short nNumber;           // serial number
    static short nCount;            // number of edit windows
    static String *pNoName;         // "Untitled"
    FileStat aLastAccess;           // Last access time of loaded file
    USHORT nSkipReload;             // Sometimes there must not be a reload
    BOOL bHasFile;                  // Otherwise reload does not make sense
    BOOL bReloadAborted;            // Is set if reload was cancelled so that we can ask again wehn closing

    short nId;                      // ID-Nummer( "Unnamed n" )
    BasicFrame* pFrame;             // Parent-Window
//  Icon* pIcon;                    // Document icon
    String aFind;                   // Search string
    String aReplace;                // Replace string
    BOOL bFind;                     // TRUE if search not replace
    void RequestHelp( const HelpEvent& );           // Help handler
    void GetFocus();                // activate
    virtual USHORT ImplSave();      // Save file
    USHORT nWinState;               // Maximized, Iconized or Normal
    Point nNormalPos;               // Position if normal
    Size nNormalSize;               // Size if Normal
    virtual long    PreNotify( NotifyEvent& rNEvt );
    USHORT nWinId;

public:
    TYPEINFO();
    AppWin( BasicFrame* );
    ~AppWin();
    DataEdit* pDataEdit;                // Data area
    virtual USHORT GetLineNr()=0;       // Current line number
    virtual long InitMenu( Menu* );     // Init of the menu
    virtual long DeInitMenu( Menu* );   // reset to enable all shortcuts
    virtual void Command( const CommandEvent& rCEvt );  // Command handler
    virtual void Resize();
    virtual void Help();
    virtual BOOL Load( const String& ); // Load file
    virtual void PostLoad(){}               // Set source at module
    virtual USHORT SaveAs();                // Save file as
    virtual void PostSaveAs(){}
    virtual void Find();                    // find text
    virtual void Replace();                 // replace text
    virtual void Repeat();                  // repeat find/replace
    virtual BOOL Close();                   // close window
    virtual void Activate();                // window was activated
    virtual FileType GetFileType()=0;   // returns the filetype
    virtual BOOL ReloadAllowed(){ return TRUE; }
    virtual void Reload();                  // Reload after change on disk
    virtual void LoadIniFile(){;}       // (re)load ini file after change
    void CheckReload();                     // Checks and asks if reload should performed
    BOOL DiskFileChanged( USHORT nWhat );   // Checks file for changes
    void UpdateFileInfo( USHORT nWhat );    // Remembers last file state
    BOOL IsSkipReload();                    // Should we test reload?
    void SkipReload( BOOL bSkip = TRUE );
    USHORT GetWinState(){ return nWinState; }
    void Maximize();
    void Restore();
    void Minimize( BOOL bMinimize );
    void Cascade( USHORT nNr );

    USHORT QuerySave( QueryBits nBits = QUERY_ALL );
    BOOL IsModified()               { return pDataEdit->IsModified(); }
    BasicFrame* GetBasicFrame() { return pFrame; }
    virtual void    TitleButtonClick( USHORT nButton );
    virtual void    SetText( const XubString& rStr );

    USHORT GetWinId() { return nWinId; }
    void SetWinId( USHORT nWId ) { nWinId = nWId; }
};

DECLARE_LIST( EditList, AppWin* )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
