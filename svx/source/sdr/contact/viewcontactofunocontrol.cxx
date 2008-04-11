/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofunocontrol.cxx,v $
 * $Revision: 1.10 $
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

#include "sdrpaintwindow.hxx"
#include <tools/diagnose_ex.h>
#include <vcl/pdfextoutdevdata.hxx>

//........................................................................
namespace sdr { namespace contact {
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XControlContainer;
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
    Reference< XControl > ViewContactOfUnoControl::getUnoControlForDevice( const OutputDevice* _pDevice, const SdrUnoObjAccessControl& ) const
    {
        sal_uInt32 vocCount = maVOCList.Count();
        for ( sal_uInt32 voc = 0; voc < vocCount; ++voc )
        {
            ViewObjectContactOfUnoControl* pUCVOC = dynamic_cast< ViewObjectContactOfUnoControl* >( maVOCList.GetObject( voc ) );
            DBG_ASSERT( pUCVOC, "ViewContactOfUnoControl::getUnoControlForDevice: wrong ViewObjectContact type!" );
            if ( !pUCVOC )
                continue;

            if ( pUCVOC->belongsToDevice( _pDevice ) )
                return pUCVOC->getControl();
        }
        return Reference< XControl >();
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
        ObjectContactOfPageView* pPageViewContact = dynamic_cast< ObjectContactOfPageView* >( &_rObjectContact  );
        if ( pPageViewContact )
        {
            // special classes for special devices:
            // - PDF export
            ::vcl::PDFExtOutDevData* pPDFExport = PTR_CAST( ::vcl::PDFExtOutDevData, pPageViewContact->GetPageWindow().GetPaintWindow().GetOutputDevice().GetExtOutDevData() );
            if ( pPDFExport != NULL )
                return *new UnoControlPDFExportContact( *pPageViewContact, *this );

            // - print preview
            if ( pPageViewContact->GetPageWindow().GetPageView().GetView().IsPrintPreview() )
                return *new UnoControlPrintOrPreviewContact( *pPageViewContact, *this );

            OutDevType eDeviceType = pPageViewContact->GetPageWindow().GetPaintWindow().GetOutputDevice().GetOutDevType();
            // - printing
            if ( eDeviceType == OUTDEV_PRINTER )
                return *new UnoControlPrintOrPreviewContact( *pPageViewContact, *this );

            // - any other virtual device
            if ( eDeviceType == OUTDEV_VIRDEV )
                return *new UnoControlDefaultContact( *pPageViewContact, *this );

            // - normal windows have special, design-mode dependent handling
            if ( eDeviceType == OUTDEV_WINDOW )
                return *new UnoControlWindowContact( *pPageViewContact, *this );
        }

        return *new UnoControlDefaultContact( _rObjectContact, *this );
    }

    //--------------------------------------------------------------------
    void ViewContactOfUnoControl::invalidateAllContacts( const SdrUnoObjAccessControl&  )
    {
        while ( maVOCList.Count() )
        {
            ViewObjectContact* pVOC( maVOCList.GetObject( 0 ) );
#ifdef DBG_UTIL
            const sal_uInt32 nCountBefore( maVOCList.Count() );
#endif
            pVOC->PrepareDelete();
            delete pVOC;
            DBG_ASSERT( maVOCList.Count() < nCountBefore,
                "ViewContactOfUnoControl::invalidateAllContacts: prepare for an infinite loop!" );
        }
    }

    //--------------------------------------------------------------------
    sal_Bool ViewContactOfUnoControl::ShouldPaintObject( DisplayInfo& _rDisplayInfo, const ViewObjectContact& _rAssociatedVOC )
    {
        // position the control
        // That's needed for alive mode, where the control is in fact a visible VCL window. In this
        // case, if the base classes ShouldPaintObject returns FALSE, there would be artifacts
        // since the VCL window is not moved to the proper position.
        // #i72694# / 2006-12-18 / frank.schoenheit@sun.com

        const ViewObjectContactOfUnoControl& rVOC( dynamic_cast< const ViewObjectContactOfUnoControl& >( _rAssociatedVOC ) );
        // #i74769# to not resize and position at each DrawLayer() use FormControl flag
        if ( _rDisplayInfo.GetControlLayerPainting() )
        {
            rVOC.positionControlForPaint( _rDisplayInfo );
        }

        // don't paint if the base class tells so
        if ( !ViewContactOfSdrObj::ShouldPaintObject( _rDisplayInfo, _rAssociatedVOC ) )
            return false;

        // always paint in design mode
        SdrPageView* pPageView = _rDisplayInfo.GetPageView();
        bool bIsDesignMode = pPageView ? pPageView->GetView().IsDesignMode() : false;
        if ( bIsDesignMode )
            return true;

        // in alive mode, don't paint if the control is not visible.
        // #i82791#
        bool bIsVisible = true;
        try
        {
            Reference< XWindow2 > xControlWindow( rVOC.getExistentControl(), UNO_QUERY );
            if ( xControlWindow.is() )
                bIsVisible = xControlWindow->isVisible();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bIsVisible;
    }

//........................................................................
} } // namespace sdr::contact
//........................................................................
