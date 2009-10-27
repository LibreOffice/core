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

#include <sal/types.h>
#include <svx/xenum.hxx>
#include <svx/outlobj.hxx>
#include <svx/sdtaitm.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SdrText;
namespace drawinglayer { namespace animation { class AnimationEntryList; }}
namespace drawinglayer { namespace attribute { class SdrFormTextAttribute; }}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrTextAttribute
        {
        private:
            // all-text attributes. The SdrText itself and a copy
            // of te OPO
            const SdrText*                          mpSdrText;
            OutlinerParaObject                      maOutlinerParaObject;

            // Set when it's a FormText; contains all FormText attributes
            SdrFormTextAttribute*                   mpSdrFormTextAttribute;

            // text distances
            sal_Int32                               maTextLeftDistance;
            sal_Int32                               maTextUpperDistance;
            sal_Int32                               maTextRightDistance;
            sal_Int32                               maTextLowerDistance;

            // #i101556# use versioning from text attributes to detect changes
            sal_uInt32                              maPropertiesVersion;

            // text alignments
            SdrTextHorzAdjust                       maSdrTextHorzAdjust;
            SdrTextVertAdjust                       maSdrTextVertAdjust;

            // bitfield
            unsigned                                mbContour : 1;
            unsigned                                mbFitToSize : 1;
            unsigned                                mbHideContour : 1;
            unsigned                                mbBlink : 1;
            unsigned                                mbScroll : 1;
            unsigned                                mbInEditMode : 1;
            unsigned                                mbFixedCellHeight : 1;

        public:
            SdrTextAttribute(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObjectPtr,
                XFormTextStyle eFormTextStyle,
                sal_Int32 aTextLeftDistance,
                sal_Int32 aTextUpperDistance,
                sal_Int32 aTextRightDistance,
                sal_Int32 aTextLowerDistance,
                SdrTextHorzAdjust aSdrTextHorzAdjust,
                SdrTextVertAdjust aSdrTextVertAdjust,
                bool bContour,
                bool bFitToSize,
                bool bHideContour,
                bool bBlink,
                bool bScroll,
                bool bInEditMode,
                bool bFixedCellHeight);
            ~SdrTextAttribute();

            // copy constructor and assigment operator
            SdrTextAttribute(const SdrTextAttribute& rCandidate);
            SdrTextAttribute& operator=(const SdrTextAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrTextAttribute& rCandidate) const;

            // data access
            const SdrText& getSdrText() const { return *mpSdrText; }
            const OutlinerParaObject& getOutlinerParaObject() const { return maOutlinerParaObject; }
            bool isContour() const { return mbContour; }
            bool isFitToSize() const { return mbFitToSize; }
            bool isHideContour() const { return mbHideContour; }
            bool isBlink() const { return mbBlink; }
            bool isScroll() const { return mbScroll; }
            bool isInEditMode() const { return mbInEditMode; }
            bool isFixedCellHeight() const { return mbFixedCellHeight; }
            const SdrFormTextAttribute* getSdrFormTextAttribute() const { return mpSdrFormTextAttribute; }
            sal_Int32 getTextLeftDistance() const { return maTextLeftDistance; }
            sal_Int32 getTextUpperDistance() const { return maTextUpperDistance; }
            sal_Int32 getTextRightDistance() const { return maTextRightDistance; }
            sal_Int32 getTextLowerDistance() const { return maTextLowerDistance; }
            sal_uInt32 getPropertiesVersion() const { return maPropertiesVersion; }
            SdrTextHorzAdjust getSdrTextHorzAdjust() const { return maSdrTextHorzAdjust; }
            SdrTextVertAdjust getSdrTextVertAdjust() const { return maSdrTextVertAdjust; }

            // animation timing generation
            void getBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const;
            void getScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX

// eof
