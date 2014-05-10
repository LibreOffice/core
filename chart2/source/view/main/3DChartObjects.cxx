/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "3DChartObjects.hxx"
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

#include <ostream>

namespace chart {

namespace opengl3D {

namespace {

std::ostream& operator<<(std::ostream& rStrm, const glm::mat4& rMatrix)
{
    for(int i = 0; i < 4; ++i)
    {
        rStrm << "( ";
        for(int j = 0; j < 4; ++j)
        {
            rStrm << rMatrix[j][i];
            rStrm << " ";
        }
        rStrm << ")\n";
    }
    return rStrm;
}

}

Renderable3DObject::Renderable3DObject(OpenGL3DRenderer* pRenderer, sal_uInt32 nId):
    mpRenderer(pRenderer),
    mnUniqueId(nId)
{
}

void Renderable3DObject::render()
{
    (void) mnUniqueId;
}

Bar::Bar(OpenGL3DRenderer* pRenderer, const glm::mat4& rPosition, sal_Int32 aColor, sal_uInt32 nId)
    : Renderable3DObject(pRenderer, nId)
    , mbRoundedCorners(false)
    , maPos(rPosition)
    , maColor(aColor)
{
    SAL_INFO("chart2.3dopengl", rPosition);
}

void Bar::render()
{
    mpRenderer->AddShape3DExtrudeObject(mbRoundedCorners, (sal_Int32)maColor.GetColor(), 0xFFFFFF, maPos);
    mpRenderer->EndAddShape3DExtrudeObject();
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

void Line::setPosition(const glm::vec3& rBegin, const glm::vec3& rEnd)
{
    maPosBegin = rBegin;
    maPosEnd = rEnd;
}

void Line::setLineColor(const Color& rColor)
{
    maLineColor = rColor;
}

Text::Text(OpenGL3DRenderer* pRenderer, const OUString& rStr, sal_uInt32 nId):
    Renderable3DObject(pRenderer, nId)
{
    // Convert OUString to BitmapEx.
    VirtualDevice aDevice(*Application::GetDefaultDevice(), 0, 0);
    Font aFont = aDevice.GetFont();
    aFont.SetColor(COL_WHITE);
    aDevice.SetFont(aFont);
    aDevice.Erase();
    aDevice.SetOutputSizePixel(Size(20,12));
    aDevice.SetBackground(Wallpaper(COL_TRANSPARENT));
    aDevice.DrawText(Point(0,0), rStr);

    maText = BitmapEx(aDevice.GetBitmapEx(Point(0,0), Size(20,12)));
}

void Text::render()
{
    glm::vec3 dir2 = maTopRight - maTopLeft;
    glm::vec3 bottomLeft = maBottomRight - dir2;
    mpRenderer->CreateTextTexture(maText, maTopLeft, maTopRight, maBottomRight, bottomLeft);
}

Size Text::getSize() const
{
    return maText.GetSizePixel();
}

void Text::setPosition(const glm::vec3& rTopLeft, const glm::vec3& rTopRight, const glm::vec3& rBottomRight)
{
    maTopLeft = rTopLeft;
    maTopRight = rTopRight;
    maBottomRight = rBottomRight;
}

Rectangle::Rectangle(OpenGL3DRenderer* pRenderer, sal_uInt32 nId):
    Renderable3DObject(pRenderer, nId)
{
}

void Rectangle::render()
{
    glm::vec3 dir1 = maBottomRight - maTopLeft;
    glm::vec3 dir2 = maTopRight - maTopLeft;
    glm::vec3 normal = glm::normalize(glm::cross(dir1, dir2));
    mpRenderer->AddShapePolygon3DObject((sal_Int32)maColor.GetColor(), false, 0, 1, 0xFFFFFF);
    glm::vec3 bottomLeft = maBottomRight - dir2;
    //set polygon points and normals
    mpRenderer->AddPolygon3DObjectPoint(maBottomRight.x, maBottomRight.y, maBottomRight.z);
    mpRenderer->AddPolygon3DObjectNormalPoint(normal.x, normal.y, normal.z);
    mpRenderer->AddPolygon3DObjectPoint(maTopRight.x, maTopRight.y, maTopRight.z);
    mpRenderer->AddPolygon3DObjectNormalPoint(normal.x, normal.y, normal.z);
    mpRenderer->AddPolygon3DObjectPoint(maTopLeft.x, maTopLeft.y, maTopLeft.z);
    mpRenderer->AddPolygon3DObjectNormalPoint(normal.x, normal.y, normal.z);
    mpRenderer->AddPolygon3DObjectPoint(bottomLeft.x, bottomLeft.y, bottomLeft.z);
    mpRenderer->AddPolygon3DObjectNormalPoint(normal.x, normal.y, normal.z);
    mpRenderer->EndAddPolygon3DObjectPoint();
    mpRenderer->EndAddPolygon3DObjectNormalPoint();
    //we should render the edge if the edge color is different from the fill color
    if ((sal_Int32)maColor.GetColor() != (sal_Int32)maLineColor.GetColor())
    {
        mpRenderer->AddShapePolygon3DObject(0, true, (sal_Int32)maLineColor.GetColor(), 0, 0xFFFFFF);
        mpRenderer->AddPolygon3DObjectPoint(maBottomRight.x, maBottomRight.y, maBottomRight.z);
        mpRenderer->AddPolygon3DObjectPoint(maTopRight.x, maTopRight.y, maTopRight.z);
        mpRenderer->AddPolygon3DObjectPoint(maTopLeft.x, maTopLeft.y, maTopLeft.z);
        mpRenderer->AddPolygon3DObjectPoint(bottomLeft.x, bottomLeft.y, bottomLeft.z);
        mpRenderer->EndAddPolygon3DObjectPoint();
    }
    mpRenderer->EndAddShapePolygon3DObject();
}

void Rectangle::setPosition(const glm::vec3& rTopLeft, const glm::vec3& rTopRight, const glm::vec3& rBottomRight)
{
    maTopLeft = rTopLeft;
    maTopRight = rTopRight;
    maBottomRight = rBottomRight;
}

void Rectangle::setFillColor(const Color& rColor)
{
    maColor = rColor;
}

void Rectangle::setLineColor(const Color& rColor)
{
    maLineColor = rColor;
}

Camera::Camera(OpenGL3DRenderer* pRenderer):
    Renderable3DObject(pRenderer, 0),
    maPos(10,-50,10),
    maUp(0, 0, 1),
    maDirection(glm::vec3(0,1,-0.2))
{
}

void Camera::render()
{
    mpRenderer->SetCameraInfo(maPos, maDirection, maUp);
}

void Camera::zoom(sal_uInt32 /*nId*/)
{
    // TODO here
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
