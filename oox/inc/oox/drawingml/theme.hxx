/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: theme.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:46:37 $
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
