
#ifndef __FRAMEWORK_CLASSES_TOOLBOXCONFIGURATION_HXX_
#include <xml/toolboxconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_TOOLBOXDOCUMENTHANDLER_HXX_
#include <xml/toolboxdocumenthandler.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_TOOLBOXLAYOUTDOCUMENTHANDLER_HXX_
#include <xml/toolboxlayoutdocumenthandler.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_SAXNAMESPACEFILTER_HXX_
#include <xml/saxnamespacefilter.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;


namespace framework
{

static Reference< XParser > GetSaxParser(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XParser >( xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" )), UNO_QUERY);
}

static Reference< XDocumentHandler > GetSaxWriter(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XDocumentHandler >( xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" )), UNO_QUERY) ;
}

// #110897#
sal_Bool ToolBoxConfiguration::LoadToolBox(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream,
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rToolbarConfiguration )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = rInputStream;

    // create namespace filter and set menudocument handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadToolBoxDocumentHandler( rToolbarConfiguration ));
    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch( SAXException& )
    {
        return sal_False;
    }
    catch( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }

    return sal_False;
}


// #110897#
sal_Bool ToolBoxConfiguration::StoreToolBox(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream,
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rToolbarConfiguration )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );

    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( rOutputStream );

    try
    {
        OWriteToolBoxDocumentHandler aWriteToolBoxDocumentHandler( rToolbarConfiguration, xWriter );
        aWriteToolBoxDocumentHandler.WriteToolBoxDocument();
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch ( SAXException& )
    {
        return sal_False;
    }
    catch ( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }

    return sal_False;
}

}
