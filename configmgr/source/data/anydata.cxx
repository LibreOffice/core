/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anydata.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:11:53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "anydata.hxx"

#ifndef INCLUDED_SHARABLE_SEQUENCE_HXX
#include "sequence.hxx"
#endif
#ifndef INCLUDED_DATA_FLAGS_HXX
#include "flags.hxx"
#endif
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
//-----------------------------------------------------------------------------
        namespace Type = data::Type;
        namespace uno = ::com::sun::star::uno;
        typedef AnyData::TypeCode TypeCode;
//-----------------------------------------------------------------------------

TypeCode getTypeCode(uno::Type const & _aType)
{
    switch (_aType.getTypeClass())
    {
    case uno::TypeClass_ANY:
    case uno::TypeClass_VOID:
        return Type::value_any;

    case uno::TypeClass_STRING:
        return Type::value_string;

    case uno::TypeClass_BOOLEAN:
        return Type::value_boolean;

    case uno::TypeClass_SHORT:
        return Type::value_short;

    case uno::TypeClass_LONG:
        return Type::value_int;

    case uno::TypeClass_HYPER:
        return Type::value_long;

    case uno::TypeClass_FLOAT:
    case uno::TypeClass_DOUBLE:
        return Type::value_double;

    case uno::TypeClass_SEQUENCE:
        {
            uno::Type aElementType = getSequenceElementType(_aType);

            if (aElementType.getTypeClass() == uno::TypeClass_BYTE)
                return Type::value_binary;

            OSL_ASSERT(aElementType != _aType); // would cause infinite recursion
            TypeCode aElementTC = getTypeCode(aElementType);

            OSL_ASSERT(Type::value_invalid & Type::flag_sequence); // ensure check works for invalid types

            if (aElementTC & Type::flag_sequence) // no sequence of sequence
                return Type::value_invalid;

            return TypeCode( aElementTC | Type::flag_sequence );
        }
    default:
        return Type::value_invalid;
    }
}
//-----------------------------------------------------------------------------

static uno::Type getUnoSimpleType( TypeCode _aSimpleType)
{
    OSL_ENSURE( _aSimpleType == (_aSimpleType & Type::mask_basetype), "Invalid type code" );

    switch (_aSimpleType)
    {
    case Type::value_string:
        return ::getCppuType(static_cast<rtl::OUString const *>(0));

    case Type::value_boolean:
        return ::getBooleanCppuType();

    case Type::value_short:
        return ::getCppuType(static_cast<sal_Int16 const *>(0));

    case Type::value_int:
        return ::getCppuType(static_cast<sal_Int32 const *>(0));

    case Type::value_long:
        return ::getCppuType(static_cast<sal_Int64 const *>(0));

    case Type::value_double:
        return ::getCppuType(static_cast<double const *>(0));

    case Type::value_binary:
        return ::getCppuType(static_cast<uno::Sequence<sal_Int8> const *>(0));

    case Type::value_any:
        //return ::getVoidCppuType();
        return ::getCppuType(static_cast<uno::Any const *>(0));

    default:
        OSL_ENSURE( false, "Invalid type code" );
        return ::getVoidCppuType();
    }
}
//-----------------------------------------------------------------------------

static uno::Type getUnoSequenceType( TypeCode _aSimpleType)
{
    OSL_ENSURE( _aSimpleType == (_aSimpleType & Type::mask_basetype), "Invalid type code" );

    switch (_aSimpleType)
    {
    case Type::value_string:
        return ::getCppuType(static_cast<uno::Sequence<rtl::OUString> const *>(0));

    case Type::value_boolean:
        return ::getCppuType(static_cast<uno::Sequence<sal_Bool> const *>(0));

    case Type::value_short:
        return ::getCppuType(static_cast<uno::Sequence<sal_Int16> const *>(0));

    case Type::value_int:
        return ::getCppuType(static_cast<uno::Sequence<sal_Int32> const *>(0));

    case Type::value_long:
        return ::getCppuType(static_cast<uno::Sequence<sal_Int64> const *>(0));

    case Type::value_double:
        return ::getCppuType(static_cast<uno::Sequence<double> const *>(0));

    case Type::value_binary:
        return ::getCppuType(static_cast<uno::Sequence<uno::Sequence<sal_Int8> > const *>(0));

    case Type::value_any: // results from value_invalid
    default:
        OSL_ENSURE( false, "Invalid type code" );
        return ::getVoidCppuType();
    }
}
//-----------------------------------------------------------------------------

uno::Type getUnoType( TypeCode _aType)
{
    OSL_ENSURE( _aType == (_aType & Type::mask_valuetype), "Invalid type code" );

    if (_aType & Type::flag_sequence)
        return getUnoSequenceType( TypeCode(_aType & Type::mask_basetype));

    else
        return getUnoSimpleType(_aType);
}
//-----------------------------------------------------------------------------

static
AnyData allocSimpleData(TypeCode _aSimpleType, uno::Any const & _aAny)
{
    OSL_ENSURE( _aSimpleType == (_aSimpleType & Type::mask_basetype), "Invalid type code" );

    AnyData aResult;
    aResult.data = 0;

    switch (_aSimpleType)
    {
    case Type::value_string:
        {
            rtl::OUString sValue;
            OSL_VERIFY(_aAny >>= sValue );
            aResult.stringValue = allocString(sValue);
        }
        break;

    case Type::value_boolean:
        OSL_VERIFY(_aAny >>= aResult.boolValue );
        break;

    case Type::value_short:
        OSL_VERIFY(_aAny >>= aResult.shortValue);
        break;

    case Type::value_int:
        OSL_VERIFY(_aAny >>= aResult.intValue);
        break;

    case Type::value_long:
        {
            sal_Int64 nValue = 0;
            OSL_VERIFY(_aAny >>= nValue);

            aResult.longValue = new sal_Int64( nValue );
        }
        break;

    case Type::value_double:
        {
            double dValue = 0;
            OSL_VERIFY(_aAny >>= dValue);

            aResult.doubleValue = new double( dValue );
        }
        break;

    case Type::value_binary:
        {
            uno::Sequence<sal_Int8> aValue;
            OSL_VERIFY(_aAny >>= aValue);
            aResult.binaryValue = allocBinary(aValue);
        }
        break;

    case Type::value_any:
        OSL_ENSURE( false, "Trying to allocate void value" );
        break;

    default:
        OSL_ENSURE( false, "Invalid type code" );
        break;
    }

    return aResult;
}
//-----------------------------------------------------------------------------

template <class E>
inline
sal_Sequence const * extractSequenceData(uno::Sequence< E > & _rSeq, uno::Any const & _aAny)
{
    if (_aAny >>= _rSeq)
    {
        return _rSeq.get();
    }

    else
    {
        OSL_ENSURE(false, "Could not extract sequence from Any");
        return NULL;
    }
}
//-----------------------------------------------------------------------------

static
AnyData allocSequenceData(TypeCode _aSimpleType, uno::Any const & _aAny)
{
    OSL_ENSURE( _aSimpleType == (_aSimpleType & Type::mask_basetype), "Invalid type code" );

    Sequence aSequence = 0;

    switch (_aSimpleType)
    {
    case Type::value_string:
        {
            uno::Sequence<rtl::OUString> aSeqValue;
            if (sal_Sequence const * pData = extractSequenceData(aSeqValue,_aAny))
                aSequence = allocSequence(_aSimpleType,pData);
        }
        break;

    case Type::value_boolean:
        {
            uno::Sequence<sal_Bool> aSeqValue;
            if (sal_Sequence const * pData = extractSequenceData(aSeqValue,_aAny))
                aSequence = allocSequence(_aSimpleType,pData);
        }
        break;

    case Type::value_short:
        {
            uno::Sequence<sal_Int16> aSeqValue;
            if (sal_Sequence const * pData = extractSequenceData(aSeqValue,_aAny))
                aSequence = allocSequence(_aSimpleType,pData);
        }
        break;

    case Type::value_int:
        {
            uno::Sequence<sal_Int32> aSeqValue;
            if (sal_Sequence const * pData = extractSequenceData(aSeqValue,_aAny))
                aSequence = allocSequence(_aSimpleType,pData);
        }
        break;

    case Type::value_long:
        {
            uno::Sequence<sal_Int64> aSeqValue;
            if (sal_Sequence const * pData = extractSequenceData(aSeqValue,_aAny))
                aSequence = allocSequence(_aSimpleType,pData);
        }
        break;

    case Type::value_double:
        {
            uno::Sequence<double> aSeqValue;
            if (sal_Sequence const * pData = extractSequenceData(aSeqValue,_aAny))
                aSequence = allocSequence(_aSimpleType,pData);
        }
        break;

    case Type::value_binary:
        {
            uno::Sequence<uno::Sequence<sal_Int8> > aSeqValue;
            if (sal_Sequence const * pData = extractSequenceData(aSeqValue,_aAny))
                aSequence = allocSequence(_aSimpleType,pData);
        }
        break;

    case Type::value_any: // results from value_invalid
    default:
        OSL_ENSURE( false, "Invalid type code" );
        break;
    }

    AnyData aResult;
    aResult.sequenceValue = aSequence;
    return aResult;
}
//-----------------------------------------------------------------------------

AnyData allocData(TypeCode _aType, uno::Any const & _aAny)
{
    OSL_ENSURE( _aType == (_aType & Type::mask_valuetype), "Invalid type code" );
    OSL_ENSURE( _aType == getTypeCode(_aAny.getValueType()), "Type code does not match value" );

    if (_aType & Type::flag_sequence)
        return allocSequenceData(TypeCode( _aType & Type::mask_basetype),_aAny);
    else
        return allocSimpleData(_aType,_aAny);
}
//-----------------------------------------------------------------------------

static
void freeSimpleData(TypeCode _aSimpleType, AnyData const & _aData)
{
    OSL_ENSURE( _aSimpleType == (_aSimpleType & Type::mask_basetype), "Invalid type code" );

    switch (_aSimpleType)
    {
    case Type::value_string:
        freeString(_aData.stringValue);
        break;

    case Type::value_boolean:
    case Type::value_short:
    case Type::value_int:
        // nothing to do
        break;

        // free memory for oversized values
    case Type::value_long:
        delete _aData.longValue;
        break;

    case Type::value_double:
        delete _aData.doubleValue;
        break;

    case Type::value_binary:
        freeBinary(_aData.binaryValue);
        break;

    case Type::value_any:
        // nothing to do for void value
        break;

    default:
        OSL_ENSURE( false, "Invalid type code" );
        break;
    }
}
//-----------------------------------------------------------------------------

void    freeData(TypeCode _aType, AnyData _aData)
{
    OSL_ENSURE( _aType == (_aType & Type::mask_valuetype), "Invalid type code" );

    if (_aType & Type::flag_sequence)
        freeSequence(TypeCode(_aType & Type::mask_basetype),_aData.sequenceValue);

    else
        freeSimpleData(_aType,_aData);
}
//-----------------------------------------------------------------------------

static
uno::Any readSimpleData(TypeCode _aSimpleType, AnyData const & _aData)
{
    OSL_ENSURE( _aSimpleType == (_aSimpleType & Type::mask_basetype), "Invalid type code" );

    switch (_aSimpleType)
    {
    case Type::value_string:
        {
            rtl::OUString sValue = readString(_aData.stringValue);
            return uno::makeAny(sValue);
        }

    case Type::value_boolean:
        return uno::makeAny( _aData.boolValue );

    case Type::value_short:
        return uno::makeAny( _aData.shortValue );

    case Type::value_int:
        return uno::makeAny( _aData.intValue );

    case Type::value_long:
    return uno::makeAny( *_aData.longValue );

    case Type::value_double:
    return uno::makeAny( *_aData.doubleValue );

    case Type::value_binary:
        {
            uno::Sequence<sal_Int8> aValue = readBinary( _aData.binaryValue );
            return uno::makeAny( aValue );
        }

    case Type::value_any: // void value
        return uno::Any();

    default:
        OSL_ENSURE( false, "Invalid type code" );
        return uno::Any();
    }
}
//-----------------------------------------------------------------------------

uno::Any readData(TypeCode _aType, AnyData _aData)
{
    OSL_ENSURE( _aType == (_aType & Type::mask_valuetype), "Invalid type code" );

    if (_aType & Type::flag_sequence)
        return readAnySequence(TypeCode(_aType & Type::mask_basetype),_aData.sequenceValue);

    else
        return readSimpleData(_aType,_aData);
}

//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
} // namespace

