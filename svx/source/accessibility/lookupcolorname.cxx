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

class ColorNameMap: private boost::noncopyable {
public:
    ColorNameMap();

    OUString lookUp(long color) const;

private:
    typedef boost::unordered_map< long, OUString > Map;

    Map map_;
};

ColorNameMap::ColorNameMap() {
    css::uno::Sequence< OUString > aNames;
    css::uno::Reference< css::container::XNameAccess > xNA;

    try
    {
        // Create color table in which to look up the given color.
        css::uno::Reference< css::container::XNameContainer > xColorTable (
            comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.drawing.ColorTable" ),
            css::uno::UNO_QUERY);

        // Get list of color names in order to iterate over the color table.
        xNA = css::uno::Reference< css::container::XNameAccess >(xColorTable, css::uno::UNO_QUERY);
        if (xNA.is())
        {
            // Lock the solar mutex here as workarround for missing lock in
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

OUString ColorNameMap::lookUp(long color) const {
    Map::const_iterator i(map_.find(color));
    if (i != map_.end()) {
        return i->second;
    }
    // Did not find the given color; return its RGB tuple representation:
    OUStringBuffer buf;
    buf.append(sal_Unicode('#'));
    buf.append(color, 16);
    return buf.makeStringAndClear();
}

struct theColorNameMap: public rtl::Static< ColorNameMap, theColorNameMap > {};

}

namespace accessibility {

OUString lookUpColorName(long color) {
    return theColorNameMap::get().lookUp(color);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
