/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "3DChartObjects.hxx"

namespace chart {

namespace opengl3D {

Renderable3DObject::Renderable3DObject(OpenGL3DRenderer* pRenderer, sal_uInt32 nId):
    mpRenderer(pRenderer),
    mnUniqueId(nId)
{
}

void Renderable3DObject::render()
{
    (void) mnUniqueId;
}

Bar::Bar(OpenGL3DRenderer* pRenderer, const glm::mat4& rPosition, sal_uInt32 nId)
    : Renderable3DObject(pRenderer, nId)
    , mbRoundedCorners(false)
    , maPos(rPosition)
{
}

void Bar::render()
{
    (void) mbRoundedCorners;
}

Line::Line(OpenGL3DRenderer* pRenderer, sal_uInt32 nId):
    Renderable3DObject(pRenderer, nId)
{
}

void Line::render()
{
    mpRenderer->AddShapePolygon3DObject(0, true, (sal_Int32)maLineColor.GetColor(), 0, 0);
    mpRenderer->AddPolygon3DObjectPoint(maPosBegin.x, maPosBegin.y, maPosBegin.z);
    mpRenderer->AddPolygon3DObjectPoint(maPosEnd.x, maPosEnd.y, maPosEnd.z);
    mpRenderer->EndAddShapePolygon3DObject();
}

Text::Text(OpenGL3DRenderer* pRenderer, sal_uInt32 nId):
    Renderable3DObject(pRenderer, nId)
{
}

Rectangle::Rectangle(OpenGL3DRenderer* pRenderer, sal_uInt32 nId):
    Renderable3DObject(pRenderer, nId)
{
}

Camera::Camera(OpenGL3DRenderer* pRenderer):
    Renderable3DObject(pRenderer, 0),
    maPos(10,10,-10),
    maUp(0, 1, 0),
    maDirection(glm::vec3(0,0,0)-maPos)
{
}

void Camera::render()
{
    mpRenderer->SetCameraInfo(maPos, maDirection, maUp, true);
}


namespace temporary {

TemporaryContext::TemporaryContext(OpenGL3DRenderer* pRenderer):
    Renderable3DObject(pRenderer, -1)
{
}

void TemporaryContext::render()
{
    maContext.renderToFile();
}

void TemporaryContext::init()
{
    maContext.init();
    Size winSize(800, 600);
    maContext.setWinSize(winSize);
    mpRenderer->SetSize(winSize);
}

}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
