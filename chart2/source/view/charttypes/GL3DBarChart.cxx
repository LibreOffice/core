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

#include <osl/time.h>
#ifdef WNT
#include <windows.h>
#endif
#include <memory>
#include <o3tl/make_unique.hxx>

#define CALC_POS_EVENT_ID 1
#define SHAPE_START_ID 10
#define FPS_TIME 500
#define DATAUPDATE_FPS_TIME 1000
#define HISTORY_NUM 51
#define COLUMNSIZE 25
#define SHOW_VALUE_COUNT 15
#define SHOW_SCROLL_TEXT_DISTANCE 1000
#define FLY_THRESHOLD 20
#define DISPLAY_BARS_NUM 3


using namespace com::sun::star;

namespace chart {

const size_t STEPS = 200;
const size_t STEPS_UPDATE = 100;
namespace {

const float TEXT_HEIGHT = 10.0f;
float DEFAULT_CAMERA_HEIGHT = 500.0f;
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

double findMaxValue(const std::vector<std::unique_ptr<VDataSeries> >& rDataSeriesContainer)
{
    double nMax = 0.0;
    for (const std::unique_ptr<VDataSeries>& rDataSeries : rDataSeriesContainer)
    {
        sal_Int32 nPointCount = rDataSeries->getTotalPointCount();
        for(sal_Int32 nIndex = 0; nIndex < nPointCount; ++nIndex)
        {
            double nVal = rDataSeries->getYValue(nIndex);
            nMax = std::max(nMax, nVal);
        }
    }
    return nMax;
}

class SharedResourceAccess
{
private:
    osl::Condition& mrCond1;
    osl::Condition& mrCond2;

public:

    SharedResourceAccess(osl::Condition& rCond1, osl::Condition& rCond2):
        mrCond1(rCond1),
        mrCond2(rCond2)
    {
        mrCond1.set();
    }

    ~SharedResourceAccess()
    {
        mrCond2.set();
    }
};

}

class RenderThread : public salhelper::Thread
{
public:
    explicit RenderThread(GL3DBarChart* pChart);

protected:

    void renderFrame();
    GL3DBarChart* mpChart;
};

RenderThread::RenderThread(GL3DBarChart* pChart):
    salhelper::Thread("RenderThread"),
    mpChart(pChart)
{
}

void RenderThread::renderFrame()
{
    if(!mpChart->mbValidContext)
        return;

    mpChart->mpWindow->getContext().makeCurrent();
    mpChart->renderFrame();
    // FIXME: SwapBuffers can take a considerable time, it'd be
    // nice if we didn't hold the chart mutex while doing that.
    mpChart->mpWindow->getContext().swapBuffers();
    mpChart->mpWindow->getContext().resetCurrent();
}

class RenderOneFrameThread : public RenderThread
{
public:
    explicit RenderOneFrameThread(GL3DBarChart* pChart):
        RenderThread(pChart)
    {}

protected:

    virtual void execute() override;
};

void RenderOneFrameThread::execute()
{
    osl::MutexGuard aGuard(mpChart->maMutex);
    renderFrame();
}

class RenderAnimationThread : public RenderThread
{
public:
    RenderAnimationThread(GL3DBarChart* pChart, const glm::vec3& rStartPos, const glm::vec3& rEndPos,
            const sal_Int32 nSteps = STEPS):
        RenderThread(pChart),
        maStartPos(rStartPos),
        maEndPos(rEndPos),
        mnSteps(nSteps)
    {
    }

protected:

    virtual void execute() override;

private:
    glm::vec3 maStartPos;
    glm::vec3 maEndPos;
    sal_Int32 mnSteps;

};

void RenderAnimationThread::execute()
{
    osl::MutexGuard aGuard(mpChart->maMutex);
    glm::vec3 aStep = (maEndPos - maStartPos)/(float)mnSteps;
    for(sal_Int32 i = 0; i < mnSteps; ++i)
    {
        mpChart->maCameraPosition += aStep;
        mpChart->mpCamera->setPosition(mpChart->maCameraPosition);
        /*
        mpChart->maCameraDirection += mpChart->maStepDirection;
        mpChart->mpCamera->setDirection(mpChart->maCameraDirection);
        */
        renderFrame();
    }
    mpChart->mpRenderer->ReleaseScreenTextShapes();
}

class RenderBenchMarkThread : public RenderThread
{
public:
    explicit RenderBenchMarkThread(GL3DBarChart * pChart)
        : RenderThread(pChart)
        , mbAutoFlyExecuting(false)
        , mbExecuting(false)
        , mbNeedFlyBack(false)
        , mnStep(0)
        , mnStepsTotal(0)
    {
        osl_getSystemTime(&maClickFlyBackStartTime);
        osl_getSystemTime(&maClickFlyBackEndTime);
    }
protected:
    virtual void execute() override;
private:
    void ProcessMouseEvent();
    void MoveCamera();
    void MoveCameraToBar();
    void MoveToBar();
    void MoveToSelectedBar();
    void MoveToDefault();
    void MoveToCorner();
    void ProcessScroll();
    void UpdateScreenText();
    void ProcessClickFlyBack();
    void AutoMoveToBar();
private:
    glm::vec3 maStartPos;
    glm::vec3 maEndPos;
    bool mbAutoFlyExecuting;
    bool mbExecuting;
    bool mbNeedFlyBack;
    glm::vec3 maStep;
    glm::vec3 maStepDirection;
    glm::mat4 maMatrixStep;
    size_t mnStep;
    size_t mnStepsTotal;
    TimeValue maClickFlyBackStartTime;
    TimeValue maClickFlyBackEndTime;
    glm::vec3 maTargetPosition;
    glm::vec3 maTargetDirection;
};

void RenderBenchMarkThread::MoveCamera()
{
    if(mnStep < mnStepsTotal)
    {
        ++mnStep;
        mpChart->maCameraPosition += maStep;
        mpChart->mpCamera->setPosition(mpChart->maCameraPosition);
        mpChart->maCameraDirection += maStepDirection;
        mpChart->mpCamera->setDirection(mpChart->maCameraDirection);
    }
    else
    {
        mnStep = 0;
        mbExecuting = false;
        mbAutoFlyExecuting = false;
        mbNeedFlyBack = false;
        mpChart->maRenderEvent = EVENT_NONE;
    }
}

void RenderBenchMarkThread::MoveCameraToBar()
{
    if(mnStep < mnStepsTotal)
    {
        ++mnStep;
        mpChart->mpRenderer->AddMatrixDiff(maMatrixStep);
    }
    else
    {
        mpChart->maCameraPosition = maTargetPosition;
        mpChart->maCameraDirection = maTargetDirection;
        mpChart->mpCamera->setPosition(maTargetPosition);
        mpChart->mpCamera->setDirection(maTargetDirection);
        mpChart->mpRenderer->ResetMatrixDiff();
        mnStep = 0;
        mbExecuting = false;
        mbAutoFlyExecuting = false;
        mbNeedFlyBack = true;
        osl_getSystemTime(&maClickFlyBackStartTime);
        osl_getSystemTime(&maClickFlyBackEndTime);
        mpChart->maRenderEvent = EVENT_SHOW_SELECT;
    }
}


void RenderBenchMarkThread::MoveToDefault()
{
    if ((mpChart->maCameraPosition == mpChart->maDefaultCameraDirection) &&
        (mpChart->maCameraDirection == mpChart->maDefaultCameraDirection))
    {
        mnStep = 0;
        mbExecuting = false;
        mpChart->maRenderEvent = EVENT_NONE;
        return;
    }
    if (!mbExecuting)
    {
        mpChart->mpRenderer->EndClick();
        mnStep = 0;
        mnStepsTotal = STEPS;
        maStep = (mpChart->maDefaultCameraPosition - mpChart->maCameraPosition)/((float)mnStepsTotal);
        maStepDirection = (mpChart->maDefaultCameraDirection - mpChart->maCameraDirection)/((float)mnStepsTotal);
        mbExecuting = true;
    }
    MoveCamera();
}

void RenderBenchMarkThread::MoveToBar()
{
    if (!mbExecuting)
    {
        mpChart->mnSelectBarId = mpChart->barIdAtPosition(mpChart->maClickPos);

        std::map<sal_uInt32, const GL3DBarChart::BarInformation>::const_iterator itr = mpChart->maBarMap.find(mpChart->mnSelectBarId);
        if(itr == mpChart->maBarMap.end())
        {
            mpChart->mnSelectBarId = mpChart->mnPreSelectBarId;
            mpChart->maRenderEvent = mpChart->maPreRenderEvent;
            mpChart->maClickCond.set();
            return;
        }
        mpChart->mpRenderer->EndClick();
        const GL3DBarChart::BarInformation& rBarInfo = itr->second;
        mnStep = 0;
        mnStepsTotal = STEPS;
        maTargetPosition = rBarInfo.maPos;
        maTargetPosition.z += 240;
        maTargetPosition.x += BAR_SIZE_X / 2.0f;
        maTargetDirection = rBarInfo.maPos;
        maTargetDirection.x += BAR_SIZE_X / 2.0f;
        maTargetDirection.y += BAR_SIZE_Y / 2.0f;
        maTargetPosition.y = maTargetDirection.y - 240;
        maMatrixStep = mpChart->mpRenderer->GetDiffOfTwoCameras(mpChart->maCameraPosition, maTargetPosition, mpChart->maCameraDirection, maTargetDirection)/((float)mnStepsTotal);
        mpChart->maClickCond.set();
        mbExecuting = true;
        mbNeedFlyBack = false;
        mpChart->mpRenderer->StartClick(mpChart->mnSelectBarId);
    }
    MoveCameraToBar();
}

void RenderBenchMarkThread::MoveToSelectedBar()
{
    mpChart->mnSelectBarId = mpChart->mnUpdateBarId;
    std::map<sal_uInt32, const GL3DBarChart::BarInformation>::const_iterator itr = mpChart->maBarMap.find(mpChart->mnSelectBarId);
    if(itr == mpChart->maBarMap.end())
    {
        mpChart->mnSelectBarId = mpChart->mnPreSelectBarId;
        mpChart->maRenderEvent = mpChart->maPreRenderEvent;
        mpChart->maClickCond.set();
        return;
    }
    mpChart->mpRenderer->EndClick();
    const GL3DBarChart::BarInformation& rBarInfo = itr->second;
    mnStep = 0;
    mnStepsTotal = STEPS_UPDATE;
    maTargetPosition = rBarInfo.maPos;
    maTargetPosition.z += 240;
    maTargetPosition.x += BAR_SIZE_X / 2.0f;
    maTargetDirection = rBarInfo.maPos;
    maTargetDirection.x += BAR_SIZE_X / 2.0f;
    maTargetDirection.y += BAR_SIZE_Y / 2.0f;
    maTargetPosition.y = maTargetDirection.y - 240;
    maMatrixStep = mpChart->mpRenderer->GetDiffOfTwoCameras( maTargetPosition,  maTargetDirection)/((float)mnStepsTotal);
    mpChart->maClickCond.set();
    mbExecuting = true;
    mbNeedFlyBack = false;
    mpChart->mpRenderer->StartClick(mpChart->mnSelectBarId);
    mpChart->maRenderEvent = EVENT_CLICK;
}

void RenderBenchMarkThread::AutoMoveToBar()
{
    if (!mbAutoFlyExecuting)
    {
        mpChart->mpRenderer->EndClick();
        std::map<sal_uInt32, const GL3DBarChart::BarInformation>::const_iterator itr = mpChart->maBarMap.find(mpChart->mnSelectBarId);
        if(itr == mpChart->maBarMap.end())
        {
            mpChart->maRenderEvent = EVENT_NONE;
            return;
        }
        const GL3DBarChart::BarInformation& rBarInfo = itr->second;
        mnStep = 0;
        mnStepsTotal = STEPS;
         maTargetPosition = rBarInfo.maPos;
        maTargetPosition.z += 240;
        maTargetPosition.x += BAR_SIZE_X / 2.0f;
        maTargetDirection = rBarInfo.maPos;
        maTargetDirection.x += BAR_SIZE_X / 2.0f;
        maTargetDirection.y += BAR_SIZE_Y / 2.0f;
        maTargetPosition.y = maTargetDirection.y - 240;
        maMatrixStep = mpChart->mpRenderer->GetDiffOfTwoCameras(mpChart->maCameraPosition, maTargetPosition, mpChart->maCameraDirection, maTargetDirection)/((float)mnStepsTotal);
        mpChart->mpRenderer->StartClick(mpChart->mnSelectBarId);
        mbAutoFlyExecuting = true;
        mbNeedFlyBack = false;
    }
    MoveCameraToBar();
}

void RenderBenchMarkThread::MoveToCorner()
{
    if (!mbExecuting)
    {
        mpChart->mpRenderer->EndClick();
        mnStep = 0;
        mnStepsTotal = STEPS;
        maStep = (mpChart->getCornerPosition(mpChart->mnCornerId) - mpChart->maCameraPosition) / float(mnStepsTotal);
        maStepDirection = (glm::vec3(mpChart->mnMaxX/2.0f, mpChart->mnMaxY/2.0f, 0) - mpChart->maCameraDirection)/ float(mnStepsTotal);
        mbExecuting = true;
    }
    MoveCamera();
}

void RenderBenchMarkThread::ProcessScroll()
{
    //will add other process later
    mpChart->mpRenderer->EndClick();
    mnStep = 0;
    mnStepsTotal = STEPS;
    mpChart->maRenderEvent = EVENT_SHOW_SCROLL;
}

void RenderBenchMarkThread::ProcessClickFlyBack()
{
    if (!mbNeedFlyBack)
        return;
    osl_getSystemTime(&maClickFlyBackEndTime);
    int nDeltaMs = GL3DBarChart::calcTimeInterval(maClickFlyBackStartTime, maClickFlyBackEndTime);
    if(nDeltaMs >= 10000)
    {
        mpChart->maRenderEvent = EVENT_MOVE_TO_DEFAULT;
    }
}

void RenderBenchMarkThread::ProcessMouseEvent()
{
    ProcessClickFlyBack();
    if (mpChart->maRenderEvent == EVENT_SELECTBAR_UPDEDATE)
    {
        MoveToSelectedBar();
    }
    else if (mpChart->maRenderEvent == EVENT_CLICK)
    {
        MoveToBar();
    }
    else if (mpChart->maRenderEvent == EVENT_MOVE_TO_DEFAULT)
    {
        MoveToDefault();
    }
    else if ((mpChart->maRenderEvent == EVENT_DRAG_LEFT) || (mpChart->maRenderEvent == EVENT_DRAG_RIGHT))
    {
        MoveToCorner();
    }
    else if (mpChart->maRenderEvent == EVENT_SCROLL)
    {
        ProcessScroll();
    }
    else if (mpChart->maRenderEvent == EVENT_AUTO_FLY)
    {
        AutoMoveToBar();
    }

}

void RenderBenchMarkThread::UpdateScreenText()
{
    if (mpChart->mbScreenTextNewRender)
    {
        mpChart->mpWindow->getContext().makeCurrent();
        mpChart->mpRenderer->ReleaseScreenTextTexture();
        for(std::unique_ptr<opengl3D::Renderable3DObject>& aObj : mpChart->maScreenTextShapes)
        {
            aObj->render();
        }
        mpChart->mbScreenTextNewRender = false;
        mpChart->mpWindow->getContext().resetCurrent();
    }
}

void RenderBenchMarkThread::execute()
{
    while (true)
    {
        {
            osl::MutexGuard aGuard(mpChart->maMutex);
            mpChart->maCond2.reset();
            if (mpChart->mbRenderDie)
                break;
            UpdateScreenText();
            ProcessMouseEvent();
            renderFrame();
            mpChart->miFrameCount++;
        }
        if (mpChart->maCond1.check())
        {
            mpChart->maCond1.reset();
            mpChart->maCond2.wait();
        }
    }
}

GL3DBarChart::GL3DBarChart(
    const css::uno::Reference<css::chart2::XChartType>& xChartType,
    OpenGLWindow* pWindow) :
    mxChartType(xChartType),
    mpRenderer(new opengl3D::OpenGL3DRenderer()),
    mpWindow(pWindow),
    mpCamera(nullptr),
    mbValidContext(true),
    mpTextCache(new opengl3D::TextCache()),
    mnMaxX(0),
    mnMaxY(0),
    mnDistance(0.0),
    mnCornerId(0),
    mbNeedsNewRender(true),
    mbCameraInit(false),
    mbRenderDie(false),
    maRenderEvent(EVENT_NONE),
    maPreRenderEvent(EVENT_NONE),
    mnSelectBarId(0),
    mnPreSelectBarId(0),
    miScrollRate(0),
    mbScrollFlg(false),
    mbScreenTextNewRender(false),
    maFPS(OUString("Render FPS: 0")),
    maDataUpdateFPS(OUString("Data Update FPS: 0")),
    miFrameCount(0),
    miDataUpdateCounter(0),
    mnColorRate(0),
    mbBenchMarkMode(false),
    mnHistoryCounter(0),
    mnBarsInRow(0),
    mbAutoFly(false),
    mnUpdateBarId(0)
{
    maFPSRenderStartTime.Seconds = maFPSRenderStartTime.Nanosec = 0;
    maFPSRenderEndTime.Seconds = maFPSRenderEndTime.Nanosec = 0;
    maDataUpdateStartTime.Seconds = maDataUpdateStartTime.Nanosec = 0;
    maDataUpdateEndTime.Seconds = maDataUpdateEndTime.Nanosec = 0;

    static const char *aBenchMark = getenv("UNLOCK_FPS_MODE");
    if (aBenchMark)
    {
        mbBenchMarkMode = atoi(aBenchMark);
    }
    if (mbBenchMarkMode)
    {
        static const char *scrollFrame = getenv("SCROLL_RATE");
        if (scrollFrame)
        {
            miScrollRate = atoi(scrollFrame);
            if (miScrollRate > 0)
            {
                mbScrollFlg = true;
                mpRenderer->SetScroll();
            }
        }
        char *aAutoFly = getenv("AUTO_FLY");
        if (aAutoFly)
        {
            mbAutoFly = atoi(aAutoFly);
        }
        maIdle.SetPriority(SchedulerPriority::REPAINT);
        maIdle.SetIdleHdl(LINK(this, GL3DBarChart, UpdateTimerHdl));
        maIdle.Start();
        osl_getSystemTime(&maFPSRenderStartTime);
        osl_getSystemTime(&maFPSRenderEndTime);
        osl_getSystemTime(&maDataUpdateStartTime);
        osl_getSystemTime(&maDataUpdateEndTime);
    }
    Size aSize = mpWindow->GetSizePixel();
    mpRenderer->SetSize(aSize);
    mpWindow->setRenderer(this);
    mpWindow->getContext().makeCurrent();
    mpRenderer->init();
    mpWindow->getContext().resetCurrent();
}

GL3DBarChart::BarInformation::BarInformation(const glm::vec3& rPos, float nVal):
    maPos(rPos),
    mnVal(nVal)
{
}

GL3DBarChart::~GL3DBarChart()
{
    if (mbBenchMarkMode)
    {
        SharedResourceAccess aResGuard(maCond1, maCond2);
        osl::MutexGuard aGuard(maMutex);
        mbRenderDie = true;
    }

    joinRenderThread();

    if(mbValidContext)
        mpWindow->setRenderer(nullptr);
}

void GL3DBarChart::create3DShapes(const std::vector<std::unique_ptr<VDataSeries> >& rDataSeriesContainer,
        ExplicitCategoriesProvider& rCatProvider)
{
    SharedResourceAccess aResGuard(maCond1, maCond2);
    osl::MutexGuard aGuard(maMutex);
    if(mnSelectBarId)
    {
        int nSelectBarId = mnSelectBarId;
        int nPreSelectBarId = nSelectBarId;
        nSelectBarId -= 10;
        sal_uInt32 nSelectRow = (nSelectBarId - SHAPE_START_ID) / ID_STEP / (mnBarsInRow + 1);
        sal_uInt32 nPreSelectRow = (nPreSelectBarId - SHAPE_START_ID) / ID_STEP / (mnBarsInRow + 1);
        if(nSelectRow == nPreSelectRow)
        {
            std::map<sal_uInt32, const GL3DBarChart::BarInformation>::const_iterator itr = maBarMap.find(nSelectBarId);
            if((maRenderEvent == EVENT_CLICK || maRenderEvent == EVENT_SHOW_SELECT || maRenderEvent == EVENT_AUTO_FLY)&&(itr != maBarMap.end()))
            {
                mnUpdateBarId = nSelectBarId;
                maRenderEvent = EVENT_SELECTBAR_UPDEDATE;
            }
        }
    }
    mpRenderer->ReleaseShapes();
    // Each series of data flows from left to right, and multiple series are
    // stacked vertically along y axis.

    sal_uInt32 nId = SHAPE_START_ID;
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
    if (mbBenchMarkMode)
    {
        mnColorRate = 0;
    }
    maShapes.push_back(o3tl::make_unique<opengl3D::Camera>(mpRenderer.get()));
    mpCamera = static_cast<opengl3D::Camera*>(maShapes.back().get());

    sal_Int32 nSeriesIndex = 0;
    sal_Int32 nMaxPointCount = 0;
    double nMaxVal = findMaxValue(rDataSeriesContainer)/100;
    if (rDataSeriesContainer.empty())
    {
        mnBarsInRow = 0;
    }
    else
    {
        const VDataSeries& rFirstRow = *rDataSeriesContainer.begin()->get();
        mnBarsInRow = rFirstRow.getTotalPointCount();
    }
    for (const std::unique_ptr<VDataSeries>& rDataSeries : rDataSeriesContainer)
    {
        nYPos = nSeriesIndex * (BAR_SIZE_Y + BAR_DISTANCE_Y) + BAR_DISTANCE_Y;

        sal_Int32 nPointCount = rDataSeries->getTotalPointCount();
        nMaxPointCount = std::max(nMaxPointCount, nPointCount);

        bool bMappedFillProperty = rDataSeries->hasPropertyMapping("FillColor");

        // Create series name text object.
        OUString aSeriesName =
            DataSeriesHelper::getDataSeriesLabel(
                rDataSeries->getModel(), mxChartType->getRoleOfSequenceForSeriesLabel());

        maSeriesNames.push_back(aSeriesName);

        if(!aSeriesName.isEmpty())
        {
            maShapes.push_back(o3tl::make_unique<opengl3D::Text>(mpRenderer.get(),
                        *mpTextCache, aSeriesName, nId));
            nId += ID_STEP;
            opengl3D::Text* p = static_cast<opengl3D::Text*>(maShapes.back().get());
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
                double nPropVal = rDataSeries->getValueByProperty(nIndex, "FillColor");
                if(!rtl::math::isNan(nPropVal))
                    nColor = static_cast<sal_uInt32>(nPropVal);
            }

            float nVal = rDataSeries->getYValue(nIndex);
            if (rtl::math::isNan(nVal))
                continue;

            float nXPos = nIndex * (BAR_SIZE_X + BAR_DISTANCE_X) + BAR_DISTANCE_X;

            glm::mat4 aScaleMatrix = glm::scale(glm::vec3(BAR_SIZE_X, BAR_SIZE_Y, float(nVal/nMaxVal)));
            glm::mat4 aTranslationMatrix = glm::translate(glm::vec3(nXPos, nYPos, 0.0f));
            glm::mat4 aBarPosition = aTranslationMatrix * aScaleMatrix;

            maBarMap.insert(std::pair<sal_uInt32, BarInformation>(nId,
                        BarInformation(glm::vec3(nXPos, nYPos, float(nVal/nMaxVal)),
                            nVal)));
            recordBarHistory(nId, nVal);
            if (mbBenchMarkMode)
            {
                std::map<sal_uInt32, sal_uInt32>::const_iterator it = maBarColorMap.find(nId);
                if (it == maBarColorMap.end())
                {
                    maBarColorMap[nId] = nColor;
                }
                else
                {
                    if(mbAutoFly)
                        processAutoFly(nId, nColor);
                }
            }
            maShapes.push_back(o3tl::make_unique<opengl3D::Bar>(mpRenderer.get(), aBarPosition, nColor, nId));
            nId += ID_STEP;
        }

        float nThisXEnd = nPointCount * (BAR_SIZE_X + BAR_DISTANCE_X);
        if (nXEnd < nThisXEnd)
            nXEnd = nThisXEnd;

        ++nSeriesIndex;
    }

    nYPos += BAR_SIZE_Y + BAR_DISTANCE_Y;

    // X axis
    maShapes.push_back(o3tl::make_unique<opengl3D::Line>(mpRenderer.get(), nId));
    nId += ID_STEP;
    opengl3D::Line* pAxis = static_cast<opengl3D::Line*>(maShapes.back().get());
    glm::vec3 aBegin;
    aBegin.y = nYPos;
    glm::vec3 aEnd = aBegin;
    aEnd.x = mbBenchMarkMode ? (mbScrollFlg ? nXEnd - BAR_SIZE_X : nXEnd) : nXEnd;
    pAxis->setPosition(aBegin, aEnd);
    pAxis->setLineColor(COL_BLUE);

    // Y axis
    maShapes.push_back(o3tl::make_unique<opengl3D::Line>(mpRenderer.get(), nId));
    nId += ID_STEP;
    pAxis = static_cast<opengl3D::Line*>(maShapes.back().get());
    aBegin.x = aBegin.y = 0;
    aEnd = aBegin;
    aEnd.y = nYPos;
    pAxis->setPosition(aBegin, aEnd);
    pAxis->setLineColor(COL_BLUE);

    // Chart background.
    maShapes.push_back(o3tl::make_unique<opengl3D::Rectangle>(mpRenderer.get(), nId));
    nId += ID_STEP;
    opengl3D::Rectangle* pRect = static_cast<opengl3D::Rectangle*>(maShapes.back().get());
    glm::vec3 aTopLeft;
    glm::vec3 aTopRight = aTopLeft;
    aTopRight.x = mbBenchMarkMode ? (mbScrollFlg ? nXEnd - BAR_SIZE_X : nXEnd + 2 * BAR_DISTANCE_X) : (nXEnd + 2 * BAR_DISTANCE_X);
    glm::vec3 aBottomRight = aTopRight;
    aBottomRight.y = nYPos;
    pRect->setPosition(aTopLeft, aTopRight, aBottomRight);
    pRect->setFillColor(COL_BLACK);
    pRect->setLineColor(COL_BLUE);
    if (mbScrollFlg)
        mpRenderer->SetSceneEdge(BAR_DISTANCE_X - 0.001f, aTopRight.x - BAR_DISTANCE_X);
    else
        mpRenderer->SetSceneEdge(-0.001f, aTopRight.x);
    // Create category texts along X-axis at the bottom.
    uno::Sequence<OUString> aCats = rCatProvider.getSimpleCategories();
    for (sal_Int32 i = 0; i < aCats.getLength(); ++i)
    {
        if (mbBenchMarkMode && mbScrollFlg && (i + 1 == aCats.getLength()))
            break;
        maCategories.push_back(aCats[i]);
        if(aCats[i].isEmpty())
            continue;

        float nXPos = i * (BAR_SIZE_X + BAR_DISTANCE_X);

        maShapes.push_back(o3tl::make_unique<opengl3D::Text>(mpRenderer.get(), *mpTextCache,
                    aCats[i], nId));
        nId += ID_STEP;
        opengl3D::Text* p = static_cast<opengl3D::Text*>(maShapes.back().get());
        aTopLeft.x = nXPos + TEXT_HEIGHT + 0.5 * BAR_SIZE_X;
        aTopLeft.y = nYPos + calculateTextWidth(aCats[i]) + 0.5 * BAR_DISTANCE_Y;
        aTopRight = aTopLeft;
        aTopRight.y = nYPos + 0.5* BAR_DISTANCE_Y;
        aBottomRight.x = nXPos;
        aBottomRight.y = nYPos + 0.5 * BAR_DISTANCE_Y;
        p->setPosition(aTopLeft, aTopRight, aBottomRight);

        // create shapes on other side as well

        maShapes.push_back(o3tl::make_unique<opengl3D::Text>(mpRenderer.get(), *mpTextCache,
                    aCats[i], nId));
        nId += ID_STEP;
        p = static_cast<opengl3D::Text*>(maShapes.back().get());
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
        float nAngleX = -pi / 6.5f;
        float nAngleZ = -pi / 8.0f;
        glm::mat4 aDefaultRotateMatrix = glm::eulerAngleYXZ(0.0f, nAngleX, nAngleZ);
        maDefaultCameraPosition = glm::vec3(aDefaultRotateMatrix * glm::vec4(maDefaultCameraPosition, 1.0f));
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
    if (mbBenchMarkMode && (!mpRenderThread.is()))
    {
        //if scroll the bars, set the speed and distance first
        if (mbScrollFlg)
        {
            mpRenderer->SetScrollSpeed((float)(BAR_SIZE_X + BAR_DISTANCE_X) / (float)miScrollRate);
            mpRenderer->SetScrollDistance((float)(BAR_SIZE_X + BAR_DISTANCE_X));
        }
        spawnRenderThread(new RenderBenchMarkThread(this));
    }
    miDataUpdateCounter++;
    mnHistoryCounter++;
    mbNeedsNewRender = true;
}

void GL3DBarChart::joinRenderThread()
{
    if(mpRenderThread.is())
    {
        // FIXME: badly want to assert that we don't
        // hold the mutex here ... but can't API-wise.
        mpRenderThread->join();
    }
}

void GL3DBarChart::spawnRenderThread(RenderThread *pThread)
{
    joinRenderThread(); // not holding maMutex

    osl::MutexGuard aGuard(maMutex);

    Size aSize = mpWindow->GetSizePixel();
    mpWindow->getContext().setWinSize(aSize);
    mpRenderThread.set(pThread);
    mpWindow->getContext().resetCurrent();
    mpRenderThread->launch();
}

void GL3DBarChart::update()
{
    if (mbBenchMarkMode)
        return;
    spawnRenderThread(new RenderOneFrameThread(this));
}

void GL3DBarChart::moveToDefault()
{
    if(mbBenchMarkMode)
    {
        // add correct handling here!!
        if ((maRenderEvent != EVENT_NONE) && (maRenderEvent != EVENT_SHOW_SCROLL) &&
            (maRenderEvent != EVENT_AUTO_FLY) && (maRenderEvent != EVENT_SHOW_SELECT))
            return;

        {
            SharedResourceAccess aResGuard(maCond1, maCond2);
            osl::MutexGuard aGuard(maMutex);
            maRenderEvent = EVENT_MOVE_TO_DEFAULT;
        }
        return;
    }

    spawnRenderThread(new RenderAnimationThread(this, maCameraPosition, maDefaultCameraPosition, STEPS));

    /*
     * TODO: moggi: add to thread
    glm::vec3 maTargetDirection = maDefaultCameraDirection;
    maStepDirection = (maTargetDirection - maCameraDirection)/((float)mnStepsTotal);
    */
}

sal_uInt32 GL3DBarChart::barIdAtPosition(const Point& rPos)
{
    sal_uInt32 nId = 5;
    {
        osl::MutexGuard aGuard(maMutex);
        mpWindow->getContext().makeCurrent();
        mpRenderer->SetPickingMode(true);
        renderFrame();
        nId = mpRenderer->GetPixelColorFromPoint(rPos.X(), rPos.Y());
        mpRenderer->SetPickingMode(false);
        mpWindow->getContext().resetCurrent();
    }
    return nId;
}

void GL3DBarChart::clickedAt(const Point& rPos, sal_uInt16 nButtons)
{
    if (nButtons == MOUSE_RIGHT)
    {
        moveToDefault();
        return;
    }

    if(nButtons != MOUSE_LEFT)
        return;

    if (mbBenchMarkMode)
    {
        // add correct handling here !!
        if ((maRenderEvent != EVENT_NONE) && (maRenderEvent != EVENT_SHOW_SCROLL) &&
            (maRenderEvent != EVENT_AUTO_FLY) && (maRenderEvent != EVENT_SHOW_SELECT))
            return;

        {
            SharedResourceAccess aResGuard(maCond1, maCond2);
            osl::MutexGuard aGuard(maMutex);
            maClickPos = rPos;
            mnPreSelectBarId = mnSelectBarId;
            maPreRenderEvent = maRenderEvent;
            maRenderEvent = EVENT_CLICK;
            maClickCond.reset();
        }
        maClickCond.wait();
        return;
    }

    sal_uInt32 nId = barIdAtPosition(rPos);

    std::map<sal_uInt32, const BarInformation>::const_iterator itr =
        maBarMap.find(nId);

    if(itr == maBarMap.end())
        return;

    const BarInformation& rBarInfo = itr->second;

    {
        osl::MutexGuard aGuard(maMutex);
        mpWindow->getContext().makeCurrent();
        glm::vec3 aTextPos = glm::vec3(rBarInfo.maPos.x + BAR_SIZE_X / 2.0f,
                rBarInfo.maPos.y + BAR_SIZE_Y / 2.0f,
                rBarInfo.maPos.z);
        maShapes.push_back(o3tl::make_unique<opengl3D::ScreenText>(mpRenderer.get(), *mpTextCache,
                    "Value: " + OUString::number(rBarInfo.mnVal), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), CALC_POS_EVENT_ID));
        opengl3D::ScreenText* pScreenText = static_cast<opengl3D::ScreenText*>(maShapes.back().get());
        pScreenText->setPosition(glm::vec2(-0.9f, 0.9f), glm::vec2(-0.6f, 0.8f), aTextPos);
        pScreenText->render();
        mpWindow->getContext().resetCurrent();
    }

    glm::vec3 aTargetPosition = rBarInfo.maPos;
    aTargetPosition.z += 240;
    aTargetPosition.y += BAR_SIZE_Y / 2.0f;

    spawnRenderThread(new RenderAnimationThread(this, maCameraPosition,
                                                aTargetPosition, STEPS));

    /*
     * TODO: moggi: add to thread
    glm::vec3 maTargetDirection = rBarInfo.maPos;
    maTargetDirection.x += BAR_SIZE_X / 2.0f;
    maTargetDirection.y += BAR_SIZE_Y / 2.0f;

    maStepDirection = (maTargetDirection - maCameraDirection)/((float)mnStepsTotal);
    */
}

void GL3DBarChart::render()
{
    if (mbBenchMarkMode)
        return;

    update();
}

void GL3DBarChart::renderFrame()
{
    Size aSize = mpWindow->GetSizePixel();
    mpRenderer->SetSize(aSize);
    if(mbNeedsNewRender)
    {
        mpRenderer->ReleaseTextTexture();
        for(std::unique_ptr<opengl3D::Renderable3DObject>& aObj : maShapes)
        {
            aObj->render();
        }
    }
    else
    {
        mpCamera->render();
    }
    mpRenderer->ProcessUnrenderedShape(mbNeedsNewRender);
    mbNeedsNewRender = false;
}

void GL3DBarChart::mouseDragMove(const Point& rStartPos, const Point& rEndPos, sal_uInt16 )
{
    long nDirection = rEndPos.X() - rStartPos.X();
    SharedResourceAccess aResGuard(maCond1, maCond2);
    osl::ClearableGuard<osl::Mutex> aGuard(maMutex);
    if ((maRenderEvent == EVENT_NONE) || (maRenderEvent == EVENT_SHOW_SCROLL) ||
        (maRenderEvent == EVENT_AUTO_FLY) || (maRenderEvent == EVENT_SHOW_SELECT))
        maRenderEvent = nDirection > 0 ? EVENT_DRAG_RIGHT : EVENT_DRAG_LEFT;
    bool bMove = false;
    if(nDirection < 0)
    {
        mnCornerId = (mnCornerId + 1) % 4;
        bMove = true;
    }
    else if(nDirection > 0)
    {
        mnCornerId = mnCornerId - 1;
        if(mnCornerId < 0)
            mnCornerId = 3;
        bMove = true;
    }

    if (bMove)
    {
        aGuard.clear();
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
    if(mbBenchMarkMode)
    {
        // add correct handling here!!
        return;
    }

    spawnRenderThread(new RenderAnimationThread(this, maCameraPosition,
                                                getCornerPosition(mnCornerId), STEPS));

    // TODO: moggi: add to thread
    // maStepDirection = (glm::vec3(mnMaxX/2.0f, mnMaxY/2.0f, 0) - maCameraDirection)/ float(mnStepsTotal);
}

void GL3DBarChart::scroll(long nDelta)
{
    {
        SharedResourceAccess aResGuard(maCond1, maCond2);
        osl::MutexGuard aGuard(maMutex);
        if ((maRenderEvent != EVENT_NONE) && (maRenderEvent != EVENT_SHOW_SCROLL) &&
            (maRenderEvent != EVENT_AUTO_FLY) && (maRenderEvent != EVENT_SHOW_SELECT))
            return;
        glm::vec3 maDir = glm::normalize(maCameraPosition - maCameraDirection);
        maCameraPosition -= ((float)nDelta/10) * maDir;
        mpCamera->setPosition(maCameraPosition);
        if(mbBenchMarkMode)
        {
            maVectorNearest.clear();
            getNearestBars(maVectorNearest);
            maRenderEvent = EVENT_SCROLL;
        }
    }

    update();
}

void GL3DBarChart::contextDestroyed()
{
    SharedResourceAccess aResGuard(maCond1, maCond2);
    osl::MutexGuard aGuard(maMutex);
    mbValidContext = false;
}

float GL3DBarChart::addScreenTextShape(OUString &nStr, const glm::vec2& rLeftOrRightTop, float nTextHeight, bool bLeftTopFlag,
                                            const glm::vec4& rColor, const glm::vec3& rPos, sal_uInt32 nEvent)
{
    maScreenTextShapes.push_back(o3tl::make_unique<opengl3D::ScreenText>(mpRenderer.get(), *mpTextCache, nStr, rColor, nEvent));
    const opengl3D::TextCacheItem& rTextCache = mpTextCache->getText(nStr);
    float nRectWidth = (float)rTextCache.maSize.Width() / (float)rTextCache.maSize.Height() * nTextHeight / 2.0f;
    opengl3D::ScreenText* pScreenText = static_cast<opengl3D::ScreenText*>(maScreenTextShapes.back().get());
    if (bLeftTopFlag)
        pScreenText->setPosition(rLeftOrRightTop, glm::vec2(rLeftOrRightTop.x + nRectWidth, rLeftOrRightTop.y - nTextHeight), rPos);
    else
        pScreenText->setPosition(glm::vec2(rLeftOrRightTop.x - nRectWidth, rLeftOrRightTop.y), glm::vec2(rLeftOrRightTop.x, rLeftOrRightTop.y - nTextHeight), rPos);
    return nRectWidth;
}

void GL3DBarChart::updateRenderFPS()
{
    int nDeltaMs = calcTimeInterval(maFPSRenderStartTime, maFPSRenderEndTime);
    if(nDeltaMs >= FPS_TIME)
    {
        osl_getSystemTime(&maFPSRenderEndTime);
        nDeltaMs = calcTimeInterval(maFPSRenderStartTime, maFPSRenderEndTime);
        int iFPS = miFrameCount * 1000 / nDeltaMs;
        maFPS =  OUString::number(iFPS);
        miFrameCount = 0;
        osl_getSystemTime(&maFPSRenderStartTime);
    }
    osl_getSystemTime(&maFPSRenderEndTime);
    OUString aFPS = "Render FPS: ";
    addScreenTextShape(aFPS, glm::vec2(-0.77f, 0.99f), 0.07f, false, glm::vec4(0.0f, 1.0f, 1.0f, 0.0f));
    addScreenTextShape(maFPS, glm::vec2(-0.77f, 0.99f), 0.07f, true,
                       glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
}

int GL3DBarChart::calcTimeInterval(TimeValue &startTime, TimeValue &endTime)
{
    TimeValue aTime;
    aTime.Seconds = endTime.Seconds - startTime.Seconds - 1;
    aTime.Nanosec = 1000000000 + endTime.Nanosec - startTime.Nanosec;
    aTime.Seconds += aTime.Nanosec / 1000000000;
    aTime.Nanosec %= 1000000000;
    return aTime.Seconds * 1000+aTime.Nanosec / 1000000;
}

void GL3DBarChart::updateScreenText()
{
    SharedResourceAccess aResGuard(maCond1, maCond2);
    osl::MutexGuard aGuard(maMutex);
    maScreenTextShapes.clear();
    mpRenderer->ReleaseScreenTextShapes();
    updateRenderFPS();
    updateDataUpdateFPS();
    updateClickEvent();
    updateScroll();
    mbScreenTextNewRender = true;
}

void GL3DBarChart::updateDataUpdateFPS()
{
    int nDeltaMs = calcTimeInterval(maDataUpdateStartTime, maDataUpdateEndTime);
    if(nDeltaMs >= DATAUPDATE_FPS_TIME)
    {
        int iFPS = miDataUpdateCounter * 1000 / nDeltaMs;
        if (iFPS)
        {
            maDataUpdateFPS = OUString::number(iFPS);
        }
        else
        {
            float fFPS = (float)miDataUpdateCounter * 1000 / (float)nDeltaMs;
            maDataUpdateFPS = OUString::number(fFPS);
        }
        miDataUpdateCounter = 0;
        osl_getSystemTime(&maDataUpdateStartTime);
    }
    osl_getSystemTime(&maDataUpdateEndTime);
    OUString aDataUpdateFPS = "Data Update Rate: ";
    addScreenTextShape(aDataUpdateFPS, glm::vec2(-0.77, 0.92f), 0.07f, false, glm::vec4(0.0f, 1.0f, 1.0f, 0.0f));
    addScreenTextShape(maDataUpdateFPS, glm::vec2(-0.77f, 0.92f), 0.07f, true, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
}

void GL3DBarChart::recordBarHistory(sal_uInt32 &nBarID, float &nVal)
{
    std::list<float>& aList = maBarHistory[nBarID];
    if(aList.size() == HISTORY_NUM)
        aList.pop_front();
    aList.push_back(nVal);
}

void GL3DBarChart::getNeighborBarID(sal_uInt32 nSelectBarId, sal_uInt32 *pNeighborBarId)
{
    sal_uInt32 nSelectRow = (nSelectBarId - SHAPE_START_ID) / ID_STEP / (mnBarsInRow + 1);
    for (sal_Int32 i = 0; i < DISPLAY_BARS_NUM; i++)
    {
        pNeighborBarId[i] = nSelectBarId + (i - DISPLAY_BARS_NUM / 2) * ID_STEP;
        if (pNeighborBarId[i] == nSelectBarId)
            continue;
        if ((pNeighborBarId[i] - SHAPE_START_ID) / ID_STEP / (mnBarsInRow + 1) != nSelectRow)
            pNeighborBarId[i] = 0;
    }
}

void GL3DBarChart::addMovementScreenText(sal_uInt32 nBarId)
{
    if (nBarId == 0)
        return;
    std::map<sal_uInt32, const BarInformation>::const_iterator itr = maBarMap.find(nBarId);
    if (itr == maBarMap.end())
        return;
    const BarInformation& rBarInfo = itr->second;
    glm::vec3 aTextPos = glm::vec3(rBarInfo.maPos.x + BAR_SIZE_X / 2.0f,
                                  rBarInfo.maPos.y + BAR_SIZE_Y / 2.0f,
                                  rBarInfo.maPos.z);
    OUString aBarValue = "Value: " + OUString::number(rBarInfo.mnVal);
    maScreenTextShapes.push_back(o3tl::make_unique<opengl3D::ScreenText>(mpRenderer.get(), *mpTextCache, aBarValue, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), CALC_POS_EVENT_ID, true));
    const opengl3D::TextCacheItem& rTextCache = mpTextCache->getText(aBarValue);
    float nRectWidth = (float)rTextCache.maSize.Width() / (float)rTextCache.maSize.Height() * 0.024;
    opengl3D::ScreenText* pScreenText = static_cast<opengl3D::ScreenText*>(maScreenTextShapes.back().get());
    pScreenText->setPosition(glm::vec2(-nRectWidth / 2, 0.03f), glm::vec2(nRectWidth / 2, -0.03f), aTextPos);
}

void GL3DBarChart::updateClickEvent()
{
    if (maRenderEvent == EVENT_CLICK || maRenderEvent == EVENT_AUTO_FLY || maRenderEvent == EVENT_SHOW_SELECT)
    {
        std::list<float>& aList = maBarHistory[mnSelectBarId];
        sal_uInt32 nIdex = 0;
        sal_uInt32 nBarIdArray[DISPLAY_BARS_NUM] = {0};
        OUString aTitle;
        OUString aBarValue;
        float nXCoordStart, nYCoordStart, nTextWidth, nMaxXCoord = 0.0f, nMinXCoord = 1.0f, nMaxHight = 0.0f;
        //write title
        if (aList.size() > 1)
        {
            aTitle = "Time      ";
            nTextWidth = addScreenTextShape(aTitle, glm::vec2(0.875, 0.99f), 0.07f, false, glm::vec4(0.0f, 1.0f, 1.0f, 0.5f));
            nMinXCoord = std::min(nMinXCoord, 0.875f - nTextWidth);
            aTitle = "   Value";
            nTextWidth = addScreenTextShape(aTitle, glm::vec2(0.875f, 0.99f), 0.07f, true, glm::vec4(0.0f, 1.0f, 1.0f, 0.5f));
            nMaxXCoord = std::max(nMaxXCoord, 0.875f + nTextWidth);
        }
        if (aList.size() > COLUMNSIZE)
        {
            aTitle = "Time      ";
            nTextWidth = addScreenTextShape(aTitle, glm::vec2(0.55f, 0.99f), 0.07f, false, glm::vec4(0.0f, 1.0f, 1.0f, 0.5f));
            nMinXCoord = std::min(nMinXCoord, 0.55f - nTextWidth);
            aTitle = "   Value";
            nTextWidth = addScreenTextShape(aTitle, glm::vec2(0.55f, 0.99f), 0.07f, true, glm::vec4(0.0f, 1.0f, 1.0f, 0.5f));
            nMaxXCoord = std::max(nMaxXCoord, 0.55f + nTextWidth);
        }
        getNeighborBarID(mnSelectBarId, nBarIdArray);
        for (std::list<float>::iterator it = aList.begin();it != aList.end();++it)
        {
            if (nIdex + 1 < aList.size())
            {
                aTitle = "[Time:" + OUString::number((mnHistoryCounter - aList.size() + nIdex)) + "]: ";
                if (nIdex == 0)
                {
                    aTitle = "Most Recent" + aTitle;
                }
                if (aList.size() <= COLUMNSIZE)
                {
                    nXCoordStart = 0.875f;
                    nYCoordStart = (nIdex + 1) * 0.07f;
                }
                else
                {
                    nXCoordStart = nIdex < COLUMNSIZE ? 0.55f : 0.875f;
                    nYCoordStart = nIdex < COLUMNSIZE ? (nIdex + 1) * 0.07f : (nIdex - 24) * 0.07f;
                }
                nMaxHight = std::max(nMaxHight, nYCoordStart + 0.07f);
                nTextWidth = addScreenTextShape(aTitle, glm::vec2(nXCoordStart, 0.99f - nYCoordStart), 0.07f, false, glm::vec4(0.0f, 1.0f, 1.0f, 0.5f));
                nMinXCoord = std::min(nMinXCoord, nXCoordStart - nTextWidth);
                aBarValue = OUString::number(*it);
                nTextWidth = addScreenTextShape(aBarValue, glm::vec2(nXCoordStart, 0.99f - nYCoordStart), 0.07f, true, glm::vec4(0.0f, 1.0f, 1.0f, 0.5f));
                nMaxXCoord = std::max(nMaxXCoord, nXCoordStart + nTextWidth);
            }
            nIdex++;
        }
        for (sal_uInt32 i = 0; i < DISPLAY_BARS_NUM; i++)
        {
            addMovementScreenText(nBarIdArray[i]);
        }
        //add translucent back ground
        aTitle = " ";
        maScreenTextShapes.push_back(o3tl::make_unique<opengl3D::ScreenText>(mpRenderer.get(), *mpTextCache, aTitle, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f), 0));
        opengl3D::ScreenText* pScreenText = static_cast<opengl3D::ScreenText*>(maScreenTextShapes.back().get());
        pScreenText->setPosition(glm::vec2(nMinXCoord, 0.99f), glm::vec2(nMaxXCoord, 0.99f - nMaxHight));
    }
}

float GL3DBarChart::calcScrollDistance(const glm::mat4& rMVP, const glm::vec3& rPos)
{
    glm::vec4 aScreenPos = rMVP * glm::vec4(rPos, 1.0);
    glm::vec3 aActualPos = glm::vec3(aScreenPos.x / aScreenPos.w, aScreenPos.y / aScreenPos.w, 0.0);
    return glm::length(aActualPos);
}

void GL3DBarChart::calcDistance(std::vector<sal_uInt32> & rVectorNearest)
{
    int i =0;
    glm::mat4 aProjection = mpRenderer->GetProjectionMatrix();
    glm::mat4 aView = mpRenderer->GetViewMatrix();
    glm::mat4 aScale = mpRenderer->GetGlobalScaleMatrix();
    glm::mat4 aMVP = aProjection * aView * aScale;
    std::map<sal_uInt32, const BarInformation>::iterator it;
    for(it= maBarMap.begin(); it!= maBarMap.end(); ++it)
    {
        sal_uInt32 nId = it->first;
        if(i < SHOW_VALUE_COUNT)
        {
            rVectorNearest.push_back(nId);
            i++;
        }
        maDistanceMap[nId] = calcScrollDistance(aMVP, glm::vec3(it->second.maPos.x + BAR_SIZE_X / 2.0f,
                                                               it->second.maPos.y + BAR_SIZE_Y / 2.0f,
                                                               it->second.maPos.z));
    }
}

void GL3DBarChart::initDistanceHeap(std::vector<sal_uInt32> &rVectorNearest)
{
    for(int i= (rVectorNearest.size()-2)/2; i>= 0; i--)
    {
        keepHeap(rVectorNearest, i);
    }
}

void GL3DBarChart::keepHeap(std::vector<sal_uInt32> &rVectorNearest, int nIndex)
{
    size_t nParentIndex = nIndex;
    while(nParentIndex < rVectorNearest.size())
    {
        size_t nLeftIndex = nParentIndex * 2 + 1;
        size_t nRightIndex = nLeftIndex +1;
        if(nLeftIndex >= rVectorNearest.size())
            break;
        size_t nFarthestIndex = nLeftIndex;
        float nFarthest = maDistanceMap[rVectorNearest[nLeftIndex]];
        if(nRightIndex < rVectorNearest.size())
        {
            float nRight = maDistanceMap[rVectorNearest[nRightIndex]];
            if(nRight > nFarthest)
            {
                nFarthest = nRight;
                nFarthestIndex = nRightIndex;
            }
        }
        float nParent = maDistanceMap[rVectorNearest[nParentIndex]];
        if(nParent >= nFarthest)
            break;
        else
        {
            swapVector(nParentIndex , nFarthestIndex, rVectorNearest);
            nParentIndex = nFarthestIndex;
        }
    }

}

void GL3DBarChart::swapVector(int i, int j, std::vector<sal_uInt32> &rVectorNearest)
{
    sal_uInt32 nTmp = rVectorNearest[i];
    rVectorNearest[i] = rVectorNearest[j];
    rVectorNearest[j] = nTmp;
}

void GL3DBarChart::getNearestBars(std::vector<sal_uInt32> &rVectorNearest)
{
    calcDistance(rVectorNearest);
    initDistanceHeap(rVectorNearest);
    std::map<sal_uInt32, float>::iterator it;
    int i = 0;
    for(it= maDistanceMap.begin(); it!= maDistanceMap.end(); ++it)
    {
        i++;
        if(i <= SHOW_VALUE_COUNT)
            continue;
        float nDistance = it->second;
        float nHeaphead = maDistanceMap[rVectorNearest[0]];
        if(nDistance < nHeaphead)
        {
            rVectorNearest[0] = it->first;
            keepHeap(rVectorNearest, 0);
        }
    }
}

void GL3DBarChart::updateScroll()
{
    if ((maRenderEvent == EVENT_SCROLL) || (maRenderEvent == EVENT_SHOW_SCROLL))
    {
        float fMinDistance = 0.0f;
        std::vector<BarInformation> aBarInfoList;
        for(size_t i= 0;i < maVectorNearest.size(); i++)
        {
            //get bar height position
            std::map<sal_uInt32, const BarInformation>::const_iterator itr = maBarMap.find(maVectorNearest[i]);
            const BarInformation& rBarInfo = itr->second;
            aBarInfoList.push_back(rBarInfo);
            glm::vec3 aPos = rBarInfo.maPos;
            fMinDistance = (fMinDistance == 0.0f) ? glm::length(aPos - maCameraPosition) :
                                 std::min(glm::length(aPos - maCameraPosition), fMinDistance);
        }

        if (fMinDistance <= SHOW_SCROLL_TEXT_DISTANCE)
        {
            //update scroll value
            for(size_t i = 0; i < aBarInfoList.size(); i++)
            {
                OUString aBarValue = "Value: " + OUString::number(aBarInfoList[i].mnVal);
                maScreenTextShapes.push_back(o3tl::make_unique<opengl3D::ScreenText>(mpRenderer.get(), *mpTextCache, aBarValue, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), CALC_POS_EVENT_ID, true));
                const opengl3D::TextCacheItem& rTextCache = mpTextCache->getText(aBarValue);
                float nRectWidth = (float)rTextCache.maSize.Width() / (float)rTextCache.maSize.Height() * 0.024;
                glm::vec3 aTextPos = glm::vec3(aBarInfoList[i].maPos.x + BAR_SIZE_X / 2.0f,
                                      aBarInfoList[i].maPos.y + BAR_SIZE_Y / 2.0f,
                                      aBarInfoList[i].maPos.z);
                opengl3D::ScreenText* pScreenText = static_cast<opengl3D::ScreenText*>(maScreenTextShapes.back().get());
                pScreenText->setPosition(glm::vec2(-nRectWidth / 2, 0.03f), glm::vec2(nRectWidth / 2, -0.03f), aTextPos);
            }
        }
    }
}

void GL3DBarChart::processAutoFly(sal_uInt32 nId, sal_uInt32 nColor)
{
    //record the color
    sal_uInt32 nPreColor = maBarColorMap[nId];
    maBarColorMap[nId] = nColor;
    //if has manul event, just record the color and process manul event first
    if ((maRenderEvent != EVENT_NONE))
    {
        return;
    }
    //calc the percentage of color change
    int nColorRate = (nColor - nPreColor) * 100 / nPreColor;
    nColorRate = abs(nColorRate);
    if (nColorRate >= FLY_THRESHOLD)
    {
        maRenderEvent = EVENT_AUTO_FLY;
        mnSelectBarId = nColorRate > mnColorRate ? nId : mnSelectBarId;
        mnPreSelectBarId = mnSelectBarId;
        mnColorRate = nColorRate > mnColorRate ? nColorRate : mnColorRate;
    }
}

IMPL_LINK_NOARG_TYPED(GL3DBarChart, UpdateTimerHdl, Idle *, void)
{
    updateScreenText();
    maIdle.Start();
}

void GL3DBarChart::setOpenGLWindow(OpenGLWindow* pWindow)
{
    if (mpWindow.get() != pWindow)
    {
        mpWindow = pWindow;
        Size aSize = mpWindow->GetSizePixel();
        mpRenderer->SetSize(aSize);
        mpWindow->setRenderer(this);
        mpWindow->getContext().makeCurrent();
        mpRenderer->init();
        mpWindow->getContext().resetCurrent();
        mbValidContext = true;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
