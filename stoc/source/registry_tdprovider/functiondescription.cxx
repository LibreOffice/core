/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
