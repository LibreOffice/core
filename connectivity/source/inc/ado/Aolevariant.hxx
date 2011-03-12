/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _CONNECTIVITY_ADO_AOLEVARIANT_HXX_
#define _CONNECTIVITY_ADO_AOLEVARIANT_HXX_

#include <sal/types.h>
#ifdef __MINGW32__
#include <windows.h>
#endif
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "ado_pre_sys_include.h"
#include <oaidl.h>
#include "ado_post_sys_include.h"

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace util
            {
                struct Date;
                struct Time;
                struct DateTime;
            }
        }
    }
}
namespace connectivity
{
    namespace ado
    {
        class OLEString
        {
            BSTR m_sStr;
        public:
            OLEString();
            OLEString(const BSTR& _sBStr);
            OLEString(const ::rtl::OUString& _sBStr);
            OLEString(const OLEString& _rRh)
            {
                OLEString::operator=(_rRh);
            }
            ~OLEString();
            OLEString& operator=(const ::rtl::OUString& _rSrc);
            OLEString& operator=(const BSTR& _rSrc);
            OLEString& operator=(const OLEString& _rSrc);
            operator ::rtl::OUString() const;
            operator BSTR() const;
            BSTR* operator &();
            sal_Int32 length() const;
        };

        class OLEVariant    :   public ::tagVARIANT
        {
        public:
            OLEVariant();
            OLEVariant(const VARIANT& varSrc);
            OLEVariant(const OLEVariant& varSrc)    ;
            OLEVariant(sal_Bool x)  ;
            OLEVariant(sal_Int8 n)  ;
            OLEVariant(sal_Int16 n) ;
            OLEVariant(sal_Int32 n) ;
            OLEVariant(sal_Int64 x) ;

            OLEVariant(const rtl::OUString& us) ;
            ~OLEVariant()                   ;
            OLEVariant(const ::com::sun::star::util::Date& x );
            OLEVariant(const ::com::sun::star::util::Time& x );
            OLEVariant(const ::com::sun::star::util::DateTime& x );
            OLEVariant(const float &x);
            OLEVariant(const double &x);
            OLEVariant(IDispatch* pDispInterface);
            OLEVariant(const ::com::sun::star::uno::Sequence< sal_Int8 >& x);
            OLEVariant& operator=(const OLEVariant& varSrc);
            // Assign a const VARIANT& (::VariantCopy handles everything)
            //
            OLEVariant& operator=(const tagVARIANT& varSrc);
            // Assign a const VARIANT* (::VariantCopy handles everything)
            //
            OLEVariant& operator=(const VARIANT* pSrc);
            void setByte(sal_uInt8 n)               ;
            void setInt16(sal_Int16 n)              ;
            void setInt32(sal_Int32 n)              ;
            void setFloat(float f)                  ;
            void setDouble(double d)                ;
            void setDate(DATE d)                    ;
            void setChar(unsigned char a)           ;
            void setCurrency(double aCur)           ;
            void setBool(sal_Bool b)                ;
            void setString(const rtl::OUString& us) ;
            void setNoArg()                         ;

            void setIDispatch(IDispatch* pDispInterface);
            void setNull()   ;
            void setEmpty()  ;

            void setUI1SAFEARRAYPtr(SAFEARRAY* pSafeAr);
            void setArray(SAFEARRAY* pSafeArray, VARTYPE vtType);
            sal_Bool isNull() const  ;
            sal_Bool isEmpty() const ;

            VARTYPE getType() const ;
            void ChangeType(VARTYPE vartype, const OLEVariant* pSrc);


            operator ::rtl::OUString() const;

            operator sal_Bool()     const { return getBool();   }
            operator sal_Int8()     const { return getInt8();   }
            operator sal_Int16()    const { return getInt16();  }
            operator sal_Int32()    const { return getInt32();  }
            operator float()        const { return getFloat();  }
            operator double()       const { return getDouble(); }

            operator ::com::sun::star::uno::Sequence< sal_Int8 >() const;
            operator ::com::sun::star::util::Date() const ;
            operator ::com::sun::star::util::Time() const ;
            operator ::com::sun::star::util::DateTime()const ;
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
            ::com::sun::star::uno::Any makeAny() const;

            static VARIANT_BOOL VariantBool(sal_Bool bEinBoolean);

        private:
            void CHS();

            void set(double n);

        };
    }
}

#endif // _CONNECTIVITY_ADO_AOLEVARIANT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
