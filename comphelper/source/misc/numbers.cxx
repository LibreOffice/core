/*************************************************************************
 *
 *  $RCSfile: numbers.cxx,v $
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
#include <comphelper/numbers.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

namespace starbeans = ::com::sun::star::beans;
namespace starlang  = ::com::sun::star::lang;

//------------------------------------------------------------------------------
sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey)
{
    sal_Int16 nReturn(starutil::NumberFormat::UNDEFINED);
    if (xFormats.is())
    {
        try
        {
            staruno::Reference<starbeans::XPropertySet> xFormat(xFormats->getByKey(nKey));
            if (xFormat.is())
                xFormat->getPropertyValue(rtl::OUString::createFromAscii("Type")) >>= nReturn;
        }
        catch(...)
        {
            OSL_TRACE("getNumberFormatType : invalid key! (maybe created with another formatter ?)");
        }
    }
    return nReturn;
}

//------------------------------------------------------------------------------
sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormatter>& xFormatter, sal_Int32 nKey)
{
    OSL_ENSHURE(xFormatter.is(), "getNumberFormatType : the formatter isn't valid !");
    staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier( xFormatter->getNumberFormatsSupplier());
    OSL_ENSHURE(xSupplier.is(), "getNumberFormatType : the formatter doesn't implement a supplier !");
    staruno::Reference<starutil::XNumberFormats> xFormats( xSupplier->getNumberFormats());
    return getNumberFormatType(xFormats, nKey);
}

//------------------------------------------------------------------------------
staruno::Any getNumberFormatDecimals(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey)
{
    if (xFormats.is())
    {
        try
        {
            staruno::Reference<starbeans::XPropertySet> xFormat( xFormats->getByKey(nKey));
            if (xFormat.is())
            {
                static ::rtl::OUString PROPERTY_DECIMALS = ::rtl::OUString::createFromAscii("Decimals");
                return xFormat->getPropertyValue(PROPERTY_DECIMALS);
            }
        }
        catch(...)
        {
            OSL_TRACE("getNumberFormatDecimals : invalid key! (may be created with another formatter ?)");
        }
    }
    return staruno::makeAny((sal_Int16)0);
}


//------------------------------------------------------------------------------
sal_Int32 getStandardFormat(
        const staruno::Reference<starutil::XNumberFormatter>& xFormatter,
        sal_Int16 nType,
        const starlang::Locale& _rLocale)
{
    staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier( xFormatter.is() ? xFormatter->getNumberFormatsSupplier() : staruno::Reference<starutil::XNumberFormatsSupplier>(NULL));
    staruno::Reference<starutil::XNumberFormats> xFormats( xSupplier.is() ? xSupplier->getNumberFormats() : staruno::Reference<starutil::XNumberFormats>(NULL));
    staruno::Reference<starutil::XNumberFormatTypes> xTypes(xFormats, staruno::UNO_QUERY);
    OSL_ENSHURE(xTypes.is(), "getStandardFormat : no format types !");

    return xTypes.is() ? xTypes->getStandardFormat(nType, _rLocale) : 0;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

