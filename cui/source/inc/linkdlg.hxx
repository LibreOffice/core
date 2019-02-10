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

#include <vcl/idle.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>

/********************** SvUpdateLinksDialog ******************************
*************************************************************************/
namespace sfx2
{
    class LinkManager;
    class SvBaseLink;
}

enum class SfxLinkUpdateMode;

class SvBaseLinksDlg : public weld::GenericDialogController
{
    OUString aStrAutolink;
    OUString aStrManuallink;
    OUString aStrBrokenlink;
    OUString aStrCloselinkmsg;
    OUString aStrCloselinkmsgMulti;
    OUString aStrWaitinglink;
    sfx2::LinkManager*  pLinkMgr;
    Idle aUpdateIdle;

    std::unique_ptr<weld::TreeView> m_xTbLinks;
    std::unique_ptr<weld::LinkButton> m_xFtFullFileName;
    std::unique_ptr<weld::Label> m_xFtFullSourceName;
    std::unique_ptr<weld::Label> m_xFtFullTypeName;
    std::unique_ptr<weld::RadioButton> m_xRbAutomatic;
    std::unique_ptr<weld::RadioButton> m_xRbManual;
    std::unique_ptr<weld::Button> m_xPbUpdateNow;
    std::unique_ptr<weld::Button> m_xPbChangeSource;
    std::unique_ptr<weld::Button> m_xPbBreakLink;

    ScopedVclPtr<VirtualDevice> m_xVirDev;

    DECL_LINK( LinksSelectHdl, weld::TreeView&, void );
    DECL_LINK( LinksDoubleClickHdl, weld::TreeView&, void );
    DECL_LINK( AutomaticClickHdl, weld::Button&, void );
    DECL_LINK( ManualClickHdl, weld::Button&, void );
    DECL_LINK( UpdateNowClickHdl, weld::Button&, void);
    DECL_LINK( ChangeSourceClickHdl, weld::Button&, void );
    DECL_LINK( BreakLinkClickHdl, weld::Button&, void );
    DECL_LINK( UpdateWaitingHdl, Timer *, void );
    DECL_LINK( EndEditHdl, sfx2::SvBaseLink&, void );
    void LinksSelectHdl(weld::TreeView* pTreeView);
    sfx2::SvBaseLink* GetSelEntry(int* pPos);
    OUString ImplGetStateStr( const sfx2::SvBaseLink& );
    void SetType(sfx2::SvBaseLink& rLink, int nPos, SfxLinkUpdateMode nType);
    void InsertEntry(const sfx2::SvBaseLink& rLink, int nPos = -1, bool bSelect = false);

    void SetManager( sfx2::LinkManager* );

public:
    SvBaseLinksDlg(weld::Window * pParent, sfx2::LinkManager*, bool bHtml);
    virtual ~SvBaseLinksDlg() override;
    void SetActLink( sfx2::SvBaseLink const * pLink );
};

#endif // INCLUDED_CUI_SOURCE_INC_LINKDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
