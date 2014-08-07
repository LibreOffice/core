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
    EVENT_DIE
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
        OpenGLWindow& rContext);

    virtual ~GL3DBarChart();

    virtual void create3DShapes(const boost::ptr_vector<VDataSeries>& rDataSeries,
        ExplicitCategoriesProvider& rCatProvider) SAL_OVERRIDE;

    virtual void render() SAL_OVERRIDE;

    virtual void update() SAL_OVERRIDE;

    virtual void clickedAt(const Point& rPos, sal_uInt16 nButtons) SAL_OVERRIDE;
    virtual void mouseDragMove(const Point& rStartPos, const Point& rEndPos, sal_uInt16 nButtons) SAL_OVERRIDE;
    virtual void scroll(long nDelta) SAL_OVERRIDE;
    virtual void contextDestroyed() SAL_OVERRIDE;

private:

    void moveToCorner();
    void moveToDefault();
    glm::vec3 getCornerPosition(sal_Int8 nCornerId);

    css::uno::Reference<css::chart2::XChartType> mxChartType;
    boost::ptr_vector<opengl3D::Renderable3DObject> maShapes;

    boost::scoped_ptr<opengl3D::OpenGL3DRenderer> mpRenderer;
    OpenGLWindow& mrWindow;

    opengl3D::Camera* mpCamera;
    bool mbValidContext;

    boost::scoped_ptr<opengl3D::TextCache> mpTextCache;

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
    sal_uInt32 mSelectBarId;
    Point maClickPos;
    sal_uInt32 miScrollRate;
    bool mbScrollFlg;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
