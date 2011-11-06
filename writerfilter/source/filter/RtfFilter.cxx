/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <osl/module.hxx>
#include <tools/solar.h>
#include <RtfFilter.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;

RtfFilter::RtfFilter( const uno::Reference< uno::XComponentContext >& rxContext)  :
    m_xContext( rxContext )
{
}

RtfFilter::~RtfFilter()
{
}

sal_Bool RtfFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
   throw (uno::RuntimeException)
{
    OSL_TRACE("%s", OSL_THIS_FUNC);
    if( m_xSrcDoc.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc( xMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.RtfExport" ))), uno::UNO_QUERY_THROW);
        if (!xIfc.is())
            return sal_False;
        uno::Reference< document::XExporter > xExprtr(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFltr(xIfc, uno::UNO_QUERY_THROW);
        if (!xExprtr.is() || !xFltr.is())
            return sal_False;
        xExprtr->setSourceDocument(m_xSrcDoc);
        return xFltr->filter(aDescriptor);
    }
    else if ( m_xDstDoc.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc( xMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.RtfImport" ))), uno::UNO_QUERY_THROW);
        if (!xIfc.is())
            return sal_False;
        uno::Reference< document::XImporter > xImprtr(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFltr(xIfc, uno::UNO_QUERY_THROW);
        if (!xImprtr.is() || !xFltr.is())
            return sal_False;
        xImprtr->setTargetDocument(m_xDstDoc);
        return xFltr->filter(aDescriptor);
    }
    return sal_False;
}

void RtfFilter::cancel(  ) throw (uno::RuntimeException)
{
}

void RtfFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xSrcDoc = xDoc;
}

void RtfFilter::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xDstDoc = xDoc;
}

void RtfFilter::initialize( const uno::Sequence< uno::Any >& /*aArguments*/ ) throw (uno::Exception, uno::RuntimeException)
{
    // The DOCX exporter here extracts 'type' of the filter, ie 'Word' or
    // 'Word Template' but we don't need it for RTF.
}

OUString RtfFilter::getImplementationName(  ) throw (uno::RuntimeException)
{
   return RtfFilter_getImplementationName();
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExportFilter"
sal_Bool RtfFilter::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
{
    return (rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) ||
            rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME2 ) ));
}

uno::Sequence< OUString > RtfFilter::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return RtfFilter_getSupportedServiceNames();
}

/* Helpers, used by shared lib exports. */

OUString RtfFilter_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.RtfFilter" ) );
}

uno::Sequence< OUString > RtfFilter_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(2);
   OUString* pArray = aRet.getArray();
   pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
   pArray[1] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
   return aRet;
}
#undef SERVICE_NAME1
#undef SERVICE_NAME2

uno::Reference< uno::XInterface > RtfFilter_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new RtfFilter( xContext );
}

/* vi:set shiftwidth=4 expandtab: */
