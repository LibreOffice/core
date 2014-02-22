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

#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <svx/sdr/contact/viewobjectcontactofgraphic.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>

#include <svx/sdgcpitm.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/event/eventhandler.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/primitive2d/sdrgrafprimitive2d.hxx>
#include "svx/svdstr.hrc"
#include <svx/svdglob.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/colritem.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>



namespace sdr
{
    namespace contact
    {
        
        
        ViewObjectContact& ViewContactOfGraphic::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfGraphic(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        ViewContactOfGraphic::ViewContactOfGraphic(SdrGrafObj& rGrafObj)
        :   ViewContactOfTextObj(rGrafObj)
        {
        }

        ViewContactOfGraphic::~ViewContactOfGraphic()
        {
        }

        void ViewContactOfGraphic::flushGraphicObjects()
        {
            
            
            
            
            
            
            flushViewIndependentPrimitive2DSequence();
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfGraphic::createVIP2DSForPresObj(
            const basegfx::B2DHomMatrix& rObjectMatrix,
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute& rAttribute) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            GraphicObject aEmptyGraphicObject;
            GraphicAttr aEmptyGraphicAttr;

            
            const drawinglayer::primitive2d::Primitive2DReference xReferenceA(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                rObjectMatrix,
                rAttribute,
                aEmptyGraphicObject,
                aEmptyGraphicAttr));
            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReferenceA, 1);

            
            
            basegfx::B2DHomMatrix aSmallerMatrix;

            
            
            
            Size aPrefSize(GetGrafObject().GetGrafPrefSize());

            if(MAP_PIXEL == GetGrafObject().GetGrafPrefMapMode().GetMapUnit())
            {
                aPrefSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MAP_100TH_MM);
            }
            else
            {
                aPrefSize = Application::GetDefaultDevice()->LogicToLogic(aPrefSize, GetGrafObject().GetGrafPrefMapMode(), MAP_100TH_MM);
            }

            
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

            const double fOffsetX((aScale.getX() - aPrefSize.getWidth()) / 2.0);
            const double fOffsetY((aScale.getY() - aPrefSize.getHeight()) / 2.0);

            if(basegfx::fTools::moreOrEqual(fOffsetX, 0.0) && basegfx::fTools::moreOrEqual(fOffsetY, 0.0))
            {
                
                
                aSmallerMatrix = basegfx::tools::createScaleTranslateB2DHomMatrix(aPrefSize.getWidth(), aPrefSize.getHeight(), fOffsetX, fOffsetY);
                aSmallerMatrix = basegfx::tools::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                    * aSmallerMatrix;

                const GraphicObject& rGraphicObject = GetGrafObject().GetGraphicObject(false);
                const GraphicAttr aLocalGrafInfo;
                const drawinglayer::primitive2d::Primitive2DReference xReferenceB(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                    aSmallerMatrix,
                    drawinglayer::attribute::SdrLineFillShadowTextAttribute(),
                    rGraphicObject,
                    aLocalGrafInfo));

                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xReferenceB);
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfGraphic::createVIP2DSForDraft(
            const basegfx::B2DHomMatrix& rObjectMatrix,
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute& rAttribute) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            GraphicObject aEmptyGraphicObject;
            GraphicAttr aEmptyGraphicAttr;

            
            const drawinglayer::primitive2d::Primitive2DReference xReferenceA(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                rObjectMatrix,
                rAttribute,
                aEmptyGraphicObject,
                aEmptyGraphicAttr));
            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReferenceA, 1);

            if(rAttribute.getLine().isDefault())
            {
                
                const Color aColor(Application::GetSettings().GetStyleSettings().GetShadowColor());
                const basegfx::BColor aBColor(aColor.getBColor());
                basegfx::B2DPolygon aOutline(basegfx::tools::createUnitPolygon());
                aOutline.transform(rObjectMatrix);

                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval,
                    drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                            aOutline,
                            aBColor)));
            }

            
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

            
            
            const double fDistance(200.0);

            
            aScale.setX(std::max(0.0, aScale.getX() - (2.0 * fDistance)));
            aScale.setY(std::max(0.0, aScale.getY() - (2.0 * fDistance)));
            aTranslate.setX(aTranslate.getX() + fDistance);
            aTranslate.setY(aTranslate.getY() + fDistance);

            
            const Bitmap aDraftBitmap(ResId(BMAP_GrafikEi, *ImpGetResMgr()));

            if(!aDraftBitmap.IsEmpty())
            {
                Size aPrefSize(aDraftBitmap.GetPrefSize());

                if(MAP_PIXEL == aDraftBitmap.GetPrefMapMode().GetMapUnit())
                {
                    aPrefSize = Application::GetDefaultDevice()->PixelToLogic(aDraftBitmap.GetSizePixel(), MAP_100TH_MM);
                }
                else
                {
                    aPrefSize = Application::GetDefaultDevice()->LogicToLogic(aPrefSize, aDraftBitmap.GetPrefMapMode(), MAP_100TH_MM);
                }

                const double fBitmapScaling(2.0);
                const double fWidth(aPrefSize.getWidth() * fBitmapScaling);
                const double fHeight(aPrefSize.getHeight() * fBitmapScaling);

                if(basegfx::fTools::more(fWidth, 1.0)
                    && basegfx::fTools::more(fHeight, 1.0)
                    && basegfx::fTools::lessOrEqual(fWidth, aScale.getX())
                    && basegfx::fTools::lessOrEqual(fHeight, aScale.getY()))
                {
                    const basegfx::B2DHomMatrix aBitmapMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                        fWidth, fHeight, fShearX, fRotate, aTranslate.getX(), aTranslate.getY()));

                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval,
                        drawinglayer::primitive2d::Primitive2DReference(
                            new drawinglayer::primitive2d::BitmapPrimitive2D(
                                BitmapEx(aDraftBitmap),
                                aBitmapMatrix)));

                    
                    aScale.setX(std::max(0.0, aScale.getX() - (fWidth + fDistance)));
                    aTranslate.setX(aTranslate.getX() + fWidth + fDistance);
                }
            }

            
            OUString aDraftText = GetGrafObject().GetFileName();

            if (aDraftText.isEmpty())
            {
                aDraftText = GetGrafObject().GetName();
                aDraftText += " ...";
            }

            if (!aDraftText.isEmpty() && GetGrafObject().GetModel())
            {
                
                
                
                
                
                
                
                

                
                SdrRectObj aRectObj(OBJ_TEXT);
                aRectObj.SetModel(GetGrafObject().GetModel());
                aRectObj.NbcSetText(aDraftText);
                aRectObj.SetMergedItem(SvxColorItem(Color(COL_LIGHTRED), EE_CHAR_COLOR));

                
                SdrText* pSdrText = aRectObj.getText(0);
                OutlinerParaObject* pOPO = aRectObj.GetOutlinerParaObject();

                if(pSdrText && pOPO)
                {
                    
                    const basegfx::B2DHomMatrix aTextRangeTransform(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                        aScale, fShearX, fRotate, aTranslate));

                    
                    drawinglayer::primitive2d::SdrBlockTextPrimitive2D aBlockTextPrimitive(
                        pSdrText,
                        *pOPO,
                        aTextRangeTransform,
                        SDRTEXTHORZADJUST_LEFT,
                        SDRTEXTVERTADJUST_TOP,
                        false,
                        false,
                        false,
                        false,
                        false);

                    
                    
                    const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(
                        xRetval,
                        aBlockTextPrimitive.get2DDecomposition(aViewInformation2D));
                }
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfGraphic::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            const SfxItemSet& rItemSet = GetGrafObject().GetMergedItemSet();

            
            GraphicAttr aLocalGrafInfo;
            const sal_uInt16 nTrans(((SdrGrafTransparenceItem&)rItemSet.Get(SDRATTR_GRAFTRANSPARENCE)).GetValue());
            const SdrGrafCropItem& rCrop((const SdrGrafCropItem&)rItemSet.Get(SDRATTR_GRAFCROP));
            aLocalGrafInfo.SetLuminance(((SdrGrafLuminanceItem&)rItemSet.Get(SDRATTR_GRAFLUMINANCE)).GetValue());
            aLocalGrafInfo.SetContrast(((SdrGrafContrastItem&)rItemSet.Get(SDRATTR_GRAFCONTRAST)).GetValue());
            aLocalGrafInfo.SetChannelR(((SdrGrafRedItem&)rItemSet.Get(SDRATTR_GRAFRED)).GetValue());
            aLocalGrafInfo.SetChannelG(((SdrGrafGreenItem&)rItemSet.Get(SDRATTR_GRAFGREEN)).GetValue());
            aLocalGrafInfo.SetChannelB(((SdrGrafBlueItem&)rItemSet.Get(SDRATTR_GRAFBLUE)).GetValue());
            aLocalGrafInfo.SetGamma(((SdrGrafGamma100Item&)rItemSet.Get(SDRATTR_GRAFGAMMA)).GetValue() * 0.01);
            aLocalGrafInfo.SetTransparency((sal_uInt8)::basegfx::fround(std::min(nTrans, (sal_uInt16)100) * 2.55));
            aLocalGrafInfo.SetInvert(((SdrGrafInvertItem&)rItemSet.Get(SDRATTR_GRAFINVERT)).GetValue());
            aLocalGrafInfo.SetDrawMode(((SdrGrafModeItem&)rItemSet.Get(SDRATTR_GRAFMODE)).GetValue());
            aLocalGrafInfo.SetCrop(rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom());

            
            const bool bHasContent(255L != aLocalGrafInfo.GetTransparency());
            drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetGrafObject().getText(0),
                    bHasContent));

            
            
            Rectangle rRectangle = GetGrafObject().GetGeoRect();
            
            
            
            rRectangle += GetGrafObject().GetGridOffset();
            const ::basegfx::B2DRange aObjectRange(
                rRectangle.Left(), rRectangle.Top(),
                rRectangle.Right(), rRectangle.Bottom());

            
            const GeoStat& rGeoStat(GetGrafObject().GetGeoStat());
            const sal_Int32 nDrehWink(rGeoStat.nDrehWink);
            const bool bRota180(18000 == nDrehWink);
            const bool bMirrored(GetGrafObject().IsMirrored());
            const sal_uInt16 nMirrorCase(bRota180 ? (bMirrored ? 3 : 4) : (bMirrored ? 2 : 1));
            bool bHMirr((2 == nMirrorCase ) || (4 == nMirrorCase));
            bool bVMirr((3 == nMirrorCase ) || (4 == nMirrorCase));

            
            
            
            
            if(bRota180)
            {
                
                
                bHMirr = !bHMirr;
                bVMirr = false;
            }

            if(bHMirr || bVMirr)
            {
                aLocalGrafInfo.SetMirrorFlags((bHMirr ? BMP_MIRROR_HORZ : 0)|(bVMirr ? BMP_MIRROR_VERT : 0));
            }

            
            const double fShearX(rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0);
            const double fRotate(nDrehWink ? (36000 - nDrehWink) * F_PI18000 : 0.0);
            const basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                aObjectRange.getWidth(), aObjectRange.getHeight(),
                fShearX, fRotate,
                aObjectRange.getMinX(), aObjectRange.getMinY()));

            
            const GraphicObject& rGraphicObject = GetGrafObject().GetGraphicObject(false);

            if(visualisationUsesPresObj())
            {
                
                
                xRetval = createVIP2DSForPresObj(aObjectMatrix, aAttribute);
            }
            else if(visualisationUsesDraft())
            {
                
                
                
                
                
                xRetval = createVIP2DSForDraft(aObjectMatrix, aAttribute);
            }
            else
            {
                
                
                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                        aObjectMatrix,
                        aAttribute,
                        rGraphicObject,
                        aLocalGrafInfo));

                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }

            
            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval,
                drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                    false, aObjectMatrix));

            return xRetval;
        }

        bool ViewContactOfGraphic::visualisationUsesPresObj() const
        {
            return GetGrafObject().IsEmptyPresObj();
        }

        bool ViewContactOfGraphic::visualisationUsesDraft() const
        {
            
            if(visualisationUsesPresObj())
                return false;

            
            const GraphicObject& rGraphicObject = GetGrafObject().GetGraphicObject(false);
            static bool bAllowReplacements(true);

            if(rGraphicObject.IsSwappedOut() && bAllowReplacements)
                return true;

            
            if(GRAPHIC_NONE == rGraphicObject.GetType() || GRAPHIC_DEFAULT == rGraphicObject.GetType())
               return true;

            return false;
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
