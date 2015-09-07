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


#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/sdr/attribute/sdrformtextattribute.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <rtl/instance.hxx>


namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrTextAttribute
        {
        public:
            // all-text attributes. The SdrText itself and a copy
            // of the OPO
            const SdrText*                      mpSdrText;
            std::shared_ptr<OutlinerParaObject> mxOutlinerParaObject;

            // Set when it's a FormText; contains all FormText attributes
            SdrFormTextAttribute                maSdrFormTextAttribute;

            // text distances
            sal_Int32                           maTextLeftDistance;
            sal_Int32                           maTextUpperDistance;
            sal_Int32                           maTextRightDistance;
            sal_Int32                           maTextLowerDistance;

            // #i101556# use versioning from text attributes to detect changes
            sal_uInt32                          maPropertiesVersion;

            // text alignments
            SdrTextHorzAdjust                   maSdrTextHorzAdjust;
            SdrTextVertAdjust                   maSdrTextVertAdjust;

            // bitfield
            bool                                mbContour : 1;
            bool                                mbFitToSize : 1;
            bool                                mbAutoFit : 1;
            bool                                mbHideContour : 1;
            bool                                mbBlink : 1;
            bool                                mbScroll : 1;
            bool                                mbInEditMode : 1;
            bool                                mbFixedCellHeight : 1;
            bool                                mbWrongSpell : 1;

            bool                                mbToBeChained : 1;
            bool                                mbChainable : 1;


        public:
            ImpSdrTextAttribute(
                const SdrText* pSdrText,
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
                bool bChainable)
            :   mpSdrText(pSdrText),
                mxOutlinerParaObject(new OutlinerParaObject(rOutlinerParaObject)),
                maSdrFormTextAttribute(),
                maTextLeftDistance(aTextLeftDistance),
                maTextUpperDistance(aTextUpperDistance),
                maTextRightDistance(aTextRightDistance),
                maTextLowerDistance(aTextLowerDistance),
                maPropertiesVersion(0),
                maSdrTextHorzAdjust(aSdrTextHorzAdjust),
                maSdrTextVertAdjust(aSdrTextVertAdjust),
                mbContour(bContour),
                mbFitToSize(bFitToSize),
                mbAutoFit(bAutoFit),
                mbHideContour(bHideContour),
                mbBlink(bBlink),
                mbScroll(bScroll),
                mbInEditMode(bInEditMode),
                mbFixedCellHeight(bFixedCellHeight),
                mbWrongSpell(bWrongSpell),
                mbToBeChained(bToBeChained),
                mbChainable(bChainable)
            {
                if(pSdrText)
                {
                    if(XFT_NONE != eFormTextStyle)
                    {
                        // text on path. Create FormText attribute
                        const SfxItemSet& rSet = pSdrText->GetItemSet();
                        maSdrFormTextAttribute = SdrFormTextAttribute(rSet);
                    }

                    // #i101556# init with version number to detect changes of single text
                    // attribute and/or style sheets in primitive data without having to
                    // copy that data locally (which would be better from principle)
                    maPropertiesVersion = pSdrText->GetObject().GetProperties().getVersion();
                }
            }

            ImpSdrTextAttribute()
            :   mpSdrText(0),
                maSdrFormTextAttribute(),
                maTextLeftDistance(0),
                maTextUpperDistance(0),
                maTextRightDistance(0),
                maTextLowerDistance(0),
                maPropertiesVersion(0),
                maSdrTextHorzAdjust(SDRTEXTHORZADJUST_LEFT),
                maSdrTextVertAdjust(SDRTEXTVERTADJUST_TOP),
                mbContour(false),
                mbFitToSize(false),
                mbAutoFit(false),
                mbHideContour(false),
                mbBlink(false),
                mbScroll(false),
                mbInEditMode(false),
                mbFixedCellHeight(false),
                mbWrongSpell(false),
                mbToBeChained(false),
                mbChainable(false)
            {
            }

            // data read access
            const SdrText& getSdrText() const
            {
                assert(mpSdrText && "Access to text of default version of ImpSdrTextAttribute (!)");
                return *mpSdrText;
            }

            const OutlinerParaObject& getOutlinerParaObject() const
            {
                assert(mxOutlinerParaObject && "Access to OutlinerParaObject of default version of ImpSdrTextAttribute (!)");
                return *mxOutlinerParaObject;
            }

            bool isContour() const { return mbContour; }
            bool isFitToSize() const { return mbFitToSize; }
            bool isAutoFit() const { return mbAutoFit; }
            bool isHideContour() const { return mbHideContour; }
            bool isBlink() const { return mbBlink; }
            bool isScroll() const { return mbScroll; }
            bool isInEditMode() const { return mbInEditMode; }
            bool isFixedCellHeight() const { return mbFixedCellHeight; }
            bool isWrongSpell() const { return mbWrongSpell; }
            bool isToBeChained() const { return mbToBeChained; }
            bool isChainable() const { return mbChainable; }
            const SdrFormTextAttribute& getSdrFormTextAttribute() const { return maSdrFormTextAttribute; }
            sal_Int32 getTextLeftDistance() const { return maTextLeftDistance; }
            sal_Int32 getTextUpperDistance() const { return maTextUpperDistance; }
            sal_Int32 getTextRightDistance() const { return maTextRightDistance; }
            sal_Int32 getTextLowerDistance() const { return maTextLowerDistance; }
            sal_uInt32 getPropertiesVersion() const { return maPropertiesVersion; }
            SdrTextHorzAdjust getSdrTextHorzAdjust() const { return maSdrTextHorzAdjust; }
            SdrTextVertAdjust getSdrTextVertAdjust() const { return maSdrTextVertAdjust; }

            // compare operator
            bool operator==(const ImpSdrTextAttribute& rCandidate) const
            {
                if (mxOutlinerParaObject.get() != rCandidate.mxOutlinerParaObject.get())
                {
                    if (mxOutlinerParaObject && rCandidate.mxOutlinerParaObject)
                    {
                        // compares OPO and it's contents, but traditionally not the RedLining
                        // which is not seen as model, but as temporary information
                        if(!(getOutlinerParaObject() == rCandidate.getOutlinerParaObject()))
                        {
                            return false;
                        }

                        // #i102062# for primitive visualisation, the WrongList (SpellChecking)
                        // is important, too, so use isWrongListEqual since there is no WrongList
                        // comparison in the regular OutlinerParaObject compare (since it's
                        // not-persistent data)
                        if(!(getOutlinerParaObject().isWrongListEqual(rCandidate.getOutlinerParaObject())))
                        {
                            return false;
                        }
                    }
                    else
                    {
                        // only one is zero; not equal
                        return false;
                    }
                }

                return (
                       getSdrFormTextAttribute() == rCandidate.getSdrFormTextAttribute()
                    && getTextLeftDistance() == rCandidate.getTextLeftDistance()
                    && getTextUpperDistance() == rCandidate.getTextUpperDistance()
                    && getTextRightDistance() == rCandidate.getTextRightDistance()
                    && getTextLowerDistance() == rCandidate.getTextLowerDistance()
                    && getPropertiesVersion() == rCandidate.getPropertiesVersion()

                    && getSdrTextHorzAdjust() == rCandidate.getSdrTextHorzAdjust()
                    && getSdrTextVertAdjust() == rCandidate.getSdrTextVertAdjust()

                    && isContour() == rCandidate.isContour()
                    && isFitToSize() == rCandidate.isFitToSize()
                    && isAutoFit() == rCandidate.isAutoFit()
                    && isHideContour() == rCandidate.isHideContour()
                    && isBlink() == rCandidate.isBlink()
                    && isScroll() == rCandidate.isScroll()
                    && isInEditMode() == rCandidate.isInEditMode()
                    && isFixedCellHeight() == rCandidate.isFixedCellHeight()
                    && isWrongSpell() == rCandidate.isWrongSpell()
                    && isToBeChained() == rCandidate.isToBeChained() );
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< SdrTextAttribute::ImplType, theGlobalDefault > {};
        }

        SdrTextAttribute::SdrTextAttribute(
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
            bool bIsToBeChained,
            bool bChainable)
        :   mpSdrTextAttribute(
                ImpSdrTextAttribute(
                    &rSdrText, rOutlinerParaObject, eFormTextStyle, aTextLeftDistance,
                    aTextUpperDistance, aTextRightDistance, aTextLowerDistance,
                    aSdrTextHorzAdjust, aSdrTextVertAdjust, bContour, bFitToSize, bAutoFit,
                    bHideContour, bBlink, bScroll, bInEditMode, bFixedCellHeight, bWrongSpell,
                    bIsToBeChained, bChainable))
        {
        }

        SdrTextAttribute::SdrTextAttribute()
            :   mpSdrTextAttribute(theGlobalDefault::get())
        {
        }

        SdrTextAttribute::SdrTextAttribute(const SdrTextAttribute& rCandidate)
        :   mpSdrTextAttribute(rCandidate.mpSdrTextAttribute)
        {
        }

        SdrTextAttribute::~SdrTextAttribute()
        {
        }

        bool SdrTextAttribute::isDefault() const
        {
            return mpSdrTextAttribute.same_object(theGlobalDefault::get());
        }

        SdrTextAttribute& SdrTextAttribute::operator=(const SdrTextAttribute& rCandidate)
        {
            mpSdrTextAttribute = rCandidate.mpSdrTextAttribute;
            return *this;
        }

        bool SdrTextAttribute::operator==(const SdrTextAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpSdrTextAttribute == mpSdrTextAttribute;
        }

        const SdrText& SdrTextAttribute::getSdrText() const
        {
            return mpSdrTextAttribute->getSdrText();
        }

        const OutlinerParaObject& SdrTextAttribute::getOutlinerParaObject() const
        {
            return mpSdrTextAttribute->getOutlinerParaObject();
        }

        bool SdrTextAttribute::isContour() const
        {
            return mpSdrTextAttribute->isContour();
        }

        bool SdrTextAttribute::isFitToSize() const
        {
            return mpSdrTextAttribute->isFitToSize();
        }

        bool SdrTextAttribute::isAutoFit() const
        {
            return mpSdrTextAttribute->isAutoFit();
        }

        bool SdrTextAttribute::isHideContour() const
        {
            return mpSdrTextAttribute->isHideContour();
        }

        bool SdrTextAttribute::isBlink() const
        {
            return mpSdrTextAttribute->isBlink();
        }

        bool SdrTextAttribute::isScroll() const
        {
            return mpSdrTextAttribute->isScroll();
        }

        bool SdrTextAttribute::isInEditMode() const
        {
            return mpSdrTextAttribute->isInEditMode();
        }

        bool SdrTextAttribute::isToBeChained() const
        {
            return mpSdrTextAttribute->isToBeChained();
        }

        bool SdrTextAttribute::isChainable() const
        {
            return mpSdrTextAttribute->isChainable();
        }


        bool SdrTextAttribute::isFixedCellHeight() const
        {
            return mpSdrTextAttribute->isFixedCellHeight();
        }

        const SdrFormTextAttribute& SdrTextAttribute::getSdrFormTextAttribute() const
        {
            return mpSdrTextAttribute->getSdrFormTextAttribute();
        }

        sal_Int32 SdrTextAttribute::getTextLeftDistance() const
        {
            return mpSdrTextAttribute->getTextLeftDistance();
        }

        sal_Int32 SdrTextAttribute::getTextUpperDistance() const
        {
            return mpSdrTextAttribute->getTextUpperDistance();
        }

        sal_Int32 SdrTextAttribute::getTextRightDistance() const
        {
            return mpSdrTextAttribute->getTextRightDistance();
        }

        sal_Int32 SdrTextAttribute::getTextLowerDistance() const
        {
            return mpSdrTextAttribute->getTextLowerDistance();
        }

        SdrTextHorzAdjust SdrTextAttribute::getSdrTextHorzAdjust() const
        {
            return mpSdrTextAttribute->getSdrTextHorzAdjust();
        }

        SdrTextVertAdjust SdrTextAttribute::getSdrTextVertAdjust() const
        {
            return mpSdrTextAttribute->getSdrTextVertAdjust();
        }

        void SdrTextAttribute::getBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const
        {
            if(isBlink())
            {
                getSdrText().GetObject().impGetBlinkTextTiming(rAnimList);
            }
        }

        void SdrTextAttribute::getScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const
        {
            if(isScroll())
            {
                getSdrText().GetObject().impGetScrollTextTiming(rAnimList, fFrameLength, fTextLength);
            }
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
