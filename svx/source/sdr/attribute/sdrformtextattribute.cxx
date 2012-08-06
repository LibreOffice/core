/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <svx/xlineit0.hxx>
#include <svx/xdash.hxx>
#include <svx/xlndsit.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <svx/sdr/attribute/sdrformtextoutlineattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// helper to get line, stroke and transparence attributes from SfxItemSet

namespace
{
    basegfx::B2DLineJoin impGetB2DLineJoin(XLineJoint eLineJoint)
    {
        switch(eLineJoint)
        {
            case XLINEJOINT_MIDDLE  :
            {
                return basegfx::B2DLINEJOIN_MIDDLE;
            }
            case XLINEJOINT_BEVEL   :
            {
                return basegfx::B2DLINEJOIN_BEVEL;
            }
            case XLINEJOINT_MITER   :
            {
                return basegfx::B2DLINEJOIN_MITER;
            }
            case XLINEJOINT_ROUND   :
            {
                return basegfx::B2DLINEJOIN_ROUND;
            }
            default :
            {
                return basegfx::B2DLINEJOIN_NONE; // XLINEJOINT_NONE
            }
        }
    }

    sal_uInt8 impGetStrokeTransparence(bool bShadow, const SfxItemSet& rSet)
    {
        sal_uInt8 nRetval;

        if(bShadow)
        {
            nRetval = (sal_uInt8)((((SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue() * 255) / 100);
        }
        else
        {
            nRetval = (sal_uInt8)((((XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue() * 255) / 100);
        }

        return nRetval;
    }

    drawinglayer::attribute::LineAttribute impGetLineAttribute(bool bShadow, const SfxItemSet& rSet)
    {
        basegfx::BColor aColorAttribute;

        if(bShadow)
        {
            const Color aShadowColor(((SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR))).GetColorValue());
            aColorAttribute = aShadowColor.getBColor();
        }
        else
        {
            const Color aLineColor(((XLineColorItem&)(rSet.Get(XATTR_LINECOLOR))).GetColorValue());
            aColorAttribute = aLineColor.getBColor();
        }

        const sal_uInt32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
        const XLineJoint eLineJoint = ((const XLineJointItem&)(rSet.Get(XATTR_LINEJOINT))).GetValue();

        return drawinglayer::attribute::LineAttribute(aColorAttribute, (double)nLineWidth, impGetB2DLineJoin(eLineJoint));
    }

    drawinglayer::attribute::StrokeAttribute impGetStrokeAttribute(const SfxItemSet& rSet)
    {
        const XLineStyle eLineStyle = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
        double fFullDotDashLen(0.0);
        ::std::vector< double > aDotDashArray;

        if(XLINE_DASH == eLineStyle)
        {
            const XDash& rDash = ((const XLineDashItem&)(rSet.Get(XATTR_LINEDASH))).GetDashValue();

            if(rDash.GetDots() || rDash.GetDashes())
            {
                const sal_uInt32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
                fFullDotDashLen = rDash.CreateDotDashArray(aDotDashArray, (double)nLineWidth);
            }
        }

        return drawinglayer::attribute::StrokeAttribute(aDotDashArray, fFullDotDashLen);
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFormTextAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

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
            unsigned                                mbFormTextMirror : 1;   // change orientation
            unsigned                                mbFormTextOutline : 1;  // show contour of objects

            explicit ImpSdrFormTextAttribute(const SfxItemSet& rSet)
            :   mnRefCount(0),
                mnFormTextDistance(((const XFormTextDistanceItem&)rSet.Get(XATTR_FORMTXTDISTANCE)).GetValue()),
                mnFormTextStart(((const XFormTextStartItem&)rSet.Get(XATTR_FORMTXTSTART)).GetValue()),
                mnFormTextShdwXVal(((const XFormTextShadowXValItem&)rSet.Get(XATTR_FORMTXTSHDWXVAL)).GetValue()),
                mnFormTextShdwYVal(((const XFormTextShadowYValItem&)rSet.Get(XATTR_FORMTXTSHDWYVAL)).GetValue()),
                mnFormTextShdwTransp(((const XFormTextShadowTranspItem&)rSet.Get(XATTR_FORMTXTSHDWTRANSP)).GetValue()),
                meFormTextStyle(((const XFormTextStyleItem&)rSet.Get(XATTR_FORMTXTSTYLE)).GetValue()),
                meFormTextAdjust(((const XFormTextAdjustItem&)rSet.Get(XATTR_FORMTXTADJUST)).GetValue()),
                meFormTextShadow(((const XFormTextShadowItem&)rSet.Get(XATTR_FORMTXTSHADOW)).GetValue()),
                maFormTextShdwColor(((const XFormTextShadowColorItem&)rSet.Get(XATTR_FORMTXTSHDWCOLOR)).GetColorValue()),
                maOutline(),
                maShadowOutline(),
                mbFormTextMirror(((const XFormTextMirrorItem&)rSet.Get(XATTR_FORMTXTMIRROR)).GetValue()),
                mbFormTextOutline(((const XFormTextOutlineItem&)rSet.Get(XATTR_FORMTXTOUTLINE)).GetValue())
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
            :   mnRefCount(0),
                mnFormTextDistance(0),
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

            static ImpSdrFormTextAttribute* get_global_default()
            {
                static ImpSdrFormTextAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrFormTextAttribute();

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrFormTextAttribute::SdrFormTextAttribute(const SfxItemSet& rSet)
        :   mpSdrFormTextAttribute(new ImpSdrFormTextAttribute(rSet))
        {
        }

        SdrFormTextAttribute::SdrFormTextAttribute()
        :   mpSdrFormTextAttribute(ImpSdrFormTextAttribute::get_global_default())
        {
            mpSdrFormTextAttribute->mnRefCount++;
        }

        SdrFormTextAttribute::SdrFormTextAttribute(const SdrFormTextAttribute& rCandidate)
        :   mpSdrFormTextAttribute(rCandidate.mpSdrFormTextAttribute)
        {
            mpSdrFormTextAttribute->mnRefCount++;
        }

        SdrFormTextAttribute::~SdrFormTextAttribute()
        {
            if(mpSdrFormTextAttribute->mnRefCount)
            {
                mpSdrFormTextAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrFormTextAttribute;
            }
        }

        bool SdrFormTextAttribute::isDefault() const
        {
            return mpSdrFormTextAttribute == ImpSdrFormTextAttribute::get_global_default();
        }

        SdrFormTextAttribute& SdrFormTextAttribute::operator=(const SdrFormTextAttribute& rCandidate)
        {
            if(rCandidate.mpSdrFormTextAttribute != mpSdrFormTextAttribute)
            {
                if(mpSdrFormTextAttribute->mnRefCount)
                {
                    mpSdrFormTextAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrFormTextAttribute;
                }

                mpSdrFormTextAttribute = rCandidate.mpSdrFormTextAttribute;
                mpSdrFormTextAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrFormTextAttribute::operator==(const SdrFormTextAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrFormTextAttribute == mpSdrFormTextAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrFormTextAttribute == *mpSdrFormTextAttribute);
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
