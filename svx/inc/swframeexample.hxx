/*************************************************************************
 *
 *  $RCSfile: swframeexample.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 14:02:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVXSWFRAMEEXAMPLE_HXX
#define _SVXSWFRAMEEXAMPLE_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

// class SwFrmPagePreview -------------------------------------------------------

class SvxSwFrameExample : public Window
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
    BOOL        bTrans;

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

    inline void SetWrap(USHORT nW)          { nWrap     = nW; }

    inline void SetHAlign(short nH)      { nHAlign   = nH; }
    inline void SetHoriRel(short nR)         { nHRel     = nR; }

    inline void SetVAlign(short nV)      { nVAlign   = nV; }
    inline void SetVertRel(short nR)         { nVRel     = nR; }

    inline void SetTransparent(BOOL bT)     { bTrans    = bT; }
    inline void SetAnchor(short nA)          { nAnchor   = nA; }

    void SetRelPos(const Point& rP);
};


#endif // _SVXSWFRAMEEXAMPLE_HXX
