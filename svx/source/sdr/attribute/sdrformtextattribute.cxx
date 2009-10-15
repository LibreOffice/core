/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrtextattribute.cxx,v $
 *
 * $Revision: 1.2 $
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

#include "precompiled_svx.hxx"

#include <svx/sdr/attribute/sdrformtextattribute.hxx>
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
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <svx/sdshcitm.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <xlinjoit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xdash.hxx>
#include <svx/xlndsit.hxx>
#include <svx/sdr/attribute/sdrformtextoutlineattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// pointer compare define
#define pointerOrContentEqual(p, q) ((p == q) || (p && q && *p == *q))

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
        SdrFormTextAttribute::SdrFormTextAttribute(const SfxItemSet& rSet)
        :   mnFormTextDistance(((const XFormTextDistanceItem&)rSet.Get(XATTR_FORMTXTDISTANCE)).GetValue()),
            mnFormTextStart(((const XFormTextStartItem&)rSet.Get(XATTR_FORMTXTSTART)).GetValue()),
            mnFormTextShdwXVal(((const XFormTextShadowXValItem&)rSet.Get(XATTR_FORMTXTSHDWXVAL)).GetValue()),
            mnFormTextShdwYVal(((const XFormTextShadowYValItem&)rSet.Get(XATTR_FORMTXTSHDWYVAL)).GetValue()),
            mnFormTextShdwTransp(((const XFormTextShadowTranspItem&)rSet.Get(XATTR_FORMTXTSHDWTRANSP)).GetValue()),
            meFormTextStyle(((const XFormTextStyleItem&)rSet.Get(XATTR_FORMTXTSTYLE)).GetValue()),
            meFormTextAdjust(((const XFormTextAdjustItem&)rSet.Get(XATTR_FORMTXTADJUST)).GetValue()),
            meFormTextShadow(((const XFormTextShadowItem&)rSet.Get(XATTR_FORMTXTSHADOW)).GetValue()),
            maFormTextShdwColor(((const XFormTextShadowColorItem&)rSet.Get(XATTR_FORMTXTSHDWCOLOR)).GetColorValue()),
            mpOutline(0),
            mpShadowOutline(0),
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

                    mpOutline = new SdrFormTextOutlineAttribute(
                        aLineAttribute, aStrokeAttribute, nTransparence);
                }

                if(XFTSHADOW_NONE != getFormTextShadow())
                {
                    // also need to prepare attributes for shadow outlines
                    const LineAttribute aLineAttribute(impGetLineAttribute(true, rSet));
                    const sal_uInt8 nTransparence(impGetStrokeTransparence(true, rSet));

                    mpShadowOutline = new SdrFormTextOutlineAttribute(
                        aLineAttribute, aStrokeAttribute, nTransparence);
                }
            }
        }

        SdrFormTextAttribute::~SdrFormTextAttribute()
        {
            if(mpOutline)
            {
                delete mpOutline;
                mpOutline = 0;
            }

            if(mpShadowOutline)
            {
                delete mpShadowOutline;
                mpShadowOutline = 0;
            }
        }

        SdrFormTextAttribute::SdrFormTextAttribute(const SdrFormTextAttribute& rCandidate)
        :   mnFormTextDistance(rCandidate.getFormTextDistance()),
            mnFormTextStart(rCandidate.getFormTextStart()),
            mnFormTextShdwXVal(rCandidate.getFormTextShdwXVal()),
            mnFormTextShdwYVal(rCandidate.getFormTextShdwYVal()),
            mnFormTextShdwTransp(rCandidate.getFormTextShdwTransp()),
            meFormTextStyle(rCandidate.getFormTextStyle()),
            meFormTextAdjust(rCandidate.getFormTextAdjust()),
            meFormTextShadow(rCandidate.getFormTextShadow()),
            maFormTextShdwColor(rCandidate.getFormTextShdwColor()),
            mpOutline(0),
            mpShadowOutline(0),
            mbFormTextMirror(rCandidate.getFormTextMirror()),
            mbFormTextOutline(rCandidate.getFormTextOutline())
        {
            if(rCandidate.getOutline())
            {
                mpOutline = new SdrFormTextOutlineAttribute(*rCandidate.getOutline());
            }

            if(rCandidate.getShadowOutline())
            {
                mpShadowOutline = new SdrFormTextOutlineAttribute(*rCandidate.getShadowOutline());
            }
        }

        SdrFormTextAttribute& SdrFormTextAttribute::operator=(const SdrFormTextAttribute& rCandidate)
        {
            mnFormTextDistance = rCandidate.getFormTextDistance();
            mnFormTextStart = rCandidate.getFormTextStart();
            mnFormTextShdwXVal = rCandidate.getFormTextShdwXVal();
            mnFormTextShdwYVal = rCandidate.getFormTextShdwYVal();
            mnFormTextShdwTransp = rCandidate.getFormTextShdwTransp();
            meFormTextStyle = rCandidate.getFormTextStyle();
            meFormTextAdjust = rCandidate.getFormTextAdjust();
            meFormTextShadow = rCandidate.getFormTextShadow();
            maFormTextShdwColor = rCandidate.getFormTextShdwColor();

            if(mpOutline)
            {
                delete mpOutline;
            }

            mpOutline = 0;

            if(rCandidate.getOutline())
            {
                mpOutline = new SdrFormTextOutlineAttribute(*rCandidate.getOutline());
            }

            if(mpShadowOutline)
            {
                delete mpShadowOutline;
            }

            mpShadowOutline = 0;

            if(rCandidate.getShadowOutline())
            {
                mpShadowOutline = new SdrFormTextOutlineAttribute(*rCandidate.getShadowOutline());
            }

            mbFormTextMirror = rCandidate.getFormTextMirror();
            mbFormTextOutline = rCandidate.getFormTextOutline();

            return *this;
        }

        bool SdrFormTextAttribute::operator==(const SdrFormTextAttribute& rCandidate) const
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
                && pointerOrContentEqual(getOutline(), rCandidate.getOutline())
                && pointerOrContentEqual(getShadowOutline(), rCandidate.getShadowOutline())
                && getFormTextMirror() == rCandidate.getFormTextMirror()
                && getFormTextOutline() == rCandidate.getFormTextOutline());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
