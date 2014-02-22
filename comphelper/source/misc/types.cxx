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

#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <osl/diagnose.h>
#include <typelib/typedescription.hxx>

#include <memory.h>


//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;


bool operator ==(const DateTime& _rLeft, const DateTime& _rRight)
{
    return ( _rLeft.NanoSeconds == _rRight.NanoSeconds) &&
    ( _rLeft.Seconds == _rRight.Seconds) &&
    ( _rLeft.Minutes == _rRight.Minutes) &&
    ( _rLeft.Hours == _rRight.Hours) &&
    ( _rLeft.Day == _rRight.Day) &&
    ( _rLeft.Month == _rRight.Month) &&
    ( _rLeft.Year == _rRight.Year) ;
}


bool operator ==(const Date& _rLeft, const Date& _rRight)
{
    return ( _rLeft.Day == _rRight.Day) &&
    ( _rLeft.Month == _rRight.Month) &&
    ( _rLeft.Year == _rRight.Year) ;
}


bool operator ==(const Time& _rLeft, const Time& _rRight)
{
    return ( _rLeft.NanoSeconds == _rRight.NanoSeconds) &&
    ( _rLeft.Seconds == _rRight.Seconds) &&
    ( _rLeft.Minutes == _rRight.Minutes) &&
    ( _rLeft.Hours == _rRight.Hours) ;
}


sal_Int64 getINT64(const Any& _rAny)
{
    sal_Int64 nReturn = 0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


sal_Int32 getINT32(const Any& _rAny)
{
    sal_Int32 nReturn = 0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


sal_Int16 getINT16(const Any& _rAny)
{
    sal_Int16 nReturn = 0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


double getDouble(const Any& _rAny)
{
    double nReturn = 0.0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


float getFloat(const Any& _rAny)
{
    float nReturn = 0.0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


OUString getString(const Any& _rAny)
{
    OUString nReturn;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


bool getBOOL(const Any& _rAny)
{
    bool nReturn = false;
    if (_rAny.getValueType() == ::getCppuBooleanType())
        nReturn = *(sal_Bool*)_rAny.getValue();
    else
        OSL_FAIL("comphelper::getBOOL : invalid argument !");
    return nReturn;
}


sal_Int32 getEnumAsINT32(const Any& _rAny) throw(IllegalArgumentException)
{
    sal_Int32 nReturn = 0;
    if (! ::cppu::enum2int(nReturn,_rAny) )
        throw IllegalArgumentException();
    return nReturn;
}


FontDescriptor  getDefaultFont()
{
    FontDescriptor aReturn;
    aReturn.Slant = FontSlant_DONTKNOW;
    aReturn.Underline = FontUnderline::DONTKNOW;
    aReturn.Strikeout = FontStrikeout::DONTKNOW;
    return aReturn;
}


bool isAssignableFrom(const Type& _rAssignable, const Type& _rFrom)
{
    // getthe type lib descriptions
    typelib_TypeDescription* pAssignable = NULL;
    _rAssignable.getDescription(&pAssignable);

    typelib_TypeDescription* pFrom = NULL;
    _rFrom.getDescription(&pFrom);

    // and ask the type lib
    return typelib_typedescription_isAssignableFrom(pAssignable, pFrom);
}


template<class TYPE>
bool tryCompare(const void* _pData, const Any& _rValue, bool& _bIdentical, TYPE& _rOut)
{
    bool bSuccess = _rValue >>= _rOut;
    _bIdentical = bSuccess && (_rOut == *reinterpret_cast<const TYPE*>(_pData));
    return bSuccess;
}


bool tryCompare(const void* _pData, const Any& _rValue, bool& _bIdentical, sal_Unicode& _rOut)
{
    bool bSuccess = ( _rValue.getValueTypeClass() == TypeClass_CHAR );
    if ( bSuccess )
        _rOut = *static_cast< const sal_Unicode* >( _rValue.getValue() );
    _bIdentical = bSuccess && ( _rOut == *static_cast< const sal_Unicode* >( _pData ) );
    return bSuccess;
}


bool compare_impl(const Type& _rType, const void* pData, const Any& _rValue)
{
    bool bRes = true;

    if (_rType.getTypeClass() == TypeClass_ANY)
    {
        // beides AnyWerte
        if (_rValue.getValueType().getTypeClass() == TypeClass_ANY)
            bRes = compare_impl(
                reinterpret_cast<const Any*>(pData)->getValueType(),
                reinterpret_cast<const Any*>(pData)->getValue(),
                *reinterpret_cast<const Any*>(_rValue.getValue()));
        else
            bRes = compare_impl(
                reinterpret_cast<const Any*>(pData)->getValueType(),
                reinterpret_cast<const Any*>(pData)->getValue(),
                _rValue);
    }
    else if (   (_rType.getTypeClass() == TypeClass_VOID)
            ||  (_rValue.getValueType().getTypeClass() == TypeClass_VOID)
            )
    {
        bRes = _rType.getTypeClass() == _rValue.getValueType().getTypeClass();
    }
    else
    {
        bool bConversionSuccess = false;
        switch (_rType.getTypeClass())
        {
            case TypeClass_VOID:
                bConversionSuccess = true;
                bRes = _rValue.getValueType().getTypeClass() == TypeClass_VOID;
                break;
            case TypeClass_BOOLEAN:
            {
                bool aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_CHAR:
            {
                sal_Unicode aDummy(0);
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_STRING:
            {
                OUString aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_FLOAT:
            {
                float aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_DOUBLE:
            {
                double aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_BYTE:
            {
                sal_Int8 aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_SHORT:
            {
                sal_Int16 aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_ENUM:
            {
                sal_Int32 nAsInt32 = 0;
                bConversionSuccess = ::cppu::enum2int(nAsInt32, _rValue);
                bRes = bConversionSuccess && (nAsInt32== *reinterpret_cast<const sal_Int32*>(pData));
                break;
            }
            case TypeClass_LONG:
            {
                sal_Int32 aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_UNSIGNED_SHORT:
            {
                sal_uInt16 aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_UNSIGNED_LONG:
            {
                sal_uInt32 aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_INTERFACE:
            {
                InterfaceRef aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_STRUCT:
                if (isA(_rType, static_cast<FontDescriptor*>(NULL)))
                {
                    FontDescriptor aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        bRes = *(FontDescriptor*)pData == aTemp;
                    }
                    else
                        bRes = false;
                    break;
                }
                if (isA(_rType, static_cast<Date*>(NULL)))
                {
                    Date aDummy;
                    bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                    break;
                }
                if (isA(_rType, static_cast<Time*>(NULL)))
                {
                    Time aDummy;
                    bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                    break;
                }
                if (isA(_rType, static_cast<DateTime*>(NULL)))
                {
                    DateTime aDummy;
                    bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                    break;
                }
                break;
            case TypeClass_SEQUENCE:
                if (isA(_rType, static_cast< Sequence<sal_Int8>* >(NULL)))
                {
                    Sequence<sal_Int8> aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence<sal_Int8>& rLeftSeq = *reinterpret_cast<const Sequence<sal_Int8>*>(pData);
                        const Sequence<sal_Int8>& rRightSeq = aTemp;
                        bRes = rLeftSeq.getLength() == rRightSeq.getLength() &&
                            memcmp(rLeftSeq.getConstArray(), rRightSeq.getConstArray(), rLeftSeq.getLength()) == 0;
                    }
                }
                else if (isA(_rType, static_cast< Sequence<sal_uInt8>* >(NULL)))
                {
                    Sequence<sal_uInt8> aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence<sal_uInt8>& rLeftSeq = *reinterpret_cast<const Sequence<sal_uInt8>*>(pData);
                        const Sequence<sal_uInt8>& rRightSeq = aTemp;
                        bRes = rLeftSeq.getLength() == rRightSeq.getLength() &&
                            memcmp(rLeftSeq.getConstArray(), rRightSeq.getConstArray(), rLeftSeq.getLength()) == 0;
                    }
                }
                else if (isA(_rType, static_cast< Sequence<sal_Int16>* >(NULL)))
                {
                    Sequence<sal_Int16> aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence<sal_Int16>& rLeftSeq = *reinterpret_cast<const Sequence<sal_Int16>*>(pData);
                        const Sequence<sal_Int16>& rRightSeq = aTemp;
                        bRes = rLeftSeq.getLength() == rRightSeq.getLength() &&
                            memcmp(rLeftSeq.getConstArray(), rRightSeq.getConstArray(), rLeftSeq.getLength()*sizeof(sal_Int16)) == 0;
                    }
                }
                else if (isA(_rType, static_cast< Sequence<sal_uInt16>* >(NULL)))
                {
                    Sequence<sal_uInt16> aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence<sal_uInt16>& rLeftSeq = *reinterpret_cast<const Sequence<sal_uInt16>*>(pData);
                        const Sequence<sal_uInt16>& rRightSeq = aTemp;
                        bRes = rLeftSeq.getLength() == rRightSeq.getLength() &&
                            memcmp(rLeftSeq.getConstArray(), rRightSeq.getConstArray(), rLeftSeq.getLength()*sizeof(sal_uInt16)) == 0;
                    }
                }
                else if (isA(_rType, static_cast< Sequence<sal_Int32>* >(NULL)))
                {
                    Sequence<sal_Int32> aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence<sal_Int32>& rLeftSeq = *reinterpret_cast<const Sequence<sal_Int32>*>(pData);
                        const Sequence<sal_Int32>& rRightSeq = aTemp;
                        bRes = rLeftSeq.getLength() == rRightSeq.getLength() &&
                            memcmp(rLeftSeq.getConstArray(), rRightSeq.getConstArray(), rLeftSeq.getLength()*sizeof(sal_Int32)) == 0;
                    }
                }
                else if (isA(_rType, static_cast< Sequence<sal_uInt32>* >(NULL)))
                {
                    Sequence<sal_uInt32> aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence<sal_uInt32>& rLeftSeq = *reinterpret_cast<const Sequence<sal_uInt32>*>(pData);
                        const Sequence<sal_uInt32>& rRightSeq = aTemp;
                        bRes = rLeftSeq.getLength() == rRightSeq.getLength() &&
                            memcmp(rLeftSeq.getConstArray(), rRightSeq.getConstArray(), rLeftSeq.getLength()*sizeof(sal_uInt32)) == 0;
                    }
                }
                else if (isA(_rType, static_cast< Sequence< OUString >* >(NULL)))
                {
                    Sequence< OUString > aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence< OUString >& rLeftSeq = *reinterpret_cast<const Sequence< OUString>*>(pData);
                        const Sequence< OUString >& rRightSeq = aTemp;
                        sal_Int32 nSeqLen = rLeftSeq.getLength();
                        bRes = ( nSeqLen == rRightSeq.getLength() );
                        for ( sal_Int32 n = 0; bRes && ( n < nSeqLen ); n++ )
                        {
                            const OUString& rS1 = rLeftSeq.getConstArray()[n];
                            const OUString& rS2 = rRightSeq.getConstArray()[n];
                            bRes = ( rS1 == rS2 );
                        }
                    }
                }
                break;
            default:
                bRes = false;
        }

        bRes = bRes && bConversionSuccess;
    }
    return bRes;
}


bool compare(const Any& rLeft, const Any& rRight)
{
    return compare_impl(rLeft.getValueType(), rLeft.getValue(), rRight);
}


bool    operator ==(const FontDescriptor& _rLeft, const FontDescriptor& _rRight)
{
    return ( _rLeft.Name.equals( _rRight.Name ) ) &&
    ( _rLeft.Height == _rRight.Height ) &&
    ( _rLeft.Width == _rRight.Width ) &&
    ( _rLeft.StyleName.equals( _rRight.StyleName ) ) &&
    ( _rLeft.Family == _rRight.Family ) &&
    ( _rLeft.CharSet == _rRight.CharSet ) &&
    ( _rLeft.Pitch == _rRight.Pitch ) &&
    ( _rLeft.CharacterWidth == _rRight.CharacterWidth ) &&
    ( _rLeft.Weight == _rRight.Weight ) &&
    ( _rLeft.Slant == _rRight.Slant ) &&
    ( _rLeft.Underline == _rRight.Underline ) &&
    ( _rLeft.Strikeout == _rRight.Strikeout ) &&
    ( _rLeft.Orientation == _rRight.Orientation ) &&
    ( _rLeft.Kerning == _rRight.Kerning ) &&
    ( _rLeft.WordLineMode == _rRight.WordLineMode ) &&
    ( _rLeft.Type == _rRight.Type ) ;
}


Type getSequenceElementType(const Type& _rSequenceType)
{
    OSL_ENSURE(_rSequenceType.getTypeClass() == TypeClass_SEQUENCE,
                "getSequenceElementType: must be called with a  sequence type!");

    if (!(_rSequenceType.getTypeClass() == TypeClass_SEQUENCE))
        return Type();

    TypeDescription aTD(_rSequenceType);
    typelib_IndirectTypeDescription* pSequenceTD =
        reinterpret_cast< typelib_IndirectTypeDescription* >(aTD.get());

    OSL_ASSERT(pSequenceTD);
    OSL_ASSERT(pSequenceTD->pType);

    if (pSequenceTD && pSequenceTD->pType)
        return Type(pSequenceTD->pType);

    return Type();
}
//.........................................................................
}   // namespace comphelper
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
