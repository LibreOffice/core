/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filterdetect.cxx,v $
 * $Revision: 1.5.4.1 $
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

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>

#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>

#include <comphelper/mediadescriptor.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/core/fasttokenhandler.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

#include <vector>

using ::rtl::OUString;
using ::rtl::OString;
using ::comphelper::MediaDescriptor;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace core {

// ============================================================================

/** Document handler specifically designed for detecting OOXML file formats.

    It takes a reference to the filter string object via its constructor, and
    puts the name of the detected filter to it if it successfully finds one.
 */
class FilterDetectDocHandler : public ::cppu::WeakImplHelper1< XFastDocumentHandler >
{
public:
    explicit            FilterDetectDocHandler( OUString& rFilter );
    virtual             ~FilterDetectDocHandler();

    // XFastDocumentHandler
    virtual void SAL_CALL startDocument() throw (SAXException, RuntimeException);
    virtual void SAL_CALL endDocument() throw (SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator ) throw (SAXException, RuntimeException);

    // XFastContextHandler
    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL startUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL endFastElement( sal_Int32 Element ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL endUnknownElement( const OUString& Namespace, const OUString& Name ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
    virtual Reference< XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL characters( const OUString& aChars ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) throw (SAXException, RuntimeException);

private:
    void                parseRelationship( const AttributeList& rAttribs );

    OUString            getFilterNameFromContentType( const OUString& rContentType ) const;
    void                parseContentTypesDefault( const AttributeList& rAttribs );
    void                parseContentTypesOverride( const AttributeList& rAttribs );

private:
    typedef ::std::vector< sal_Int32 > ContextVector;

    OUString&           mrFilter;
    ContextVector       maContextStack;
    OUString            maTargetPath;
};

// ============================================================================

FilterDetectDocHandler::FilterDetectDocHandler( OUString& rFilter ) :
    mrFilter( rFilter )
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

// ===========================================================================

void SAL_CALL FilterDetectDocHandler::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rAttribs )
    throw (SAXException,RuntimeException)
{
    AttributeList aAttribs( rAttribs );
    switch ( nElement )
    {
        // cases for _rels/.rels
        case NMSP_PACKAGE_RELATIONSHIPS|XML_Relationships:
        break;
        case NMSP_PACKAGE_RELATIONSHIPS|XML_Relationship:
            if( !maContextStack.empty() && (maContextStack.back() == (NMSP_PACKAGE_RELATIONSHIPS|XML_Relationships)) )
                parseRelationship( aAttribs );
        break;

        // cases for [Content_Types].xml
        case NMSP_CONTENT_TYPES|XML_Types:
        break;
        case NMSP_CONTENT_TYPES|XML_Default:
            if( !maContextStack.empty() && (maContextStack.back() == (NMSP_CONTENT_TYPES|XML_Types)) )
                parseContentTypesDefault( aAttribs );
        break;
        case NMSP_CONTENT_TYPES|XML_Override:
            if( !maContextStack.empty() && (maContextStack.back() == (NMSP_CONTENT_TYPES|XML_Types)) )
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

// ============================================================================

void FilterDetectDocHandler::parseRelationship( const AttributeList& rAttribs )
{
    OUString aType = rAttribs.getString( XML_Type, OUString() );
    if( aType.equalsAscii( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" ) )
        maTargetPath = OUString( sal_Unicode( '/' ) ) + rAttribs.getString( XML_Target, OUString() );
}

OUString FilterDetectDocHandler::getFilterNameFromContentType( const OUString& rContentType ) const
{
    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-excel.sheet.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS Excel 2007 XML" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-excel.template.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS Excel 2007 XML Template" );

    if( rContentType.equalsAscii( "application/vnd.ms-excel.sheet.binary.macroEnabled.main" ) )
        return CREATE_OUSTRING( "MS Excel 2007 Binary" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS PowerPoint 2007 XML" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.presentationml.template.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-powerpoint.template.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS PowerPoint 2007 XML Template" );

    return OUString();
}

void FilterDetectDocHandler::parseContentTypesDefault( const AttributeList& rAttribs )
{
    // only if no overridden part name found
    if( mrFilter.getLength() == 0 )
    {
        // check if target path ends with extension
        OUString aExtension = rAttribs.getString( XML_Extension, OUString() );
        sal_Int32 nExtPos = maTargetPath.getLength() - aExtension.getLength();
        if( (nExtPos > 0) && (maTargetPath[ nExtPos - 1 ] == '.') && maTargetPath.match( aExtension, nExtPos ) )
            mrFilter = getFilterNameFromContentType( rAttribs.getString( XML_ContentType, OUString() ) );
    }
}

void FilterDetectDocHandler::parseContentTypesOverride( const AttributeList& rAttribs )
{
    if( rAttribs.getString( XML_PartName, OUString() ).equals( maTargetPath ) )
        mrFilter = getFilterNameFromContentType( rAttribs.getString( XML_ContentType, OUString() ) );
}

// ============================================================================

class FilterDetect : public ::cppu::WeakImplHelper2< XExtendedFilterDetection, XServiceInfo >
{
public:
    explicit            FilterDetect( const Reference< XMultiServiceFactory >& xFactory );
    virtual             ~FilterDetect();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw( RuntimeException );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( RuntimeException );

    // XExtendedFilterDetect
    virtual OUString SAL_CALL detect( Sequence< PropertyValue >& lDescriptor ) throw( RuntimeException );

private:
    Reference< XMultiServiceFactory > mxFactory;
};

// ----------------------------------------------------------------------------

/* Helper for XServiceInfo */
Sequence< OUString > FilterDetect_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = CREATE_OUSTRING( "com.sun.star.frame.ExtendedTypeDetection" );
    return aServiceNames;
}

/* Helper for XServiceInfo */
OUString FilterDetect_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.FormatDetector" );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL FilterDetect_createInstance( const Reference< XMultiServiceFactory >& xServiceManager ) throw( Exception )
{
    return Reference< XInterface >( *new FilterDetect( xServiceManager ) );
}

FilterDetect::FilterDetect( const Reference< XMultiServiceFactory >& xFactory ) :
    mxFactory( xFactory )
{
    OSL_ENSURE( mxFactory.is(), "FilterDetect::FilterDetect - no service factory" );
}

FilterDetect::~FilterDetect()
{
}

// com.sun.star.document.XExtendedFilterDetect interface ----------------------

OUString SAL_CALL FilterDetect::detect( Sequence< PropertyValue >& lDescriptor ) throw( RuntimeException )
{
    OUString aFilter;

    if( mxFactory.is() ) try
    {
        Reference< XFastParser > xParser( mxFactory->createInstance(
            CREATE_OUSTRING( "com.sun.star.xml.sax.FastParser" ) ), UNO_QUERY_THROW );

        xParser->setFastDocumentHandler( new FilterDetectDocHandler( aFilter ) );
        xParser->setTokenHandler( new FastTokenHandler );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/relationships" ), NMSP_PACKAGE_RELATIONSHIPS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships" ), NMSP_RELATIONSHIPS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/content-types" ), NMSP_CONTENT_TYPES );

        MediaDescriptor aDescriptor( lDescriptor );
        aDescriptor.addInputStream();
        Reference< XInputStream > xInputStream( aDescriptor[ MediaDescriptor::PROP_INPUTSTREAM() ], UNO_QUERY_THROW );
        StorageRef xStorage( new ZipStorage( mxFactory, xInputStream ) );

        // Parse _rels/.rels to get the target path.
        InputSource aParserInput;
        aParserInput.sSystemId = CREATE_OUSTRING( "_rels/.rels" );
        aParserInput.aInputStream = xStorage->openInputStream( aParserInput.sSystemId );
        xParser->parseStream( aParserInput );

        // Parse [Content_Types].xml to determine the content type of the part at the target path.
        aParserInput.sSystemId = CREATE_OUSTRING( "[Content_Types].xml" );
        aParserInput.aInputStream = xStorage->openInputStream( aParserInput.sSystemId );
        xParser->parseStream( aParserInput );
    }
    catch ( const Exception& )
    {
    }

    return aFilter;
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

// ============================================================================

} // namespace core
} // namespace oox

