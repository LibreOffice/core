/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _COMPHELPER_NUMBERS_HXX_
#define _COMPHELPER_NUMBERS_HXX_

#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace staruno   = ::com::sun::star::uno;
    namespace starlang  = ::com::sun::star::lang;
    namespace starutil  = ::com::sun::star::util;

    /// returns the ::com::sun::star::util::NumberFormat of the given key under the given formats
    COMPHELPER_DLLPUBLIC sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey);

    /// returns the ::com::sun::star::util::NumberFormat of the given key under the given formatter
    COMPHELPER_DLLPUBLIC sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormatter>& xFormatter, sal_Int32 nKey);

    /// returns the decimals of the given numeric number formatunder the given formats
    COMPHELPER_DLLPUBLIC staruno::Any getNumberFormatDecimals(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey);

    /** returns the standard format for the given type and the given _rLocale
    */
    sal_Int32 getStandardFormat(
            const staruno::Reference<starutil::XNumberFormatter>& xFormatter,
            sal_Int16 nType,
            const starlang::Locale& _rLocale);

    /** retrieves a the value of a given property for a given format key, relating to a given formatter
    */
    COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Any getNumberFormatProperty(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
        sal_Int32 _nKey,
        const rtl::OUString& _rPropertyName
    );

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_NUMBERS_HXX_

