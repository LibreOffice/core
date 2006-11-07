/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hltpbase.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:50:03 $
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
#ifndef _SVX_TABBASE_HYPERLINK_HXX
#define _SVX_TABBASE_HYPERLINK_HXX

#define INET_TELNET_SCHEME      "telnet://"

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _MACROPG_HXX
#include <sfx2/macropg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SVTOOLS_INETTBC_HXX
#include <svtools/inettbc.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#include "dialmgr.hxx"
#include <sfx2/docfile.hxx>
#include "dialogs.hrc"

#ifndef _SVX_HELPID_HRC
#include "helpid.hrc"
#endif

#include "hlnkitem.hxx"

#include "hlmarkwn.hxx"

#ifndef _ICCDLG_HXX
#include "iconcdlg.hxx"
#endif


/*************************************************************************
|*
|* ComboBox-Control, wich is filled with all current framenames
|*
\************************************************************************/

class SvxFramesComboBox : public ComboBox
{
public:
    SvxFramesComboBox (Window* pParent, const ResId& rResId, SfxDispatcher* pDispatch);
    ~SvxFramesComboBox ();
};

/*************************************************************************
|*
|* ComboBox-Control for URL's with History and Autocompletion
|*
\************************************************************************/

class SvxHyperURLBox : public SvtURLBox, public DropTargetHelper
{
private:
    BOOL   mbAccessAddress;

//  String GetAllEmailNamesFromDragItem( USHORT nItem );

protected:

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual long        Notify( NotifyEvent& rNEvt );
    virtual void        Select();
    virtual void        Modify();
    virtual long        PreNotify( NotifyEvent& rNEvt );

public:
    SvxHyperURLBox( Window* pParent, INetProtocol eSmart = INET_PROT_FILE, BOOL bAddresses = FALSE );

};

/*************************************************************************
|*
|* Tabpage : Basisclass
|*
\************************************************************************/

class SvxHyperlinkTabPageBase : public IconChoicePage
{
private:
    FixedLine           *mpGrpMore;
    FixedText           *mpFtFrame;
    SvxFramesComboBox   *mpCbbFrame;
    FixedText           *mpFtForm;
    ListBox             *mpLbForm;
    FixedText           *mpFtIndication;
    Edit                *mpEdIndication;
    FixedText           *mpFtText;
    Edit                *mpEdText;
    ImageButton         *mpBtScript;

    sal_Bool            mbIsCloseDisabled;

protected:
    Window*             mpDialog;

    BOOL                mbStdControlsInit;

    String              maStrInitURL;

    Timer               maTimer;

    SvxHlinkDlgMarkWnd* mpMarkWnd;

    void InitStdControls ();
    virtual void FillStandardDlgFields ( SvxHyperlinkItem* pHyperlinkItem );
    virtual void FillDlgFields         ( String& aStrURL ) = 0;
    virtual void GetCurentItemData     ( String& aStrURL, String& aStrName,
                                         String& aStrIntName, String& aStrFrame,
                                         SvxLinkInsertMode& eMode ) = 0;
    virtual String CreateUiNameFromURL( const String& aStrURL );

    void         GetDataFromCommonFields( String& aStrName,
                                          String& aStrIntName, String& aStrFrame,
                                          SvxLinkInsertMode& eMode );

    DECL_LINK (ClickScriptHdl_Impl, void * );       // Button : Script

    String              aEmptyStr;

    BOOL            FileExists( const INetURLObject& rURL );
    static String   GetSchemeFromURL( String aStrURL );

    inline void     DisableClose( sal_Bool _bDisable ) { mbIsCloseDisabled = _bDisable; }

public:
    SvxHyperlinkTabPageBase ( Window *pParent, const ResId &rResId, const SfxItemSet& rItemSet);
    virtual ~SvxHyperlinkTabPageBase ();

    virtual BOOL AskApply ();
    virtual void DoApply ();
    virtual void SetOnlineMode( BOOL bEnable );
    virtual void SetInitFocus();
    virtual void SetMarkStr ( String& aStrMark );
    virtual void Reset( const SfxItemSet& );
    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void ActivatePage( const SfxItemSet& rItemSet );
    virtual int  DeactivatePage( SfxItemSet* pSet = 0 );

    BOOL IsMarkWndVisible ()      { return ((Window*)mpMarkWnd)->IsVisible(); }
    Size GetSizeExtraWnd ()       { return ( mpMarkWnd->GetSizePixel() ); }
    BOOL MoveToExtraWnd ( Point aNewPos, BOOL bDisConnectDlg = FALSE );

    virtual void        ActivatePage();
    virtual void        DeactivatePage();
    virtual sal_Bool    QueryClose();

protected:
    virtual BOOL ShouldOpenMarkWnd();
    virtual void SetMarkWndShouldOpen(BOOL bOpen);

    void ShowMarkWnd ();
    void HideMarkWnd ()           { ( ( Window* ) mpMarkWnd )->Hide(); }
    void InvalidateMarkWnd ()     { ( ( Window* ) mpMarkWnd )->Invalidate(); }

    SfxDispatcher* GetDispatcher() const;

    USHORT             GetMacroEvents();
    SvxMacroTableDtor* GetMacroTable();

    const BOOL IsHTMLDoc() const;
};

#endif // _SVX_TABBASE_HYPERLINK_HXX

