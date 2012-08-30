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
#ifndef _SVX_TABBASE_HYPERLINK_HXX
#define _SVX_TABBASE_HYPERLINK_HXX

#include <sfx2/app.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/inettbc.hxx>
#include <vcl/timer.hxx>

#include <dialmgr.hxx>
#include <sfx2/docfile.hxx>
#include <cuires.hrc>
#include <com/sun/star/frame/XFrame.hpp>
#include "helpid.hrc"
#include <svx/hlnkitem.hxx>

#include "hlmarkwn.hxx"
#include "iconcdlg.hxx"

/// ComboBox-Control, wich is filled with all current framenames
class SvxFramesComboBox : public ComboBox
{
public:
    SvxFramesComboBox (Window* pParent, const ResId& rResId, SfxDispatcher* pDispatch);
    ~SvxFramesComboBox ();
};

/// ComboBox-Control for URL's with History and Autocompletion
class SvxHyperURLBox : public SvtURLBox, public DropTargetHelper
{
protected:
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual long        Notify( NotifyEvent& rNEvt );
    virtual void        Select();
    virtual void        Modify();
    virtual long        PreNotify( NotifyEvent& rNEvt );

public:
    SvxHyperURLBox( Window* pParent, INetProtocol eSmart = INET_PROT_FILE );

};

/// Tabpage : Basisclass
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

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        mxDocumentFrame;

protected:
    Window*             mpDialog;

    sal_Bool                mbStdControlsInit;

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

    DECL_LINK (ClickScriptHdl_Impl, void * ); ///< Button : Script

    String              aEmptyStr;

    static String   GetSchemeFromURL( String aStrURL );

    inline void     DisableClose( sal_Bool _bDisable ) { mbIsCloseDisabled = _bDisable; }

public:
    SvxHyperlinkTabPageBase (
        Window *pParent,
        const ResId &rResId,
        const SfxItemSet& rItemSet
    );
    virtual ~SvxHyperlinkTabPageBase ();

    void    SetDocumentFrame(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame )
    {
        mxDocumentFrame = rxDocumentFrame;
    }

    virtual sal_Bool AskApply ();
    virtual void DoApply ();
    virtual void SetOnlineMode( sal_Bool bEnable );
    virtual void SetInitFocus();
    virtual void SetMarkStr ( String& aStrMark );
    virtual void Reset( const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void ActivatePage( const SfxItemSet& rItemSet );
    virtual int  DeactivatePage( SfxItemSet* pSet = 0 );

    sal_Bool IsMarkWndVisible ()      { return ((Window*)mpMarkWnd)->IsVisible(); }
    Size GetSizeExtraWnd ()       { return ( mpMarkWnd->GetSizePixel() ); }
    sal_Bool MoveToExtraWnd ( Point aNewPos, sal_Bool bDisConnectDlg = sal_False );

    virtual void        ActivatePage();
    virtual void        DeactivatePage();
    virtual sal_Bool    QueryClose();

protected:
    virtual sal_Bool ShouldOpenMarkWnd();
    virtual void SetMarkWndShouldOpen(sal_Bool bOpen);

    void ShowMarkWnd ();
    void HideMarkWnd ()           { ( ( Window* ) mpMarkWnd )->Hide(); }
    void InvalidateMarkWnd ()     { ( ( Window* ) mpMarkWnd )->Invalidate(); }

    SfxDispatcher* GetDispatcher() const;

    sal_uInt16             GetMacroEvents();
    SvxMacroTableDtor* GetMacroTable();

    sal_Bool IsHTMLDoc() const;
};

#endif // _SVX_TABBASE_HYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
