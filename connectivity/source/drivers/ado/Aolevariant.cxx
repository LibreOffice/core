/*************************************************************************
 *
 *  $RCSfile: Aolevariant.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-30 11:40:50 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _CONNECTIVITY_ADO_AOLEVARIANT_HXX_
#include "ado/Aolevariant.hxx"
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif



using namespace connectivity::ado;
OLEString::OLEString()
    :m_sStr(NULL)
{
}
OLEString::OLEString(const BSTR& _sBStr)
    :m_sStr(_sBStr)
{
}
OLEString::OLEString(const ::rtl::OUString& _sBStr)
{
    m_sStr = ::SysAllocString(_sBStr);
}
OLEString::~OLEString()
{
    if(m_sStr)
        ::SysFreeString(m_sStr);
}
OLEString& OLEString::operator=(const ::rtl::OUString& _rSrc)
{
    if(m_sStr)
        ::SysFreeString(m_sStr);
    m_sStr = ::SysAllocString(_rSrc);
    return *this;
}
OLEString& OLEString::operator=(const BSTR& _rSrc)
{
    if(m_sStr)
        ::SysFreeString(m_sStr);
    m_sStr = _rSrc;
    return *this;
}
OLEString::operator ::rtl::OUString() const
{
    return (m_sStr != NULL) ? ::rtl::OUString(m_sStr,::SysStringLen(m_sStr)) : ::rtl::OUString();
}
OLEString::operator BSTR() const
{
    return m_sStr;
}
BSTR* OLEString::operator &()
{
    return &m_sStr;
}
sal_Int32 OLEString::length() const
{
    return (m_sStr != NULL) ? ::SysStringLen(m_sStr) : 0;
}

OLEVariant::OLEVariant()
{
    VariantInit(this);
}
OLEVariant::OLEVariant(const VARIANT& varSrc)
{
    ::VariantInit(this);
    ::VariantCopy(this, const_cast<VARIANT*>(&varSrc));
}
OLEVariant::OLEVariant(const OLEVariant& varSrc)
{
    ::VariantInit(this);
    ::VariantCopy(this, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc)));
}

OLEVariant::OLEVariant(sal_Bool x)              {   VariantInit(this);  vt = VT_BOOL;   boolVal     = (x ? VARIANT_TRUE : VARIANT_FALSE);}
OLEVariant::OLEVariant(sal_Int8 n)              {   VariantInit(this);  vt = VT_I1;     bVal        = n;}
OLEVariant::OLEVariant(sal_Int16 n)             {   VariantInit(this);  vt = VT_I2;     intVal      = n;}
OLEVariant::OLEVariant(sal_Int32 n)             {   VariantInit(this);  vt = VT_I4;     lVal        = n;}
OLEVariant::OLEVariant(sal_Int64 x)             {   VariantInit(this);  vt = VT_I4;     lVal        = x;}

OLEVariant::OLEVariant(const rtl::OUString& us)
{
    ::VariantInit(this);
    vt      = VT_BSTR;
    bstrVal = SysAllocString(us);
}
OLEVariant::~OLEVariant()
{
    ::VariantClear(this);
} // clears all the memory that was allocated before

OLEVariant::OLEVariant(const ::com::sun::star::util::Date& x )
{
    VariantInit(this);
    vt      = VT_DATE;
    dblVal  = ::dbtools::DBTypeConversion::toDouble(x,::com::sun::star::util::Date(30,12,1899));
}
OLEVariant::OLEVariant(const ::com::sun::star::util::Time& x )
{
    VariantInit(this);
    vt      = VT_R8;
    dblVal  = ::dbtools::DBTypeConversion::toDouble(x);
}
OLEVariant::OLEVariant(const ::com::sun::star::util::DateTime& x )
{
    VariantInit(this);
    vt      = VT_R8;
    dblVal  = ::dbtools::DBTypeConversion::toDouble(x,::com::sun::star::util::Date(30,12,1899));
}
OLEVariant::OLEVariant(const float &x)
{
    VariantInit(this);
    vt      = VT_R4;
    fltVal  = x;
}
OLEVariant::OLEVariant(const double &x)
{
    VariantInit(this);
    vt      = VT_R8;
    dblVal  = x;
}


OLEVariant::OLEVariant(IDispatch* pDispInterface)
{
    VariantInit(this);
    vt = VT_DISPATCH;
    pdispVal = pDispInterface;
}

OLEVariant::OLEVariant(const ::com::sun::star::uno::Sequence< sal_Int8 >& x)
{
    VariantInit(this);
    vt      = VT_ARRAY|VT_UI1;
    parray  = SafeArrayCreateVector(VT_UI1, 0, x.getLength());
    const sal_Int8* pBegin = x.getConstArray();
    const sal_Int8* pEnd = pBegin + x.getLength();
    for(sal_Int32 i=0;pBegin != pEnd;++i,++pBegin)
        SafeArrayPutElement(parray,&i,&pBegin);
}

OLEVariant& OLEVariant::operator=(const OLEVariant& varSrc)
{
    VariantCopy(this, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc)));
    return *this;
}
// Assign a const VARIANT& (::VariantCopy handles everything)
//
OLEVariant& OLEVariant::operator=(const tagVARIANT& varSrc)
{
    ::VariantCopy(this, const_cast<VARIANT*>(&varSrc));

    return *this;
}

// Assign a const VARIANT* (::VariantCopy handles everything)
//
OLEVariant& OLEVariant::operator=(const VARIANT* pSrc)
{
    ::VariantCopy(this, const_cast<VARIANT*>(pSrc));

    return *this;
}

void OLEVariant::setByte(sal_uInt8 n)               {   VariantClear(this); vt = VT_UI1;    bVal        = n;}
void OLEVariant::setInt16(sal_Int16 n)              {   VariantClear(this); vt = VT_I2;     iVal        = n;}
void OLEVariant::setInt32(sal_Int32 n)              {   VariantClear(this); vt = VT_I4;     lVal        = n;}
void OLEVariant::setFloat(float f)                  {   VariantClear(this); vt = VT_R4;     fltVal      = f;}
void OLEVariant::setDouble(double d)                {   VariantClear(this); vt = VT_R8;     dblVal      = d;}
void OLEVariant::setDate(DATE d)                    {   VariantClear(this); vt = VT_DATE;   date        = d;}
void OLEVariant::setChar(unsigned char a)           {   VariantClear(this); vt = VT_UI1;    bVal        = a;}
void OLEVariant::setCurrency(double aCur)           {   VariantClear(this); vt = VT_CY;     set(aCur*10000);}
void OLEVariant::setBool(sal_Bool b)                {   VariantClear(this); vt = VT_BOOL;   boolVal     = b ? VARIANT_TRUE : VARIANT_FALSE;}
void OLEVariant::setString(const rtl::OUString& us){    VariantClear(this); vt = VT_BSTR;   bstrVal     = ::SysAllocString(us);}
void OLEVariant::setNoArg()                         {   VariantClear(this); vt = VT_ERROR;  scode       = DISP_E_PARAMNOTFOUND;}

void OLEVariant::setIDispatch(IDispatch* pDispInterface)
                                {   VariantClear(this); vt = VT_DISPATCH; pdispVal = pDispInterface;}

void OLEVariant::setNull()                  {   VariantClear(this); vt = VT_NULL;}
void OLEVariant::setEmpty()                 {   VariantClear(this); vt = VT_EMPTY;}

void OLEVariant::setUI1SAFEARRAYPtr(SAFEARRAY* pSafeAr)
                                {   VariantClear(this); vt = VT_ARRAY|VT_UI1; parray = pSafeAr; }

void OLEVariant::setArray(SAFEARRAY* pSafeArray, VARTYPE vtType)
                                {   VariantClear(this); vt = VT_ARRAY|vtType; parray = pSafeArray; }

sal_Bool OLEVariant::isNull() const  {  return (vt == VT_NULL);     }
sal_Bool OLEVariant::isEmpty() const {  return (vt == VT_EMPTY);    }

VARTYPE OLEVariant::getType() const { return vt; }

OLEVariant::operator ::com::sun::star::util::Date() const
{
    return isNull() ? ::com::sun::star::util::Date(30,12,1899) : ::dbtools::DBTypeConversion::toDate(getDate(),::com::sun::star::util::Date(30,12,1899));
}
OLEVariant::operator ::com::sun::star::util::Time() const
{
    return isNull() ? ::com::sun::star::util::Time() : ::dbtools::DBTypeConversion::toTime(getDate());
}
OLEVariant::operator ::com::sun::star::util::DateTime()const
{
    return isNull() ? ::com::sun::star::util::DateTime() : ::dbtools::DBTypeConversion::toDateTime(getDate(),::com::sun::star::util::Date(30,12,1899));
}

VARIANT_BOOL OLEVariant::VariantBool(sal_Bool bEinBoolean)
{
    return (bEinBoolean ? VARIANT_TRUE : VARIANT_FALSE);
}

void OLEVariant::CHS()
{
    cyVal.Lo  ^= (sal_uInt32)-1;
    cyVal.Hi ^= -1;
    cyVal.Lo++;
    if( !cyVal.Lo )
        cyVal.Hi++;
}

void OLEVariant::set(double n)
{
    if( n >= 0 )
    {
        cyVal.Hi = (sal_Int32)(n / (double)4294967296.0);
        cyVal.Lo  = (sal_uInt32)(n - ((double)cyVal.Hi * (double)4294967296.0));
    }
    else {
        cyVal.Hi = (sal_Int32)(-n / (double)4294967296.0);
        cyVal.Lo  = (sal_uInt32)(-n - ((double)cyVal.Hi * (double)4294967296.0));
        CHS();
    }
}

OLEVariant::operator rtl::OUString() const
{
    if (V_VT(this) == VT_BSTR)
        return V_BSTR(this);

    if(isNull())
        return ::rtl::OUString();

    OLEVariant varDest;

    varDest.ChangeType(VT_BSTR, this);

    return V_BSTR(&varDest);
}
// -----------------------------------------------------------------------------
OLEVariant::operator ::com::sun::star::uno::Sequence< sal_Int8 >() const
{
    ::com::sun::star::uno::Sequence< sal_Int8 > aRet;
    if(V_VT(this) == VT_BSTR)
    {
        OLEString sStr(V_BSTR(this));
        aRet = ::com::sun::star::uno::Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>((const wchar_t*)sStr),sizeof(sal_Unicode)*sStr.length());
    }
    else if(!isNull())
    {
        SAFEARRAY* pArray = getUI1SAFEARRAYPtr();

        if(pArray)
        {
            HRESULT hresult1,hresult2;
            long lBound,uBound;
            // Verify that the SafeArray is the proper shape.
            hresult1 = ::SafeArrayGetLBound(pArray, 1, &lBound);
            hresult2 = ::SafeArrayGetUBound(pArray, 1, &uBound);
            if(SUCCEEDED(hresult1) && SUCCEEDED(hresult2))
            {
                long nIndex = 0;
                long nCount = uBound-lBound+1;
                aRet.realloc(nCount);
                for(long i=0;i<nCount;++i)
                {
                    ::SafeArrayGetElement(pArray,&nIndex,(void*)aRet.getArray()[i]);
                }
            }
        }
    }

    return aRet;
}
// -----------------------------------------------------------------------------
::rtl::OUString OLEVariant::getString() const
{
    return isNull() ? ::rtl::OUString() : (rtl::OUString)*this;
}
// -----------------------------------------------------------------------------
sal_Bool OLEVariant::getBool() const
{
    if (V_VT(this) == VT_BOOL)
        return V_BOOL(this) == VARIANT_TRUE ? sal_True : sal_False;
    if(isNull())
        return sal_False;

    OLEVariant varDest;

    varDest.ChangeType(VT_BOOL, this);

    return V_BOOL(&varDest) == VARIANT_TRUE ? sal_True : sal_False;
}
// -----------------------------------------------------------------------------
IUnknown* OLEVariant::getIUnknown() const
{
    if (V_VT(this) == VT_UNKNOWN)
    {
        V_UNKNOWN(this)->AddRef();
        return V_UNKNOWN(this);
    }
    if(isNull())
        return NULL;

    OLEVariant varDest;

    varDest.ChangeType(VT_UNKNOWN, this);

    V_UNKNOWN(&varDest)->AddRef();
    return V_UNKNOWN(&varDest);
}
// -----------------------------------------------------------------------------
IDispatch* OLEVariant::getIDispatch() const
{
    if (V_VT(this) == VT_DISPATCH)
    {
        V_DISPATCH(this)->AddRef();
        return V_DISPATCH(this);
    }

    if(isNull())
        return NULL;

    OLEVariant varDest;

    varDest.ChangeType(VT_DISPATCH, this);

    V_DISPATCH(&varDest)->AddRef();
    return V_DISPATCH(&varDest);
}
// -----------------------------------------------------------------------------
sal_uInt8 OLEVariant::getByte() const
{
    if (V_VT(this) == VT_UI1)
        return V_UI1(this);

    if(isNull())
        return sal_Int8(0);
    OLEVariant varDest;

    varDest.ChangeType(VT_UI1, this);

    return V_UI1(&varDest);
}
// -----------------------------------------------------------------------------
sal_Int16 OLEVariant::getInt16() const
{
    if (V_VT(this) == VT_I2)
        return V_I2(this);

    if(isNull())
        return sal_Int16(0);
    OLEVariant varDest;

    varDest.ChangeType(VT_I2, this);

    return V_I2(&varDest);
}
// -----------------------------------------------------------------------------
sal_Int8 OLEVariant::getInt8() const
{
    if (V_VT(this) == VT_I1)
        return V_I1(this);

    if(isNull())
        return sal_Int8(0);

    OLEVariant varDest;

    varDest.ChangeType(VT_I1, this);

    return V_I1(&varDest);
}
// -----------------------------------------------------------------------------
sal_Int32 OLEVariant::getInt32() const
{
    if (V_VT(this) == VT_I4)
        return V_I4(this);

    if(isNull())
        return sal_Int32(0);

    OLEVariant varDest;

    varDest.ChangeType(VT_I4, this);

    return V_I4(&varDest);
}
// -----------------------------------------------------------------------------
sal_uInt32 OLEVariant::getUInt32() const
{
    if (V_VT(this) == VT_UI4)
        return V_UI4(this);

    if(isNull())
        return sal_uInt32(0);

    OLEVariant varDest;

    varDest.ChangeType(VT_UI4, this);

    return V_UI4(&varDest);
}
// -----------------------------------------------------------------------------
float OLEVariant::getFloat() const
{
    if (V_VT(this) == VT_R4)
        return V_R4(this);

    if(isNull())
        return float(0);
    OLEVariant varDest;

    varDest.ChangeType(VT_R4, this);

    return V_R4(&varDest);
}
// -----------------------------------------------------------------------------
double OLEVariant::getDouble() const
{
    if (V_VT(this) == VT_R8)
        return V_R8(this);

    if(isNull())
        return double(0);
    OLEVariant varDest;

    varDest.ChangeType(VT_R8, this);

    return V_R8(&varDest);
}
// -----------------------------------------------------------------------------
double OLEVariant::getDate() const
{
    if (V_VT(this) == VT_DATE)
        return V_DATE(this);

    if(isNull())
        return double(0);
    OLEVariant varDest;

    varDest.ChangeType(VT_DATE, this);

    return V_DATE(&varDest);
}
// -----------------------------------------------------------------------------
CY OLEVariant::getCurrency() const
{
    if (V_VT(this) == VT_CY)
        return V_CY(this);

    if(isNull())
    {
        CY aVar;
        return aVar;
    }
    OLEVariant varDest;

    varDest.ChangeType(VT_CY, this);

    return V_CY(&varDest);
}
// -----------------------------------------------------------------------------
SAFEARRAY* OLEVariant::getUI1SAFEARRAYPtr() const
{
    if (V_VT(this) == (VT_ARRAY|VT_UI1))
        return V_ARRAY(this);

    if(isNull())
        return (0);
    OLEVariant varDest;

    varDest.ChangeType((VT_ARRAY|VT_UI1), this);

    return V_ARRAY(&varDest);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
