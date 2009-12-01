/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrtextattribute.hxx,v $
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

#ifndef _SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX

#include <svx/xenum.hxx>
#include <sal/types.h>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SdrText;
class OutlinerParaObject;

namespace drawinglayer { namespace animation {
    class AnimationEntryList;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrTextAttribute
        {
            const SdrText&                          mrSdrText;
            const OutlinerParaObject                maOutlinerParaObject;
            XFormTextStyle                          meFormTextStyle;

            sal_Int32                               maTextLeftDistance;
            sal_Int32                               maTextUpperDistance;
            sal_Int32                               maTextRightDistance;
            sal_Int32                               maTextLowerDistance;

            // bitfield
            unsigned                                mbContour : 1;
            unsigned                                mbFitToSize : 1;
            unsigned                                mbHideContour : 1;
            unsigned                                mbBlink : 1;
            unsigned                                mbScroll : 1;
            unsigned                                mbInEditMode : 1;

        public:
            SdrTextAttribute(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
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
                bool bInEditMode);
            bool operator==(const SdrTextAttribute& rCandidate) const;

            // data access
            const SdrText& getSdrText() const { return mrSdrText; }
            const OutlinerParaObject& getOutlinerParaObject() const { return maOutlinerParaObject; }
            bool isContour() const { return mbContour; }
            bool isFontwork() const { return (XFT_NONE != meFormTextStyle); }
            bool isFitToSize() const { return mbFitToSize; }
            bool isHideContour() const { return mbHideContour; }
            bool isBlink() const { return mbBlink; }
            bool isScroll() const { return mbScroll; }
            bool isInEditMode() const { return mbInEditMode; }
            XFormTextStyle getFormTextStyle() const { return meFormTextStyle; }
            sal_Int32 getTextLeftDistance() const { return maTextLeftDistance; }
            sal_Int32 getTextUpperDistance() const { return maTextUpperDistance; }
            sal_Int32 getTextRightDistance() const { return maTextRightDistance; }
            sal_Int32 getTextLowerDistance() const { return maTextLowerDistance; }

            // animation timing generation
            void getBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const;
            void getScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX

// eof
