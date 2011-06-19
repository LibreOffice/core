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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "DGColorNameLookUp.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>


using ::rtl::OUString;
using namespace ::com::sun::star;

namespace accessibility {

// Initialize the class instance with NULL.  A true instance is created only
// when the static <member>Instance</member> is called for the first time.
DGColorNameLookUp* DGColorNameLookUp::mpInstance = NULL;

DGColorNameLookUp& DGColorNameLookUp::Instance (void)
{
    // Using double check pattern to make sure that exactly one instance of
    // the shape type handler is instantiated.
    if (mpInstance == NULL)
    {
        SolarMutexGuard aGuard;
        if (mpInstance == NULL)
        {
            // Create the single instance of the color name look up.
            mpInstance = new DGColorNameLookUp();
        }
    }

    return *mpInstance;
}




OUString DGColorNameLookUp::LookUpColor (long int nColor) const
{
    OUString sColorName;
    tColorValueToNameMap::const_iterator I;
    I = maColorValueToNameMap.find (nColor);
    if (I != maColorValueToNameMap.end())
        // Found the color value.  Return the associated name.
        sColorName = I->second;
    else
    {
        // Did not find the given color.  Append its rgb tuple to the
        // description.
        ::rtl::OUStringBuffer sNameBuffer;
        sNameBuffer.append (sal_Unicode('#'));
        sNameBuffer.append (nColor, 16);
        sColorName = sNameBuffer.makeStringAndClear();
    }
    return sColorName;
}




DGColorNameLookUp::DGColorNameLookUp (void)
{
    uno::Sequence<OUString> aNames;
    uno::Reference<container::XNameAccess> xNA;

    try
    {
        // Create color table in which to look up the given color.
        uno::Reference<container::XNameContainer> xColorTable (
            ::comphelper::getProcessServiceFactory()->createInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ColorTable")) ),
            uno::UNO_QUERY);

        // Get list of color names in order to iterate over the color table.
        xNA = uno::Reference<container::XNameAccess>(xColorTable, uno::UNO_QUERY);
        if (xNA.is())
        {
            // Look the solar mutex here as workarround for missing lock in
            // called function.
            SolarMutexGuard aGuard;
            aNames = xNA->getElementNames();
        }
    }
    catch (uno::RuntimeException const&)
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
                uno::Any aColor (xNA->getByName (aNames[i]));
                long nColor = 0;
                aColor >>= nColor;
                maColorValueToNameMap[nColor] = aNames[i];
            }
            catch (uno::RuntimeException const&)
            {
                // Ignore the exception: the color who lead to the exception
                // is not included into the map.
            }
        }
}




DGColorNameLookUp::~DGColorNameLookUp (void)
{
    maColorValueToNameMap.clear();
}

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
