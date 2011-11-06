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



#ifndef _RTFEXPORTFILTER_HXX_
#define _RTFEXPORTFILTER_HXX_

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <tools/stream.hxx>
#include <shellio.hxx>

// This is just here so that we don't have to copy&paste its string format methods
class RtfWriter : public Writer
{
protected:
    sal_uLong WriteStream() { return 0; }
};

/// The physical access to the RTF document (for writing).
class RtfExportFilter : public cppu::WeakImplHelper2
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XExporter
>
{
protected:
    ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xSrcDoc;
    SvStream* m_pStream;
public:
    RtfExportFilter( const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );
    virtual ~RtfExportFilter();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    RtfWriter m_aWriter;
};

::rtl::OUString RtfExport_getImplementationName();
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL RtfExport_getSupportedServiceNames()
    throw();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL RtfExport_createInstance(
                                                                        const ::com::sun::star::uno::Reference<
                                                                        com::sun::star::lang::XMultiServiceFactory > &xMSF)
    throw( ::com::sun::star::uno::Exception );

#define IMPL_NAME_RTFEXPORT "com.sun.star.comp.Writer.RtfExport"

#endif // _RTFEXPORTFILTER_HXX_
/* vi:set shiftwidth=4 expandtab: */
