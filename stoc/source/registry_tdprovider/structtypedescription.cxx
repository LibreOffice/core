/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
            rtl::OUString("std::bad_alloc"),
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
