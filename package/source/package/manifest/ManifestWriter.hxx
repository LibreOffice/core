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



#ifndef _MANIFEST_WRITER_HXX
#define _MANIFEST_WRITER_HXX

#include <cppuhelper/implbase2.hxx>
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTWRITER_HPP
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XPSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; class XSingleServiceFactory; }
} } }

class ManifestWriter: public ::cppu::WeakImplHelper2
<
    ::com::sun::star::packages::manifest::XManifestWriter,
    ::com::sun::star::lang::XServiceInfo
>
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory;
public:
    ManifestWriter( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xNewFactory );
    virtual ~ManifestWriter();

    // XManifestWriter
    virtual void SAL_CALL writeManifestSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rStream, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& rSequence )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    // Component constructor
    static ::rtl::OUString static_getImplementationName();
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > static_getSupportedServiceNames();
    static ::com::sun::star::uno::Reference < com::sun::star::lang::XSingleServiceFactory > createServiceFactory( com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory );
    sal_Bool SAL_CALL static_supportsService(rtl::OUString const & rServiceName);
};
#endif

