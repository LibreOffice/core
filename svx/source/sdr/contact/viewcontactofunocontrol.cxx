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

#include <sal/config.h>

#include <sdr/contact/viewcontactofunocontrol.hxx>
#include <sdr/contact/viewobjectcontactofunocontrol.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/sdrpagewindow.hxx>

#include <vcl/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <osl/diagnose.h>


namespace sdr::contact {


    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XControlContainer;
    using ::com::sun::star::awt::XControlModel;


    //= ViewContactOfUnoControl

    ViewContactOfUnoControl::ViewContactOfUnoControl( SdrUnoObj& _rUnoObject )
        :ViewContactOfSdrObj( _rUnoObject )
    {
    }


    ViewContactOfUnoControl::~ViewContactOfUnoControl()
    {
    }


    Reference< XControl > ViewContactOfUnoControl::getTemporaryControlForWindow(
        const vcl::Window& _rWindow, Reference< XControlContainer >& _inout_ControlContainer ) const
    {
        SdrUnoObj* pUnoObject = dynamic_cast< SdrUnoObj* >( TryToGetSdrObject() );
        OSL_ENSURE( pUnoObject, "ViewContactOfUnoControl::getTemporaryControlForDevice: no SdrUnoObj!" );
        if ( !pUnoObject )
            return nullptr;
        return ViewObjectContactOfUnoControl::getTemporaryControlForWindow( _rWindow, _inout_ControlContainer, *pUnoObject );
    }


    ViewObjectContact& ViewContactOfUnoControl::CreateObjectSpecificViewObjectContact( ObjectContact& _rObjectContact )
    {
        // print or print preview requires special handling
        const OutputDevice* pDevice = _rObjectContact.TryToGetOutputDevice();
        ObjectContactOfPageView* const pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &_rObjectContact  );

        const bool bPrintOrPreview = pPageViewContact
            && ( ( ( pDevice != nullptr ) && ( pDevice->GetOutDevType() == OUTDEV_PRINTER ) )
                    || pPageViewContact->GetPageWindow().GetPageView().GetView().IsPrintPreview()
               )
            ;

        if ( bPrintOrPreview )
            return *new UnoControlPrintOrPreviewContact( *pPageViewContact, *this );

        // all others are nowadays served by the same implementation
        return *new ViewObjectContactOfUnoControl( _rObjectContact, *this );
    }


    void ViewContactOfUnoControl::createViewIndependentPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
    {
        // create range. Use model data directly, not getBoundRect()/getSnapRect; these will use
        // the primitive data themselves in the long run. Use SdrUnoObj's (which is a SdrRectObj)
        // call to GetGeoRect() to access SdrTextObj::aRect directly and without executing anything
        const basegfx::B2DRange aRange = vcl::unotools::b2DRectangleFromRectangle(GetSdrUnoObj().GetGeoRect());

        // create object transform
        basegfx::B2DHomMatrix aTransform;

        aTransform.set(0, 0, aRange.getWidth());
        aTransform.set(1, 1, aRange.getHeight());
        aTransform.set(0, 2, aRange.getMinX());
        aTransform.set(1, 2, aRange.getMinY());

        Reference< XControlModel > xControlModel = GetSdrUnoObj().GetUnoControlModel();

        if(xControlModel.is())
        {
            // create control primitive WITHOUT possibly existing XControl; this would be done in
            // the VOC in createPrimitive2DSequence()
            const drawinglayer::primitive2d::Primitive2DReference xRetval(
                new drawinglayer::primitive2d::ControlPrimitive2D(
                    aTransform,
                    xControlModel));

            rVisitor.visit(xRetval);
        }
        else
        {
            // always append an invisible outline for the cases where no visible content exists
            const drawinglayer::primitive2d::Primitive2DReference xRetval(
                drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                    aTransform));

            rVisitor.visit(xRetval);
        }
    }


} // namespace sdr::contact


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
