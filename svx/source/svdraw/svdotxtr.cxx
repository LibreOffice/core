/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdotext.hxx>
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>   // fuer Convert
#include <svx/svdmodel.hxx>  // fuer Convert
#include <editeng/outliner.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svl/itemset.hxx>
#include <svx/svditer.hxx>
#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrTextObj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    // Adapt Width and Height only when text frame (not object with text).
    // Also do not do it in edit mode, let the object get as small as the
    // minimum frame width/height without changing these.
    const bool bTextAdaption(bTextFrame && !IsPasteResize() && !IsInEditMode());
    basegfx::B2DVector aOldSize;

    if(bTextAdaption)
    {
        aOldSize = basegfx::absolute(getSdrObjectScale());
    }

    // call parent
    SdrAttrObj::setSdrObjectTransformation(rTransformation);

    // TTTT: check if the rebuild works
    //
    //sal_Int32 nHDist=GetTextLeftDistance()+GetTextRightDistance();
    //sal_Int32 nVDist=GetTextUpperDistance()+GetTextLowerDistance();
    //sal_Int32 nTWdt0=aRect.GetWidth ()-1-nHDist; if (nTWdt0<0) nTWdt0=0;
    //sal_Int32 nTHgt0=aRect.GetHeight()-1-nVDist; if (nTHgt0<0) nTHgt0=0;
    //sal_Int32 nTWdt1=rRect.GetWidth ()-1-nHDist; if (nTWdt1<0) nTWdt1=0;
    //sal_Int32 nTHgt1=rRect.GetHeight()-1-nVDist; if (nTHgt1<0) nTHgt1=0;
    //  if (nTWdt0!=nTWdt1 && IsAutoGrowWidth() ) SetMinTextFrameWidth(nTWdt1);
    //  if (nTHgt0!=nTHgt1 && IsAutoGrowHeight()) SetMinTextFrameHeight(nTHgt1);
    //  if (GetFitToSize()==SDRTEXTFIT_RESIZEATTR) {
    //      ResizeTextAttributes(Fraction(nTWdt1,nTWdt0),Fraction(nTHgt1,nTHgt0));
    //  }
    //  AdjustTextFrameWidthAndHeight();

    if(bTextAdaption)
    {
        basegfx::B2DVector aNewSize(basegfx::absolute(getSdrObjectScale()));

        if(!aNewSize.equal(aOldSize))
        {
            const basegfx::B2DVector aBorders(
                GetTextLeftDistance() + GetTextRightDistance(),
                GetTextUpperDistance() + GetTextLowerDistance());

            aNewSize -= aBorders;
            aOldSize -= aBorders;

            if(IsAutoGrowWidth() && !basegfx::fTools::equal(aOldSize.getX(), aNewSize.getX()))
            {
                SetMinTextFrameWidth(aNewSize.getX());
            }

            if(IsAutoGrowHeight() && !basegfx::fTools::equal(aOldSize.getY(), aNewSize.getY()))
            {
                SetMinTextFrameHeight(aNewSize.getY());
            }

            if(SDRTEXTFIT_RESIZEATTR == GetFitToSize())
            {
                const double fFactorX(aNewSize.getX() / (basegfx::fTools::equalZero(aOldSize.getX()) ? 1.0 : aOldSize.getX()));
                const double fFactorY(aNewSize.getY() / (basegfx::fTools::equalZero(aOldSize.getY()) ? 1.0 : aOldSize.getY()));

                ResizeTextAttributes(fFactorX, fFactorY);
            }

            AdjustTextFrameWidthAndHeight();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdrObject* SdrTextObj::ImpConvertContainedTextToSdrPathObjs(bool bToPoly) const
{
    SdrObject* pRetval = 0;

    if(!ImpCanConvTextToCurve())
    {
        // suppress HelpTexts from PresObj's
        return 0;
    }

    // get primitives
    const drawinglayer::primitive2d::Primitive2DSequence xSequence(GetViewContact().getViewIndependentPrimitive2DSequence());

    if(xSequence.hasElements())
    {
        // create an extractor with neutral ViewInformation
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
        drawinglayer::processor2d::TextAsPolygonExtractor2D aExtractor(aViewInformation2D);

        // extract text as polygons
        aExtractor.process(xSequence);

        // get results
        const drawinglayer::processor2d::TextAsPolygonDataNodeVector& rResult = aExtractor.getTarget();
        const sal_uInt32 nResultCount(rResult.size());

        if(nResultCount)
        {
            // prepare own target
            SdrObjGroup* pGroup = new SdrObjGroup(getSdrModelFromSdrObject());
            SdrObjList* pObjectList = pGroup->getChildrenOfSdrObject();

            // process results
            for(sal_uInt32 a(0); a < nResultCount; a++)
            {
                const drawinglayer::processor2d::TextAsPolygonDataNode& rCandidate = rResult[a];
                basegfx::B2DPolyPolygon aPolyPolygon(rCandidate.getB2DPolyPolygon());

                if(aPolyPolygon.count())
                {
                    // take care of wanted polygon type
                    if(bToPoly)
                    {
                        if(aPolyPolygon.areControlPointsUsed())
                        {
                            aPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(aPolyPolygon);
                        }
                    }
                    else
                    {
                        if(!aPolyPolygon.areControlPointsUsed())
                        {
                            aPolyPolygon = basegfx::tools::expandToCurve(aPolyPolygon);
                        }
                    }

                    // create ItemSet with object attributes
                    SfxItemSet aAttributeSet(GetObjectItemSet());
                    SdrPathObj* pPathObj = 0;

                    // always clear objectshadow; this is included in the extraction
                    aAttributeSet.Put(SdrOnOffItem(SDRATTR_SHADOW, false));

                    if(rCandidate.getIsFilled())
                    {
                        // set needed items
                        aAttributeSet.Put(XFillColorItem(String(), Color(rCandidate.getBColor())));
                        aAttributeSet.Put(XLineStyleItem(XLINE_NONE));
                        aAttributeSet.Put(XFillStyleItem(XFILL_SOLID));

                        // create filled SdrPathObj
                        pPathObj = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPolygon);
                    }
                    else
                    {
                        // set needed items
                        aAttributeSet.Put(XLineColorItem(String(), Color(rCandidate.getBColor())));
                        aAttributeSet.Put(XLineStyleItem(XLINE_SOLID));
                        aAttributeSet.Put(XLineWidthItem(0));
                        aAttributeSet.Put(XFillStyleItem(XFILL_NONE));

                        // create line SdrPathObj
                        pPathObj = new SdrPathObj(getSdrModelFromSdrObject(), aPolyPolygon);
                    }

                    // copy basic information from original
                    pPathObj->SetAnchorPos(GetAnchorPos());
                    pPathObj->SetLayer(GetLayer());
                    //pPathObj->SetModel(getSdrModelFromSdrObject());
                    pPathObj->SetStyleSheet(GetStyleSheet(), true);

                    // apply prepared ItemSet and add to target
                    pPathObj->SetMergedItemSet(aAttributeSet);
                    pObjectList->InsertObjectToSdrObjList(*pPathObj);
                }
            }

            // postprocess; if no result and/or only one object, simplify
            if(!pObjectList->GetObjCount())
            {
                deleteSdrObjectSafeAndClearPointer(pGroup);
            }
            else if(1 == pObjectList->GetObjCount())
            {
                pRetval = pObjectList->RemoveObjectFromSdrObjList(0);
                deleteSdrObjectSafeAndClearPointer(pGroup);
            }
            else
            {
                pRetval = pGroup;
            }
        }
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////

SdrObject* SdrTextObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    if(bAddText)
    {
        return ImpConvertContainedTextToSdrPathObjs(!bBezier);
    }

    return 0;
}

bool SdrTextObj::ImpCanConvTextToCurve() const
{
    return !IsOutlText();
}

SdrObject* SdrTextObj::ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed, bool bBezier, bool bNoSetAttr) const
{
    basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPolygon);

    // #i37011#
    if(!bBezier)
    {
        aB2DPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(aB2DPolyPolygon);
    }

    SdrPathObj* pPathObj = new SdrPathObj(getSdrModelFromSdrObject(), aB2DPolyPolygon);

    if(bBezier)
    {
        // create bezier curves
        pPathObj->setB2DPolyPolygonInObjectCoordinates(basegfx::tools::expandToCurve(pPathObj->getB2DPolyPolygonInObjectCoordinates()));
    }

    if(pPathObj)
    {
        pPathObj->SetAnchorPos(GetAnchorPos());
        pPathObj->SetLayer(GetLayer());

        if(!bNoSetAttr)
        {
            sdr::properties::ItemChangeBroadcaster aC(*pPathObj);

            pPathObj->ClearMergedItem();
            pPathObj->SetMergedItemSet(GetObjectItemSet());
            pPathObj->SetStyleSheet(GetStyleSheet(), true);
        }
    }

    return pPathObj;
}

SdrObject* SdrTextObj::ImpConvertAddText(SdrObject* pObj, bool bBezier) const
{
    if(!ImpCanConvTextToCurve())
    {
        return pObj;
    }

    SdrObject* pText = ImpConvertContainedTextToSdrPathObjs(!bBezier);

    if(!pText)
    {
        return pObj;
    }

    if(!pObj)
    {
        return pText;
    }

    if(pText->getChildrenOfSdrObject())
    {
        // is already group object, add partial shape in front
        SdrObjList* pOL = pText->getChildrenOfSdrObject();
        pOL->InsertObjectToSdrObjList(*pObj, 0);

        return pText;
    }
    else
    {
        // not yet a group, create one and add partial and new shapes
        SdrObjGroup* pGrp = new SdrObjGroup(getSdrModelFromSdrObject());
        pGrp->InsertObjectToSdrObjList(*pObj);
        pGrp->InsertObjectToSdrObjList(*pText);

        return pGrp;
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
