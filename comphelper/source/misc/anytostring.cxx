/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anytostring.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:48:34 $
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

#include "comphelper/anytostring.hxx"

#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "typelib/typedescription.h"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace comphelper
{

//------------------------------------------------------------------------------
static void appendTypeError(
    OUStringBuffer & buf, typelib_TypeDescriptionReference * typeRef )
{
    buf.appendAscii(
        RTL_CONSTASCII_STRINGPARAM("<cannot get type description of type ") );
    buf.append( OUString::unacquired( &typeRef->pTypeName ) );
    buf.append( static_cast< sal_Unicode >('>') );
}

//------------------------------------------------------------------------------
static inline void appendChar( OUStringBuffer & buf, sal_Unicode c )
{
    if (c < ' ' || c > '~')
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\\X") );
        OUString s( OUString::valueOf( static_cast< sal_Int32 >(c), 16 ) );
        for ( sal_Int32 f = 4 - s.getLength(); f > 0; --f )
            buf.append( static_cast< sal_Unicode >('0') );
        buf.append( s );
    }
    else
    {
        buf.append( c );
    }
}

//------------------------------------------------------------------------------
static void appendValue(
    OUStringBuffer & buf,
    void const * val, typelib_TypeDescriptionReference * typeRef,
    bool prependType )
{
    if (typeRef->eTypeClass == typelib_TypeClass_VOID)
    {
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
        buf.append( OUString::unacquired( &typeRef->pTypeName ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(") ") );
    }

    switch (typeRef->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE:
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
        buf.append( reinterpret_cast< sal_Int64 >(
                        *reinterpret_cast< void * const * >(val) ), 16 );
        Reference< lang::XServiceInfo > xServiceInfo(
            *reinterpret_cast< XInterface * const * >(val), UNO_QUERY );
        if (xServiceInfo.is())
        {
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                 " (ImplementationName = \"") );
            buf.append( xServiceInfo->getImplementationName() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\")") );
        }
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
        typelib_TypeDescription * typeDescr = 0;
        typelib_typedescriptionreference_getDescription( &typeDescr, typeRef );
        if (typeDescr == 0 || !typelib_typedescription_complete( &typeDescr )) {
            appendTypeError( buf, typeRef );
        }
        else
        {
            typelib_CompoundTypeDescription * compType =
                reinterpret_cast< typelib_CompoundTypeDescription * >(
                    typeDescr );
            sal_Int32 nDescr = compType->nMembers;

            if (compType->pBaseTypeDescription)
            {
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
                if (memberType == 0)
                {
                    appendTypeError( buf, ppTypeRefs[ nPos ] );
                }
                else
                {
                    appendValue( buf,
                                 reinterpret_cast< char const * >(
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
    case typelib_TypeClass_SEQUENCE:
    {
        typelib_TypeDescription * typeDescr = 0;
        TYPELIB_DANGER_GET( &typeDescr, typeRef );
        if (typeDescr == 0)
        {
            appendTypeError( buf,typeRef );
        }
        else
        {
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
                    *reinterpret_cast< uno_Sequence * const * >(val);
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
    case typelib_TypeClass_ANY:
    {
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("{ ") );
        uno_Any const * pAny = reinterpret_cast< uno_Any const * >(val);
        appendValue( buf, pAny->pData, pAny->pType, true );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" }") );
        break;
    }
    case typelib_TypeClass_TYPE:
        buf.append( (*reinterpret_cast<
                     typelib_TypeDescriptionReference * const * >(val)
                        )->pTypeName );
        break;
    case typelib_TypeClass_STRING:
    {
        buf.append( static_cast< sal_Unicode >('\"') );
        OUString const & str = OUString::unacquired(
            reinterpret_cast< rtl_uString * const * >(val) );
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
    case typelib_TypeClass_ENUM:
    {
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
                if (pValues[ nPos ] == *reinterpret_cast< int const * >(val))
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
        if (*reinterpret_cast< sal_Bool const * >(val) != sal_False)
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("true") );
        else
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("false") );
        break;
    case typelib_TypeClass_CHAR:
    {
        buf.append( static_cast< sal_Unicode >('\'') );
        sal_Unicode c = *reinterpret_cast< sal_Unicode const * >(val);
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
        buf.append( *reinterpret_cast< float const * >(val) );
        break;
    case typelib_TypeClass_DOUBLE:
        buf.append( *reinterpret_cast< double const * >(val) );
        break;
    case typelib_TypeClass_BYTE:
        buf.append( static_cast< sal_Int32 >(
                        *reinterpret_cast< sal_Int8 const * >(val) ) );
        break;
    case typelib_TypeClass_SHORT:
        buf.append( static_cast< sal_Int32 >(
                        *reinterpret_cast< sal_Int16 const * >(val) ) );
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        buf.append( static_cast< sal_Int32 >(
                        *reinterpret_cast< sal_uInt16 const * >(val) ) );
        break;
    case typelib_TypeClass_LONG:
        buf.append( *reinterpret_cast< sal_Int32 const * >(val) );
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        buf.append( static_cast< sal_Int64 >(
                        *reinterpret_cast< sal_uInt32 const * >(val) ) );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        buf.append( *reinterpret_cast< sal_Int64 const * >(val) );
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

//==============================================================================
OUString anyToString( Any const & value )
{
    OUStringBuffer buf;
    appendValue( buf, value.getValue(), value.getValueTypeRef(), true );
    return buf.makeStringAndClear();
}

}

