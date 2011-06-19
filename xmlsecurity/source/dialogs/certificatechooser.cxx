/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>

#include <dialogs.hrc>
#include <resourcemanager.hxx>
#include <vcl/msgbox.hxx>

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;

#define INVAL_SEL       0xFFFF

sal_uInt16 CertificateChooser::GetSelectedEntryPos( void ) const
{
    sal_uInt16  nSel = INVAL_SEL;

    SvLBoxEntry* pSel = maCertLB.FirstSelected();
    if( pSel )
        nSel = (sal_uInt16) ( sal_uIntPtr ) pSel->GetUserData();

    return (sal_uInt16) nSel;
}

CertificateChooser::CertificateChooser( Window* _pParent, uno::Reference< uno::XComponentContext>& _rxCtx, uno::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment, const SignatureInformations& _rCertsToIgnore )
    :ModalDialog    ( _pParent, XMLSEC_RES( RID_XMLSECDLG_CERTCHOOSER ) )
    ,maCertsToIgnore( _rCertsToIgnore )
    ,maHintFT       ( this, XMLSEC_RES( FT_HINT_SELECT ) )
    ,maCertLB       ( this, XMLSEC_RES( LB_SIGNATURES ) )
    ,maViewBtn      ( this, XMLSEC_RES( BTN_VIEWCERT ) )
    ,maBottomSepFL  ( this, XMLSEC_RES( FL_BOTTOM_SEP ) )
    ,maOKBtn        ( this, XMLSEC_RES( BTN_OK ) )
    ,maCancelBtn    ( this, XMLSEC_RES( BTN_CANCEL ) )
    ,maHelpBtn      ( this, XMLSEC_RES( BTN_HELP ) )
{
    static long nTabs[] = { 3, 0, 30*CS_LB_WIDTH/100, 60*CS_LB_WIDTH/100 };
    maCertLB.SetTabs( &nTabs[0] );
    maCertLB.InsertHeaderEntry( String( XMLSEC_RES( STR_HEADERBAR ) ) );
    maCertLB.SetSelectHdl( LINK( this, CertificateChooser, CertificateHighlightHdl ) );
    maCertLB.SetDoubleClickHdl( LINK( this, CertificateChooser, CertificateSelectHdl ) );
    maViewBtn.SetClickHdl( LINK( this, CertificateChooser, ViewButtonHdl ) );

    FreeResource();

    mxCtx = _rxCtx;
    mxSecurityEnvironment = _rxSecurityEnvironment;
    mbInitialized = sal_False;

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
        pParent->EnableInput( sal_False );
    pMe->Show();
    pMe->Update();
    ImplInitialize();
    if ( pParent )
        pParent->EnableInput( sal_True );
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

        uno::Reference< dcss::security::XSerialNumberAdapter> xSerialNumberAdapter =
            ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

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
                        ( xSerialNumberAdapter->toString( xCert->getSerialNumber() ) == rInf.ouX509SerialNumber ) )
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
            pEntry->SetUserData( ( void* )(sal_IntPtr)nC ); // missuse user data as index
        }

        // enable/disable buttons
        CertificateHighlightHdl( NULL );
        mbInitialized = sal_True;
    }
}


uno::Reference< dcss::security::XCertificate > CertificateChooser::GetSelectedCertificate()
{
    uno::Reference< dcss::security::XCertificate > xCert;
    sal_uInt16  nSelected = GetSelectedEntryPos();
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
        CertificateViewer aViewer( this, mxSecurityEnvironment, xCert, sal_True );
        aViewer.Execute();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
