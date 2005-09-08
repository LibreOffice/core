/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numbers.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:51:22 $
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
    OSL_ENSURE(xFormatter.is(), "getNumberFormatType : the formatter isn't valid !");
    staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier( xFormatter->getNumberFormatsSupplier());
    OSL_ENSURE(xSupplier.is(), "getNumberFormatType : the formatter doesn't implement a supplier !");
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
    OSL_ENSURE(xTypes.is(), "getStandardFormat : no format types !");

    return xTypes.is() ? xTypes->getStandardFormat(nType, _rLocale) : 0;
}

//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;

//------------------------------------------------------------------------------
Any getNumberFormatProperty( const Reference< XNumberFormatter >& _rxFormatter, sal_Int32 _nKey, const rtl::OUString& _rPropertyName )
{
    Any aReturn;

    OSL_ENSURE( _rxFormatter.is() && _rPropertyName.getLength(), "getNumberFormatProperty: invalid arguments!" );
    try
    {
        Reference< XNumberFormatsSupplier > xSupplier;
        Reference< XNumberFormats > xFormats;
        Reference< XPropertySet > xFormatProperties;

        if ( _rxFormatter.is() )
            xSupplier = _rxFormatter->getNumberFormatsSupplier();
        if ( xSupplier.is() )
            xFormats = xSupplier->getNumberFormats();
        if ( xFormats.is() )
            xFormatProperties = xFormats->getByKey( _nKey );

        if ( xFormatProperties.is() )
            aReturn = xFormatProperties->getPropertyValue( _rPropertyName );
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "::getNumberFormatProperty: caught an exception (did you create the key with another formatter?)!" );
    }

    return aReturn;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

