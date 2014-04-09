/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL/glew.h>

#include "3DBarChart.hxx"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "3DChartObjects.hxx"

namespace chart {

Bar3DChart::Bar3DChart(const std::vector<VDataSeries*>& rDataSeries):
    maDataSeries(rDataSeries),
    mxContext(new opengl3D::temporary::TemporaryContext())
{
}

Bar3DChart::~Bar3DChart()
{
}

void Bar3DChart::create3DShapes()
{
    const float nBarSizeX = 10;
    const float nBarSizeY = 10;
    const float nBarDistanceX = nBarSizeX / 2;
    const float nBarDistanceY = nBarSizeY / 2;

    maShapes.clear();
    maShapes.push_back(new opengl3D::Camera());
    sal_Int32 nSeriesIndex = 0;
    sal_uInt32 nId = 1;
    for(std::vector<VDataSeries*>::const_iterator itr = maDataSeries.begin(),
            itrEnd = maDataSeries.end(); itr != itrEnd; ++itr)
    {
        VDataSeries* pDataSeries = *itr;
        sal_Int32 nPointCount = pDataSeries->getTotalPointCount();
        for(sal_Int32 nIndex = 0; nIndex < nPointCount; ++nIndex)
        {
            float nVal = pDataSeries->getYValue(nIndex);
            float nXPos = nIndex * (nBarSizeX + nBarDistanceX);
            float nYPos = nSeriesIndex * (nBarSizeY + nBarDistanceY);

            glm::mat4 aBarPosition;
            glm::scale(aBarPosition, nBarSizeX, nBarSizeY, nVal);
            glm::translate(aBarPosition, nXPos, nYPos, nVal/2);

            maShapes.push_back(new opengl3D::Bar(aBarPosition, nId++));
        }

        ++nSeriesIndex;
    }
}

void Bar3DChart::render()
{
    mxContext->init();
    for(boost::ptr_vector<opengl3D::Renderable3DObject>::iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        itr->render();
    }
    mxContext->render();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
