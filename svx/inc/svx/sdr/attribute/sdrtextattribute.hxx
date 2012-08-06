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

#ifndef _SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/xenum.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtaitm.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SdrText;

namespace drawinglayer { namespace animation {
    class AnimationEntryList;
}}

namespace drawinglayer { namespace attribute {
    class SdrFormTextAttribute;
}}

namespace drawinglayer { namespace attribute {
    class ImpSdrTextAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrTextAttribute
        {
        private:
            ImpSdrTextAttribute*        mpSdrTextAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrTextAttribute(
                const SdrText& rSdrText,
                const OutlinerParaObject& rOutlinerParaObject,
                XFormTextStyle eFormTextStyle,
                sal_Int32 aTextLeftDistance,
                sal_Int32 aTextUpperDistance,
                sal_Int32 aTextRightDistance,
                sal_Int32 aTextLowerDistance,
                SdrTextHorzAdjust aSdrTextHorzAdjust,
                SdrTextVertAdjust aSdrTextVertAdjust,
                bool bContour,
                bool bFitToSize,
                bool bAutoFit,
                bool bHideContour,
                bool bBlink,
                bool bScroll,
                bool bInEditMode,
                bool bFixedCellHeight,
                bool bWrongSpell);
            SdrTextAttribute();
            SdrTextAttribute(const SdrTextAttribute& rCandidate);
            SdrTextAttribute& operator=(const SdrTextAttribute& rCandidate);
            ~SdrTextAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrTextAttribute& rCandidate) const;

            // data read access
            const SdrText& getSdrText() const;
            const OutlinerParaObject& getOutlinerParaObject() const;
            bool isContour() const;
            bool isFitToSize() const;
            bool isAutoFit() const;
            bool isHideContour() const;
            bool isBlink() const;
            bool isScroll() const;
            bool isInEditMode() const;
            bool isFixedCellHeight() const;
            const SdrFormTextAttribute& getSdrFormTextAttribute() const;
            sal_Int32 getTextLeftDistance() const;
            sal_Int32 getTextUpperDistance() const;
            sal_Int32 getTextRightDistance() const;
            sal_Int32 getTextLowerDistance() const;
            SdrTextHorzAdjust getSdrTextHorzAdjust() const;
            SdrTextVertAdjust getSdrTextVertAdjust() const;

            // helpers: animation timing generators
            void getBlinkTextTiming(
                drawinglayer::animation::AnimationEntryList& rAnimList) const;
            void getScrollTextTiming(
                drawinglayer::animation::AnimationEntryList& rAnimList,
                double fFrameLength,
                double fTextLength) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
