/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EDITENG_STRIPPORTIONSHELPER_HXX
#define INCLUDED_EDITENG_STRIPPORTIONSHELPER_HXX

#include <editeng/editengdllapi.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <rtl/ustring.hxx>
#include <vcl/kernarray.hxx>
#include <vcl/GraphicObject.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/eedata.hxx>
#include <editeng/flditem.hxx>

namespace com::sun::star::lang
{
struct Locale;
}

class EDITENG_DLLPUBLIC DrawPortionInfo
{
public:
    const Point& mrStartPos;
    const OUString maText;
    sal_Int32 mnTextStart;
    sal_Int32 mnTextLen;
    KernArraySpan mpDXArray;
    std::span<const sal_Bool> mpKashidaArray;
    const SvxFont& mrFont;
    sal_Int32 mnPara;
    sal_uInt8 mnBiDiLevel;
    const EEngineData::WrongSpellVector* mpWrongSpellVector;
    const SvxFieldData* mpFieldData;
    bool mbEndOfLine : 1;
    bool mbEndOfParagraph : 1;
    bool mbEndOfBullet : 1;
    const com::sun::star::lang::Locale* mpLocale;
    const Color maOverlineColor;
    const Color maTextLineColor;

    bool IsRTL() const { return mnBiDiLevel % 2 == 1; }

    DrawPortionInfo(const Point& rPos, OUString aTxt, sal_Int32 nTxtStart, sal_Int32 nTxtLen,
                    KernArraySpan pDXArr, std::span<const sal_Bool> pKashidaArr,
                    const SvxFont& rFnt, sal_Int32 nPar, sal_uInt8 nBiDiLevel,
                    const EEngineData::WrongSpellVector* pWrongSpellVector,
                    const SvxFieldData* pFieldData, bool bEndOfLine, bool bEndOfParagraph,
                    bool bEndOfBullet, const com::sun::star::lang::Locale* pLocale,
                    const Color& rOverlineColor, const Color& rTextLineColor)
        : mrStartPos(rPos)
        , maText(std::move(aTxt))
        , mnTextStart(nTxtStart)
        , mnTextLen(nTxtLen)
        , mpDXArray(pDXArr)
        , mpKashidaArray(pKashidaArr)
        , mrFont(rFnt)
        , mnPara(nPar)
        , mnBiDiLevel(nBiDiLevel)
        , mpWrongSpellVector(pWrongSpellVector)
        , mpFieldData(pFieldData)
        , mbEndOfLine(bEndOfLine)
        , mbEndOfParagraph(bEndOfParagraph)
        , mbEndOfBullet(bEndOfBullet)
        , mpLocale(pLocale)
        , maOverlineColor(rOverlineColor)
        , maTextLineColor(rTextLineColor)
    {
    }
};

class EDITENG_DLLPUBLIC DrawBulletInfo
{
public:
    const GraphicObject maBulletGraphicObject;
    Point maBulletPosition;
    Size maBulletSize;

    DrawBulletInfo(const GraphicObject& rBulletGraphicObject, const Point& rBulletPosition,
                   const Size& rBulletSize)
        : maBulletGraphicObject(rBulletGraphicObject)
        , maBulletPosition(rBulletPosition)
        , maBulletSize(rBulletSize)
    {
    }
};

namespace drawinglayer::primitive2d
{
class Primitive2DContainer;
}
namespace basegfx
{
class B2DHomMatrix;
}

void EDITENG_DLLPUBLIC CreateTextPortionPrimitivesFromDrawPortionInfo(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DHomMatrix& rNewTransformA, const basegfx::B2DHomMatrix& rNewTransformB,
    const DrawPortionInfo& rInfo);
void EDITENG_DLLPUBLIC CreateDrawBulletPrimitivesFromDrawBulletInfo(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DHomMatrix& rNewTransformA, const basegfx::B2DHomMatrix& rNewTransformB,
    const DrawBulletInfo& rInfo);

#endif // INCLUDED_EDITENG_STRIPPORTIONSHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
