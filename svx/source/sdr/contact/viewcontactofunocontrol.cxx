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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewcontactofunocontrol.hxx>
#include <svx/sdr/contact/viewobjectcontactofunocontrol.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/sdrpagewindow.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/awt/XWindow2.hpp>
/** === end UNO includes === **/

#include "svx/sdrpaintwindow.hxx"
#include <tools/diagnose_ex.h>
#include <vcl/pdfextoutdevdata.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>

//........................................................................
namespace sdr { namespace contact {
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XControlContainer;
    using ::com::sun::star::awt::XControlModel;
    using ::com::sun::star::awt::XWindow2;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    /** === end UNO using === **/

    //====================================================================
    //= ViewContactOfUnoControl
    //====================================================================
    class ViewContactOfUnoControl_Impl
    {
    public:
        ViewContactOfUnoControl_Impl();
        ~ViewContactOfUnoControl_Impl();

    private:
        ViewContactOfUnoControl_Impl( const ViewContactOfUnoControl_Impl& );            // never implemented
        ViewContactOfUnoControl_Impl& operator=( const ViewContactOfUnoControl_Impl& ); // never implemented
    };

    //--------------------------------------------------------------------
    ViewContactOfUnoControl_Impl::ViewContactOfUnoControl_Impl()
    {
    }

    //--------------------------------------------------------------------
    ViewContactOfUnoControl_Impl::~ViewContactOfUnoControl_Impl()
    {
    }

    //====================================================================
    //= ViewContactOfUnoControl
    //====================================================================
    DBG_NAME( ViewContactOfUnoControl )
    //--------------------------------------------------------------------
    ViewContactOfUnoControl::ViewContactOfUnoControl( SdrUnoObj& _rUnoObject )
        :ViewContactOfSdrObj( _rUnoObject )
        ,m_pImpl( new ViewContactOfUnoControl_Impl )
    {
        DBG_CTOR( ViewContactOfUnoControl, NULL );
    }

    //--------------------------------------------------------------------
    ViewContactOfUnoControl::~ViewContactOfUnoControl()
    {
        DBG_DTOR( ViewContactOfUnoControl, NULL );
    }

    //--------------------------------------------------------------------
    Reference< XControl > ViewContactOfUnoControl::getTemporaryControlForWindow(
        const Window& _rWindow, Reference< XControlContainer >& _inout_ControlContainer ) const
    {
        SdrUnoObj* pUnoObject = dynamic_cast< SdrUnoObj* >( TryToGetSdrObject() );
        OSL_ENSURE( pUnoObject, "ViewContactOfUnoControl::getTemporaryControlForDevice: no SdrUnoObj!" );
        if ( !pUnoObject )
            return NULL;
        return ViewObjectContactOfUnoControl::getTemporaryControlForWindow( _rWindow, _inout_ControlContainer, *pUnoObject );
    }

    //--------------------------------------------------------------------
    ViewObjectContact& ViewContactOfUnoControl::CreateObjectSpecificViewObjectContact( ObjectContact& _rObjectContact )
    {
        // print or print preview requires special handling
        const OutputDevice* pDevice = _rObjectContact.TryToGetOutputDevice();
        bool bPrintOrPreview = ( pDevice != NULL ) && ( pDevice->GetOutDevType() == OUTDEV_PRINTER );

        ObjectContactOfPageView* pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &_rObjectContact  );
        bPrintOrPreview |= ( pPageViewContact != NULL ) && pPageViewContact->GetPageWindow().GetPageView().GetView().IsPrintPreview();

        if ( bPrintOrPreview )
            return *new UnoControlPrintOrPreviewContact( *pPageViewContact, *this );

        // all others are nowadays served by the same implementation
        return *new ViewObjectContactOfUnoControl( _rObjectContact, *this );
    }

    //--------------------------------------------------------------------
    drawinglayer::primitive2d::Primitive2DSequence ViewContactOfUnoControl::createViewIndependentPrimitive2DSequence() const
    {
        // create range. Use model data directly, not getBoundRect()/getSnapRect; these will use
        // the primitive data themselves in the long run. Use SdrUnoObj's (which is a SdrRectObj)
        // call to GetGeoRect() to access SdrTextObj::aRect directly and without executing anything
        const Rectangle& rRectangle(GetSdrUnoObj().GetGeoRect());
        const basegfx::B2DRange aRange(
            rRectangle.Left(), rRectangle.Top(),
            rRectangle.Right(), rRectangle.Bottom());

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

            return drawinglayer::primitive2d::Primitive2DSequence(&xRetval, 1);
        }
        else
        {
            // always append an invisible outline for the cases where no visible content exists
            const drawinglayer::primitive2d::Primitive2DReference xRetval(
                drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                    false, aTransform));

            return drawinglayer::primitive2d::Primitive2DSequence(&xRetval, 1);
        }
    }

//........................................................................
} } // namespace sdr::contact
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
