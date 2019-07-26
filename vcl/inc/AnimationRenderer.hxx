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

#ifndef INCLUDED_VCL_INC_ANIMATERENDERER_HXX
#define INCLUDED_VCL_INC_ANIMATERENDERER_HXX

#include <vcl/animate/Animation.hxx>
#include <vcl/vclptr.hxx>

class Animation;
class OutputDevice;
class VirtualDevice;
struct AnimationFrame;

class VCL_DLLPUBLIC AnimationRenderer
{
public:
    AnimationRenderer(Animation* pParent, OutputDevice* pOut, const Point& rPt, const Size& rSz,
                      sal_uLong nCallerId, OutputDevice* pFirstFrameOutDev = nullptr);

    virtual ~AnimationRenderer();

    bool Matches(OutputDevice* pOut, long nCallerId) const;
    virtual void DrawToIndex(sal_uLong nIndex);
    void Draw(sal_uLong nIndex, VirtualDevice* pVDev = nullptr);
    void Repaint();
    AnimationData* CreateAnimationData() const;

    Point GetPosition(const AnimationFrame& rAnm);
    Size GetSize(const AnimationFrame& rAnm);

    const Point& GetOriginPosition() const { return maOriginPt; }
    const Size& GetSizePx() const { return maSizePx; }

    void Pause(bool bIsPaused) { mbIsPaused = bIsPaused; }
    bool IsPaused() const { return mbIsPaused; }

    void SetMarked(bool bMarked) { mbIsMarked = bMarked; }
    bool IsMarked() const { return mbIsMarked; }

protected:
    Animation* const mpParent;
    VclPtr<OutputDevice> mpRenderContext;
    vcl::Region maClip;
    Point maDispPt;
    Size maSizePx;
    Size maDispSz;

private:
    long const mnCallerId;
    Point const maOriginPt;
    Point maRestPt;
    Size const maLogicalSize;
    Size maRestSz;
    VclPtr<VirtualDevice> mpBackground;
    VclPtr<VirtualDevice> mpRestore;
    sal_uLong mnActIndex;
    Disposal meLastDisposal;
    bool mbIsPaused;
    bool mbIsMarked;
    bool const mbIsMirroredHorizontally;
    bool const mbIsMirroredVertically;

    SAL_DLLPRIVATE double CalculateXScaling();
    SAL_DLLPRIVATE double CalculateYScaling();
    SAL_DLLPRIVATE Point GetBottomRightPoint(const AnimationFrame& rAnimationFrame);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
