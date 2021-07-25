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

#include <svtools/svtdllapi.h>
#include <sal/types.h>
#include <tools/color.hxx>

namespace SvtOptionsDrawinglayer
{

/*-****************************************************************************************************
    @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Drawinglayer/..."
    @descr      These options describe internal states to enable/disable features of installed office.

                IsOverlayBuffer()
                SetOverlayBuffer()      =>  Activate this field for letting Overlay use a buffer

                IsPaintBuffer()
                SetPaintBuffer()        =>  Activate this field for letting Paint use a prerender buffer

                GetStripeColorA()
                SetStripeColorA()       =>  Set first of two colors which overlay uses to draw stripes

                GetStripeColorB()
                SetStripeColorB()       =>  Set second of two colors which overlay uses to draw stripes

                GetStripeLength()
                SetStripeLength()       =>  Set length of a single stripe in pixels

    @seealso    configuration package "org.openoffice.Office.Common/Drawinglayer"
*//*-*****************************************************************************************************/

SVT_DLLPUBLIC bool        IsOverlayBuffer();
SVT_DLLPUBLIC bool        IsPaintBuffer();
SVT_DLLPUBLIC Color       GetStripeColorA();
SVT_DLLPUBLIC Color       GetStripeColorB();
SVT_DLLPUBLIC sal_uInt16  GetStripeLength();

// #i73602#
SVT_DLLPUBLIC bool        IsOverlayBuffer_Calc();
SVT_DLLPUBLIC bool        IsOverlayBuffer_Writer();
SVT_DLLPUBLIC bool        IsOverlayBuffer_DrawImpress();

// #i74769#, #i75172#
SVT_DLLPUBLIC bool        IsPaintBuffer_Calc();
SVT_DLLPUBLIC bool        IsPaintBuffer_Writer();
SVT_DLLPUBLIC bool        IsPaintBuffer_DrawImpress();

// #i4219#
SVT_DLLPUBLIC sal_uInt32  GetMaximumPaperWidth();
SVT_DLLPUBLIC sal_uInt32  GetMaximumPaperHeight();
SVT_DLLPUBLIC sal_uInt32  GetMaximumPaperLeftMargin();
SVT_DLLPUBLIC sal_uInt32  GetMaximumPaperRightMargin();
SVT_DLLPUBLIC sal_uInt32  GetMaximumPaperTopMargin();
SVT_DLLPUBLIC sal_uInt32  GetMaximumPaperBottomMargin();

// #i95644# helper to check if AA is allowed on this system. Currently, for WIN it's disabled
// and OutDevSupportType::TransparentRect is checked (this  hits XRenderExtension, e.g.
// currently for SunRay as long as not supported there)
SVT_DLLPUBLIC bool       IsAAPossibleOnThisSystem();

// primitives
SVT_DLLPUBLIC bool        IsAntiAliasing();
SVT_DLLPUBLIC bool        IsSnapHorVerLinesToDiscrete();
SVT_DLLPUBLIC bool        IsSolidDragCreate();
SVT_DLLPUBLIC bool        IsRenderDecoratedTextDirect();
SVT_DLLPUBLIC bool        IsRenderSimpleTextDirect();
SVT_DLLPUBLIC sal_uInt32  GetQuadratic3DRenderLimit();
SVT_DLLPUBLIC sal_uInt32  GetQuadraticFormControlRenderLimit();

SVT_DLLPUBLIC void        SetAntiAliasing( bool bOn, bool bTemporary );

// #i97672# selection settings
SVT_DLLPUBLIC bool        IsTransparentSelection();
SVT_DLLPUBLIC sal_uInt16  GetTransparentSelectionPercent();
SVT_DLLPUBLIC sal_uInt16  GetSelectionMaximumLuminancePercent();

// get system highlight color, limited to the maximum allowed luminance
// (defined in GetSelectionMaximumLuminancePercent() in SvtOptionsDrawinglayer,
// combined with Application::GetSettings().GetStyleSettings().GetHighlightColor())
SVT_DLLPUBLIC Color getHilightColor();

} // namespace SvtOptionsDrawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
