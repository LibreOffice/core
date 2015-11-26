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

#ifndef INCLUDED_VCL_SOURCE_GDI_IMPANMVW_HXX
#define INCLUDED_VCL_SOURCE_GDI_IMPANMVW_HXX

#include <vcl/animate.hxx>

class Animation;
class OutputDevice;
class VirtualDevice;
struct AnimationBitmap;

class ImplAnimView
{
private:

    friend class Animation;

    Animation*      mpParent;
    VclPtr<OutputDevice>   mpOut;
    long            mnExtraData;
    Point           maPt;
    Point           maDispPt;
    Point           maRestPt;
    Size            maSz;
    Size            maSzPix;
    Size            maDispSz;
    Size            maRestSz;
    vcl::Region     maClip;
    VclPtr<VirtualDevice>  mpBackground;
    VclPtr<VirtualDevice>  mpRestore;
    sal_uLong       mnActPos;
    Disposal        meLastDisposal;
    bool            mbPause;
    bool            mbMarked;
    bool            mbHMirr;
    bool            mbVMirr;

private:
                    ImplAnimView( Animation* pParent, OutputDevice* pOut,
                                  const Point& rPt, const Size& rSz, sal_uLong nExtraData,
                                  OutputDevice* pFirstFrameOutDev = nullptr );
                    ~ImplAnimView();

    bool            matches( OutputDevice* pOut, long nExtraData ) const;
    void            drawToPos( sal_uLong nPos );
    void            draw( sal_uLong nPos, VirtualDevice* pVDev=nullptr );
    void            repaint();
    AInfo*          createAInfo() const;

    void            getPosSize( const AnimationBitmap& rAnm, Point& rPosPix, Size& rSizePix );

    const Point&    getOutPos() const { return maPt; }

    const Size&     getOutSizePix() const { return maSzPix; }

    void            pause( bool bPause ) { mbPause = bPause; }
    bool            isPause() const { return mbPause; }

    void            setMarked( bool bMarked ) { mbMarked = bMarked; }
    bool            isMarked() const { return mbMarked; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
