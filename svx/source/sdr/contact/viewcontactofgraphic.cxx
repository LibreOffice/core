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

#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <svx/sdr/contact/viewobjectcontactofgraphic.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>

#ifndef ITEMID_GRF_CROP
#define ITEMID_GRF_CROP 0
#endif

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

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
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
            // #i102380# The graphic is swapped out. To let that have an effect ist is necessary to
            // delete copies of the GraphicObject which are not swapped out and have no SwapHandler set
            // (this is what happnes when the GraphicObject gets copied to a SdrGrafPrimitive2D). This
            // is best achieved for the VC by clearing the local decomposition cache. It would be possible
            // to also do this for the VOC cache, but that VOCs exist exactly expresss that the object
            // gets visualised, so this would be wrong.
            flushViewIndependentPrimitive2DSequence();
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfGraphic::createVIP2DSForPresObj(
            const basegfx::B2DHomMatrix& rObjectMatrix,
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute& rAttribute,
            const GraphicAttr& rLocalGrafInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            GraphicObject aEmptyGraphicObject;
            GraphicAttr aEmptyGraphicAttr;

            // SdrGrafPrimitive2D without content in original size which carries all eventual attributes and texts
            const drawinglayer::primitive2d::Primitive2DReference xReferenceA(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                rObjectMatrix,
                rAttribute,
                aEmptyGraphicObject,
                aEmptyGraphicAttr));
            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReferenceA, 1);

            // SdrGrafPrimitive2D with content (which is the preview graphic) scaled to smaller size and
            // without attributes
            basegfx::B2DHomMatrix aSmallerMatrix;

            // #i94431# for some reason, i forgot to take the PrefMapMode of the graphic
            // into account. Since EmptyPresObj's are only used in Draw/Impress, it is
            // safe to assume 100th mm as target.
            Size aPrefSize(GetGrafObject().GetGrafPrefSize());

            if(MAP_PIXEL == GetGrafObject().GetGrafPrefMapMode().GetMapUnit())
            {
                aPrefSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MAP_100TH_MM);
            }
            else
            {
                aPrefSize = Application::GetDefaultDevice()->LogicToLogic(aPrefSize, GetGrafObject().GetGrafPrefMapMode(), MAP_100TH_MM);
            }

            // decompose object matrix to get single values
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

            const double fOffsetX((aScale.getX() - aPrefSize.getWidth()) / 2.0);
            const double fOffsetY((aScale.getY() - aPrefSize.getHeight()) / 2.0);

            if(basegfx::fTools::moreOrEqual(fOffsetX, 0.0) && basegfx::fTools::moreOrEqual(fOffsetY, 0.0))
            {
                // create the EmptyPresObj fallback visualisation. The fallback graphic
                // is already provided in rGraphicObject in this case, use it
                aSmallerMatrix = basegfx::tools::createScaleTranslateB2DHomMatrix(aPrefSize.getWidth(), aPrefSize.getHeight(), fOffsetX, fOffsetY);
                aSmallerMatrix = basegfx::tools::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                    * aSmallerMatrix;

                const GraphicObject& rGraphicObject = GetGrafObject().GetGraphicObject(false);
                const drawinglayer::primitive2d::Primitive2DReference xReferenceB(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                    aSmallerMatrix,
                    drawinglayer::attribute::SdrLineFillShadowTextAttribute(),
                    rGraphicObject,
                    rLocalGrafInfo));

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

            // SdrGrafPrimitive2D without content in original size which carries all eventual attributes and texts
            const drawinglayer::primitive2d::Primitive2DReference xReferenceA(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                rObjectMatrix,
                rAttribute,
                aEmptyGraphicObject,
                aEmptyGraphicAttr));
            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReferenceA, 1);

            if(rAttribute.getLine().isDefault())
            {
                // create a surrounding frame when no linestyle given
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

            // decompose object matrix to get single values
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

            // define a distance value, used for distance from bitmap to borders and from bitmap
            // to text, too (2 mm)
            const double fDistance(200.0);

            // consume borders from values
            aScale.setX(std::max(0.0, aScale.getX() - (2.0 * fDistance)));
            aScale.setY(std::max(0.0, aScale.getY() - (2.0 * fDistance)));
            aTranslate.setX(aTranslate.getX() + fDistance);
            aTranslate.setY(aTranslate.getY() + fDistance);

            // draw a draft bitmap
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

                    // consume bitmap size in X
                    aScale.setX(std::max(0.0, aScale.getX() - (fWidth + fDistance)));
                    aTranslate.setX(aTranslate.getX() + fWidth + fDistance);
                }
            }

            // Build the text for the draft object
            XubString aDraftText = GetGrafObject().GetFileName();

            if(!aDraftText.Len())
            {
                aDraftText = GetGrafObject().GetName();
                aDraftText.AppendAscii(" ...");
            }

            if(aDraftText.Len() && GetGrafObject().GetModel())
            {
                // #i103255# Goal is to produce TextPrimitives which hold the given text as
                // BlockText in the available space. It would be very tricky to do
                // an own word wrap/line layout here.
                // Using SdrBlockTextPrimitive2D OTOH is critical since it internally
                // uses the SdrObject it references. To solve this, create a temp
                // SdrObject with Attributes and Text, generate a SdrBlockTextPrimitive2D
                // directly and immediately decompose it. After that, it is no longer
                // needed and can be deleted.

                // create temp RectObj as TextObj and set needed attributes
                SdrRectObj aRectObj(OBJ_TEXT);
                aRectObj.SetModel(GetGrafObject().GetModel());
                aRectObj.NbcSetText(aDraftText);
                aRectObj.SetMergedItem(SvxColorItem(Color(COL_LIGHTRED), EE_CHAR_COLOR));

                // get SdrText and OPO
                SdrText* pSdrText = aRectObj.getText(0);
                OutlinerParaObject* pOPO = aRectObj.GetOutlinerParaObject();

                if(pSdrText && pOPO)
                {
                    // directly use the remaining space as TextRangeTransform
                    const basegfx::B2DHomMatrix aTextRangeTransform(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                        aScale, fShearX, fRotate, aTranslate));

                    // directly create temp SdrBlockTextPrimitive2D
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

                    // decompose immediately with neutral ViewInformation. This will
                    // layout the text to more simple TextPrimitives from drawinglayer
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
            drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetGrafObject().getText(0)));

            // create and fill GraphicAttr
            GraphicAttr aLocalGrafInfo;
            const sal_uInt16 nTrans(((SdrGrafTransparenceItem&)rItemSet.Get(SDRATTR_GRAFTRANSPARENCE)).GetValue());
            const SdrGrafCropItem& rCrop((const SdrGrafCropItem&)rItemSet.Get(SDRATTR_GRAFCROP));
            aLocalGrafInfo.SetLuminance(((SdrGrafLuminanceItem&)rItemSet.Get(SDRATTR_GRAFLUMINANCE)).GetValue());
            aLocalGrafInfo.SetContrast(((SdrGrafContrastItem&)rItemSet.Get(SDRATTR_GRAFCONTRAST)).GetValue());
            aLocalGrafInfo.SetChannelR(((SdrGrafRedItem&)rItemSet.Get(SDRATTR_GRAFRED)).GetValue());
            aLocalGrafInfo.SetChannelG(((SdrGrafGreenItem&)rItemSet.Get(SDRATTR_GRAFGREEN)).GetValue());
            aLocalGrafInfo.SetChannelB(((SdrGrafBlueItem&)rItemSet.Get(SDRATTR_GRAFBLUE)).GetValue());
            aLocalGrafInfo.SetGamma(((SdrGrafGamma100Item&)rItemSet.Get(SDRATTR_GRAFGAMMA)).GetValue() * 0.01);
            aLocalGrafInfo.SetTransparency((sal_uInt8)::basegfx::fround(Min(nTrans, (sal_uInt16)100) * 2.55));
            aLocalGrafInfo.SetInvert(((SdrGrafInvertItem&)rItemSet.Get(SDRATTR_GRAFINVERT)).GetValue());
            aLocalGrafInfo.SetDrawMode(((SdrGrafModeItem&)rItemSet.Get(SDRATTR_GRAFMODE)).GetValue());
            aLocalGrafInfo.SetCrop(rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom());

            if(aAttribute.isDefault() && 255L != aLocalGrafInfo.GetTransparency())
            {
                // no fill, no line, no text (invisible), but the graphic content is visible.
                // Create evtl. shadow for content which was not created by createNewSdrLineFillShadowTextAttribute yet
                const drawinglayer::attribute::SdrShadowAttribute aShadow(
                    drawinglayer::primitive2d::createNewSdrShadowAttribute(rItemSet));

                if(!aShadow.isDefault())
                {
                    // create new attribute set if indeed shadow is used
                    aAttribute = drawinglayer::attribute::SdrLineFillShadowTextAttribute(
                        aAttribute.getLine(),
                        aAttribute.getFill(),
                        aAttribute.getLineStartEnd(),
                        aShadow,
                        aAttribute.getFillFloatTransGradient(),
                        aAttribute.getText());
                }
            }

            // take unrotated snap rect for position and size. Directly use model data, not getBoundRect() or getSnapRect()
            // which will use the primitive data we just create in the near future
            const Rectangle& rRectangle = GetGrafObject().GetGeoRect();
            const ::basegfx::B2DRange aObjectRange(
                rRectangle.Left(), rRectangle.Top(),
                rRectangle.Right(), rRectangle.Bottom());

            // look for mirroring
            const GeoStat& rGeoStat(GetGrafObject().GetGeoStat());
            const sal_Int32 nDrehWink(rGeoStat.nDrehWink);
            const bool bRota180(18000 == nDrehWink);
            const bool bMirrored(GetGrafObject().IsMirrored());
            const sal_uInt16 nMirrorCase(bRota180 ? (bMirrored ? 3 : 4) : (bMirrored ? 2 : 1));
            bool bHMirr((2 == nMirrorCase ) || (4 == nMirrorCase));
            bool bVMirr((3 == nMirrorCase ) || (4 == nMirrorCase));

            // set mirror flags at LocalGrafInfo. Take into account that the geometry in
            // aObjectRange is already changed and rotated when bRota180 is used. To rebuild
            // that old behaviour (as long as part of the model data), correct the H/V flags
            // accordingly. The created bitmapPrimitive WILL use the rotation, too.
            if(bRota180)
            {
                // if bRota180 which is used for vertical mirroring, the graphic will already be rotated
                // by 180 degrees. To correct, switch off VMirror and invert HMirroring.
                bHMirr = !bHMirr;
                bVMirr = false;
            }

            if(bHMirr || bVMirr)
            {
                aLocalGrafInfo.SetMirrorFlags((bHMirr ? BMP_MIRROR_HORZ : 0)|(bVMirr ? BMP_MIRROR_VERT : 0));
            }

            // fill object matrix
            const double fShearX(rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0);
            const double fRotate(nDrehWink ? (36000 - nDrehWink) * F_PI18000 : 0.0);
            const basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                aObjectRange.getWidth(), aObjectRange.getHeight(),
                fShearX, fRotate,
                aObjectRange.getMinX(), aObjectRange.getMinY()));

            // get the current, unchenged graphic obect from SdrGrafObj
            const GraphicObject& rGraphicObject = GetGrafObject().GetGraphicObject(false);

            if(visualisationUsesPresObj())
            {
                // it's an EmptyPresObj, create the SdrGrafPrimitive2D without content and another scaled one
                // with the content which is the placeholder graphic
                xRetval = createVIP2DSForPresObj(aObjectMatrix, aAttribute, aLocalGrafInfo);
            }
            else if(visualisationUsesDraft())
            {
                // #i102380# The graphic is swapped out. To not force a swap-in here, there is a mechanism
                // which shows a swapped-out-visualisation (which gets created here now) and an asynchronious
                // visual update mechanism for swapped-out grapgics when they were loaded (see AsynchGraphicLoadingEvent
                // and ViewObjectContactOfGraphic implementation). Not forcing the swap-in here allows faster
                // (non-blocking) processing here and thus in the effect e.g. fast scrolling through pages
                xRetval = createVIP2DSForDraft(aObjectMatrix, aAttribute);
            }
            else
            {
                // create primitive. Info: Calling the copy-constructor of GraphicObject in this
                // SdrGrafPrimitive2D constructor will force a full swap-in of the graphic
                const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                    aObjectMatrix,
                    aAttribute,
                    rGraphicObject,
                    aLocalGrafInfo));

                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
            }

            // always append an invisible outline for the cases where no visible content exists
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
            // no draft when already PresObj
            if(visualisationUsesPresObj())
                return false;

            // draft when swapped out
            const GraphicObject& rGraphicObject = GetGrafObject().GetGraphicObject(false);
            static bool bAllowReplacements(true);

            if(rGraphicObject.IsSwappedOut() && bAllowReplacements)
                return true;

            // draft when no graphic
            if(GRAPHIC_NONE == rGraphicObject.GetType() || GRAPHIC_DEFAULT == rGraphicObject.GetType())
               return true;

            return false;
        }

    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
