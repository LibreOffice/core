/*************************************************************************
 *
 *  $RCSfile: numbers.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace staruno   = ::com::sun::star::uno;
    namespace starlang  = ::com::sun::star::lang;
    namespace starutil  = ::com::sun::star::util;

    /// returns the ::com::sun::star::util::NumberFormat of the given key under the given formats
    sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey);

    /// returns the ::com::sun::star::util::NumberFormat of the given key under the given formatter
    sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormatter>& xFormatter, sal_Int32 nKey);

    /// returns the decimals of the given numeric number formatunder the given formats
    staruno::Any getNumberFormatDecimals(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey);

    /** returns the standard format for the given type and the given _rLocale
    */
    sal_Int32 getStandardFormat(
            const staruno::Reference<starutil::XNumberFormatter>& xFormatter,
            sal_Int16 nType,
            const starlang::Locale& _rLocale);

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_NUMBERS_HXX_

