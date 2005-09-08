/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: passwd.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:17:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_PASSWD_HXX
#define _SVX_PASSWD_HXX

// include ---------------------------------------------------------------

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxPasswordDialog -----------------------------------------------

class SVX_DLLPUBLIC SvxPasswordDialog : public SfxModalDialog
{
private:
    FixedLine       aOldFL;
    FixedText       aOldPasswdFT;
    Edit            aOldPasswdED;
    FixedLine       aNewFL;
    FixedText       aNewPasswdFT;
    Edit            aNewPasswdED;
    FixedText       aRepeatPasswdFT;
    Edit            aRepeatPasswdED;
    OKButton        aOKBtn;
    CancelButton    aEscBtn;
    HelpButton      aHelpBtn;

    String          aOldPasswdErrStr;
    String          aRepeatPasswdErrStr;

    Link            aCheckPasswordHdl;

    BOOL            bEmpty;

    DECL_LINK( ButtonHdl, OKButton * );
    DECL_LINK( EditModifyHdl, Edit * );

public:
                    SvxPasswordDialog( Window* pParent, BOOL bAllowEmptyPasswords = FALSE, BOOL bDisableOldPassword = FALSE );
                    ~SvxPasswordDialog();

    String          GetOldPassword() const { return aOldPasswdED.GetText(); }
    String          GetNewPassword() const { return aNewPasswdED.GetText(); }

    void            SetCheckPasswordHdl( const Link& rLink ) { aCheckPasswordHdl = rLink; }
};


#endif

