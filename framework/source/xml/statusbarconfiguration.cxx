
#ifndef __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_
#include <xml/statusbarconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_STATUSBARDOCUMENTHANDLER_HXX_
#include <xml/statusbardocumenthandler.hxx>
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


namespace framework
{

SV_IMPL_PTRARR( StatusBarDescriptor, StatusBarItemDescriptorPtr);

static Reference< XParser > GetSaxParser(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    //Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    //return Reference< XParser >( xServiceManager->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" )), UNO_QUERY);
    return Reference< XParser >( xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" )), UNO_QUERY);
}

static Reference< XDocumentHandler > GetSaxWriter(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    //Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    //return Reference< XDocumentHandler >( xServiceManager->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" )), UNO_QUERY) ;
    return Reference< XDocumentHandler >( xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" )), UNO_QUERY) ;
}

// #110897#
sal_Bool StatusBarConfiguration::LoadStatusBar(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    SvStream& rInStream, StatusBarDescriptor& aItems )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );
    Reference< XInputStream > xInputStream(
                                (::cppu::OWeakObject *)new utl::OInputStreamWrapper( rInStream ),
                                UNO_QUERY );

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = xInputStream;

    // create namespace filter and set menudocument handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadStatusBarDocumentHandler( aItems ));
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
sal_Bool StatusBarConfiguration::StoreStatusBar(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    SvStream& rOutStream, const StatusBarDescriptor& aItems )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );

    Reference< XOutputStream > xOutputStream(
                                (::cppu::OWeakObject *)new utl::OOutputStreamWrapper( rOutStream ),
                                UNO_QUERY );

    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( xOutputStream );

    try
    {
        OWriteStatusBarDocumentHandler aWriteStatusBarDocumentHandler( aItems, xWriter );
        aWriteStatusBarDocumentHandler.WriteStatusBarDocument();
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
