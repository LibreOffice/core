/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <oox/dllapi.h>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <rtl/ref.hxx>

namespace oox::drawingml
{
class OOX_DLLPUBLIC ThemeFilterBase final : public core::XmlFilterBase
{
public:
    typedef rtl::Reference<ThemeFilterBase> Pointer_t;

    explicit ThemeFilterBase(css::uno::Reference<css::uno::XComponentContext> const& rxContext);

    virtual ~ThemeFilterBase() override;

    /** Has to be implemented by each filter, returns the current theme. */
    virtual const oox::drawingml::Theme* getCurrentTheme() const override;

    /** May be implemented by filters which handle Diagrams, default returns empty ptr */
    virtual std::shared_ptr<oox::drawingml::Theme> getCurrentThemePtr() const override;

    void setCurrentTheme(const oox::drawingml::ThemePtr& pTheme);

    /** Has to be implemented by each filter to return the collection of VML shapes. */
    virtual oox::vml::Drawing* getVmlDrawing() override;

    /** Has to be implemented by each filter to return TableStyles. */
    virtual oox::drawingml::table::TableStyleListPtr getTableStyles() override;

    virtual oox::drawingml::chart::ChartConverter* getChartConverter() override;

    virtual oox::ole::VbaProject* implCreateVbaProject() const override;

    virtual bool importDocument() override { return true; }
    virtual bool exportDocument() override { return false; }

private:
    virtual OUString SAL_CALL getImplementationName() override;

    oox::drawingml::ThemePtr mpTheme;
};

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
