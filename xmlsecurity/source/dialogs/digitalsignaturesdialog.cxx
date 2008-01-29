/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: digitalsignaturesdialog.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:54:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"
#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/biginteger.hxx>
#include "xmlsecurity/baseencoding.hxx"

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
#include <com/sun/star/security/CertificateValidity.hdl>

#ifndef _COM_SUN_STAR_PACKAGES_WRONGPASSWORDEXCEPTION_HPP_
#include <com/sun/star/packages/WrongPasswordException.hpp>
#endif

#include <tools/date.hxx>
#include <tools/time.hxx>

#include "dialogs.hrc"
#include "helpids.hrc"
#include "resourcemanager.hxx"

#include <vcl/msgbox.hxx> // Until encrypted docs work...

using namespace ::com::sun::star::security;
namespace css = ::com::sun::star;

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
    ,maHintDocFT        ( this, XMLSEC_RES( FT_HINT_DOC ) )
    ,maHintBasicFT      ( this, XMLSEC_RES( FT_HINT_BASIC ) )
    ,maHintPackageFT    ( this, XMLSEC_RES( FT_HINT_PACK ) )
    ,maSignaturesLB     ( this, XMLSEC_RES( LB_SIGNATURES ) )
    ,maSigsValidImg     ( this, XMLSEC_RES( IMG_STATE_VALID ) )
    ,maSigsValidFI      ( this, XMLSEC_RES( FI_STATE_VALID ) )
    ,maSigsInvalidImg   ( this, XMLSEC_RES( IMG_STATE_BROKEN ) )
    ,maSigsInvalidFI    ( this, XMLSEC_RES( FI_STATE_BROKEN ) )
    ,maSigsNotvalidatedImg( this, XMLSEC_RES( IMG_STATE_NOTVALIDATED ) )
    ,maSigsNotvalidatedFI ( this, XMLSEC_RES( FI_STATE_NOTVALIDATED ) )
    ,maViewBtn          ( this, XMLSEC_RES( BTN_VIEWCERT ) )
    ,maAddBtn           ( this, XMLSEC_RES( BTN_ADDCERT ) )
    ,maRemoveBtn        ( this, XMLSEC_RES( BTN_REMOVECERT ) )
    ,maBottomSepFL      ( this, XMLSEC_RES( FL_BOTTOM_SEP ) )
    ,maOKBtn            ( this, XMLSEC_RES( BTN_OK ) )
    ,maCancelBtn        ( this, XMLSEC_RES( BTN_CANCEL ) )
    ,maHelpBtn          ( this, XMLSEC_RES( BTN_HELP ) )
{
    // --> PB #i48253 the tablistbox needs its own unique id
    maSignaturesLB.Window::SetUniqueId( HID_XMLSEC_TREE_SIGNATURESDLG );
    // <--
    static long nTabs[] = { 4, 0, 6*DS_LB_WIDTH/100, 36*DS_LB_WIDTH/100, 74*DS_LB_WIDTH/100 };
    maSignaturesLB.SetTabs( &nTabs[ 0 ] );
    maSignaturesLB.InsertHeaderEntry( String( XMLSEC_RES( STR_HEADERBAR ) ) );

    maSigsNotvalidatedFI.SetText( String( XMLSEC_RES( STR_NO_INFO_TO_VERIFY ) ) );

    if ( GetBackground().GetColor().IsDark() )
    {
        // high contrast mode needs other images
        maSigsValidImg.SetImage( Image( XMLSEC_RES( IMG_STATE_VALID_HC ) ) );
        maSigsInvalidImg.SetImage( Image( XMLSEC_RES( IMG_STATE_BROKEN_HC ) ) );
        maSigsNotvalidatedImg.SetImage( Image( XMLSEC_RES( IMG_STATE_NOTVALIDATED_HC ) ) );
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
    XmlSec::AlignAndFitImageAndControl( maSigsNotvalidatedImg, maSigsNotvalidatedFI, 5 );
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

            maSignatureHelper.SetX509Certificate( nSecurityId, xCert->getIssuerName(), aCertSerial,
                baseEncode(xCert->getEncoded(), BASE64));

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

            // If stream was not provided, we are responsible for committing it....
            if ( !mxSignatureStream.is() )
            {
                uno::Reference< embed::XTransactedObject > xTrans( aStreamHelper.xSignatureStorage, uno::UNO_QUERY );
                xTrans->commit();
            }

            aStreamHelper = SignatureStreamHelper();    // release objects...

            sal_Int32 nStatus = maSignatureHelper.GetSignatureInformation( nSecurityId ).nStatus;
            if ( nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
            {
                mbSignaturesChanged = true;

                // Can't simply remember current information, need parsing for getting full information :(
                // We need to verify the signatures again, otherwise the status in the signature information
                // will not contain
                // SecurityOperationStatus_OPERATION_SUCCEEDED
                mbVerifySignatures = true;
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

            // If stream was not provided, we are responsible for committing it....
            if ( !mxSignatureStream.is() )
            {
                uno::Reference< embed::XTransactedObject > xTrans( aStreamHelper.xSignatureStorage, uno::UNO_QUERY );
                xTrans->commit();
            }

            aStreamHelper = SignatureStreamHelper();    // release objects...

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
    int nValidSigs = 0, nValidCerts = 0;

    if( nInfos )
    {
        std::vector< rtl::OUString > aElementsToBeVerified = DocumentSignatureHelper::CreateElementList( mxStore, ::rtl::OUString(), meSignatureMode );
        for( int n = 0; n < nInfos; ++n )
        {
            const SignatureInformation& rInfo = maCurrentSignatureInformations[n];
            //First we try to get the certificate which is embedded in the XML Signature
            if (rInfo.ouX509Certificate.getLength())
                xCert = xSecEnv->createCertificateFromAscii(rInfo.ouX509Certificate);
            else
                //There must be an embedded certificate because we use it to get the
                //issuer name. We cannot use /Signature/KeyInfo/X509Data/X509IssuerName
                //because it could be modified by an attacker. The issuer is displayed
                //in the digital signature dialog.
                //Comparing the X509IssuerName with the one from the X509Certificate in order
                //to find out if the X509IssuerName was modified does not work. See #i62684
                DBG_ASSERT(sal_False, "Could not find embedded certificate!");

            //In case there is no embedded certificate we try to get it from a local store
            //Todo: This probably could be removed, see above.
            if (!xCert.is())
                xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );

            DBG_ASSERT( xCert.is(), "Certificate not found and can't be created!" );

            String  aSubject;
            String  aIssuer;
            String  aDateTimeStr;

            bool bSigValid = false;
            bool bCertValid = false;
            if( xCert.is() )
            {
                //check the validity of the cert
                try {
                    sal_Int32 certResult = xSecEnv->verifyCertificate(xCert);

                    //These errors are alloweds
                    sal_Int32 validErrors = css::security::CertificateValidity::VALID
                        | css::security::CertificateValidity::UNKNOWN_REVOKATION;

                    //Build a  mask to filter out the allowed errors
                    sal_Int32 mask = ~validErrors;
                    // "subtract" the allowed error flags from the result
                    sal_Int32 errors = certResult & mask;
                    bCertValid = errors > 0 ? false : true;
                    if ( bCertValid )
                        nValidCerts++;

                } catch (css::uno::SecurityException& ) {
                    OSL_ENSURE(0, "Verification of certificate failed");
                    bCertValid = false;
                }

                aSubject = XmlSec::GetContentPart( xCert->getSubjectName() );
                aIssuer = XmlSec::GetContentPart( xCert->getIssuerName() );
                // --> PB 2004-10-12 #i20172# String with date and time information
                aDateTimeStr = XmlSec::GetDateTimeString( rInfo.stDateTime );
            }
            bSigValid = ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED );

            if ( bSigValid )
            {
                // Can only be valid if ALL streams are signed, which means real stream count == signed stream count
                unsigned int nRealCount = 0;
                for ( int i = rInfo.vSignatureReferenceInfors.size(); i; )
                {
                    const SignatureReferenceInformation& rInf = rInfo.vSignatureReferenceInfors[--i];
                    // There is also an extra entry of type TYPE_SAMEDOCUMENT_REFERENCE because of signature date.
                    if ( ( rInf.nType == TYPE_BINARYSTREAM_REFERENCE ) || ( rInf.nType == TYPE_XMLSTREAM_REFERENCE ) )
                        nRealCount++;
                }
                bSigValid = ( aElementsToBeVerified.size() == nRealCount );

                if( bSigValid )
                    nValidSigs++;
            }

            Image aImage;
            if ( bSigValid && bCertValid )
                aImage = maSigsValidImg.GetImage();
            else if ( bSigValid && !bCertValid )
                aImage = maSigsNotvalidatedImg.GetImage();
            else if ( !bSigValid )
                aImage = maSigsInvalidImg.GetImage();

            SvLBoxEntry* pEntry = maSignaturesLB.InsertEntry( aNullStr, aImage, aImage );
            maSignaturesLB.SetEntryText( aSubject, pEntry, 1 );
            maSignaturesLB.SetEntryText( aIssuer, pEntry, 2 );
            maSignaturesLB.SetEntryText( aDateTimeStr, pEntry, 3 );
            pEntry->SetUserData( ( void* ) n );     // missuse user data as index
        }
    }

    bool bAllSigsValid = ( nValidSigs == nInfos );
    bool bAllCertsValid = ( nValidCerts == nInfos );
    bool bShowValidState = nInfos && ( bAllSigsValid && bAllCertsValid );
    bool bShowNotValidatedState = nInfos && ( bAllSigsValid && !bAllCertsValid );
    bool bShowInvalidState = nInfos && !bAllSigsValid;
    maSigsValidImg.Show( bShowValidState );
    maSigsValidFI.Show( bShowValidState );
    maSigsInvalidImg.Show( bShowInvalidState );
    maSigsInvalidFI.Show( bShowInvalidState );
    maSigsNotvalidatedImg.Show( bShowNotValidatedState );
    maSigsNotvalidatedFI.Show( bShowNotValidatedState );

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
        css::uno::Reference<css::xml::crypto::XSecurityEnvironment > xSecEnv =
            maSignatureHelper.GetSecurityEnvironment();
        // Use Certificate from doc, not from key store
        uno::Reference< dcss::security::XCertificate > xCert;
        if (rInfo.ouX509Certificate.getLength())
            xCert = xSecEnv->createCertificateFromAscii(rInfo.ouX509Certificate);
        //fallback if no certificate is embedded, get if from store
        if (!xCert.is())
            xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );

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

