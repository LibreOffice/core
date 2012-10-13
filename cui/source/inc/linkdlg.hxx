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

#ifndef _LINKDLG_HXX
#define _LINKDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>

#include <svtools/svmedit.hxx>  // MultiLineEdit
#include <svtools/svtabbx.hxx>  // MultiLineEdit

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
    String aStrAutolink;
    String aStrManuallink;
    String aStrBrokenlink;
    String aStrGraphiclink;
    String aStrButtonclose;
    String aStrCloselinkmsg;
    String aStrCloselinkmsgMulti;
    String aStrWaitinglink;
    sfx2::LinkManager*  pLinkMgr;
    sal_Bool            bHtmlMode;
    Timer aUpdateTimer;

#if _SOLAR__PRIVATE
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
    sfx2::SvBaseLink* GetSelEntry( sal_uInt16* pPos );
    String ImplGetStateStr( const sfx2::SvBaseLink& );
    void SetType( sfx2::SvBaseLink& rLink, sal_uInt16 nPos, sal_uInt16 nType );
    void InsertEntry( const sfx2::SvBaseLink& rLink, sal_uInt16 nPos = LISTBOX_APPEND, sal_Bool bSelect = sal_False);
#endif

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

    String&         Autolink()      { return aStrAutolink; }
    String&         Manuallink()    { return aStrManuallink; }
    String&         Brokenlink()    { return aStrBrokenlink; }
    String&         Graphiclink()   { return aStrGraphiclink; }
    String&         Buttonclose()   { return aStrButtonclose; }
    String&         Closelinkmsg()  { return aStrCloselinkmsg; }
    String&         CloselinkmsgMulti() { return aStrCloselinkmsgMulti; }
    String&         Waitinglink()   { return aStrWaitinglink; }
    void SetManager( sfx2::LinkManager* );

public:
    SvBaseLinksDlg( Window * pParent, sfx2::LinkManager*, sal_Bool bHtml = sal_False );
    ~SvBaseLinksDlg();
    void SetActLink( sfx2::SvBaseLink * pLink );
};

#endif // _LINKDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
