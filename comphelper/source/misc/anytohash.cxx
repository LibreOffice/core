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


#include <comphelper/anytohash.hxx>

#include <o3tl/hash_combine.hxx>
#include <typelib/typedescription.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include "typedescriptionref.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::TypeDescription;
using ::comphelper::detail::TypeDescriptionRef;

namespace comphelper {
namespace {

std::optional<size_t> hashValue( size_t hash,
                  void const * val, typelib_TypeDescriptionReference * typeRef )
{
    o3tl::hash_combine( hash, typeRef->eTypeClass );
    if (typeRef->eTypeClass == typelib_TypeClass_VOID) {
        return hash;
    }
    assert(val != nullptr);

    switch (typeRef->eTypeClass) {
    case typelib_TypeClass_INTERFACE: {
        return std::nullopt; // not implemented
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION: {
        TypeDescription typeDescr( typeRef );
        if (!typeDescr.is())
            typeDescr.makeComplete();
        if (!typeDescr.is())
            return std::nullopt;

        typelib_CompoundTypeDescription * compType =
            reinterpret_cast< typelib_CompoundTypeDescription * >(
                typeDescr.get() );
        sal_Int32 nDescr = compType->nMembers;

        if (compType->pBaseTypeDescription) {
            std::optional<size_t> tmpHash = hashValue(
                hash, val, reinterpret_cast<
                typelib_TypeDescription * >(
                    compType->pBaseTypeDescription)->pWeakRef);
            if(!tmpHash.has_value())
                return std::nullopt;
            hash = *tmpHash;
        }

        typelib_TypeDescriptionReference ** ppTypeRefs =
            compType->ppTypeRefs;
        sal_Int32 * memberOffsets = compType->pMemberOffsets;

        for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
        {
            TypeDescriptionRef memberType( ppTypeRefs[ nPos ] );
            if (!memberType.is())
                return std::nullopt;

            std::optional<size_t> tmpHash = hashValue( hash,
                             static_cast< char const * >(
                                 val ) + memberOffsets[ nPos ],
                             memberType->pWeakRef );
            if(!tmpHash.has_value())
                return std::nullopt;
            hash = *tmpHash;
        }
        break;
    }
    case typelib_TypeClass_SEQUENCE: {
        TypeDescriptionRef typeDescr( typeRef );
        if (!typeDescr.is())
            return std::nullopt;

        typelib_TypeDescriptionReference * elementTypeRef =
            reinterpret_cast<
            typelib_IndirectTypeDescription * >(typeDescr.get())->pType;
        TypeDescriptionRef elementTypeDescr( elementTypeRef );
        if (!elementTypeDescr.is())
            return std::nullopt;

        sal_Int32 nElementSize = elementTypeDescr->nSize;
        uno_Sequence * seq =
            *static_cast< uno_Sequence * const * >(val);
        sal_Int32 nElements = seq->nElements;

        if (nElements > 0)
        {
            char const * pElements = seq->elements;
            for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
            {
                std::optional<size_t> tmpHash = hashValue( hash,
                    pElements + (nElementSize * nPos),
                    elementTypeDescr->pWeakRef );
                if(!tmpHash.has_value())
                    return std::nullopt;
                hash = *tmpHash;
            }
        }
        break;
    }
    case typelib_TypeClass_ANY: {
        uno_Any const * pAny = static_cast< uno_Any const * >(val);
        return hashValue( hash, pAny->pData, pAny->pType );
    }
    case typelib_TypeClass_TYPE: {
        OUString const & str = OUString::unacquired(
            &(*static_cast<
                     typelib_TypeDescriptionReference * const * >(val)
                        )->pTypeName );
        o3tl::hash_combine( hash, str.hashCode() );
        break;
    }
    case typelib_TypeClass_STRING: {
        OUString const & str = OUString::unacquired(
            static_cast< rtl_uString * const * >(val) );
        o3tl::hash_combine( hash, str.hashCode() );
        break;
    }
    case typelib_TypeClass_ENUM: {
        TypeDescription typeDescr( typeRef );
        if (!typeDescr.is())
            typeDescr.makeComplete();
        if (!typeDescr.is())
            return std::nullopt;

        o3tl::hash_combine( hash, *static_cast< int const * >(val));
        break;
    }
    case typelib_TypeClass_BOOLEAN:
        if (*static_cast< sal_Bool const * >(val))
            o3tl::hash_combine( hash, true );
        else
            o3tl::hash_combine( hash, false );
        break;
    case typelib_TypeClass_CHAR: {
        o3tl::hash_combine( hash, *static_cast< sal_Unicode const * >(val));
        break;
    }
    case typelib_TypeClass_FLOAT:
        o3tl::hash_combine( hash, *static_cast< float const * >(val) );
        break;
    case typelib_TypeClass_DOUBLE:
        o3tl::hash_combine( hash, *static_cast< double const * >(val) );
        break;
    case typelib_TypeClass_BYTE:
        o3tl::hash_combine( hash, *static_cast< sal_Int8 const * >(val) );
        break;
    case typelib_TypeClass_SHORT:
        o3tl::hash_combine( hash, *static_cast< sal_Int16 const * >(val) );
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        o3tl::hash_combine( hash, *static_cast< sal_uInt16 const * >(val) );
        break;
    case typelib_TypeClass_LONG:
        o3tl::hash_combine( hash, *static_cast< sal_Int32 const * >(val) );
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        o3tl::hash_combine( hash, *static_cast< sal_uInt32 const * >(val) );
        break;
    case typelib_TypeClass_HYPER:
        o3tl::hash_combine( hash, *static_cast< sal_Int64 const * >(val) );
        break;
    case typelib_TypeClass_UNSIGNED_HYPER:
        o3tl::hash_combine( hash, *static_cast< sal_uInt64 const * >(val) );
        break;
//     case typelib_TypeClass_UNKNOWN:
//     case typelib_TypeClass_SERVICE:
//     case typelib_TypeClass_MODULE:
    default:
        return std::nullopt;
    }
    return hash;
}

} // anon namespace


std::optional<size_t> anyToHash( uno::Any const & value )
{
    size_t hash = 0;
    return hashValue( hash, value.getValue(), value.getValueTypeRef());
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
