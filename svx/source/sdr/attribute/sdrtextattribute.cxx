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
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/svdotext.hxx>
#include <svx/outlobj.hxx>
#include <svx/editobj.hxx>
#include <svx/flditem.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrTextAttribute::SdrTextAttribute(
            const SdrText& rSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            XFormTextStyle eFormTextStyle,
            sal_Int32 aTextLeftDistance,
            sal_Int32 aTextUpperDistance,
            sal_Int32 aTextRightDistance,
            sal_Int32 aTextLowerDistance,
            bool bContour,
            bool bFitToSize,
            bool bHideContour,
            bool bBlink,
            bool bScroll,
            bool bInEditMode)
        :   mrSdrText(rSdrText),
            maOutlinerParaObject(rOutlinerParaObject),
            meFormTextStyle(eFormTextStyle),
            maTextLeftDistance(aTextLeftDistance),
            maTextUpperDistance(aTextUpperDistance),
            maTextRightDistance(aTextRightDistance),
            maTextLowerDistance(aTextLowerDistance),
            mbContour(bContour),
            mbFitToSize(bFitToSize),
            mbHideContour(bHideContour),
            mbBlink(bBlink),
            mbScroll(bScroll),
            mbInEditMode(bInEditMode)
        {
        }

        bool SdrTextAttribute::operator==(const SdrTextAttribute& rCandidate) const
        {
            return (getOutlinerParaObject() == rCandidate.getOutlinerParaObject()
                && getFormTextStyle() == rCandidate.getFormTextStyle()
                && getTextLeftDistance() == rCandidate.getTextLeftDistance()
                && getTextUpperDistance() == rCandidate.getTextUpperDistance()
                && getTextRightDistance() == rCandidate.getTextRightDistance()
                && getTextLowerDistance() == rCandidate.getTextLowerDistance()
                && isContour() == rCandidate.isContour()
                && isFitToSize() == rCandidate.isFitToSize()
                && isHideContour() == rCandidate.isHideContour()
                && isBlink() == rCandidate.isBlink()
                && isScroll() == rCandidate.isScroll()
                && isInEditMode() == rCandidate.isInEditMode());
        }

        void SdrTextAttribute::getBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const
        {
            if(isBlink())
            {
                mrSdrText.GetObject().impGetBlinkTextTiming(rAnimList);
            }
        }

        void SdrTextAttribute::getScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const
        {
            if(isScroll())
            {
                mrSdrText.GetObject().impGetScrollTextTiming(rAnimList, fFrameLength, fTextLength);
            }
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
