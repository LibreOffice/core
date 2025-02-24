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

#include <oox/core/filterdetect.hxx>

#include <com/sun/star/io/XStream.hpp>
#include <comphelper/docpasswordhelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <oox/core/fastparser.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/zipstorage.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

#include <oox/crypto/DocumentDecryption.hxx>

#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <o3tl/string_view.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace oox::core {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uri;

using utl::MediaDescriptor;
using comphelper::IDocPasswordVerifier;
using comphelper::DocPasswordVerifierResult;

FilterDetectDocHandler::FilterDetectDocHandler( const  Reference< XComponentContext >& rxContext, OUString& rFilterName, OUString aFileName ) :
    mrFilterName( rFilterName ),
    maFileName(std::move(aFileName)),
    maOOXMLVariant( OOXMLVariant::ECMA_Transitional ),
    mxContext( rxContext )
{
    maContextStack.reserve( 2 );
}

FilterDetectDocHandler::~FilterDetectDocHandler()
{
}

void SAL_CALL FilterDetectDocHandler::startDocument()
{
}

void SAL_CALL FilterDetectDocHandler::endDocument()
{
}

void SAL_CALL FilterDetectDocHandler::processingInstruction( const OUString& /*rTarget*/, const OUString& /*rData*/ )
{
}

void SAL_CALL FilterDetectDocHandler::setDocumentLocator( const Reference<XLocator>& /*xLocator*/ )
{
}

void SAL_CALL FilterDetectDocHandler::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rAttribs )
{
    AttributeList aAttribs( rAttribs );
    switch ( nElement )
    {
        // cases for word/settings.xml
        case W_TOKEN(settings):
        case W_TOKEN(compat):
            break;
        case W_TOKEN(compatSetting):
            if (!maContextStack.empty() && (maContextStack.back() == W_TOKEN(compat)))
                parseSettings(aAttribs);
            break;

        // cases for _rels/.rels
        case PR_TOKEN( Relationships ):
        break;
        case PR_TOKEN( Relationship ):
            if( !maContextStack.empty() && (maContextStack.back() == PR_TOKEN( Relationships )) )
                parseRelationship( aAttribs );
        break;

        // cases for [Content_Types].xml
        case PC_TOKEN( Types ):
        break;
        case PC_TOKEN( Default ):
            if( !maContextStack.empty() && (maContextStack.back() == PC_TOKEN( Types )) )
                parseContentTypesDefault( aAttribs );
        break;
        case PC_TOKEN( Override ):
            if( !maContextStack.empty() && (maContextStack.back() == PC_TOKEN( Types )) )
                parseContentTypesOverride( aAttribs );
        break;
    }
    maContextStack.push_back( nElement );
}

void SAL_CALL FilterDetectDocHandler::startUnknownElement(
    const OUString& /*Namespace*/, const OUString& /*Name*/, const Reference<XFastAttributeList>& /*Attribs*/ )
{
}

void SAL_CALL FilterDetectDocHandler::endFastElement( sal_Int32 /*nElement*/ )
{
    maContextStack.pop_back();
}

void SAL_CALL FilterDetectDocHandler::endUnknownElement(
    const OUString& /*Namespace*/, const OUString& /*Name*/ )
{
}

Reference<XFastContextHandler> SAL_CALL FilterDetectDocHandler::createFastChildContext(
    sal_Int32 /*Element*/, const Reference<XFastAttributeList>& /*Attribs*/ )
{
    return this;
}

Reference<XFastContextHandler> SAL_CALL FilterDetectDocHandler::createUnknownChildContext(
    const OUString& /*Namespace*/, const OUString& /*Name*/, const Reference<XFastAttributeList>& /*Attribs*/)
{
    return this;
}

void SAL_CALL FilterDetectDocHandler::characters( const OUString& /*aChars*/ )
{
}

void FilterDetectDocHandler::parseSettings(const AttributeList& rAttribs)
{
    // tdf#131936 Remember filter when opening file as 'Office Open XML Text'
    if (rAttribs.getStringDefaulted(W_TOKEN(name)).equalsIgnoreAsciiCase("compatibilityMode"))
    {
        const sal_Int32 nVal = rAttribs.getInteger(W_TOKEN(val), 12); // default to Word 2007
        // if specified multiple times, highest value wins
        if (nVal > 12 && maOOXMLVariant == OOXMLVariant::ECMA_Transitional)
            maOOXMLVariant = OOXMLVariant::ISO_Transitional; // Word 2010+
    }
}

void FilterDetectDocHandler::parseRelationship( const AttributeList& rAttribs )
{
    OUString aType = rAttribs.getStringDefaulted( XML_Type);

    // tdf#131936 Remember filter when opening file as 'Office Open XML Text'
    if (aType.startsWithIgnoreAsciiCase("http://schemas.openxmlformats.org/officedocument/2006/relationships/metadata/core-properties"))
        maOOXMLVariant = OOXMLVariant::ISO_Transitional;
    else if (aType.startsWithIgnoreAsciiCase("http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"))
        maOOXMLVariant = OOXMLVariant::ECMA_Transitional;
    else if (aType.startsWithIgnoreAsciiCase("http://purl.oclc.org/ooxml/officeDocument"))
        maOOXMLVariant = OOXMLVariant::ISO_Strict;

    if ( aType != "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" // OOXML Transitional
          && aType != "http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument" ) //OOXML strict
        return;

    Reference<XUriReferenceFactory> xFactory = UriReferenceFactory::create( mxContext );
    try
    {
         // use '/' to represent the root of the zip package ( and provide a 'file' scheme to
         // keep the XUriReference implementation happy )
         Reference< XUriReference > xBase = xFactory->parse( u"file:///"_ustr );

         Reference< XUriReference > xPart = xFactory->parse(  rAttribs.getStringDefaulted( XML_Target) );
         Reference< XUriReference > xAbs = xFactory->makeAbsolute(  xBase, xPart, true, RelativeUriExcessParentSegments_RETAIN );

         if ( xAbs.is() )
             maTargetPath = xAbs->getPath();
    }
    catch( const Exception& )
    {
    }
}

OUString FilterDetectDocHandler::getFilterNameFromContentType( std::u16string_view rContentType, std::u16string_view rFileName )
{
    bool bDocm = o3tl::endsWithIgnoreAsciiCase(rFileName, ".docm");

    if( rContentType == u"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml" && !bDocm )
    {
        switch (maOOXMLVariant)
        {
            case OOXMLVariant::ISO_Transitional:
            case OOXMLVariant::ISO_Strict: // Not supported, map to ISO transitional
                return u"writer_OOXML"_ustr;
            case OOXMLVariant::ECMA_Transitional:
                return u"writer_MS_Word_2007"_ustr;
        }
    }

    if( rContentType == u"application/vnd.ms-word.document.macroEnabled.main+xml" || bDocm )
        return u"writer_MS_Word_2007_VBA"_ustr;

    if( rContentType == u"application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml" ||
        rContentType == u"application/vnd.ms-word.template.macroEnabledTemplate.main+xml" )
    {
        switch (maOOXMLVariant)
        {
            case OOXMLVariant::ISO_Transitional:
            case OOXMLVariant::ISO_Strict: // Not supported, map to ISO transitional
                return u"writer_OOXML_Text_Template"_ustr;
            case OOXMLVariant::ECMA_Transitional:
                return u"writer_MS_Word_2007_Template"_ustr;
        }
    }

    if( rContentType == u"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml")
        return u"MS Excel 2007 XML"_ustr;

    if (rContentType == u"application/vnd.ms-excel.sheet.macroEnabled.main+xml")
        return u"MS Excel 2007 VBA XML"_ustr;

    if( rContentType == u"application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml" ||
        rContentType == u"application/vnd.ms-excel.template.macroEnabled.main+xml" )
        return u"MS Excel 2007 XML Template"_ustr;

    if ( rContentType == u"application/vnd.ms-excel.sheet.binary.macroEnabled.main" )
        return u"MS Excel 2007 Binary"_ustr;

    if (rContentType == u"application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml")
        return u"MS PowerPoint 2007 XML"_ustr;

    if (rContentType == u"application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml")
        return u"MS PowerPoint 2007 XML VBA"_ustr;

    if( rContentType == u"application/vnd.openxmlformats-officedocument.presentationml.slideshow.main+xml" ||
        rContentType == u"application/vnd.ms-powerpoint.slideshow.macroEnabled.main+xml" )
        return u"MS PowerPoint 2007 XML AutoPlay"_ustr;

    if( rContentType == u"application/vnd.openxmlformats-officedocument.presentationml.template.main+xml" ||
        rContentType == u"application/vnd.ms-powerpoint.template.macroEnabled.main+xml" )
        return u"MS PowerPoint 2007 XML Template"_ustr;

    return OUString();
}

void FilterDetectDocHandler::parseContentTypesDefault( const AttributeList& rAttribs )
{
    // only if no overridden part name found
    if( mrFilterName.isEmpty() )
    {
        // check if target path ends with extension
        OUString aExtension = rAttribs.getStringDefaulted( XML_Extension);
        sal_Int32 nExtPos = maTargetPath.getLength() - aExtension.getLength();
        if( (nExtPos > 0) && (maTargetPath[ nExtPos - 1 ] == '.') && maTargetPath.match( aExtension, nExtPos ) )
            mrFilterName = getFilterNameFromContentType( rAttribs.getStringDefaulted( XML_ContentType), maFileName );
    }
}

void FilterDetectDocHandler::parseContentTypesOverride( const AttributeList& rAttribs )
{
    if( rAttribs.getStringDefaulted( XML_PartName) == maTargetPath )
        mrFilterName = getFilterNameFromContentType( rAttribs.getStringDefaulted( XML_ContentType), maFileName );
}

FilterDetect::FilterDetect( const Reference< XComponentContext >& rxContext ) :
    mxContext( rxContext, UNO_SET_THROW )
{
}

FilterDetect::~FilterDetect()
{
}

namespace
{

bool lclIsZipPackage( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm, bool bRepairPackage )
{
    ZipStorage aZipStorage(rxContext, rxInStrm, bRepairPackage);
    return aZipStorage.isStorage();
}

class PasswordVerifier : public IDocPasswordVerifier
{
public:
    explicit PasswordVerifier( crypto::DocumentDecryption& aDecryptor );

    virtual DocPasswordVerifierResult verifyPassword( const OUString& rPassword, Sequence<NamedValue>& rEncryptionData ) override;

    virtual DocPasswordVerifierResult verifyEncryptionData( const Sequence<NamedValue>& rEncryptionData ) override;
private:
    crypto::DocumentDecryption& mDecryptor;
};

PasswordVerifier::PasswordVerifier( crypto::DocumentDecryption& aDecryptor ) :
    mDecryptor(aDecryptor)
{}

comphelper::DocPasswordVerifierResult PasswordVerifier::verifyPassword( const OUString& rPassword, Sequence<NamedValue>& rEncryptionData )
{
    try
    {
        if (mDecryptor.generateEncryptionKey(rPassword))
            rEncryptionData = mDecryptor.createEncryptionData(rPassword);
    }
    catch (...)
    {
        // Any exception is a reason to abort
        return comphelper::DocPasswordVerifierResult::Abort;
    }

    return rEncryptionData.hasElements() ? comphelper::DocPasswordVerifierResult::OK : comphelper::DocPasswordVerifierResult::WrongPassword;
}

comphelper::DocPasswordVerifierResult PasswordVerifier::verifyEncryptionData( const Sequence<NamedValue>&  )
{
    return comphelper::DocPasswordVerifierResult::WrongPassword;
}

} // namespace

Reference< XInputStream > FilterDetect::extractUnencryptedPackage( MediaDescriptor& rMediaDescriptor ) const
{
    const bool bRepairPackage(rMediaDescriptor.getUnpackedValueOrDefault(u"RepairPackage"_ustr, false));
    // try the plain input stream
    Reference<XInputStream> xInputStream( rMediaDescriptor[ MediaDescriptor::PROP_INPUTSTREAM ], UNO_QUERY );
    if (!xInputStream.is() || lclIsZipPackage(mxContext, xInputStream, bRepairPackage))
        return xInputStream;

    // check if a temporary file is passed in the 'ComponentData' property
    Reference<XStream> xDecrypted( rMediaDescriptor.getComponentDataEntry( u"DecryptedPackage"_ustr ), UNO_QUERY );
    if( xDecrypted.is() )
    {
        Reference<XInputStream> xDecryptedInputStream = xDecrypted->getInputStream();
        if (lclIsZipPackage(mxContext, xDecryptedInputStream, bRepairPackage))
            return xDecryptedInputStream;
    }

    // try to decrypt an encrypted OLE package
    oox::ole::OleStorage aOleStorage( mxContext, xInputStream, false );
    if( aOleStorage.isStorage() )
    {
        try
        {
            crypto::DocumentDecryption aDecryptor(mxContext, aOleStorage);

            if( aDecryptor.readEncryptionInfo() )
            {
                /*  "VelvetSweatshop" is the built-in default encryption
                    password used by MS Excel for the "workbook protection"
                    feature with password. Try this first before prompting the
                    user for a password. */
                std::vector<OUString> aDefaultPasswords;
                aDefaultPasswords.emplace_back("VelvetSweatshop");

                /*  Use the comphelper password helper to request a password.
                    This helper returns either with the correct password
                    (according to the verifier), or with an empty string if
                    user has cancelled the password input dialog. */
                PasswordVerifier aVerifier( aDecryptor );
                Sequence<NamedValue> aEncryptionData = rMediaDescriptor.requestAndVerifyDocPassword(
                                                aVerifier,
                                                comphelper::DocPasswordRequestType::MS,
                                                &aDefaultPasswords );

                if( !aEncryptionData.hasElements() )
                {
                    rMediaDescriptor[ MediaDescriptor::PROP_ABORTED ] <<= true;
                }
                else
                {
                    // create MemoryStream for unencrypted package - rather not put this in a tempfile
                    Reference<XStream> const xTempStream(
                        mxContext->getServiceManager()->createInstanceWithContext(
                            u"com.sun.star.comp.MemoryStream"_ustr, mxContext),
                        UNO_QUERY_THROW);

                    // if decryption was unsuccessful (corrupted file or any other reason)
                    if (!aDecryptor.decrypt(xTempStream))
                    {
                        rMediaDescriptor[ MediaDescriptor::PROP_ABORTED ] <<= true;
                    }
                    else
                    {
                        // store temp file in media descriptor to keep it alive
                        rMediaDescriptor.setComponentDataEntry( u"DecryptedPackage"_ustr, Any( xTempStream ) );

                        Reference<XInputStream> xDecryptedInputStream = xTempStream->getInputStream();
                        if (lclIsZipPackage(mxContext, xDecryptedInputStream, bRepairPackage))
                            return xDecryptedInputStream;
                    }
                }
            }
        }
        catch( const Exception& )
        {
        }
    }
    return Reference<XInputStream>();
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

OUString SAL_CALL FilterDetect::getImplementationName()
{
    return u"com.sun.star.comp.oox.FormatDetector"_ustr;
}

sal_Bool SAL_CALL FilterDetect::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ExtendedTypeDetection"_ustr };
}

// com.sun.star.document.XExtendedFilterDetection interface -------------------

OUString SAL_CALL FilterDetect::detect( Sequence< PropertyValue >& rMediaDescSeq )
{
    OUString aFilterName;
    MediaDescriptor aMediaDescriptor( rMediaDescSeq );

    try
    {
        aMediaDescriptor.addInputStream();

        /*  Get the unencrypted input stream. This may include creation of a
            temporary file that contains the decrypted package. This temporary
            file will be stored in the 'ComponentData' property of the media
            descriptor. */
        Reference< XInputStream > xInputStream( extractUnencryptedPackage( aMediaDescriptor ), UNO_SET_THROW );

        // stream must be a ZIP package
        ZipStorage aZipStorage(mxContext, xInputStream,
                               aMediaDescriptor.getUnpackedValueOrDefault(u"RepairPackage"_ustr, false));
        if( aZipStorage.isStorage() )
        {
            // create the fast parser, register the XML namespaces, set document handler
            FastParser aParser;
            aParser.registerNamespace( NMSP_packageRel );
            aParser.registerNamespace( NMSP_officeRel );
            aParser.registerNamespace( NMSP_packageContentTypes );
            aParser.registerNamespace(NMSP_doc); // for W_TOKEN

            OUString aFileName;
            aMediaDescriptor[utl::MediaDescriptor::PROP_URL] >>= aFileName;

            aParser.setDocumentHandler( new FilterDetectDocHandler( mxContext, aFilterName, aFileName ) );

            /*  Parse '_rels/.rels' to get the target path and '[Content_Types].xml'
                to determine the content type of the part at the target path. */
            aParser.parseStream( aZipStorage, u"_rels/.rels"_ustr );
            try
            {
                // Text documents can't use .rels to determine maOOXMLVariant. Use compatibilityMode
                 aParser.parseStream(aZipStorage, u"word/settings.xml"_ustr);
            }
            catch(const Exception&)
            {
                // not a MS Word text document, or file might not exist
            }
            // Order is critical: .rels and then settings.xml must be parsed before [Content_Types]
            aParser.parseStream( aZipStorage, u"[Content_Types].xml"_ustr );
        }
    }
    catch( const Exception& )
    {
        if ( aMediaDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_ABORTED, false ) )
            /*  The user chose to abort detection, e.g. by hitting 'Cancel' in the password input dialog,
                so we have to return non-empty type name to abort the detection loop. The loading code is
                supposed to check whether the "Aborted" flag is present in the descriptor, and to not attempt
                to actually load the file then.

                The returned type name is the one we got as an input, which typically was detected by the flat
                detection (i.e. by file extension), so normally that's the correct one. Also at this point we
                already know that the file is OLE encrypted package, so trying with other type detectors doesn't
                make much sense anyway.
            */
            aFilterName = aMediaDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_TYPENAME, OUString() );
    }

    // write back changed media descriptor members
    aMediaDescriptor >> rMediaDescSeq;
    return aFilterName;
}

} // namespace oox::core

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_oox_FormatDetector_get_implementation(uno::XComponentContext* pCtx,
                                                        uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new oox::core::FilterDetect(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
