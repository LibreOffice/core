/*************************************************************************
 *
 *  $RCSfile: digitalsignaturesdialog.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: mmi $ $Date: 2004-08-12 02:29:21 $
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

#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/biginteger.hxx>

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#ifndef _COM_SUN_STAR_PACKAGES_WRONGPASSWORDEXCEPTION_HPP_
#include <com/sun/star/packages/WrongPasswordException.hpp>
#endif

#include <tools/intn.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>

#include "dialogs.hrc"
#include "resourcemanager.hxx"

#include <vcl/msgbox.hxx> // Until encrypted docs work...

using namespace ::com::sun::star::security;

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;

DigitalSignaturesDialog::DigitalSignaturesDialog( Window* pParent, uno::Reference< lang::XMultiServiceFactory >& rxMSF, DocumentSignatureMode eMode, sal_Bool bReadOnly )
    :ModalDialog        ( pParent, XMLSEC_RES( RID_XMLSECDLG_DIGSIG ) )
    ,maSignatureHelper  ( rxMSF )
    ,meSignatureMode    ( eMode )
    ,maHintDocFT        ( this, ResId( FT_HINT_DOC ) )
    ,maHintBasicFT      ( this, ResId( FT_HINT_BASIC ) )
    ,maHintPackageFT    ( this, ResId( FT_HINT_PACK ) )
    ,maSignaturesLB     ( this, ResId( LB_SIGNATURES ) )
    ,maSigsValidImg     ( this, ResId( IMG_STATE_VALID ) )
    ,maSigsValidFI      ( this, ResId( FI_STATE_VALID ) )
    ,maSigsInvalidImg   ( this, ResId( IMG_STATE_BROKEN ) )
    ,maSigsInvalidFI    ( this, ResId( FI_STATE_BROKEN ) )
    ,maViewBtn          ( this, ResId( BTN_VIEWCERT ) )
    ,maAddBtn           ( this, ResId( BTN_ADDCERT ) )
    ,maRemoveBtn        ( this, ResId( BTN_REMOVECERT ) )
    ,maBottomSepFL      ( this, ResId( FL_BOTTOM_SEP ) )
    ,maOKBtn            ( this, ResId( BTN_OK ) )
    ,maCancelBtn        ( this, ResId( BTN_CANCEL ) )
    ,maHelpBtn          ( this, ResId( BTN_HELP ) )
{
    static long nTabs[] = { 4, 0, 8*DS_LB_WIDTH/100, 36*DS_LB_WIDTH/100, 74*DS_LB_WIDTH/100 };
    maSignaturesLB.SetTabs( &nTabs[ 0 ] );
    maSignaturesLB.InsertHeaderEntry( String( ResId( STR_HEADERBAR ) ) );

    FreeResource();

    mbVerifySignatures = true;
    mbSignaturesChanged = false;

    maSignaturesLB.SetSelectHdl( LINK( this, DigitalSignaturesDialog, SignatureHighlightHdl ) );
    maSignaturesLB.SetDoubleClickHdl( LINK( this, DigitalSignaturesDialog, SignatureSelectHdl ) );

    maViewBtn.SetClickHdl( LINK( this, DigitalSignaturesDialog, ViewButtonHdl ) );
    maViewBtn.Disable();

    maAddBtn.SetClickHdl( LINK( this, DigitalSignaturesDialog, AddButtonHdl ) );
    if ( bReadOnly )
        maAddBtn.Disable();

    maRemoveBtn.SetClickHdl( LINK( this, DigitalSignaturesDialog, RemoveButtonHdl ) );
    maRemoveBtn.Disable();

    switch( meSignatureMode )
    {
        case SignatureModeDocumentContent:  maHintDocFT.Show();     break;
        case SignatureModeMacros:           maHintBasicFT.Show();   break;
        case SignatureModePackage:          maHintPackageFT.Show(); break;
    }

    // adjust fixed text to images
    XmlSec::AlignAndFitImageAndControl( maSigsValidImg, maSigsValidFI, 5 );
    XmlSec::AlignAndFitImageAndControl( maSigsInvalidImg, maSigsInvalidFI, 5 );
}

DigitalSignaturesDialog::~DigitalSignaturesDialog()
{
}

BOOL DigitalSignaturesDialog::Init( const rtl::OUString& rTokenName )
{
    bool bInit = maSignatureHelper.Init( rTokenName );

    DBG_ASSERT( bInit, "Error initializing security context!" );

    if ( bInit )
    {
        maSignatureHelper.SetStartVerifySignatureHdl( LINK( this, DigitalSignaturesDialog, StartVerifySignatureHdl ) );
    }

    return bInit;
}

void DigitalSignaturesDialog::SetStorage( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStore )
{
    mxStore = rxStore;
    maSignatureHelper.SetStorage( mxStore );
}



short DigitalSignaturesDialog::Execute()
{
    // Verify Signatures and add certificates to ListBox...
    mbVerifySignatures = true;
    ImplGetSignatureInformations();
    ImplFillSignaturesBox();

    // Only verify once, content will not change.
    // But for refreshing signature information, StartVerifySignatureHdl will be called after each add/remove
    mbVerifySignatures = false;

    short nRet = Dialog::Execute();

    return nRet;
}

IMPL_LINK( DigitalSignaturesDialog, SignatureHighlightHdl, void*, EMPTYARG )
{
    bool bSel = maSignaturesLB.FirstSelected() ? true : false;
    maViewBtn.Enable( bSel );
    if ( maAddBtn.IsEnabled() ) // not read only
        maRemoveBtn.Enable( bSel );

    return 0;
}

IMPL_LINK( DigitalSignaturesDialog, SignatureSelectHdl, void*, EMPTYARG )
{
    ImplShowSignaturesDetails();
    return 0;
}

IMPL_LINK( DigitalSignaturesDialog, ViewButtonHdl, Button*, EMPTYARG )
{
    ImplShowSignaturesDetails();
    return 0;
}

IMPL_LINK( DigitalSignaturesDialog, AddButtonHdl, Button*, EMPTYARG )
{
    try
    {
        // Check if we have the password for the key store.
        // If no, we don't want to raise an empty dialog.
        maSignatureHelper.GetSecurityEnvironment()->getPersonalCertificates();

        uno::Reference<com::sun::star::xml::crypto::XSecurityEnvironment> xSecEnv = maSignatureHelper.GetSecurityEnvironment();
        CertificateChooser aChooser( this, xSecEnv, maCurrentSignatureInformations );
        if( aChooser.Execute() )
        {
            uno::Reference< ::com::sun::star::security::XCertificate > xCert = aChooser.GetSelectedCertificate();
            rtl::OUString aCertSerial = bigIntegerToNumericString( xCert->getSerialNumber() );
            if ( !aCertSerial.getLength() )
            {
                DBG_ERROR( "Error in Certificate, problem with serial number!" );
                return -1;
            }

            maSignatureHelper.StartMission();

            sal_Int32 nSecurityId = maSignatureHelper.GetNewSecurityId();
            maSignatureHelper.SetX509Certificate( nSecurityId, xCert->getIssuerName(), aCertSerial );

            std::vector< rtl::OUString > aElements = DocumentSignatureHelper::CreateElementList( mxStore, rtl::OUString(), meSignatureMode );

            ::rtl::OUString aXMLExt( RTL_CONSTASCII_USTRINGPARAM( "XML" ) );
            sal_Int32 nElements = aElements.size();
            for ( sal_Int32 n = 0; n < nElements; n++ )
            {
                bool bBinaryMode = true;
                bool bEncrypted = false;
                sal_Int32 nSep = aElements[n].lastIndexOf( '.' );
                if ( nSep != (-1) )
                {
                    ::rtl::OUString aExt = aElements[n].copy( nSep+1 );
                    if ( aExt.equalsIgnoreAsciiCase( aXMLExt ) )
                    {
                        bBinaryMode = false;
                        // if it's encrypted, we will get a packages::WrongPasswordException and must use binary mode.
                        try
                        {
                            maSignatureHelper.GetUriBinding()->getUriBinding( aElements[n] );
                        }
                        catch (packages::WrongPasswordException)
                        {
                            bBinaryMode = true;
                            // opening encrypted stream soesn't work, so don't sign ecncrypted docs in EA
                            ErrorBox( this, WB_OK, String( RTL_CONSTASCII_USTRINGPARAM( "This version does not support signing of encrypted documents right now." ) ) ).Execute();
                            return -1;
                        }
                    }
                }
                maSignatureHelper.AddForSigning( nSecurityId, aElements[n], aElements[n], bBinaryMode );
            }

            maSignatureHelper.SetDateTime( nSecurityId, Date(), Time() );

            SignatureStreamHelper aStreamHelper = DocumentSignatureHelper::OpenSignatureStream( mxStore, embed::ElementModes::WRITE|embed::ElementModes::TRUNCATE, meSignatureMode );
            uno::Reference< io::XOutputStream > xOutputStream( aStreamHelper.xSignatureStream, uno::UNO_QUERY );
            uno::Reference< com::sun::star::xml::sax::XDocumentHandler> xDocumentHandler = maSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream );

            // Export old signatures...
            int nInfos = maCurrentSignatureInformations.size();
            for ( int n = 0; n < nInfos; n++ )
                maSignatureHelper.ExportSignature( xDocumentHandler, maCurrentSignatureInformations[n]);

            // Create a new one...
            bool bDone = maSignatureHelper.CreateAndWriteSignature( xDocumentHandler );

            // That's it...
            maSignatureHelper.CloseDocumentHandler( xDocumentHandler);

            maSignatureHelper.EndMission();

            if ( bDone )
            {
                uno::Reference< embed::XTransactedObject > xTrans( aStreamHelper.xSignatureStorage, uno::UNO_QUERY );
                xTrans->commit();

                uno::Reference< embed::XTransactedObject > xTrans2( mxStore, uno::UNO_QUERY );
                xTrans2->commit();

                aStreamHelper.Clear();

                mbSignaturesChanged = true;

                // Can't simply remember current information, need parsing for getting full information :(
                ImplGetSignatureInformations();
                ImplFillSignaturesBox();
            }

            aStreamHelper.Clear();
        }
    }
    catch (NoPasswordException&)
    {
    }

    return 0;
}

IMPL_LINK( DigitalSignaturesDialog, RemoveButtonHdl, Button*, EMPTYARG )
{
    if( maSignaturesLB.FirstSelected() )
    {
        USHORT nSelected = (USHORT) (sal_Int32) maSignaturesLB.FirstSelected()->GetUserData();
        maCurrentSignatureInformations.erase( maCurrentSignatureInformations.begin()+nSelected );

        // Export all other signatures...
        SignatureStreamHelper aStreamHelper = DocumentSignatureHelper::OpenSignatureStream( mxStore, embed::ElementModes::WRITE|embed::ElementModes::TRUNCATE, meSignatureMode );
        uno::Reference< io::XOutputStream > xOutputStream( aStreamHelper.xSignatureStream, uno::UNO_QUERY );
        uno::Reference< com::sun::star::xml::sax::XDocumentHandler> xDocumentHandler = maSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream );

        int nInfos = maCurrentSignatureInformations.size();
        for( int n = 0 ; n < nInfos ; ++n )
            maSignatureHelper.ExportSignature( xDocumentHandler, maCurrentSignatureInformations[ n ] );

        maSignatureHelper.CloseDocumentHandler( xDocumentHandler);

        uno::Reference< embed::XTransactedObject > xTrans( aStreamHelper.xSignatureStorage, uno::UNO_QUERY );
        xTrans->commit();

        uno::Reference< embed::XTransactedObject > xTrans2( mxStore, uno::UNO_QUERY );
        xTrans2->commit();

        mbSignaturesChanged = true;

        aStreamHelper.Clear();

        ImplFillSignaturesBox();
    }

    return 0;
}

IMPL_LINK( DigitalSignaturesDialog, StartVerifySignatureHdl, void*, EMPTYARG )
{
    return mbVerifySignatures ? 1 : 0;
}

void DigitalSignaturesDialog::ImplFillSignaturesBox()
{
    maSignaturesLB.Clear();

    uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecEnv = maSignatureHelper.GetSecurityEnvironment();
    uno::Reference< ::com::sun::star::security::XCertificate > xCert;

    String aCN_Id( String::CreateFromAscii( "CN" ) );
    String aNullStr;
    int nInfos = maCurrentSignatureInformations.size();
    int nValidSigs = 0;

    if( nInfos )
    {
        std::vector< rtl::OUString > aElementsToBeVerified = DocumentSignatureHelper::CreateElementList( mxStore, ::rtl::OUString(), meSignatureMode );
        for( int n = 0; n < nInfos; ++n )
        {
            const SignatureInformation& rInfo = maCurrentSignatureInformations[n];
            xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );

            // If we don't get it, create it from signature data:
            if ( !xCert.is() )
                xCert = xSecEnv->createCertificateFromAscii( rInfo.ouX509Certificate ) ;

            DBG_ASSERT( xCert.is(), "Certificate not found and can't be created!" );

            String  aSubject;
            String  aIssuer;
            String  aDateTimeStr;
            if( xCert.is() )
            {
                aSubject = XmlSec::GetContentPart( xCert->getSubjectName(), aCN_Id );
                aIssuer = XmlSec::GetContentPart( rInfo.ouX509IssuerName, aCN_Id );

                    // MM : the ouDate and ouTime fields have been replaced with stDateTime (com::sun::star::util::DataTime)
                    /*
                aDateTimeStr = XmlSec::GetDateTimeString( rInfo.ouDate, rInfo.ouTime );
                */
            }

            // New signatures are not verified, must be valid. Status is INIT.
            bool bValid = ( rInfo.nStatus == STATUS_VERIFY_SUCCEED ) || ( rInfo.nStatus == STATUS_INIT );
            if ( bValid )
            {
                // Can only be valid if ALL streams are signed, which means real stream count == signed stream count
                int nRealCount = 0;
                for ( int i = rInfo.vSignatureReferenceInfors.size(); i; )
                {
                    const SignatureReferenceInformation& rInf = rInfo.vSignatureReferenceInfors[--i];
                    // There is also an extra entry of type TYPE_SAMEDOCUMENT_REFERENCE because of signature date.
                    if ( ( rInf.nType == TYPE_BINARYSTREAM_REFERENCE ) || ( rInf.nType == TYPE_XMLSTREAM_REFERENCE ) )
                        nRealCount++;
                }
                bValid = ( aElementsToBeVerified.size() == nRealCount );

                if( bValid )
                    nValidSigs++;
            }

            Image aImg( bValid? maSigsValidImg.GetImage() : maSigsInvalidImg.GetImage() );
            SvLBoxEntry* pEntry = maSignaturesLB.InsertEntry( aNullStr, aImg, aImg );
            maSignaturesLB.SetEntryText( aSubject, pEntry, 1 );
            maSignaturesLB.SetEntryText( aIssuer, pEntry, 2 );
            maSignaturesLB.SetEntryText( aDateTimeStr, pEntry, 3 );
            pEntry->SetUserData( ( void* ) n );     // missuse user data as index
        }
    }

    bool bAllSigsValid = ( nValidSigs == nInfos );
    bool bShowValidState = nInfos && bAllSigsValid;
    bool bShowInvalidState = nInfos && !bAllSigsValid;
    maSigsValidImg.Show( bShowValidState );
    maSigsValidFI.Show( bShowValidState );
    maSigsInvalidImg.Show( bShowInvalidState );
    maSigsInvalidFI.Show( bShowInvalidState );


    SignatureHighlightHdl( NULL );
}

void DigitalSignaturesDialog::ImplGetSignatureInformations()
{
    maCurrentSignatureInformations.clear();

    maSignatureHelper.StartMission();

    SignatureStreamHelper aStreamHelper = DocumentSignatureHelper::OpenSignatureStream( mxStore, embed::ElementModes::READ, meSignatureMode );
    if ( aStreamHelper.xSignatureStream.is() )
    {
        uno::Reference< io::XInputStream > xInputStream( aStreamHelper.xSignatureStream, uno::UNO_QUERY );
        maSignatureHelper.ReadAndVerifySignature( xInputStream );
    }
    maSignatureHelper.EndMission();

    maCurrentSignatureInformations = maSignatureHelper.GetSignatureInformations();

    aStreamHelper.Clear();

    mbVerifySignatures = false;
}

void DigitalSignaturesDialog::ImplShowSignaturesDetails()
{
    if( maSignaturesLB.FirstSelected() )
    {
        USHORT nSelected = (USHORT) (sal_Int32) maSignaturesLB.FirstSelected()->GetUserData();
        const SignatureInformation& rInfo = maCurrentSignatureInformations[ nSelected ];
        uno::Reference< dcss::security::XCertificate > xCert = maSignatureHelper.GetSecurityEnvironment()->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );

        // If we don't get it, create it from signature data:
        if ( !xCert.is() )
            xCert = maSignatureHelper.GetSecurityEnvironment()->createCertificateFromAscii( rInfo.ouX509Certificate ) ;

        DBG_ASSERT( xCert.is(), "Certificate not found and can't be created!" );

        uno::Reference<com::sun::star::xml::crypto::XSecurityEnvironment> xSecEnv = maSignatureHelper.GetSecurityEnvironment();
        CertificateViewer aViewer( this, xSecEnv, xCert );
        aViewer.Execute();
    }
}
