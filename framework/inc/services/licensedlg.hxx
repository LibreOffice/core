/*************************************************************************
 *
 *  $RCSfile: licensedlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 13:19:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef __LICENSEDLG_HXX__
#define __LICENSEDLG_HXX__

#include <sal/types.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/string.hxx>
#include <tools/lang.hxx>
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