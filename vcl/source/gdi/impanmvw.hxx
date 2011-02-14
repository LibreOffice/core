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

#ifndef _SV_IMPANMVW_HXX
#define _SV_IMPANMVW_HXX

#include <vcl/animate.hxx>

// ----------------
// - ImplAnimView -
// ----------------

class Animation;
class OutputDevice;
class VirtualDevice;
struct AnimationBitmap;

class ImplAnimView
{
private:

    Animation*      mpParent;
    OutputDevice*   mpOut;
    long            mnExtraData;
    Point           maPt;
    Point           maDispPt;
    Point           maRestPt;
    Size            maSz;
    Size            maSzPix;
    Size            maDispSz;
    Size            maRestSz;
    MapMode         maMap;
    Region          maClip;
    VirtualDevice*  mpBackground;
    VirtualDevice*  mpRestore;
    sal_uLong           mnActPos;
    Disposal        meLastDisposal;
    sal_Bool            mbPause;
    sal_Bool            mbFirst;
    sal_Bool            mbMarked;
    sal_Bool            mbHMirr;
    sal_Bool            mbVMirr;

    void            ImplGetPosSize( const AnimationBitmap& rAnm, Point& rPosPix, Size& rSizePix );
    void            ImplDraw( sal_uLong nPos, VirtualDevice* pVDev );

public:

                    ImplAnimView( Animation* pParent, OutputDevice* pOut,
                                  const Point& rPt, const Size& rSz, sal_uLong nExtraData,
                                  OutputDevice* pFirstFrameOutDev = NULL );
                    ~ImplAnimView();

    sal_Bool            ImplMatches( OutputDevice* pOut, long nExtraData ) const;
    void            ImplDrawToPos( sal_uLong nPos );
    void            ImplDraw( sal_uLong nPos );
    void            ImplRepaint();
    AInfo*          ImplCreateAInfo() const;

    const Point&    ImplGetOutPos() const { return maPt; }

    const Size&     ImplGetOutSize() const { return maSz; }
    const Size&     ImplGetOutSizePix() const { return maSzPix; }

    void            ImplPause( sal_Bool bPause ) { mbPause = bPause; }
    sal_Bool            ImplIsPause() const { return mbPause; }

    void            ImplSetMarked( sal_Bool bMarked ) { mbMarked = bMarked; }
    sal_Bool            ImplIsMarked() const { return mbMarked; }
};

#endif
