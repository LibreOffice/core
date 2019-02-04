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

#include <sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <svx/svdotext.hxx>
#include <basegfx/color/bcolor.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>


using namespace com::sun::star;


namespace
{
    sal_Int16 getPageNumber(const uno::Reference< drawing::XDrawPage >& rxDrawPage)
    {
        sal_Int16 nRetval(0);
        uno::Reference< beans::XPropertySet > xSet(rxDrawPage, uno::UNO_QUERY);

        if (xSet.is())
        {
            try
            {
                const uno::Any aNumber(xSet->getPropertyValue("Number"));
                aNumber >>= nRetval;
            }
            catch(const uno::Exception&)
            {
                OSL_ASSERT(false);
            }
        }

        return nRetval;
    }

    sal_Int16 getPageCount(const uno::Reference< drawing::XDrawPage >& rxDrawPage)
    {
        sal_Int16 nRetval(0);
        SdrPage* pPage = GetSdrPageFromXDrawPage(rxDrawPage);

        if(pPage)
        {
            if( (pPage->GetPageNum() == 0) && !pPage->IsMasterPage() )
            {
                // handout page!
                return pPage->getSdrModelFromSdrPage().getHandoutPageCount();
            }
            else
            {
                const sal_uInt16 nPageCount(pPage->getSdrModelFromSdrPage().GetPageCount());
                nRetval = (static_cast<sal_Int16>(nPageCount) - 1) / 2;
            }
        }

        return nRetval;
    }
} // end of anonymous namespace


namespace drawinglayer
{
    namespace primitive2d
    {
        // support for XTEXT_PAINTSHAPE_BEGIN/XTEXT_PAINTSHAPE_END Metafile comments
        // for slideshow. This uses TextHierarchyBlockPrimitive2D to mark a text block.
        // ATM there is only one text block per SdrObject, this may get more in the future
        void SdrTextPrimitive2D::encapsulateWithTextHierarchyBlockPrimitive2D(Primitive2DContainer& rContainer, const Primitive2DContainer& rCandidate)
        {
            rContainer.push_back(new TextHierarchyBlockPrimitive2D(rCandidate));
        }

        SdrTextPrimitive2D::SdrTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject)
        :   BufferedDecompositionPrimitive2D(),
            mrSdrText(const_cast< SdrText* >(pSdrText)),
            maOutlinerParaObject(rOutlinerParaObject),
            mxLastVisualizingPage(),
            mnLastPageNumber(0),
            mnLastPageCount(0),
            maLastTextBackgroundColor(),
            mbContainsPageField(false),
            mbContainsPageCountField(false),
            mbContainsOtherFields(false)
        {
            const EditTextObject& rETO = maOutlinerParaObject.GetTextObject();

            mbContainsPageField = rETO.HasField(SvxPageField::CLASS_ID);
            mbContainsPageCountField = rETO.HasField(SvxPagesField::CLASS_ID);
            mbContainsOtherFields = rETO.HasField(SvxHeaderField::CLASS_ID)
                || rETO.HasField(SvxFooterField::CLASS_ID)
                || rETO.HasField(SvxDateTimeField::CLASS_ID)
                || rETO.HasField(SvxAuthorField::CLASS_ID);
        }

        const SdrText* SdrTextPrimitive2D::getSdrText() const { return mrSdrText.get(); }

        bool SdrTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrTextPrimitive2D& rCompare = static_cast<const SdrTextPrimitive2D&>(rPrimitive);

                return (

                    // compare OPO and content, but not WrongList
                    getOutlinerParaObject() == rCompare.getOutlinerParaObject()

                    // also compare WrongList (not-persistent data, but visualized)
                    && getOutlinerParaObject().isWrongListEqual(rCompare.getOutlinerParaObject()));
            }

            return false;
        }

        void SdrTextPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            uno::Reference< drawing::XDrawPage > xCurrentlyVisualizingPage;
            bool bCurrentlyVisualizingPageIsSet(false);
            Color aNewTextBackgroundColor;
            bool bNewTextBackgroundColorIsSet(false);
            sal_Int16 nCurrentlyValidPageNumber(0);
            sal_Int16 nCurrentlyValidPageCount(0);

            if(!getBuffered2DDecomposition().empty())
            {
                bool bDoDelete(false);

                // check visualized page
                if(mbContainsPageField || mbContainsPageCountField || mbContainsOtherFields)
                {
                    // get visualized page and remember
                    xCurrentlyVisualizingPage = rViewInformation.getVisualizedPage();
                    bCurrentlyVisualizingPageIsSet = true;

                    if(xCurrentlyVisualizingPage != mxLastVisualizingPage)
                    {
                        bDoDelete = true;
                    }

                    // #i98870# check visualized PageNumber
                    if(!bDoDelete && mbContainsPageField)
                    {
                        nCurrentlyValidPageNumber = getPageNumber(xCurrentlyVisualizingPage);

                        if(nCurrentlyValidPageNumber != mnLastPageNumber)
                        {
                            bDoDelete = true;
                        }
                    }

                    // #i98870# check visualized PageCount, too
                    if(!bDoDelete && mbContainsPageCountField)
                    {
                        nCurrentlyValidPageCount = getPageCount(xCurrentlyVisualizingPage);

                        if(nCurrentlyValidPageCount != mnLastPageCount)
                        {
                            bDoDelete = true;
                        }
                    }
                }

                // #i101443#  check change of TextBackgroundolor
                if(!bDoDelete && getSdrText())
                {
                    SdrOutliner& rDrawOutliner = getSdrText()->GetObject().getSdrModelFromSdrObject().GetDrawOutliner();
                    aNewTextBackgroundColor = rDrawOutliner.GetBackgroundColor();
                    bNewTextBackgroundColorIsSet = true;

                    if(aNewTextBackgroundColor != maLastTextBackgroundColor)
                    {
                        bDoDelete = true;
                    }
                }

                if(bDoDelete)
                {
                    const_cast< SdrTextPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
                }
            }

            if(getBuffered2DDecomposition().empty())
            {
                if(!bCurrentlyVisualizingPageIsSet && mbContainsPageField)
                {
                    xCurrentlyVisualizingPage = rViewInformation.getVisualizedPage();
                }

                if(!nCurrentlyValidPageNumber && mbContainsPageField)
                {
                    nCurrentlyValidPageNumber = getPageNumber(xCurrentlyVisualizingPage);
                }

                if(!nCurrentlyValidPageCount && mbContainsPageCountField)
                {
                    nCurrentlyValidPageCount = getPageCount(xCurrentlyVisualizingPage);
                }

                if(!bNewTextBackgroundColorIsSet && getSdrText())
                {
                    SdrOutliner& rDrawOutliner = getSdrText()->GetObject().getSdrModelFromSdrObject().GetDrawOutliner();
                    aNewTextBackgroundColor = rDrawOutliner.GetBackgroundColor();
                }

                const_cast< SdrTextPrimitive2D* >(this)->mxLastVisualizingPage = xCurrentlyVisualizingPage;
                const_cast< SdrTextPrimitive2D* >(this)->mnLastPageNumber = nCurrentlyValidPageNumber;
                const_cast< SdrTextPrimitive2D* >(this)->mnLastPageCount = nCurrentlyValidPageCount;
                const_cast< SdrTextPrimitive2D* >(this)->maLastTextBackgroundColor = aNewTextBackgroundColor;
            }

            // call parent
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace primitive2d
    {
        void SdrContourTextPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeContourTextPrimitive(aRetval, *this, aViewInformation);

            encapsulateWithTextHierarchyBlockPrimitive2D(rContainer, aRetval);
        }

        SdrContourTextPrimitive2D::SdrContourTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maUnitPolyPolygon(rUnitPolyPolygon),
            maObjectTransform(rObjectTransform)
        {
        }

        bool SdrContourTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrContourTextPrimitive2D& rCompare = static_cast<const SdrContourTextPrimitive2D&>(rPrimitive);

                return (getUnitPolyPolygon() == rCompare.getUnitPolyPolygon()
                    && getObjectTransform() == rCompare.getObjectTransform());
            }

            return false;
        }

        std::unique_ptr<SdrTextPrimitive2D> SdrContourTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return std::make_unique<SdrContourTextPrimitive2D>(
                getSdrText(),
                getOutlinerParaObject(),
                getUnitPolyPolygon(),
                rTransform * getObjectTransform());
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrContourTextPrimitive2D, PRIMITIVE2D_ID_SDRCONTOURTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace primitive2d
    {
        void SdrPathTextPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposePathTextPrimitive(aRetval, *this, aViewInformation);

            encapsulateWithTextHierarchyBlockPrimitive2D(rContainer, aRetval);
        }

        SdrPathTextPrimitive2D::SdrPathTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            const basegfx::B2DPolyPolygon& rPathPolyPolygon,
            const attribute::SdrFormTextAttribute& rSdrFormTextAttribute)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maPathPolyPolygon(rPathPolyPolygon),
            maSdrFormTextAttribute(rSdrFormTextAttribute)
        {
        }

        bool SdrPathTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrPathTextPrimitive2D& rCompare = static_cast<const SdrPathTextPrimitive2D&>(rPrimitive);

                return (getPathPolyPolygon() == rCompare.getPathPolyPolygon()
                    && getSdrFormTextAttribute() == rCompare.getSdrFormTextAttribute());
            }

            return false;
        }

        std::unique_ptr<SdrTextPrimitive2D> SdrPathTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            basegfx::B2DPolyPolygon aNewPolyPolygon(getPathPolyPolygon());
            aNewPolyPolygon.transform(rTransform);

            return std::make_unique<SdrPathTextPrimitive2D>(
                getSdrText(),
                getOutlinerParaObject(),
                aNewPolyPolygon,
                getSdrFormTextAttribute());
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrPathTextPrimitive2D, PRIMITIVE2D_ID_SDRPATHTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace primitive2d
    {
        void SdrBlockTextPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeBlockTextPrimitive(aRetval, *this, aViewInformation);

            encapsulateWithTextHierarchyBlockPrimitive2D(rContainer, aRetval);
        }

        SdrBlockTextPrimitive2D::SdrBlockTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            const basegfx::B2DHomMatrix& rTextRangeTransform,
            SdrTextHorzAdjust aSdrTextHorzAdjust,
            SdrTextVertAdjust aSdrTextVertAdjust,
            bool bFixedCellHeight,
            bool bUnlimitedPage,
            bool bCellText,
            bool bWordWrap,
            bool bClipOnBounds)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maTextRangeTransform(rTextRangeTransform),
            maSdrTextHorzAdjust(aSdrTextHorzAdjust),
            maSdrTextVertAdjust(aSdrTextVertAdjust),
            mbFixedCellHeight(bFixedCellHeight),
            mbUnlimitedPage(bUnlimitedPage),
            mbCellText(bCellText),
            mbWordWrap(bWordWrap),
            mbClipOnBounds(bClipOnBounds)
        {
        }

        bool SdrBlockTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrBlockTextPrimitive2D& rCompare = static_cast<const SdrBlockTextPrimitive2D&>(rPrimitive);

                return (getTextRangeTransform() == rCompare.getTextRangeTransform()
                    && getSdrTextHorzAdjust() == rCompare.getSdrTextHorzAdjust()
                    && getSdrTextVertAdjust() == rCompare.getSdrTextVertAdjust()
                    && isFixedCellHeight() == rCompare.isFixedCellHeight()
                    && getUnlimitedPage() == rCompare.getUnlimitedPage()
                    && getCellText() == rCompare.getCellText()
                    && getWordWrap() == rCompare.getWordWrap()
                    && getClipOnBounds() == rCompare.getClipOnBounds());
            }

            return false;
        }

        std::unique_ptr<SdrTextPrimitive2D> SdrBlockTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return std::make_unique<SdrBlockTextPrimitive2D>(
                getSdrText(),
                getOutlinerParaObject(),
                rTransform * getTextRangeTransform(),
                getSdrTextHorzAdjust(),
                getSdrTextVertAdjust(),
                isFixedCellHeight(),
                getUnlimitedPage(),
                getCellText(),
                getWordWrap(),
                getClipOnBounds());
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrBlockTextPrimitive2D, PRIMITIVE2D_ID_SDRBLOCKTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace primitive2d
    {
         void SdrAutoFitTextPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const
         {
             Primitive2DContainer aRetval;
             getSdrText()->GetObject().impDecomposeAutoFitTextPrimitive(aRetval, *this, aViewInformation);

             encapsulateWithTextHierarchyBlockPrimitive2D(rContainer, aRetval);
         }

         SdrAutoFitTextPrimitive2D::SdrAutoFitTextPrimitive2D(
             const SdrText* pSdrText,
             const OutlinerParaObject& rParaObj,
             const ::basegfx::B2DHomMatrix& rTextRangeTransform,
             bool bWordWrap)
         :  SdrTextPrimitive2D(pSdrText, rParaObj),
             maTextRangeTransform(rTextRangeTransform),
             mbWordWrap(bWordWrap)
         {
         }

         bool SdrAutoFitTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
         {
             if(SdrTextPrimitive2D::operator==(rPrimitive))
             {
                 const SdrBlockTextPrimitive2D& rCompare = static_cast<const SdrBlockTextPrimitive2D&>(rPrimitive);

                 return (getTextRangeTransform() == rCompare.getTextRangeTransform()
                     && getWordWrap() == rCompare.getWordWrap());
             }

             return false;
         }

         std::unique_ptr<SdrTextPrimitive2D> SdrAutoFitTextPrimitive2D::createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const
         {
             return std::make_unique<SdrAutoFitTextPrimitive2D>(getSdrText(), getOutlinerParaObject(), rTransform * getTextRangeTransform(), getWordWrap());
         }

         // provide unique ID
         ImplPrimitive2DIDBlock(SdrAutoFitTextPrimitive2D, PRIMITIVE2D_ID_SDRAUTOFITTEXTPRIMITIVE2D)

     } // end of namespace primitive2d
 } // end of namespace drawinglayer

namespace drawinglayer
{
    namespace primitive2d
    {

        SdrChainedTextPrimitive2D::SdrChainedTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            const basegfx::B2DHomMatrix& rTextRangeTransform)
        : SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
          maTextRangeTransform(rTextRangeTransform)
        { }

        void SdrChainedTextPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeChainedTextPrimitive(aRetval, *this, aViewInformation);

            encapsulateWithTextHierarchyBlockPrimitive2D(rContainer, aRetval);
        }

        bool SdrChainedTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
         {
             if(SdrTextPrimitive2D::operator==(rPrimitive))
             {
                 const SdrBlockTextPrimitive2D& rCompare = static_cast<const SdrBlockTextPrimitive2D&>(rPrimitive);

                 return (getTextRangeTransform() == rCompare.getTextRangeTransform());
             }

             return false;
         }

        std::unique_ptr<SdrTextPrimitive2D> SdrChainedTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return std::make_unique<SdrChainedTextPrimitive2D>(getSdrText(), getOutlinerParaObject(), rTransform * getTextRangeTransform());
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrChainedTextPrimitive2D, PRIMITIVE2D_ID_SDRCHAINEDTEXTPRIMITIVE2D)
    } // end of namespace primitive2d
} // end of namespace drawinglayer


 namespace drawinglayer
 {
     namespace primitive2d
     {
        void SdrStretchTextPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeStretchTextPrimitive(aRetval, *this, aViewInformation);

            encapsulateWithTextHierarchyBlockPrimitive2D(rContainer, aRetval);
        }

        SdrStretchTextPrimitive2D::SdrStretchTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            const basegfx::B2DHomMatrix& rTextRangeTransform,
            bool bFixedCellHeight)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maTextRangeTransform(rTextRangeTransform),
            mbFixedCellHeight(bFixedCellHeight)
        {
        }

        bool SdrStretchTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(SdrTextPrimitive2D::operator==(rPrimitive))
            {
                const SdrStretchTextPrimitive2D& rCompare = static_cast<const SdrStretchTextPrimitive2D&>(rPrimitive);

                return (getTextRangeTransform() == rCompare.getTextRangeTransform()
                    && isFixedCellHeight() == rCompare.isFixedCellHeight());
            }

            return false;
        }

        std::unique_ptr<SdrTextPrimitive2D> SdrStretchTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return std::make_unique<SdrStretchTextPrimitive2D>(
                getSdrText(),
                getOutlinerParaObject(),
                rTransform * getTextRangeTransform(),
                isFixedCellHeight());
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrStretchTextPrimitive2D, PRIMITIVE2D_ID_SDRSTRETCHTEXTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
