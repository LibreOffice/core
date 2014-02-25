/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* KeynoteImportFilter: Sets up the filter, and calls OdpExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <boost/shared_ptr.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/uno/Reference.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <iostream>
#include <libetonyek/libetonyek.h>
#include <libodfgen/libodfgen.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include <ucbhelper/content.hxx>
#include <xmloff/attrlist.hxx>

#include "common/DirectoryStream.hxx"
#include "common/DocumentHandler.hxx"
#include "common/WPXSvStream.hxx"
#include "KeynoteImportFilter.hxx"

using boost::shared_ptr;

using namespace ::com::sun::star::uno;
using com::sun::star::uno::Reference;
using com::sun::star::io::XInputStream;
using com::sun::star::io::XSeekable;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

namespace beans = com::sun::star::beans;
namespace container = com::sun::star::container;
namespace ucb = com::sun::star::ucb;

namespace
{

template<class T>
sal_Bool lcl_queryIsPackage( const Sequence<T> &lComponentData )
{
    sal_Bool bIsPackage = sal_False;

    const sal_Int32 nLength = lComponentData.getLength();
    const T *pValue = lComponentData.getConstArray();
    for ( sal_Int32 i = 0; i < nLength; ++i)
    {
        if ( pValue[i].Name == "IsPackage" )
        {
            pValue[i].Value >>= bIsPackage;
            break;
        }
    }

    return bIsPackage;
}

sal_Bool lcl_isPackage( const Any &rComponentData )
{
    Sequence < beans::NamedValue > lComponentDataNV;
    Sequence < beans::PropertyValue > lComponentDataPV;

    if ( rComponentData >>= lComponentDataNV )
        return lcl_queryIsPackage( lComponentDataNV );
    else if ( rComponentData >>= lComponentDataPV )
        return lcl_queryIsPackage( lComponentDataPV );

    return false;
}
}

sal_Bool SAL_CALL KeynoteImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::filter");
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    Reference < ucb::XContent > xContent;
    sal_Bool bIsPackage = sal_False;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "ComponentData" )
            bIsPackage = lcl_isPackage( pValue[i].Value );
        else if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name == "UCBContent" )
            pValue[i].Value >>= xContent;
    }
    if ( !xInputStream.is() )
    {
        OSL_ASSERT( false );
        return sal_False;
    }

    if ( bIsPackage && !xContent.is() )
    {
        SAL_WARN("writerperfect", "the input claims to be a package, but does not have UCBContent");
        bIsPackage = false;
    }

    // An XML import service: what we push sax messages to..
    Reference < XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.comp.Draw.XMLOasisImporter", mxContext),
        css::uno::UNO_QUERY_THROW);

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument( mxDoc );

    // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    shared_ptr< WPXInputStream > input;
    if ( bIsPackage )
        input.reset( new writerperfect::DirectoryStream( xContent ) );
    else
        input.reset( new WPXSvInputStream( xInputStream ) );

    OdpGenerator exporter(&xHandler, ODF_FLAT_XML);
    bool tmpParseResult = libetonyek::KEYDocument::parse(input.get(), &exporter);
    return tmpParseResult;
}

void SAL_CALL KeynoteImportFilter::cancel(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::cancel");
}

// XImporter
void SAL_CALL KeynoteImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::setTargetDocument");
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL KeynoteImportFilter::detect( com::sun::star::uno::Sequence< PropertyValue >& Descriptor )
throw( com::sun::star::uno::RuntimeException, std::exception )
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::detect");

    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 nNewLength = nLength + 2;
    sal_Int32 nComponentDataLocation = -1;
    sal_Int32 nTypeNameLocation = -1;
    sal_Int32 nUCBContentLocation = -1;
    bool bIsPackage = false;
    bool bUCBContentChanged = false;
    const PropertyValue *pValue = Descriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    Reference < ucb::XContent > xContent;
    Sequence < beans::NamedValue > lComponentDataNV;
    Sequence < beans::PropertyValue > lComponentDataPV;
    bool bComponentDataNV = true;

    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "TypeName" )
        {
            nTypeNameLocation = i;
            --nNewLength;
        }
        if ( pValue[i].Name == "ComponentData" )
        {
            bComponentDataNV = pValue[i].Value >>= lComponentDataNV;
            if (!bComponentDataNV)
                pValue[i].Value >>= lComponentDataPV;
            nComponentDataLocation = i;
            --nNewLength;
        }
        else if ( pValue[i].Name == "InputStream" )
        {
            pValue[i].Value >>= xInputStream;
        }
        else if ( pValue[i].Name == "UCBContent" )
        {
            pValue[i].Value >>= xContent;
            nUCBContentLocation = i;
        }
    }

    assert(nNewLength >= nLength);

    if (!xInputStream.is())
        return OUString();

    shared_ptr< WPXInputStream > input( new WPXSvInputStream( xInputStream ) );

    /* Apple Keynote documents come in two variants:
     * * actual files (zip), only produced by Keynote 5 (at least with
     *   default settings)
     * * packages (IOW, directories), produced by Keynote 1-4 and again
     *   starting with 6.
     * But since the libetonyek import only works with a stream, we need
     * to pass it one for the whole package. Here we determine if that
     * is needed.
     *
     * Note: for convenience, we also recognize that the main XML file
     * from a package was passed and pass the whole package to the
     * filter instead.
     */
    if ( xContent.is() )
    {
        ucbhelper::Content aContent( xContent, Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        try
        {
            if ( aContent.isFolder() )
            {
                input.reset( new writerperfect::DirectoryStream( xContent ) );
                bIsPackage = true;
            }
        }
        catch (...)
        {
            return OUString();
        }

        libetonyek::KEYDocumentType type = libetonyek::KEY_DOCUMENT_TYPE_UNKNOWN;
        if ( !libetonyek::KEYDocument::isSupported( input.get(), &type ) )
            return OUString();

        if ( type == libetonyek::KEY_DOCUMENT_TYPE_APXL_FILE )
        {
            assert( !bIsPackage );

            const Reference < container::XChild > xChild( xContent, UNO_QUERY );
            if ( xChild.is() )
            {
                const Reference < ucb::XContent > xPackageContent( xChild->getParent(), UNO_QUERY );
                if ( xPackageContent.is() )
                {
                    input.reset( new writerperfect::DirectoryStream( xPackageContent ) );
                    if ( libetonyek::KEYDocument::isSupported( input.get() ) )
                    {
                        xContent = xPackageContent;
                        bUCBContentChanged = true;
                        bIsPackage = true;
                    }
                }
            }
        }
    }

    // we do not need to insert ComponentData if this is not a package
    if ( !bIsPackage && ( nComponentDataLocation == -1 ) )
        --nNewLength;

    if ( nNewLength > nLength )
        Descriptor.realloc( nNewLength );

    if ( nTypeNameLocation == -1 )
    {
        assert( nLength < nNewLength );
        nTypeNameLocation = nLength++;
        Descriptor[nTypeNameLocation].Name = "TypeName";
    }

    if ( bIsPackage && ( nComponentDataLocation == -1 ) )
    {
        assert( nLength < nNewLength );
        nComponentDataLocation = nLength++;
        Descriptor[nComponentDataLocation].Name = "ComponentData";
    }

    if ( bIsPackage )
    {
        if (bComponentDataNV)
        {
            const sal_Int32 nCDSize = lComponentDataNV.getLength();
            lComponentDataNV.realloc( nCDSize + 1 );
            beans::NamedValue aValue;
            aValue.Name = "IsPackage";
            aValue.Value = comphelper::makeBoolAny(true);
            lComponentDataNV[nCDSize] = aValue;
            Descriptor[nComponentDataLocation].Value <<= lComponentDataNV;
        }
        else
        {
            const sal_Int32 nCDSize = lComponentDataPV.getLength();
            lComponentDataPV.realloc( nCDSize + 1 );
            beans::PropertyValue aProp;
            aProp.Name = "IsPackage";
            aProp.Value = comphelper::makeBoolAny(true);
            aProp.Handle = -1;
            aProp.State = beans::PropertyState_DIRECT_VALUE;
            lComponentDataPV[nCDSize] = aProp;
            Descriptor[nComponentDataLocation].Value <<= lComponentDataPV;
        }
    }

    if ( bUCBContentChanged )
        Descriptor[nUCBContentLocation].Value <<= xContent;

    const OUString sTypeName("impress_AppleKeynote");
    Descriptor[nTypeNameLocation].Value <<= sTypeName;

    return sTypeName;
}

// XInitialization
void SAL_CALL KeynoteImportFilter::initialize( const Sequence< Any >& aArguments )
throw (Exception, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::initialize");
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue *pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue[i].Name == "Type" )
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}

OUString KeynoteImportFilter_getImplementationName ()
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter_getImplementationName");
    return OUString ( "org.libreoffice.comp.Impress.KeynoteImportFilter" );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"

Sequence< OUString > SAL_CALL KeynoteImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter_getSupportedServiceNames");
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  OUString ( SERVICE_NAME1 );
    pArray[1] =  OUString ( SERVICE_NAME2 );
    return aRet;
}

#undef SERVICE_NAME2
#undef SERVICE_NAME1

Reference< XInterface > SAL_CALL KeynoteImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    SAL_INFO("writerperfect", "KeynoteImportFilter_createInstance");
    return (cppu::OWeakObject *) new KeynoteImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL KeynoteImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::getImplementationName");
    return KeynoteImportFilter_getImplementationName();
}

sal_Bool SAL_CALL KeynoteImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::supportsService");
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL KeynoteImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::getSupportedServiceNames");
    return KeynoteImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
