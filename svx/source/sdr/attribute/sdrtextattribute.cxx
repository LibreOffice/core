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
#include <svx/sdr/attribute/sdrformtextattribute.hxx>
#include <svx/svdotext.hxx>
#include <svx/outlobj.hxx>
#include <svx/editobj.hxx>
#include <svx/flditem.hxx>
#include <svx/sdr/properties/properties.hxx>

//////////////////////////////////////////////////////////////////////////////
// pointer compare define
#define pointerOrContentEqual(p, q) ((p == q) || (p && q && *p == *q))

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
        :   mpSdrText(&rSdrText),
            maOutlinerParaObject(rOutlinerParaObject),
            mpSdrFormTextAttribute(0),
            maTextLeftDistance(aTextLeftDistance),
            maTextUpperDistance(aTextUpperDistance),
            maTextRightDistance(aTextRightDistance),
            maTextLowerDistance(aTextLowerDistance),
            maPropertiesVersion(0),
            mbContour(bContour),
            mbFitToSize(bFitToSize),
            mbHideContour(bHideContour),
            mbBlink(bBlink),
            mbScroll(bScroll),
            mbInEditMode(bInEditMode)
        {
            if(XFT_NONE != eFormTextStyle)
            {
                // text on path. Create FormText attribute
                const SfxItemSet& rSet = getSdrText().GetItemSet();
                mpSdrFormTextAttribute = new SdrFormTextAttribute(rSet);
            }

            // #i101556# init with version number to detect changes of single text
            // attribute and/or style sheets in primitive data without having to
            // copy that data locally (which would be better from principle)
            maPropertiesVersion = rSdrText.GetObject().GetProperties().getVersion();
        }

        SdrTextAttribute::~SdrTextAttribute()
        {
            if(mpSdrFormTextAttribute)
            {
                delete mpSdrFormTextAttribute;
                mpSdrFormTextAttribute = 0;
            }
        }

        SdrTextAttribute::SdrTextAttribute(const SdrTextAttribute& rCandidate)
        :   mpSdrText(&rCandidate.getSdrText()),
            maOutlinerParaObject(rCandidate.getOutlinerParaObject()),
            mpSdrFormTextAttribute(0),
            maTextLeftDistance(rCandidate.getTextLeftDistance()),
            maTextUpperDistance(rCandidate.getTextUpperDistance()),
            maTextRightDistance(rCandidate.getTextRightDistance()),
            maTextLowerDistance(rCandidate.getTextLowerDistance()),
            mbContour(rCandidate.isContour()),
            mbFitToSize(rCandidate.isFitToSize()),
            mbHideContour(rCandidate.isHideContour()),
            mbBlink(rCandidate.isBlink()),
            mbScroll(rCandidate.isScroll()),
            mbInEditMode(rCandidate.isInEditMode())
        {
            if(rCandidate.getSdrFormTextAttribute())
            {
                mpSdrFormTextAttribute = new SdrFormTextAttribute(*rCandidate.getSdrFormTextAttribute());
            }
        }

        SdrTextAttribute& SdrTextAttribute::operator=(const SdrTextAttribute& rCandidate)
        {
            mpSdrText = &rCandidate.getSdrText();
            maOutlinerParaObject = rCandidate.getOutlinerParaObject();

            if(mpSdrFormTextAttribute)
            {
                delete mpSdrFormTextAttribute;
            }

            mpSdrFormTextAttribute = 0;

            if(rCandidate.getSdrFormTextAttribute())
            {
                mpSdrFormTextAttribute = new SdrFormTextAttribute(*rCandidate.getSdrFormTextAttribute());
            }

            maTextLeftDistance = rCandidate.getTextLeftDistance();
            maTextUpperDistance = rCandidate.getTextUpperDistance();
            maTextRightDistance = rCandidate.getTextRightDistance();
            maTextLowerDistance = rCandidate.getTextLowerDistance();
            mbContour = rCandidate.isContour();
            mbFitToSize = rCandidate.isFitToSize();
            mbHideContour = rCandidate.isHideContour();
            mbBlink = rCandidate.isBlink();
            mbScroll = rCandidate.isScroll();
            mbInEditMode = rCandidate.isInEditMode();

            return *this;
        }

        bool SdrTextAttribute::operator==(const SdrTextAttribute& rCandidate) const
        {
            return (getOutlinerParaObject() == rCandidate.getOutlinerParaObject()
                // #i102062# for primitive visualisation, the WrongList (SpellChecking)
                // is important, too, so use isWrongListEqual since there is no WrongList
                // comparison in the regular OutlinerParaObject compare (since it's
                // not-persistent data)
                && getOutlinerParaObject().isWrongListEqual(rCandidate.getOutlinerParaObject())
                && pointerOrContentEqual(getSdrFormTextAttribute(), rCandidate.getSdrFormTextAttribute())
                && getTextLeftDistance() == rCandidate.getTextLeftDistance()
                && getTextUpperDistance() == rCandidate.getTextUpperDistance()
                && getTextRightDistance() == rCandidate.getTextRightDistance()
                && getTextLowerDistance() == rCandidate.getTextLowerDistance()
                && getPropertiesVersion() == rCandidate.getPropertiesVersion()
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
                mpSdrText->GetObject().impGetBlinkTextTiming(rAnimList);
            }
        }

        void SdrTextAttribute::getScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const
        {
            if(isScroll())
            {
                mpSdrText->GetObject().impGetScrollTextTiming(rAnimList, fFrameLength, fTextLength);
            }
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
