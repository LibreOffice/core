/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactofunocontrol.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:12:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX
#include <svx/sdr/contact/viewcontactofunocontrol.hxx>
#endif
#ifndef SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX
#include <svx/sdr/contact/viewobjectcontactofunocontrol.hxx>
#endif
#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _SVDOUNO_HXX
#include "svdouno.hxx"
#endif
#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif
#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif
#ifndef _SDRPAGEWINDOW_HXX
#include "sdrpagewindow.hxx"
#endif
#ifndef _SDRPAINTWINDOW_HXX
#include "sdrpaintwindow.hxx"
#endif

#ifndef _VCL_PDFEXTOUTDEVDATA_HXX
#include <vcl/pdfextoutdevdata.hxx>
#endif

//........................................................................
namespace sdr { namespace contact {
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XControlContainer;
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
            sal_uInt32 nCountBefore( maVOCList.Count() );
#endif
            pVOC->PrepareDelete();
            delete pVOC;
#ifdef DBG_UTIL
            DBG_ASSERT( maVOCList.Count() < nCountBefore,
                "ViewContactOfUnoControl::invalidateAllContacts: prepare for an infinite loop!" );
#endif
        }
    }

//........................................................................
} } // namespace sdr::contact
//........................................................................
