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
#include <vcl/svapp.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <utility>
#include <osl/diagnose.h>


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
                const uno::Any aNumber(xSet->getPropertyValue(u"Number"_ustr));
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


namespace drawinglayer::primitive2d
{
        // support for XTEXT_PAINTSHAPE_BEGIN/XTEXT_PAINTSHAPE_END Metafile comments
        // for slideshow. This uses TextHierarchyBlockPrimitive2D to mark a text block.
        // ATM there is only one text block per SdrObject, this may get more in the future
        Primitive2DReference SdrTextPrimitive2D::encapsulateWithTextHierarchyBlockPrimitive2D(Primitive2DContainer&& aCandidate)
        {
            return new TextHierarchyBlockPrimitive2D(std::move(aCandidate));
        }

        SdrTextPrimitive2D::SdrTextPrimitive2D(
            const SdrText* pSdrText,
            OutlinerParaObject aOutlinerParaObject)
        :   mxSdrText(const_cast< SdrText* >(pSdrText)),
            maOutlinerParaObject(std::move(aOutlinerParaObject)),
            mnLastPageNumber(0),
            mnLastPageCount(0),
            mbContainsPageField(false),
            mbContainsPageCountField(false),
            mbContainsOtherFields(false),
            mbEditViewActive(false)
        {
            const EditTextObject& rETO = maOutlinerParaObject.GetTextObject();

            mbContainsPageField = rETO.HasField(SvxPageField::CLASS_ID);
            mbContainsPageCountField = rETO.HasField(SvxPagesField::CLASS_ID);
            mbContainsOtherFields = rETO.HasField(SvxHeaderField::CLASS_ID)
                || rETO.HasField(SvxFooterField::CLASS_ID)
                || rETO.HasField(SvxDateTimeField::CLASS_ID)
                || rETO.HasField(SvxAuthorField::CLASS_ID);
        }

        const SdrText* SdrTextPrimitive2D::getSdrText() const { return mxSdrText.get().get(); }

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

            if(hasBuffered2DDecomposition())
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

                // tdf#167511 check if EditViewActive has changed, but only if we are in
                // HighContrast mode at all
                if (!bDoDelete && mbEditViewActive != rViewInformation.getEditViewActive()
                    && Application::GetSettings().GetStyleSettings().GetHighContrastMode())
                {
                    bDoDelete = true;
                }

                if(bDoDelete)
                {
                    const_cast< SdrTextPrimitive2D* >(this)->setBuffered2DDecomposition(nullptr);
                }
            }

            if(!hasBuffered2DDecomposition())
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

                const_cast< SdrTextPrimitive2D* >(this)->mxLastVisualizingPage = std::move(xCurrentlyVisualizingPage);
                const_cast< SdrTextPrimitive2D* >(this)->mnLastPageNumber = nCurrentlyValidPageNumber;
                const_cast< SdrTextPrimitive2D* >(this)->mnLastPageCount = nCurrentlyValidPageCount;
                const_cast< SdrTextPrimitive2D* >(this)->maLastTextBackgroundColor = aNewTextBackgroundColor;
                const_cast< SdrTextPrimitive2D* >(this)->mbEditViewActive = rViewInformation.getEditViewActive();
            }

            // call parent
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }




        Primitive2DReference SdrContourTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeContourTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(std::move(aRetval));
        }

        SdrContourTextPrimitive2D::SdrContourTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            basegfx::B2DPolyPolygon aUnitPolyPolygon,
            basegfx::B2DHomMatrix aObjectTransform)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maUnitPolyPolygon(std::move(aUnitPolyPolygon)),
            maObjectTransform(std::move(aObjectTransform))
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

        rtl::Reference<SdrTextPrimitive2D> SdrContourTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrContourTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                getUnitPolyPolygon(),
                rTransform * getObjectTransform());
        }

        // provide unique ID
        sal_uInt32 SdrContourTextPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDRCONTOURTEXTPRIMITIVE2D;
        }



        Primitive2DReference SdrPathTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposePathTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(std::move(aRetval));
        }

        SdrPathTextPrimitive2D::SdrPathTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            basegfx::B2DPolyPolygon aPathPolyPolygon,
            attribute::SdrFormTextAttribute aSdrFormTextAttribute)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maPathPolyPolygon(std::move(aPathPolyPolygon)),
            maSdrFormTextAttribute(std::move(aSdrFormTextAttribute))
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

        rtl::Reference<SdrTextPrimitive2D> SdrPathTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            basegfx::B2DPolyPolygon aNewPolyPolygon(getPathPolyPolygon());
            aNewPolyPolygon.transform(rTransform);

            return new SdrPathTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                std::move(aNewPolyPolygon),
                getSdrFormTextAttribute());
        }

        // provide unique ID
        sal_uInt32 SdrPathTextPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDRPATHTEXTPRIMITIVE2D;
        }



        Primitive2DReference SdrBlockTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeBlockTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(std::move(aRetval));
        }

        SdrBlockTextPrimitive2D::SdrBlockTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            basegfx::B2DHomMatrix aTextRangeTransform,
            SdrTextHorzAdjust aSdrTextHorzAdjust,
            SdrTextVertAdjust aSdrTextVertAdjust,
            bool bFixedCellHeight,
            bool bUnlimitedPage,
            bool bCellText,
            bool bWordWrap)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maTextRangeTransform(std::move(aTextRangeTransform)),
            maSdrTextHorzAdjust(aSdrTextHorzAdjust),
            maSdrTextVertAdjust(aSdrTextVertAdjust),
            mbFixedCellHeight(bFixedCellHeight),
            mbUnlimitedPage(bUnlimitedPage),
            mbCellText(bCellText),
            mbWordWrap(bWordWrap)
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
                    && getWordWrap() == rCompare.getWordWrap());
            }

            return false;
        }

        rtl::Reference<SdrTextPrimitive2D> SdrBlockTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrBlockTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                rTransform * getTextRangeTransform(),
                getSdrTextHorzAdjust(),
                getSdrTextVertAdjust(),
                isFixedCellHeight(),
                getUnlimitedPage(),
                getCellText(),
                getWordWrap());
        }

        // provide unique ID
        sal_uInt32 SdrBlockTextPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDRBLOCKTEXTPRIMITIVE2D;
        }



         Primitive2DReference SdrAutoFitTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
         {
             Primitive2DContainer aRetval;
             getSdrText()->GetObject().impDecomposeAutoFitTextPrimitive(aRetval, *this, aViewInformation);

             return encapsulateWithTextHierarchyBlockPrimitive2D(std::move(aRetval));
         }

         SdrAutoFitTextPrimitive2D::SdrAutoFitTextPrimitive2D(
             const SdrText* pSdrText,
             const OutlinerParaObject& rParaObj,
             ::basegfx::B2DHomMatrix aTextRangeTransform,
             bool bWordWrap,
             bool bFixedCellHeight)
         :  SdrTextPrimitive2D(pSdrText, rParaObj),
             maTextRangeTransform(std::move(aTextRangeTransform)),
             mbWordWrap(bWordWrap),
             mbFixedCellHeight(bFixedCellHeight)
         {
         }

         bool SdrAutoFitTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
         {
             if(SdrTextPrimitive2D::operator==(rPrimitive))
             {
                 const SdrBlockTextPrimitive2D& rCompare = static_cast<const SdrBlockTextPrimitive2D&>(rPrimitive);

                 return (getTextRangeTransform() == rCompare.getTextRangeTransform()
                     && getWordWrap() == rCompare.getWordWrap()
                     && isFixedCellHeight() == rCompare.isFixedCellHeight());
             }

             return false;
         }

         rtl::Reference<SdrTextPrimitive2D> SdrAutoFitTextPrimitive2D::createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const
         {
             return new SdrAutoFitTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                rTransform * getTextRangeTransform(),
                getWordWrap(),
                isFixedCellHeight());
         }

        // provide unique ID
        sal_uInt32 SdrAutoFitTextPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDRAUTOFITTEXTPRIMITIVE2D;
        }




        SdrChainedTextPrimitive2D::SdrChainedTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            basegfx::B2DHomMatrix aTextRangeTransform)
        : SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
          maTextRangeTransform(std::move(aTextRangeTransform))
        { }

        Primitive2DReference SdrChainedTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeChainedTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(std::move(aRetval));
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

        rtl::Reference<SdrTextPrimitive2D> SdrChainedTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrChainedTextPrimitive2D(getSdrText(), getOutlinerParaObject(), rTransform * getTextRangeTransform());
        }

        // provide unique ID
        sal_uInt32 SdrChainedTextPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDRCHAINEDTEXTPRIMITIVE2D;
        }


        Primitive2DReference SdrStretchTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            getSdrText()->GetObject().impDecomposeStretchTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(std::move(aRetval));
        }

        SdrStretchTextPrimitive2D::SdrStretchTextPrimitive2D(
            const SdrText* pSdrText,
            const OutlinerParaObject& rOutlinerParaObject,
            basegfx::B2DHomMatrix aTextRangeTransform,
            bool bFixedCellHeight)
        :   SdrTextPrimitive2D(pSdrText, rOutlinerParaObject),
            maTextRangeTransform(std::move(aTextRangeTransform)),
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

        rtl::Reference<SdrTextPrimitive2D> SdrStretchTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrStretchTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                rTransform * getTextRangeTransform(),
                isFixedCellHeight());
        }

        // provide unique ID
        sal_uInt32 SdrStretchTextPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDRSTRETCHTEXTPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
