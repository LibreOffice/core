/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsViewCacheContext.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_SLIDESORTER_VIEW_THEME_HXX
#define SD_SLIDESORTER_VIEW_THEME_HXX

#include "model/SlsVisualState.hxx"

#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>
#include <vcl/gradient.hxx>
#include <tools/color.hxx>

#include <boost/shared_ptr.hpp>


namespace sd { namespace slidesorter { namespace controller {
class Properties;
} } }

namespace sd { namespace slidesorter { namespace view {


/** Collection of colors and styles that are used to paint the slide sorter
    view.
*/
class Theme
{
public:
    Theme (const ::boost::shared_ptr<controller::Properties>& rpProperties);

    /** Call this method to update some colors as response to a change of
        a system color change.
    */
    void Update (const ::boost::shared_ptr<controller::Properties>& rpProperties);

    BitmapEx GetInsertIndicatorIcon (void) const;

    enum FontType { PageNumberFont };
    ::boost::shared_ptr<Font> CreateFont (
        const FontType eType,
        OutputDevice& rDevice) const;

    ColorData GetColorForVisualState (const model::VisualState::State eState) const;

    enum ColorType {
        Background,
        ButtonBackground,
        MouseOverColor,
        PageNumberBorder,
        PageNumberColor,
        Selection
    };
    ColorData GetColor (const ColorType eType);

    enum GradientColorType {
        NormalPage,
        SelectedPage,
        MouseOverPage
    };
    enum GradientColorClass {
        Border1,
        Border2,
        Fill1,
        Fill2
    };
    ColorData GetGradientColor (
        const GradientColorType eType,
        const GradientColorClass eClass);

    enum IconType
    {
        InsertionIndicator,
        RawShadow
    };
    BitmapEx GetIcon (const IconType eType);

private:
    class GradientDescriptor
    {
    public:
        ColorData maFillColor1;
        ColorData maFillColor2;
        ColorData maBorderColor1;
        ColorData maBorderColor2;
    };
    ColorData maBackgroundColor;
    GradientDescriptor maNormalGradient;
    GradientDescriptor maSelectedGradient;
    GradientDescriptor maMouseOverGradient;
    BitmapEx maRawShadow;
    BitmapEx maInsertionIndicator;
};


} } } // end of namespace ::sd::slidesorter::view

#endif
