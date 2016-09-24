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

#ifndef INCLUDED_SVX_SWFRAMEEXAMPLE_HXX
#define INCLUDED_SVX_SWFRAMEEXAMPLE_HXX

#include <vcl/window.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <svx/svxdllapi.h>

// class SwFrmPagePreview -------------------------------------------------------

class SVX_DLLPUBLIC SvxSwFrameExample : public vcl::Window
{
    Color       m_aTransColor;      // transparency
    Color       m_aBgCol;           // background
    Color       m_aFrameColor;      // graphic frame
    Color       m_aAlignColor;      // align anchor
    Color       m_aBorderCol;       // frame of doc
    Color       m_aPrintAreaCol;    // frame of printable area of doc
    Color       m_aTxtCol;          // symbolised text
    Color       m_aBlankCol;        // area of symbol for blank
    Color       m_aBlankFrameCol;   // frame of symbol for blank

    Rectangle   aPage;
    Rectangle   aPagePrtArea;
    Rectangle   aTextLine;
    Rectangle   aPara;
    Rectangle   aParaPrtArea;
    Rectangle   aFrameAtFrame;
    Rectangle   aDrawObj;
    Rectangle   aAutoCharFrame;
    Size        aFrmSize;

    short       nHAlign;
    short       nHRel;

    short       nVAlign;
    short       nVRel;

    short       nWrap;
    short       nAnchor;
    bool        bTrans;

    Point       aRelPos;

    void InitColors_Impl();
    void InitAllRects_Impl(vcl::RenderContext& rRenderContext);
    void CalcBoundRect_Impl(Rectangle &rRect);
    Rectangle DrawInnerFrame_Impl(vcl::RenderContext& rRenderContext, const Rectangle &rRect, const Color &rFillColor, const Color &rBorderColor);

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle&) override;
    virtual Size GetOptimalSize() const override;
protected:
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
public:

    SvxSwFrameExample(vcl::Window* pParent, WinBits nStyle);

    inline void SetWrap(sal_uInt16 nW)          { nWrap     = nW; }

    inline void SetHAlign(short nH)          { nHAlign   = nH; }
    inline void SetHoriRel(short nR)         { nHRel     = nR; }

    inline void SetVAlign(short nV)          { nVAlign   = nV; }
    inline void SetVertRel(short nR)         { nVRel     = nR; }

    inline void SetTransparent(bool bT)      { bTrans    = bT; }
    inline void SetAnchor(short nA)          { nAnchor   = nA; }

    void SetRelPos(const Point& rP);
};


#endif // INCLUDED_SVX_SWFRAMEEXAMPLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
