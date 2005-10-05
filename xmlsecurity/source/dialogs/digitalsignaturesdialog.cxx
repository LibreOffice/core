/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: digitalsignaturesdialog.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 14:57:17 $
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
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

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

sal_Bool HandleStreamAsXML_Impl( const uno::Reference < embed::XStorage >& rxStore, const rtl::OUString& rURI )
{
    sal_Bool bResult = sal_False;

    try
    {
        sal_Int32 nSepPos = rURI.indexOf( '/' );
        if ( nSepPos == -1 )
        {
            uno::Reference< io::XStream > xStream;
            xStream = rxStore->cloneStreamElement( rURI );
            if ( !xStream.is() )
                throw uno::RuntimeException();

            ::rtl::OUString aMediaType;
            sal_Bool bEncrypted = sal_False;
            uno::Reference< beans::XPropertySet > xProps( xStream, uno::UNO_QUERY_THROW );
            xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ) ) >>= aMediaType;
            xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsEncrypted" ) ) ) >>= bEncrypted;
            bResult = ( aMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "text/xml" ) ) ) && !bEncrypted );
        }
        else
        {
            rtl::OUString aStoreName = rURI.copy( 0, nSepPos );
            rtl::OUString aElement = rURI.copy( nSepPos+1 );
            uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aStoreName, embed::ElementModes::READ );
            bResult = HandleStreamAsXML_Impl( xSubStore, aElement );
        }
    }
    catch( uno::Exception& )
    {
    }

    return bResult;
}

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
    static long nTabs[] = { 4, 0, 6*DS_LB_WIDTH/100, 36*DS_LB_WIDTH/100, 74*DS_LB_WIDTH/100 };
    maSignaturesLB.SetTabs( &nTabs[ 0 ] );
    maSignaturesLB.InsertHeaderEntry( String( ResId( STR_HEADERBAR ) ) );

    if ( GetBackground().GetColor().IsDark() )
    {
        // high contrast mode needs other images
        maSigsValidImg.SetImage( Image( ResId( IMG_STATE_VALID_HC ) ) );
        maSigsInvalidImg.SetImage( Image( ResId( IMG_STATE_BROKEN_HC ) ) );
    }

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

void DigitalSignaturesDialog::SetSignatureStream( const cssu::Reference < css::io::XStream >& rxStream )
{
    mxSignatureStream = rxStream;
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

    return Dialog::Execute();
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
        uno::Reference<com::sun::star::xml::crypto::XSecurityEnvironment> xSecEnv = maSignatureHelper.GetSecurityEnvironment();
        CertificateChooser aChooser( this, xSecEnv, maCurrentSignatureInformations );
        if ( aChooser.Execute() == RET_OK )
        {
            uno::Reference< ::com::sun::star::security::XCertificate > xCert = aChooser.GetSelectedCertificate();
            if ( !xCert.is() )
            {
                DBG_ERRORFILE( "no certificate selected" );
                return -1;
            }
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
                sal_Int32 nSep = aElements[n].lastIndexOf( '.' );
                if ( nSep != (-1) )
                {
                    ::rtl::OUString aExt = aElements[n].copy( nSep+1 );
                    if ( aExt.equalsIgnoreAsciiCase( aXMLExt ) )
                    {
                        bBinaryMode = !HandleStreamAsXML_Impl( mxStore, aElements[n] );
                    }
                }
                maSignatureHelper.AddForSigning( nSecurityId, aElements[n], aElements[n], bBinaryMode );
            }

            maSignatureHelper.SetDateTime( nSecurityId, Date(), Time() );

            SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream( embed::ElementModes::WRITE|embed::ElementModes::TRUNCATE );
            uno::Reference< io::XOutputStream > xOutputStream( aStreamHelper.xSignatureStream, uno::UNO_QUERY );
            uno::Reference< com::sun::star::xml::sax::XDocumentHandler> xDocumentHandler = maSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream );

            // Export old signatures...
            int nInfos = maCurrentSignatureInformations.size();
            for ( int n = 0; n < nInfos; n++ )
                maSignatureHelper.ExportSignature( xDocumentHandler, maCurrentSignatureInformations[n]);

            // Create a new one...
            maSignatureHelper.CreateAndWriteSignature( xDocumentHandler );

            // That's it...
            maSignatureHelper.CloseDocumentHandler( xDocumentHandler);

            maSignatureHelper.EndMission();

            sal_Int32 nStatus = maSignatureHelper.GetSignatureInformation( nSecurityId ).nStatus;
            if ( nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
            {
                mbSignaturesChanged = true;

                // Can't simply remember current information, need parsing for getting full information :(
                ImplGetSignatureInformations();
                ImplFillSignaturesBox();
            }
        }
    }
    catch ( uno::Exception& )
    {
        DBG_ERROR( "Exception while adding a signature!" );
        // Don't keep invalid entries...
        ImplGetSignatureInformations();
        ImplFillSignaturesBox();
    }

    return 0;
}

IMPL_LINK( DigitalSignaturesDialog, RemoveButtonHdl, Button*, EMPTYARG )
{
    if( maSignaturesLB.FirstSelected() )
    {
        try
        {
            USHORT nSelected = (USHORT) (sal_uIntPtr) maSignaturesLB.FirstSelected()->GetUserData();
            maCurrentSignatureInformations.erase( maCurrentSignatureInformations.begin()+nSelected );

            // Export all other signatures...
            SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream( embed::ElementModes::WRITE|embed::ElementModes::TRUNCATE );
            uno::Reference< io::XOutputStream > xOutputStream( aStreamHelper.xSignatureStream, uno::UNO_QUERY );
            uno::Reference< com::sun::star::xml::sax::XDocumentHandler> xDocumentHandler = maSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream );

            int nInfos = maCurrentSignatureInformations.size();
            for( int n = 0 ; n < nInfos ; ++n )
                maSignatureHelper.ExportSignature( xDocumentHandler, maCurrentSignatureInformations[ n ] );

            maSignatureHelper.CloseDocumentHandler( xDocumentHandler);

            mbSignaturesChanged = true;

            ImplFillSignaturesBox();
        }
        catch ( uno::Exception& )
        {
            DBG_ERROR( "Exception while removing a signature!" );
            // Don't keep invalid entries...
            ImplGetSignatureInformations();
            ImplFillSignaturesBox();
        }
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
            // MT: Maybe after 2.0: Why not always use the attached certificate?
            if ( !xCert.is() && rInfo.ouX509Certificate.getLength() )
                xCert = xSecEnv->createCertificateFromAscii( rInfo.ouX509Certificate ) ;

            DBG_ASSERT( xCert.is(), "Certificate not found and can't be created!" );

            String  aSubject;
            String  aIssuer;
            String  aDateTimeStr;
            if( xCert.is() )
            {
                aSubject = XmlSec::GetContentPart( xCert->getSubjectName() );
                aIssuer = XmlSec::GetContentPart( rInfo.ouX509IssuerName );
                // --> PB 2004-10-12 #i20172# String with date and time information
                aDateTimeStr = XmlSec::GetDateTimeString( rInfo.stDateTime );
            }

            // New signatures are not verified, must be valid. Status is INIT.
            // HACK for #i46696#
            // KEY_NOT_FOUND only happen because of author or issuer certificates are missing in keystore.
            // We always have the key from authors certificate, because it's attached.
            // This is a question of trust, not of a *broken* signature.
            bool bValid = ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
                || ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN )
                || ( ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_KEY_NOT_FOUND) && rInfo.ouX509Certificate.getLength());

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

    SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream( embed::ElementModes::READ );
    if ( aStreamHelper.xSignatureStream.is() )
    {
        uno::Reference< io::XInputStream > xInputStream( aStreamHelper.xSignatureStream, uno::UNO_QUERY );
        maSignatureHelper.ReadAndVerifySignature( xInputStream );
    }
    maSignatureHelper.EndMission();

    maCurrentSignatureInformations = maSignatureHelper.GetSignatureInformations();

    mbVerifySignatures = false;
}

void DigitalSignaturesDialog::ImplShowSignaturesDetails()
{
    if( maSignaturesLB.FirstSelected() )
    {
        USHORT nSelected = (USHORT) (sal_uIntPtr) maSignaturesLB.FirstSelected()->GetUserData();
        const SignatureInformation& rInfo = maCurrentSignatureInformations[ nSelected ];

        // Use Certificate from doc, not from key store
        uno::Reference< dcss::security::XCertificate > xCert = rInfo.ouX509Certificate.getLength() ? maSignatureHelper.GetSecurityEnvironment()->createCertificateFromAscii( rInfo.ouX509Certificate ) : NULL;
        DBG_ASSERT( xCert.is(), "Error getting cCertificate!" );
        if ( xCert.is() )
        {
            CertificateViewer aViewer( this, maSignatureHelper.GetSecurityEnvironment(), xCert, FALSE );
            aViewer.Execute();
        }
    }
}

SignatureStreamHelper DigitalSignaturesDialog::ImplOpenSignatureStream( sal_Int32 nStreamOpenMode )
{
    SignatureStreamHelper aHelper;
    if ( !mxSignatureStream.is() )
    {
        aHelper = DocumentSignatureHelper::OpenSignatureStream( mxStore, nStreamOpenMode, meSignatureMode );
    }
    else
    {
        aHelper.xSignatureStream = mxSignatureStream;
        if ( nStreamOpenMode & embed::ElementModes::TRUNCATE )
        {
            css::uno::Reference < css::io::XTruncate > xTruncate( mxSignatureStream, uno::UNO_QUERY );
            DBG_ASSERT( xTruncate.is(), "ImplOpenSignatureStream - Stream does not support xTruncate!" );
            xTruncate->truncate();
        }
        else
        {
            css::uno::Reference < css::io::XSeekable > xSeek( mxSignatureStream, uno::UNO_QUERY );
            DBG_ASSERT( xSeek.is(), "ImplOpenSignatureStream - Stream does not support xSeekable!" );
            xSeek->seek( 0 );
        }
    }

    return aHelper;
}

