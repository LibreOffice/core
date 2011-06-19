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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

bool isListType(Type type) {
    return type >= TYPE_BOOLEAN_LIST;
}

Type elementType(Type type) {
    switch (type) {
    case TYPE_BOOLEAN_LIST:
        return TYPE_BOOLEAN;
    case TYPE_SHORT_LIST:
        return TYPE_SHORT;
    case TYPE_INT_LIST:
        return TYPE_INT;
    case TYPE_LONG_LIST:
        return TYPE_LONG;
    case TYPE_DOUBLE_LIST:
        return TYPE_DOUBLE;
    case TYPE_STRING_LIST:
        return TYPE_STRING;
    case TYPE_HEXBINARY_LIST:
        return TYPE_HEXBINARY;
    default:
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

css::uno::Type mapType(Type type) {
    switch (type) {
    case TYPE_ANY:
        return cppu::UnoType< css::uno::Any >::get();
    case TYPE_BOOLEAN:
        return cppu::UnoType< sal_Bool >::get();
    case TYPE_SHORT:
        return cppu::UnoType< sal_Int16 >::get();
    case TYPE_INT:
        return cppu::UnoType< sal_Int32 >::get();
    case TYPE_LONG:
        return cppu::UnoType< sal_Int64 >::get();
    case TYPE_DOUBLE:
        return cppu::UnoType< double >::get();
    case TYPE_STRING:
        return cppu::UnoType< rtl::OUString >::get();
    case TYPE_HEXBINARY:
        return cppu::UnoType< css::uno::Sequence< sal_Int8 > >::get();
    case TYPE_BOOLEAN_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Bool > >::get();
    case TYPE_SHORT_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Int16 > >::get();
    case TYPE_INT_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Int32 > >::get();
    case TYPE_LONG_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Int64 > >::get();
    case TYPE_DOUBLE_LIST:
        return cppu::UnoType< css::uno::Sequence< double > >::get();
    case TYPE_STRING_LIST:
        return cppu::UnoType< css::uno::Sequence< rtl::OUString > >::get();
    case TYPE_HEXBINARY_LIST:
        return cppu::UnoType<
            css::uno::Sequence< css::uno::Sequence< sal_Int8 > > >::get();
    default:
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

Type getDynamicType(css::uno::Any const & value) {
    switch (value.getValueType().getTypeClass()) {
    case css::uno::TypeClass_VOID:
        return TYPE_NIL;
    case css::uno::TypeClass_BOOLEAN:
        return TYPE_BOOLEAN;
    case css::uno::TypeClass_BYTE:
        return TYPE_SHORT;
    case css::uno::TypeClass_SHORT:
        return TYPE_SHORT;
    case css::uno::TypeClass_UNSIGNED_SHORT:
        return value.has< sal_Int16 >() ? TYPE_SHORT : TYPE_INT;
    case css::uno::TypeClass_LONG:
        return TYPE_INT;
    case css::uno::TypeClass_UNSIGNED_LONG:
        return value.has< sal_Int32 >() ? TYPE_INT : TYPE_LONG;
    case css::uno::TypeClass_HYPER:
        return TYPE_LONG;
    case css::uno::TypeClass_UNSIGNED_HYPER:
        return value.has< sal_Int64 >() ? TYPE_LONG : TYPE_ERROR;
    case css::uno::TypeClass_FLOAT:
    case css::uno::TypeClass_DOUBLE:
        return TYPE_DOUBLE;
    case css::uno::TypeClass_STRING:
        return TYPE_STRING;
    case css::uno::TypeClass_SEQUENCE: //TODO
        {
            rtl::OUString name(value.getValueType().getTypeName());
            if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]byte"))) {
                return TYPE_HEXBINARY;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[]boolean")))
            {
                return TYPE_BOOLEAN_LIST;
            } else if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]short")))
            {
                return TYPE_SHORT_LIST;
            } else if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]long")))
            {
                return TYPE_INT_LIST;
            } else if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]hyper")))
            {
                return TYPE_LONG_LIST;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[]double")))
            {
                return TYPE_DOUBLE_LIST;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[]string")))
            {
                return TYPE_STRING_LIST;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[][]byte")))
            {
                return TYPE_HEXBINARY_LIST;
            }
        }
        // fall through
    default:
        return TYPE_ERROR;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
