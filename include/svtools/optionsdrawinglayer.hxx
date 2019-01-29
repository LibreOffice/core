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

#ifndef INCLUDED_SVTOOLS_OPTIONSDRAWINGLAYER_HXX
#define INCLUDED_SVTOOLS_OPTIONSDRAWINGLAYER_HXX

#include <svtools/svtdllapi.h>
#include <sal/types.h>
#include <tools/color.hxx>
#include <memory>

namespace osl { class Mutex; }

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtOptionsDrawinglayer_Impl;

/*-************************************************************************************************************
    @short          collect information about startup features
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SVT_DLLPUBLIC SvtOptionsDrawinglayer
{
    public:

         SvtOptionsDrawinglayer();
        ~SvtOptionsDrawinglayer();

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

        bool        IsOverlayBuffer() const;
        bool        IsPaintBuffer() const;
        Color       GetStripeColorA() const;
        Color       GetStripeColorB() const;
        sal_uInt16  GetStripeLength() const;

        // #i73602#
        bool        IsOverlayBuffer_Calc() const;
        bool        IsOverlayBuffer_Writer() const;
        bool        IsOverlayBuffer_DrawImpress() const;

        // #i74769#, #i75172#
        bool        IsPaintBuffer_Calc() const;
        bool        IsPaintBuffer_Writer() const;
        bool        IsPaintBuffer_DrawImpress() const;

        // #i4219#
        sal_uInt32  GetMaximumPaperWidth() const;
        sal_uInt32  GetMaximumPaperHeight() const;
        sal_uInt32  GetMaximumPaperLeftMargin() const;
        sal_uInt32  GetMaximumPaperRightMargin() const;
        sal_uInt32  GetMaximumPaperTopMargin() const;
        sal_uInt32  GetMaximumPaperBottomMargin() const;

        // #i95644# helper to check if AA is allowed on this system. Currently, for WIN its disabled
        // and OutDevSupportType::TransparentRect is checked (this  hits XRenderExtension, e.g.
        // currently for SunRay as long as not supported there)
        bool       IsAAPossibleOnThisSystem() const;

        // primitives
        bool        IsAntiAliasing() const;
        bool        IsSnapHorVerLinesToDiscrete() const;
        bool        IsSolidDragCreate() const;
        bool        IsRenderDecoratedTextDirect() const;
        bool        IsRenderSimpleTextDirect() const;
        sal_uInt32  GetQuadratic3DRenderLimit() const;
        sal_uInt32  GetQuadraticFormControlRenderLimit() const;

        void        SetAntiAliasing( bool bState );

        // #i97672# selection settings
        bool        IsTransparentSelection() const;
        sal_uInt16  GetTransparentSelectionPercent() const;
        sal_uInt16  GetSelectionMaximumLuminancePercent() const;

        // get system highlight color, limited to the maximum allowed luminance
        // (defined in GetSelectionMaximumLuminancePercent() in SvtOptionsDrawinglayer,
        // combined with Application::GetSettings().GetStyleSettings().GetHighlightColor())
        Color getHilightColor() const;

    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class use his own static mutex to be threadsafe.
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/
        SVT_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

        std::shared_ptr<SvtOptionsDrawinglayer_Impl>     m_pImpl;
};

#endif  // #ifndef INCLUDED_SVTOOLS_OPTIONSDRAWINGLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
