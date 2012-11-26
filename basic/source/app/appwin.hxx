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



#ifndef _APPWIN_HXX
#define _APPWIN_HXX

//#include <sb.hxx>
#include <vcl/dockwin.hxx>
#include <tools/fsys.hxx>

#include "app.hxx"
#include "dataedit.hxx"

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
//  Icon* pIcon;                    // Document icon
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

DECLARE_LIST( EditList, AppWin* )

#endif
