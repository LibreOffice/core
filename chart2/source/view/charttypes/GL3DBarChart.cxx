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
    mnMaxX(0),
    mnMaxY(0),
    mnCornerId(0),
    mbBlockUserInput(false),
    mbNeedsNewRender(true),
    mbCameraInit(false)
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

const float TEXT_HEIGHT = 10.0f;
float DEFAULT_CAMERA_HEIGHT = 500.0f;
const size_t STEPS = 200;
const sal_uLong TIMEOUT = 5;
const sal_uInt32 ID_STEP = 10;

#if 0
const float BAR_SIZE_X = 15.0f;
const float BAR_SIZE_Y = 15.0f;
#else
const float BAR_SIZE_X = 30.0f;
const float BAR_SIZE_Y = 5.0f;
#endif
const float BAR_DISTANCE_X = 5.0f;
const float BAR_DISTANCE_Y = 5.0;

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
    mpRenderer->ReleaseShapes();
    // Each series of data flows from left to right, and multiple series are
    // stacked vertically along y axis.

    sal_uInt32 nId = 1;
    float nXEnd = 0.0;
    float nYPos = 0.0;

    const Color aSeriesColor[] = {
        COL_RED, COL_GREEN, COL_YELLOW, COL_BROWN, COL_BLUE
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
        nYPos = nSeriesIndex * (BAR_SIZE_Y + BAR_DISTANCE_Y) + BAR_DISTANCE_Y;

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
                        *mpTextCache, aSeriesName, nId));
            nId += ID_STEP;
            opengl3D::Text* p = static_cast<opengl3D::Text*>(&maShapes.back());
            glm::vec3 aTopLeft, aTopRight, aBottomRight;
            aTopRight.x = -BAR_DISTANCE_Y;
            aTopRight.y = nYPos + BAR_DISTANCE_Y;
            aTopLeft.x = calculateTextWidth(aSeriesName) * -1.0 - BAR_DISTANCE_Y;
            aTopLeft.y = nYPos + BAR_DISTANCE_Y;
            aBottomRight = aTopRight;
            aBottomRight.y -= TEXT_HEIGHT;
            p->setPosition(aTopLeft, aTopRight, aBottomRight);
        }

        sal_Int32 nColor = aSeriesColor[nSeriesIndex % SAL_N_ELEMENTS(aSeriesColor)].GetColor();
        for(sal_Int32 nIndex = 0; nIndex < nPointCount; ++nIndex)
        {
            if(bMappedFillProperty)
            {
                double nPropVal = rDataSeries.getValueByProperty(nIndex, "FillColor");
                if(!rtl::math::isNan(nPropVal))
                    nColor = static_cast<sal_uInt32>(nPropVal);
            }

            float nVal = rDataSeries.getYValue(nIndex);
            float nXPos = nIndex * (BAR_SIZE_X + BAR_DISTANCE_X) + BAR_DISTANCE_X;

            glm::mat4 aScaleMatrix = glm::scale(glm::vec3(BAR_SIZE_X, BAR_SIZE_Y, float(nVal/nMaxVal)));
            glm::mat4 aTranslationMatrix = glm::translate(glm::vec3(nXPos, nYPos, 0.0f));
            glm::mat4 aBarPosition = aTranslationMatrix * aScaleMatrix;

            maBarMap.insert(std::pair<sal_uInt32, BarInformation>(nId,
                        BarInformation(glm::vec3(nXPos, nYPos, float(nVal/nMaxVal)),
                            nVal, nIndex, nSeriesIndex)));

            maShapes.push_back(new opengl3D::Bar(mpRenderer.get(), aBarPosition, nColor, nId));
            nId += ID_STEP;
        }

        float nThisXEnd = nPointCount * (BAR_SIZE_X + BAR_DISTANCE_X);
        if (nXEnd < nThisXEnd)
            nXEnd = nThisXEnd;

        ++nSeriesIndex;
    }

    nYPos += BAR_SIZE_Y + BAR_DISTANCE_Y;

    // X axis
    maShapes.push_back(new opengl3D::Line(mpRenderer.get(), nId));
    nId += ID_STEP;
    opengl3D::Line* pAxis = static_cast<opengl3D::Line*>(&maShapes.back());
    glm::vec3 aBegin;
    aBegin.y = nYPos;
    glm::vec3 aEnd = aBegin;
    aEnd.x = nXEnd;
    pAxis->setPosition(aBegin, aEnd);
    pAxis->setLineColor(COL_BLUE);

    // Y axis
    maShapes.push_back(new opengl3D::Line(mpRenderer.get(), nId));
    nId += ID_STEP;
    pAxis = static_cast<opengl3D::Line*>(&maShapes.back());
    aBegin.x = aBegin.y = 0;
    aEnd = aBegin;
    aEnd.y = nYPos;
    pAxis->setPosition(aBegin, aEnd);
    pAxis->setLineColor(COL_BLUE);

    // Chart background.
    maShapes.push_back(new opengl3D::Rectangle(mpRenderer.get(), nId));
    nId += ID_STEP;
    opengl3D::Rectangle* pRect = static_cast<opengl3D::Rectangle*>(&maShapes.back());
    glm::vec3 aTopLeft;
    glm::vec3 aTopRight = aTopLeft;
    aTopRight.x = nXEnd + 2 * BAR_DISTANCE_X;
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

        float nXPos = i * (BAR_SIZE_X + BAR_DISTANCE_X);

        maShapes.push_back(new opengl3D::Text(mpRenderer.get(), *mpTextCache,
                    aCats[i], nId));
        nId += ID_STEP;
        opengl3D::Text* p = static_cast<opengl3D::Text*>(&maShapes.back());
        aTopLeft.x = nXPos + TEXT_HEIGHT + 0.5 * BAR_SIZE_X;
        aTopLeft.y = nYPos + calculateTextWidth(aCats[i]) + 0.5 * BAR_DISTANCE_Y;
        aTopRight = aTopLeft;
        aTopRight.y = nYPos + 0.5* BAR_DISTANCE_Y;
        aBottomRight.x = nXPos;
        aBottomRight.y = nYPos + 0.5 * BAR_DISTANCE_Y;
        p->setPosition(aTopLeft, aTopRight, aBottomRight);

        // create shapes on other side as well

        maShapes.push_back(new opengl3D::Text(mpRenderer.get(), *mpTextCache,
                    aCats[i], nId));
        nId += ID_STEP;
        p = static_cast<opengl3D::Text*>(&maShapes.back());
        aTopLeft.x = nXPos + TEXT_HEIGHT + 0.5 * BAR_SIZE_X;
        aTopLeft.y =  - 0.5 * BAR_DISTANCE_Y;
        aTopRight = aTopLeft;
        aTopRight.y = -calculateTextWidth(aCats[i]) - 0.5* BAR_DISTANCE_Y;
        aBottomRight.x = nXPos;
        aBottomRight.y = -calculateTextWidth(aCats[i]) - 0.5 * BAR_DISTANCE_Y;
        p->setPosition(aTopLeft, aTopRight, aBottomRight);
    }

    mnMaxX = nMaxPointCount * (BAR_SIZE_X + BAR_DISTANCE_X) + 40;
    mnMaxY = nSeriesIndex * (BAR_SIZE_Y + BAR_DISTANCE_Y) + 40;
    if (!mbCameraInit)
    {
        mnDistance = sqrt(mnMaxX * mnMaxX + mnMaxY * mnMaxY + DEFAULT_CAMERA_HEIGHT * DEFAULT_CAMERA_HEIGHT);
        maDefaultCameraDirection = glm::vec3(mnMaxX * 0.4, mnMaxY * 0.35, 0);
        maDefaultCameraPosition = glm::vec3(maDefaultCameraDirection.x, maDefaultCameraDirection.y - mnDistance, DEFAULT_CAMERA_HEIGHT * 2);
        mnCornerId = 0;
        mbCameraInit = true;
        float pi = 3.1415926f;
        float angleX = -pi / 6.5f;
        float angleZ = -pi / 8.0f;
        glm::mat4 maDefaultRotateMatrix = glm::eulerAngleYXZ(0.0f, angleX, angleZ);
        maDefaultCameraPosition = glm::vec3(maDefaultRotateMatrix * glm::vec4(maDefaultCameraPosition, 1.0f));
        maCameraPosition = maDefaultCameraPosition;
        maCameraDirection = maDefaultCameraDirection;
        mpCamera->setPosition(maCameraPosition);
        mpCamera->setDirection(maCameraDirection);
    }
    else
    {
        mpCamera->setPosition(maCameraPosition);
        mpCamera->setDirection(maCameraDirection);
    }
    mbNeedsNewRender = true;
}

void GL3DBarChart::render()
{
    if(!mbValidContext)
        return;

    mrWindow.getContext().makeCurrent();
    Size aSize = mrWindow.GetSizePixel();
    mpRenderer->SetSize(aSize);
    mrWindow.getContext().setWinSize(aSize);
    if(mbNeedsNewRender)
    {
        for(boost::ptr_vector<opengl3D::Renderable3DObject>::iterator itr = maShapes.begin(),
                itrEnd = maShapes.end(); itr != itrEnd; ++itr)
        {
            itr->render();
        }
    }
    else
    {
        mpCamera->render();
    }
    mpRenderer->ProcessUnrenderedShape(mbNeedsNewRender);
    mbNeedsNewRender = false;
    mrWindow.getContext().swapBuffers();
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

void GL3DBarChart::moveToDefault()
{
    mnStepsTotal = STEPS;
    mnStep = 0;
    mbBlockUserInput = true;
    glm::vec3 maTargetPosition = maDefaultCameraPosition;
    maStep = (maTargetPosition - maCameraPosition)/((float)mnStepsTotal);

    glm::vec3 maTargetDirection = maDefaultCameraDirection;
    maStepDirection = (maTargetDirection - maCameraDirection)/((float)mnStepsTotal);
    maTimer.SetTimeout(TIMEOUT);
    maTimer.SetTimeoutHdl(LINK(this, GL3DBarChart, MoveToBar));
    maTimer.Start();
}

void GL3DBarChart::clickedAt(const Point& rPos, sal_uInt16 nButtons)
{
    if(mbBlockUserInput)
        return;
    if (nButtons == MOUSE_RIGHT)
    {
        moveToDefault();
    }

    if(nButtons != MOUSE_LEFT)
        return;

    sal_uInt32 nId = 5;
    {
        PickingModeSetter aPickingModeSetter(mpRenderer.get());
        render();
        nId = mpRenderer->GetPixelColorFromPoint(rPos.X(), rPos.Y());
    }

    std::map<sal_uInt32, const BarInformation>::const_iterator itr =
        maBarMap.find(nId);

    if(itr == maBarMap.end())
        return;

    mbBlockUserInput = true;

    const BarInformation& rBarInfo = itr->second;
    mnStepsTotal = STEPS;
    mnStep = 0;
    render();

    glm::vec3 maTargetPosition = rBarInfo.maPos;
    maTargetPosition.z += 240;
    maTargetPosition.y += BAR_SIZE_Y / 2.0f;
    maStep = (maTargetPosition - maCameraPosition)/((float)mnStepsTotal);

    glm::vec3 maTargetDirection = rBarInfo.maPos;
    maTargetDirection.x += BAR_SIZE_X / 2.0f;
    maTargetDirection.y += BAR_SIZE_Y / 2.0f;

    maStepDirection = (maTargetDirection - maCameraDirection)/((float)mnStepsTotal);

    maTimer.SetTimeout(TIMEOUT);
    maTimer.SetTimeoutHdl(LINK(this, GL3DBarChart, MoveToBar));
    maTimer.Start();

    maShapes.push_back(new opengl3D::ScreenText(mpRenderer.get(), *mpTextCache,
                OUString("Value: ") + OUString::number(rBarInfo.mnVal), 0));
    opengl3D::ScreenText* pScreenText = static_cast<opengl3D::ScreenText*>(&maShapes.back());
    pScreenText->setPosition(glm::vec2(-0.9f, 0.9f), glm::vec2(-0.6f, 0.8f));
    pScreenText->render();
}

void GL3DBarChart::mouseDragMove(const Point& rStartPos, const Point& rEndPos, sal_uInt16 )
{
    if(mbBlockUserInput)
        return;

    mbBlockUserInput = true;
    long direction = rEndPos.X() - rStartPos.X();
    if(direction < 0)
    {
        mnCornerId = (mnCornerId + 1) % 4;
        moveToCorner();
    }
    else if(direction > 0)
    {
        mnCornerId = mnCornerId - 1;
        if(mnCornerId < 0)
            mnCornerId = 3;
        moveToCorner();
    }
}

glm::vec3 GL3DBarChart::getCornerPosition(sal_Int8 nId)
{
    float pi = 3.1415926f;
    switch(nId)
    {
        case 0:
        {
            return glm::vec3(mnMaxX / 2 - mnDistance * sin(pi / 4), mnMaxY / 2 - mnDistance * cos(pi / 4), DEFAULT_CAMERA_HEIGHT * 2);
        }
        break;
        case 1:
        {
            return glm::vec3(mnMaxX / 2 + mnDistance * sin(pi / 4), mnMaxY / 2 - mnDistance * cos(pi / 4), DEFAULT_CAMERA_HEIGHT * 2);
        }
        break;
        case 2:
        {
            return glm::vec3(mnMaxX / 2 + mnDistance * sin(pi / 4), mnMaxY / 2 + mnDistance * cos(pi / 4), DEFAULT_CAMERA_HEIGHT * 2);
        }
        break;
        case 3:
        {
            return glm::vec3(mnMaxX / 2 - mnDistance * sin(pi / 4), mnMaxY / 2 + mnDistance * cos(pi / 4), DEFAULT_CAMERA_HEIGHT * 2);
        }
        break;
        default:
            assert(false);
    }
    return maDefaultCameraPosition;
}

void GL3DBarChart::moveToCorner()
{
    mnStepsTotal = STEPS;
    maStep = (getCornerPosition(mnCornerId) - maCameraPosition) / float(mnStepsTotal);

    maStepDirection = (glm::vec3(mnMaxX/2.0f, mnMaxY/2.0f, 0) - maCameraDirection)/ float(mnStepsTotal);
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
        maCameraDirection += maStepDirection;
        mpCamera->setDirection(maCameraDirection);
        render();
        maTimer.SetTimeout(TIMEOUT);
        maTimer.Start();
    }
    else
    {
        mbBlockUserInput = false;
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
        maCameraDirection += maStepDirection;
        mpCamera->setDirection(maCameraDirection);
        render();
        maTimer.SetTimeout(TIMEOUT);
        maTimer.Start();
    }
    else
    {
        maShapes.pop_back();
        mpRenderer->ReleaseScreenTextShapes();
        mbBlockUserInput = false;
        mnStep = 0;
    }

    return 0;
}

void GL3DBarChart::scroll(long nDelta)
{
    if(mbBlockUserInput)
        return;

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
