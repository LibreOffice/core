/*************************************************************************
 *
 *  $RCSfile: Aolevariant.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-17 09:19:41 $
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
#ifndef _CONNECTIVITY_ADO_AOLEVARIANT_HXX_
#define _CONNECTIVITY_ADO_AOLEVARIANT_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#include <oaidl.h>
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include "connectivity/DateConversion.hxx"
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif

namespace connectivity
{
    namespace ado
    {
        class OLEString
        {
            BSTR m_sStr;
        public:
            OLEString()
                :m_sStr(NULL)
            {
            }
            OLEString(const BSTR& _sBStr)
                :m_sStr(_sBStr)
            {
            }
            OLEString(const ::rtl::OUString& _sBStr)
            {
                m_sStr = ::SysAllocString(_sBStr);
            }
            ~OLEString()
            {
                if(m_sStr)
                    ::SysFreeString(m_sStr);
            }
            OLEString& operator=(const ::rtl::OUString& _rSrc)
            {
                if(m_sStr)
                    ::SysFreeString(m_sStr);
                m_sStr = ::SysAllocString(_rSrc);
                return *this;
            }
            OLEString& operator=(const BSTR& _rSrc)
            {
                if(m_sStr)
                    ::SysFreeString(m_sStr);
                m_sStr = _rSrc;
                return *this;
            }
            operator ::rtl::OUString() const
            {
                return (m_sStr != NULL) ? ::rtl::OUString(m_sStr,::SysStringLen(m_sStr)) : ::rtl::OUString();
            }
            operator BSTR() const
            {
                return m_sStr;
            }
            BSTR* operator &()
            {
                return &m_sStr;
            }
            sal_Int32 length() const
            {
                return (m_sStr != NULL) ? ::SysStringLen(m_sStr) : 0;
            }
        };

        class OLEVariant    :   public ::tagVARIANT
        {
        public:
            OLEVariant()                    {   VariantInit(this);}
            OLEVariant(const VARIANT& varSrc)
            {
                ::VariantInit(this);
                ::VariantCopy(this, const_cast<VARIANT*>(&varSrc));
            }
            OLEVariant(const OLEVariant& varSrc)
            {
                ::VariantInit(this);
                ::VariantCopy(this, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc)));
            }

            OLEVariant(sal_Bool x)              {   VariantInit(this);  vt = VT_BOOL;   boolVal     = (x ? VARIANT_TRUE : VARIANT_FALSE);}
            OLEVariant(sal_Int8 n)              {   VariantInit(this);  vt = VT_I1;     bVal        = n;}
            OLEVariant(sal_Int16 n)             {   VariantInit(this);  vt = VT_I2;     intVal      = n;}
            OLEVariant(sal_Int32 n)             {   VariantInit(this);  vt = VT_I4;     lVal        = n;}
            OLEVariant(sal_Int64 x)             {   VariantInit(this);  vt = VT_I4;     lVal        = x;}

            OLEVariant(const rtl::OUString& us)
            {
                ::VariantInit(this);
                vt      = VT_BSTR;
                bstrVal = SysAllocString(us);
            }
            ~OLEVariant()
            {
                ::VariantClear(this);
            } // clears all the memory that was allocated before

            OLEVariant(const ::com::sun::star::util::Date& x )
            {
                VariantInit(this);
                vt      = VT_R8;
                dblVal  = ::dbtools::DBTypeConversion::toDouble(x);
            }
            OLEVariant(const ::com::sun::star::util::Time& x )
            {
                VariantInit(this);
                vt      = VT_R8;
                dblVal  = ::dbtools::DBTypeConversion::toDouble(x);
            }
            OLEVariant(const ::com::sun::star::util::DateTime& x )
            {
                VariantInit(this);
                vt      = VT_R8;
                dblVal  = ::dbtools::DBTypeConversion::toDouble(x);
            }
            OLEVariant(float x)
            {
                VariantInit(this);
                vt      = VT_R4;
                fltVal  = x;
            }
            OLEVariant(double x)
            {
                VariantInit(this);
                vt      = VT_R8;
                dblVal  = x;
            }


            OLEVariant(IDispatch* pDispInterface)
            {
                VariantInit(this);
                vt = VT_DISPATCH;
                pdispVal = pDispInterface;
            }

            OLEVariant(const ::com::sun::star::uno::Sequence< sal_Int8 >& x)
            {
                VariantInit(this);
                vt      = VT_ARRAY|VT_UI1;
                parray  = SafeArrayCreateVector(VT_UI1, 0, x.getLength());
                const sal_Int8* pBegin = x.getConstArray();
                const sal_Int8* pEnd = pBegin + x.getLength();
                for(sal_Int32 i=0;pBegin != pEnd;++i,++pBegin)
                    SafeArrayPutElement(parray,&i,&pBegin);
            }

            OLEVariant& operator=(const OLEVariant& varSrc)
            {
                VariantCopy(this, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc)));
                return *this;
            }
            // Assign a const VARIANT& (::VariantCopy handles everything)
            //
            inline OLEVariant& operator=(const tagVARIANT& varSrc)
            {
                ::VariantCopy(this, const_cast<VARIANT*>(&varSrc));

                return *this;
            }

            // Assign a const VARIANT* (::VariantCopy handles everything)
            //
            inline OLEVariant& operator=(const VARIANT* pSrc)
            {
                ::VariantCopy(this, const_cast<VARIANT*>(pSrc));

                return *this;
            }

            void setByte(sal_uInt8 n)               {   VariantClear(this); vt = VT_UI1;    bVal        = n;}
            void setInt16(sal_Int16 n)              {   VariantClear(this); vt = VT_I2;     iVal        = n;}
            void setInt32(sal_Int32 n)              {   VariantClear(this); vt = VT_I4;     lVal        = n;}
            void setFloat(float f)                  {   VariantClear(this); vt = VT_R4;     fltVal      = f;}
            void setDouble(double d)                {   VariantClear(this); vt = VT_R8;     dblVal      = d;}
            void setDate(DATE d)                    {   VariantClear(this); vt = VT_DATE;   date        = d;}
            void setChar(unsigned char a)           {   VariantClear(this); vt = VT_UI1;    bVal        = a;}
            void setCurrency(double aCur)           {   VariantClear(this); vt = VT_CY;     set(aCur*10000);}
            void setBool(sal_Bool b)                {   VariantClear(this); vt = VT_BOOL;   boolVal     = b ? VARIANT_TRUE : VARIANT_FALSE;}
            void setString(const rtl::OUString& us){    VariantClear(this); vt = VT_BSTR;   bstrVal     = ::SysAllocString(us);}
            void setNoArg()                         {   VariantClear(this); vt = VT_ERROR;  scode       = DISP_E_PARAMNOTFOUND;}

            void setIDispatch(IDispatch* pDispInterface)
                                            {   VariantClear(this); vt = VT_DISPATCH; pdispVal = pDispInterface;}

            void setNull()                  {   VariantClear(this); vt = VT_NULL;}
            void setEmpty()                 {   VariantClear(this); vt = VT_EMPTY;}

            void setUI1SAFEARRAYPtr(SAFEARRAY* pSafeAr)
                                            {   VariantClear(this); vt = VT_ARRAY|VT_UI1; parray = pSafeAr; }

            inline void setArray(SAFEARRAY* pSafeArray, VARTYPE vtType)
                                            {   VariantClear(this); vt = VT_ARRAY|vtType; parray = pSafeArray; }

            sal_Bool isNull() const  {  return (vt == VT_NULL);     }
            sal_Bool isEmpty() const {  return (vt == VT_EMPTY);    }

            VARTYPE getType() const { return vt; }
            void ChangeType(VARTYPE vartype, const OLEVariant* pSrc);


            operator ::rtl::OUString() const;

            operator sal_Bool()     const { return getBool();   }
            operator sal_Int8()     const { return getInt8();   }
            operator sal_Int16()    const { return getInt16();  }
            operator sal_Int32()    const { return getInt32();  }
            operator float()        const { return getFloat();  }
            operator double()       const { return getDouble(); }

            operator ::com::sun::star::uno::Sequence< sal_Int8 >() const;
            operator ::com::sun::star::util::Date() const
            {
                return connectivity::DateConversion::toDate(date,::com::sun::star::util::Date(30,12,1899));
            }
            operator ::com::sun::star::util::Time() const
            {
                return connectivity::DateConversion::toTime(date);
            }
            operator ::com::sun::star::util::DateTime()const
            {
                return connectivity::DateConversion::toDateTime(date,::com::sun::star::util::Date(30,12,1899));
            }

            ::rtl::OUString getString()     const;
            sal_Bool        getBool()       const;
            IUnknown*       getIUnknown()   const;
            IDispatch*      getIDispatch()  const;
            sal_uInt8       getByte()       const;
            sal_Int16       getInt16()      const;
            sal_Int8        getInt8()       const;
            sal_Int32       getInt32()      const;
            sal_uInt32      getUInt32()     const;
            float           getFloat()      const;
            double          getDouble()     const;
            double          getDate()       const;
            CY              getCurrency()   const;
            SAFEARRAY*      getUI1SAFEARRAYPtr() const;

            inline static VARIANT_BOOL VariantBool(sal_Bool bEinBoolean)
            {
                return (bEinBoolean ? VARIANT_TRUE : VARIANT_FALSE);
            }

        private:
            void CHS()
            {
                cyVal.Lo  ^= (sal_uInt32)-1;
                cyVal.Hi ^= -1;
                cyVal.Lo++;
                if( !cyVal.Lo )
                    cyVal.Hi++;
            }

            void set(double n)
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

        };
        // -----------------------------------------------------------------------------
        // inline implementaion
        // cast operator
        inline OLEVariant::operator rtl::OUString() const
        {
            if (V_VT(this) == VT_BSTR)
                return V_BSTR(this);


            OLEVariant varDest;

            varDest.ChangeType(VT_BSTR, this);

            return V_BSTR(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline OLEVariant::operator ::com::sun::star::uno::Sequence< sal_Int8 >() const
        {
            ::com::sun::star::uno::Sequence< sal_Int8 > aRet;
            if(V_VT(this) == VT_BSTR)
            {
                OLEString sStr(V_BSTR(this));
                aRet = ::com::sun::star::uno::Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>((const wchar_t*)sStr),sizeof(sal_Unicode)*sStr.length());
            }
            else
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
        inline ::rtl::OUString OLEVariant::getString() const
        {
            return (rtl::OUString)*this;
        }
        // -----------------------------------------------------------------------------
        inline sal_Bool OLEVariant::getBool() const
        {
            if (V_VT(this) == VT_BOOL)
                return V_BOOL(this) == VARIANT_TRUE ? sal_True : sal_False;

            OLEVariant varDest;

            varDest.ChangeType(VT_BOOL, this);

            return V_BOOL(&varDest) == VARIANT_TRUE ? sal_True : sal_False;
        }
        // -----------------------------------------------------------------------------
        inline IUnknown* OLEVariant::getIUnknown() const
        {
            if (V_VT(this) == VT_UNKNOWN)
            {
                V_UNKNOWN(this)->AddRef();
                return V_UNKNOWN(this);
            }

            OLEVariant varDest;

            varDest.ChangeType(VT_UNKNOWN, this);

            V_UNKNOWN(&varDest)->AddRef();
            return V_UNKNOWN(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline IDispatch* OLEVariant::getIDispatch() const
        {
            if (V_VT(this) == VT_DISPATCH)
            {
                V_DISPATCH(this)->AddRef();
                return V_DISPATCH(this);
            }

            OLEVariant varDest;

            varDest.ChangeType(VT_DISPATCH, this);

            V_DISPATCH(&varDest)->AddRef();
            return V_DISPATCH(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline sal_uInt8 OLEVariant::getByte() const
        {
            if (V_VT(this) == VT_UI1)
                return V_UI1(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_UI1, this);

            return V_UI1(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline sal_Int16 OLEVariant::getInt16() const
        {
            if (V_VT(this) == VT_I2)
                return V_I2(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_I2, this);

            return V_I2(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline sal_Int8 OLEVariant::getInt8() const
        {
            if (V_VT(this) == VT_I1)
                return V_I1(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_I1, this);

            return V_I1(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline sal_Int32 OLEVariant::getInt32() const
        {
            if (V_VT(this) == VT_I4)
                return V_I4(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_I4, this);

            return V_I4(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline sal_uInt32 OLEVariant::getUInt32() const
        {
            if (V_VT(this) == VT_UI4)
                return V_UI4(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_UI4, this);

            return V_UI4(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline float OLEVariant::getFloat() const
        {
            if (V_VT(this) == VT_R4)
                return V_R4(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_R4, this);

            return V_R4(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline double OLEVariant::getDouble() const
        {
            if (V_VT(this) == VT_R8)
                return V_R8(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_R8, this);

            return V_R8(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline double OLEVariant::getDate() const
        {
            if (V_VT(this) == VT_DATE)
                return V_DATE(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_DATE, this);

            return V_DATE(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline CY OLEVariant::getCurrency() const
        {
            if (V_VT(this) == VT_CY)
                return V_CY(this);

            OLEVariant varDest;

            varDest.ChangeType(VT_CY, this);

            return V_CY(&varDest);
        }
        // -----------------------------------------------------------------------------
        inline SAFEARRAY* OLEVariant::getUI1SAFEARRAYPtr() const
        {
            if (V_VT(this) == (VT_ARRAY|VT_UI1))
                return V_ARRAY(this);

            OLEVariant varDest;

            varDest.ChangeType((VT_ARRAY|VT_UI1), this);

            return V_ARRAY(&varDest);
        }
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
        inline void OLEVariant::ChangeType(VARTYPE vartype, const OLEVariant* pSrc)
        {
            //
            // If pDest is NULL, convert type in place
            //
            if (pSrc == NULL)
                pSrc = this;

            if ((this != pSrc) || (vartype != V_VT(this)))
            {
                if(FAILED(::VariantChangeType(static_cast<VARIANT*>(this),
                                  const_cast<VARIANT*>(static_cast<const VARIANT*>(pSrc)),
                                  0, vartype)))
                                  throw ::com::sun::star::sdbc::SQLException(::rtl::OUString::createFromAscii("Could convert type!"),NULL,::rtl::OUString(),1000,::com::sun::star::uno::Any());
            }
        }
    }
}

#endif // _CONNECTIVITY_ADO_AOLEVARIANT_HXX_

