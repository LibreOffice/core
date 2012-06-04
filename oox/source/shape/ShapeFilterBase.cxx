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

#include "ShapeFilterBase.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/ole/vbaproject.hxx"
#include "oox/drawingml/theme.hxx"

namespace oox {
namespace shape {

using namespace ::com::sun::star;

ShapeFilterBase::ShapeFilterBase( const uno::Reference< uno::XComponentContext >& rxContext ) throw( uno::RuntimeException ) :
    XmlFilterBase( rxContext ),
    mxChartConv( new ::oox::drawingml::chart::ChartConverter )
{
}

ShapeFilterBase::~ShapeFilterBase()
{
}

const ::oox::drawingml::Theme* ShapeFilterBase::getCurrentTheme() const
{
    return mpTheme.get();
}

void ShapeFilterBase::setCurrentTheme(::oox::drawingml::ThemePtr pTheme)
{
    mpTheme = pTheme;
}

::oox::vml::Drawing* ShapeFilterBase::getVmlDrawing()
{
    return 0;
}

const ::oox::drawingml::table::TableStyleListPtr ShapeFilterBase::getTableStyles()
{
    return ::oox::drawingml::table::TableStyleListPtr();
}

::oox::drawingml::chart::ChartConverter* ShapeFilterBase::getChartConverter()
{
    return mxChartConv.get();
}

::oox::ole::VbaProject* ShapeFilterBase::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject( getComponentContext(), getModel(), CREATE_OUSTRING( "Writer" ) );
}

::rtl::OUString ShapeFilterBase::implGetImplementationName() const
{
    return ::rtl::OUString();
}

/// Graphic helper for shapes, that can manage color schemes.
class ShapeGraphicHelper : public GraphicHelper
{
public:
    explicit            ShapeGraphicHelper( const ShapeFilterBase& rFilter );
    virtual sal_Int32   getSchemeColor( sal_Int32 nToken ) const;
private:
    const ShapeFilterBase& mrFilter;
};

ShapeGraphicHelper::ShapeGraphicHelper( const ShapeFilterBase& rFilter ) :
    GraphicHelper( rFilter.getComponentContext(), rFilter.getTargetFrame(), rFilter.getStorage() ),
    mrFilter( rFilter )
{
}

sal_Int32 ShapeGraphicHelper::getSchemeColor( sal_Int32 nToken ) const
{
    return mrFilter.getSchemeColor( nToken );
}

GraphicHelper* ShapeFilterBase::implCreateGraphicHelper() const
{
    return new ShapeGraphicHelper( *this );
}

sal_Int32 ShapeFilterBase::getSchemeColor( sal_Int32 nToken ) const
{
    sal_Int32 nColor = 0;

    if (mpTheme.get())
        mpTheme->getClrScheme().getColor( nToken, nColor );

    return nColor;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
