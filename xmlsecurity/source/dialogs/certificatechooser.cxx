/*************************************************************************
 *
 *  $RCSfile: certificatechooser.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-22 15:37:37 $
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

#include <xmlsecurity/certificatechooser.hxx>
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
#include <com/sun/star/security/CertificateCharacters.hpp>
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

#define INVAL_SEL       0xFFFF

USHORT CertificateChooser::GetSelectedEntryPos( void ) const
{
    USHORT  nSel = INVAL_SEL;

    SvLBoxEntry* pSel = maCertLB.FirstSelected();
    if( pSel )
        nSel = ( sal_Int32 ) pSel->GetUserData();

    return (USHORT) nSel;
}

CertificateChooser::CertificateChooser( Window* _pParent, uno::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment, const SignatureInformations& _rCertsToIgnore )
    :ModalDialog    ( _pParent, XMLSEC_RES( RID_XMLSECDLG_CERTCHOOSER ) )
    ,maHintFT       ( this, ResId( FT_HINT_SELECT ) )
    ,maCertLB       ( this, ResId( LB_SIGNATURES ) )
    ,maViewBtn      ( this, ResId( BTN_VIEWCERT ) )
    ,maBottomSepFL  ( this, ResId( FL_BOTTOM_SEP ) )
    ,maOKBtn        ( this, ResId( BTN_OK ) )
    ,maCancelBtn    ( this, ResId( BTN_CANCEL ) )
    ,maHelpBtn      ( this, ResId( BTN_HELP ) )
{
    static long nTabs[] = { 3, 0, 30*CS_LB_WIDTH/100, 60*CS_LB_WIDTH/100 };
    maCertLB.SetTabs( &nTabs[0] );
    maCertLB.InsertHeaderEntry( String( ResId( STR_HEADERBAR ) ) );

    FreeResource();

    mxSecurityEnvironment = _rxSecurityEnvironment;

    maCertLB.SetSelectHdl( LINK( this, CertificateChooser, CertificateHighlightHdl ) );
    maCertLB.SetDoubleClickHdl( LINK( this, CertificateChooser, CertificateSelectHdl ) );

    try
    {
        maCerts = mxSecurityEnvironment->getPersonalCertificates();
    }
    catch (NoPasswordException&)
    {
    }

    sal_Int32 nCertificates = maCerts.getLength();
    sal_Int32 nCertificatesToIgnore = _rCertsToIgnore.size();
    for( sal_Int32 nCert = nCertificates; nCert; )
    {
        uno::Reference< security::XCertificate > xCert = maCerts[ --nCert ];
        sal_Bool bIgnoreThis = false;

        // Do we already use that?
        if( nCertificatesToIgnore )
        {
            rtl::OUString aIssuerName = xCert->getIssuerName();
            for( sal_Int32 nSig = 0; nSig < nCertificatesToIgnore; ++nSig )
            {
                const SignatureInformation& rInf = _rCertsToIgnore[ nSig ];
                if( ( aIssuerName == rInf.ouX509IssuerName ) && ( bigIntegerToNumericString( xCert->getSerialNumber() ) == rInf.ouX509SerialNumber ) )
                {
                    bIgnoreThis = true;
                    break;
                }
            }
        }

        if ( !bIgnoreThis )
        {
            // Check if we have a private key for this...
            long nCertificateCharacters = mxSecurityEnvironment->getCertificateCharacters( xCert );

            if ( !( nCertificateCharacters & security::CertificateCharacters::CERT_CHARACTER_HAS_PRIVATE_KEY ) )
                bIgnoreThis = true;

        }

        if ( bIgnoreThis )
        {
            ::comphelper::removeElementAt( maCerts, nCert );
            nCertificates = maCerts.getLength();
        }
    }

    String  aCN_Id( String::CreateFromAscii( "CN" ) );

    for( sal_Int32 nC = 0; nC < nCertificates; ++nC )
    {
        String          aSubject    ( XmlSec::GetContentPart( maCerts[ nC ]->getSubjectName(), aCN_Id ) );
        String          aDateTimeStr( XmlSec::GetDateString( maCerts[ nC ]->getNotAfter() ) );
        String          aIssuer     ( XmlSec::GetContentPart( maCerts[ nC ]->getIssuerName(), aCN_Id ) );

        SvLBoxEntry*    pEntry = maCertLB.InsertEntry( aSubject );
        maCertLB.SetEntryText( aIssuer, pEntry, 1 );
        maCertLB.SetEntryText( aDateTimeStr, pEntry, 2 );
        pEntry->SetUserData( ( void* ) nC );        // missuse user data as index
    }

    maViewBtn.SetClickHdl( LINK( this, CertificateChooser, ViewButtonHdl ) );
    maViewBtn.Disable();
}

CertificateChooser::~CertificateChooser()
{
}

uno::Reference< dcss::security::XCertificate > CertificateChooser::GetSelectedCertificate()
{
    uno::Reference< dcss::security::XCertificate > xCert;
    USHORT  nSelected = GetSelectedEntryPos();
    if( nSelected < maCerts.getLength() )
        xCert = maCerts[ nSelected ];

    return xCert;
}

IMPL_LINK( CertificateChooser, CertificateHighlightHdl, void*, EMPTYARG )
{
    maViewBtn.Enable();
    return 0;
}

IMPL_LINK( CertificateChooser, CertificateSelectHdl, void*, EMPTYARG )
{
    EndDialog( 1 );
    return 0;
}

IMPL_LINK( CertificateChooser, ViewButtonHdl, Button*, EMPTYARG )
{
    ImplShowCertificateDetails();
    return 0;
}

void CertificateChooser::ImplShowCertificateDetails()
{
    uno::Reference< dcss::security::XCertificate > xCert = GetSelectedCertificate();
    if( xCert.is() )
    {
        CertificateViewer aViewer( this, mxSecurityEnvironment, xCert );
        aViewer.Execute();
    }
}