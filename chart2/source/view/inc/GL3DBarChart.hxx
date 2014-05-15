/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_GL3DBARCHART_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_GL3DBARCHART_HXX

#include <GL3DPlotterBase.hxx>

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include "VDataSeries.hxx"

#include <vcl/openglwin.hxx>

namespace chart {

class ExplicitCategoriesProvider;

namespace opengl3D {

class Renderable3DObject;
class OpenGL3DRenderer;
class Camera;

}

class GL3DBarChart : public GL3DPlotterBase, public IRenderer
{
public:
    GL3DBarChart(
        const css::uno::Reference<css::chart2::XChartType>& xChartType,
        OpenGLWindow& rContext);

    virtual ~GL3DBarChart();

    virtual void create3DShapes(const boost::ptr_vector<VDataSeries>& rDataSeries,
        ExplicitCategoriesProvider& rCatProvider) SAL_OVERRIDE;

    virtual void render() SAL_OVERRIDE;

    virtual void update() SAL_OVERRIDE;

    virtual void clickedAt(const Point& rPos) SAL_OVERRIDE;

private:
    bool mbPickingMode;
    css::uno::Reference<css::chart2::XChartType> mxChartType;
    boost::ptr_vector<opengl3D::Renderable3DObject> maShapes;

    boost::scoped_ptr<opengl3D::OpenGL3DRenderer> mpRenderer;
    OpenGLWindow& mrWindow;

    opengl3D::Camera* mpCamera;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
