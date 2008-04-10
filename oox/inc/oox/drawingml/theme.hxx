/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: theme.hxx,v $
 * $Revision: 1.4 $
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

#ifndef OOX_DRAWINGML_THEME_HXX
#define OOX_DRAWINGML_THEME_HXX

#include <boost/shared_ptr.hpp>
#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include <oox/drawingml/clrscheme.hxx>
#include <map>

namespace oox { namespace drawingml {

class Shape;

typedef boost::shared_ptr< Shape > ShapePtr;

class Theme
{

public:

    Theme();
    ~Theme();

    oox::drawingml::ClrSchemePtr            getClrScheme() const { return mpClrSchemePtr; };

    rtl::OUString&                          getStyleName() { return maStyleName; };
    const rtl::OUString&                    getStyleName() const { return maStyleName; };
    std::vector< FillPropertiesPtr >&       getFillStyleList() { return maFillStyleList; };
    const std::vector< FillPropertiesPtr >& getFillStyleList() const { return maFillStyleList; };
    std::vector< LinePropertiesPtr >&       getLineStyleList() { return maLineStyleList; };
    const std::vector< LinePropertiesPtr >& getLineStyleList() const { return maLineStyleList; };
    std::vector< PropertyMap >&             getEffectStyleList() { return maEffectStyleList; };
    const std::vector< PropertyMap >&       getEffectStyleList() const { return maEffectStyleList; };
    std::vector< FillPropertiesPtr >&       getBgFillStyleList() { return maBgFillStyleList; };
    const std::vector< FillPropertiesPtr >& getBgFillStyleList() const { return maBgFillStyleList; };

    ShapePtr                                getspDef() const { return mpspDefPtr; };
    ShapePtr                                getlnDef() const { return mplnDefPtr; };
    ShapePtr                                gettxDef() const { return mptxDefPtr; };

private:

    rtl::OUString                           maStyleName;
    std::vector< FillPropertiesPtr >        maFillStyleList;
    std::vector< LinePropertiesPtr >        maLineStyleList;
    std::vector< PropertyMap >              maEffectStyleList;
    std::vector< FillPropertiesPtr >        maBgFillStyleList;

    ClrSchemePtr                            mpClrSchemePtr;
    ShapePtr                                mpspDefPtr;
    ShapePtr                                mplnDefPtr;
    ShapePtr                                mptxDefPtr;
};

typedef boost::shared_ptr< Theme > ThemePtr;

} }

#endif  //  OOX_DRAWINGML_THEME_HXX
