/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/drawingml/ThemeFilterBase.hxx>
#include <oox/drawingml/theme.hxx>

using namespace css;

namespace oox::drawingml
{
ThemeFilterBase::ThemeFilterBase(const uno::Reference<uno::XComponentContext>& rxContext)
    : XmlFilterBase(rxContext)
{
}

ThemeFilterBase::~ThemeFilterBase() = default;

const oox::drawingml::Theme* ThemeFilterBase::getCurrentTheme() const { return mpTheme.get(); }

std::shared_ptr<oox::drawingml::Theme> ThemeFilterBase::getCurrentThemePtr() const
{
    return mpTheme;
}

void ThemeFilterBase::setCurrentTheme(const ::oox::drawingml::ThemePtr& pTheme)
{
    mpTheme = pTheme;
}

oox::vml::Drawing* ThemeFilterBase::getVmlDrawing() { return nullptr; }

oox::drawingml::table::TableStyleListPtr ThemeFilterBase::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

oox::drawingml::chart::ChartConverter* ThemeFilterBase::getChartConverter() { return nullptr; }

oox::ole::VbaProject* ThemeFilterBase::implCreateVbaProject() const { return nullptr; }

OUString ThemeFilterBase::getImplementationName() { return OUString(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
