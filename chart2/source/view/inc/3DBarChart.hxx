/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include "VDataSeries.hxx"

#include <vcl/OpenGLContext.hxx>

namespace chart {

namespace opengl3D {

class Renderable3DObject;

namespace temporary {

class TemporaryContext;

} }

class Bar3DChart
{
public:
    Bar3DChart(const std::vector<VDataSeries*>& rDataSeries);
    ~Bar3DChart();

    void create3DShapes();

    void render();

private:

    std::vector<VDataSeries*> maDataSeries;
    boost::ptr_vector<opengl3D::Renderable3DObject> maShapes;

    boost::scoped_ptr<opengl3D::temporary::TemporaryContext> mxContext;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
