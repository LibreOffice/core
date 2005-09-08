/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numbers.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:33:17 $
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

#ifndef _COMPHELPER_NUMBERS_HXX_
#define _COMPHELPER_NUMBERS_HXX_

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

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

