/*************************************************************************
 *
 *  $RCSfile: types.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hjs $ $Date: 2001-03-13 18:21:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif

#include <memory.h>


//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;

//------------------------------------------------------------------------------
sal_Int32 getINT32(const Any& _rAny)
{
    sal_Int32 nReturn = 0;
    _rAny >>= nReturn;

    return nReturn;
}

//------------------------------------------------------------------------------
sal_Int16 getINT16(const Any& _rAny)
{
    sal_Int16 nReturn = 0;
    _rAny >>= nReturn;
    return nReturn;
}

//------------------------------------------------------------------------------
double getDouble(const Any& _rAny)
{
    double nReturn = 0.0;
    _rAny >>= nReturn;
    return nReturn;
}

//------------------------------------------------------------------------------
float getFloat(const Any& _rAny)
{
    float nReturn = 0.0;
    _rAny >>= nReturn;
    return nReturn;
}

//------------------------------------------------------------------------------
::rtl::OUString getString(const Any& _rAny)
{
    ::rtl::OUString nReturn;
    _rAny >>= nReturn;
    return nReturn;
}

//------------------------------------------------------------------------------
sal_Bool getBOOL(const Any& _rAny)
{
    sal_Bool nReturn = sal_False;
    if (_rAny.getValueType() == ::getCppuBooleanType())
        nReturn = *(sal_Bool*)_rAny.getValue();
    else
        OSL_ENSHURE(sal_False, "comphelper::getBOOL : invalid argument !");
    return nReturn;
}

//------------------------------------------------------------------------------
sal_Int32 getEnumAsINT32(const Any& _rAny) throw(IllegalArgumentException)
{
    sal_Int32 nReturn;
    if (! ::cppu::enum2int(nReturn,_rAny) )
        throw IllegalArgumentException();
    return nReturn;
}

//------------------------------------------------------------------------------
FontDescriptor  getDefaultFont()
{
    FontDescriptor aReturn;
    aReturn.Slant = FontSlant_DONTKNOW;
    aReturn.Underline = FontUnderline::DONTKNOW;
    aReturn.Strikeout = FontStrikeout::DONTKNOW;
    return aReturn;
}

//------------------------------------------------------------------------------
sal_Bool isAssignableFrom(const Type& _rAssignable, const Type& _rFrom)
{
    // getthe type lib descriptions
    typelib_TypeDescription* pAssignable = NULL;
    _rAssignable.getDescription(&pAssignable);

    typelib_TypeDescription* pFrom = NULL;
    _rFrom.getDescription(&pFrom);

    // and ask the type lib
    return typelib_typedescription_isAssignableFrom(pAssignable, pFrom);
}

//------------------------------------------------------------------
template<class TYPE>
sal_Bool tryCompare(const void* _pData, const Any& _rValue, sal_Bool& _bIdentical, TYPE& _rOut)
{
    sal_Bool bSuccess = _rValue >>= _rOut;
    _bIdentical = bSuccess && (_rOut == *reinterpret_cast<const TYPE*>(_pData));
    return bSuccess;
}

//------------------------------------------------------------------
sal_Bool tryCompare(const void* _pData, const Any& _rValue, sal_Bool& _bIdentical, sal_Char& _rOut)
{
    sal_Int8 nDummy;
    sal_Bool bSuccess = _rValue >>= nDummy;
    _rOut = (sal_Char)nDummy;
    _bIdentical = bSuccess && (_rOut == *reinterpret_cast<const sal_Bool*>(_pData));
    return bSuccess;
}

//------------------------------------------------------------------
sal_Bool compare_impl(const Type& _rType, const void* pData, const Any& _rValue)
{
    sal_Bool bRes = sal_True;

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
        sal_Bool bConversionSuccess = sal_False;
        switch (_rType.getTypeClass())
        {
            case TypeClass_VOID:
                bConversionSuccess = sal_True;
                bRes = _rValue.getValueType().getTypeClass() == TypeClass_VOID;
                break;
            case TypeClass_BOOLEAN:
            {
                sal_Bool aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_CHAR:
            {
                sal_Char aDummy;
                bConversionSuccess = tryCompare(pData, _rValue, bRes, aDummy);
                break;
            }
            case TypeClass_STRING:
            {
                ::rtl::OUString aDummy;
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
                sal_Int32 nAsInt32;
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
                        bRes = sal_False;
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
                else if (isA(_rType, static_cast< Sequence< ::rtl::OUString >* >(NULL)))
                {
                    Sequence< ::rtl::OUString > aTemp;
                    bConversionSuccess = _rValue >>= aTemp;
                    if (bConversionSuccess)
                    {
                        const Sequence< ::rtl::OUString >& rLeftSeq = *reinterpret_cast<const Sequence< ::rtl::OUString>*>(pData);
                        const Sequence< ::rtl::OUString >& rRightSeq = aTemp;
                        sal_uInt32 nSeqLen = rLeftSeq.getLength();
                        bRes = ( nSeqLen == rRightSeq.getLength() );
                        for ( sal_uInt32 n = 0; bRes && ( n < nSeqLen ); n++ )
                        {
                            const ::rtl::OUString& rS1 = rLeftSeq.getConstArray()[n];
                            const ::rtl::OUString& rS2 = rRightSeq.getConstArray()[n];
                            bRes = ( rS1 == rS2 );
                        }
                    }
                }
                break;
            default:
                bRes = sal_False;
        }

        bRes = bRes && bConversionSuccess;
    }
    return bRes;
}

//------------------------------------------------------------------------------
sal_Bool compare(const Any& rLeft, const Any& rRight)
{
    return compare_impl(rLeft.getValueType(), rLeft.getValue(), rRight);
}

//-------------------------------------------------------------------------
sal_Bool    operator ==(const FontDescriptor& _rLeft, const FontDescriptor& _rRight)
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

//-------------------------------------------------------------------------
Type getSequenceElementType(const Type& _rSequenceType)
{
    OSL_ENSHURE(_rSequenceType.getTypeClass() == TypeClass_SEQUENCE,
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

