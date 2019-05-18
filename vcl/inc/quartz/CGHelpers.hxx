/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_QUARTZ_CGHELPER_HXX
#define INCLUDED_VCL_INC_QUARTZ_CGHELPER_HXX

#include <premac.h>
#include <CoreGraphics/CoreGraphics.h>
#include <postmac.h>

#include <quartz/utils.h>

class CGLayerHolder
{
private:
    CGLayerRef mpLayer;

public:
    CGLayerHolder()
        : mpLayer(nullptr)
    {
    }

    CGLayerHolder(CGLayerRef pLayer)
        : mpLayer(pLayer)
    {
    }

    CGLayerRef get() const { return mpLayer; }

    bool isSet() const { return mpLayer != nullptr; }

    void set(CGLayerRef const& pLayer) { mpLayer = pLayer; }
};

class CGContextHolder
{
private:
    CGContextRef mpContext;
#if OSL_DEBUG_LEVEL > 0
    int mnContextStackDepth;
#endif

public:
    CGContextHolder()
        : mpContext(nullptr)
#if OSL_DEBUG_LEVEL > 0
        , mnContextStackDepth(0)
#endif
    {
    }

    CGContextHolder(CGContextRef pContext)
        : mpContext(pContext)
#if OSL_DEBUG_LEVEL > 0
        , mnContextStackDepth(0)
#endif
    {
    }

    CGContextRef get() const { return mpContext; }

    bool isSet() const { return mpContext != nullptr; }

    void set(CGContextRef const& pContext) { mpContext = pContext; }

    void saveState()
    {
        SAL_INFO("vcl.cg", "CGContextSaveGState(" << mpContext << ") " << ++mnContextStackDepth);
        CGContextSaveGState(mpContext);
    }

    void restoreState()
    {
        SAL_INFO("vcl.cg", "CGContextRestoreGState(" << mpContext << ") " << mnContextStackDepth--);
        CGContextRestoreGState(mpContext);
    }
};

#endif // INCLUDED_VCL_INC_QUARTZ_CGHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
