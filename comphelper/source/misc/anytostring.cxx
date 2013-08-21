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


#include "comphelper/anytostring.hxx"
#include "osl/diagnose.h"
#include "rtl/ustrbuf.hxx"
#include "typelib/typedescription.h"
#include "com/sun/star/lang/XServiceInfo.hpp"

using namespace ::com::sun::star;

namespace comphelper {
namespace {

void appendTypeError(
    OUStringBuffer & buf, typelib_TypeDescriptionReference * typeRef )
{
    buf.append( "<cannot get type description of type " );
    buf.append( OUString::unacquired( &typeRef->pTypeName ) );
    buf.append( static_cast< sal_Unicode >('>') );
}

inline void appendChar( OUStringBuffer & buf, sal_Unicode c )
{
    if (c < ' ' || c > '~') {
        buf.append( "\\X" );
        OUString const s(
            OUString::number( static_cast< sal_Int32 >(c), 16 ) );
        for ( sal_Int32 f = 4 - s.getLength(); f > 0; --f )
            buf.append( static_cast< sal_Unicode >('0') );
        buf.append( s );
    }
    else {
        buf.append( c );
    }
}

//------------------------------------------------------------------------------
void appendValue( OUStringBuffer & buf,
                  void const * val, typelib_TypeDescriptionReference * typeRef,
                  bool prependType )
{
    if (typeRef->eTypeClass == typelib_TypeClass_VOID) {
        buf.append( "void" );
        return;
    }
    OSL_ASSERT( val != 0 );

    if (prependType &&
        typeRef->eTypeClass != typelib_TypeClass_STRING &&
        typeRef->eTypeClass != typelib_TypeClass_CHAR &&
        typeRef->eTypeClass != typelib_TypeClass_BOOLEAN)
    {
        buf.append( static_cast< sal_Unicode >('(') );
        buf.append( OUString::unacquired( &typeRef->pTypeName ) );
        buf.append( ") " );
    }

    switch (typeRef->eTypeClass) {
    case typelib_TypeClass_INTERFACE: {
        buf.append( static_cast<sal_Unicode>('@') );
        buf.append( reinterpret_cast< sal_Int64 >(
                        *static_cast< void * const * >(val) ), 16 );
        uno::Reference< lang::XServiceInfo > xServiceInfo(
            *static_cast< uno::XInterface * const * >(val),
            uno::UNO_QUERY );
        if (xServiceInfo.is()) {
            buf.append( " (ImplementationName = \"" );
            buf.append( xServiceInfo->getImplementationName() );
            buf.append( "\")" );
        }
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION: {
        buf.append( "{ " );
        typelib_TypeDescription * typeDescr = 0;
        typelib_typedescriptionreference_getDescription( &typeDescr, typeRef );
        if (typeDescr == 0 || !typelib_typedescription_complete( &typeDescr )) {
            appendTypeError( buf, typeRef );
        }
        else {
            typelib_CompoundTypeDescription * compType =
                reinterpret_cast< typelib_CompoundTypeDescription * >(
                    typeDescr );
            sal_Int32 nDescr = compType->nMembers;

            if (compType->pBaseTypeDescription) {
                appendValue(
                    buf, val, reinterpret_cast<
                    typelib_TypeDescription * >(
                        compType->pBaseTypeDescription)->pWeakRef, false );
                if (nDescr > 0)
                    buf.append( ", " );
            }

            typelib_TypeDescriptionReference ** ppTypeRefs =
                compType->ppTypeRefs;
            sal_Int32 * memberOffsets = compType->pMemberOffsets;
            rtl_uString ** ppMemberNames = compType->ppMemberNames;

            for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
            {
                buf.append( ppMemberNames[ nPos ] );
                buf.append( " = " );
                typelib_TypeDescription * memberType = 0;
                TYPELIB_DANGER_GET( &memberType, ppTypeRefs[ nPos ] );
                if (memberType == 0) {
                    appendTypeError( buf, ppTypeRefs[ nPos ] );
                }
                else {
                    appendValue( buf,
                                 static_cast< char const * >(
                                     val ) + memberOffsets[ nPos ],
                                 memberType->pWeakRef, true );
                    TYPELIB_DANGER_RELEASE( memberType );
                }
                if (nPos < (nDescr - 1))
                    buf.append( ", " );
            }
        }
        buf.append( " }" );
        if (typeDescr != 0)
            typelib_typedescription_release( typeDescr );
        break;
    }
    case typelib_TypeClass_SEQUENCE: {
        typelib_TypeDescription * typeDescr = 0;
        TYPELIB_DANGER_GET( &typeDescr, typeRef );
        if (typeDescr == 0) {
            appendTypeError( buf,typeRef );
        }
        else {
            typelib_TypeDescriptionReference * elementTypeRef =
                reinterpret_cast<
                typelib_IndirectTypeDescription * >(typeDescr)->pType;
            typelib_TypeDescription * elementTypeDescr = 0;
            TYPELIB_DANGER_GET( &elementTypeDescr, elementTypeRef );
            if (elementTypeDescr == 0)
            {
                appendTypeError( buf, elementTypeRef );
            }
            else
            {
                sal_Int32 nElementSize = elementTypeDescr->nSize;
                uno_Sequence * seq =
                    *static_cast< uno_Sequence * const * >(val);
                sal_Int32 nElements = seq->nElements;

                if (nElements > 0)
                {
                    buf.append( "{ " );
                    char const * pElements = seq->elements;
                    for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                    {
                        appendValue(
                            buf, pElements + (nElementSize * nPos),
                            elementTypeDescr->pWeakRef, false );
                        if (nPos < (nElements - 1))
                            buf.append( ", " );
                    }
                    buf.append( " }" );
                }
                else
                {
                    buf.append( "{}" );
                }
                TYPELIB_DANGER_RELEASE( elementTypeDescr );
            }
            TYPELIB_DANGER_RELEASE( typeDescr );
        }
        break;
    }
    case typelib_TypeClass_ANY: {
        buf.append( "{ " );
        uno_Any const * pAny = static_cast< uno_Any const * >(val);
        appendValue( buf, pAny->pData, pAny->pType, true );
        buf.append( " }" );
        break;
    }
    case typelib_TypeClass_TYPE:
        buf.append( (*reinterpret_cast<
                     typelib_TypeDescriptionReference * const * >(val)
                        )->pTypeName );
        break;
    case typelib_TypeClass_STRING: {
        buf.append( static_cast< sal_Unicode >('\"') );
        OUString const & str = OUString::unacquired(
            static_cast< rtl_uString * const * >(val) );
        sal_Int32 len = str.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            sal_Unicode c = str[ pos ];
            if (c == '\"')
                buf.append( "\\\"" );
            else if (c == '\\')
                buf.append( "\\\\" );
            else
                appendChar( buf, c );
        }
        buf.append( static_cast< sal_Unicode >('\"') );
        break;
    }
    case typelib_TypeClass_ENUM: {
        typelib_TypeDescription * typeDescr = 0;
        typelib_typedescriptionreference_getDescription( &typeDescr, typeRef );
        if (typeDescr == 0 || !typelib_typedescription_complete( &typeDescr )) {
            appendTypeError( buf, typeRef );
        }
        else
        {
            sal_Int32 * pValues =
                reinterpret_cast< typelib_EnumTypeDescription * >(
                    typeDescr )->pEnumValues;
            sal_Int32 nPos = reinterpret_cast< typelib_EnumTypeDescription * >(
                typeDescr )->nEnumValues;
            while (nPos--)
            {
                if (pValues[ nPos ] == *static_cast< int const * >(val))
                    break;
            }
            if (nPos >= 0)
            {
                buf.append( reinterpret_cast< typelib_EnumTypeDescription * >(
                                typeDescr )->ppEnumNames[ nPos ] );
            }
            else
            {
                buf.append( "?unknown enum value?" );
            }
        }
        if (typeDescr != 0)
            typelib_typedescription_release( typeDescr );
        break;
    }
    case typelib_TypeClass_BOOLEAN:
        if (*static_cast< sal_Bool const * >(val) != sal_False)
            buf.append( "true" );
        else
            buf.append( "false" );
        break;
    case typelib_TypeClass_CHAR: {
        buf.append( static_cast< sal_Unicode >('\'') );
        sal_Unicode c = *static_cast< sal_Unicode const * >(val);
        if (c == '\'')
            buf.append( "\\\'" );
        else if (c == '\\')
            buf.append( "\\\\" );
        else
            appendChar( buf, c );
        buf.append( static_cast< sal_Unicode >('\'') );
        break;
    }
    case typelib_TypeClass_FLOAT:
        buf.append( *static_cast< float const * >(val) );
        break;
    case typelib_TypeClass_DOUBLE:
        buf.append( *static_cast< double const * >(val) );
        break;
    case typelib_TypeClass_BYTE:
        buf.append( static_cast< sal_Int32 >(
                        *static_cast< sal_Int8 const * >(val) ) );
        break;
    case typelib_TypeClass_SHORT:
        buf.append( static_cast< sal_Int32 >(
                        *static_cast< sal_Int16 const * >(val) ) );
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        buf.append( static_cast< sal_Int32 >(
                        *static_cast< sal_uInt16 const * >(val) ) );
        break;
    case typelib_TypeClass_LONG:
        buf.append( *static_cast< sal_Int32 const * >(val) );
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        buf.append( static_cast< sal_Int64 >(
                        *static_cast< sal_uInt32 const * >(val) ) );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        buf.append( *static_cast< sal_Int64 const * >(val) );
        break;
//     case typelib_TypeClass_UNION:
//     case typelib_TypeClass_ARRAY:
//     case typelib_TypeClass_UNKNOWN:
//     case typelib_TypeClass_SERVICE:
//     case typelib_TypeClass_MODULE:
    default:
        buf.append( static_cast< sal_Unicode >('?') );
        break;
    }
}

} // anon namespace

//==============================================================================
OUString anyToString( uno::Any const & value )
{
    OUStringBuffer buf;
    appendValue( buf, value.getValue(), value.getValueTypeRef(), true );
    return buf.makeStringAndClear();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
