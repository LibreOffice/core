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



#ifndef _RTFIMPORTFILTER_HXX_
#define _RTFIMPORTFILTER_HXX_

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <tools/stream.hxx>
#include <shellio.hxx>

/// The physical access to the RTF document (for reading).
class RtfImportFilter : public cppu::WeakImplHelper2
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter
>
{
protected:
    ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xDstDoc;
public:
    RtfImportFilter( const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );
    virtual ~RtfImportFilter();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};

::rtl::OUString RtfImport_getImplementationName();
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL RtfImport_getSupportedServiceNames()
    throw();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL RtfImport_createInstance(
                                                                        const ::com::sun::star::uno::Reference<
                                                                        com::sun::star::lang::XMultiServiceFactory > &xMSF)
    throw( ::com::sun::star::uno::Exception );

#define IMPL_NAME_RTFIMPORT "com.sun.star.comp.Writer.RtfImport"

#endif // _RTFIMPORTFILTER_HXX_
/* vi:set shiftwidth=4 expandtab: */
