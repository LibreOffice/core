/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofgraphic.cxx,v $
 * $Revision: 1.14.18.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <svx/sdr/contact/viewobjectcontactofgraphic.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svtools/itemset.hxx>

#ifndef ITEMID_GRF_CROP
#define ITEMID_GRF_CROP 0
#endif

#include <svx/sdgcpitm.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/event/eventhandler.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/primitive2d/sdrgrafprimitive2d.hxx>
#include "svdstr.hrc"
#include <svdglob.hxx>
#include <vcl/svapp.hxx>

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

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfGraphic::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            SdrText* pSdrText = GetGrafObject().getText(0);

            if(pSdrText)
            {
                const SfxItemSet& rItemSet = GetGrafObject().GetMergedItemSet();
                drawinglayer::attribute::SdrLineFillShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(rItemSet, *pSdrText);
                bool bVisible(pAttribute && pAttribute->isVisible());

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
                aLocalGrafInfo.SetTransparency((BYTE)::basegfx::fround(Min(nTrans, (USHORT)100) * 2.55));
                aLocalGrafInfo.SetInvert(((SdrGrafInvertItem&)rItemSet.Get(SDRATTR_GRAFINVERT)).GetValue());
                aLocalGrafInfo.SetDrawMode(((SdrGrafModeItem&)rItemSet.Get(SDRATTR_GRAFMODE)).GetValue());
                aLocalGrafInfo.SetCrop(rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom());

                if(!bVisible && 255L != aLocalGrafInfo.GetTransparency())
                {
                    // content is visible, so force some fill stuff
                    delete pAttribute;
                    bVisible = true;

                    // check shadow
                    drawinglayer::attribute::SdrShadowAttribute* pShadow = drawinglayer::primitive2d::createNewSdrShadowAttribute(rItemSet);

                    if(pShadow && !pShadow->isVisible())
                    {
                        delete pShadow;
                        pShadow = 0L;
                    }

                    // create new attribute set
                    pAttribute = new drawinglayer::attribute::SdrLineFillShadowTextAttribute(0L, 0L, 0L, pShadow, 0L, 0L);
                }

                if(pAttribute)
                {
                    if(pAttribute->isVisible() || bVisible)
                    {
                        // take unrotated snap rect for position and size. Directly use model data, not getBoundRect() or getSnapRect()
                        // which will use the primitive data we just create in the near future
                        const Rectangle& rRectangle = GetGrafObject().GetGeoRect();
                        const ::basegfx::B2DRange aObjectRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());
                        ::basegfx::B2DHomMatrix aObjectMatrix;

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
                        aObjectMatrix.scale(aObjectRange.getWidth(), aObjectRange.getHeight());
                        aObjectMatrix.shearX(fShearX);
                        aObjectMatrix.rotate(fRotate);
                        aObjectMatrix.translate(aObjectRange.getMinX(), aObjectRange.getMinY());

                        // get the current, unchenged graphic obect from SdrGrafObj
                        const GraphicObject& rGraphicObject = GetGrafObject().GetGraphicObject(false);

                        if(GetGrafObject().IsEmptyPresObj())
                        {
                            // it's an EmptyPresObj, create the SdrGrafPrimitive2D without content and another scaled one
                            // with the content which is the placeholder graphic
                            GraphicObject aEmptyGraphicObject;
                            GraphicAttr aEmptyGraphicAttr;
                            drawinglayer::attribute::SdrLineFillShadowTextAttribute aEmptyAttributes(0, 0, 0, 0, 0, 0);

                            // SdrGrafPrimitive2D without content in original size which carries all eventual attributes and texts
                            const drawinglayer::primitive2d::Primitive2DReference xReferenceA(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                                aObjectMatrix, *pAttribute, aEmptyGraphicObject, aEmptyGraphicAttr));
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

                            const double fOffsetX((aObjectRange.getWidth() - aPrefSize.getWidth()) / 2.0);
                            const double fOffsetY((aObjectRange.getHeight() - aPrefSize.getHeight()) / 2.0);

                            if(basegfx::fTools::moreOrEqual(fOffsetX, 0.0) && basegfx::fTools::moreOrEqual(fOffsetY, 0.0))
                            {
                                aSmallerMatrix.scale(aPrefSize.getWidth(), aPrefSize.getHeight());
                                aSmallerMatrix.translate(fOffsetX, fOffsetY);
                                aSmallerMatrix.shearX(fShearX);
                                aSmallerMatrix.rotate(fRotate);
                                aSmallerMatrix.translate(aObjectRange.getMinX(), aObjectRange.getMinY());

                                const drawinglayer::primitive2d::Primitive2DReference xReferenceB(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                                    aSmallerMatrix,
                                    aEmptyAttributes,
                                    rGraphicObject,
                                    aLocalGrafInfo));

                                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xReferenceB);
                            }
                        }
                        else
                        {
                            // create primitive
                            const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrGrafPrimitive2D(
                                aObjectMatrix,
                                *pAttribute,
                                rGraphicObject,
                                aLocalGrafInfo));

                            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                        }
                    }

                    delete pAttribute;
                }
            }

            return xRetval;
        }

    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
