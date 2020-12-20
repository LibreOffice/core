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

#include <sal/config.h>

#include <memory>
#include <string_view>

#include <svx/hlnkitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>

#include "iconcdlg.hxx"

/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg;
class SvxHlinkCtrl : public SfxControllerItem
{
private:
    SvxHpLinkDlg* pParent;

    SfxStatusForwarder aRdOnlyForwarder;

public:
    SvxHlinkCtrl( sal_uInt16 nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg);
    virtual void dispose() override;

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                const SfxPoolItem* pState ) override;
};


/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class SvxHpLinkDlg : public SfxModelessDialogController
{
private:
    friend class IconChoicePage;

    std::vector< std::unique_ptr<IconChoicePageData> > maPageList;

    OString msCurrentPageId;

    const SfxItemSet*       pSet;
    std::unique_ptr<SfxItemSet>     pOutSet;
    SfxItemSet*             pExampleSet;
    std::unique_ptr<sal_uInt16[]>   pRanges;

    SvxHlinkCtrl        maCtrl;         ///< Controller
    std::unique_ptr<SfxItemSet> mpItemSet;

    bool            mbGrabFocus : 1;
    bool            mbIsHTMLDoc : 1;

    std::unique_ptr<weld::Notebook> m_xIconCtrl;
    std::unique_ptr<weld::Button> m_xOKBtn;
    std::unique_ptr<weld::Button> m_xApplyBtn;
    std::unique_ptr<weld::Button> m_xCancelBtn;
    std::unique_ptr<weld::Button> m_xHelpBtn;
    std::unique_ptr<weld::Button> m_xResetBtn;

    DECL_LINK( ChosePageHdl_Impl, const OString&, void );

    IconChoicePageData*     GetPageData ( std::string_view rId );

    void                    SwitchPage( const OString& rId );

    DECL_LINK( ResetHdl, weld::Button&, void) ;
    DECL_LINK (ClickOkHdl_Impl, weld::Button&, void );
    DECL_LINK (ClickApplyHdl_Impl, weld::Button&, void );

    IconChoicePage*         GetTabPage( std::string_view rPageId )
                                { return GetPageData(rPageId)->xPage.get(); }

    void                    ActivatePageImpl ();
    void                    DeActivatePageImpl ();
    void                    ResetPageImpl ();

    void Activate() override;
    virtual void Close() override;
    void Apply();

public:
    SvxHpLinkDlg(SfxBindings* pBindings, SfxChildWindow* pChild, weld::Window* pParent);
    virtual ~SvxHpLinkDlg () override;

    // interface
    void AddTabPage(const OString &rId, CreatePage pCreateFunc /* != NULL */);

    void                SetCurPageId( const OString& rId ) { msCurrentPageId = rId; SwitchPage(rId ); }
    OString             GetCurPageId() const       { return msCurrentPageId; }
    void                ShowPage( const OString& rId );

    /// gives via map converted local slots if applicable
    const sal_uInt16*   GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );

    void                Start();
    bool                QueryClose();

    void                PageCreated(IconChoicePage& rPage);

    void                SetPage( SvxHyperlinkItem const * pItem );
    void                SetReadOnlyMode( bool bReadOnly );
    bool                IsHTMLDoc() const { return mbIsHTMLDoc; }

    SfxDispatcher*   GetDispatcher() const { return GetBindings().GetDispatcher(); }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
