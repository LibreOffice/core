/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <svx/unodraw/SvxTableShape.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

namespace chart
{
class VSeriesPlotter;

class DataTableView final
{
    css::uno::Reference<css::drawing::XShapes> m_xTarget;
    rtl::Reference<SvxTableShape> m_xTableShape;

    std::vector<OUString> m_aDataSeriesNames;
    std::vector<OUString> m_aXValues;
    std::vector<std::vector<OUString>> m_pDataSeriesValues;

public:
    DataTableView();
    void initializeShapes(const css::uno::Reference<css::drawing::XShapes>& xTarget);
    void initializeValues(std::vector<std::unique_ptr<VSeriesPlotter>>& rSeriesPlotterList);
    void createShapes(basegfx::B2DVector const& rStart, basegfx::B2DVector const& rEnd,
                      sal_Int32 nDistance);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
