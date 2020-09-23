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

#ifndef INCLUDED_OOX_SHAPE_SHAPEFILTERBASE_HXX
#define INCLUDED_OOX_SHAPE_SHAPEFILTERBASE_HXX

#include <memory>
#include <oox/dllapi.h>
#include <oox/vml/vmldrawing.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <com/sun/star/graphic/XGraphicMapper.hpp>

namespace oox::drawingml::table {

class TableStyleList;
typedef std::shared_ptr< TableStyleList > TableStyleListPtr;

}

namespace oox::shape {

class OOX_DLLPUBLIC ShapeFilterBase final : public core::XmlFilterBase
{
public:
    typedef std::shared_ptr<ShapeFilterBase> Pointer_t;

    /// @throws css::uno::RuntimeException
    explicit            ShapeFilterBase(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual             ~ShapeFilterBase() override;

    /** Has to be implemented by each filter, returns the current theme. */
    virtual const ::oox::drawingml::Theme* getCurrentTheme() const override;

    void setCurrentTheme(const ::oox::drawingml::ThemePtr& pTheme);

    /** Has to be implemented by each filter to return the collection of VML shapes. */
    virtual ::oox::vml::Drawing* getVmlDrawing() override;

    /** Has to be implemented by each filter to return TableStyles. */
    virtual ::oox::drawingml::table::TableStyleListPtr getTableStyles() override;

    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() override;

    virtual bool importDocument() override { return true; }
    virtual bool exportDocument() override { return true; }

    ::Color getSchemeColor( sal_Int32 nToken ) const;

    void importTheme();

    void setGraphicMapper(css::uno::Reference<css::graphic::XGraphicMapper> const & rxGraphicMapper)
    {
        mxGraphicMapper = rxGraphicMapper;
    }

private:
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual GraphicHelper* implCreateGraphicHelper() const override;

    std::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;
    ::oox::drawingml::ThemePtr mpTheme;
    css::uno::Reference<css::graphic::XGraphicMapper> mxGraphicMapper;
};

} // namespace oox::shape

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
