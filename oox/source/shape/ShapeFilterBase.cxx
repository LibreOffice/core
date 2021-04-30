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

#include <config_wasm_strip.h>

#include <oox/shape/ShapeFilterBase.hxx>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/drawingml/theme.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>

namespace oox::shape {

using namespace ::com::sun::star;

ShapeFilterBase::ShapeFilterBase( const uno::Reference< uno::XComponentContext >& rxContext ) :
    XmlFilterBase( rxContext ),
#ifdef ENABLE_WASM_STRIP_CHART
    // WASM_CHART change
    mxChartConv( )
#else
    mxChartConv( std::make_shared<::oox::drawingml::chart::ChartConverter>() )
#endif
{
}

ShapeFilterBase::~ShapeFilterBase()
{
}

const ::oox::drawingml::Theme* ShapeFilterBase::getCurrentTheme() const
{
    return mpTheme.get();
}

void ShapeFilterBase::setCurrentTheme(const ::oox::drawingml::ThemePtr& pTheme)
{
    mpTheme = pTheme;
}

::oox::vml::Drawing* ShapeFilterBase::getVmlDrawing()
{
    return nullptr;
}

::oox::drawingml::table::TableStyleListPtr ShapeFilterBase::getTableStyles()
{
    return ::oox::drawingml::table::TableStyleListPtr();
}

::oox::drawingml::chart::ChartConverter* ShapeFilterBase::getChartConverter()
{
    return mxChartConv.get();
}

::oox::ole::VbaProject* ShapeFilterBase::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject( getComponentContext(), getModel(), u"Writer" );
}

OUString ShapeFilterBase::getImplementationName()
{
    return OUString();
}

namespace {

/// Graphic helper for shapes, that can manage color schemes.
class ShapeGraphicHelper : public GraphicHelper
{
public:
    explicit            ShapeGraphicHelper( const ShapeFilterBase& rFilter );
    virtual ::Color     getSchemeColor( sal_Int32 nToken ) const override;
private:
    const ShapeFilterBase& mrFilter;
};

}

ShapeGraphicHelper::ShapeGraphicHelper( const ShapeFilterBase& rFilter ) :
    GraphicHelper( rFilter.getComponentContext(), rFilter.getTargetFrame(), rFilter.getStorage() ),
    mrFilter( rFilter )
{
}

::Color ShapeGraphicHelper::getSchemeColor( sal_Int32 nToken ) const
{
    return mrFilter.getSchemeColor( nToken );
}

GraphicHelper* ShapeFilterBase::implCreateGraphicHelper() const
{
    GraphicHelper* pGraphicHelper = new ShapeGraphicHelper(*this);
    if (mxGraphicMapper.is())
        pGraphicHelper->setGraphicMapper(mxGraphicMapper);
    return pGraphicHelper;
}

::Color ShapeFilterBase::getSchemeColor( sal_Int32 nToken ) const
{
    ::Color nColor;

    if (mpTheme)
        mpTheme->getClrScheme().getColor( nToken, nColor );

    return nColor;
}

void ShapeFilterBase::importTheme()
{
    drawingml::ThemePtr pTheme = std::make_shared<drawingml::Theme>();
    uno::Reference<beans::XPropertySet> xPropSet(getModel(), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aGrabBag;
    xPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;

    for (const auto& rProp : std::as_const(aGrabBag))
    {
        if (rProp.Name == "OOXTheme")
        {
            uno::Reference<xml::sax::XFastSAXSerializable> xDoc;
            if (rProp.Value >>= xDoc)
            {
                rtl::Reference<core::FragmentHandler> xFragmentHandler(
                    new drawingml::ThemeFragmentHandler(*this, OUString(), *pTheme));
                importFragment(xFragmentHandler, xDoc);
                setCurrentTheme(pTheme);
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
