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

    FixedText aFtFiles;
    FixedText aFtLinks;
    FixedText aFtType;
    FixedText aFtStatus;
    SvTabListBox aTbLinks;
    FixedText aFtFiles2;
    FixedText aFtFullFileName;
    FixedText aFtSource2;
    FixedText aFtFullSourceName;
    FixedText aFtType2;
    FixedText aFtFullTypeName;
    FixedText aFtUpdate;
    RadioButton aRbAutomatic;
    RadioButton aRbManual;
    CancelButton aCancelButton1;
    HelpButton aHelpButton1;
    PushButton aPbUpdateNow;
    PushButton aPbOpenSource;
    PushButton aPbChangeSource;
    PushButton aPbBreakLink;
    OUString aStrAutolink;
    OUString aStrManuallink;
    OUString aStrBrokenlink;
    OUString aStrGraphiclink;
    OUString aStrButtonclose;
    OUString aStrCloselinkmsg;
    OUString aStrCloselinkmsgMulti;
    OUString aStrWaitinglink;
    sfx2::LinkManager*  pLinkMgr;
    sal_Bool            bHtmlMode;
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
    void InsertEntry( const sfx2::SvBaseLink& rLink, sal_uLong nPos = TREELIST_APPEND, sal_Bool bSelect = sal_False);

    void StartUpdateTimer()         { aUpdateTimer.Start(); }

    SvTabListBox&     Links()       { return aTbLinks; }
    FixedText&      FileName()      { return aFtFullFileName; }
    FixedText&      SourceName()    { return aFtFullSourceName; }
    FixedText&      TypeName()      { return aFtFullTypeName; }
    RadioButton&    Automatic()     { return aRbAutomatic; }
    RadioButton&    Manual()        { return aRbManual; }
    PushButton&     UpdateNow()     { return aPbUpdateNow; }
    PushButton&     OpenSource()    { return aPbOpenSource; }
    PushButton&     ChangeSource()  { return aPbChangeSource; }
    PushButton&     BreakLink()     { return aPbBreakLink; }

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
    SvBaseLinksDlg( Window * pParent, sfx2::LinkManager*, sal_Bool bHtml = sal_False );
    ~SvBaseLinksDlg();
    void SetActLink( sfx2::SvBaseLink * pLink );
};

#endif // INCLUDED_CUI_SOURCE_INC_LINKDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
