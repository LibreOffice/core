/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
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

        if(pPage && pPage->GetModel())
        {
            if( (pPage->GetPageNum() == 0) && !pPage->IsMasterPage() )
            {
                
                return pPage->GetModel()->getHandoutPageCount();
            }
            else
            {
                const sal_uInt16 nPageCount(pPage->GetModel()->GetPageCount());
                nRetval = ((sal_Int16)nPageCount - 1) / 2;
            }
        }

        return nRetval;
    }
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        
        
        
        Primitive2DSequence SdrTextPrimitive2D::encapsulateWithTextHierarchyBlockPrimitive2D(const Primitive2DSequence& rCandidate) const
        {
            Primitive2DReference xReference(new TextHierarchyBlockPrimitive2D(rCandidate));
            Primitive2DSequence xRetval(&xReference, 1);

            return xRetval;
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

            mbContainsPageField = rETO.HasField(SvxPageField::StaticClassId());
            mbContainsPageCountField = rETO.HasField(SvxPagesField::StaticClassId());
            mbContainsOtherFields = rETO.HasField(SvxHeaderField::StaticClassId())
                || rETO.HasField(SvxFooterField::StaticClassId())
                || rETO.HasField(SvxDateTimeField::StaticClassId())
                || rETO.HasField(SvxAuthorField::StaticClassId());
        }

        bool SdrTextPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrTextPrimitive2D& rCompare = (SdrTextPrimitive2D&)rPrimitive;

                return (

                    
                    getOutlinerParaObject() == rCompare.getOutlinerParaObject()

                    
                    && getOutlinerParaObject().isWrongListEqual(rCompare.getOutlinerParaObject()));
            }

            return false;
        }

        Primitive2DSequence SdrTextPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            uno::Reference< drawing::XDrawPage > xCurrentlyVisualizingPage;
            bool bCurrentlyVisualizingPageIsSet(false);
            Color aNewTextBackgroundColor;
            bool bNewTextBackgroundColorIsSet(false);
            sal_Int16 nCurrentlyValidPageNumber(0);
            sal_Int16 nCurrentlyValidPageCount(0);

            if(getBuffered2DDecomposition().hasElements())
            {
                bool bDoDelete(false);

                
                if(mbContainsPageField || mbContainsPageCountField || mbContainsOtherFields)
                {
                    
                    xCurrentlyVisualizingPage = rViewInformation.getVisualizedPage();
                    bCurrentlyVisualizingPageIsSet = true;

                    if(xCurrentlyVisualizingPage != mxLastVisualizingPage)
                    {
                        bDoDelete = true;
                    }

                    
                    if(!bDoDelete && mbContainsPageField)
                    {
                        nCurrentlyValidPageNumber = getPageNumber(xCurrentlyVisualizingPage);

                        if(nCurrentlyValidPageNumber != mnLastPageNumber)
                        {
                            bDoDelete = true;
                        }
                    }

                    
                    if(!bDoDelete && mbContainsPageCountField)
                    {
                        nCurrentlyValidPageCount = getPageCount(xCurrentlyVisualizingPage);

                        if(nCurrentlyValidPageCount != mnLastPageCount)
                        {
                            bDoDelete = true;
                        }
                    }
                }

                
                if(!bDoDelete && getSdrText() && getSdrText()->GetModel())
                {
                    SdrOutliner& rDrawOutliner = getSdrText()->GetModel()->GetDrawOutliner(0);
                    aNewTextBackgroundColor = rDrawOutliner.GetBackgroundColor();
                    bNewTextBackgroundColorIsSet = true;

                    if(aNewTextBackgroundColor != maLastTextBackgroundColor)
                    {
                        bDoDelete = true;
                    }
                }

                if(bDoDelete)
                {
                    const_cast< SdrTextPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
                }
            }

            if(!getBuffered2DDecomposition().hasElements())
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

                if(!bNewTextBackgroundColorIsSet && getSdrText() && getSdrText()->GetModel())
                {
                    SdrOutliner& rDrawOutliner = getSdrText()->GetModel()->GetDrawOutliner(0);
                    aNewTextBackgroundColor = rDrawOutliner.GetBackgroundColor();
                }

                const_cast< SdrTextPrimitive2D* >(this)->mxLastVisualizingPage = xCurrentlyVisualizingPage;
                const_cast< SdrTextPrimitive2D* >(this)->mnLastPageNumber = nCurrentlyValidPageNumber;
                const_cast< SdrTextPrimitive2D* >(this)->mnLastPageCount = nCurrentlyValidPageCount;
                const_cast< SdrTextPrimitive2D* >(this)->maLastTextBackgroundColor = aNewTextBackgroundColor;
            }

            
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }
    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrContourTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            getSdrText()->GetObject().impDecomposeContourTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
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
                const SdrContourTextPrimitive2D& rCompare = (SdrContourTextPrimitive2D&)rPrimitive;

                return (getUnitPolyPolygon() == rCompare.getUnitPolyPolygon()
                    && getObjectTransform() == rCompare.getObjectTransform());
            }

            return false;
        }

        SdrTextPrimitive2D* SdrContourTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrContourTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                getUnitPolyPolygon(),
                rTransform * getObjectTransform());
        }

        
        ImplPrimitive2DIDBlock(SdrContourTextPrimitive2D, PRIMITIVE2D_ID_SDRCONTOURTEXTPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrPathTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            getSdrText()->GetObject().impDecomposePathTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
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
                const SdrPathTextPrimitive2D& rCompare = (SdrPathTextPrimitive2D&)rPrimitive;

                return (getPathPolyPolygon() == rCompare.getPathPolyPolygon()
                    && getSdrFormTextAttribute() == rCompare.getSdrFormTextAttribute());
            }

            return false;
        }

        SdrTextPrimitive2D* SdrPathTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            basegfx::B2DPolyPolygon aNewPolyPolygon(getPathPolyPolygon());
            aNewPolyPolygon.transform(rTransform);

            return new SdrPathTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                aNewPolyPolygon,
                getSdrFormTextAttribute());
        }

        
        ImplPrimitive2DIDBlock(SdrPathTextPrimitive2D, PRIMITIVE2D_ID_SDRPATHTEXTPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrBlockTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            getSdrText()->GetObject().impDecomposeBlockTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
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
                const SdrBlockTextPrimitive2D& rCompare = (SdrBlockTextPrimitive2D&)rPrimitive;

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

        SdrTextPrimitive2D* SdrBlockTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
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
                getWordWrap(),
                getClipOnBounds());
        }

        
        ImplPrimitive2DIDBlock(SdrBlockTextPrimitive2D, PRIMITIVE2D_ID_SDRBLOCKTEXTPRIMITIVE2D)

    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
         Primitive2DSequence SdrAutoFitTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
         {
             Primitive2DSequence aRetval;
             getSdrText()->GetObject().impDecomposeAutoFitTextPrimitive(aRetval, *this, aViewInformation);

             return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
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
                 const SdrBlockTextPrimitive2D& rCompare = (SdrBlockTextPrimitive2D&)rPrimitive;

                 return (getTextRangeTransform() == rCompare.getTextRangeTransform()
                     && getWordWrap() == rCompare.getWordWrap());
             }

             return false;
         }

         SdrTextPrimitive2D* SdrAutoFitTextPrimitive2D::createTransformedClone(const ::basegfx::B2DHomMatrix& rTransform) const
         {
             return new SdrAutoFitTextPrimitive2D(getSdrText(), getOutlinerParaObject(), rTransform * getTextRangeTransform(), getWordWrap());
         }

         
         ImplPrimitive2DIDBlock(SdrAutoFitTextPrimitive2D, PRIMITIVE2D_ID_SDRAUTOFITTEXTPRIMITIVE2D)

     } 
 } 

 

 namespace drawinglayer
 {
     namespace primitive2d
     {
        Primitive2DSequence SdrStretchTextPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            getSdrText()->GetObject().impDecomposeStretchTextPrimitive(aRetval, *this, aViewInformation);

            return encapsulateWithTextHierarchyBlockPrimitive2D(aRetval);
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
                const SdrStretchTextPrimitive2D& rCompare = (SdrStretchTextPrimitive2D&)rPrimitive;

                return (getTextRangeTransform() == rCompare.getTextRangeTransform()
                    && isFixedCellHeight() == rCompare.isFixedCellHeight());
            }

            return false;
        }

        SdrTextPrimitive2D* SdrStretchTextPrimitive2D::createTransformedClone(const basegfx::B2DHomMatrix& rTransform) const
        {
            return new SdrStretchTextPrimitive2D(
                getSdrText(),
                getOutlinerParaObject(),
                rTransform * getTextRangeTransform(),
                isFixedCellHeight());
        }

        
        ImplPrimitive2DIDBlock(SdrStretchTextPrimitive2D, PRIMITIVE2D_ID_SDRSTRETCHTEXTPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
