/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include <sfx2/tabdlg.hxx>
#include <vcl/transfer.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/inettbc.hxx>
#include <vcl/timer.hxx>
#include <vcl/waitobj.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <svx/hlnkitem.hxx>

#include "hlmarkwn.hxx"
#include "iconcdlg.hxx"

/// ComboBox-Control for URL's with History and Autocompletion
class SvxHyperURLBox : public SvtURLBox, public DropTargetHelper
{
protected:
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    SvxHyperURLBox(std::unique_ptr<weld::ComboBox> xWidget);
};

/// Tabpage : Basisclass
class SvxHyperlinkTabPageBase : public IconChoicePage
{
private:
    std::unique_ptr<weld::ComboBox> mxCbbFrame;
    std::unique_ptr<weld::ComboBox> mxLbForm;
    std::unique_ptr<weld::Entry> mxEdIndication;
    std::unique_ptr<weld::Entry> mxEdText;
    std::unique_ptr<weld::Button> mxBtScript;
    std::unique_ptr<weld::Label> mxFormLabel;
    std::unique_ptr<weld::Label> mxFrameLabel;

    bool                        mbIsCloseDisabled;

    css::uno::Reference< css::frame::XFrame >
                                mxDocumentFrame;

protected:
    SvxHpLinkDlg* mpDialog;

    bool                mbStdControlsInit;

    OUString            maStrInitURL;

    Timer               maTimer;

    TopLevelWindowLocker maBusy;

    std::shared_ptr<SvxHlinkDlgMarkWnd> mxMarkWnd;

    void InitStdControls ();
    void FillStandardDlgFields ( const SvxHyperlinkItem* pHyperlinkItem );
    virtual void FillDlgFields(const OUString& rStrURL) = 0;
    virtual void GetCurentItemData     ( OUString& rStrURL, OUString& aStrName,
                                         OUString& aStrIntName, OUString& aStrFrame,
                                         SvxLinkInsertMode& eMode ) = 0;

    void         GetDataFromCommonFields( OUString& aStrName,
                                          OUString& aStrIntName, OUString& aStrFrame,
                                          SvxLinkInsertMode& eMode );

    DECL_LINK (ClickScriptHdl_Impl, weld::Button&, void ); ///< Button : Script

    static OUString GetSchemeFromURL( const OUString& rStrURL );

    void     DisableClose( bool _bDisable );

public:
    SvxHyperlinkTabPageBase (
        weld::Container* pParent,
        SvxHpLinkDlg* pDlg,
        const OUString& rUIXMLDescription,
        const OString& rID,
        const SfxItemSet* pItemSet
    );
    virtual ~SvxHyperlinkTabPageBase () override;

    void    SetDocumentFrame(
        const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame )
    {
        mxDocumentFrame = rxDocumentFrame;
    }

    virtual bool AskApply ();
    virtual void DoApply ();
    virtual void SetInitFocus();
    virtual void SetMarkStr ( const OUString& aStrMark );
    virtual void Reset( const SfxItemSet& ) override;
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void ActivatePage( const SfxItemSet& rItemSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    bool IsMarkWndVisible() const { return static_cast<bool>(mxMarkWnd); }
    void MoveToExtraWnd ( Point aNewPos );

    virtual bool        QueryClose() override;

protected:
    virtual bool ShouldOpenMarkWnd();
    virtual void SetMarkWndShouldOpen(bool bOpen);

    void ShowMarkWnd();
    void HideMarkWnd();

    SfxDispatcher* GetDispatcher() const;

    HyperDialogEvent   GetMacroEvents() const;
    SvxMacroTableDtor* GetMacroTable();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
