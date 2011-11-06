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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
    class XComponentContext;
} } } }

namespace stoc_services
{
    // typeconv
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL TypeConverter_Impl_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & )
    throw( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > tcv_getSupportedServiceNames();
    ::rtl::OUString tcv_getImplementationName();

    namespace ExternalUriReferenceTranslator
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriReferenceFactory
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >  const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTexpand
    {
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const &)
        SAL_THROW((::com::sun::star::uno::Exception));
    ::rtl::OUString SAL_CALL getImplementationName();
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace UriSchemeParser_vndDOTsunDOTstarDOTscript
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const &)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }

    namespace VndSunStarPkgUrlReferenceFactory
    {
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context)
        SAL_THROW((com::sun::star::uno::Exception));
    rtl::OUString SAL_CALL getImplementationName();
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames();
    }
} // namespace
