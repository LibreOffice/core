/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL3DBarChart.hxx>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "3DChartObjects.hxx"
#include "GL3DRenderer.hxx"
#include <ExplicitCategoriesProvider.hxx>
#include <DataSeriesHelper.hxx>

using namespace com::sun::star;

namespace chart {

GL3DBarChart::GL3DBarChart(
    const css::uno::Reference<css::chart2::XChartType>& xChartType,
    const boost::ptr_vector<VDataSeries>& rDataSeries,
    OpenGLWindow& rWindow, ExplicitCategoriesProvider& rCatProvider ) :
    mxChartType(xChartType),
    maDataSeries(rDataSeries),
    mpRenderer(new opengl3D::OpenGL3DRenderer()),
    mrWindow(rWindow),
    mrCatProvider(rCatProvider)
{
}

GL3DBarChart::~GL3DBarChart()
{
}

void GL3DBarChart::create3DShapes()
{
    // Each series of data flows from left to right, and multiple series are
    // stacked vertically along y axis.

    // NOTE: These objects are created and positioned in a totally blind
    // fashion since we don't even have a way to see them on screen.  So, no
    // guarantee they are positioned correctly.  In fact, they are guaranteed
    // to be positioned incorrectly.

    const float nBarSizeX = 10;
    const float nBarSizeY = 10;
    const float nBarDistanceX = nBarSizeX / 2;
    const float nBarDistanceY = nBarSizeY / 2;

    sal_uInt32 nId = 1;
    float nYPos = 0.0;

    maShapes.clear();
    maShapes.push_back(new opengl3D::Camera(mpRenderer.get()));
    sal_Int32 nSeriesIndex = 0;
    for (boost::ptr_vector<VDataSeries>::const_iterator itr = maDataSeries.begin(),
            itrEnd = maDataSeries.end(); itr != itrEnd; ++itr)
    {
        nYPos = nSeriesIndex * (nBarSizeY + nBarDistanceY);

        const VDataSeries& rDataSeries = *itr;
        sal_Int32 nPointCount = rDataSeries.getTotalPointCount();

        // Create series name text object.
        OUString aSeriesName =
            DataSeriesHelper::getDataSeriesLabel(
                rDataSeries.getModel(), mxChartType->getRoleOfSequenceForSeriesLabel());

        maShapes.push_back(new opengl3D::Text(mpRenderer.get(), aSeriesName, nId++));
        opengl3D::Text* p = static_cast<opengl3D::Text*>(&maShapes.back());
        Size aTextSize = p->getSize();
        glm::vec3 aTopLeft, aTopRight, aBottomRight;
        aTopLeft.x = aTextSize.getWidth() * -1.0;
        aTopLeft.y = nYPos;
        aTopRight.y = nYPos;
        aBottomRight = aTopRight;
        aBottomRight.y += aTextSize.getHeight();
        p->setPosition(aTopLeft, aTopRight, aBottomRight);

        for(sal_Int32 nIndex = 0; nIndex < nPointCount; ++nIndex)
        {
            float nVal = rDataSeries.getYValue(nIndex);
            float nXPos = nIndex * (nBarSizeX + nBarDistanceX);

            sal_Int32 nColor = COL_BLUE;

            glm::mat4 aBarPosition;
            aBarPosition = glm::scale(aBarPosition, nBarSizeX, nBarSizeY, nVal);
            aBarPosition = glm::translate(aBarPosition, nXPos, nYPos, nVal/2);

            maShapes.push_back(new opengl3D::Bar(mpRenderer.get(), aBarPosition, nColor, nId++));
        }

        ++nSeriesIndex;
    }

    nYPos += nBarSizeY + nBarDistanceY;

    // Create category texts along X-axis at the bottom.
    uno::Sequence<OUString> aCats = mrCatProvider.getSimpleCategories();
    for (sal_Int32 i = 0; i < aCats.getLength(); ++i)
    {
        float nXPos = i * (nBarSizeX + nBarDistanceX);

        maShapes.push_back(new opengl3D::Text(mpRenderer.get(), aCats[i], nId++));
        opengl3D::Text* p = static_cast<opengl3D::Text*>(&maShapes.back());
        Size aTextSize = p->getSize();
        glm::vec3 aTopLeft;
        aTopLeft.x = nXPos;
        aTopLeft.y = nYPos;
        glm::vec3 aTopRight = aTopLeft;
        aTopRight.x += aTextSize.getWidth();
        glm::vec3 aBottomRight = aTopRight;
        aBottomRight.y += aTextSize.getHeight();
        p->setPosition(aTopLeft, aTopRight, aBottomRight);
    }
}

void GL3DBarChart::render()
{
    mrWindow.getContext()->makeCurrent();
    Size aSize = mrWindow.GetSizePixel();
    mpRenderer->SetSize(aSize);
    mrWindow.getContext()->setWinSize(aSize);
    for(boost::ptr_vector<opengl3D::Renderable3DObject>::iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        itr->render();
    }
    mrWindow.getContext()->swapBuffers();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
