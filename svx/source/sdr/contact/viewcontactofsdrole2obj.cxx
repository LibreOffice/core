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


#include <svx/sdr/contact/viewcontactofsdrole2obj.hxx>
#include <svx/svdoole2.hxx>
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
#include <svx/sdr/primitive2d/sdrolecontentprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

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

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters() const
        {
            // take unrotated snap rect (direct model data) for position and size
            const Rectangle& rRectangle = GetOle2Obj().GetGeoRect();
            const basegfx::B2DRange aObjectRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

            // create object matrix
            const GeoStat& rGeoStat(GetOle2Obj().GetGeoStat());
            const double fShearX(rGeoStat.nShearWink ? tan((36000 - rGeoStat.nShearWink) * F_PI18000) : 0.0);
            const double fRotate(rGeoStat.nDrehWink ? (36000 - rGeoStat.nDrehWink) * F_PI18000 : 0.0);
            const basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                aObjectRange.getWidth(), aObjectRange.getHeight(), fShearX, fRotate,
                aObjectRange.getMinX(), aObjectRange.getMinY()));

            // Prepare attribute settings, will be used soon anyways
            const SfxItemSet& rItemSet = GetOle2Obj().GetMergedItemSet();
            const drawinglayer::attribute::SdrLineFillShadowTextAttribute aAttribute(
                drawinglayer::primitive2d::createNewSdrLineFillShadowTextAttribute(
                    rItemSet,
                    GetOle2Obj().getText(0)));

            // #i102063# embed OLE content in an own primitive; this will be able to decompose accessing
            // the weak SdrOle2 reference and will also implement getB2DRange() for fast BoundRect
            // calculations without OLE Graphic access (which may trigger e.g. chart recalculation).
            // It will also take care of HighContrast and ScaleContent
            const drawinglayer::primitive2d::Primitive2DReference xOleContent(
                new drawinglayer::primitive2d::SdrOleContentPrimitive2D(
                    GetOle2Obj(),
                    aObjectMatrix,

                    // #i104867# add GraphicVersion number to be able to check for
                    // content change in the primitive later
                    GetOle2Obj().getEmbeddedObjectRef().getGraphicVersion()
                ));

            // create primitive. Use Ole2 primitive here. Prepare attribute settings, will
            // be used soon anyways. Always create primitives to allow the decomposition of
            // SdrOle2Primitive2D to create needed invisible elements for HitTest and/or BoundRect
            const drawinglayer::primitive2d::Primitive2DSequence xOLEContent(&xOleContent, 1);
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::SdrOle2Primitive2D(
                    xOLEContent,
                    aObjectMatrix,
                    aAttribute));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfSdrOle2Obj::createViewIndependentPrimitive2DSequence() const
        {
            return createPrimitive2DSequenceWithParameters();
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
