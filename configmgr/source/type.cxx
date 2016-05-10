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

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppu/unotype.hxx>
#include <rtl/string.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "type.hxx"

namespace configmgr {

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
        assert(false);
        throw css::uno::RuntimeException("this cannot happen");
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
        return cppu::UnoType< OUString >::get();
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
        return cppu::UnoType< css::uno::Sequence< OUString > >::get();
    case TYPE_HEXBINARY_LIST:
        return cppu::UnoType<
            css::uno::Sequence< css::uno::Sequence< sal_Int8 > > >::get();
    default:
        assert(false);
        throw css::uno::RuntimeException("this cannot happen");
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
            OUString name(value.getValueType().getTypeName());
            if ( name == "[]byte" ) {
                return TYPE_HEXBINARY;
            } else if (name == "[]boolean")
            {
                return TYPE_BOOLEAN_LIST;
            } else if ( name == "[]short" )
            {
                return TYPE_SHORT_LIST;
            } else if ( name == "[]long" )
            {
                return TYPE_INT_LIST;
            } else if ( name == "[]hyper" )
            {
                return TYPE_LONG_LIST;
            } else if (name == "[]double")
            {
                return TYPE_DOUBLE_LIST;
            } else if (name == "[]string")
            {
                return TYPE_STRING_LIST;
            } else if (name == "[][]byte")
            {
                return TYPE_HEXBINARY_LIST;
            }
        }
        SAL_FALLTHROUGH;
    default:
        return TYPE_ERROR;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
