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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AOLEVARIANT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AOLEVARIANT_HXX

#include <sal/types.h>
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
            OLEString(const OUString& _sBStr);
            OLEString(const OLEString& _rRh)
            {
                OLEString::operator=(_rRh);
            }
            ~OLEString();
            OLEString& operator=(const OUString& _rSrc);
            OLEString& operator=(const BSTR& _rSrc);
            OLEString& operator=(const OLEString& _rSrc);
            OUString asOUString() const;
            BSTR asBSTR() const;
            BSTR* getAddress();
            sal_Int32 length() const;
        };

        class OLEVariant    :   public ::tagVARIANT
        {
        public:
            OLEVariant();
            OLEVariant(const VARIANT& varSrc);
            OLEVariant(const OLEVariant& varSrc)    ;
            OLEVariant(bool x)  ;
            OLEVariant(sal_Bool) = delete;
            OLEVariant(sal_Int8 n)  ;
            OLEVariant(sal_Int16 n) ;
            OLEVariant(sal_Int32 n) ;
            OLEVariant(sal_Int64 x) ;

            OLEVariant(const OUString& us) ;
            ~OLEVariant()                   ;
            OLEVariant(const css::util::Date& x );
            OLEVariant(const css::util::Time& x );
            OLEVariant(const css::util::DateTime& x );
            OLEVariant(float x);
            OLEVariant(const double &x);
            OLEVariant(IDispatch* pDispInterface);
            OLEVariant(const css::uno::Sequence< sal_Int8 >& x);
            OLEVariant& operator=(const OLEVariant& varSrc);
            // Assign a const VARIANT& (::VariantCopy handles everything)

            OLEVariant& operator=(const tagVARIANT& varSrc);
            // Assign a const VARIANT* (::VariantCopy handles everything)

            OLEVariant& operator=(const VARIANT* pSrc);
            void setByte(sal_uInt8 n)               ;
            void setInt16(sal_Int16 n)              ;
            void setInt32(sal_Int32 n)              ;
            void setFloat(float f)                  ;
            void setDouble(double d)                ;
            void setDate(DATE d)                    ;
            void setChar(unsigned char a)           ;
            void setCurrency(double aCur)           ;
            void setBool(bool b)                    ;
            void setString(const OUString& us) ;
            void setNoArg()                         ;

            void setIDispatch(IDispatch* pDispInterface);
            void setNull()   ;
            void setEmpty()  ;

            void setUI1SAFEARRAYPtr(SAFEARRAY* pSafeAr);
            void setArray(SAFEARRAY* pSafeArray, VARTYPE vtType);
            bool isNull() const  ;
            bool isEmpty() const ;

            VARTYPE getType() const ;
            void ChangeType(VARTYPE vartype, const OLEVariant* pSrc);

            OUString        getString()     const;
            bool            getBool()       const;
            IUnknown*       getIUnknown()   const;
            IDispatch*      getIDispatch()  const;
            sal_uInt8       getByte()       const;
            sal_Int16       getInt16()      const;
            sal_Int8        getInt8()       const;
            sal_Int32       getInt32()      const;
            sal_uInt32      getUInt32()     const;
            float           getFloat()      const;
            double          getDouble()     const;
            double          getDateAsDouble() const;
            CY              getCurrency()   const;
            css::util::Date getDate()       const;
            css::util::Time getTime()       const;
            css::util::DateTime getDateTime() const;
            css::uno::Sequence<sal_Int8> getByteSequence() const;
            SAFEARRAY*      getUI1SAFEARRAYPtr() const;
            css::uno::Any makeAny() const;

            static VARIANT_BOOL VariantBool(bool bEinBoolean);

        private:
            void CHS();

            void set(double n);

        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AOLEVARIANT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
