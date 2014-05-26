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
    mpCamera(NULL),
    mbValidContext(true),
    mpTextCache(new opengl3D::TextCache()),
    mnStep(0),
    mnStepsTotal(0),
    mnCornerId(0)
{
    Size aSize = mrWindow.GetSizePixel();
    mpRenderer->SetSize(aSize);
    mrWindow.setRenderer(this);
    mpRenderer->init();
}

GL3DBarChart::BarInformation::BarInformation(const glm::vec3& rPos, float nVal,
        sal_Int32 nIndex, sal_Int32 nSeriesIndex):
    maPos(rPos),
    mnVal(nVal),
    mnIndex(nIndex),
    mnSeriesIndex(nSeriesIndex)
{
}

GL3DBarChart::~GL3DBarChart()
{
    if(mbValidContext)
        mrWindow.setRenderer(NULL);
}

namespace {

const float TEXT_HEIGHT = 15.0f;
const sal_uLong TIMEOUT = 5;

float calculateTextWidth(const OUString& rText)
{
    return rText.getLength() * 10;
}

double findMaxValue(const boost::ptr_vector<VDataSeries>& rDataSeriesContainer)
{
    double nMax = 0.0;
    for (boost::ptr_vector<VDataSeries>::const_iterator itr = rDataSeriesContainer.begin(),
            itrEnd = rDataSeriesContainer.end(); itr != itrEnd; ++itr)
    {
        const VDataSeries& rDataSeries = *itr;
        sal_Int32 nPointCount = rDataSeries.getTotalPointCount();
        for(sal_Int32 nIndex = 0; nIndex < nPointCount; ++nIndex)
        {
            double nVal = rDataSeries.getYValue(nIndex);
            nMax = std::max(nMax, nVal);
        }
    }
    return nMax;
}

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

    const float nBarSizeX = 5.0f;
    const float nBarSizeY = 30.0f;
    const float nBarDistanceX = 5.0f;
    const float nBarDistanceY = 5.0;

    sal_uInt32 nId = 1;
    float nXEnd = 0.0;
    float nYPos = 0.0;

    const Color aSeriesColor[] = {
        COL_RED, COL_GREEN, COL_YELLOW, COL_BROWN, COL_GRAY
    };

    maCategories.clear();
    maSeriesNames.clear();
    maSeriesNames.reserve(rDataSeriesContainer.size());
    maBarMap.clear();
    maShapes.clear();
    maShapes.push_back(new opengl3D::Camera(mpRenderer.get()));
    mpCamera = static_cast<opengl3D::Camera*>(&maShapes.back());

    sal_Int32 nSeriesIndex = 0;
    sal_Int32 nMaxPointCount = 0;
    double nMaxVal = findMaxValue(rDataSeriesContainer)/100;
    for (boost::ptr_vector<VDataSeries>::const_iterator itr = rDataSeriesContainer.begin(),
            itrEnd = rDataSeriesContainer.end(); itr != itrEnd; ++itr)
    {
        nYPos = nSeriesIndex * (nBarSizeY + nBarDistanceY) + nBarDistanceY;

        const VDataSeries& rDataSeries = *itr;
        sal_Int32 nPointCount = rDataSeries.getTotalPointCount();
        nMaxPointCount = std::max(nMaxPointCount, nPointCount);

        bool bMappedFillProperty = rDataSeries.hasPropertyMapping("FillColor");

        // Create series name text object.
        OUString aSeriesName =
            DataSeriesHelper::getDataSeriesLabel(
                rDataSeries.getModel(), mxChartType->getRoleOfSequenceForSeriesLabel());

        maSeriesNames.push_back(aSeriesName);

        if(!aSeriesName.isEmpty())
        {
            maShapes.push_back(new opengl3D::Text(mpRenderer.get(),
                        *mpTextCache, aSeriesName, nId++));
            opengl3D::Text* p = static_cast<opengl3D::Text*>(&maShapes.back());
            glm::vec3 aTopLeft, aTopRight, aBottomRight;
            aTopLeft.x = -nBarDistanceY;
            aTopLeft.y = nYPos + 0.25 * nBarSizeY;
            aTopRight.x = calculateTextWidth(aSeriesName) * -1.0 - nBarDistanceY;
            aTopRight.y = nYPos + 0.25 * nBarSizeY;
            aBottomRight = aTopRight;
            aBottomRight.y += TEXT_HEIGHT;
            p->setPosition(aTopLeft, aTopRight, aBottomRight);
        }

        sal_Int32 nColor = aSeriesColor[nSeriesIndex % SAL_N_ELEMENTS(aSeriesColor)].GetColor();
        for(sal_Int32 nIndex = 0; nIndex < nPointCount; ++nIndex)
        {
            if(bMappedFillProperty)
            {
                nColor = static_cast<sal_uInt32>(rDataSeries.getValueByProperty(nIndex, "FillColor"));
            }

            float nVal = rDataSeries.getYValue(nIndex);
            float nXPos = nIndex * (nBarSizeX + nBarDistanceX) + nBarDistanceX;

            glm::mat4 aScaleMatrix = glm::scale(nBarSizeX, nBarSizeY, float(nVal/nMaxVal));
            glm::mat4 aTranslationMatrix = glm::translate(nXPos, nYPos, 0.0f);
            glm::mat4 aBarPosition = aTranslationMatrix * aScaleMatrix;

            maBarMap.insert(std::pair<sal_uInt32, BarInformation>(nId,
                        BarInformation(glm::vec3(nXPos, nYPos, float(nVal/nMaxVal)),
                            nVal, nIndex, nSeriesIndex)));

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
    aTopRight.x = nXEnd + 2 * nBarDistanceX;
    glm::vec3 aBottomRight = aTopRight;
    aBottomRight.y = nYPos;
    pRect->setPosition(aTopLeft, aTopRight, aBottomRight);
    pRect->setFillColor(COL_BLACK);
    pRect->setLineColor(COL_BLUE);

    // Create category texts along X-axis at the bottom.
    uno::Sequence<OUString> aCats = rCatProvider.getSimpleCategories();
    for (sal_Int32 i = 0; i < aCats.getLength(); ++i)
    {
        maCategories.push_back(aCats[i]);
        if(aCats[i].isEmpty())
            continue;

        float nXPos = i * (nBarSizeX + nBarDistanceX);

        maShapes.push_back(new opengl3D::Text(mpRenderer.get(), *mpTextCache,
                    aCats[i], nId++));
        opengl3D::Text* p = static_cast<opengl3D::Text*>(&maShapes.back());
        aTopLeft.x = nXPos + TEXT_HEIGHT;
        aTopLeft.y = nYPos + calculateTextWidth(aCats[i]) + 0.5 * nBarDistanceY;
        aTopRight = aTopLeft;
        aTopRight.y = nYPos + 0.5* nBarDistanceY;
        aBottomRight.x = nXPos;
        aBottomRight.y = nYPos + 0.5 * nBarDistanceY;
        p->setPosition(aTopLeft, aTopRight, aBottomRight);

        // create shapes on other side as well

        maShapes.push_back(new opengl3D::Text(mpRenderer.get(), *mpTextCache,
                    aCats[i], nId++));
        p = static_cast<opengl3D::Text*>(&maShapes.back());
        aTopLeft.x = nXPos + TEXT_HEIGHT;
        aTopLeft.y =  - 0.5 * nBarDistanceY;
        aTopRight = aTopLeft;
        aTopRight.y = -calculateTextWidth(aCats[i]) - 0.5* nBarDistanceY;
        aBottomRight.x = nXPos;
        aBottomRight.y = -calculateTextWidth(aCats[i]) - 0.5 * nBarDistanceY;
        p->setPosition(aTopLeft, aTopRight, aBottomRight);
    }

    mnMaxX = nMaxPointCount * (nBarSizeX + nBarDistanceX) + 40;
    mnMaxY = nSeriesIndex * (nBarSizeY + nBarDistanceY) + 40;

    maCameraPosition = glm::vec3(-30, -30, 200);
    mpCamera->setPosition(maCameraPosition);
    maCameraDirection = glm::vec3(0, 0, 0);
    mpCamera->setDirection(maCameraDirection);
}

void GL3DBarChart::render()
{
    if(!mbValidContext)
        return;

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

void GL3DBarChart::clickedAt(const Point& /*rPos*/)
{
    sal_uInt32 nId = 5;
    /*
    {
        PickingModeSetter aPickingModeSetter(mpRenderer.get());
        render();
        nId = mpRenderer->GetPixelColorFromPoint(rPos.X(), rPos.Y());
    }
    */

    std::map<sal_uInt32, const BarInformation>::const_iterator itr =
        maBarMap.find(nId);

    if(itr == maBarMap.end())
        return;

    const BarInformation& rBarInfo = itr->second;
    mnStepsTotal = 100;
    mnStep = 0;
    maOldCameraDirection = maCameraDirection;
    maCameraDirection = rBarInfo.maPos;
    render();

    maStep = (rBarInfo.maPos - maCameraPosition)/102.0f;

    maTimer.SetTimeout(TIMEOUT);
    maTimer.SetTimeoutHdl(LINK(this, GL3DBarChart, MoveToBar));
    maTimer.Start();

    maShapes.push_back(new opengl3D::ScreenText(mpRenderer.get(), *mpTextCache,
                OUString("Value: ") + OUString::number(rBarInfo.mnVal), 0));
    opengl3D::ScreenText* pScreenText = static_cast<opengl3D::ScreenText*>(&maShapes.back());
    pScreenText->setPosition(glm::vec2(-1.0f, 0.9f), glm::vec2(-0.6f, 0.75f));

}

void GL3DBarChart::mouseDragMove(const Point& rStartPos, const Point& rEndPos, sal_uInt16 nButtons)
{
    SAL_WARN("chart2.opengl", "Dragging: " << rStartPos << " to : " << rEndPos << " Buttons: " << nButtons);
    if(nButtons == MOUSE_RIGHT)
    {
        mnCornerId = (mnCornerId + 1) % 4;
        moveToCorner();
    }
    else if(nButtons == MOUSE_LEFT)
    {
        mnCornerId = mnCornerId - 1;
        if(mnCornerId < 0)
            mnCornerId = 3;
        moveToCorner();
    }
}

glm::vec3 GL3DBarChart::getCornerPosition(sal_Int8 nId)
{
    switch(nId)
    {
        case 0:
        {
            return glm::vec3(-30, -30, 200);
        }
        break;
        case 1:
        {
            return glm::vec3(mnMaxX, -30, 200);
        }
        break;
        case 2:
        {
            return glm::vec3(mnMaxX, mnMaxY, 200);
        }
        break;
        case 3:
        {
            return glm::vec3(-30, mnMaxY, 200);
        }
        break;
        default:
            assert(false);
    }

    return glm::vec3(-30, -30, 200);
}

void GL3DBarChart::moveToCorner()
{
    mnStepsTotal = 100;
    maStep = (getCornerPosition(mnCornerId) - maCameraPosition) / float(mnStepsTotal);
    maTimer.SetTimeout(TIMEOUT);
    maTimer.SetTimeoutHdl(LINK(this, GL3DBarChart, MoveCamera));
    maTimer.Start();
}

IMPL_LINK_NOARG(GL3DBarChart, MoveCamera)
{
    maTimer.Stop();
    if(mnStep < mnStepsTotal)
    {
        ++mnStep;
        maCameraPosition += maStep;
        mpCamera->setPosition(maCameraPosition);
        render();
        maTimer.SetTimeout(TIMEOUT);
        maTimer.Start();
    }
    else
    {
        mnStep = 0;
    }

    return 0;
}

IMPL_LINK_NOARG(GL3DBarChart, MoveToBar)
{
    maTimer.Stop();
    if(mnStep < mnStepsTotal)
    {
        ++mnStep;
        maCameraPosition += maStep;
        mpCamera->setPosition(maCameraPosition);
        render();
        maTimer.SetTimeout(TIMEOUT);
        maTimer.Start();
    }
    else
    {
        maShapes.pop_back();
        mnStep = 0;
    }

    return 0;
}

void GL3DBarChart::scroll(long nDelta)
{
    glm::vec3 maDir = glm::normalize(maCameraPosition - maCameraDirection);
    maCameraPosition -= (float((nDelta/10)) * maDir);
    mpCamera->setPosition(maCameraPosition);
    render();
}

void GL3DBarChart::contextDestroyed()
{
    mbValidContext = false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
