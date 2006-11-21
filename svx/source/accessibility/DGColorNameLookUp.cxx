/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DGColorNameLookUp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:06:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVX_ACCESSIBILITY_DG_COLOR_NAME_LOOK_UP_HXX
#include "DGColorNameLookUp.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


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
        // When an excpetion occured then whe have an empty name sequence
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
