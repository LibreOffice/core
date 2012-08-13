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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
