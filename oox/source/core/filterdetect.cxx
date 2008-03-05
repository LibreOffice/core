/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filterdetect.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:13:10 $
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

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>

#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/helper/helper.hxx"
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
    const OUString      maWordFilterName;
    const OUString      maExcelFilterName;
    const OUString      maExcelBinFilterName;
    const OUString      maPowerPointFilterName;
    OUString            maTargetPath;
};

// ============================================================================

FilterDetectDocHandler::FilterDetectDocHandler( OUString& rFilter ) :
    mrFilter( rFilter ),
    maWordFilterName( CREATE_OUSTRING( "MS Word 2007 XML" ) ),
    maExcelFilterName( CREATE_OUSTRING( "MS Excel 2007 XML" ) ),
    maExcelBinFilterName( CREATE_OUSTRING( "MS Excel 2007 Binary" ) ),
    maPowerPointFilterName( CREATE_OUSTRING( "MS PowerPoint 2007 XML" ) )
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
    OUString aType = rAttribs.getString( XML_Type );
    if( aType.equalsAscii( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" ) )
        maTargetPath = OUString( sal_Unicode( '/' ) ) + rAttribs.getString( XML_Target );
}

OUString FilterDetectDocHandler::getFilterNameFromContentType( const OUString& rContentType ) const
{
    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml" ) )
        return maWordFilterName;
    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-excel.sheet.macroEnabled.main+xml" ) )
        return maExcelFilterName;
    if( rContentType.equalsAscii( "application/vnd.ms-excel.sheet.binary.macroEnabled.main" ) )
        return maExcelBinFilterName;
    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml" ) )
        return maPowerPointFilterName;
    return OUString();
}

void FilterDetectDocHandler::parseContentTypesDefault( const AttributeList& rAttribs )
{
    // only if no overridden part name found
    if( mrFilter.getLength() == 0 )
    {
        // check if target path ends with extension
        OUString aExtension = rAttribs.getString( XML_Extension );
        sal_Int32 nExtPos = maTargetPath.getLength() - aExtension.getLength();
        if( (nExtPos > 0) && (maTargetPath[ nExtPos - 1 ] == '.') && maTargetPath.match( aExtension, nExtPos ) )
            mrFilter = getFilterNameFromContentType( rAttribs.getString( XML_ContentType ) );
    }
}

void FilterDetectDocHandler::parseContentTypesOverride( const AttributeList& rAttribs )
{
    if( rAttribs.getString( XML_PartName ).equals( maTargetPath ) )
        mrFilter = getFilterNameFromContentType( rAttribs.getString( XML_ContentType ) );
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
}

FilterDetect::~FilterDetect()
{
}

// com.sun.star.document.XExtendedFilterDetect interface ----------------------

OUString SAL_CALL FilterDetect::detect( Sequence< PropertyValue >& lDescriptor ) throw( RuntimeException )
{
    OUString aFilter;

    Reference< XFastDocumentHandler > xHandler( new FilterDetectDocHandler(aFilter) );
    Reference< XFastTokenHandler > xTokenHandler( new ::oox::FastTokenHandler );

    try
    {
        Reference< XFastParser > xParser( ::comphelper::getProcessServiceFactory()->createInstance(
            CREATE_OUSTRING( "com.sun.star.xml.sax.FastParser" ) ), UNO_QUERY_THROW );

        xParser->setFastDocumentHandler( xHandler );
        xParser->setTokenHandler( xTokenHandler );

        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/relationships" ),
                                    NMSP_PACKAGE_RELATIONSHIPS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships" ),
                                    NMSP_RELATIONSHIPS );
        xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/content-types" ),
                                    NMSP_CONTENT_TYPES );

        MediaDescriptor aDescriptor( lDescriptor );
        aDescriptor.addInputStream();
        Reference< XInputStream > xInputStream(
            aDescriptor[MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY_THROW );

        Reference< XStorage > xStorage( ::comphelper::OStorageHelper::GetStorageFromInputStream( xInputStream, mxFactory ),
                                        UNO_QUERY_THROW );

        // Parse _rels/.rels to get the target path.
        Reference< XStorage > xRels( xStorage->openStorageElement( CREATE_OUSTRING("_rels"), ElementModes::READ ) );
        Reference< XInputStream > xStream( xRels->openStreamElement( CREATE_OUSTRING(".rels"), ElementModes::READ ),
                                           UNO_QUERY_THROW );

        InputSource aParserInput;
        aParserInput.sSystemId = CREATE_OUSTRING("_rels/.rels");
        aParserInput.aInputStream = xStream;
        xParser->parseStream( aParserInput );

        // Parse [Content_Types].xml to determine the content type of the part at the target path.
        OUString aContentTypeName = CREATE_OUSTRING("[Content_Types].xml");
        xStream.set( xStorage->openStreamElement( aContentTypeName, ElementModes::READ ), UNO_QUERY_THROW );

        aParserInput.sSystemId = aContentTypeName;
        aParserInput.aInputStream = xStream;
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

