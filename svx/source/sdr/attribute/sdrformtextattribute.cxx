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

#include <svx/sdr/attribute/sdrformtextattribute.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <svl/itemset.hxx>
#include <svx/xftdiit.hxx>
#include <svx/xftstit.hxx>
#include <svx/xftshxy.hxx>
#include <svx/xftshtit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xftadit.hxx>
#include <svx/xftshit.hxx>
#include <svx/xftshcit.hxx>
#include <svx/xftmrit.hxx>
#include <svx/xftouit.hxx>
#include <svx/sdshtitm.hxx>
#include <svx/xlntrit.hxx>
#include <svx/sdshcitm.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xdash.hxx>
#include <svx/xlndsit.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <sdr/attribute/sdrformtextoutlineattribute.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <rtl/instance.hxx>


// helper to get line, stroke and transparence attributes from SfxItemSet

namespace
{
    basegfx::B2DLineJoin impGetB2DLineJoin(css::drawing::LineJoint eLineJoint)
    {
        switch(eLineJoint)
        {
            case css::drawing::LineJoint_MIDDLE :
            {
                return basegfx::B2DLineJoin::Middle;
            }
            case css::drawing::LineJoint_BEVEL :
            {
                return basegfx::B2DLineJoin::Bevel;
            }
            case css::drawing::LineJoint_MITER :
            {
                return basegfx::B2DLineJoin::Miter;
            }
            case css::drawing::LineJoint_ROUND :
            {
                return basegfx::B2DLineJoin::Round;
            }
            default : // css::drawing::LineJoint_NONE
            {
                return basegfx::B2DLineJoin::NONE; // XLINEJOINT_NONE
            }
        }
    }

    sal_uInt8 impGetStrokeTransparence(bool bShadow, const SfxItemSet& rSet)
    {
        sal_uInt8 nRetval;

        if(bShadow)
        {
            nRetval = (sal_uInt8)(((static_cast<const SdrPercentItem&>(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue() * 255) / 100);
        }
        else
        {
            nRetval = (sal_uInt8)(((static_cast<const XLineTransparenceItem&>(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue() * 255) / 100);
        }

        return nRetval;
    }

    drawinglayer::attribute::LineAttribute impGetLineAttribute(bool bShadow, const SfxItemSet& rSet)
    {
        basegfx::BColor aColorAttribute;

        if(bShadow)
        {
            const Color aShadowColor((static_cast<const XColorItem&>(rSet.Get(SDRATTR_SHADOWCOLOR))).GetColorValue());
            aColorAttribute = aShadowColor.getBColor();
        }
        else
        {
            const Color aLineColor((static_cast<const XLineColorItem&>(rSet.Get(XATTR_LINECOLOR))).GetColorValue());
            aColorAttribute = aLineColor.getBColor();
        }

        const sal_uInt32 nLineWidth = (static_cast<const XLineWidthItem&>(rSet.Get(XATTR_LINEWIDTH))).GetValue();
        const css::drawing::LineJoint eLineJoint = (static_cast<const XLineJointItem&>(rSet.Get(XATTR_LINEJOINT))).GetValue();
        const css::drawing::LineCap eLineCap = (static_cast<const XLineCapItem&>(rSet.Get(XATTR_LINECAP))).GetValue();

        return drawinglayer::attribute::LineAttribute(
            aColorAttribute,
            (double)nLineWidth,
            impGetB2DLineJoin(eLineJoint),
            eLineCap);
    }

    drawinglayer::attribute::StrokeAttribute impGetStrokeAttribute(const SfxItemSet& rSet)
    {
        const css::drawing::LineStyle eLineStyle = (static_cast<const XLineStyleItem&>(rSet.Get(XATTR_LINESTYLE))).GetValue();
        double fFullDotDashLen(0.0);
        ::std::vector< double > aDotDashArray;

        if(css::drawing::LineStyle_DASH == eLineStyle)
        {
            const XDash& rDash = (static_cast<const XLineDashItem&>(rSet.Get(XATTR_LINEDASH))).GetDashValue();

            if(rDash.GetDots() || rDash.GetDashes())
            {
                const sal_uInt32 nLineWidth = (static_cast<const XLineWidthItem&>(rSet.Get(XATTR_LINEWIDTH))).GetValue();
                fFullDotDashLen = rDash.CreateDotDashArray(aDotDashArray, (double)nLineWidth);
            }
        }

        return drawinglayer::attribute::StrokeAttribute(aDotDashArray, fFullDotDashLen);
    }
} // end of anonymous namespace



namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFormTextAttribute
        {
        public:
            // FormText (FontWork) Attributes
            sal_Int32                               mnFormTextDistance;     // distance from line in upright direction
            sal_Int32                               mnFormTextStart;        // shift from polygon start
            sal_Int32                               mnFormTextShdwXVal;     // shadow distance or 10th degrees
            sal_Int32                               mnFormTextShdwYVal;     // shadow distance or scaling
            sal_uInt16                              mnFormTextShdwTransp;   // shadow transparence
            XFormTextStyle                          meFormTextStyle;        // on/off and char orientation
            XFormTextAdjust                         meFormTextAdjust;       // adjustment (left/right/center) and scale
            XFormTextShadow                         meFormTextShadow;       // shadow mode
            Color                                   maFormTextShdwColor;    // shadow color

            // outline attributes; used when getFormTextOutline() is true and (for
            // shadow) when getFormTextShadow() != XFTSHADOW_NONE
            SdrFormTextOutlineAttribute             maOutline;
            SdrFormTextOutlineAttribute             maShadowOutline;

            // bitfield
            bool                                    mbFormTextMirror : 1;   // change orientation
            bool                                    mbFormTextOutline : 1;  // show contour of objects

            explicit ImpSdrFormTextAttribute(const SfxItemSet& rSet)
            :   mnFormTextDistance(static_cast<const XFormTextDistanceItem&>(rSet.Get(XATTR_FORMTXTDISTANCE)).GetValue()),
                mnFormTextStart(static_cast<const XFormTextStartItem&>(rSet.Get(XATTR_FORMTXTSTART)).GetValue()),
                mnFormTextShdwXVal(static_cast<const XFormTextShadowXValItem&>(rSet.Get(XATTR_FORMTXTSHDWXVAL)).GetValue()),
                mnFormTextShdwYVal(static_cast<const XFormTextShadowYValItem&>(rSet.Get(XATTR_FORMTXTSHDWYVAL)).GetValue()),
                mnFormTextShdwTransp(static_cast<const XFormTextShadowTranspItem&>(rSet.Get(XATTR_FORMTXTSHDWTRANSP)).GetValue()),
                meFormTextStyle(static_cast<const XFormTextStyleItem&>(rSet.Get(XATTR_FORMTXTSTYLE)).GetValue()),
                meFormTextAdjust(static_cast<const XFormTextAdjustItem&>(rSet.Get(XATTR_FORMTXTADJUST)).GetValue()),
                meFormTextShadow(static_cast<const XFormTextShadowItem&>(rSet.Get(XATTR_FORMTXTSHADOW)).GetValue()),
                maFormTextShdwColor(static_cast<const XFormTextShadowColorItem&>(rSet.Get(XATTR_FORMTXTSHDWCOLOR)).GetColorValue()),
                maOutline(),
                maShadowOutline(),
                mbFormTextMirror(static_cast<const XFormTextMirrorItem&>(rSet.Get(XATTR_FORMTXTMIRROR)).GetValue()),
                mbFormTextOutline(static_cast<const XFormTextOutlineItem&>(rSet.Get(XATTR_FORMTXTOUTLINE)).GetValue())
            {
                if(getFormTextOutline())
                {
                    const StrokeAttribute aStrokeAttribute(impGetStrokeAttribute(rSet));

                    // also need to prepare attributes for outlines
                    {
                        const LineAttribute aLineAttribute(impGetLineAttribute(false, rSet));
                        const sal_uInt8 nTransparence(impGetStrokeTransparence(false, rSet));

                        maOutline = SdrFormTextOutlineAttribute(
                            aLineAttribute, aStrokeAttribute, nTransparence);
                    }

                    if(XFTSHADOW_NONE != getFormTextShadow())
                    {
                        // also need to prepare attributes for shadow outlines
                        const LineAttribute aLineAttribute(impGetLineAttribute(true, rSet));
                        const sal_uInt8 nTransparence(impGetStrokeTransparence(true, rSet));

                        maShadowOutline = SdrFormTextOutlineAttribute(
                            aLineAttribute, aStrokeAttribute, nTransparence);
                    }
                }
            }

            ImpSdrFormTextAttribute()
            :   mnFormTextDistance(0),
                mnFormTextStart(0),
                mnFormTextShdwXVal(0),
                mnFormTextShdwYVal(0),
                mnFormTextShdwTransp(0),
                meFormTextStyle(XFT_NONE),
                meFormTextAdjust(XFT_CENTER),
                meFormTextShadow(XFTSHADOW_NONE),
                maFormTextShdwColor(),
                maOutline(),
                maShadowOutline(),
                mbFormTextMirror(false),
                mbFormTextOutline(false)
            {
            }

            // data read access
            sal_Int32 getFormTextDistance() const { return mnFormTextDistance; }
            sal_Int32 getFormTextStart() const { return mnFormTextStart; }
            sal_Int32 getFormTextShdwXVal() const { return mnFormTextShdwXVal; }
            sal_Int32 getFormTextShdwYVal() const { return mnFormTextShdwYVal; }
            sal_uInt16 getFormTextShdwTransp() const { return mnFormTextShdwTransp; }
            XFormTextStyle getFormTextStyle() const { return meFormTextStyle; }
            XFormTextAdjust getFormTextAdjust() const { return meFormTextAdjust; }
            XFormTextShadow getFormTextShadow() const { return meFormTextShadow; }
            Color getFormTextShdwColor() const { return maFormTextShdwColor; }
            const SdrFormTextOutlineAttribute& getOutline() const { return maOutline; }
            const SdrFormTextOutlineAttribute& getShadowOutline() const { return maShadowOutline; }
            bool getFormTextMirror() const { return mbFormTextMirror; }
            bool getFormTextOutline() const { return mbFormTextOutline; }

            // compare operator
            bool operator==(const ImpSdrFormTextAttribute& rCandidate) const
            {
                return (getFormTextDistance() == rCandidate.getFormTextDistance()
                    && getFormTextStart() == rCandidate.getFormTextStart()
                    && getFormTextShdwXVal() == rCandidate.getFormTextShdwXVal()
                    && getFormTextShdwYVal() == rCandidate.getFormTextShdwYVal()
                    && getFormTextShdwTransp() == rCandidate.getFormTextShdwTransp()
                    && getFormTextStyle() == rCandidate.getFormTextStyle()
                    && getFormTextAdjust() == rCandidate.getFormTextAdjust()
                    && getFormTextShadow() == rCandidate.getFormTextShadow()
                    && getFormTextShdwColor() == rCandidate.getFormTextShdwColor()
                    && getOutline() == rCandidate.getOutline()
                    && getShadowOutline() == rCandidate.getShadowOutline()
                    && getFormTextMirror() == rCandidate.getFormTextMirror()
                    && getFormTextOutline() == rCandidate.getFormTextOutline());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< SdrFormTextAttribute::ImplType, theGlobalDefault > {};
        }

        SdrFormTextAttribute::SdrFormTextAttribute(const SfxItemSet& rSet)
        :   mpSdrFormTextAttribute(ImpSdrFormTextAttribute(rSet))
        {
        }

        SdrFormTextAttribute::SdrFormTextAttribute()
        :   mpSdrFormTextAttribute(theGlobalDefault::get())
        {
        }

        SdrFormTextAttribute::SdrFormTextAttribute(const SdrFormTextAttribute& rCandidate)
        :   mpSdrFormTextAttribute(rCandidate.mpSdrFormTextAttribute)
        {
        }

        SdrFormTextAttribute::~SdrFormTextAttribute()
        {
        }

        bool SdrFormTextAttribute::isDefault() const
        {
            return mpSdrFormTextAttribute.same_object(theGlobalDefault::get());
        }

        SdrFormTextAttribute& SdrFormTextAttribute::operator=(const SdrFormTextAttribute& rCandidate)
        {
            mpSdrFormTextAttribute = rCandidate.mpSdrFormTextAttribute;
            return *this;
        }

        bool SdrFormTextAttribute::operator==(const SdrFormTextAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpSdrFormTextAttribute == mpSdrFormTextAttribute;
        }

        sal_Int32 SdrFormTextAttribute::getFormTextDistance() const
        {
            return mpSdrFormTextAttribute->getFormTextDistance();
        }

        sal_Int32 SdrFormTextAttribute::getFormTextStart() const
        {
            return mpSdrFormTextAttribute->getFormTextStart();
        }

        sal_Int32 SdrFormTextAttribute::getFormTextShdwXVal() const
        {
            return mpSdrFormTextAttribute->getFormTextShdwXVal();
        }

        sal_Int32 SdrFormTextAttribute::getFormTextShdwYVal() const
        {
            return mpSdrFormTextAttribute->getFormTextShdwYVal();
        }

        XFormTextStyle SdrFormTextAttribute::getFormTextStyle() const
        {
            return mpSdrFormTextAttribute->getFormTextStyle();
        }

        XFormTextAdjust SdrFormTextAttribute::getFormTextAdjust() const
        {
            return mpSdrFormTextAttribute->getFormTextAdjust();
        }

        XFormTextShadow SdrFormTextAttribute::getFormTextShadow() const
        {
            return mpSdrFormTextAttribute->getFormTextShadow();
        }

        Color SdrFormTextAttribute::getFormTextShdwColor() const
        {
            return mpSdrFormTextAttribute->getFormTextShdwColor();
        }

        const SdrFormTextOutlineAttribute& SdrFormTextAttribute::getOutline() const
        {
            return mpSdrFormTextAttribute->getOutline();
        }

        const SdrFormTextOutlineAttribute& SdrFormTextAttribute::getShadowOutline() const
        {
            return mpSdrFormTextAttribute->getShadowOutline();
        }

        bool SdrFormTextAttribute::getFormTextMirror() const
        {
            return mpSdrFormTextAttribute->getFormTextMirror();
        }

        bool SdrFormTextAttribute::getFormTextOutline() const
        {
            return mpSdrFormTextAttribute->getFormTextOutline();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
