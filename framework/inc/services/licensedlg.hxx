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
#ifndef __LICENSEDLG_HXX__
#define __LICENSEDLG_HXX__

#include <sal/types.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/string.hxx>
#include <tools/resmgr.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/scrbar.hxx>
#include <svtools/svmedit.hxx>
#include <svl/lstner.hxx>

namespace framework {

class LicenseView : public MultiLineEdit, public SfxListener
{
    sal_Bool            mbEndReached;
    Link            maEndReachedHdl;
    Link            maScrolledHdl;

public:
    LicenseView( Window* pParent, const ResId& rResId );
    ~LicenseView();

    void ScrollDown( ScrollType eScroll );

    sal_Bool IsEndReached() const;
    sal_Bool EndReached() const { return mbEndReached; }
    void SetEndReached( sal_Bool bEnd ) { mbEndReached = bEnd; }

    void SetEndReachedHdl( const Link& rHdl )  { maEndReachedHdl = rHdl; }
    const Link& GetAutocompleteHdl() const { return maEndReachedHdl; }

    void SetScrolledHdl( const Link& rHdl )  { maScrolledHdl = rHdl; }
    const Link& GetScrolledHdl() const { return maScrolledHdl; }

    using MultiLineEdit::Notify;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

class LicenseDialog : public ModalDialog
{
    LicenseView     aLicenseML;
    FixedText       aInfo1FT;
    FixedText       aInfo2FT;
    FixedText       aInfo3FT;
    FixedText       aInfo2_1FT;
    FixedText       aInfo3_1FT;
    FixedLine       aFixedLine;
    PushButton      aPBPageDown;
    PushButton      aPBDecline;
    PushButton      aPBAccept;
    FixedImage      aArrow;
    OUString        aStrAccept;
    OUString        aStrNotAccept;
    OUString        aOldCancelText;
    sal_Bool        bEndReached;

    void                EnableControls();

    DECL_LINK(PageDownHdl, void *);
    DECL_LINK(EndReachedHdl, void *);
    DECL_LINK(ScrolledHdl, void *);
    DECL_LINK(AcceptBtnHdl, void *);
    DECL_LINK(DeclineBtnHdl, void *);

    public:
        LicenseDialog(const OUString& aLicense, ResMgr *pResMgr);
        virtual ~LicenseDialog();

};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
