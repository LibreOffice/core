/*************************************************************************
 *
 *  $RCSfile: SlsFontProvider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:37:42 $
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
 *  The Initial Developer of the Original Ckode is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "view/SlsFontProvider.hxx"

#include "controller/SlideSorterController.hxx"

#include <sfx2/app.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>


using namespace ::sd::slidesorter;

namespace sd { namespace slidesorter { namespace view {

FontProvider* FontProvider::mpInstance = NULL;

FontProvider& FontProvider::Instance (void)
{
    if (mpInstance == NULL)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (mpInstance == NULL)
        {
            // Create an instance of the class and register it at the
            // SdGlobalResourceContainer so that it is eventually released.
            FontProvider* pInstance = new FontProvider();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::auto_ptr<SdGlobalResource>(pInstance));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            mpInstance = pInstance;
        }
    }
    else
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();

    // We throw an exception when for some strange reason no instance of
    // this class exists.
    if (mpInstance == NULL)
        throw ::com::sun::star::uno::RuntimeException(::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.IndexedPropertyValues")),
            NULL);

    return *mpInstance;
}




FontProvider::FontProvider (void)
    : maFont(),
      maMapMode()
{
}




FontProvider::~FontProvider (void)
{
}




void FontProvider::Invalidate (void)
{
    maFont.reset();
}




FontProvider::SharedFontPointer FontProvider::GetFont (const OutputDevice& rDevice)
{
    // Reset the font when the map mode has changed since its creation.
    if (maMapMode != rDevice.GetMapMode())
        maFont.reset();

    if (maFont.get() == NULL)
    {
        // Initialize the font from the application style settings.
        maFont.reset(new Font (Application::GetSettings().GetStyleSettings().GetAppFont()));
        maFont->SetTransparent(TRUE);
        maFont->SetWeight(WEIGHT_NORMAL);

        // Transform the point size to pixel size.
        MapMode aFontMapMode (MAP_POINT);
        Size aFontSize (rDevice.LogicToPixel(maFont->GetSize(), aFontMapMode));

        // Transform the font size to the logical coordinates of the device.
        maFont->SetSize (rDevice.PixelToLogic(aFontSize));

        // Remember the map mode of the given device to detect different
        // devices or modified zoom scales on future calls.
        maMapMode = rDevice.GetMapMode();
    }

    return maFont;
}

} } }  // end of namespace ::sd::slidesorter::view
