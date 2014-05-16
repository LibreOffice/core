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
    OpenGLWindow& rWindow) :
    mxChartType(xChartType),
    mpRenderer(new opengl3D::OpenGL3DRenderer()),
    mrWindow(rWindow),
    mpCamera(NULL)
{
    mrWindow.setRenderer(this);
    mpRenderer->init();
}

GL3DBarChart::~GL3DBarChart()
{
    mrWindow.setRenderer(NULL);
}

void GL3DBarChart::create3DShapes(const boost::ptr_vector<VDataSeries>& rDataSeriesContainer,
        ExplicitCategoriesProvider& rCatProvider)
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
    float nXEnd = 0.0;
    float nYPos = 0.0;

    const Color aSeriesColor[] = {
        COL_RED, COL_GREEN, COL_YELLOW, COL_BROWN, COL_GRAY
    };

    maShapes.clear();
    maShapes.push_back(new opengl3D::Camera(mpRenderer.get()));
    mpCamera = static_cast<opengl3D::Camera*>(&maShapes.back());

    sal_Int32 nSeriesIndex = 0;
    for (boost::ptr_vector<VDataSeries>::const_iterator itr = rDataSeriesContainer.begin(),
            itrEnd = rDataSeriesContainer.end(); itr != itrEnd; ++itr)
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

        sal_Int32 nColor = aSeriesColor[nSeriesIndex % SAL_N_ELEMENTS(aSeriesColor)].GetColor();
        for(sal_Int32 nIndex = 0; nIndex < nPointCount; ++nIndex)
        {
            float nVal = rDataSeries.getYValue(nIndex);
            float nXPos = nIndex * (nBarSizeX + nBarDistanceX);


            glm::mat4 aScaleMatrix = glm::scale(nBarSizeX, nBarSizeY, nVal);
            glm::mat4 aTranslationMatrix = glm::translate(nXPos, nYPos, static_cast<float>(0));
            glm::mat4 aBarPosition = aTranslationMatrix * aScaleMatrix;

            maShapes.push_back(new opengl3D::Bar(mpRenderer.get(), aBarPosition, nColor, nId++));
        }

        float nThisXEnd = nPointCount * (nBarSizeX + nBarDistanceX);
        if (nXEnd < nThisXEnd)
            nXEnd = nThisXEnd;

        ++nSeriesIndex;
    }

    nYPos += nBarSizeY + nBarDistanceY;

    // X axis
    maShapes.push_back(new opengl3D::Line(mpRenderer.get(), nId++));
    opengl3D::Line* pAxis = static_cast<opengl3D::Line*>(&maShapes.back());
    glm::vec3 aBegin;
    aBegin.y = nYPos;
    glm::vec3 aEnd = aBegin;
    aEnd.x = nXEnd;
    pAxis->setPosition(aBegin, aEnd);
    pAxis->setLineColor(COL_BLUE);

    // Y axis
    maShapes.push_back(new opengl3D::Line(mpRenderer.get(), nId++));
    pAxis = static_cast<opengl3D::Line*>(&maShapes.back());
    aBegin.x = aBegin.y = 0;
    aEnd = aBegin;
    aEnd.y = nYPos;
    pAxis->setPosition(aBegin, aEnd);
    pAxis->setLineColor(COL_BLUE);

    // Chart background.
    maShapes.push_back(new opengl3D::Rectangle(mpRenderer.get(), nId++));
    opengl3D::Rectangle* pRect = static_cast<opengl3D::Rectangle*>(&maShapes.back());
    glm::vec3 aTopLeft;
    glm::vec3 aTopRight = aTopLeft;
    aTopRight.x = nXEnd;
    glm::vec3 aBottomRight = aTopRight;
    aBottomRight.y = nYPos;
    pRect->setPosition(aTopLeft, aTopRight, aBottomRight);
    pRect->setFillColor(COL_BLACK);
    pRect->setLineColor(COL_BLUE);

    // Create category texts along X-axis at the bottom.
    uno::Sequence<OUString> aCats = rCatProvider.getSimpleCategories();
    for (sal_Int32 i = 0; i < aCats.getLength(); ++i)
    {
        float nXPos = i * (nBarSizeX + nBarDistanceX);

        maShapes.push_back(new opengl3D::Text(mpRenderer.get(), aCats[i], nId++));
        opengl3D::Text* p = static_cast<opengl3D::Text*>(&maShapes.back());
        Size aTextSize = p->getSize();
        aTopLeft.x = nXPos;
        aTopLeft.y = nYPos;
        aTopRight = aTopLeft;
        aTopRight.x += aTextSize.getWidth();
        aBottomRight = aTopRight;
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
    mpRenderer->ProcessUnrenderedShape();
    mrWindow.getContext()->swapBuffers();
}

void GL3DBarChart::update()
{
    render();
}

namespace {

class PickingModeSetter
{
private:
    opengl3D::OpenGL3DRenderer* mpRenderer;

public:
    PickingModeSetter(opengl3D::OpenGL3DRenderer* pRenderer):
        mpRenderer(pRenderer)
    {
        mpRenderer->SetPickingMode(true);
    }

    ~PickingModeSetter()
    {
        mpRenderer->SetPickingMode(false);
    }
};

}

void GL3DBarChart::clickedAt(const Point& )
{
    {
        PickingModeSetter(mpRenderer.get());
        render();
    }
    if (mpCamera)
        mpCamera->zoom(1);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
