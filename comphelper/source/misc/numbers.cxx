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

#include <comphelper/numbers.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/Locale.hpp>

//.........................................................................
namespace comphelper
{
//.........................................................................

namespace starbeans = ::com::sun::star::beans;
namespace starlang  = ::com::sun::star::lang;


sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey)
{
    sal_Int16 nReturn(starutil::NumberFormat::UNDEFINED);
    if (xFormats.is())
    {
        try
        {
            staruno::Reference<starbeans::XPropertySet> xFormat(xFormats->getByKey(nKey));
            if (xFormat.is())
                xFormat->getPropertyValue("Type") >>= nReturn;
        }
        catch(...)
        {
            OSL_TRACE("getNumberFormatType : invalid key! (maybe created with another formatter ?)");
        }
    }
    return nReturn;
}


sal_Int16 getNumberFormatType(const staruno::Reference<starutil::XNumberFormatter>& xFormatter, sal_Int32 nKey)
{
    OSL_ENSURE(xFormatter.is(), "getNumberFormatType : the formatter isn't valid !");
    staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier( xFormatter->getNumberFormatsSupplier());
    OSL_ENSURE(xSupplier.is(), "getNumberFormatType : the formatter doesn't implement a supplier !");
    staruno::Reference<starutil::XNumberFormats> xFormats( xSupplier->getNumberFormats());
    return getNumberFormatType(xFormats, nKey);
}


staruno::Any getNumberFormatDecimals(const staruno::Reference<starutil::XNumberFormats>& xFormats, sal_Int32 nKey)
{
    if (xFormats.is())
    {
        try
        {
            staruno::Reference<starbeans::XPropertySet> xFormat( xFormats->getByKey(nKey));
            if (xFormat.is())
            {
                static OUString PROPERTY_DECIMALS( "Decimals" );
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


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;


Any getNumberFormatProperty( const Reference< XNumberFormatter >& _rxFormatter, sal_Int32 _nKey, const OUString& _rPropertyName )
{
    Any aReturn;

    OSL_ENSURE( _rxFormatter.is() && !_rPropertyName.isEmpty(), "getNumberFormatProperty: invalid arguments!" );
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
        OSL_FAIL( "::getNumberFormatProperty: caught an exception (did you create the key with another formatter?)!" );
    }

    return aReturn;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
