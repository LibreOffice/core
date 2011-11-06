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
#include "precompiled_svl.hxx"

#include <unotools/pathoptions.hxx>
#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace css = com::sun::star;
using rtl::OUString;

// -----------------------------------------------------------------------

class PathService : public ::cppu::WeakImplHelper2< css::frame::XConfigManager, css::lang::XServiceInfo >
{
    SvtPathOptions m_aOptions;

public:
    PathService()
        {}

    virtual OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException)
        {
            return OUString::createFromAscii("com.sun.star.comp.svl.PathService");
        }

    virtual sal_Bool SAL_CALL supportsService (
        const OUString & rName)
        throw(css::uno::RuntimeException)
        {
            return (rName.compareToAscii("com.sun.star.config.SpecialConfigManager") == 0);
        }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException)
        {
            css::uno::Sequence< OUString > aRet(1);
            aRet.getArray()[0] = OUString::createFromAscii("com.sun.star.config.SpecialConfigManager");
            return aRet;
        }

    virtual OUString SAL_CALL substituteVariables (
        const OUString& sText)
        throw(css::uno::RuntimeException)
        {
            return m_aOptions.SubstituteVariable( sText );
        }

    virtual void SAL_CALL addPropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException)
        {}

    virtual void SAL_CALL removePropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException)
        {}

    virtual void SAL_CALL flush()
        throw(css::uno::RuntimeException)
        {}
};

// -----------------------------------------------------------------------

css::uno::Reference< css::uno::XInterface > PathService_CreateInstance (
    const css::uno::Reference< css::lang::XMultiServiceFactory > &)
{
    return css::uno::Reference< css::uno::XInterface >(
        static_cast< cppu::OWeakObject* >(new PathService()));
}

// -----------------------------------------------------------------------
