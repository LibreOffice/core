/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>


#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>

#include "dialogs.hrc"
#include "digitalsignaturesdialog.hrc"
#include "helpids.hrc"
#include "resourcemanager.hxx"

#include <vcl/msgbox.hxx> // Until encrypted docs work...
#include <unotools/configitem.hxx>
#include <comphelper/componentcontext.hxx>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star::security;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
namespace css = ::com::sun::star;

namespace
{
    class SaveODFItem: public utl::ConfigItem
    {
        sal_Int16 m_nODF;
    public:
    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
        SaveODFItem();
        //See group ODF in Common.xcs
        bool isLessODF1_2()
        {
            return m_nODF < 3;
        }
    };

void SaveODFItem::Commit() {}
void SaveODFItem::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

    SaveODFItem::SaveODFItem(): utl::ConfigItem(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "Office.Common/Save"))), m_nODF(0)
    {
        OUString sDef(RTL_CONSTASCII_USTRINGPARAM("ODF/DefaultVersion"));
        Sequence< css::uno::Any > aValues = GetProperties( Sequence<OUString>(&sDef,1) );
        if ( aValues.getLength() == 1)
        {
            sal_Int16 nTmp = 0;
            if ( aValues[0] >>= nTmp )
                m_nODF = nTmp;
            else
                throw uno::RuntimeException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "[xmlsecurity]SaveODFItem::SaveODFItem(): Wrong Type!")), 0 );

        }
        else
            throw uno::RuntimeException(
                OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "[xmlsecurity] Could not open property Office.Common/Save/ODF/DefaultVersion")), 0);
    }
}

/* Using the zip storage, we cannot get the properties "MediaType" and "IsEncrypted"
    We use the manifest to find out if a file is xml and if it is encrypted.
    The parameter is an encoded uri. However, the manifest contains paths. Therefore
    the path is encoded as uri, so they can be compared.
*/
bool DigitalSignaturesDialog::isXML(const rtl::OUString& rURI )
{
    OSL_ASSERT(mxStore.is());

    bool bIsXML = false;
    bool bPropsAvailable = false;
    const OUString sPropFullPath(RTL_CONSTASCII_USTRINGPARAM("FullPath"));
    const OUString sPropMediaType(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
    const OUString sPropDigest(RTL_CONSTASCII_USTRINGPARAM("Digest"));

    for (int i = 0; i < m_manifest.getLength(); i++)
    {
        Any digest;
        const Sequence< css::beans::PropertyValue >& entry = m_manifest[i];
        OUString sPath, sMediaType;
        bool bEncrypted = false;
        for (int j = 0; j < entry.getLength(); j++)
        {
            const css::beans::PropertyValue & prop = entry[j];

            if (prop.Name.equals( sPropFullPath ) )
                prop.Value >>= sPath;
            else if (prop.Name.equals( sPropMediaType ) )
                prop.Value >>= sMediaType;
            else if (prop.Name.equals( sPropDigest ) )
                bEncrypted = true;
        }
        if (DocumentSignatureHelper::equalsReferenceUriManifestPath(rURI, sPath))
        {
            bIsXML = sMediaType.equals(OUSTR("text/xml")) && ! bEncrypted;
            bPropsAvailable = true;
            break;
        }
    }
    if (!bPropsAvailable)
    {
        //This would be the case for at least mimetype, META-INF/manifest.xml
        //META-INF/macrosignatures.xml.
        //Files can only be encrypted if they are in the manifest.xml.
        //That is, the current file cannot be encrypted, otherwise bPropsAvailable
        //would be true.
        OUString aXMLExt( RTL_CONSTASCII_USTRINGPARAM( "XML" ) );
        sal_Int32 nSep = rURI.lastIndexOf( '.' );
        if ( nSep != (-1) )
        {
            OUString aExt = rURI.copy( nSep+1 );
            if (aExt.equalsIgnoreAsciiCase(aXMLExt ))
                bIsXML = true;
        }
     }
    return bIsXML;
}

DigitalSignaturesDialog::DigitalSignaturesDialog(
    Window* pParent,
    uno::Reference< uno::XComponentContext >& rxCtx, DocumentSignatureMode eMode,
    sal_Bool bReadOnly, const ::rtl::OUString& sODFVersion, bool bHasDocumentSignature)
    :ModalDialog        ( pParent, XMLSEC_RES( RID_XMLSECDLG_DIGSIG ) )
    ,mxCtx              ( rxCtx )
    ,maSignatureHelper  ( rxCtx )
    ,meSignatureMode    ( eMode )
    ,maHintDocFT        ( this, XMLSEC_RES( FT_HINT_DOC ) )
    ,maHintBasicFT      ( this, XMLSEC_RES( FT_HINT_BASIC ) )
    ,maHintPackageFT    ( this, XMLSEC_RES( FT_HINT_PACK ) )
    ,maSignaturesLBContainer(this, XMLSEC_RES(LB_SIGNATURES))
    ,maSignaturesLB(maSignaturesLBContainer)
    ,maSigsValidImg     ( this, XMLSEC_RES( IMG_STATE_VALID ) )
    ,maSigsValidFI      ( this, XMLSEC_RES( FI_STATE_VALID ) )
    ,maSigsInvalidImg   ( this, XMLSEC_RES( IMG_STATE_BROKEN ) )
    ,maSigsInvalidFI    ( this, XMLSEC_RES( FI_STATE_BROKEN ) )
    ,maSigsNotvalidatedImg( this, XMLSEC_RES( IMG_STATE_NOTVALIDATED ) )
    ,maSigsNotvalidatedFI ( this, XMLSEC_RES( FI_STATE_NOTVALIDATED ) )
    ,maSigsOldSignatureFI ( this, XMLSEC_RES( FI_STATE_OLDSIGNATURE) )
    ,maViewBtn          ( this, XMLSEC_RES( BTN_VIEWCERT ) )
    ,maAddBtn           ( this, XMLSEC_RES( BTN_ADDCERT ) )
    ,maRemoveBtn        ( this, XMLSEC_RES( BTN_REMOVECERT ) )
    ,maBottomSepFL      ( this, XMLSEC_RES( FL_BOTTOM_SEP ) )
    ,maOKBtn            ( this, XMLSEC_RES( BTN_OK ) )
    ,maHelpBtn          ( this, XMLSEC_RES( BTN_HELP ) )
    ,m_sODFVersion (sODFVersion)
    ,m_bHasDocumentSignature(bHasDocumentSignature)
    ,m_bWarningShowSignMacro(false)
{
    // #i48253# the tablistbox needs its own unique id
    maSignaturesLB.Window::SetUniqueId( HID_XMLSEC_TREE_SIGNATURESDLG );
    Size aControlSize( maSignaturesLB.GetSizePixel() );
    aControlSize = maSignaturesLB.PixelToLogic( aControlSize, MapMode( MAP_APPFONT ) );
    const long nControlWidth = aControlSize.Width();
    static long nTabs[] = { 4, 0, 6*nControlWidth/100, 36*nControlWidth/100, 74*nControlWidth/100 };
    maSignaturesLB.SetTabs( &nTabs[ 0 ] );
    maSignaturesLB.InsertHeaderEntry( XMLSEC_RES( STR_HEADERBAR ) );

    maSigsNotvalidatedFI.SetText( XMLSEC_RES( STR_NO_INFO_TO_VERIFY ) );

    FreeResource();

    mbVerifySignatures = true;
    mbSignaturesChanged = false;

    maSignaturesLB.SetSelectHdl( LINK( this, DigitalSignaturesDialog, SignatureHighlightHdl ) );
    maSignaturesLB.SetDoubleClickHdl( LINK( this, DigitalSignaturesDialog, SignatureSelectHdl ) );

    maViewBtn.SetClickHdl( LINK( this, DigitalSignaturesDialog, ViewButtonHdl ) );
    maViewBtn.Disable();

    maAddBtn.SetClickHdl( LINK( this, DigitalSignaturesDialog, AddButtonHdl ) );
    if ( bReadOnly  )
        maAddBtn.Disable();

    maRemoveBtn.SetClickHdl( LINK( this, DigitalSignaturesDialog, RemoveButtonHdl ) );
    maRemoveBtn.Disable();

    maOKBtn.SetClickHdl( LINK( this, DigitalSignaturesDialog, OKButtonHdl) );

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
    XmlSec::AlignAndFitImageAndControl( maSigsNotvalidatedImg, maSigsOldSignatureFI, 5 );
}

DigitalSignaturesDialog::~DigitalSignaturesDialog()
{
}

sal_Bool DigitalSignaturesDialog::Init()
{
    bool bInit = maSignatureHelper.Init();

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
    maSignatureHelper.SetStorage( mxStore, m_sODFVersion);

    Reference < css::packages::manifest::XManifestReader > xReader =
        css::packages::manifest::ManifestReader::create(mxCtx);

    //Get the manifest.xml
    Reference < css::embed::XStorage > xSubStore(rxStore->openStorageElement(
                OUSTR("META-INF"), css::embed::ElementModes::READ), UNO_QUERY_THROW);

    Reference< css::io::XInputStream > xStream(
        xSubStore->openStreamElement(OUSTR("manifest.xml"), css::embed::ElementModes::READ),
        UNO_QUERY_THROW);

    m_manifest = xReader->readManifestSequence(xStream);
}

void DigitalSignaturesDialog::SetSignatureStream( const cssu::Reference < css::io::XStream >& rxStream )
{
    mxSignatureStream = rxStream;
}

bool DigitalSignaturesDialog::canAddRemove()
{
    //m56
    bool ret = true;
    OSL_ASSERT(mxStore.is());
    bool bDoc1_1 = DocumentSignatureHelper::isODFPre_1_2(m_sODFVersion);
    SaveODFItem item;
    bool bSave1_1 = item.isLessODF1_2();

    // see specification
    //cvs: specs/www/appwide/security/Electronic_Signatures_and_Security.sxw
    //Paragraph 'Behavior with regard to ODF 1.2'
    //For both, macro and document
    if ( (!bSave1_1  && bDoc1_1) || (bSave1_1 && bDoc1_1) )
    {
        //#4
        ErrorBox err(NULL, XMLSEC_RES(RID_XMLSECDLG_OLD_ODF_FORMAT));
        err.Execute();
        ret = false;
    }

    //As of OOo 3.2 the document signature includes in macrosignatures.xml. That is
    //adding a macro signature will break an existing document signature.
    //The sfx2 will remove the documentsignature when the user adds a macro signature
    if (meSignatureMode == SignatureModeMacros
        && ret)
    {
        if (m_bHasDocumentSignature && !m_bWarningShowSignMacro)
        {
            //The warning says that the document signatures will be removed if the user
            //continues. He can then either press 'OK' or 'NO'
            //It the user presses 'Add' or 'Remove' several times then, then the warning
            //is shown every time until the user presses 'OK'. From then on, the warning
            //is not displayed anymore as long as the signatures dialog is alive.
            if (QueryBox(
                NULL, XMLSEC_RES(MSG_XMLSECDLG_QUERY_REMOVEDOCSIGNBEFORESIGN)).Execute() == RET_NO)
                ret = false;
            else
                m_bWarningShowSignMacro = true;

        }
    }
    return ret;
}

bool DigitalSignaturesDialog::canAdd()
{
    if (canAddRemove())
        return true;
    return false;
}

bool DigitalSignaturesDialog::canRemove()
{
    if (canAddRemove())
        return true;
    return false;
}

short DigitalSignaturesDialog::Execute()
{
    // Verify Signatures and add certificates to ListBox...
    mbVerifySignatures = true;
    ImplGetSignatureInformations(false);
    ImplFillSignaturesBox();

    // Only verify once, content will not change.
    // But for refreshing signature information, StartVerifySignatureHdl will be called after each add/remove
    mbVerifySignatures = false;

    return Dialog::Execute();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, SignatureHighlightHdl)
{
    bool bSel = maSignaturesLB.FirstSelected() ? true : false;
    maViewBtn.Enable( bSel );
    if ( maAddBtn.IsEnabled() ) // not read only
        maRemoveBtn.Enable( bSel );

    return 0;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, OKButtonHdl)
{
    // Export all other signatures...
    SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream(
        embed::ElementModes::WRITE|embed::ElementModes::TRUNCATE, false );
    uno::Reference< io::XOutputStream > xOutputStream(
        aStreamHelper.xSignatureStream, uno::UNO_QUERY );
    uno::Reference< com::sun::star::xml::sax::XWriter> xSaxWriter =
        maSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream );

    uno::Reference< xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter, UNO_QUERY_THROW);
    size_t nInfos = maCurrentSignatureInformations.size();
    for( size_t n = 0 ; n < nInfos ; ++n )
        maSignatureHelper.ExportSignature(
        xDocumentHandler, maCurrentSignatureInformations[ n ] );

    maSignatureHelper.CloseDocumentHandler( xDocumentHandler);

    // If stream was not provided, we are responsible for committing it....
    if ( !mxSignatureStream.is() )
    {
        uno::Reference< embed::XTransactedObject > xTrans(
            aStreamHelper.xSignatureStorage, uno::UNO_QUERY );
        xTrans->commit();
    }

    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, SignatureSelectHdl)
{
    ImplShowSignaturesDetails();
    return 0;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, ViewButtonHdl)
{
    ImplShowSignaturesDetails();
    return 0;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, AddButtonHdl)
{
    if( ! canAdd())
        return 0;
    try
    {
        uno::Reference<com::sun::star::xml::crypto::XSecurityEnvironment> xSecEnv = maSignatureHelper.GetSecurityEnvironment();

        uno::Reference<com::sun::star::security::XSerialNumberAdapter> xSerialNumberAdapter =
            ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);
        CertificateChooser aChooser( this, mxCtx, xSecEnv, maCurrentSignatureInformations );
        if ( aChooser.Execute() == RET_OK )
        {
            uno::Reference< ::com::sun::star::security::XCertificate > xCert = aChooser.GetSelectedCertificate();
            if ( !xCert.is() )
            {
                SAL_WARN( "xmlsecurity.dialogs", "no certificate selected" );
                return -1;
            }
            rtl::OUString aCertSerial = xSerialNumberAdapter->toString( xCert->getSerialNumber() );
            if ( aCertSerial.isEmpty() )
            {
                OSL_FAIL( "Error in Certificate, problem with serial number!" );
                return -1;
            }

            maSignatureHelper.StartMission();

            sal_Int32 nSecurityId = maSignatureHelper.GetNewSecurityId();

            rtl::OUStringBuffer aStrBuffer;
            ::sax::Converter::encodeBase64(aStrBuffer, xCert->getEncoded());

            maSignatureHelper.SetX509Certificate( nSecurityId,
                xCert->getIssuerName(), aCertSerial,
                aStrBuffer.makeStringAndClear());

            std::vector< rtl::OUString > aElements =
                DocumentSignatureHelper::CreateElementList(
                    mxStore, rtl::OUString(), meSignatureMode, OOo3_2Document);

            sal_Int32 nElements = aElements.size();
            for ( sal_Int32 n = 0; n < nElements; n++ )
            {
                bool bBinaryMode = !isXML(aElements[n]);
                maSignatureHelper.AddForSigning( nSecurityId, aElements[n], aElements[n], bBinaryMode );
            }

            maSignatureHelper.SetDateTime( nSecurityId, Date( Date::SYSTEM ), Time( Time::SYSTEM ) );

            // We open a signature stream in which the existing and the new
            //signature is written. ImplGetSignatureInformation (later in this function) will
            //then read the stream an will fill  maCurrentSignatureInformations. The final signature
            //is written when the user presses OK. Then only maCurrentSignatureInformation and
            //a sax writer are used to write the information.
            SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream(
                css::embed::ElementModes::WRITE|css::embed::ElementModes::TRUNCATE, true);
            Reference< css::io::XOutputStream > xOutputStream(
                aStreamHelper.xSignatureStream, UNO_QUERY_THROW);
            Reference< css::xml::sax::XWriter> xSaxWriter =
                maSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream );

            // Export old signatures...
            uno::Reference< xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter, UNO_QUERY_THROW);
            size_t nInfos = maCurrentSignatureInformations.size();
            for ( size_t n = 0; n < nInfos; n++ )
                maSignatureHelper.ExportSignature( xDocumentHandler, maCurrentSignatureInformations[n]);

            // Create a new one...
            maSignatureHelper.CreateAndWriteSignature( xDocumentHandler );

            // That's it...
            maSignatureHelper.CloseDocumentHandler( xDocumentHandler);

            maSignatureHelper.EndMission();

            aStreamHelper = SignatureStreamHelper();    // release objects...

            mbSignaturesChanged = true;

            sal_Int32 nStatus = maSignatureHelper.GetSignatureInformation( nSecurityId ).nStatus;

            if ( nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
            {
                mbSignaturesChanged = true;

                // Can't simply remember current information, need parsing for getting full information :(
                // We need to verify the signatures again, otherwise the status in the signature information
                // will not contain
                // SecurityOperationStatus_OPERATION_SUCCEEDED
                mbVerifySignatures = true;
                ImplGetSignatureInformations(true);
                ImplFillSignaturesBox();
            }
        }
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "Exception while adding a signature!" );
        // Don't keep invalid entries...
        ImplGetSignatureInformations(true);
        ImplFillSignaturesBox();
    }

    return 0;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, RemoveButtonHdl)
{
    if (!canRemove())
        return 0;
    if( maSignaturesLB.FirstSelected() )
    {
        try
        {
            sal_uInt16 nSelected = (sal_uInt16) (sal_uIntPtr) maSignaturesLB.FirstSelected()->GetUserData();
            maCurrentSignatureInformations.erase( maCurrentSignatureInformations.begin()+nSelected );

            // Export all other signatures...
            SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream(
                css::embed::ElementModes::WRITE | css::embed::ElementModes::TRUNCATE, true);
            Reference< css::io::XOutputStream > xOutputStream(
                aStreamHelper.xSignatureStream, UNO_QUERY_THROW);
            Reference< css::xml::sax::XWriter> xSaxWriter =
                maSignatureHelper.CreateDocumentHandlerWithHeader( xOutputStream );

            uno::Reference< xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter, UNO_QUERY_THROW);
            size_t nInfos = maCurrentSignatureInformations.size();
            for( size_t n = 0 ; n < nInfos ; ++n )
                maSignatureHelper.ExportSignature( xDocumentHandler, maCurrentSignatureInformations[ n ] );

            maSignatureHelper.CloseDocumentHandler( xDocumentHandler);

            mbSignaturesChanged = true;

            aStreamHelper = SignatureStreamHelper();    // release objects...

            ImplFillSignaturesBox();
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "Exception while removing a signature!" );
            // Don't keep invalid entries...
            ImplGetSignatureInformations(true);
            ImplFillSignaturesBox();
        }
    }

    return 0;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, StartVerifySignatureHdl)
{
    return mbVerifySignatures ? 1 : 0;
}

void DigitalSignaturesDialog::ImplFillSignaturesBox()
{
    maSignaturesLB.Clear();

    uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecEnv = maSignatureHelper.GetSecurityEnvironment();
    uno::Reference<com::sun::star::security::XSerialNumberAdapter> xSerialNumberAdapter =
        ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

    uno::Reference< ::com::sun::star::security::XCertificate > xCert;

    size_t nInfos = maCurrentSignatureInformations.size();
    size_t nValidSigs = 0, nValidCerts = 0;
    bool bAllNewSignatures = true;

    if( nInfos )
    {
        for( size_t n = 0; n < nInfos; ++n )
        {
            DocumentSignatureAlgorithm mode = DocumentSignatureHelper::getDocumentAlgorithm(
                m_sODFVersion, maCurrentSignatureInformations[n]);
            std::vector< rtl::OUString > aElementsToBeVerified =
                DocumentSignatureHelper::CreateElementList(
                mxStore, ::rtl::OUString(), meSignatureMode, mode);

            const SignatureInformation& rInfo = maCurrentSignatureInformations[n];
            //First we try to get the certificate which is embedded in the XML Signature
            if (!rInfo.ouX509Certificate.isEmpty())
                xCert = xSecEnv->createCertificateFromAscii(rInfo.ouX509Certificate);
            else {
                //There must be an embedded certificate because we use it to get the
                //issuer name. We cannot use /Signature/KeyInfo/X509Data/X509IssuerName
                //because it could be modified by an attacker. The issuer is displayed
                //in the digital signature dialog.
                //Comparing the X509IssuerName with the one from the X509Certificate in order
                //to find out if the X509IssuerName was modified does not work. See #i62684
                DBG_ASSERT(sal_False, "Could not find embedded certificate!");
            }

            //In case there is no embedded certificate we try to get it from a local store
            //Todo: This probably could be removed, see above.
            if (!xCert.is())
                xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, xSerialNumberAdapter->toSequence( rInfo.ouX509SerialNumber ) );

            DBG_ASSERT( xCert.is(), "Certificate not found and can't be created!" );

            OUString aSubject;
            OUString aIssuer;
            OUString aDateTimeStr;

            bool bSigValid = false;
            bool bCertValid = false;
            if( xCert.is() )
            {
                //check the validity of the cert
                try {
                    sal_Int32 certResult = xSecEnv->verifyCertificate(xCert,
                        Sequence<css::uno::Reference<css::security::XCertificate> >());

                    bCertValid = certResult == css::security::CertificateValidity::VALID ? true : false;
                    if ( bCertValid )
                        nValidCerts++;

                } catch (css::uno::SecurityException& ) {
                    OSL_FAIL("Verification of certificate failed");
                    bCertValid = false;
                }

                aSubject = XmlSec::GetContentPart( xCert->getSubjectName() );
                aIssuer = XmlSec::GetContentPart( xCert->getIssuerName() );
                // String with date and time information (#i20172#)
                aDateTimeStr = XmlSec::GetDateTimeString( rInfo.stDateTime );
            }
            bSigValid = ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED );

            if ( bSigValid )
            {
                 bSigValid = DocumentSignatureHelper::checkIfAllFilesAreSigned(
                      aElementsToBeVerified, rInfo, mode);

                if( bSigValid )
                    nValidSigs++;
            }

            Image aImage;
            if (!bSigValid)
            {
                aImage = maSigsInvalidImg.GetImage();
            }
            else if (bSigValid && !bCertValid)
            {
                aImage = maSigsNotvalidatedImg.GetImage();
            }
            //Check if the signature is a "old" document signature, that is, which was created
            //by an version of OOo previous to 3.2
            else if (meSignatureMode == SignatureModeDocumentContent
                && bSigValid && bCertValid && !DocumentSignatureHelper::isOOo3_2_Signature(
                maCurrentSignatureInformations[n]))
            {
                aImage = maSigsNotvalidatedImg.GetImage();
                bAllNewSignatures &= false;
            }
            else if (meSignatureMode == SignatureModeDocumentContent
                && bSigValid && bCertValid && DocumentSignatureHelper::isOOo3_2_Signature(
                maCurrentSignatureInformations[n]))
            {
                aImage = maSigsValidImg.GetImage();
            }
            else if (meSignatureMode == SignatureModeMacros
                && bSigValid && bCertValid)
            {
                aImage = maSigsValidImg.GetImage();
            }

            SvTreeListEntry* pEntry = maSignaturesLB.InsertEntry( OUString(), aImage, aImage );
            maSignaturesLB.SetEntryText( aSubject, pEntry, 1 );
            maSignaturesLB.SetEntryText( aIssuer, pEntry, 2 );
            maSignaturesLB.SetEntryText( aDateTimeStr, pEntry, 3 );
            pEntry->SetUserData( ( void* ) n );     // missuse user data as index
        }
    }

    bool bAllSigsValid = (nValidSigs == nInfos);
    bool bAllCertsValid = (nValidCerts == nInfos);
    bool bShowValidState = nInfos && (bAllSigsValid && bAllCertsValid && bAllNewSignatures);

    bool bShowNotValidatedState = nInfos && (bAllSigsValid && (!bAllCertsValid || !bAllNewSignatures));
    bool bShowInvalidState = nInfos && !bAllSigsValid;

    maSigsValidImg.Show( bShowValidState);
    maSigsValidFI.Show( bShowValidState );
    maSigsInvalidImg.Show( bShowInvalidState );
    maSigsInvalidFI.Show( bShowInvalidState );

    maSigsNotvalidatedImg.Show(bShowNotValidatedState);
    //bAllNewSignatures is always true if we are not in document mode
    maSigsNotvalidatedFI.Show(nInfos && bAllSigsValid && ! bAllCertsValid);
    maSigsOldSignatureFI.Show(nInfos && bAllSigsValid && bAllCertsValid && !bAllNewSignatures);

    SignatureHighlightHdl( NULL );
}


//If bUseTempStream is true then the temporary signature stream is used.
//Otherwise the real signature stream is used.
void DigitalSignaturesDialog::ImplGetSignatureInformations(bool bUseTempStream)
{
    maCurrentSignatureInformations.clear();

    maSignatureHelper.StartMission();

    SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream(
        css::embed::ElementModes::READ, bUseTempStream);
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
        sal_uInt16 nSelected = (sal_uInt16) (sal_uIntPtr) maSignaturesLB.FirstSelected()->GetUserData();
        const SignatureInformation& rInfo = maCurrentSignatureInformations[ nSelected ];
        css::uno::Reference<css::xml::crypto::XSecurityEnvironment > xSecEnv =
            maSignatureHelper.GetSecurityEnvironment();
        css::uno::Reference<com::sun::star::security::XSerialNumberAdapter> xSerialNumberAdapter =
            ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);
        // Use Certificate from doc, not from key store
        uno::Reference< dcss::security::XCertificate > xCert;
        if (!rInfo.ouX509Certificate.isEmpty())
            xCert = xSecEnv->createCertificateFromAscii(rInfo.ouX509Certificate);
        //fallback if no certificate is embedded, get if from store
        if (!xCert.is())
            xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, xSerialNumberAdapter->toSequence( rInfo.ouX509SerialNumber ) );

        DBG_ASSERT( xCert.is(), "Error getting cCertificate!" );
        if ( xCert.is() )
        {
            CertificateViewer aViewer( this, maSignatureHelper.GetSecurityEnvironment(), xCert, false );
            aViewer.Execute();
        }
    }
}

//If bTempStream is true, then a temporary stream is return. If it is false then, the actual
//signature stream is used.
//Everytime the user presses Add a new temporary stream is created.
//We keep the temporary stream as member because ImplGetSignatureInformations
//will later access the stream to create DocumentSignatureInformation objects
//which are stored in maCurrentSignatureInformations.
SignatureStreamHelper DigitalSignaturesDialog::ImplOpenSignatureStream(
    sal_Int32 nStreamOpenMode, bool bTempStream)
{
    SignatureStreamHelper aHelper;
    if (bTempStream)
    {
        if (nStreamOpenMode & css::embed::ElementModes::TRUNCATE)
        {
            //We write always into a new temporary stream.
            mxTempSignatureStream = Reference < css::io::XStream >(css::io::TempFile::create(mxCtx), UNO_QUERY_THROW);
            aHelper.xSignatureStream = mxTempSignatureStream;
        }
        else
        {
            //When we read from the temp stream, then we must have previously
            //created one.
            OSL_ASSERT(mxTempSignatureStream.is());
        }
        aHelper.xSignatureStream = mxTempSignatureStream;
    }
    else
    {
        //No temporary stream
        if (!mxSignatureStream.is())
        {
            //We may not have a dedicated stream for writing the signature
            //So we take one directly from the storage
            //Or DocumentDigitalSignatures::showDocumentContentSignatures was called,
            //in which case Add/Remove is not allowed. This is done, for example, if the
            //document is readonly
            aHelper = DocumentSignatureHelper::OpenSignatureStream(
                mxStore, nStreamOpenMode, meSignatureMode );
        }
        else
        {
            aHelper.xSignatureStream = mxSignatureStream;
        }
    }

    if (nStreamOpenMode & css::embed::ElementModes::TRUNCATE)
    {
        css::uno::Reference < css::io::XTruncate > xTruncate(
            aHelper.xSignatureStream, UNO_QUERY_THROW);
        DBG_ASSERT( xTruncate.is(), "ImplOpenSignatureStream - Stream does not support xTruncate!" );
        xTruncate->truncate();
    }
    else if ( bTempStream || mxSignatureStream.is())
    {
        //In case we read the signature stream from the storage directly,
        //which is the case when DocumentDigitalSignatures::showDocumentContentSignatures
        //then XSeakable is not supported
        css::uno::Reference < css::io::XSeekable > xSeek(
            aHelper.xSignatureStream, UNO_QUERY_THROW);
        DBG_ASSERT( xSeek.is(), "ImplOpenSignatureStream - Stream does not support xSeekable!" );
        xSeek->seek( 0 );
    }

    return aHelper;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
