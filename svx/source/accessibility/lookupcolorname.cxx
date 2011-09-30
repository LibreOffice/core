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

#include "precompiled_svx.hxx"
#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "boost/unordered_map.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "comphelper/processfactory.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "vcl/svapp.hxx"

namespace {

namespace css = com::sun::star;

class ColorNameMap: private boost::noncopyable {
public:
    ColorNameMap();

    rtl::OUString lookUp(long color) const;

private:
    typedef boost::unordered_map< long, rtl::OUString > Map;

    Map map_;
};

ColorNameMap::ColorNameMap() {
    css::uno::Sequence< rtl::OUString > aNames;
    css::uno::Reference< css::container::XNameAccess > xNA;

    try
    {
        // Create color table in which to look up the given color.
        css::uno::Reference< css::container::XNameContainer > xColorTable (
            comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ColorTable")) ),
            css::uno::UNO_QUERY);

        // Get list of color names in order to iterate over the color table.
        xNA = css::uno::Reference< css::container::XNameAccess >(xColorTable, css::uno::UNO_QUERY);
        if (xNA.is())
        {
            // Look the solar mutex here as workarround for missing lock in
            // called function.
            SolarMutexGuard aGuard;
            aNames = xNA->getElementNames();
        }
    }
    catch (css::uno::RuntimeException const&)
    {
        // When an exception occurred then whe have an empty name sequence
        // and the loop below is not entered.
    }

    // Fill the map to convert from numerical color values to names.
    if (xNA.is())
        for (long int i=0; i<aNames.getLength(); i++)
        {
            // Get the numerical value for the i-th color name.
            try
            {
                css::uno::Any aColor (xNA->getByName (aNames[i]));
                long nColor = 0;
                aColor >>= nColor;
                map_[nColor] = aNames[i];
            }
            catch (css::uno::RuntimeException const&)
            {
                // Ignore the exception: the color who lead to the exception
                // is not included into the map.
            }
        }
}

rtl::OUString ColorNameMap::lookUp(long color) const {
    Map::const_iterator i(map_.find(color));
    if (i != map_.end()) {
        return i->second;
    }
    // Did not find the given color; return its RGB tuple representation:
    rtl::OUStringBuffer buf;
    buf.append(sal_Unicode('#'));
    buf.append(color, 16);
    return buf.makeStringAndClear();
}

struct theColorNameMap: public rtl::Static< ColorNameMap, theColorNameMap > {};

}

namespace accessibility {

rtl::OUString lookUpColorName(long color) {
    return theColorNameMap::get().lookUp(color);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
