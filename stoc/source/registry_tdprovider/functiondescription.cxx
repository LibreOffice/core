/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: functiondescription.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:04:22 $
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
