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

#include <svtools/svmedit.hxx>
#include <svtools/svtabbx.hxx>

/********************** SvUpdateLinksDialog ******************************
*************************************************************************/
namespace sfx2
{
    class LinkManager;
    class SvBaseLink;
}

class SvBaseLinksDlg : public ModalDialog
{
    using Window::SetType;

    SvTabListBox *m_pTbLinks;
    FixedText *m_pFtFullFileName;
    FixedText *m_pFtFullSourceName;
    FixedText *m_pFtFullTypeName;
    RadioButton *m_pRbAutomatic;
    RadioButton *m_pRbManual;
    PushButton *m_pPbUpdateNow;
    PushButton *m_pPbOpenSource;
    PushButton *m_pPbChangeSource;
    PushButton *m_pPbBreakLink;
    OUString aStrAutolink;
    OUString aStrManuallink;
    OUString aStrBrokenlink;
    OUString aStrGraphiclink;
    OUString aStrButtonclose;
    OUString aStrCloselinkmsg;
    OUString aStrCloselinkmsgMulti;
    OUString aStrWaitinglink;
    sfx2::LinkManager*  pLinkMgr;
    bool            bHtmlMode;
    Timer aUpdateTimer;

    DECL_LINK( LinksSelectHdl, SvTabListBox * );
    DECL_LINK( LinksDoubleClickHdl, SvTabListBox * );
    DECL_LINK( AutomaticClickHdl, RadioButton * );
    DECL_LINK( ManualClickHdl, RadioButton * );
    DECL_LINK(UpdateNowClickHdl, void *);
    DECL_LINK( OpenSourceClickHdl, PushButton * );
    DECL_LINK( ChangeSourceClickHdl, PushButton * );
    DECL_LINK( BreakLinkClickHdl, PushButton * );
    DECL_LINK( UpdateWaitingHdl, Timer * );
    DECL_LINK( EndEditHdl, sfx2::SvBaseLink* );
    sfx2::SvBaseLink* GetSelEntry( sal_uLong* pPos );
    OUString ImplGetStateStr( const sfx2::SvBaseLink& );
    void SetType( sfx2::SvBaseLink& rLink, sal_uLong nPos, sal_uInt16 nType );
    void InsertEntry( const sfx2::SvBaseLink& rLink, sal_uLong nPos = TREELIST_APPEND, bool bSelect = false);

    void StartUpdateTimer()         { aUpdateTimer.Start(); }

    OUString&       Autolink()      { return aStrAutolink; }
    OUString&       Manuallink()    { return aStrManuallink; }
    OUString&       Brokenlink()    { return aStrBrokenlink; }
    OUString&       Graphiclink()   { return aStrGraphiclink; }
    OUString&       Buttonclose()   { return aStrButtonclose; }
    OUString&       Closelinkmsg()  { return aStrCloselinkmsg; }
    OUString&       CloselinkmsgMulti() { return aStrCloselinkmsgMulti; }
    OUString&       Waitinglink()   { return aStrWaitinglink; }
    void SetManager( sfx2::LinkManager* );

public:
    SvBaseLinksDlg( Window * pParent, sfx2::LinkManager*, bool bHtml = false );
    virtual ~SvBaseLinksDlg();
    void SetActLink( sfx2::SvBaseLink * pLink );
};

#endif // INCLUDED_CUI_SOURCE_INC_LINKDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
