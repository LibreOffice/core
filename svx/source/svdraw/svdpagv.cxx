/*************************************************************************
 *
 *  $RCSfile: svdpagv.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:48:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODECHANGEBROADCASTER_HPP_
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#include <comphelper/processfactory.hxx>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifdef OS2
#include <tools/svpm.h>
#include <vcl/sysdep.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#include "svdedxv.hxx"
#include "svdpagv.hxx"
#include "svdoutl.hxx"
#include "svdpagv.hxx"
#include "outliner.hxx"
#include "xpoly.hxx"
#include "svdxout.hxx"
#include "svdetc.hxx"
#include "svdobj.hxx"
#include "svdouno.hxx"
#include "svdpage.hxx"
#include "svdio.hxx"
#include "svdview.hxx"
#include "svditer.hxx"
#include "svdogrp.hxx"
#include "svdtypes.hxx"

#include "svdotext.hxx" // fuer PaintOutlinerView
#include "svdoole2.hxx"

// #110094#
#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTREDIRECTOR_HXX
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#endif

// for search on vector
#include <algorithm>

using namespace ::rtl;
using namespace ::com::sun::star;

TYPEINIT1(SdrPageView, SfxListener);


// Klasse muﬂ als listener fungieren, um den Zustand, ob ein Object sichtbar ist oder nicht
// festzuhalten
//------------------------------------------------------------------------------
SdrUnoControlRec::SdrUnoControlRec(SdrUnoControlList* _pParent, SdrUnoObj* _pObj, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > _xControl) throw()
:   pObj(_pObj)
    ,xControl(_xControl)
    ,bVisible(TRUE)
    ,bIsListening(FALSE)
    ,bDisposed(FALSE)
    ,pParent(_pParent)
    ,mnPaintLevel( 0 )
{
    DBG_ASSERT( xControl.is(), "SdrUnoControlRec::SdrUnoControlRec: invalid control, this will crash!" );

    bVisible = xControl.is() ? !xControl->isDesignMode() : TRUE;
    bool bOldVisible = bVisible;

    // if bVisible is TRUE here, then switchControlListening will also start
    // DesignModeListening
    switchControlListening( true );

    // adjust the initial visibility according to the visibility of the layer
    // 2003-06-03 - #110592# - fs@openoffice.org
    adjustControlVisibility( true );

    // no start this design mode listening
    if ( bOldVisible && !bOldVisible )
        // visibility changed from true to false -> explicitly
        // start DesignModeListening
        // 2003-07-18 - #110916# - fs@openoffice.org
        switchDesignModeListening( true );
}

//------------------------------------------------------------------------------
SdrUnoControlRec::~SdrUnoControlRec() throw()
{
}

//------------------------------------------------------------------------------
void SdrUnoControlRec::adjustControlVisibility( bool _bForce )
{
    uno::Reference< awt::XWindow > xControlWindow( xControl, uno::UNO_QUERY );
    if ( xControlWindow.is() && !xControl->isDesignMode() )
    {
        // the layer of our object
        SdrLayerID nObjectLayer = pObj->GetLayer();
        // the SdrPageView we're living in
        SdrPageView& rView = pParent->rPageView;
        // is the layer we're residing in visible in this view?
        bool bIsObjectLayerVisible = rView.GetVisibleLayers().IsSet( nObjectLayer );

        if ( _bForce || ( bIsObjectLayerVisible != bVisible ) )
            xControlWindow->setVisible( bIsObjectLayerVisible );
    }

}

//------------------------------------------------------------------------------
void SdrUnoControlRec::switchControlListening( bool _bStart )
{
    uno::Reference< awt::XWindow> xWindow( xControl, uno::UNO_QUERY );
    if ( xWindow.is() )
    {
        // listen for visibility changes
        if ( _bStart )
            xWindow->addWindowListener( this );
        else
            xWindow->removeWindowListener( this );

        if ( !bVisible )
            switchDesignModeListening( _bStart );

        // Am Property fuer das DefaultControl lauschen um das Control eventuell auszutauschen
        switchPropertyListening( _bStart, false );

        // listen for design mode changes
        uno::Reference< util::XModeChangeBroadcaster > xDesignModeChanges( xControl, uno::UNO_QUERY );
        if ( xDesignModeChanges.is() )
            if ( _bStart )
                xDesignModeChanges->addModeChangeListener( this );
            else
                xDesignModeChanges->removeModeChangeListener( this );
    }
}

//------------------------------------------------------------------------------
void SdrUnoControlRec::switchPropertyListening( bool _bStart, bool _bListenForAll )
{
    DBG_ASSERT( xControl.is(), "SdrUnoControlRec::switchPropertyListening: no control!" );
    if ( xControl.is() )
    {
        uno::Reference< beans::XPropertySet > xSet( xControl->getModel(), uno::UNO_QUERY );
        if ( xSet.is() )
        {
            ::rtl::OUString sPropertyToListenFor;

            if ( !_bListenForAll )
        {
                // listen for the DefaultControl property only, if available
                ::rtl::OUString sDefaultControlPropertyName( RTL_CONSTASCII_USTRINGPARAM( "DefaultControl" ) );
            uno::Reference< beans::XPropertySetInfo > xPropInfo( xSet->getPropertySetInfo() );
                if ( xPropInfo.is() && xPropInfo->hasPropertyByName( sDefaultControlPropertyName ) )
                    sPropertyToListenFor = sDefaultControlPropertyName;
            }

            if ( _bStart )
                xSet->addPropertyChangeListener( sPropertyToListenFor, this );
            else
                xSet->removePropertyChangeListener( sPropertyToListenFor, this );
        }
        }
}

//------------------------------------------------------------------------------
void SAL_CALL SdrUnoControlRec::disposing( const ::com::sun::star::lang::EventObject& Source )
    throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XControl > xSource( Source.Source, uno::UNO_QUERY);
    if (xSource.is())
    {   // it's the control we're responsible for
        switchControlListening( false );

        if (pParent)
        {
            uno::Reference< uno::XInterface > xThis(*this);
            xControl = NULL;
            pObj = NULL;

            // jetzt darf mich der Vater freigeben
            pParent->Disposing(this);
            pParent = NULL;
        }

        bDisposed = TRUE;
    }
}

//------------------------------------------------------------------------------

// XWindowListener
void SAL_CALL SdrUnoControlRec::windowResized( const ::com::sun::star::awt::WindowEvent& e )
    throw(::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SdrUnoControlRec::windowMoved( const ::com::sun::star::awt::WindowEvent& e )
    throw(::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SdrUnoControlRec::windowShown( const ::com::sun::star::lang::EventObject& e )
    throw(::com::sun::star::uno::RuntimeException)
{
    if ( !mnPaintLevel )
    {
        if (!IsVisible())
            switchDesignModeListening( false );
    }

    bVisible = TRUE;
}

//------------------------------------------------------------------------------
void SAL_CALL SdrUnoControlRec::windowHidden( const ::com::sun::star::lang::EventObject& e )
    throw(::com::sun::star::uno::RuntimeException)
{
    bVisible = FALSE;

    // Im Designmodus ist das Control nicht sichtbar und der drawing layer ist
    // verantwortlich fuer die Darstellung des Controls
    // In diesem Fall auf Aenderungen an den Eigenschaften der Controls gelauscht,
    // um ein Repaint auszufuehren.
    if ( !mnPaintLevel )
    {
        if (!bDisposed)
            switchDesignModeListening( true );
    }
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
void SAL_CALL SdrUnoControlRec::propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt )
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!xControl.is())
        return;

    if (evt.PropertyName == rtl::OUString::createFromAscii("DefaultControl"))
    {
        // anlegen eines neuen Controls
        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
        if( xFactory.is() )
        {
            OUString aControlName;
            if(evt.NewValue >>= aControlName)
            {
                uno::Reference< awt::XControl > xUnoControl( xFactory->createInstance(aControlName), uno::UNO_QUERY);
                ReplaceControl(xUnoControl);
            }
        }
    }
    else
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        // Attributaenderung:
        // Bereich neu Zeichnen
        OutputDevice* pOut = pObj->GetOutputDevice(xControl);
        if (pOut && pOut->GetOutDevType() == OUTDEV_WINDOW)
            ((Window*)pOut)->Invalidate(pObj->GetCurrentBoundRect());
    }
}

// XImageConsumer
//------------------------------------------------------------------------------
void SAL_CALL SdrUnoControlRec::complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer )
    throw(::com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------------
void SAL_CALL SdrUnoControlRec::init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SdrUnoControlRec::setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SdrUnoControlRec::setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SdrUnoControlRec::setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------------
void SAL_CALL SdrUnoControlRec::modeChanged( const util::ModeChangeEvent& _rSource ) throw (uno::RuntimeException)
{
    // if the control is part of a invisible layer, we need to explicitly hide it in alive mode
    // 2003-06-03 - #110592# - fs@openoffice.org
    adjustControlVisibility( false );
}

//------------------------------------------------------------------------------
void SdrUnoControlRec::Clear(BOOL bDispose)
{
    if (xControl.is())
    {
        switchControlListening( false );

        if (bDispose)
            xControl->dispose();
        xControl = NULL;
    }
}

//------------------------------------------------------------------------------
void SdrUnoControlRec::ReplaceControl(uno::Reference< awt::XControl > _xControl)
{
    uno::Reference< awt::XWindow > xWindow(_xControl, uno::UNO_QUERY);
    if (xWindow.is())
    {
        uno::Reference< awt::XControlContainer > xControlContainer(xControl->getContext(), uno::UNO_QUERY);
        _xControl->setModel(xControl->getModel());

        // Feststellen, welcher Modus eingeschaltet ist
        BOOL bDesignMode = xControl->isDesignMode();

        // set the correct zoom
        OutputDevice* pOutDev = pObj->GetOutputDevice(xControl);
        if (pOutDev && !bDesignMode)
        {
            uno::Reference< awt::XView > xView(_xControl, uno::UNO_QUERY);
            if (xView.is())
            {
                const MapMode& rMap = pOutDev->GetMapMode();
                xView->setZoom((float) double(rMap.GetScaleX()),
                               (float) double(rMap.GetScaleY()));
            }
        }

        // Groeﬂe setzen
        if (pOutDev && pOutDev->GetOutDevType() == OUTDEV_WINDOW)
        {
            Rectangle aRect(pObj->GetLogicRect());
            Point aPixPos(pOutDev->LogicToPixel(aRect.TopLeft()));
            Size aPixSize(pOutDev->LogicToPixel(aRect.GetSize()));
            xWindow->setPosSize(aPixPos.X(), aPixPos.Y(), aPixSize.Width(), aPixSize.Height(), awt::PosSize::POSSIZE);
        }

        // Control wird nicht mehr benoetigt
        // zunaechst als Listener austragen
        Clear(TRUE);

        xControlContainer->addControl(String(), _xControl);

        // Designmodus erst setzen, wenn Peer bereits existiert!
        _xControl->setDesignMode(bDesignMode);

        xControl = _xControl;

        // und wieder alle Listener anmelden
        switchControlListening( true );
    }
}

//------------------------------------------------------------------------------

void SdrUnoControlRec::switchDesignModeListening( bool _bStart )
{
    if ( (bool)IsListening() != _bStart )
    {
        bIsListening = _bStart;

        if (xControl.is())
        {
            switchPropertyListening( _bStart, true );

            uno::Reference< form::XImageProducerSupplier > xImg( xControl->getModel(), uno::UNO_QUERY );
            if (xImg.is())
            {
                uno::Reference< awt::XImageProducer > xProducer = xImg->getImageProducer();
                if (xProducer.is())
                    if ( _bStart )
                        xProducer->addConsumer(this);
                    else
                    xProducer->removeConsumer(this);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SV_IMPL_OP_PTRARR_SORT( SdrUnoControlAccessArr, SdrUnoControlAccessPtr )

//------------------------------------------------------------------------
SdrUnoControlRec* SdrUnoControlList::GetObject(USHORT i) const
{
    SdrUnoControlAccess* pAccess = aAccessArr[i];
    return pAccess->pControlRec;
}

//------------------------------------------------------------------------
void SdrUnoControlList::Clear(BOOL bDispose)
{
    for( USHORT i=aAccessArr.Count(); i > 0; )
        Delete(--i, bDispose);

    DBG_ASSERT(!aList.Count(), "SdrUnoControlList::Clear(BOOL bDispose): List not empty");
}

//------------------------------------------------------------------------
void SdrUnoControlList::Insert(SdrUnoControlRec* pRec)
{
    aList.Insert( pRec, LIST_APPEND );
    pRec->acquire();

    SdrUnoControlAccess* pAccess = new SdrUnoControlAccess(pRec->GetControl()->getModel(), pRec);
    aAccessArr.Insert( pAccess );
}

//------------------------------------------------------------------------
void SdrUnoControlList::Delete(USHORT nPos, BOOL bDispose)
{
    SdrUnoControlAccess* pAccess = aAccessArr[nPos];
    SdrUnoControlRec* pRec = pAccess->pControlRec;

    aList.Remove( pRec );
    aAccessArr.Remove( nPos );

    // Rec laeﬂt alles los und wird anschlieﬂend zerstoert
    pRec->Clear(bDispose);

    pRec->release();
    delete pAccess;
}

//------------------------------------------------------------------------
void SdrUnoControlList::Disposing(SdrUnoControlRec* pRec)
{
    SdrUnoControlAccess* pAccess = NULL;
    for (USHORT i = 0; i < aAccessArr.Count(); i++)
    {
        pAccess = aAccessArr[i];
        if (pAccess->pControlRec == pRec)
        {
            aList.Remove( pRec );
            aAccessArr.Remove( i );

            pRec->release();
            delete pAccess;
            break;
        }
    }
}

//------------------------------------------------------------------------
USHORT SdrUnoControlList::Find(SdrUnoControlRec* pUCR) const
{
    return Find( pUCR->GetControl()->getModel() );
}

//------------------------------------------------------------------------
USHORT SdrUnoControlList::Find(uno::Reference< awt::XControl > rUnoControl) const
{
    return Find( rUnoControl->getModel() );
}

//------------------------------------------------------------------------
USHORT SdrUnoControlList::Find(uno::Reference< awt::XControlModel > rUnoControlModel) const
{
    SdrUnoControlAccess aAccess(rUnoControlModel);
    USHORT nPos;
    if( !aAccessArr.Seek_Entry(&aAccess, &nPos) )
        return SDRUNOCONTROL_NOTFOUND;

    return nPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPageViewWindow::CreateControlContainer()
{
    if (!mxControlContainer.is())
    {
        SdrView& rView = GetPageView().GetView();

        if(OUTDEV_WINDOW == GetOutputDevice().GetOutDevType() && !rView.IsPrintPreview())
        {
            Window* pWindow = (Window*)(&GetOutputDevice());
            mxControlContainer = VCLUnoHelper::CreateControlContainer( pWindow );

            // #100394# xC->setVisible triggers window->Show() and this has
            // problems when the view is not completely constructed which may
            // happen when loading. This leads to accessibility broadcasts which
            // throw asserts due to the not finished view. All this chan be avoided
            // since xC->setVisible is here called only for the side effect in
            // UnoControlContainer::setVisible(...) which calls createPeer(...).
            // This will now be called directly from here.

            // UnoContainerModel erzeugen
            // uno::Reference< awt::XWindow > xC(mxControlContainer, uno::UNO_QUERY);
            // CreateControlContainer() is only used from
            // , thus it seems not necessary to make
            // it visible her at all.
            // #58917# Das Show darf nicht am VCL-Fenster landen, weil dann Assertion vom SFX
            // BOOL bVis = pWindow->IsVisible();
            // xC->setVisible(TRUE);
            // if ( !bVis )
            //  pWindow->Hide();
            //  if( !mxContext.is() && bVisible )
            //      // Es ist ein TopWindow, also automatisch anzeigen
            //      createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (), ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > () );

            uno::Reference< awt::XControl > xControl(mxControlContainer, uno::UNO_QUERY);
            if(xControl.is())
            {
                uno::Reference< uno::XInterface > xContext = xControl->getContext();
                if(!xContext.is())
                {
                    xControl->createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (),
                        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > () );
                }
            }
        }
        else
        {
            // Printer und VirtualDevice, bzw. kein OutDev
            uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            if( xFactory.is() )
            {
                mxControlContainer = uno::Reference< awt::XControlContainer >(xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainer")), uno::UNO_QUERY);
                uno::Reference< awt::XControlModel > xModel(xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainerModel")), uno::UNO_QUERY);
                uno::Reference< awt::XControl > xControl(mxControlContainer, uno::UNO_QUERY);
                if (xControl.is())
                    xControl->setModel(xModel);

                Point aPosPix = GetOutputDevice().GetMapMode().GetOrigin();
                Size aSizePix = GetOutputDevice().GetOutputSizePixel();

                uno::Reference< awt::XWindow > xContComp(mxControlContainer, uno::UNO_QUERY);
                if( xContComp.is() )
                    xContComp->setPosSize(aPosPix.X(), aPosPix.Y(), aSizePix.Width(), aSizePix.Height(), awt::PosSize::POSSIZE);
            }
        }

        rView.InsertControlContainer(mxControlContainer);
    }
}

SdrPageViewWindow::SdrPageViewWindow(SdrPageView& rPageView, OutputDevice& rOut)
:   mpObjectContact(0L), // #110094#
    mrPageView(rPageView),
    mrOutputDevice(rOut),
    mpIAOManager(0L),
    mpControlList(new SdrUnoControlList(rPageView))
{
    // is it a window?
    if(OUTDEV_WINDOW == GetOutputDevice().GetOutDevType())
    {
        // create B2dIAOManager for this window
        mpIAOManager = new B2dIAOManager((Window*)(&GetOutputDevice()));
    }
}

SdrPageViewWindow::~SdrPageViewWindow()
{
    // #110094#
    if(mpObjectContact)
    {
        mpObjectContact->PrepareDelete();
        delete mpObjectContact;
        mpObjectContact = 0L;
    }

    if (mxControlContainer.is())
    {
        SdrView& rView = GetPageView().GetView();

        // notify derived views
        rView.RemoveControlContainer(mxControlContainer);

        // clear the control place holders
        mpControlList->Clear(sal_False);

        // dispose the control container
        uno::Reference< lang::XComponent > xComponent(mxControlContainer, uno::UNO_QUERY);
        xComponent->dispose();
    }

    // cleanup IAOManager for this window
    if(mpIAOManager)
    {
        delete mpIAOManager;
        mpIAOManager = 0L;
    }

    // cleanup SdrUnoControlList
    delete mpControlList;
}

SdrPageView& SdrPageViewWindow::GetPageView() const
{
    return mrPageView;
}

OutputDevice& SdrPageViewWindow::GetOutputDevice() const
{
    return mrOutputDevice;
}

SdrUnoControlList& SdrPageViewWindow::GetControlList() const
{
    return *mpControlList;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > SdrPageViewWindow::GetControlContainerRef() const
{
    return mxControlContainer;
}

B2dIAOManager* SdrPageViewWindow::GetIAOManager() const
{
    return mpIAOManager;
}

// #110094# ObjectContact section
sdr::contact::ObjectContact* SdrPageViewWindow::CreateViewSpecificObjectContact()
{
    return new sdr::contact::ObjectContactOfPageView(*this);
}

SdrPaintInfoRec* SdrPageViewWindow::ImpCreateNewPageInfoRec(const Rectangle& rDirtyRect,
    sal_uInt16 nPaintMode, const SdrLayerID* pId) const
{
    SdrPaintInfoRec* pInfoRec = new SdrPaintInfoRec();
    sal_Bool bPrinter(OUTDEV_PRINTER == mrOutputDevice.GetOutDevType());
    Rectangle aCheckRect(rDirtyRect);
    Size a1PixSiz(mrOutputDevice.PixelToLogic(Size(1, 1)));
    const SdrView& rView = mrPageView.GetView();

    aCheckRect.Left() -= a1PixSiz.Width();
    aCheckRect.Top() -= a1PixSiz.Height();
    aCheckRect.Right() += a1PixSiz.Width();
    aCheckRect.Bottom() += a1PixSiz.Height();

    pInfoRec->pPV = &mrPageView;
    pInfoRec->bPrinter = bPrinter;
    pInfoRec->aDirtyRect = rDirtyRect + mrPageView.GetOffset();
    pInfoRec->aCheckRect = aCheckRect;

    if(bPrinter)
    {
        if(rView.IsLineDraftPrn()) nPaintMode |= SDRPAINTMODE_DRAFTLINE;
        if(rView.IsFillDraftPrn()) nPaintMode |= SDRPAINTMODE_DRAFTFILL;
        if(rView.IsTextDraftPrn()) nPaintMode |= SDRPAINTMODE_DRAFTTEXT;
        if(rView.IsGrafDraftPrn()) nPaintMode |= SDRPAINTMODE_DRAFTGRAF;
    }
    else
    {
        if(rView.IsLineDraft()) nPaintMode |= SDRPAINTMODE_DRAFTLINE;
        if(rView.IsFillDraft()) nPaintMode |= SDRPAINTMODE_DRAFTFILL;
        if(rView.IsTextDraft()) nPaintMode |= SDRPAINTMODE_DRAFTTEXT;
        if(rView.IsGrafDraft()) nPaintMode |= SDRPAINTMODE_DRAFTGRAF;
        if(rView.IsHideGrafDraft()) nPaintMode |= SDRPAINTMODE_HIDEDRAFTGRAF;
    }

    if(pId)
    {
        pInfoRec->aPaintLayer.ClearAll();
        pInfoRec->aPaintLayer.Set(*pId);
    }
    else
    {
        pInfoRec->aPaintLayer = bPrinter
            ? mrPageView.GetPrintableLayers()
            : mrPageView.GetVisibleLayers();
    }

    pInfoRec->nPaintMode = nPaintMode;

    if(mrPageView.GetObjList() != mrPageView.GetPage())
    {
        pInfoRec->pAktList = mrPageView.GetObjList();
    }

    return pInfoRec;
}

void SdrPageViewWindow::Redraw(const Region& rReg, sal_uInt16 nPaintMode,
    const SdrLayerID* pId, ::sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    // set redirector if given
    if(pRedirector)
    {
        GetObjectContact().SetViewObjectContactRedirector(pRedirector);
    }

    const SdrView& rView = mrPageView.GetView();
    SdrModel* pModel = (SdrModel*)rView.GetModel();
    pModel->SetPaintingPageView(&mrPageView);

    ExtOutputDevice* pXOut = rView.GetExtendedOutputDevice();
    sal_Bool bTextEdit(rView.IsTextEdit() && rView.GetTextEditPageView() == &mrPageView);
    pXOut->SetOffset(mrPageView.GetOffset());

    if(bTextEdit && pId)
    {
        SdrObject* pObj = rView.GetTextEditObject();

        // do not paint TextEdit if there is no object or object is not on
        // given layer
        if(!pObj || pObj->GetLayer() != *pId)
        {
            bTextEdit = sal_False;
        }
    }

    sal_Bool bPrinter(OUTDEV_PRINTER == mrOutputDevice.GetOutDevType());
    SetOfByte aProcessLayers = bPrinter ? mrPageView.GetPrintableLayers() : mrPageView.GetVisibleLayers();

    // is the given layer visible at all?
    if(!pId || aProcessLayers.IsSet(*pId))
    {
        // force output to this one given target
        pXOut->SetOutDev(&mrOutputDevice);

        // create PaintInfoRec, #114359# use Rectangle only temporarily
        Rectangle aDirtyRectForInfo(rReg.GetBoundRect() - mrPageView.GetOffset());
        SdrPaintInfoRec* pInfoRec = ImpCreateNewPageInfoRec(aDirtyRectForInfo, nPaintMode, pId);

        // create processing data
        sdr::contact::DisplayInfo aDisplayInfo(&mrPageView);

        if(pId)
        {
            aProcessLayers.ClearAll();
            aProcessLayers.Set(*pId);
        }

        aDisplayInfo.SetProcessLayers(aProcessLayers);
        aDisplayInfo.SetExtendedOutputDevice(pXOut);
        aDisplayInfo.SetPaintInfoRec(pInfoRec);
        aDisplayInfo.SetOutputDevice(&mrOutputDevice);

        // #114359# Set region as redraw area, not a rectangle
        Region aRegionWithoutOffset(rReg);
        aRegionWithoutOffset.Move(-mrPageView.GetOffset().X(), -mrPageView.GetOffset().Y());
        aDisplayInfo.SetRedrawArea(aRegionWithoutOffset);

        if(pId)
        {
            // Writer or calc, coming from original RedrawOneLayer.
            aDisplayInfo.SetPreRenderingAllowed(sal_False);
            aDisplayInfo.SetPagePainting(sal_False);
        }
        else
        {
            // Draw/Impress
            // #114898#
            aDisplayInfo.SetPreRenderingAllowed(rView.IsBufferedOutputAllowed());
            aDisplayInfo.SetPagePainting(sal_True);
        }

        // keep draw hierarchy up-to-date
        GetObjectContact().PreProcessDisplay(aDisplayInfo);

        // paint page
        GetObjectContact().ProcessDisplay(aDisplayInfo);

        if(bTextEdit)
        {
            mrPageView.PaintOutlinerView(&mrOutputDevice, pInfoRec->aCheckRect);
        }

        // delete PaintInfoRec
        delete pInfoRec;
    }

    // reset redirector
    if(pRedirector)
    {
        GetObjectContact().SetViewObjectContactRedirector(0L);
    }

    // refresh handles
    // ?!?!if(mpIAOManager)
    // ?!?!{
    // ?!?! mpIAOManager->UpdateDisplay();
    // ?!?!}
}

// Invalidate call, used from ObjectContact(OfPageView) in InvalidatePartOfView(...)
void SdrPageViewWindow::Invalidate(const Rectangle& rRectangle)
{
    if(GetPageView().IsVisible() && OUTDEV_WINDOW == mrOutputDevice.GetOutDevType())
    {
        Rectangle aRectWithOffset(rRectangle + GetPageView().GetOffset());
        ((Window&)mrOutputDevice).Invalidate(aRectWithOffset, INVALIDATE_NOERASE);
    }
}

// #110094# ObjectContact section
sdr::contact::ObjectContact& SdrPageViewWindow::GetObjectContact() const
{
    if(!mpObjectContact)
    {
        ((SdrPageViewWindow*)this)->mpObjectContact = ((SdrPageViewWindow*)this)->CreateViewSpecificObjectContact();
    }

    return *mpObjectContact;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrPageView);

////////////////////////////////////////////////////////////////////////////////////////////////////
// interface to SdrPageViewWindow

SdrPageViewWindow* SdrPageView::FindWindow(OutputDevice& rOut) const
{
    for(SdrPageViewWindowVector::const_iterator a = maWindowVector.begin(); a != maWindowVector.end(); a++)
    {
        if(&((*a)->GetOutputDevice()) == &rOut)
        {
            return *a;
        }
    }

    return 0L;
}

SdrPageViewWindow* SdrPageView::GetWindow(sal_uInt32 nIndex) const
{
    SdrPageViewWindowVector::const_reference aObject = maWindowVector[nIndex];
    return aObject;
}

void SdrPageView::ClearWindows()
{
    while(maWindowVector.size())
    {
        SdrPageViewWindowVector::reference aLastObject = maWindowVector.back();
        maWindowVector.pop_back();
        delete aLastObject;
    }
}

void SdrPageView::AppendWindow(SdrPageViewWindow& rNew)
{
    maWindowVector.push_back(&rNew);
}

SdrPageViewWindow* SdrPageView::RemoveWindow(sal_uInt32 nPos)
{
    if(nPos < maWindowVector.size())
    {
        SdrPageViewWindowVector::iterator aAccess = maWindowVector.begin() + nPos;
        // #114376# remember return value
        SdrPageViewWindow* pErasedSdrPageViewWindow = *aAccess;
        maWindowVector.erase(aAccess);
        return pErasedSdrPageViewWindow;
    }

    return 0L;
}

SdrPageViewWindow* SdrPageView::RemoveWindow(SdrPageViewWindow& rOld)
{
    const SdrPageViewWindowVector::iterator aFindResult = ::std::find(maWindowVector.begin(), maWindowVector.end(), &rOld);

    if(aFindResult != maWindowVector.end())
    {
        // #114376# remember return value
        SdrPageViewWindow* pErasedSdrPageViewWindow = *aFindResult;
        maWindowVector.erase(aFindResult);
        return pErasedSdrPageViewWindow;
    }

    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

SdrPageView::SdrPageView(SdrPage* pPage1, const Point& rOffs, SdrView& rNewView)
:   mpDisplayInfo(0L),
    mrView(rNewView),
    aOfs(rOffs),
    // #103911# col_auto color lets the view takes the default SvxColorConfig entry
    maDocumentColor( COL_AUTO )
{
    DBG_CTOR(SdrPageView,NULL);
    pDragPoly0=new XPolyPolygon;
    pDragPoly=new XPolyPolygon;
    //pWinList=new SdrPageViewWinList;
    mpPage = pPage1;

    if(mpPage)
    {
        aPgOrg.X()=mpPage->GetLftBorder();
        aPgOrg.Y()=mpPage->GetUppBorder();
    }
    mbHasMarked = sal_False;
    aLayerVisi.SetAll();
    aLayerPrn.SetAll();

    mbVisible = sal_False;

    pAktList = NULL;
    pAktGroup = NULL;
    SetAktGroupAndList(NULL, mpPage);

    StartListening(*rNewView.GetModel());
    USHORT nWinAnz=rNewView.GetWinCount();

    for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++)
    {
        AddWindowToPageView(*rNewView.GetWin(nWinNum));
    }
}

SdrPageView::~SdrPageView()
{
    if (GetView().GetModel()->GetPaintingPageView() == this)
    {
        // Abmelden
        GetView().GetModel()->SetPaintingPageView(0L);
    }

    DBG_DTOR(SdrPageView,NULL);
    delete pDragPoly0;
    delete pDragPoly;

    // cleanup window vector
    ClearWindows();
}

SdrPageViewWindow& SdrPageView::ImpMakePageViewWinRec(OutputDevice& rOut)
{
    // MIB 3.7.08: Das WinRec muss sofort in die Liste eingetragen werden,
    // weil sich das InsertControlContainer darauf verlaesst
    //SdrPageViewWinRec* pRec = new SdrPageViewWinRec( *this, pOut );
    //pWinList->Insert(pRec);
    SdrPageViewWindow& rWindow = *(new SdrPageViewWindow(*this, rOut));
    AppendWindow(rWindow);

    ULONG nObjAnz=GetPage()!=NULL?GetPage()->GetObjCount():0;

    for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++)
    {
        SdrObject* pObj = GetPage()->GetObj(nObjNum);

        if (pObj->IsUnoObj())
        {
            SdrUnoObj* pSdrUnoObj = PTR_CAST(SdrUnoObj, pObj);
            ImpInsertControl(pSdrUnoObj, rWindow);
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRUP &&
                 pObj->GetObjInventor() == SdrInventor)
        {
            // Gruppenobjekt: sind Uno-Objekte enthalten?
            SdrObjListIter aIter(*((SdrObjGroup*) pObj)->GetSubList(), IM_DEEPNOGROUPS);

            SdrObject* pObj = NULL;

            while (aIter.IsMore())
            {
                pObj = aIter.Next();

                if (pObj && pObj->IsUnoObj())
                {
                    SdrUnoObj* pSdrUnoObj = PTR_CAST(SdrUnoObj, pObj);
                    ImpInsertControl(pSdrUnoObj, rWindow);
                }
            }
        }
    }

    return rWindow;
}

void SdrPageView::AddWindowToPageView(OutputDevice& rOut)
{
    if(!FindWindow(rOut))
    {
        ImpMakePageViewWinRec(rOut);
    }
}

void SdrPageView::DeleteWindowFromPageView(OutputDevice& rOut)
{
    SdrPageViewWindow* pCandidate = FindWindow(rOut);

    if(pCandidate)
    {
        pCandidate = RemoveWindow(*pCandidate);

        if(pCandidate)
        {
            delete pCandidate;
        }
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > SdrPageView::GetControlContainer( const OutputDevice* _pDevice )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > xReturn;
    SdrPageViewWindow* pCandidate = FindWindow(*(const_cast< OutputDevice* >(_pDevice)));

    if(pCandidate)
    {
        xReturn = xReturn.query(pCandidate->GetControlContainerRef());

        if(!xReturn.is())
        {
            pCandidate->CreateControlContainer();
            xReturn = xReturn.query(pCandidate->GetControlContainerRef());
        }
    }

    return xReturn;
}

void SdrPageView::ImpInsertControl(const SdrUnoObj* pSdrUnoObj, SdrPageViewWindow& rWindow)
{
    if (pSdrUnoObj)
    {
        uno::Reference< awt::XControlModel > xUnoControlModel(pSdrUnoObj->GetUnoControlModel());
        if (!xUnoControlModel.is())
            return;

        USHORT nCtrlNum = rWindow.GetControlList().Find(xUnoControlModel);
        if (nCtrlNum == SDRUNOCONTROL_NOTFOUND)
        {
            // Control fuer die View erzeugen
            uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
            uno::Reference< awt::XControl > xUnoControl;
            if( xFactory.is() )
                xUnoControl = uno::Reference< awt::XControl >(xFactory->createInstance(pSdrUnoObj->GetUnoControlTypeName()), uno::UNO_QUERY);

            if (xUnoControl.is())
            {
                xUnoControl->setModel(xUnoControlModel);
                if (rWindow.GetOutputDevice().GetOutDevType() != OUTDEV_WINDOW)
                {
                    uno::Reference< awt::XView > xView = xUnoControl->getView();
                    if (xView.is())
                    {
//                        xView->setGraphics(pRec->pOutDev->GetGraphicsInterface(TRUE));
                    }
                }

                // ControlContainer ggf. erzeugen
                rWindow.CreateControlContainer();

                // xControl in ControlContainer einfuegen (Eigentuemer-Uebergang)
                if (rWindow.GetControlContainerRef().is())
                {
                    uno::Reference< awt::XWindow > xWindow(xUnoControl, uno::UNO_QUERY);
                    if (xWindow.is())
                    {
                        Rectangle aRect(pSdrUnoObj->GetLogicRect());
                        OutputDevice& rOut = rWindow.GetOutputDevice();

                        Point aPixPos(rOut.LogicToPixel(aRect.TopLeft()));
                        Size aPixSize(rOut.LogicToPixel(aRect.GetSize()));

                        xWindow->setPosSize(aPixPos.X(), aPixPos.Y(), aPixSize.Width(), aPixSize.Height(), awt::PosSize::POSSIZE);
                    }

                    // set the correct zoom
                    if (!GetView().IsDesignMode())
                    {
                        uno::Reference< awt::XView > xView(xUnoControl, uno::UNO_QUERY);
                        if (xView.is())
                        {
                            const MapMode& rMap = rWindow.GetOutputDevice().GetMapMode();
                            xView->setZoom((float) double(rMap.GetScaleX()),
                                           (float) double(rMap.GetScaleY()));
                        }
                    }

                    // #107049# set design mode before peer is created,
                    // this is also needed for accessibility
                    xUnoControl->setDesignMode(GetView().IsDesignMode());
                    SdrUnoControlList& rControlList = rWindow.GetControlList();

                    SdrUnoControlRec* pUCR = new SdrUnoControlRec(&rControlList, (SdrUnoObj*)pSdrUnoObj, xUnoControl);
                    rControlList.Insert(pUCR);

                    // #108327# do this last - the addition of the control triggeres processes which rely
                    // on the control already being inserted into the aControlList
                    rWindow.GetControlContainerRef()->addControl(pSdrUnoObj->GetUnoControlTypeName(), xUnoControl);
                }
            }
        }
    }
}

void SdrPageView::ImpUnoInserted(const SdrObject* pObj)
{
    SdrUnoObj* pSdrUnoObj = (SdrUnoObj*)pObj;
    sal_uInt32 nCount(WindowCount());

    for(;nCount;)
    {
        SdrPageViewWindow& rWindow = *GetWindow(--nCount);
        ImpInsertControl(pSdrUnoObj, rWindow);
    }

    //USHORT nPos = pWinList->GetCount();
    //for (; nPos; )
    //  ImpInsertControl(pSdrUnoObj, &(*pWinList)[--nPos]);
}

void SdrPageView::ImpUnoRemoved(const SdrObject* pObj)
{
    SdrUnoObj* pSdrUno = (SdrUnoObj*)pObj;
    sal_uInt32 nCount(WindowCount());

    for(;nCount;)
    {
        const SdrPageViewWindow& rWindow = *GetWindow(--nCount);
        sal_uInt16 nControlPos = rWindow.GetControlList().Find(pSdrUno->GetUnoControlModel());

        if(SDRUNOCONTROL_NOTFOUND != nControlPos)
        {
            SdrUnoControlList& rControlList = rWindow.GetControlList();
            rControlList.Delete(nControlPos, sal_True);
        }
    }
}

void __EXPORT SdrPageView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    if(IsVisible())
    {
        SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
        if (pSdrHint!=NULL) {
            SdrHintKind eKind=pSdrHint->GetKind();
            const SdrObject* pObj=pSdrHint->GetObject();
            if ( pObj!=NULL && pObj->GetPage() == GetPage() )
            {
                if (pObj->IsUnoObj())
                {
                    if (eKind == HINT_OBJINSERTED ||
                        eKind == HINT_CONTROLINSERTED)
                    {
                        ImpUnoInserted(pObj);
                    }
                    else if (eKind == HINT_OBJREMOVED ||
                             eKind == HINT_CONTROLREMOVED)

                    {
                        ImpUnoRemoved(pObj);
                    }
                }
                else if (pObj->GetObjIdentifier() == OBJ_GRUP &&
                         pObj->GetObjInventor() == SdrInventor)
                {
                    // Gruppenobjekt: sind Uno-Objekte enthalten?
                    SdrObjListIter aIter(*((SdrObjGroup*) pObj)->GetSubList(), IM_DEEPNOGROUPS);

                    SdrObject* pObj = NULL;

                    while (aIter.IsMore())
                    {
                        pObj = aIter.Next();

                        if (pObj && pObj->IsUnoObj())
                        {
                            if (eKind == HINT_OBJINSERTED ||
                                eKind == HINT_CONTROLINSERTED)

                            {
                                ImpUnoInserted(pObj);
                            }
                            else if (eKind == HINT_OBJREMOVED ||
                                     eKind == HINT_CONTROLREMOVED)

                            {
                                ImpUnoRemoved(pObj);
                            }
                        }
                    }
                }
            }
        }
    }
}

void SdrPageView::ModelHasChanged()
{
    if (GetAktGroup()!=NULL) CheckAktGroup();
}

sal_Bool SdrPageView::IsReadOnly() const
{
    return (0L == GetPage() || GetView().GetModel()->IsReadOnly() || GetPage()->IsReadOnly() || GetObjList()->IsReadOnly());
}

void SdrPageView::Show()
{
    if(!IsVisible())
    {
        mbVisible = sal_True;
        InvalidateAllWin();
        USHORT nWinAnz = GetView().GetWinCount();

        for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++)
        {
            // AddWin(rView.GetWin(nWinNum));
            AddWindowToPageView(*GetView().GetWin(nWinNum));
        }
    }
}

void SdrPageView::Hide()
{
    if(IsVisible())
    {
        InvalidateAllWin();
        mbVisible = sal_False;

        //pWinList->Clear();
        ClearWindows();
    }
}

void SdrPageView::SetOffset(const Point& rPnt)
{
    if (aOfs!=rPnt)
    {
        if(IsVisible())
        {
            InvalidateAllWin();
        }

        aOfs = rPnt;

        if(IsVisible())
        {
            InvalidateAllWin();
        }
    }
}

Rectangle SdrPageView::GetPageRect() const
{
    if (GetPage()==NULL) return Rectangle();
    return Rectangle(GetOffset(),Size(GetPage()->GetWdt()+1,GetPage()->GetHgt()+1));
}

void SdrPageView::InvalidateAllWin()
{
    if(IsVisible() && GetPage())
    {
        Rectangle aRect(Point(0,0),Size(GetPage()->GetWdt()+1,GetPage()->GetHgt()+1));
        aRect.Union(GetPage()->GetAllObjBoundRect());
        aRect.Move(aOfs.X(),aOfs.Y());
        GetView().InvalidateAllWin(aRect);
    }
}

void SdrPageView::InvalidateAllWin(const Rectangle& rRect, sal_Bool bPlus1Pix)
{
    if(IsVisible())
    {
        GetView().InvalidateAllWin(rRect + GetOffset(), bPlus1Pix);
    }
}

void SdrPageView::PaintOutlinerView(OutputDevice* pOut, const Rectangle& rRect) const
{
    if (GetView().pTextEditOutliner==NULL) return;
    const SdrObject* pTextObjTmp=GetView().GetTextEditObject();
    const SdrTextObj* pText=PTR_CAST(SdrTextObj,pTextObjTmp);
    FASTBOOL bTextFrame=pText!=NULL && pText->IsTextFrame();
    ULONG nViewAnz=GetView().pTextEditOutliner->GetViewCount();
    for (ULONG i=0; i<nViewAnz; i++) {
        OutlinerView* pOLV=GetView().pTextEditOutliner->GetView(i);
        if (pOLV->GetWindow()==pOut) {
            GetView().ImpPaintOutlinerView(*pOLV,&rRect);
            return;
        }
    }
}

void SdrPageView::CompleteRedraw(OutputDevice* pGivenTarget, const Region& rReg, sal_uInt16 nPaintMode,
    ::sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    if(GetPage())
    {
        if(pGivenTarget)
        {
            SdrPageViewWindow* pKnownTarget = FindWindow(*pGivenTarget);

            if(pKnownTarget)
            {
                // paint known target
                pKnownTarget->Redraw(rReg, nPaintMode, 0L, pRedirector);
            }
            else
            {
                // None of the known OutputDevices is the target of this paint, use
                // a temporary SdrPageViewWindow for this Redraw.
                SdrPageViewWindow aTemporaryPageViewWindow(*((SdrPageView*)this), *pGivenTarget);

                aTemporaryPageViewWindow.Redraw(rReg, nPaintMode, 0L, pRedirector);
            }
        }
        else
        {
            // paint in all known windows
            for(sal_uInt32 a(0L); a < WindowCount(); a++)
            {
                SdrPageViewWindow* pTarget = GetWindow(a);
                pTarget->Redraw(rReg, nPaintMode, 0L, pRedirector);
            }
        }
    }

    mrView.RefreshAllIAOManagers();
    mrView.RestartAfterPaintTimer();
}

void SdrPageView::DrawLayer(SdrLayerID nId, const Rectangle& rRect, OutputDevice* pGivenTarget, sal_uInt16 nPaintMode) const
{
    if(GetPage())
    {
        if(pGivenTarget)
        {
            SdrPageViewWindow* pKnownTarget = FindWindow(*pGivenTarget);

            if(pKnownTarget)
            {
                // paint known target
                Region aRedrawRegion(rRect);
                pKnownTarget->Redraw(aRedrawRegion, nPaintMode, &nId);
            }
            else
            {
                // None of the known OutputDevices is the target of this paint, use
                // a temporary SdrPageViewWindow for this Redraw.
                SdrPageViewWindow aTemporaryPageViewWindow(*((SdrPageView*)this), *pGivenTarget);
                Region aRedrawRegion(rRect);

                aTemporaryPageViewWindow.Redraw(aRedrawRegion, nPaintMode, &nId);
            }
        }
        else
        {
            // paint in all known windows
            Region aRedrawRegion(rRect);

            for(sal_uInt32 a(0L); a < WindowCount(); a++)
            {
                SdrPageViewWindow* pTarget = GetWindow(a);
                pTarget->Redraw(aRedrawRegion, nPaintMode, &nId);
            }
        }
    }

    // Do not call RefreshAllIAOManagers here since then it would be called
    // more then once in SW and SC Paint()s (which would be bad). Look there,
    // where this gets called. In principle do this once at the end of the paint.
    // These Ends may be found when looking for DrawGrid() in those apps.
    //
    // mrView.RefreshAllIAOManagers();
    mrView.RestartAfterPaintTimer();
}

#ifdef OS2
#define RGBCOLOR(r,g,b) ((ULONG)(((BYTE)(b) | ((USHORT)(g)<<8)) | (((ULONG)(BYTE)(r))<<16)))
#endif

void SdrPageView::DrawGrid(OutputDevice& rOut, const Rectangle& rRect, Color aColor)
{
    if (GetPage()==NULL)
        return;

    long nx1=GetView().aGridBig.Width();
    long nx2=GetView().aGridFin.Width();
    long ny1=GetView().aGridBig.Height();
    long ny2=GetView().aGridFin.Height();

    if (nx1==0) nx1=nx2;
    if (nx2==0) nx2=nx1;
    if (ny1==0) ny1=ny2;
    if (ny2==0) ny2=ny1;
    if (nx1==0) { nx1=ny1; nx2=ny2; }
    if (ny1==0) { ny1=nx1; ny2=nx2; }
    if (nx1<0) nx1=-nx1;
    if (nx2<0) nx2=-nx2;
    if (ny1<0) ny1=-ny1;
    if (ny2<0) ny2=-ny2;

    if (nx1!=0)
    {
        // no more global output size, use window size instead to decide grid sizes
        long nScreenWdt = rOut.GetOutputSizePixel().Width();
        // old: long nScreenWdt=System::GetDesktopRectPixel().GetWidth();

        // Grid bei kleinen Zoomstufen etwas erweitern
        //Size a1PixSiz(rOut.PixelToLogic(Size(1,1)));
        long nMinDotPix=2;
        long nMinLinPix=4;

        if (nScreenWdt>=1600)
        {
            nMinDotPix=4;
            nMinLinPix=8;
        }
        else if (nScreenWdt>=1024)
        {
            nMinDotPix=3;
            nMinLinPix=6;
        }
        else
        { // z.B. 640x480
            nMinDotPix=2;
            nMinLinPix=4;
        }
        Size aMinDotDist(rOut.PixelToLogic(Size(nMinDotPix,nMinDotPix)));
        //Size a3PixSiz(rOut.PixelToLogic(Size(2,2)));
        Size aMinLinDist(rOut.PixelToLogic(Size(nMinLinPix,nMinLinPix)));
        FASTBOOL bHoriSolid=nx2<aMinDotDist.Width();
        FASTBOOL bVertSolid=ny2<aMinDotDist.Height();
        // Linienabstand vergroessern (mind. 4 Pixel)
        // Vergroesserung: *2 *5 *10 *20 *50 *100 ...
        int nTgl=0;
        long nVal0=nx1;
        while (nx1<aMinLinDist.Width())
        {
            long a=nx1;

            if (nTgl==0) nx1*=2;
            if (nTgl==1) nx1=nVal0*5; // => nx1*=2.5
            if (nTgl==2) nx1*=2;

            nVal0=a;
            nTgl++; if (nTgl>=3) nTgl=0;
        }
        nTgl=0;
        nVal0=ny1;
        while (ny1<aMinLinDist.Height())
        {
            long a=ny1;

            if (nTgl==0) ny1*=2;
            if (nTgl==1) ny1=nVal0*5; // => ny1*=2.5
            if (nTgl==2) ny1*=2;

            nVal0=a;
            nTgl++;

            if (nTgl>=3) nTgl=0;
        }
        // Keine Zwischenpunkte, wenn...
        //if (nx2<a2PixSiz.Width()) nx2=nx1;
        //if (ny2<a2PixSiz.Height()) ny2=ny1;

        FASTBOOL bHoriFine=nx2<nx1;
        FASTBOOL bVertFine=ny2<ny1;
        FASTBOOL bHoriLines=bHoriSolid || bHoriFine || !bVertFine;
        FASTBOOL bVertLines=bVertSolid || bVertFine;

        Color aColorMerk( rOut.GetLineColor() );
        rOut.SetLineColor( aColor );

        FASTBOOL bMap0=rOut.IsMapModeEnabled();
#ifdef WIN // SetPixel-Profiling fuer Windows
       COLORREF aWinColRef=PALETTERGB(aColor.GetRed()>>8,aColor.GetGreen()>>8,aColor.GetBlue()>>8);
       HDC aWinhDC=Sysdepen::GethDC(rOut);
#endif
#ifdef OS2 // SetPixel-Profiling fuer OS/2
        HPS aOS2hPS=Sysdepen::GethPS(rOut);
        LINEBUNDLE aOS2BundleMerker;
        memset(&aOS2BundleMerker,0,sizeof(aOS2BundleMerker));
        GpiQueryAttrs(aOS2hPS,PRIM_LINE,LBB_COLOR,&aOS2BundleMerker);
        LINEBUNDLE aOS2Bundle;
        memset(&aOS2Bundle,0,sizeof(aOS2Bundle));
        aOS2Bundle.lColor=RGBCOLOR(aColor.GetRed()>>8,aColor.GetGreen()>>8,aColor.GetBlue()>>8);
        GpiSetAttrs(aOS2hPS,PRIM_LINE,LBB_COLOR,0,&aOS2Bundle);
        long nOS2MaxYPix=rOut.GetOutputSizePixel().Height()-1;
#endif

        //Point aWriterPageOffset(pPage->GetOffset());
        long nWrX=0;//aWriterPageOffset.X();
        long nWrY=0;//aWriterPageOffset.Y();
        Point aOrg(aPgOrg);
        long xo=aOfs.X();
        long yo=aOfs.Y();
        long x1=GetPage()->GetLftBorder()+1+nWrX;
        long x2=GetPage()->GetWdt()-GetPage()->GetRgtBorder()-1+nWrY;
        long y1=GetPage()->GetUppBorder()+1+nWrX;
        long y2=GetPage()->GetHgt()-GetPage()->GetLwrBorder()-1+nWrY;
        const SdrPageGridFrameList* pFrames=GetPage()->GetGridFrameList(this,NULL);
        USHORT nBufSiz=1024; // 4k Buffer = max. 512 Punkte
        // #90353# long* pBuf = NULL;
        unsigned nGridPaintAnz=1;
        if (pFrames!=NULL) nGridPaintAnz=pFrames->GetCount();
        for (unsigned nGridPaintNum=0; nGridPaintNum<nGridPaintAnz; nGridPaintNum++) {
            if (pFrames!=NULL) {
                const SdrPageGridFrame& rGF=(*pFrames)[nGridPaintNum];
                nWrX=rGF.GetPaperRect().Left();
                nWrY=rGF.GetPaperRect().Top();
                x1=rGF.GetUserArea().Left();
                x2=rGF.GetUserArea().Right();
                y1=rGF.GetUserArea().Top();
                y2=rGF.GetUserArea().Bottom();
                aOrg=rGF.GetUserArea().TopLeft();
                aOrg-=rGF.GetPaperRect().TopLeft();
            }
            if (!rRect.IsEmpty()) {
                Size a1PixSiz(rOut.PixelToLogic(Size(1,1)));
                long nX1Pix=a1PixSiz.Width();  // 1 Pixel Toleranz drauf
                long nY1Pix=a1PixSiz.Height();
                if (x1<rRect.Left()  -nX1Pix/*-nWrX/**/) x1=rRect.Left()  -nX1Pix/*-nWrX/**/;
                if (x2>rRect.Right() +nX1Pix/*-nWrX/**/) x2=rRect.Right() +nX1Pix/*-nWrX/**/;
                if (y1<rRect.Top()   -nY1Pix/*-nWrY/**/) y1=rRect.Top()   -nY1Pix/*-nWrY/**/;
                if (y2>rRect.Bottom()+nY1Pix/*-nWrY/**/) y2=rRect.Bottom()+nY1Pix/*-nWrY/**/;
            }
            Point aPnt;

            long xBigOrg=aOrg.X()+nWrX;
            while (xBigOrg>=x1) xBigOrg-=nx1;
            while (xBigOrg<x1) xBigOrg+=nx1;
            long xFinOrg=xBigOrg;
            while (xFinOrg>=x1) xFinOrg-=nx2;
            while (xFinOrg<x1) xFinOrg+=nx2;

            long yBigOrg=aOrg.Y()+nWrY;
            while (yBigOrg>=y1) yBigOrg-=ny1;
            while (yBigOrg<y1) yBigOrg+=ny1;
            long yFinOrg=yBigOrg;
            while (yFinOrg>=y1) yFinOrg-=ny2;
            while (yFinOrg<y1) yFinOrg+=ny2;

            if( x1 <= x2 && y1 <= y2 )
            {
                if( bHoriLines )
                {
                    ULONG nGridFlags = ( bHoriSolid ? GRID_HORZLINES : GRID_DOTS );
                    UINT16 nSteps = sal_uInt16(nx1 / nx2);
                    UINT32 nRestPerStepMul1000 = nSteps ? ( ((nx1 * 1000L)/ nSteps) - (nx2 * 1000L) ) : 0;
                    UINT32 nStepOffset = 0;
                    UINT16 nPointOffset = 0;

                    for(UINT16 a=0;a<nSteps;a++)
                    {
                        // Zeichnen
                        rOut.DrawGrid(
                            Rectangle( xo + xFinOrg + (a * nx2) + nPointOffset, yo + yBigOrg, x2, y2 ),
                            Size( nx1, ny1 ), nGridFlags );

                        // Schritt machen
                        nStepOffset += nRestPerStepMul1000;
                        while(nStepOffset >= 1000)
                        {
                            nStepOffset -= 1000;
                            nPointOffset++;
                        }
                    }

                    // rOut.DrawGrid( Rectangle( xo + xFinOrg, yo + yBigOrg, x2, y2 ), Size( nx2, ny1 ), nGridFlags );
                }

                if( bVertLines )
                {
                    ULONG nGridFlags = ( bVertSolid ? GRID_VERTLINES : GRID_DOTS );
                    UINT16 nSteps = sal_uInt16(ny1 / ny2);
                    UINT32 nRestPerStepMul1000 = nSteps ? ( ((ny1 * 1000L)/ nSteps) - (ny2 * 1000L) ) : 0;
                    UINT32 nStepOffset = 0;
                    UINT16 nPointOffset = 0;

                    for(UINT16 a=0;a<nSteps;a++)
                    {
                        // Zeichnen
                        rOut.DrawGrid(
                            Rectangle( xo + xBigOrg, yo + yFinOrg + (a * ny2) + nPointOffset, x2, y2 ),
                            Size( nx1, ny1 ), nGridFlags );

                        // Schritt machen
                        nStepOffset += nRestPerStepMul1000;
                        while(nStepOffset >= 1000)
                        {
                            nStepOffset -= 1000;
                            nPointOffset++;
                        }
                    }

                    // rOut.DrawGrid( Rectangle( xo + xBigOrg, yo + yFinOrg, x2, y2 ), Size( nx1, ny2 ), nGridFlags );
                }
            }
        }
        // #90353# delete [] pBuf;
        rOut.EnableMapMode(bMap0);
        rOut.SetLineColor(aColorMerk);
#ifdef OS2 // SetPixel-Profiling fuer OS/2
        // OS2-LineAttribute restaurieren
        GpiSetAttrs(aOS2hPS,PRIM_LINE,LBB_COLOR,0,&aOS2BundleMerker);
#endif
    }
}

void SdrPageView::AdjHdl()
{
    GetView().AdjustMarkHdl();
}

void SdrPageView::SetLayer(const XubString& rName, SetOfByte& rBS, sal_Bool bJa)
{
    if(!GetPage())
        return;

    SdrLayerID nID = GetPage()->GetLayerAdmin().GetLayerID(rName, sal_True);

    if(SDRLAYER_NOTFOUND != nID)
    {
        // #116657#
        // Remember if change took place
        sal_Bool bChangeTookPlace(bJa != rBS.IsSet(nID));

        rBS.Set(nID, bJa);

        // #116657#
        // If yes, let the UnoControlRec update to evtl. visibility change
        if(bChangeTookPlace)
        {
            for(sal_uInt32 i(0L); i < WindowCount(); i++)
            {
                SdrPageViewWindow& rPageViewWindow = *GetWindow(i);
                SdrUnoControlList& rControlList = rPageViewWindow.GetControlList();

                for(sal_uInt32 j(0L); j < rControlList.GetCount(); j++)
                {
                    SdrUnoControlRec& rControlRec = rControlList[(sal_uInt16) j];
                    rControlRec.adjustControlVisibility( false );
                }
            }
        }
    }
}

sal_Bool SdrPageView::IsLayer(const XubString& rName, const SetOfByte& rBS) const
{
    if(!GetPage())
        return sal_False;

    sal_Bool bRet(sal_False);

    if(rName.Len())
    {
        SdrLayerID nId = GetPage()->GetLayerAdmin().GetLayerID(rName, sal_True);

        if(SDRLAYER_NOTFOUND != nId)
        {
            bRet = rBS.IsSet(nId);
        }
    }

    return bRet;
}

void SdrPageView::SetAllLayers(SetOfByte& rB, sal_Bool bJa)
{
    if(bJa)
    {
        rB.SetAll();
        rB.Clear(SDRLAYER_NOTFOUND);
    }
    else
    {
        rB.ClearAll();
    }
}

sal_Bool SdrPageView::IsObjMarkable(SdrObject* pObj) const
{
    if(pObj)
    {
        // Vom Markieren ausgeschlossen?
        if(pObj->IsMarkProtect())
        {
            return sal_False;
        }

        // #112440#
        if(pObj->ISA(SdrObjGroup))
        {
            // If object is a Group object, visibility depends evtl. on
            // multiple layers. If one object is markable, Group is markable.
            SdrObjList* pObjList = ((SdrObjGroup*)pObj)->GetSubList();
            sal_Bool bGroupIsMarkable(sal_False);

            for(sal_uInt32 a(0L); !bGroupIsMarkable && a < pObjList->GetObjCount(); a++)
            {
                SdrObject* pCandidate = pObjList->GetObj(a);

                // call recursively
                if(IsObjMarkable(pCandidate))
                {
                    bGroupIsMarkable = sal_True;
                }
            }

            return bGroupIsMarkable;
        }
        else
        {
            // Der Layer muss sichtbar und darf nicht gesperrt sein
            SdrLayerID nL = pObj->GetLayer();
            return (aLayerVisi.IsSet(BYTE(nL)) && !aLayerLock.IsSet(BYTE(nL)));
        }
    }

    return sal_False;
}

void SdrPageView::SetPageOrigin(const Point& rOrg)
{
    if (rOrg!=aPgOrg) {
        aPgOrg=rOrg;
        if (GetView().IsGridVisible()) {
            InvalidateAllWin();
        }
    }
}

void SdrPageView::ImpInvalidateHelpLineArea(USHORT nNum) const
{
    if (GetView().IsHlplVisible() && nNum<aHelpLines.GetCount()) {
        const SdrHelpLine& rHL=aHelpLines[nNum];
        for (USHORT i=0; i<GetView().GetWinCount(); i++) {
            OutputDevice* pOut=GetView().GetWin(i);
            if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
                Rectangle aR(rHL.GetBoundRect(*pOut));
                Size aSiz(pOut->PixelToLogic(Size(1,1)));
                aR.Left  ()-=aSiz.Width();
                aR.Right ()+=aSiz.Width();
                aR.Top   ()-=aSiz.Height();
                aR.Bottom()+=aSiz.Height();
                aR.Move(aOfs.X(),aOfs.Y());
                ((SdrView&)GetView()).InvalidateOneWin(*(Window*)pOut,aR);
            }
        }
    }
}

void SdrPageView::SetHelpLines(const SdrHelpLineList& rHLL)
{
    aHelpLines=rHLL;
    InvalidateAllWin();
}

void SdrPageView::SetHelpLine(USHORT nNum, const SdrHelpLine& rNewHelpLine)
{
    if (nNum<aHelpLines.GetCount() && aHelpLines[nNum]!=rNewHelpLine) {
        FASTBOOL bNeedRedraw=TRUE;
        if (aHelpLines[nNum].GetKind()==rNewHelpLine.GetKind()) {
            switch (rNewHelpLine.GetKind()) {
                case SDRHELPLINE_VERTICAL  : if (aHelpLines[nNum].GetPos().X()==rNewHelpLine.GetPos().X()) bNeedRedraw=FALSE; break;
                case SDRHELPLINE_HORIZONTAL: if (aHelpLines[nNum].GetPos().Y()==rNewHelpLine.GetPos().Y()) bNeedRedraw=FALSE; break;
            } // switch
        }
        if (bNeedRedraw) ImpInvalidateHelpLineArea(nNum);
        aHelpLines[nNum]=rNewHelpLine;
        if (bNeedRedraw) ImpInvalidateHelpLineArea(nNum);
    }
}

void SdrPageView::DeleteHelpLine(USHORT nNum)
{
    if (nNum<aHelpLines.GetCount()) {
        ImpInvalidateHelpLineArea(nNum);
        aHelpLines.Delete(nNum);
    }
}

void SdrPageView::InsertHelpLine(const SdrHelpLine& rHL, USHORT nNum)
{
    if (nNum>aHelpLines.GetCount()) nNum=aHelpLines.GetCount();
    aHelpLines.Insert(rHL,nNum);
    if (GetView().IsHlplVisible()) {
        if (GetView().IsHlplFront()) {
            // Hier optimieren ...
            ImpInvalidateHelpLineArea(nNum);
         } else {
            ImpInvalidateHelpLineArea(nNum);
        }
    }
}

// Betretene Gruppe und Liste setzen
void SdrPageView::SetAktGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList)
{
    if(pAktGroup != pNewGroup)
    {
        pAktGroup = pNewGroup;
    }
    if(pAktList != pNewList)
    {
        pAktList = pNewList;
    }
}

sal_Bool SdrPageView::EnterGroup(SdrObject* pObj)
{
    sal_Bool bRet(sal_False);

    if(pObj && pObj->IsGroupObject())
    {
        // sal_Bool bDisInvalidate(rView.pDisabledAttr != 0L);
        sal_Bool bGlueInvalidate(/*!bDisInvalidate &&*/ GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }

        // deselect all
        GetView().UnmarkAll();

        // set current group and list
        SdrObjList* pNewObjList = pObj->GetSubList();
        SetAktGroupAndList(pObj, pNewObjList);

        // select contained object if only one object is contained,
        // else select nothing and let the user decide what to do next
        if(pNewObjList && pNewObjList->GetObjCount() == 1)
        {
            SdrObject* pFirstObject = pNewObjList->GetObj(0L);

            for(UINT16 nv = 0; nv < GetView().GetPageViewCount(); nv++)
            {
                GetView().MarkObj(pFirstObject, GetView().GetPageViewPvNum(nv));
            }
        }

        // build new handles
        GetView().AdjustMarkHdl();

        // invalidate only when view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
        {
            InvalidateAllWin();
        }

        if (bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }

        bRet = sal_True;
    }

    return bRet;
}

void SdrPageView::LeaveOneGroup()
{
    if(GetAktGroup())
    {
        // BOOL bDisInvalidate = (rView.pDisabledAttr != NULL);
        BOOL bGlueInvalidate = (/*!bDisInvalidate &&*/ GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
            GetView().GlueInvalidate();

        SdrObject* pLastGroup = GetAktGroup();
        SdrObject* pParentGroup = GetAktGroup()->GetUpGroup();
        SdrObjList* pParentList = GetPage();

        if(pParentGroup)
            pParentList = pParentGroup->GetSubList();

        // Alles deselektieren
        GetView().UnmarkAll();

        // Zuweisungen, pAktGroup und pAktList muessen gesetzt sein
        SetAktGroupAndList(pParentGroup, pParentList);

        // gerade verlassene Gruppe selektieren
        if(pLastGroup)
            for(UINT16 nv = 0; nv < GetView().GetPageViewCount(); nv++)
                GetView().MarkObj(pLastGroup, GetView().GetPageViewPvNum(nv));

        GetView().AdjustMarkHdl();

        // invalidate only when view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
            InvalidateAllWin();

        if(bGlueInvalidate)
            GetView().GlueInvalidate();
    }
}

void SdrPageView::LeaveAllGroup()
{
    if(GetAktGroup())
    {
        // BOOL bDisInvalidate = (rView.pDisabledAttr != NULL);
        BOOL bGlueInvalidate = (/*!bDisInvalidate &&*/ GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
            GetView().GlueInvalidate();

        SdrObject* pLastGroup = GetAktGroup();

        // Alles deselektieren
        GetView().UnmarkAll();

        // Zuweisungen, pAktGroup und pAktList muessen gesetzt sein
        SetAktGroupAndList(NULL, GetPage());

        // Oberste letzte Gruppe finden und selektieren
        if(pLastGroup)
        {
            while(pLastGroup->GetUpGroup())
                pLastGroup = pLastGroup->GetUpGroup();

            for(UINT16 nv = 0; nv < GetView().GetPageViewCount(); nv++)
                GetView().MarkObj(pLastGroup, GetView().GetPageViewPvNum(nv));
        }

        GetView().AdjustMarkHdl();

        // invalidate only when view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
            InvalidateAllWin();

        if(bGlueInvalidate)
            GetView().GlueInvalidate();
    }
}

USHORT SdrPageView::GetEnteredLevel() const
{
    USHORT nAnz=0;
    SdrObject* pGrp=GetAktGroup();
    while (pGrp!=NULL) {
        nAnz++;
        pGrp=pGrp->GetUpGroup();
    }
    return nAnz;
}

XubString SdrPageView::GetActualGroupName() const
{
    if(GetAktGroup())
    {
        XubString aStr(GetAktGroup()->GetName());

        if(!aStr.Len())
            aStr += sal_Unicode('?');

        return aStr;
    }
    else
        return String();
}

XubString SdrPageView::GetActualPathName(sal_Unicode cSep) const
{
    XubString aStr;
    BOOL bNamFnd(FALSE);
    SdrObject* pGrp = GetAktGroup();

    while(pGrp)
    {
        XubString aStr1(pGrp->GetName());

        if(!aStr1.Len())
            aStr1 += sal_Unicode('?');
        else
            bNamFnd = TRUE;

        aStr += aStr1;
        pGrp = pGrp->GetUpGroup();

        if(pGrp)
            aStr += cSep;
    }

    if(!bNamFnd && GetAktGroup())
    {
        aStr = String();
        aStr += sal_Unicode('(');
        aStr += String::CreateFromInt32( GetEnteredLevel() );
        aStr += sal_Unicode(')');
    }

    return aStr;
}

void SdrPageView::CheckAktGroup()
{
    SdrObject* pGrp=GetAktGroup();
    while (pGrp!=NULL &&
           (!pGrp->IsInserted() || pGrp->GetObjList()==NULL ||
            pGrp->GetPage()==NULL || pGrp->GetModel()==NULL)) { // irgendwas daneben?
        pGrp=pGrp->GetUpGroup();
    }
    if (pGrp!=GetAktGroup()) {
        if (pGrp!=NULL) EnterGroup(pGrp);
        else LeaveAllGroup();
    }
}

SvStream& operator<<(SvStream& rOut, const SdrPageView& rPageView)
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOPgVwID);
    {
        if (rPageView.GetPage()!=NULL) {
            SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_PAGVIEW);
            rOut<<BOOL(rPageView.IsVisible());
            rOut<<BOOL(rPageView.GetPage()->IsMasterPage());
            rOut<<rPageView.GetPage()->GetPageNum();
            rOut<<rPageView.aOfs;
            rOut<<rPageView.aPgOrg;
        }
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_PAGVLAYER);
        rOut<<rPageView.aLayerVisi;
        rOut<<rPageView.aLayerLock;
        rOut<<rPageView.aLayerPrn;
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_PAGVHELPLINES);
        rOut<<rPageView.aHelpLines;
    }
    //if (GetAktGroup()!=NULL) {
    //    // ...
    //    //rOut<<aAktGroup;
    //}
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrPageView& rPageView)
{
    if (rIn.GetError()!=0) return rIn;
    SdrIOHeader aHead(rIn,STREAM_READ);
    while (aHead.GetBytesLeft()>0 && rIn.GetError()==0 && !rIn.IsEof()) {
        SdrNamedSubRecord aSubRecord(rIn,STREAM_READ);
        if (aSubRecord.GetInventor()==SdrInventor) {
            switch (aSubRecord.GetIdentifier()) {
                case SDRIORECNAME_PAGVIEW: {
                    sal_Bool bVisible;
                    BOOL bMaster;
                    USHORT nPgNum;
                    rIn>>bVisible;
                    rPageView.mbVisible = bVisible;
                    rIn>>bMaster;
                    rIn>>nPgNum;
                    rIn>>rPageView.aOfs;
                    rIn>>rPageView.aPgOrg;
                    SdrModel* pMod=rPageView.GetView().GetModel();
                    if (!bMaster) rPageView.mpPage=pMod->GetPage(nPgNum);
                    else rPageView.mpPage=pMod->GetMasterPage(nPgNum);
                    rPageView.pAktList=rPageView.GetPage();
                } break;
                case SDRIORECNAME_PAGVLAYER: {
                    rIn>>rPageView.aLayerVisi;
                    rIn>>rPageView.aLayerLock;
                    rIn>>rPageView.aLayerPrn;
                } break;
                case SDRIORECNAME_PAGVHELPLINES: {
                    rIn>>rPageView.aHelpLines;
                } break;
                case SDRIORECNAME_PAGVAKTGROUP: {
                    //rIn>>aAktGroup; fehlende Implementation!
                } break;
            }
        }
    }
    return rIn;
}

// #103834# Set background color for svx at SdrPageViews
void SdrPageView::SetApplicationBackgroundColor(Color aBackgroundColor)
{
    maBackgroundColor = aBackgroundColor;
}

// #109585#
Color SdrPageView::GetApplicationBackgroundColor() const
{
    return maBackgroundColor;
}

// #103911# Set document color for svx at SdrPageViews
void SdrPageView::SetApplicationDocumentColor(Color aDocumentColor)
{
    maDocumentColor = aDocumentColor;
}

Color SdrPageView::GetApplicationDocumentColor() const
{
    return maDocumentColor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
