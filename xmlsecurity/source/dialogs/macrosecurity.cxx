/*************************************************************************
 *
 *  $RCSfile: macrosecurity.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: gt $ $Date: 2004-07-16 06:27:38 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <xmlsecurity/macrosecurity.hxx>
#include <xmlsecurity/certificateviewer.hxx>

#ifndef _COM_SUN_STAR_XML_CRYPTO_XSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

// MM : added for password exception
#include <vcl/msgbox.hxx>
#include <com/sun/star/security/NoPasswordException.hpp>
using namespace ::com::sun::star::security;

// Only for bigIntegerToNumericString
#include <xmlsecurity/xmlsignaturehelper.hxx>

#include "dialogs.hrc"
#include "resourcemanager.hxx"

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star;


MacroSecurity::MacroSecurity( Window* _pParent, cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment )
    :TabDialog      ( _pParent, XMLSEC_RES( RID_XMLSECTP_MACROSEC ) )
    ,maTabCtrl      ( this, ResId( 1 ) )
    ,maOkBtn        ( this, ResId( BTN_OK ) )
    ,maCancelBtn    ( this, ResId( BTN_CANCEL ) )
    ,maHelpBtn      ( this, ResId( BTN_HELP ) )
    ,maResetBtn     ( this, ResId( BTN_RESET ) )
{
    FreeResource();

    mxSecurityEnvironment = _rxSecurityEnvironment;

    maTabCtrl.SetTabPage( RID_XMLSECTP_SECLEVEL, new MacroSecurityLevelTP( &maTabCtrl, this ) );
    maTabCtrl.SetTabPage( RID_XMLSECTP_TRUSTSOURCES, new MacroSecurityTrustedSourcesTP( &maTabCtrl, this ) );
    maTabCtrl.SetCurPageId( RID_XMLSECTP_SECLEVEL );
}

MacroSecurity::~MacroSecurity()
{
}


MacroSecurityTP::MacroSecurityTP( Window* _pParent, const ResId& _rResId, MacroSecurity* _pDlg )
    :TabPage        ( _pParent, _rResId )
    ,mpDlg          ( _pDlg )
{
}


MacroSecurityLevelTP::MacroSecurityLevelTP( Window* _pParent, MacroSecurity* _pDlg )
    :MacroSecurityTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_SECLEVEL ), _pDlg )
    ,maSecLevelFL       ( this, ResId( FL_SECLEVEL ) )
    ,maVeryHighRB       ( this, ResId( RB_VERYHIGH ) )
    ,maHighRB           ( this, ResId( RB_HIGH ) )
    ,maMediumRB         ( this, ResId( RB_MEDIUM ) )
    ,maLowRB            ( this, ResId( RB_LOW ) )
{
    FreeResource();
}

void MacroSecurityLevelTP::ActivatePage()
{
}


IMPL_LINK( MacroSecurityTrustedSourcesTP, AddCertPBHdl, void*, EMTYARG )
{
    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, ViewCertPBHdl, void*, EMTYARG )
{
    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, RemoveCertPBHdl, void*, EMTYARG )
{
    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, AddLocPBHdl, void*, EMTYARG )
{
    return 0;
}

IMPL_LINK( MacroSecurityTrustedSourcesTP, RemoveLocPBHdl, void*, EMTYARG )
{
    return 0;
}

MacroSecurityTrustedSourcesTP::MacroSecurityTrustedSourcesTP( Window* _pParent, MacroSecurity* _pDlg )
    :MacroSecurityTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_TRUSTSOURCES ), _pDlg )
    ,maTrustCertFL      ( this, ResId( FL_TRUSTCERT ) )
    ,maTrustCertLB      ( this, ResId( LB_TRUSTCERT ) )
    ,maAddCertPB        ( this, ResId( PB_ADD_TRUSTCERT ) )
    ,maViewCertPB       ( this, ResId( PB_VIEW_TRUSTCERT ) )
    ,maRemoveCertPB     ( this, ResId( PB_REMOVE_TRUSTCERT ) )
    ,maTrustFileLocFL   ( this, ResId( FL_TRUSTFILELOC ) )
    ,maTrustFileLocFI   ( this, ResId( FI_TRUSTFILELOC ) )
    ,maTrustFileLocLB   ( this, ResId( LB_TRUSTFILELOC ) )
    ,maAddLocPB         ( this, ResId( FL_ADD_TRUSTFILELOC ) )
    ,maRemoveLocPB      ( this, ResId( FL_REMOVE_TRUSTFILELOC ) )
{
    static long nTabs[] = { 3, 0, 35*CS_LB_WIDTH/100, 70*CS_LB_WIDTH/100 };
    maTrustCertLB.SetTabs( &nTabs[ 0 ] );
    maTrustCertLB.InsertHeaderEntry( String( ResId( STR_HEADERBAR ) ) );

    FreeResource();
}

void MacroSecurityTrustedSourcesTP::ActivatePage()
{
}
