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

                        GetStripeColorA()
                        SetStripeColorA()       =>  Set first of two colors which overlay uses to draw stripes

                        GetStripeColorB()
                        SetStripeColorB()       =>  Set second of two colors which overlay uses to draw stripes

                        GetStripeLength()
                        SetStripeLength()       =>  Set length of a single stripe in pixels

            @seealso    configuration package "org.openoffice.Office.Common/Drawinglayer"
        *//*-*****************************************************************************************************/

        Color       GetStripeColorA() const;
        Color       GetStripeColorB() const;
        sal_uInt16  GetStripeLength() const;

        /**
          Specifies the maximum allowed Paper Width for page definitions
          in cm. Default is 6m, i.e. 600 cm. When this is changed to higher
          values, it is done on own risk.
        */
        static sal_uInt32  GetMaximumPaperWidth() { return 600; }
        /**
          Specifies the maximum allowed Paper Height for page definitions
          in cm. Default is 6m, i.e. 600 cm. When this is changed to higher
          values, it is done on own risk.
         */
        static sal_uInt32  GetMaximumPaperHeight() { return 600; }
        /**
          Specifies the maximum allowed Left Margin for the page
          definitions in 1/100th cm.
         */
        static sal_uInt32  GetMaximumPaperLeftMargin() { return 9999; }
        /**
          Specifies the maximum allowed Right Margin for the page
          definitions in 1/100th cm.
         */
        static sal_uInt32  GetMaximumPaperRightMargin() { return 9999; }
        /**
          Specifies the maximum allowed Top Margin for the page
          definitions in 1/100th cm.
         */
        static sal_uInt32  GetMaximumPaperTopMargin() { return 9999; }
        /*
          Specifies the maximum allowed Bottom Margin for the page
          definitions in 1/100th cm.
        */
        static sal_uInt32  GetMaximumPaperBottomMargin() { return 9999; }

        // #i95644# helper to check if AA is allowed on this system. Currently, for WIN it's disabled
        // and OutDevSupportType::TransparentRect is checked (this  hits XRenderExtension, e.g.
        // currently for SunRay as long as not supported there)
        bool       IsAAPossibleOnThisSystem() const;

        // primitives
        bool        IsAntiAliasing() const;
    	/**
          Defines a Limitation for the default raster conversion from
          3D Scenes to Bitmaps. The number is the maximum number of pixels to
          use, e.g. 1000x1000 Pixels is allowed as default. When Scenes would
          need more Pixels than this, the Bitmap will be limited and scaled to
          the needed pixel size at paint time.
	     */
        static sal_uInt32  GetQuadratic3DRenderLimit() { return 1000000; }
        /**
          Defines a Limitation for the default raster conversion of
          FormControls in edit mode. These have the ability to be displayed
          using this fallback to Bitmaps. The number is the maximum number of
          pixels to use, e.g. 300x150 Pixels is allowed as default. When
          FormControls would need more Pixels than this, the Bitmap will be
          limited and scaled to the needed pixel size at paint time.
         */
        static sal_uInt32  GetQuadraticFormControlRenderLimit() { return 45000; }

        void        SetAntiAliasing( bool bState );

        /**
          Specifies the degree of transparence to be used when transparent
          selection is used. The value is a percent value. Since neither no
          transparence nor complete transparence makes sense, the value is
          limited to a range of 10% - 90%. If the given value is outside this
          range, it is cropped to it.
         */
        static sal_uInt16 GetTransparentSelectionPercent() { return 75; }

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
