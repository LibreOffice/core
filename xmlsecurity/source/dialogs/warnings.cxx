/*************************************************************************
 *
 *  $RCSfile: warnings.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-26 07:29:32 $
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

#include <xmlsecurity/warnings.hxx>
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


#include "dialogs.hrc"
#include "resourcemanager.hxx"

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star;


MacroWarning::MacroWarning( Window* _pParent, uno::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment, cssu::Reference< dcss::security::XCertificate >& _rxCert )
    :ModalDialog        ( _pParent, XMLSEC_RES( RID_XMLSECTP_MACROWARN ) )
    ,maDocNameFI        ( this, ResId( FI_DOCNAME ) )
    ,maDescr1aFI        ( this, ResId( FI_DESCR1A ) )
    ,maDescr1bFI        ( this, ResId( FI_DESCR1B ) )
    ,maSignsFI          ( this, ResId( FI_SIGNS ) )
    ,maViewSignsBtn     ( this, ResId( PB_VIEWSIGNS ) )
    ,maDescr2FI         ( this, ResId( FI_DESCR2 ) )
    ,maAlwaysTrustCB    ( this, ResId( CB_ALWAYSTRUST ) )
    ,maBottomSepFL      ( this, ResId( FL_BOTTOM_SEP ) )
    ,maEnableBtn        ( this, ResId( PB_DISABLE ) )
    ,maDisableBtn       ( this, ResId( PB_DISABLE ) )
    ,maHelpBtn          ( this, ResId( BTN_HELP ) )
    ,mbSignedMode       ( true )
{
    FreeResource();

    mxSecurityEnvironment = _rxSecurityEnvironment;
    mxCert = _rxCert;

    // hide unused parts
    maDescr1bFI.Hide();

    maViewSignsBtn.SetClickHdl( LINK( this, MacroWarning, ViewSignsBtnHdl ) );
    maEnableBtn.SetClickHdl( LINK( this, MacroWarning, EnableBtnHdl ) );
//  maDisableBtn.SetClickHdl( LINK( this, MacroWarning, DisableBtnHdl ) );

    if( mxCert.is() )
    {
        String  aCN_Id( String::CreateFromAscii( "CN" ) );
        String  s;
        s = XmlSec::GetContentPart( mxCert->getSubjectName(), aCN_Id );

        maSignsFI.SetText( s );
    }
    else
        // nothing to view!
        maViewSignsBtn.Disable();
}

MacroWarning::MacroWarning( Window* _pParent )
    :ModalDialog        ( _pParent, XMLSEC_RES( RID_XMLSECTP_MACROWARN ) )
    ,maDocNameFI        ( this, ResId( FI_DOCNAME ) )
    ,maDescr1aFI        ( this, ResId( FI_DESCR1A ) )
    ,maDescr1bFI        ( this, ResId( FI_DESCR1B ) )
    ,maSignsFI          ( this, ResId( FI_SIGNS ) )
    ,maViewSignsBtn     ( this, ResId( PB_VIEWSIGNS ) )
    ,maDescr2FI         ( this, ResId( FI_DESCR2 ) )
    ,maAlwaysTrustCB    ( this, ResId( CB_ALWAYSTRUST ) )
    ,maBottomSepFL      ( this, ResId( FL_BOTTOM_SEP ) )
    ,maEnableBtn        ( this, ResId( PB_DISABLE ) )
    ,maDisableBtn       ( this, ResId( PB_DISABLE ) )
    ,maHelpBtn          ( this, ResId( BTN_HELP ) )
    ,mbSignedMode       ( false )
{
    FreeResource();

    // hide unused parts
    maDescr1aFI.Hide();
    maSignsFI.Hide();
    maViewSignsBtn.Hide();
    maAlwaysTrustCB.Hide();
    maDescr2FI.Hide();

    // move hint up to position of signer list
    maDescr1bFI.SetPosPixel( maSignsFI.GetPosPixel() );
}

MacroWarning::~MacroWarning()
{
}

IMPL_LINK( MacroWarning, ViewSignsBtnHdl, void*, EMPTYARG )
{
    DBG_ASSERT( mxCert.is(), "*MacroWarning::ViewSignsBtnHdl(): no certificate set!" );

    CertificateViewer   aViewer( this, mxSecurityEnvironment, mxCert );
    aViewer.Execute();

    return 0;
}

IMPL_LINK( MacroWarning, EnableBtnHdl, void*, EMPTYARG )
{
    if( mbSignedMode && maAlwaysTrustCB.IsChecked() )
    {   // insert path into trusted path list

    }

    EndDialog( RET_OK );
    return 0;
}

/*IMPL_LINK( MacroWarning, DisableBtnHdl, void*, EMPTYARG )
{
    return 0;
}*/

