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

#ifndef INCLUDED_CUI_SOURCE_INC_LINKDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_LINKDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/idle.hxx>

#include <svtools/svmedit.hxx>
#include <svtools/svtabbx.hxx>

/********************** SvUpdateLinksDialog ******************************
*************************************************************************/
namespace sfx2
{
    class LinkManager;
    class SvBaseLink;
}

enum class SfxLinkUpdateMode;

class SvBaseLinksDlg : public ModalDialog
{
    using Window::SetType;

    VclPtr<SvTabListBox> m_pTbLinks;
    VclPtr<FixedText> m_pFtFullFileName;
    VclPtr<FixedText> m_pFtFullSourceName;
    VclPtr<FixedText> m_pFtFullTypeName;
    VclPtr<RadioButton> m_pRbAutomatic;
    VclPtr<RadioButton> m_pRbManual;
    VclPtr<PushButton> m_pPbUpdateNow;
    VclPtr<PushButton> m_pPbOpenSource;
    VclPtr<PushButton> m_pPbChangeSource;
    VclPtr<PushButton> m_pPbBreakLink;
    OUString aStrAutolink;
    OUString aStrManuallink;
    OUString aStrBrokenlink;
    OUString aStrCloselinkmsg;
    OUString aStrCloselinkmsgMulti;
    OUString aStrWaitinglink;
    sfx2::LinkManager*  pLinkMgr;
    bool            bHtmlMode;
    Idle aUpdateIdle;

    DECL_LINK_TYPED( LinksSelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( LinksDoubleClickHdl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( AutomaticClickHdl, Button *, void );
    DECL_LINK_TYPED( ManualClickHdl, Button *, void );
    DECL_LINK_TYPED( UpdateNowClickHdl, Button *, void);
    DECL_LINK_TYPED( ChangeSourceClickHdl, Button *, void );
    DECL_LINK_TYPED( BreakLinkClickHdl, Button *, void );
    DECL_LINK_TYPED( UpdateWaitingHdl, Idle *, void );
    DECL_LINK_TYPED( EndEditHdl, sfx2::SvBaseLink&, void );
    sfx2::SvBaseLink* GetSelEntry( sal_uLong* pPos );
    OUString ImplGetStateStr( const sfx2::SvBaseLink& );
    void SetType( sfx2::SvBaseLink& rLink, sal_uLong nPos, SfxLinkUpdateMode nType );
    void InsertEntry( const sfx2::SvBaseLink& rLink, sal_uLong nPos = TREELIST_APPEND, bool bSelect = false);

    void StartUpdateTimer()         { aUpdateIdle.Start(); }

    OUString&       Autolink()      { return aStrAutolink; }
    OUString&       Manuallink()    { return aStrManuallink; }
    OUString&       Brokenlink()    { return aStrBrokenlink; }
    OUString&       Closelinkmsg()  { return aStrCloselinkmsg; }
    OUString&       CloselinkmsgMulti() { return aStrCloselinkmsgMulti; }
    OUString&       Waitinglink()   { return aStrWaitinglink; }
    void SetManager( sfx2::LinkManager* );

public:
    SvBaseLinksDlg( vcl::Window * pParent, sfx2::LinkManager*, bool bHtml = false );
    virtual ~SvBaseLinksDlg();
    virtual void dispose() SAL_OVERRIDE;
    void SetActLink( sfx2::SvBaseLink * pLink );
};

#endif // INCLUDED_CUI_SOURCE_INC_LINKDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
