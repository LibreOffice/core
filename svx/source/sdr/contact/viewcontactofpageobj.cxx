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

#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#include <svx/svdopage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewobjectcontactofpageobj.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact& ViewContactOfPageObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfPageObj(rObjectContact, *this);
            return *pRetval;
        }

        ViewContactOfPageObj::ViewContactOfPageObj(SdrPageObj& rPageObj)
        :   ViewContactOfSdrObj(rPageObj)
        {
        }

        ViewContactOfPageObj::~ViewContactOfPageObj()
        {
        }

        // #i35972# React on changes of the object of this ViewContact
        void ViewContactOfPageObj::ActionChanged()
        {
            static bool bIsInActionChange(false);

            if(!bIsInActionChange)
            {
                // set recursion flag, see description in *.hxx
                bIsInActionChange = true;

                // call parent
                ViewContactOfSdrObj::ActionChanged();

                // reset recursion flag, see description in *.hxx
                bIsInActionChange = false;
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContactOfPageObj::createViewIndependentPrimitive2DSequence() const
        {
            // ceate graphical visualisation data. Since this is the view-independent version which should not be used,
            // create a replacement graphic visualisation here. Use GetLastBoundRect to access the model data directly
            // which is aOutRect for SdrPageObj.
            const Rectangle aModelRectangle(GetPageObj().GetLastBoundRect());
            const basegfx::B2DRange aModelRange(aModelRectangle.Left(), aModelRectangle.Top(), aModelRectangle.Right(), aModelRectangle.Bottom());
            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aModelRange));
            const basegfx::BColor aYellow(1.0, 1.0, 0.0);
            const drawinglayer::primitive2d::Primitive2DReference xReference(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aYellow));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
