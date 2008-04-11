/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: numbers.hxx,v $
 * $Revision: 1.5 $
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

