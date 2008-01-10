/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImportFilter.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:54:59 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SU_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
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
#ifndef INCLUDED_OOXML_DOCUMENT_HXX
#include <ooxml/OOXMLDocument.hxx>
#endif
#ifdef DEBUG_IMPORT
#include <iostream>
#include <osl/process.h>
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
    ::rtl::OUString sFilterName;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "InputStream" ) ) )
            pValue[i].Value >>= xInputStream;
        else if( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "FilterName" ) ) )
            pValue[i].Value >>= sFilterName;
    }
    if ( !xInputStream.is() )
    {
        return sal_False;
    }

#ifdef DEBUG_IMPORT
    TimeValue t1;
    osl_getSystemTime(&t1);

    writerfilter::logger("DEBUG", "<out>");
    writerfilter::logger("DEBUG", string("<starttime>") + string(t1.Seconds)
                         + "</starttime>");
#endif

    writerfilter::dmapper::SourceDocumentType eType = m_sFilterName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "writer_MS_Word_2007" ) ) ?
                writerfilter::dmapper::DOCUMENT_OOXML : writerfilter::dmapper::DOCUMENT_DOC;
    writerfilter::Stream::Pointer_t pStream(new writerfilter::dmapper::DomainMapper(m_xContext, m_xDoc, eType));
    //create the tokenizer and domain mapper
    if( eType == writerfilter::dmapper::DOCUMENT_OOXML )
    {
        writerfilter::ooxml::OOXMLStream::Pointer_t pDocStream = writerfilter::ooxml::OOXMLDocumentFactory::createStream(m_xContext, xInputStream);
        writerfilter::ooxml::OOXMLDocument::Pointer_t pDocument(writerfilter::ooxml::OOXMLDocumentFactory::createDocument(pDocStream));

        uno::Reference<frame::XModel> xModel(m_xDoc, uno::UNO_QUERY_THROW);
        pDocument->setModel(xModel);

        uno::Reference<drawing::XDrawPageSupplier> xDrawings
            (m_xDoc, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShapes> xShapes
            (xDrawings->getDrawPage(), uno::UNO_QUERY_THROW);
        pDocument->setShapes(xShapes);

        pDocument->resolve(*pStream);
    }
    else
    {
        writerfilter::doctok::WW8Stream::Pointer_t pDocStream = writerfilter::doctok::WW8DocumentFactory::createStream(m_xContext, xInputStream);
        writerfilter::doctok::WW8Document::Pointer_t pDocument(writerfilter::doctok::WW8DocumentFactory::createDocument(pDocStream));

        pDocument->resolve(*pStream);
    }

#ifdef DEBUG_IMPORT
    TimeValue t2;
    osl_getSystemTime(&t2);

    writerfilter::logger("DEBUG", string("<importtime>")
                         + string(t2.Seconds - t1.Seconds) + "</importtime>");
    writerfilter::logger("DEBUG", "</out>");
#endif

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
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilter_supportsService( const OUString& ServiceName ) throw (uno::RuntimeException)
{
   return (ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ));
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > WriterFilter_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(1);
   OUString* pArray = aRet.getArray();
   pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
   return aRet;
}
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

