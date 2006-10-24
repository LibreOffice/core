#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef INCLUDED_DOMAINMAPPER_HXX
#include <dmapper/DomainMapper.hxx>
#endif
#ifndef _WRITERFILTER_HXX
#include <WriterFilter.hxx>
#endif
#ifndef INCLUDED_WW8_DOCUMENT_HXX
#include <doctok/WW8Document.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
   throw (uno::RuntimeException)
{
    sal_Int32 nLength = aDescriptor.getLength();
    const beans::PropertyValue * pValue = aDescriptor.getConstArray();
    uno::Reference < io::XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "InputStream" ) ) )
            pValue[i].Value >>= xInputStream;
    }
    if ( !xInputStream.is() )
    {
        return sal_False;
    }
    //create the tokenizer and domain mapper
    doctok::WW8Stream::Pointer_t pDocStream = doctok::WW8DocumentFactory::createStream(m_xContext, xInputStream);
    doctok::WW8Document::Pointer_t pDocument = doctok::WW8DocumentFactory::createDocument(pDocStream);
    doctok::Stream::Pointer_t pStream(new dmapper::DomainMapper(m_xDoc));
    pDocument->resolve(*pStream);

    return sal_True;
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void WriterFilter::cancel(  ) throw (uno::RuntimeException)
{
}

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void WriterFilter::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xDoc = xDoc;
}

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilter::detect( uno::Sequence< beans::PropertyValue >& /*Descriptor*/ )
   throw( uno::RuntimeException )
{
   OUString sTypeName;
   // TODO: place the detection here
   return sTypeName;
}


/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void WriterFilter::initialize( const uno::Sequence< uno::Any >& aArguments ) throw (uno::Exception, uno::RuntimeException)
{
   uno::Sequence < beans::PropertyValue > aAnySeq;
   sal_Int32 nLength = aArguments.getLength();
   if ( nLength && ( aArguments[0] >>= aAnySeq ) )
   {
       const beans::PropertyValue * pValue = aAnySeq.getConstArray();
       nLength = aAnySeq.getLength();
       for ( sal_Int32 i = 0 ; i < nLength; i++)
       {
           if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Type" ) ) )
           {
               pValue[i].Value >>= m_sFilterName;
               break;
           }
       }
   }
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilter_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.WriterFilter" ) );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilter_supportsService( const OUString& ServiceName ) throw (uno::RuntimeException)
{
   return (ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) ||
       ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME2 ) ) );
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > WriterFilter_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(2);
   OUString* pArray = aRet.getArray();
   pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
   pArray[1] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
   return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< uno::XInterface > WriterFilter_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new WriterFilter( xContext );
}

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilter::getImplementationName(  ) throw (uno::RuntimeException)
{
   return WriterFilter_getImplementationName();
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilter::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
{
    return WriterFilter_supportsService( rServiceName );
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > WriterFilter::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return WriterFilter_getSupportedServiceNames();
}

