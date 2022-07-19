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

#pragma once

#include <vcl/dllapi.h>
#include <vcl/animate/Animation.hxx>
#include <vcl/vclptr.hxx>

class Animation;
class OutputDevice;
class VirtualDevice;
struct AnimationFrame;

struct AnimationData
{
    Point maOriginStartPt;
    Size maStartSize;
    VclPtr<OutputDevice> mpRenderContext;
    void* mpRendererData;
    tools::Long mnRendererId;
    bool mbIsPaused;

    AnimationData();
};


class VCL_DLLPUBLIC AnimationRenderer
{
private:

    friend class Animation;

    Animation*      mpParent;
    VclPtr<OutputDevice>  mpRenderContext;
    tools::Long     mnRendererId;
    Point           maPt;
    Point           maDispPt;
    Point           maRestPt;
    Size            maLogicalSize;
    Size            maSizePx;
    Size            maDispSz;
    Size            maRestSz;
    vcl::Region     maClip;
    VclPtr<VirtualDevice>  mpBackground;
    VclPtr<VirtualDevice>  mpRestore;
    sal_uLong       mnActIndex;
    Disposal        meLastDisposal;
    bool            mbIsPaused;
    bool            mbIsMarked;
    bool            mbIsMirroredHorizontally;
    bool            mbIsMirroredVertically;

public:
                    AnimationRenderer( Animation* pParent, OutputDevice* pOut,
                                  const Point& rPt, const Size& rSz, sal_uLong nRendererId,
                                  OutputDevice* pFirstFrameOutDev = nullptr );
                    AnimationRenderer(AnimationRenderer&&) = delete;
                    ~AnimationRenderer();

    bool            Matches(const OutputDevice* pOut, tools::Long nRendererId) const;
    void            DrawToIndex( sal_uLong nIndex );
    void            Draw( sal_uLong nIndex, VirtualDevice* pVDev=nullptr );
    void            Repaint();
    AnimationData*  CreateAnimationData() const;

    void            GetPosSize( const AnimationFrame& rAnm, Point& rPosPix, Size& rSizePix );

    const Point&    GetOriginPosition() const { return maPt; }

    const Size&     GetOutSizePix() const { return maSizePx; }

    void            Pause( bool bIsPaused ) { mbIsPaused = bIsPaused; }
    bool            IsPaused() const { return mbIsPaused; }

    void            SetMarked( bool bIsMarked ) { mbIsMarked = bIsMarked; }
    bool            IsMarked() const { return mbIsMarked; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
