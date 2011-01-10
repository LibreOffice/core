/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVXSWFRAMEEXAMPLE_HXX
#define _SVXSWFRAMEEXAMPLE_HXX

#include <vcl/window.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"

// class SwFrmPagePreview -------------------------------------------------------

class SVX_DLLPUBLIC SvxSwFrameExample : public Window
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
    sal_Bool        bTrans;

    Point       aRelPos;

    void InitColors_Impl( void );
    void InitAllRects_Impl();
    void CalcBoundRect_Impl(Rectangle &rRect);
    Rectangle DrawInnerFrame_Impl(const Rectangle &rRect, const Color &rFillColor, const Color &rBorderColor);

    void DrawRect_Impl(const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor);
    virtual void Paint(const Rectangle&);
protected:
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
public:

    SvxSwFrameExample(Window* pParent, const ResId& rResID);
    ~SvxSwFrameExample();

    inline void SetWrap(sal_uInt16 nW)          { nWrap     = nW; }

    inline void SetHAlign(short nH)      { nHAlign   = nH; }
    inline void SetHoriRel(short nR)         { nHRel     = nR; }

    inline void SetVAlign(short nV)      { nVAlign   = nV; }
    inline void SetVertRel(short nR)         { nVRel     = nR; }

    inline void SetTransparent(sal_Bool bT)     { bTrans    = bT; }
    inline void SetAnchor(short nA)          { nAnchor   = nA; }

    void SetRelPos(const Point& rP);
};


#endif // _SVXSWFRAMEEXAMPLE_HXX
