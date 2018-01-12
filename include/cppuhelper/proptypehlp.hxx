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

#ifndef INCLUDED_CPPUHELPER_PROPTYPEHLP_HXX
#define INCLUDED_CPPUHELPER_PROPTYPEHLP_HXX

#include "cppuhelper/proptypehlp.h"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/uno/TypeClass.hpp"

namespace cppu
{

template < class target >
inline void SAL_CALL convertPropertyValue( target &value , const  css::uno::Any & a)
{

    if( !( a >>= value ) ) {
        throw css::lang::IllegalArgumentException();
    }
}

void convertPropertyValue(bool & b, const css::uno::Any & a)
{
    if( !(a >>= b) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BYTE:
            b = a.get<sal_Int8>() != 0;
            break;
        case css::uno::TypeClass_SHORT:
            b = a.get<sal_Int16>() != 0;
            break;
        case css::uno::TypeClass_UNSIGNED_SHORT:
            {
                sal_uInt16 i16 = 0;
                a >>= i16;
                b = i16 != 0;
                break;
            }
        case css::uno::TypeClass_LONG:
            b = a.get<sal_Int32>() != 0;
            break;
        case css::uno::TypeClass_UNSIGNED_LONG:
            b = a.get<sal_uInt32>() != 0;
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c = *static_cast<sal_Unicode const *>(a.getValue());
                b = c != 0;
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

void convertPropertyValue(sal_Bool & target, css::uno::Any const & source) {
    bool b;
    convertPropertyValue(b, source);
    target = b;
}

inline void SAL_CALL convertPropertyValue( sal_Int64 & i  , const css::uno::Any & a )
{
    if( !(a >>= i) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            i = static_cast<sal_Int64>(a.get<bool>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                i = static_cast<sal_Int64>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}


inline void SAL_CALL convertPropertyValue( sal_uInt64 & i  , const css::uno::Any & a )
{
    if( !(a >>= i) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            i = static_cast<sal_uInt64>(a.get<bool>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                i = static_cast<sal_uInt64>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

inline void SAL_CALL convertPropertyValue( sal_Int32 & i  , const css::uno::Any & a )
{
    if( !(a >>= i) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            i = static_cast<sal_Int32>(a.get<bool>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                i = static_cast<sal_Int32>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

inline void SAL_CALL convertPropertyValue( sal_uInt32 & i  , const css::uno::Any & a )
{
    if( !(a >>= i) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            i = static_cast<sal_uInt32>(a.get<bool>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                i = static_cast<sal_uInt32>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

inline void SAL_CALL convertPropertyValue( sal_Int16 & i  , const css::uno::Any & a )
{
    if( !(a >>= i) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            i = static_cast<sal_Int16>(a.get<bool>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                i = static_cast<sal_Int16>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

inline void SAL_CALL convertPropertyValue( sal_uInt16 & i  , const css::uno::Any & a )
{
    if( !(a >>= i) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            i = static_cast<sal_uInt16>(a.get<bool>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                i = static_cast<sal_Int16>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

inline void SAL_CALL convertPropertyValue( sal_Int8 & i  , const css::uno::Any & a )
{
    if( !(a >>= i) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            i = static_cast<sal_Int8>(a.get<bool>());
            break;
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

inline void SAL_CALL convertPropertyValue( float &f , const css::uno::Any &a )
{
    if( !(a >>= f) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            f = static_cast<float>(a.get<bool>());
            break;
        case css::uno::TypeClass_LONG:
            f = static_cast<float>(a.get<sal_Int32>());
            break;
        case css::uno::TypeClass_UNSIGNED_LONG:
            f = static_cast<float>(a.get<sal_uInt32>());
            break;
        case css::uno::TypeClass_HYPER:
            f = static_cast<float>(a.get<sal_Int64>());
            break;
        case css::uno::TypeClass_UNSIGNED_HYPER:
            f = static_cast<float>(a.get<sal_uInt64>());
            break;
        case css::uno::TypeClass_DOUBLE:
            f = static_cast<float>(a.get<double>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                f = static_cast<float>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

inline void SAL_CALL convertPropertyValue( double &d , const css::uno::Any &a )
{
    if( !(a >>= d) ) {
        switch( a.getValueType().getTypeClass() ) {
        case css::uno::TypeClass_BOOLEAN:
            d = static_cast<double>(a.get<bool>());
            break;
        case css::uno::TypeClass_HYPER:
            d = static_cast<double>(a.get<sal_Int64>());
            break;
        case css::uno::TypeClass_UNSIGNED_HYPER:
            d = static_cast<double>(a.get<sal_uInt64>());
            break;
        case css::uno::TypeClass_CHAR:
            {
                sal_Unicode c;
                c = *static_cast<sal_Unicode const *>(a.getValue());
                d = static_cast<double>(c);
                break;
            }
        default:
            throw css::lang::IllegalArgumentException();
        }
    }
}

}     // end namespace cppu

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
