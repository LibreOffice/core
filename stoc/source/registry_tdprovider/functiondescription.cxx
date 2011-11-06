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

#include "functiondescription.hxx"

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/reflection/XCompoundTypeDescription.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/implbase1.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "registry/reader.hxx"
#include "registry/version.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace css = com::sun::star;

using stoc::registry_tdprovider::FunctionDescription;

FunctionDescription::FunctionDescription(
    css::uno::Reference< css::container::XHierarchicalNameAccess > const &
        manager,
    com::sun::star::uno::Sequence< sal_Int8 > const & bytes,
    sal_uInt16 index):
    m_manager(manager), m_bytes(bytes), m_index(index), m_exceptionsInit(false)
{}

FunctionDescription::~FunctionDescription() {}

css::uno::Sequence<
    css::uno::Reference< css::reflection::XCompoundTypeDescription > >
FunctionDescription::getExceptions() const {
    {
        osl::MutexGuard guard(m_mutex);
        if (m_exceptionsInit) {
            return m_exceptions;
        }
    }
    typereg::Reader reader(getReader());
    sal_uInt16 n = reader.getMethodExceptionCount(m_index);
    css::uno::Sequence<
        css::uno::Reference< css::reflection::XCompoundTypeDescription > >
            exceptions(n);
    for (sal_uInt16 i = 0; i < n; ++i) {
        rtl::OUString name(
            reader.getMethodExceptionTypeName(m_index, i).replace('/', '.'));
        css::uno::Any any;
        try {
            any = m_manager->getByHierarchicalName(name);
        } catch (css::container::NoSuchElementException & e) {
            throw new css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.container.NoSuchElementException: "))
                 + e.Message),
                css::uno::Reference< css::uno::XInterface >()); //TODO
        }
        if (!(any >>= exceptions[i])
            || exceptions[i]->getTypeClass() != css::uno::TypeClass_EXCEPTION)
        {
            throw new css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("not an exception type: "))
                 + name),
                css::uno::Reference< css::uno::XInterface >()); //TODO
        }
        OSL_ASSERT(exceptions[i].is());
    }
    osl::MutexGuard guard(m_mutex);
    if (!m_exceptionsInit) {
        m_exceptions = exceptions;
        m_exceptionsInit = true;
    }
    return m_exceptions;
}

typereg::Reader FunctionDescription::getReader() const {
    return typereg::Reader(
        m_bytes.getConstArray(), m_bytes.getLength(), false, TYPEREG_VERSION_1);
}
