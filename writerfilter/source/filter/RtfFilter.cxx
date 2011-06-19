/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Miklos Vajna.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
