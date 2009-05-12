/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anytostring.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_comphelper.hxx"

#include "comphelper/anytostring.hxx"
#include "osl/diagnose.h"
#include "rtl/ustrbuf.hxx"
#include "typelib/typedescription.h"
#include "com/sun/star/lang/XServiceInfo.hpp"

using namespace ::com::sun::star;

namespace comphelper {
namespace {

void appendTypeError(
    rtl::OUStringBuffer & buf, typelib_TypeDescriptionReference * typeRef )
{
    buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM("<cannot get type description of type ") );
    buf.append( rtl::OUString::unacquired( &typeRef->pTypeName ) );
    buf.append( static_cast< sal_Unicode >('>') );
}

inline void appendChar( rtl::OUStringBuffer & buf, sal_Unicode c )
{
    if (c < ' ' || c > '~') {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\\X") );
        rtl::OUString const s(
            rtl::OUString::valueOf( static_cast< sal_Int32 >(c), 16 ) );
        for ( sal_Int32 f = 4 - s.getLength(); f > 0; --f )
            buf.append( static_cast< sal_Unicode >('0') );
        buf.append( s );
    }
    else {
        buf.append( c );
    }
}

//------------------------------------------------------------------------------
void appendValue( rtl::OUStringBuffer & buf,
                  void const * val, typelib_TypeDescriptionReference * typeRef,
                  bool prependType )
{
    if (typeRef->eTypeClass == typelib_TypeClass_VOID) {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("void") );
        return;
    }
    OSL_ASSERT( val != 0 );

    if (prependType &&
        typeRef->eTypeClass != typelib_TypeClass_STRING &&
        typeRef->eTypeClass != typelib_TypeClass_CHAR &&
        typeRef->eTypeClass != typelib_TypeClass_BOOLEAN)
    {
        buf.append( static_cast< sal_Unicode >('(') );
        buf.append( rtl::OUString::unacquired( &typeRef->pTypeName ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(") ") );
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
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                 " (ImplementationName = \"") );
            buf.append( xServiceInfo->getImplementationName() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
        }
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION: {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
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
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
            }

            typelib_TypeDescriptionReference ** ppTypeRefs =
                compType->ppTypeRefs;
            sal_Int32 * memberOffsets = compType->pMemberOffsets;
            rtl_uString ** ppMemberNames = compType->ppMemberNames;

            for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
            {
                buf.append( ppMemberNames[ nPos ] );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" = ") );
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
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
            }
        }
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" }") );
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
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
                    char const * pElements = seq->elements;
                    for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
                    {
                        appendValue(
                            buf, pElements + (nElementSize * nPos),
                            elementTypeDescr->pWeakRef, false );
                        if (nPos < (nElements - 1))
                            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
                    }
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" }") );
                }
                else
                {
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{}") );
                }
                TYPELIB_DANGER_RELEASE( elementTypeDescr );
            }
            TYPELIB_DANGER_RELEASE( typeDescr );
        }
        break;
    }
    case typelib_TypeClass_ANY: {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
        uno_Any const * pAny = static_cast< uno_Any const * >(val);
        appendValue( buf, pAny->pData, pAny->pType, true );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" }") );
        break;
    }
    case typelib_TypeClass_TYPE:
        buf.append( (*reinterpret_cast<
                     typelib_TypeDescriptionReference * const * >(val)
                        )->pTypeName );
        break;
    case typelib_TypeClass_STRING: {
        buf.append( static_cast< sal_Unicode >('\"') );
        rtl::OUString const & str = rtl::OUString::unacquired(
            static_cast< rtl_uString * const * >(val) );
        sal_Int32 len = str.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            sal_Unicode c = str[ pos ];
            if (c == '\"')
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\\\"") );
            else if (c == '\\')
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\\\\") );
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
                buf.appendAscii(
                    RTL_CONSTASCII_STRINGPARAM("?unknown enum value?") );
            }
        }
        if (typeDescr != 0)
            typelib_typedescription_release( typeDescr );
        break;
    }
    case typelib_TypeClass_BOOLEAN:
        if (*static_cast< sal_Bool const * >(val) != sal_False)
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("true") );
        else
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("false") );
        break;
    case typelib_TypeClass_CHAR: {
        buf.append( static_cast< sal_Unicode >('\'') );
        sal_Unicode c = *static_cast< sal_Unicode const * >(val);
        if (c == '\'')
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\\\'") );
        else if (c == '\\')
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\\\\") );
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
rtl::OUString anyToString( uno::Any const & value )
{
    rtl::OUStringBuffer buf;
    appendValue( buf, value.getValue(), value.getValueTypeRef(), true );
    return buf.makeStringAndClear();
}

} // namespace comphelper

