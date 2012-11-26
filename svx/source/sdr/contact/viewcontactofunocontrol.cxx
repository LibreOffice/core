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
    //--------------------------------------------------------------------
    ViewContactOfUnoControl::ViewContactOfUnoControl( SdrUnoObj& _rUnoObject )
        :ViewContactOfSdrObj( _rUnoObject )
        ,m_pImpl( new ViewContactOfUnoControl_Impl )
    {
    }

    //--------------------------------------------------------------------
    ViewContactOfUnoControl::~ViewContactOfUnoControl()
    {
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
        drawinglayer::primitive2d::Primitive2DSequence xRetval;

        // get object transformation and control model
        const basegfx::B2DHomMatrix& rObjectMatrix(GetSdrUnoObj().getSdrObjectTransformation());
        Reference< XControlModel > xControlModel = GetSdrUnoObj().GetUnoControlModel();

        if(xControlModel.is())
        {
            // create control primitive WITHOUT possibly existing XControl; this would be done in
            // the VOC in createPrimitive2DSequence()
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::ControlPrimitive2D(
                    rObjectMatrix,
                    xControlModel));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }
        else
        {
            // always append an invisible outline for the cases where no visible content exists
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                    false, rObjectMatrix));

            xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

        return xRetval;
    }

//........................................................................
} } // namespace sdr::contact
//........................................................................
