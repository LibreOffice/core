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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "DGColorNameLookUp.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <vos/mutex.hxx>
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
        ::vos::OGuard aGuard (::Application::GetSolarMutex());
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
                OUString::createFromAscii("com.sun.star.drawing.ColorTable")),
            uno::UNO_QUERY);

        // Get list of color names in order to iterate over the color table.
        xNA = uno::Reference<container::XNameAccess>(xColorTable, uno::UNO_QUERY);
        if (xNA.is())
        {
            // Look the solar mutex here as workarround for missing lock in
            // called function.
            ::vos::OGuard aGuard (::Application::GetSolarMutex());
            aNames = xNA->getElementNames();
        }
    }
    catch (uno::RuntimeException e)
    {
        // When an excpetion occurred then whe have an empty name sequence
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
            catch (uno::RuntimeException e)
            {
                // Ignore the exception: the color who lead to the excpetion
                // is not included into the map.
            }
        }
}




DGColorNameLookUp::~DGColorNameLookUp (void)
{
    maColorValueToNameMap.clear();
}

} // end of namespace accessibility
