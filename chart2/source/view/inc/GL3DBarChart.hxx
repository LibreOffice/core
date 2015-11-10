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

#include <glm/glm.hpp>

#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/openglwin.hxx>

#include <rtl/ref.hxx>
#include <salhelper/thread.hxx>
#include <osl/conditn.hxx>

namespace chart {

class ExplicitCategoriesProvider;

namespace opengl3D {

class Renderable3DObject;
class OpenGL3DRenderer;
class TextCache;
class Camera;

}

enum RenderEventType
{
    EVENT_NONE,
    EVENT_CLICK,
    EVENT_MOVE_TO_DEFAULT,
    EVENT_DRAG_LEFT,
    EVENT_DRAG_RIGHT,
    EVENT_SCROLL,
    EVENT_SHOW_SCROLL,
    EVENT_SHOW_SELECT,
    EVENT_AUTO_FLY,
    EVENT_DIE,
    EVENT_SELECTBAR_UPDEDATE
};

class RenderThread;
class RenderOneFrameThread;
class RenderAnimationThread;

class GL3DBarChart : public GL3DPlotterBase, public IRenderer
{
    friend class RenderThread;
    friend class RenderOneFrameThread;
    friend class RenderAnimationThread;
    friend class RenderBenchMarkThread;
public:
    GL3DBarChart(
        const css::uno::Reference<css::chart2::XChartType>& xChartType,
        OpenGLWindow* pContext);

    virtual ~GL3DBarChart();

    virtual void create3DShapes(const boost::ptr_vector<VDataSeries>& rDataSeries,
        ExplicitCategoriesProvider& rCatProvider) override;

    virtual void render() override;

    /// Render one frame of the 3D bar chart.
    void renderFrame();

    virtual void update() override;

    /// Draw to the framebuffer context, and provide the ID of the bar that the user has clicked.
    sal_uInt32 barIdAtPosition(const Point& rPos);
    virtual void clickedAt(const Point& rPos, sal_uInt16 nButtons) override;

    virtual void mouseDragMove(const Point& rStartPos, const Point& rEndPos, sal_uInt16 nButtons) override;
    virtual void scroll(long nDelta) override;
    virtual void contextDestroyed() override;

    void setOpenGLWindow(OpenGLWindow* pWindow);

private:

    void spawnRenderThread(RenderThread *pThread);
    void joinRenderThread();
    void moveToCorner();
    void moveToDefault();
    glm::vec3 getCornerPosition(sal_Int8 nCornerId);
    void updateScreenText();
    void updateRenderFPS();
    void updateDataUpdateFPS();
    DECL_LINK_TYPED(UpdateTimerHdl, Idle*, void);
    static int calcTimeInterval(TimeValue &startTime, TimeValue &endTime);
    float addScreenTextShape(OUString &nStr, const glm::vec2& rLeftOrRightTop, float nTextHeight, bool bLeftTopFlag = true,
                                  const glm::vec4& rColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.0f),
                                  const glm::vec3& rPos = glm::vec3(0.0f, 0.0f, 0.0f),
                                  sal_uInt32 nEvent = 0);
    void recordBarHistory(sal_uInt32 &nBarID, float &nVal);
    void updateClickEvent();
    void calcDistance(std::vector<sal_uInt32> &vectorNearest);
    static float calcScrollDistance(const glm::mat4 &mvp, const glm::vec3& rPos);
    void initDistanceHeap(std::vector<sal_uInt32> &vectorNearest);
    void keepHeap(std::vector<sal_uInt32> &vectorNearest, int index);
    static void swapVector(int i, int j, std::vector<sal_uInt32> &vectorNearest);
    void getNearestBars(std::vector<sal_uInt32> &vectorNearest);
    void updateScroll();
    void processAutoFly(sal_uInt32 nId, sal_uInt32 nColor);
    void getNeighborBarID(sal_uInt32 nSelectBarId, sal_uInt32 *pNeighborBarId);
    void addMovementScreenText(sal_uInt32 nBarId);
    css::uno::Reference<css::chart2::XChartType> mxChartType;
    std::vector<std::unique_ptr<opengl3D::Renderable3DObject> > maShapes;

    std::unique_ptr<opengl3D::OpenGL3DRenderer> mpRenderer;
    VclPtr<OpenGLWindow> mpWindow;

    opengl3D::Camera* mpCamera;
    bool mbValidContext;

    std::unique_ptr<opengl3D::TextCache> mpTextCache;

    glm::vec3 maCameraPosition;
    glm::vec3 maCameraDirection;

    glm::vec3 maDefaultCameraPosition;
    glm::vec3 maDefaultCameraDirection;

    glm::vec3 maStepDirection;
    float mnMaxX;
    float mnMaxY;
    float mnDistance;
    /**
     * 0 = corner at (0,0,0);
     * numbering counter clockwise
     */
    sal_Int8 mnCornerId;

    std::vector<OUString> maCategories;
    std::vector<OUString> maSeriesNames;
    struct BarInformation
    {
        glm::vec3 maPos;
        float mnVal;
        sal_Int32 mnIndex;
        sal_Int32 mnSeriesIndex;

        BarInformation(const glm::vec3& rPos, float nVal,
                sal_Int32 nIndex, sal_Int32 nSeriesIndex);
    };

    std::map<sal_uInt32, const BarInformation> maBarMap;
    bool mbNeedsNewRender;
    bool mbCameraInit;

    osl::Mutex maMutex;
    rtl::Reference<RenderThread> mpRenderThread;
    bool mbRenderDie;
    ::osl::Condition maClickCond;
    RenderEventType maRenderEvent;
    RenderEventType maPreRenderEvent;
    sal_uInt32 mnSelectBarId;
    sal_uInt32 mnPreSelectBarId;
    Point maClickPos;
    sal_uInt32 miScrollRate;
    bool mbScrollFlg;
    Idle maIdle;
    bool mbScreenTextNewRender;
    std::vector<std::unique_ptr<opengl3D::Renderable3DObject>> maScreenTextShapes;
    OUString maFPS;
    OUString maDataUpdateFPS;
    sal_uInt32 miFrameCount;
    sal_uInt32 miDataUpdateCounter;
    TimeValue maFPSRenderStartTime;
    TimeValue maFPSRenderEndTime;
    TimeValue maDataUpdateStartTime;
    TimeValue maDataUpdateEndTime;
    std::map<sal_uInt32, std::list<float> > maBarHistory;
    std::vector<sal_uInt32> maVectorNearest;
    std::map<sal_uInt32, float> maDistanceMap;
    std::map<sal_uInt32, sal_uInt32> maBarColorMap;
    int mnColorRate;
    bool mbBenchMarkMode;
    sal_uInt32 mnHistoryCounter;
    sal_uInt32 mnBarsInRow;
    bool mbAutoFly;
    sal_Int32 mnUpdateBarId;

    // these form a pair:
    // main thread sets condition 1 and waits until it can take the mutex
    // render thread checks condition 1 before taking the mutex and waits in case it
    // is set until condition 2 is set
    //
    // only necessary for the benchmark mode
    osl::Condition maCond1;
    osl::Condition maCond2;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
