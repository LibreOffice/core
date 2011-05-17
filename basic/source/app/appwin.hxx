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

#include <vector>
typedef sal_uInt16 QueryBits;

#define QUERY_NONE              ( QueryBits ( 0x00 ) )
#define QUERY_DIRTY             ( QueryBits ( 0x01 ) )
#define QUERY_DISK_CHANGED      ( QueryBits ( 0x02 ) )
#define QUERY_ALL               ( QUERY_DIRTY | QUERY_DISK_CHANGED )
#define SAVE_NOT_DIRTY          ( QueryBits ( 0x04 ) )

#define SAVE_RES_SAVED          sal_True
#define SAVE_RES_NOT_SAVED      sal_False
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
    sal_uInt16 nSkipReload;             // Sometimes there must not be a reload
    sal_Bool bHasFile;                  // Otherwise reload does not make sense
    sal_Bool bReloadAborted;            // Is set if reload was cancelled so that we can ask again wehn closing

    short nId;                      // ID-Nummer( "Unnamed n" )
    BasicFrame* pFrame;             // Parent-Window
    String aFind;                   // Search string
    String aReplace;                // Replace string
    sal_Bool bFind;                     // sal_True if search not replace
    void RequestHelp( const HelpEvent& );           // Help handler
    void GetFocus();                // activate
    virtual sal_uInt16 ImplSave();      // Save file
    sal_uInt16 nWinState;               // Maximized, Iconized or Normal
    Point nNormalPos;               // Position if normal
    Size nNormalSize;               // Size if Normal
    virtual long    PreNotify( NotifyEvent& rNEvt );
    sal_uInt16 nWinId;

public:
    TYPEINFO();
    AppWin( BasicFrame* );
    ~AppWin();
    DataEdit* pDataEdit;                // Data area
    virtual sal_uInt16 GetLineNr()=0;       // Current line number
    virtual long InitMenu( Menu* );     // Init of the menu
    virtual long DeInitMenu( Menu* );   // reset to enable all shortcuts
    virtual void Command( const CommandEvent& rCEvt );  // Command handler
    virtual void Resize();
    virtual void Help();
    virtual sal_Bool Load( const String& ); // Load file
    virtual void PostLoad(){}               // Set source at module
    virtual sal_uInt16 SaveAs();                // Save file as
    virtual void PostSaveAs(){}
    virtual void Find();                    // find text
    virtual void Replace();                 // replace text
    virtual void Repeat();                  // repeat find/replace
    virtual sal_Bool Close();                   // close window
    virtual void Activate();                // window was activated
    virtual FileType GetFileType()=0;   // returns the filetype
    virtual sal_Bool ReloadAllowed(){ return sal_True; }
    virtual void Reload();                  // Reload after change on disk
    virtual void LoadIniFile(){;}       // (re)load ini file after change
    void CheckReload();                     // Checks and asks if reload should performed
    sal_Bool DiskFileChanged( sal_uInt16 nWhat );   // Checks file for changes
    void UpdateFileInfo( sal_uInt16 nWhat );    // Remembers last file state
    sal_Bool IsSkipReload();                    // Should we test reload?
    void SkipReload( sal_Bool bSkip = sal_True );
    sal_uInt16 GetWinState(){ return nWinState; }
    void Maximize();
    void Restore();
    void Minimize( sal_Bool bMinimize );
    void Cascade( sal_uInt16 nNr );

    sal_uInt16 QuerySave( QueryBits nBits = QUERY_ALL );
    sal_Bool IsModified()               { return pDataEdit->IsModified(); }
    BasicFrame* GetBasicFrame() { return pFrame; }
    virtual void    TitleButtonClick( sal_uInt16 nButton );
    virtual void    SetText( const XubString& rStr );

    sal_uInt16 GetWinId() { return nWinId; }
    void SetWinId( sal_uInt16 nWId ) { nWinId = nWId; }
};

typedef ::std::vector< AppWin* > EditList;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
