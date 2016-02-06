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

#include <vcl/opengl/GLMHelper.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/bitmapaccess.hxx>

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

Bar::Bar(OpenGL3DRenderer* pRenderer, const glm::mat4& rPosition, sal_uInt32 aColor, sal_uInt32 nId)
    : Renderable3DObject(pRenderer, nId)
    , mbRoundedCorners(true)
    , maPos(rPosition)
    , maColor(aColor)
{
    SAL_INFO("chart2.3dopengl", rPosition);
}

void Bar::render()
{
    mpRenderer->AddShape3DExtrudeObject(mbRoundedCorners, maColor.GetColor(), 0xFFFFFF, maPos, mnUniqueId);
    mpRenderer->EndAddShape3DExtrudeObject();
}

Line::Line(OpenGL3DRenderer* pRenderer, sal_uInt32 nId):
    Renderable3DObject(pRenderer, nId)
{
}

void Line::render()
{
    mpRenderer->AddShapePolygon3DObject(0, true, maLineColor.GetColor(), 0, 0, mnUniqueId);
    mpRenderer->AddPolygon3DObjectPoint(maPosBegin.x, maPosBegin.y, maPosBegin.z);
    mpRenderer->AddPolygon3DObjectPoint(maPosEnd.x, maPosEnd.y, maPosEnd.z);
    mpRenderer->EndAddPolygon3DObjectPoint();
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

const TextCacheItem& TextCache::getText(OUString const & rText, bool bIs3dText)
{
    TextCacheType::const_iterator itr = maTextCache.find(rText);
    if(itr != maTextCache.end())
        return *itr->second;

    ScopedVclPtrInstance< VirtualDevice > pDevice(*Application::GetDefaultDevice(),
                                                  DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    vcl::Font aFont;
    if(bIs3dText)
        aFont = vcl::Font("Brillante St",Size(0,0));
    else
        aFont = pDevice->GetFont();
    aFont.SetSize(Size(0, 96));
    static bool bOldRender = getenv("OLDRENDER");
    if (bOldRender)
        aFont.SetColor(COL_BLACK);
    else
        aFont.SetColor(COL_GREEN); // RGB_COLORDATA(0xf0, 0xf0, 0xf0));
    pDevice->SetFont(aFont);
    pDevice->Erase();

    pDevice->SetOutputSize(Size(pDevice->GetTextWidth(rText), pDevice->GetTextHeight()));
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->DrawText(Point(0,0), rText);

    BitmapEx aText(pDevice->GetBitmapEx(Point(0,0), pDevice->GetOutputSize()));
//    TextCacheItem *pItem = new TextCacheItem(OpenGLHelper::ConvertBitmapExToRGBABuffer(aText), aText.GetSizePixel());
    Bitmap aBitmap (aText.GetBitmap());
    BitmapReadAccess *pAcc = aBitmap.AcquireReadAccess();
    sal_uInt8 *buf = reinterpret_cast<sal_uInt8 *>(pAcc->GetBuffer());
    long nBmpWidth = aText.GetSizePixel().Width();
    long nBmpHeight = aText.GetSizePixel().Height();
    sal_uInt8* pBitmapBuf(new sal_uInt8[3* nBmpWidth * nBmpHeight]);
    memcpy(pBitmapBuf, buf, 3* nBmpWidth * nBmpHeight);
    TextCacheItem *pItem = new TextCacheItem(pBitmapBuf, aText.GetSizePixel());
    maTextCache.insert(rText, pItem);
    Bitmap::ReleaseAccess(pAcc);
    return *maTextCache.find(rText)->second;
}

Text::Text(OpenGL3DRenderer* pRenderer, TextCache& rTextCache, const OUString& rStr, sal_uInt32 nId):
    Renderable3DObject(pRenderer, nId),
    maText(rTextCache.getText(rStr))
{
}

void Text::render()
{
    glm::vec3 dir2 = maTopRight - maTopLeft;
    glm::vec3 bottomLeft = maBottomRight - dir2;
    mpRenderer->CreateTextTexture(maText.maPixels, maText.maSize,
                                  maTopLeft, maTopRight, maBottomRight, bottomLeft,
                                  mnUniqueId);
}

void Text::setPosition(const glm::vec3& rTopLeft, const glm::vec3& rTopRight, const glm::vec3& rBottomRight)
{
    maTopLeft = rTopLeft;
    maTopRight = rTopRight;
    maBottomRight = rBottomRight;
}

ScreenText::ScreenText(OpenGL3DRenderer* pRenderer, TextCache& rTextCache,
        const OUString& rStr, glm::vec4 rColor, sal_uInt32 nId, bool bIs3dText):
    Renderable3DObject(pRenderer, nId),
    maText(rTextCache.getText(rStr,bIs3dText)),
    maColor(rColor)
{
}

void ScreenText::setPosition(const glm::vec2& rTopLeft, const glm::vec2& rBottomRight,
        const glm::vec3& r3DPos)
{
    maTopLeft = rTopLeft;
    maBottomRight = rBottomRight;
    ma3DPos = r3DPos;
}

void ScreenText::render()
{
    mpRenderer->CreateScreenTextTexture(maText.maPixels, maText.maSize,
                                        maTopLeft, maBottomRight, ma3DPos, maColor,
                                        mnUniqueId);
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
    mpRenderer->AddShapePolygon3DObject(maColor.GetColor(), false, 0, 1, 0xFFFFFF, mnUniqueId);
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
    mpRenderer->EndAddShapePolygon3DObject();
    //we should render the edge if the edge color is different from the fill color
    if (maColor.GetColor() != maLineColor.GetColor())
    {
        mpRenderer->AddShapePolygon3DObject(0, true, maLineColor.GetColor(), 0, 0xFFFFFF, mnUniqueId);
        mpRenderer->AddPolygon3DObjectPoint(maBottomRight.x, maBottomRight.y, maBottomRight.z);
        mpRenderer->AddPolygon3DObjectPoint(maTopRight.x, maTopRight.y, maTopRight.z);
        mpRenderer->AddPolygon3DObjectPoint(maTopLeft.x, maTopLeft.y, maTopLeft.z);
        mpRenderer->AddPolygon3DObjectPoint(bottomLeft.x, bottomLeft.y, bottomLeft.z);
        mpRenderer->EndAddPolygon3DObjectPoint();
        mpRenderer->EndAddShapePolygon3DObject();
    }
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
    maPos(10,-50,20),
    maUp(0, 0, 1),
    maDirection(glm::vec3(0,0,0))
{
}

void Camera::render()
{
    mpRenderer->SetCameraInfo(maPos, maDirection, maUp);
}

void Camera::setPosition(const glm::vec3& rPos)
{
    maPos = rPos;
}

void Camera::setDirection(const glm::vec3& rDir)
{
    maDirection = rDir;
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
