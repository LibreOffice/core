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

#include "structtypedescription.hxx"

#include "base.hxx"

#include "com/sun/star/reflection/XTypeDescription.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "registry/types.h"
#include "registry/version.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include <new>

namespace css = com::sun::star;
using stoc::registry_tdprovider::StructTypeDescription;

StructTypeDescription::StructTypeDescription(
    css::uno::Reference< css::container::XHierarchicalNameAccess > const &
        manager,
    rtl::OUString const & name, rtl::OUString const & baseTypeName,
    css::uno::Sequence< sal_Int8 > const & data, bool published):
    m_data(data),
    m_base(
        new stoc_rdbtdp::CompoundTypeDescriptionImpl(
            manager, css::uno::TypeClass_STRUCT, name, baseTypeName, data,
            published))
{}

StructTypeDescription::~StructTypeDescription()
{}

css::uno::TypeClass StructTypeDescription::getTypeClass()
    throw (css::uno::RuntimeException)
{
    return m_base->getTypeClass();
}

rtl::OUString StructTypeDescription::getName()
    throw (css::uno::RuntimeException)
{
    return m_base->getName();
}

css::uno::Reference< css::reflection::XTypeDescription >
StructTypeDescription::getBaseType() throw (css::uno::RuntimeException)
{
    return m_base->getBaseType();
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
StructTypeDescription::getMemberTypes() throw (css::uno::RuntimeException)
{
    return m_base->getMemberTypes();
}

css::uno::Sequence< rtl::OUString > StructTypeDescription::getMemberNames()
    throw (css::uno::RuntimeException)
{
    return m_base->getMemberNames();
}

css::uno::Sequence< rtl::OUString > StructTypeDescription::getTypeParameters()
    throw (css::uno::RuntimeException)
{
    try {
        typereg::Reader reader(
            m_data.getConstArray(), m_data.getLength(), false,
            TYPEREG_VERSION_1);
        OSL_ASSERT(reader.isValid());
        sal_uInt16 n = reader.getReferenceCount();
        css::uno::Sequence< rtl::OUString > parameters(n);
        for (sal_uInt16 i = 0; i < n; ++i) {
            if (reader.getReferenceFlags(i) != RT_ACCESS_INVALID
                || reader.getReferenceSort(i) != RT_REF_TYPE_PARAMETER)
            {
                throw css::uno::RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "type parameter of polymorphic struct type template"
                            " not RT_ACCESS_INVALID/RT_REF_TYPE_PARAMETER")),
                    static_cast< cppu::OWeakObject * >(this));
            }
            parameters[i] = reader.getReferenceTypeName(i);
        }
        return parameters;
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("std::bad_alloc")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Sequence< css::uno::Reference< css::reflection::XTypeDescription > >
StructTypeDescription::getTypeArguments() throw (css::uno::RuntimeException)
{
    return css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > >();
}

sal_Bool StructTypeDescription::isPublished() throw (css::uno::RuntimeException)
{
    return m_base->isPublished();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
