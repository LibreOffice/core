/*************************************************************************
 *
 *  $RCSfile: Aolevariant.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-21 13:45:07 $
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

namespace connectivity
{
    namespace ado
    {

        class OLEVariant    :   public ::tagVARIANT
        {
        public:
            OLEVariant()                    {   VariantInit(this);}
            OLEVariant(const VARIANT& varSrc)
            {
                ::VariantInit(this);
                ::VariantCopy(this, const_cast<VARIANT*>(&varSrc));
            }
            OLEVariant(const OLEVariant& varSrc)    {   VariantInit(this);
                    VariantCopy(this, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc)));}
            OLEVariant(sal_Int32 n)             {   VariantInit(this);  vt = VT_I4;     lVal        = n;}
            OLEVariant(const rtl::OUString& us) {   VariantInit(this);  vt = VT_BSTR;   bstrVal     = SysAllocString(us);}
            ~OLEVariant()                   {   VariantClear(this); } // clears all the memory that was allocated before

            OLEVariant(const ::com::sun::star::util::Date& x )
            {
                VariantInit(this);  vt = VT_R8;
                dblVal      = ::dbtools::DBTypeConversion::toDouble(x);
            }
            OLEVariant(const ::com::sun::star::util::Time& x )
            {
                VariantInit(this);  vt = VT_R8;
                dblVal      = ::dbtools::DBTypeConversion::toDouble(x);
            }
            OLEVariant(const ::com::sun::star::util::DateTime& x )
            {
                VariantInit(this);  vt = VT_R8;
                dblVal      = ::dbtools::DBTypeConversion::toDouble(x);
            }
            OLEVariant(IDispatch* pDispInterface)
            {   VariantInit(this); vt = VT_DISPATCH; pdispVal = pDispInterface;}

                        OLEVariant(const ::com::sun::star::uno::Sequence< sal_Int8 >& x)
            {
                VariantInit(this);  vt = VT_ARRAY|VT_UI1;
                parray = SafeArrayCreateVector(VT_UI1, 0, x.getLength());
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
            void setBool(sal_Bool b)                {   VariantClear(this); vt = VT_BOOL;   boolVal     = b ? -1:0;}
            void setString(const rtl::OUString& us){    VariantClear(this); vt = VT_BSTR;   bstrVal     = SysAllocString(us);}
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

            operator rtl::OUString()
            {
                if(vt == VT_NULL)
                    return rtl::OUString();
                if (vt != VT_BSTR) VariantChangeType(this, this, NULL, VT_BSTR);
                return rtl::OUString((sal_Unicode*)bstrVal);
            }

            rtl::OUString getString()
            {
                return (rtl::OUString)*this;
            }
            operator sal_Bool()     {   return getBool();   }
            operator sal_Int32()    {   return getInt32();  }
            operator sal_Int16()    {   return getInt16();  }
            operator sal_Int8() {   return getInt8();   }
            operator float()    {   return getFloat();  }
            operator double()   {   return getDouble(); }
            operator ::com::sun::star::util::Date()
            {
                return connectivity::DateConversion::toDate(date,::com::sun::star::util::Date(30,12,1899));
            }
            operator ::com::sun::star::util::Time()
            {
                return connectivity::DateConversion::toTime(date);
            }
            operator ::com::sun::star::util::DateTime()
            {
                return connectivity::DateConversion::toDateTime(date,::com::sun::star::util::Date(30,12,1899));
            }

            sal_Bool getBool()
            {
                if (vt != VT_BOOL) VariantChangeType(this, this, NULL, VT_BOOL);
                return boolVal ? sal_True : sal_False;
            };

            IUnknown* getIUnknown()
            {
                if (vt != VT_UNKNOWN) VariantChangeType(this, this, NULL, VT_UNKNOWN);
                return (IUnknown*) punkVal;

            }

            IDispatch* getIDispatch()
            {
                if (vt != VT_DISPATCH) VariantChangeType(this,this, NULL, VT_DISPATCH);
                return (IDispatch*) pdispVal;

            }


            sal_uInt8 getByte()
            {
                if (vt != VT_UI1) VariantChangeType(this, this, NULL, VT_UI1);
                return (sal_uInt8) bVal;
            }

            sal_Int16 getInt16()
            {
                if (vt != VT_I2) VariantChangeType(this, this, NULL, VT_I2);
                return (sal_Int16) iVal;
            }

            sal_Int8 getInt8()
            {
                if (vt != VT_UI1) VariantChangeType(this, this, NULL, VT_UI1);
                return (sal_Int8) iVal;
            }

            sal_Int32 getInt32()
            {
                if (vt != VT_I4) VariantChangeType(this, this, NULL, VT_I4);
                return (sal_Int32) lVal;
            }

            sal_Int32 getUInt32()
            {
                if (vt != VT_UI4) VariantChangeType(this, this, NULL, VT_UI4);
                return (sal_uInt32) lVal;
            }

            float getFloat()
            {
                if (vt != VT_R4) VariantChangeType(this, this, NULL, VT_R4);
                return (float) fltVal;
            }

            double getDouble()
            {
                if (vt != VT_R8) VariantChangeType(this, this, NULL, VT_R8);
                return (double) dblVal;
            }

            double getDate()
            {
                if (vt != VT_DATE) VariantChangeType(this, this, NULL, VT_DATE);
                return (double) date;
            }

            double getCurrency()
            {
                if (vt != VT_CY) VariantChangeType(this, this, NULL, VT_CY);
                double toRet = ((double)cyVal.Hi*(double)4294967296.0 + (double)cyVal.Lo) / 10000;
                return toRet;
            }

            SAFEARRAY* getUI1SAFEARRAYPtr()
            {
                if (vt != (VT_ARRAY|VT_UI1))
                    VariantChangeType(this,this, NULL, VT_ARRAY|VT_UI1);
                return (parray);
            }

            // static methods

            // ACHTUNG! Der zurueckgegebene UnicodeString muß vom
            // Rufer mit delete[] freigegeben werden!!!!
            inline static OLECHAR* UniCodeFromC(const char* pText)
            {

                int nLength = strlen(pText)+1;
                OLECHAR* aReturnString = new OLECHAR[nLength];

                MultiByteToWideChar(CP_ACP, //ANSI Code Page
                                MB_PRECOMPOSED,
                                pText,
                                nLength,
                                (OLECHAR*) aReturnString,
                                nLength);

                return aReturnString;

            }

            // Der zurueckgegebene BSTR muss noch mit
            // SysFreeString() wieder freigegeben werden!
        //  inline static BSTR BSTRFromC(const char* pText)
        //  {
        //      OLECHAR* pUniString = UniCodeFromC(pText);
        //      BSTR aBSTR = SysAllocString(pUniString);
        //      delete[] pUniString;
        //      return aBSTR;
        //  }

            inline static VARIANT_BOOL VariantBool(sal_Bool bEinBoolean)
            {
                return (VARIANT_BOOL) (bEinBoolean? -1:0);
            }

        //  static String StringFromBSTR(BSTR& aBSTR)
        //  {
        //      sal_uInt16 nLength = SysStringLen(aBSTR);
        //      String sRetString;
        //      char* pBuf = sRetString.AllocStrBuf(nLength);
        //
        //      WideCharToMultiByte(CP_ACP, //ANSI Code Page
        //                      WC_COMPOSITECHECK,
        //                      aBSTR,
        //                      nLength,
        //                      pBuf,
        //                      nLength,
        //                      NULL, NULL);
        //
        //      return sRetString;
        //  }
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
    }
}
#endif // _CONNECTIVITY_ADO_AOLEVARIANT_HXX_

