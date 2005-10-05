/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: certificatechooser.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 14:56:55 $
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

#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/biginteger.hxx>

#ifndef _COM_SUN_STAR_XML_CRYPTO_XSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>

#include <dialogs.hrc>
#include <resourcemanager.hxx>

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;

#define INVAL_SEL       0xFFFF

USHORT CertificateChooser::GetSelectedEntryPos( void ) const
{
    USHORT  nSel = INVAL_SEL;

    SvLBoxEntry* pSel = maCertLB.FirstSelected();
    if( pSel )
        nSel = (USHORT) ( sal_uIntPtr ) pSel->GetUserData();

    return (USHORT) nSel;
}

CertificateChooser::CertificateChooser( Window* _pParent, uno::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment, const SignatureInformations& _rCertsToIgnore )
    :ModalDialog    ( _pParent, XMLSEC_RES( RID_XMLSECDLG_CERTCHOOSER ) )
    ,maCertsToIgnore( _rCertsToIgnore )
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
    maCertLB.SetSelectHdl( LINK( this, CertificateChooser, CertificateHighlightHdl ) );
    maCertLB.SetDoubleClickHdl( LINK( this, CertificateChooser, CertificateSelectHdl ) );
    maViewBtn.SetClickHdl( LINK( this, CertificateChooser, ViewButtonHdl ) );

    FreeResource();

    mxSecurityEnvironment = _rxSecurityEnvironment;
    mbInitialized = FALSE;

    // disable buttons
    CertificateHighlightHdl( NULL );
}

CertificateChooser::~CertificateChooser()
{
}

short CertificateChooser::Execute()
{
    // #i48432#
    // We can't check for personal certificates before raising this dialog,
    // because the mozilla implementation throws a NoPassword exception,
    // if the user pressed cancel, and also if the database does not exist!
    // But in the later case, the is no password query, and the user is confused
    // that nothing happens when pressing "Add..." in the SignatureDialog.

    // PostUserEvent( LINK( this, CertificateChooser, Initialize ) );

    // PostUserLink behavior is to slow, so do it directly before Execute().
    // Problem: This Dialog should be visible right now, and the parent should not be accessible.
    // Show, Update, DIsableInput...

    Window* pMe = this;
    Window* pParent = GetParent();
    if ( pParent )
        pParent->EnableInput( FALSE );
    pMe->Show();
    pMe->Update();
    ImplInitialize();
    if ( pParent )
        pParent->EnableInput( TRUE );
    return ModalDialog::Execute();
}

// IMPL_LINK( CertificateChooser, Initialize, void*, EMPTYARG )
void CertificateChooser::ImplInitialize()
{
    if ( !mbInitialized )
    {
        try
        {
            maCerts = mxSecurityEnvironment->getPersonalCertificates();
        }
        catch (security::NoPasswordException&)
        {
        }

        sal_Int32 nCertificates = maCerts.getLength();
        sal_Int32 nCertificatesToIgnore = maCertsToIgnore.size();
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
                    const SignatureInformation& rInf = maCertsToIgnore[ nSig ];
                    if ( ( aIssuerName == rInf.ouX509IssuerName ) &&
                        ( bigIntegerToNumericString( xCert->getSerialNumber() ) == rInf.ouX509SerialNumber ) )
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

                if ( !( nCertificateCharacters & security::CertificateCharacters::HAS_PRIVATE_KEY ) )
                    bIgnoreThis = true;

            }

            if ( bIgnoreThis )
            {
                ::comphelper::removeElementAt( maCerts, nCert );
                nCertificates = maCerts.getLength();
            }
        }

        // fill list of certificates; the first entry will be selected
        for ( sal_Int32 nC = 0; nC < nCertificates; ++nC )
        {
            String sEntry( XmlSec::GetContentPart( maCerts[ nC ]->getSubjectName() ) );
            sEntry += '\t';
            sEntry += XmlSec::GetContentPart( maCerts[ nC ]->getIssuerName() );
            sEntry += '\t';
            sEntry += XmlSec::GetDateString( maCerts[ nC ]->getNotValidAfter() );
            SvLBoxEntry* pEntry = maCertLB.InsertEntry( sEntry );
            pEntry->SetUserData( ( void* )nC ); // missuse user data as index
        }

        // enable/disable buttons
        CertificateHighlightHdl( NULL );
        mbInitialized = TRUE;
    }
}


uno::Reference< dcss::security::XCertificate > CertificateChooser::GetSelectedCertificate()
{
    uno::Reference< dcss::security::XCertificate > xCert;
    USHORT  nSelected = GetSelectedEntryPos();
    if ( nSelected < maCerts.getLength() )
        xCert = maCerts[ nSelected ];
    return xCert;
}

IMPL_LINK( CertificateChooser, CertificateHighlightHdl, void*, EMPTYARG )
{
    sal_Bool bEnable = GetSelectedCertificate().is();
    maViewBtn.Enable( bEnable );
    maOKBtn.Enable( bEnable );
    return 0;
}

IMPL_LINK( CertificateChooser, CertificateSelectHdl, void*, EMPTYARG )
{
    EndDialog( RET_OK );
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
        CertificateViewer aViewer( this, mxSecurityEnvironment, xCert, TRUE );
        aViewer.Execute();
    }
}

