/*************************************************************************
 *
 *  $RCSfile: sfxhelp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:24 $
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
#ifndef _SFX_HELP_HXX
#define _SFX_HELP_HXX

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#include <childwin.hxx>
#include <dockwin.hxx>

#include <docfac.hxx>
#include <viewfac.hxx>
#include <objsh.hxx>
#include <viewsh.hxx>

class HelpPI;
class SHelpInfo;
struct SpecialLinkInfo;
class Library;
class SfxPrinter;
class DirEntry;
class SvStringsDtor;

struct HelpFileInfo
{
    String  aFileName;
    String  aTitle;
};


class SfxHelpPIWrapper : public SfxChildWindow
{
public:
                            SfxHelpPIWrapper(Window *pParent, USHORT nId,
                                SfxBindings *pBindings, SfxChildWinInfo *pInfo);

                            SFX_DECL_CHILDWINDOW(SfxHelpPIWrapper);

    virtual BOOL            QueryClose();
};

class SfxHelpTipsWrapper : public SfxChildWindow
{
public:
                            SfxHelpTipsWrapper(Window *pParent, USHORT nId,
                                SfxBindings *pBindings, SfxChildWinInfo *pInfo);

                            SFX_DECL_CHILDWINDOW(SfxHelpTipsWrapper);
};



class SfxHelpTipsWindow : public SfxDockingWindow
{
private:
    HelpPI*         mpHelpPI;
    PushButton      maCloseButton;
    Window          maTipWindow;
    CheckBox        maCheckBox;

protected:
    DECL_LINK(      CloseButtonHdl, Button* );
    DECL_LINK(      ShowTip, void* );
    DECL_LINK(      CheckBoxHdl, CheckBox* );

public:
                    SfxHelpTipsWindow( SfxBindings* pBindimgs, SfxChildWindow* pChildWin, Window* pParent );
                    ~SfxHelpTipsWindow();

    virtual void    FillInfo( SfxChildWinInfo& ) const;
    virtual void    Resize();
};



class SfxHelpPI : public SfxDockingWindow
{
    HelpPI*             pHelpPI;
    Window*             pInnerWindow;

    Timer               aTopicJustRequestedTimer;

    ULONG               nTip;
    CheckBox            aTipBox;

    BOOL                bInShowMe;

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& );

public:
                    SfxHelpPI( SfxBindings* pBindimgs, SfxChildWindow* pChildWin,
                                Window* pParent, USHORT nScale );
                    ~SfxHelpPI();

    void            LoadTopic( const String& rFileName, ULONG nId );
    void            LoadTopic( ULONG nId );
    void            LoadTopic( const String& rKeyword );
    void            ResetTopic();

    BOOL            Close();

    BOOL            IsConstructed() const { return ( pHelpPI != 0 ); }
    String          GetExtraInfo() const;

    HelpPI*         GetHelpPI() const { return pHelpPI; }

    virtual void    FillInfo( SfxChildWinInfo& ) const;

    void            SetTip( ULONG nId );
    ULONG           GetTip() const { return nTip; }
    void            SetTipText( const String& rText );

    BOOL            IsInShowMe() const { return bInShowMe; }

    // Nach F1 fuer einige ms nicht aufgrund von FocusChanged ein anderes Topic laden...
    BOOL            IsTopicJustRequested() const { return aTopicJustRequestedTimer.IsActive(); }
    void            SetTopicJustRequested( BOOL bOn ) { if( bOn )
                                                            aTopicJustRequestedTimer.Start();
                                                        else
                                                            aTopicJustRequestedTimer.Stop(); }

#if __PRIVATE
    DECL_LINK(      TopicChangedHdl_Impl, void* );
    DECL_LINK(      SpecialLinkHdl, SpecialLinkInfo* );
    DECL_LINK(      TipBoxHdl, CheckBox* );
    DECL_LINK(      PIToolboxHdl, ToolBox* );
#endif
};



class SfxHelp
{
public:
    static  BOOL            ShowHelp( ULONG nId, BOOL bShowInHelpAgent, const char* pFileName = 0, BOOL bQuiet = FALSE );
    static  BOOL            ShowHelp( const String& rKeyword, BOOL bShowInHelpAgent, const char* pFileName = 0 );
    static  void            ShowHint( ULONG nId );
    static  void            SetCustomHelpFile( const String& rName );
    static  USHORT          GetHelpFileInfoCount();
    static  HelpFileInfo*   GetHelpFileInfo( USHORT n );
};

USHORT ImplSetLanguageGroup( Config& rConfig, const String& rGroupName, BOOL bSearchLanguage );



#if __PRIVATE

SV_DECL_VARARR_SORT( SortedULONGs, ULONG, 0, 4 );
//SV_DECL_PTRARR_DEL( HelpTextCaches, HelpTextCache*, 0, 4 );


class SfxHelp_Impl : public Help, public SfxListener
{
    friend class SfxHelp;

    String              aCustomHelpFile;
    String              aCurHelpFile;   // Kurzer Name ohne Pfad

    SHelpInfo*          pHelpInfo;      // FÅr GetHelpText()

//    HelpTextCache*      pHelpCache;
//    HelpTextCaches      aHelpCaches;

    Timer               aDialogDetector;

    SortedULONGs*       pPIStarterList;

    ULONG               nLastDialog;
    BOOL                bForcedFloatingPI;

    List*               pHelpFileInfos;

private:
    BOOL            ImplStart( ULONG nHelpId, BOOL bCheckHelpFile, BOOL bChangeHelpFile, BOOL bHelpAgent );
    virtual BOOL    Start( ULONG nHelpId );
#ifndef ENABLEUNICODE
    virtual BOOL    Start( const String& rKeyWord );
#else
    virtual BOOL    Start( const UniString& rKeyWord );
#endif
    void            SetCurrentHelpFile( ULONG nId );
    String          GetCurrentHelpFile() const { return aCurHelpFile; }

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );


    inline SortedULONGs*    GetPIStarterList();
    void                    CreatePIStarterList();


    DECL_LINK(      DialogDetectHdl, Timer* );

protected:
    void            AssertValidHelpDocInfo();

public:

                    SfxHelp_Impl();
                    ~SfxHelp_Impl();

    static String   GetHelpPath();
    static String   GetHelpFileName( ULONG nId );
    BOOL            CheckHelpFile( BOOL bPrompt ) const;
    static BOOL     CheckHelpFile( const String& rFilename, BOOL bPrompt );

    static Window*  SearchFocusWindowParent();
    void            CheckPIPosition();

    void            SetHelpFile( const String &rHelpFileName, BOOL bAdjustExt = TRUE );
    const String&   GetCurHelpFile() const { return aCurHelpFile; }

    XubString       GetHelpText( ULONG nHelpId );

    void            GetHelpURLs( const String& rRootURL, SvStringsDtor& rLst );
    void            GetBookmarks( SvStringsDtor& rLst );
    void            AddBookmark( const String& rName, const String& rURL );
    void            RemoveBookmark( const String& rName );
    void            RenameBookmark( const String& rOldTitle, const String& rNewTitle );

    void            SlotExecutedOrFocusChanged( ULONG nId, BOOL bSlot, BOOL bAutoStart );
    void            EnableTip( ULONG nTip, BOOL bEnable );

    void            ResetPIStarterList();

    void            HelpAgentClosed();

    void            StartHelpPI( ULONG nHelpId, BOOL bSlot, BOOL bTip = FALSE );

    USHORT          GetHelpFileInfoCount();
    HelpFileInfo*   GetHelpFileInfo( USHORT n );

    static String   GetConfigDir( BOOL bGetSharedConfig );
    static String   GetHelpAgentConfig();
};

inline SortedULONGs* SfxHelp_Impl::GetPIStarterList()
{
    if ( !pPIStarterList )
        CreatePIStarterList();
    return pPIStarterList;
}

#endif // _PRIVATE

#endif // #ifndef _SFX_HELP_HXX

