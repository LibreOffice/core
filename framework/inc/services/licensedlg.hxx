/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: licensedlg.hxx,v $
 * $Revision: 1.6 $
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
#include <svtools/lstner.hxx>

namespace framework {

class LicenseView : public MultiLineEdit, public SfxListener
{
    BOOL            mbEndReached;
    Link            maEndReachedHdl;
    Link            maScrolledHdl;

public:
    LicenseView( Window* pParent, const ResId& rResId );
    ~LicenseView();

    void ScrollDown( ScrollType eScroll );

    BOOL IsEndReached() const;
    BOOL EndReached() const { return mbEndReached; }
    void SetEndReached( BOOL bEnd ) { mbEndReached = bEnd; }

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
    String          aStrAccept;
    String          aStrNotAccept;
    String          aOldCancelText;
    BOOL            bEndReached;

    void                EnableControls();

    DECL_LINK(          PageDownHdl, PushButton * );
    DECL_LINK(          EndReachedHdl, LicenseView * );
    DECL_LINK(          ScrolledHdl, LicenseView * );
    DECL_LINK(          AcceptBtnHdl, PushButton * );
    DECL_LINK(          DeclineBtnHdl, PushButton * );

    public:
        LicenseDialog(const rtl::OUString& aLicense, ResMgr *pResMgr);
        virtual ~LicenseDialog();

};
}
#endif
