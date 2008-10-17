/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofsdrole2obj.cxx,v $
 *
 * $Revision: 1.2.18.1 $
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

#include <svx/sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <svx/sdr/contact/viewobjectcontactofsdrole2obj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/primitive2d/sdrole2primitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <vcl/svapp.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfSdrOle2Obj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfSdrOle2Obj(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContact::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        ViewContactOfSdrOle2Obj::ViewContactOfSdrOle2Obj(SdrOle2Obj& rOle2Obj)
        :   ViewContactOfSdrRectObj(rOle2Obj)
        {
        }

        ViewContactOfSdrOle2Obj::~ViewContactOfSdrOle2Obj()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithGivenGraphic(const Graphic& rOLEGraphic, bool bScaleContent) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;
            SdrText* pSdrText = GetOle2Obj().getText(0);

            if(pSdrText)
            {
                // take unrotated snap rect (direct model data) for position and size
                const Rectangle& rRectangle = GetOle2Obj().GetGeoRect();
                const basegfx::B2DRange aObjectRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

                // create object matrix
                const GeoStat& rGeoStat(GetOle2Obj().GetGeoStat());
                const double fShearX(rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0);
                const double fRotate(rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0);
                basegfx::B2DHomMatrix aObjectMatrix;

                aObjectMatrix.scale(aObjectRange.getWidth(), aObjectRange.getHeight());
                aObjectMatrix.shearX(fShearX);
                aObjectMatrix.rotate(fRotate);
                aObjectMatrix.translate(aObjectRange.getMinX(), aObjectRange.getMinY());

                // Prepare attribute settings, will be used soon anyways
                const SfxItemSet& rItemSet = GetOle2Obj().GetMergedItemSet();
                drawinglayer::attribute::SdrLineFillShadowTextAttribute* pAttribute = drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(rItemSet, *pSdrText);

                if(!pAttribute)
                {
                    // force existence, even when not visible
                    pAttribute = new drawinglayer::attribute::SdrLineFillShadowTextAttribute(0, 0, 0, 0, 0, 0);
                }

                // Prepare OLE filling. This is normally the metafile describing OLE content, but may also
                // be the empty OLE bitmap for empty/not loaded OLEs
                const GraphicObject aGraphicObject(rOLEGraphic);
                const GraphicAttr aGraphicAttr;
                drawinglayer::primitive2d::Primitive2DSequence xOLEContent;

                if(bScaleContent)
                {
                    // Create outline and placeholder graphic with some scaling
                    // #i94431# for some reason, i forgot to take the PrefMapMode of the graphic
                    // into account. Since EmptyPresObj's are only used in Draw/Impress, it is
                    // safe to assume 100th mm as target.
                    Size aPrefSize(rOLEGraphic.GetPrefSize());

                    if(MAP_PIXEL == rOLEGraphic.GetPrefMapMode().GetMapUnit())
                    {
                        aPrefSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MAP_100TH_MM);
                    }
                    else
                    {
                        aPrefSize = Application::GetDefaultDevice()->LogicToLogic(aPrefSize, rOLEGraphic.GetPrefMapMode(), MAP_100TH_MM);
                    }

                    const double fOffsetX((aObjectRange.getWidth() - aPrefSize.getWidth()) / 2.0);
                    const double fOffsetY((aObjectRange.getHeight() - aPrefSize.getHeight()) / 2.0);

                    if(basegfx::fTools::moreOrEqual(fOffsetX, 0.0) && basegfx::fTools::moreOrEqual(fOffsetY, 0.0))
                    {
                        // if content fits into frame, create it
                        basegfx::B2DHomMatrix aInnerObjectMatrix;

                        aInnerObjectMatrix.scale(aPrefSize.getWidth(), aPrefSize.getHeight());
                        aInnerObjectMatrix.translate(fOffsetX, fOffsetY);
                        aInnerObjectMatrix.shearX(fShearX);
                        aInnerObjectMatrix.rotate(fRotate);
                        aInnerObjectMatrix.translate(aObjectRange.getMinX(), aObjectRange.getMinY());

                        drawinglayer::primitive2d::Primitive2DReference xScaledContent(
                            new drawinglayer::primitive2d::GraphicPrimitive2D(aInnerObjectMatrix, aGraphicObject, aGraphicAttr));
                        xOLEContent = drawinglayer::primitive2d::Primitive2DSequence(&xScaledContent, 1);
                    }
                }
                else
                {
                    // create graphic primitive for content
                    drawinglayer::primitive2d::Primitive2DReference xDirectContent(
                        new drawinglayer::primitive2d::GraphicPrimitive2D(aObjectMatrix, aGraphicObject, aGraphicAttr));
                    xOLEContent = drawinglayer::primitive2d::Primitive2DSequence(&xDirectContent, 1);
                }

                // create primitive. Use Ole2 primitive here. Prepare attribute settings, will be used soon anyways.
                const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::SdrOle2Primitive2D(
                    xOLEContent,
                    aObjectMatrix,
                    *pAttribute));
                xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                delete pAttribute;

                // a standard gray outline is created for scaled content
                if(bScaleContent)
                {
                    const svtools::ColorConfig aColorConfig;
                    const svtools::ColorConfigValue aColor(aColorConfig.GetColorValue(svtools::OBJECTBOUNDARIES));

                    if(aColor.bIsVisible)
                    {
                        basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0, 0, 1, 1)));
                        const Color aVclColor(aColor.nColor);
                        aOutline.transform(aObjectMatrix);
                        const drawinglayer::primitive2d::Primitive2DReference xOutline(
                            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aVclColor.getBColor()));
                        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(xRetval, xOutline);
                    }
                }
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrOle2Obj::createViewIndependentPrimitive2DSequence() const
        {
            const Graphic* pOLEGraphic = GetOle2Obj().GetGraphic();

            if(pOLEGraphic)
            {
                // there is a graphic set, use it
                return createPrimitive2DSequenceWithGivenGraphic(*pOLEGraphic, GetOle2Obj().IsEmptyPresObj());
            }
            else
            {
                // no graphic, use default empty OLE bitmap
                const Bitmap aEmptyOLEBitmap(GetOle2Obj().GetEmtyOLEReplacementBitmap());
                const Graphic aEmtyOLEGraphic(aEmptyOLEBitmap);

                return createPrimitive2DSequenceWithGivenGraphic(aEmtyOLEGraphic, true);
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
