/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_GL3DBARCHART_HXX
#define CHART2_GL3DBARCHART_HXX

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include "VDataSeries.hxx"

#include <vcl/opengl/OpenGLContext.hxx>

namespace chart {

namespace opengl3D {

class Renderable3DObject;

namespace temporary {

class TemporaryContext;

} }

class GL3DBarChart
{
public:
    GL3DBarChart(const std::vector<VDataSeries*>& rDataSeries);
    ~GL3DBarChart();

    void create3DShapes();

    void render();

private:

    std::vector<VDataSeries*> maDataSeries;
    boost::ptr_vector<opengl3D::Renderable3DObject> maShapes;

    boost::scoped_ptr<opengl3D::temporary::TemporaryContext> mxContext;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
