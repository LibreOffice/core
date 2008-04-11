/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impanmvw.hxx,v $
 * $Revision: 1.4 $
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
    ULONG           mnActPos;
    Disposal        meLastDisposal;
    BOOL            mbPause;
    BOOL            mbFirst;
    BOOL            mbMarked;
    BOOL            mbHMirr;
    BOOL            mbVMirr;

    void            ImplGetPosSize( const AnimationBitmap& rAnm, Point& rPosPix, Size& rSizePix );
    void            ImplDraw( ULONG nPos, VirtualDevice* pVDev );

public:

                    ImplAnimView( Animation* pParent, OutputDevice* pOut,
                                  const Point& rPt, const Size& rSz, ULONG nExtraData,
                                  OutputDevice* pFirstFrameOutDev = NULL );
                    ~ImplAnimView();

    BOOL            ImplMatches( OutputDevice* pOut, long nExtraData ) const;
    void            ImplDrawToPos( ULONG nPos );
    void            ImplDraw( ULONG nPos );
    void            ImplRepaint();
    AInfo*          ImplCreateAInfo() const;

    const Point&    ImplGetOutPos() const { return maPt; }

    const Size&     ImplGetOutSize() const { return maSz; }
    const Size&     ImplGetOutSizePix() const { return maSzPix; }

    void            ImplPause( BOOL bPause ) { mbPause = bPause; }
    BOOL            ImplIsPause() const { return mbPause; }

    void            ImplSetMarked( BOOL bMarked ) { mbMarked = bMarked; }
    BOOL            ImplIsMarked() const { return mbMarked; }
};

#endif
