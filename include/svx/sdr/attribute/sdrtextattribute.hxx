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

#ifndef INCLUDED_SVX_SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX
#define INCLUDED_SVX_SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/xenum.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtaitm.hxx>
#include <o3tl/cow_wrapper.hxx>

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



namespace drawinglayer
{
    namespace attribute
    {
        class SdrTextAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdrTextAttribute > ImplType;

        private:
            ImplType mpSdrTextAttribute;

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
                bool bWrongSpell,
                bool bToBeChained,
                bool bChainable);

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

            bool isToBeChained() const;
            bool isChainable() const;


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



#endif // INCLUDED_SVX_SDR_ATTRIBUTE_SDRTEXTATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
