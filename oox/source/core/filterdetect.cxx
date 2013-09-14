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

#include "oox/core/filterdetect.hxx"

#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/mediadescriptor.hxx>

#include "oox/core/fastparser.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/ole/olestorage.hxx"

#include "oox/crypto/DocumentDecryption.hxx"

#include <com/sun/star/uri/UriReferenceFactory.hpp>

namespace oox {
namespace core {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uri;

using comphelper::MediaDescriptor;
using comphelper::SequenceAsHashMap;
using comphelper::IDocPasswordVerifier;
using comphelper::DocPasswordVerifierResult;

FilterDetectDocHandler::FilterDetectDocHandler( const  Reference< XComponentContext >& rxContext, OUString& rFilterName ) :
    mrFilterName( rFilterName ),
    mxContext( rxContext )
{
    maContextStack.reserve( 2 );
}

FilterDetectDocHandler::~FilterDetectDocHandler()
{
}

void SAL_CALL FilterDetectDocHandler::startDocument()
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::endDocument()
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::setDocumentLocator( const Reference<XLocator>& /*xLocator*/ )
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rAttribs )
    throw (SAXException,RuntimeException)
{
    AttributeList aAttribs( rAttribs );
    switch ( nElement )
    {
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
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::endFastElement( sal_Int32 /*nElement*/ )
    throw (SAXException, RuntimeException)
{
    maContextStack.pop_back();
}

void SAL_CALL FilterDetectDocHandler::endUnknownElement(
    const OUString& /*Namespace*/, const OUString& /*Name*/ ) throw (SAXException, RuntimeException)
{
}

Reference<XFastContextHandler> SAL_CALL FilterDetectDocHandler::createFastChildContext(
    sal_Int32 /*Element*/, const Reference<XFastAttributeList>& /*Attribs*/ )
    throw (SAXException, RuntimeException)
{
    return this;
}

Reference<XFastContextHandler> SAL_CALL FilterDetectDocHandler::createUnknownChildContext(
    const OUString& /*Namespace*/, const OUString& /*Name*/, const Reference<XFastAttributeList>& /*Attribs*/)
    throw (SAXException, RuntimeException)
{
    return this;
}

void SAL_CALL FilterDetectDocHandler::characters( const OUString& /*aChars*/ )
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
    throw (SAXException, RuntimeException)
{
}

void FilterDetectDocHandler::parseRelationship( const AttributeList& rAttribs )
{
    OUString aType = rAttribs.getString( XML_Type, OUString() );
    if ( aType == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" )
    {
        Reference<XUriReferenceFactory> xFactory = UriReferenceFactory::create( mxContext );
        try
        {
             // use '/' to representent the root of the zip package ( and provide a 'file' scheme to
             // keep the XUriReference implementation happy )
             Reference< XUriReference > xBase = xFactory->parse( OUString("file:///") );

             Reference< XUriReference > xPart = xFactory->parse(  rAttribs.getString( XML_Target, OUString() ) );
             Reference< XUriReference > xAbs = xFactory->makeAbsolute(  xBase, xPart, sal_True, RelativeUriExcessParentSegments_RETAIN );

             if ( xAbs.is() )
                 maTargetPath = xAbs->getPath();
        }
        catch( const Exception& )
        {
        }
    }
}

OUString FilterDetectDocHandler::getFilterNameFromContentType( const OUString& rContentType ) const
{
    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml" ) ||
        rContentType.equalsAscii("application/vnd.ms-word.document.macroEnabled.main+xml" ) )
        return OUString( "writer_MS_Word_2007" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-word.template.macroEnabledTemplate.main+xml") )
        return OUString( "writer_MS_Word_2007_Template" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-excel.sheet.macroEnabled.main+xml") )
        return OUString( "MS Excel 2007 XML" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-excel.template.macroEnabled.main+xml") )
        return OUString( "MS Excel 2007 XML Template" );

    if ( rContentType == "application/vnd.ms-excel.sheet.binary.macroEnabled.main" )
        return OUString( "MS Excel 2007 Binary" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml") )
        return OUString( "MS PowerPoint 2007 XML" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.presentationml.slideshow.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-powerpoint.slideshow.macroEnabled.main+xml") )
        return OUString( "MS PowerPoint 2007 XML AutoPlay" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.presentationml.template.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-powerpoint.template.macroEnabled.main+xml") )
        return OUString( "MS PowerPoint 2007 XML Template" );

    return OUString();
}

void FilterDetectDocHandler::parseContentTypesDefault( const AttributeList& rAttribs )
{
    // only if no overridden part name found
    if( mrFilterName.isEmpty() )
    {
        // check if target path ends with extension
        OUString aExtension = rAttribs.getString( XML_Extension, OUString() );
        sal_Int32 nExtPos = maTargetPath.getLength() - aExtension.getLength();
        if( (nExtPos > 0) && (maTargetPath[ nExtPos - 1 ] == '.') && maTargetPath.match( aExtension, nExtPos ) )
            mrFilterName = getFilterNameFromContentType( rAttribs.getString( XML_ContentType, OUString() ) );
    }
}

void FilterDetectDocHandler::parseContentTypesOverride( const AttributeList& rAttribs )
{
    if( rAttribs.getString( XML_PartName, OUString() ).equals( maTargetPath ) )
        mrFilterName = getFilterNameFromContentType( rAttribs.getString( XML_ContentType, OUString() ) );
}

/* Helper for XServiceInfo */
Sequence< OUString > FilterDetect_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = "com.sun.star.frame.ExtendedTypeDetection";
    return aServiceNames;
}

/* Helper for XServiceInfo */
OUString FilterDetect_getImplementationName()
{
    return OUString( "com.sun.star.comp.oox.FormatDetector" );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL FilterDetect_createInstance( const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new FilterDetect( rxContext ) );
}

FilterDetect::FilterDetect( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    mxContext( rxContext, UNO_SET_THROW )
{
}

FilterDetect::~FilterDetect()
{
}

namespace
{

bool lclIsZipPackage( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm )
{
    ZipStorage aZipStorage( rxContext, rxInStrm );
    return aZipStorage.isStorage();
}

class PasswordVerifier : public IDocPasswordVerifier
{
public:
    explicit PasswordVerifier( DocumentDecryption& aDecryptor );

    virtual DocPasswordVerifierResult verifyPassword( const OUString& rPassword, Sequence<NamedValue>& rEncryptionData );

    virtual DocPasswordVerifierResult verifyEncryptionData( const Sequence<NamedValue>& rEncryptionData );
private:
    DocumentDecryption& mDecryptor;
};

PasswordVerifier::PasswordVerifier( DocumentDecryption& aDecryptor ) :
    mDecryptor(aDecryptor)
{}

comphelper::DocPasswordVerifierResult PasswordVerifier::verifyPassword( const OUString& rPassword, Sequence<NamedValue>& rEncryptionData )
{
    if(mDecryptor.generateEncryptionKey(rPassword))
        rEncryptionData = mDecryptor.createEncryptionData(rPassword);

    return rEncryptionData.hasElements() ? comphelper::DocPasswordVerifierResult_OK : comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

comphelper::DocPasswordVerifierResult PasswordVerifier::verifyEncryptionData( const Sequence<NamedValue>& rEncryptionData )
{
    comphelper::DocPasswordVerifierResult aResult = comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
    if (DocumentDecryption::checkEncryptionData(rEncryptionData))
        aResult = comphelper::DocPasswordVerifierResult_OK;
    return aResult;
}

} // namespace


Reference< XInputStream > FilterDetect::extractUnencryptedPackage( MediaDescriptor& rMediaDescriptor ) const
{
    // try the plain input stream
    Reference<XInputStream> xInputStream( rMediaDescriptor[ MediaDescriptor::PROP_INPUTSTREAM() ], UNO_QUERY );
    if( !xInputStream.is() || lclIsZipPackage( mxContext, xInputStream ) )
        return xInputStream;

    // check if a temporary file is passed in the 'ComponentData' property
    Reference<XStream> xDecrypted( rMediaDescriptor.getComponentDataEntry( "DecryptedPackage" ), UNO_QUERY );
    if( xDecrypted.is() )
    {
        Reference<XInputStream> xDecryptedInputStream = xDecrypted->getInputStream();
        if( lclIsZipPackage( mxContext, xDecryptedInputStream ) )
            return xDecryptedInputStream;
    }

    // try to decrypt an encrypted OLE package
    oox::ole::OleStorage aOleStorage( mxContext, xInputStream, false );
    if( aOleStorage.isStorage() )
    {
        try
        {
            DocumentDecryption aDecryptor(aOleStorage, mxContext);

            if( aDecryptor.readEncryptionInfo() )
            {
                /*  "VelvetSweatshop" is the built-in default encryption
                    password used by MS Excel for the "workbook protection"
                    feature with password. Try this first before prompting the
                    user for a password. */
                std::vector<OUString> aDefaultPasswords;
                aDefaultPasswords.push_back("VelvetSweatshop");

                /*  Use the comphelper password helper to request a password.
                    This helper returns either with the correct password
                    (according to the verifier), or with an empty string if
                    user has cancelled the password input dialog. */
                PasswordVerifier aVerifier( aDecryptor );
                Sequence<NamedValue> aEncryptionData;
                aEncryptionData = comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
                                                aVerifier, rMediaDescriptor,
                                                comphelper::DocPasswordRequestType_MS,
                                                &aDefaultPasswords );

                if( aEncryptionData.getLength() == 0 )
                {
                    rMediaDescriptor[ MediaDescriptor::PROP_ABORTED() ] <<= true;
                }
                else
                {
                    // create temporary file for unencrypted package
                    Reference<XStream> xTempFile( TempFile::create(mxContext), UNO_QUERY_THROW );
                    aDecryptor.decrypt( xTempFile );

                    // store temp file in media descriptor to keep it alive
                    rMediaDescriptor.setComponentDataEntry( "DecryptedPackage", Any( xTempFile ) );

                    Reference<XInputStream> xDecryptedInputStream = xTempFile->getInputStream();
                    if( lclIsZipPackage( mxContext, xDecryptedInputStream ) )
                        return xDecryptedInputStream;
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

OUString SAL_CALL FilterDetect::getImplementationName() throw( RuntimeException )
{
    return FilterDetect_getImplementationName();
}

sal_Bool SAL_CALL FilterDetect::supportsService( const OUString& rServiceName ) throw( RuntimeException )
{
    const Sequence< OUString > aServices = FilterDetect_getSupportedServiceNames();
    const OUString* pArray = aServices.getConstArray();
    const OUString* pArrayEnd = pArray + aServices.getLength();
    return ::std::find( pArray, pArrayEnd, rServiceName ) != pArrayEnd;
}

Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames() throw( RuntimeException )
{
    return FilterDetect_getSupportedServiceNames();
}

// com.sun.star.document.XExtendedFilterDetection interface -------------------

OUString SAL_CALL FilterDetect::detect( Sequence< PropertyValue >& rMediaDescSeq ) throw( RuntimeException )
{
    OUString aFilterName;
    MediaDescriptor aMediaDescriptor( rMediaDescSeq );

    /*  Check that the user has not choosen to abort detection, e.g. by hitting
        'Cancel' in the password input dialog. This may happen because this
        filter detection is used by different filters. */
    bool bAborted = aMediaDescriptor.getUnpackedValueOrDefault( MediaDescriptor::PROP_ABORTED(), false );
    if( !bAborted ) try
    {
        aMediaDescriptor.addInputStream();

        /*  Get the unencrypted input stream. This may include creation of a
            temporary file that contains the decrypted package. This temporary
            file will be stored in the 'ComponentData' property of the media
            descriptor. */
        Reference< XInputStream > xInputStream( extractUnencryptedPackage( aMediaDescriptor ), UNO_SET_THROW );

        // stream must be a ZIP package
        ZipStorage aZipStorage( mxContext, xInputStream );
        if( aZipStorage.isStorage() )
        {
            // create the fast parser, register the XML namespaces, set document handler
            FastParser aParser( mxContext );
            aParser.registerNamespace( NMSP_packageRel );
            aParser.registerNamespace( NMSP_officeRel );
            aParser.registerNamespace( NMSP_packageContentTypes );
            aParser.setDocumentHandler( new FilterDetectDocHandler( mxContext, aFilterName ) );

            /*  Parse '_rels/.rels' to get the target path and '[Content_Types].xml'
                to determine the content type of the part at the target path. */
            aParser.parseStream( aZipStorage, "_rels/.rels" );
            aParser.parseStream( aZipStorage, "[Content_Types].xml" );
        }
    }
    catch( const Exception& )
    {
    }

    // write back changed media descriptor members
    aMediaDescriptor >> rMediaDescSeq;
    return aFilterName;
}

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
