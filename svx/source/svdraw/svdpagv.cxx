/*************************************************************************
 *
 *  $RCSfile: svdpagv.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-27 13:27:41 $
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

#pragma hdrstop

using namespace ::rtl;
using namespace ::com::sun::star;

TYPEINIT1(SdrPageView, SfxListener);


// Klasse muﬂ als listener fungieren, um den Zustand, ob ein Object sichtbar ist oder nicht
// festzuhalten
//------------------------------------------------------------------------------
SdrUnoControlRec::SdrUnoControlRec(SdrUnoControlList* _pParent, SdrUnoObj* _pObj, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > _xControl) throw()
                 :pObj(_pObj)
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
            ((Window*)pOut)->Invalidate(pObj->GetBoundRect());
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
void SdrPageViewWinList::Clear()
{
    USHORT nAnz=GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

USHORT SdrPageViewWinList::Find(OutputDevice* pOut) const
{
    USHORT nAnz=GetCount();
    USHORT nRet=SDRPAGEVIEWWIN_NOTFOUND;
    for (USHORT nNum=0; nNum<nAnz && nRet==SDRPAGEVIEWWIN_NOTFOUND; nNum++) {
        if (GetObject(nNum)->MatchOutputDevice(pOut))
            nRet=nNum;
    }
    return nRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
SdrPageViewWinRec::SdrPageViewWinRec(SdrPageView& rNewPageView, OutputDevice* pOut)
    :rView( rNewPageView.GetView() )
    ,pOutDev( pOut )
    ,aControlList( rNewPageView )
{
}

SdrPageViewWinRec::~SdrPageViewWinRec()
{
    if (xControlContainer.is())
    {
        // notify derived views
        rView.RemoveControlContainer(xControlContainer);

        // clear the control place holders
        aControlList.Clear(FALSE);

        // dispose the control container
        uno::Reference< lang::XComponent > xComponent(xControlContainer, uno::UNO_QUERY);
        xComponent->dispose();
    }
}

void SdrPageViewWinRec::CreateControlContainer()
{
    if (!xControlContainer.is())
    {
        if (pOutDev && pOutDev->GetOutDevType() == OUTDEV_WINDOW &&
            !rView.IsPrintPreview())
        {
            Window* pWindow = (Window*) pOutDev;
            xControlContainer = VCLUnoHelper::CreateControlContainer( pWindow );

            // #100394# xC->setVisible triggers window->Show() and this has
            // problems when the view is not completely constructed which may
            // happen when loading. This leads to accessibility broadcasts which
            // throw asserts due to the not finished view. All this chan be avoided
            // since xC->setVisible is here called only for the side effect in
            // UnoControlContainer::setVisible(...) which calls createPeer(...).
            // This will now be called directly from here.

            // UnoContainerModel erzeugen
            // uno::Reference< awt::XWindow > xC(xControlContainer, uno::UNO_QUERY);
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

            uno::Reference< awt::XControl > xControl(xControlContainer, uno::UNO_QUERY);
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
                xControlContainer = uno::Reference< awt::XControlContainer >(xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainer")), uno::UNO_QUERY);
                uno::Reference< awt::XControlModel > xModel(xFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainerModel")), uno::UNO_QUERY);
                uno::Reference< awt::XControl > xControl(xControlContainer, uno::UNO_QUERY);
                if (xControl.is())
                    xControl->setModel(xModel);

                Point aPosPix;
                Size aSizePix;

                if ( pOutDev )
                {
                    aPosPix = pOutDev->GetMapMode().GetOrigin();
                    aSizePix = pOutDev->GetOutputSizePixel();
                }

                uno::Reference< awt::XWindow > xContComp(xControlContainer, uno::UNO_QUERY);
                if( xContComp.is() )
                    xContComp->setPosSize(aPosPix.X(), aPosPix.Y(), aSizePix.Width(), aSizePix.Height(), awt::PosSize::POSSIZE);
            }
        }

        rView.InsertControlContainer(xControlContainer);
    }
}

BOOL SdrPageViewWinRec::MatchOutputDevice(OutputDevice* pOut) const
{
    BOOL bMatch = FALSE;

    if (pOutDev == pOut)
    {
        bMatch = TRUE;
    }

    return (bMatch);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrPageView);


SdrPageView::SdrPageView(SdrPage* pPage1, const Point& rOffs, SdrView& rNewView):
    rView(rNewView),
    //aRedraw(1024,16,16),
    aOfs(rOffs),
    pPaintingPageObj( NULL ),
    maDocumentColor( COL_AUTO )     // #103911# col_auto color lets the view takes the default SvxColorConfig entry
{
    DBG_CTOR(SdrPageView,NULL);
    pDragPoly0=new XPolyPolygon;
    pDragPoly=new XPolyPolygon;
    pWinList=new SdrPageViewWinList;
    pPage=pPage1;
    if (pPage!=NULL) {
        aPgOrg.X()=pPage->GetLftBorder();
        aPgOrg.Y()=pPage->GetUppBorder();
    }
//    aOut.SetOutDev(rView.pWin);
//    aOut.SetOffset(rOffs);
//    eDrwStat=RS_READY;
    bHasMarked=FALSE;
    //aDragPoly.Clear();
    aLayerVisi.SetAll();
    aLayerPrn.SetAll();
    bVisible=FALSE;

    pAktList = NULL;
    pAktGroup = NULL;
    SetAktGroupAndList(NULL, pPage);

    StartListening(*rNewView.GetModel());
    USHORT nWinAnz=rNewView.GetWinCount();
    for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++) {
        AddWin(rNewView.GetWin(nWinNum));
    }
}


SdrPageView::~SdrPageView()
{
    if (rView.GetModel()->GetPaintingPageView() == this)
    {
        // Abmelden
        rView.GetModel()->SetPaintingPageView(NULL);
    }

    DBG_DTOR(SdrPageView,NULL);
    delete pWinList;
    delete pDragPoly0;
    delete pDragPoly;
}

SdrPageViewWinRec* SdrPageView::ImpMakePageViewWinRec(OutputDevice* pOut)
{
    // MIB 3.7.08: Das WinRec muss sofort in die Liste eingetragen werden,
    // weil sich das InsertControlContainer darauf verlaesst
    SdrPageViewWinRec* pRec = new SdrPageViewWinRec( *this, pOut );
    pWinList->Insert(pRec);

    ULONG nObjAnz=pPage!=NULL?pPage->GetObjCount():0;

    for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++)
    {
        SdrObject* pObj = pPage->GetObj(nObjNum);

        if (pObj->IsUnoObj())
        {
            SdrUnoObj* pSdrUnoObj = PTR_CAST(SdrUnoObj, pObj);
            ImpInsertControl(pSdrUnoObj, pRec);
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
                    ImpInsertControl(pSdrUnoObj, pRec);
                }
            }
        }
    }
    return pRec;
}

void SdrPageView::AddWin(OutputDevice* pOutDev)
{
    USHORT nPos = pWinList->Find(pOutDev);

    if (nPos == SDRPAGEVIEWWIN_NOTFOUND)
        SdrPageViewWinRec* pWinRec = ImpMakePageViewWinRec(pOutDev);
}

void SdrPageView::DelWin(OutputDevice* pOutDev)
{
    USHORT nPos=pWinList->Find(pOutDev);

    if (nPos != SDRPAGEVIEWWIN_NOTFOUND)
    {
        pWinList->Delete(nPos);
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > SdrPageView::GetControlContainer( const OutputDevice* _pDevice )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > xReturn;
    USHORT nWinPos = pWinList->Find( const_cast< OutputDevice* >( _pDevice ) );
    if ( SDRPAGEVIEWWIN_NOTFOUND != nWinPos )
    {
        xReturn = xReturn.query( ((*pWinList)[ nWinPos ]).GetControlContainerRef( ) );
        if ( !xReturn.is() )
            ((*pWinList)[ nWinPos ]).CreateControlContainer( );
        xReturn = xReturn.query( ((*pWinList)[ nWinPos ]).GetControlContainerRef( ) );
    }
    return xReturn;
}

void SdrPageView::ImpInsertControl(const SdrUnoObj* pSdrUnoObj,
                                   SdrPageViewWinRec* pRec)
{
    if (pSdrUnoObj)
    {
        uno::Reference< awt::XControlModel > xUnoControlModel(pSdrUnoObj->GetUnoControlModel());
        if (!xUnoControlModel.is())
            return;

        USHORT nCtrlNum = pRec->aControlList.Find(xUnoControlModel);
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
                if (pRec->pOutDev->GetOutDevType() != OUTDEV_WINDOW)
                {
                    uno::Reference< awt::XView > xView = xUnoControl->getView();
                    if (xView.is())
                    {
//                        xView->setGraphics(pRec->pOutDev->GetGraphicsInterface(TRUE));
                    }
                }

                // ControlContainer ggf. erzeugen
                pRec->CreateControlContainer();

                // xControl in ControlContainer einfuegen (Eigentuemer-Uebergang)
                if (pRec->GetControlContainerRef().is())
                {
                    uno::Reference< awt::XWindow > xWindow(xUnoControl, uno::UNO_QUERY);
                    if (xWindow.is())
                    {
                        Rectangle aRect(pSdrUnoObj->GetLogicRect());
                        Point aPixPos(pRec->pOutDev->LogicToPixel(aRect.TopLeft()));
                        Size aPixSize(pRec->pOutDev->LogicToPixel(aRect.GetSize()));
                        xWindow->setPosSize(aPixPos.X(), aPixPos.Y(), aPixSize.Width(), aPixSize.Height(), awt::PosSize::POSSIZE);
                    }

                    // set the correct zoom
                    if (!GetView().IsDesignMode())
                    {
                        uno::Reference< awt::XView > xView(xUnoControl, uno::UNO_QUERY);
                        if (xView.is())
                        {
                            const MapMode& rMap = pRec->pOutDev->GetMapMode();
                            xView->setZoom((float) double(rMap.GetScaleX()),
                                           (float) double(rMap.GetScaleY()));
                        }
                    }

                    // #107049# set design mode before peer is created,
                    // this is also needed for accessibility
                    xUnoControl->setDesignMode(GetView().IsDesignMode());

                    SdrUnoControlRec* pUCR = new SdrUnoControlRec(&pRec->aControlList, (SdrUnoObj*)pSdrUnoObj, xUnoControl);
                    pRec->aControlList.Insert(pUCR);

                    // #108327# do this last - the addition of the control triggeres processes which rely
                    // on the control already being inserted into the aControlList
                    pRec->GetControlContainerRef()->addControl(pSdrUnoObj->GetUnoControlTypeName(), xUnoControl);
                }
            }
        }
    }
}


void SdrPageView::ImpUnoInserted(const SdrObject* pObj)
{
    SdrUnoObj* pSdrUnoObj = (SdrUnoObj*)pObj;
    USHORT nPos = pWinList->GetCount();

    for (; nPos; )
        ImpInsertControl(pSdrUnoObj, &(*pWinList)[--nPos]);

}

void SdrPageView::ImpUnoRemoved(const SdrObject* pObj)
{
    SdrUnoObj* pSdrUno=(SdrUnoObj*)pObj;
    USHORT nPos = pWinList->GetCount();
    for (; nPos; )
    {
        SdrPageViewWinRec& rPVWR=(*pWinList)[--nPos];
        USHORT nControlPos=rPVWR.aControlList.Find(pSdrUno->GetUnoControlModel());

        if (nControlPos != SDRUNOCONTROL_NOTFOUND)
            rPVWR.aControlList.Delete(nControlPos, TRUE);
    }
}


void __EXPORT SdrPageView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    if (bVisible) {
        SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
        if (pSdrHint!=NULL) {
            SdrHintKind eKind=pSdrHint->GetKind();
            const SdrObject* pObj=pSdrHint->GetObject();
            if ( pObj!=NULL && pObj->GetPage() == pPage )
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
            if (pSdrHint->IsNeedRepaint()) {
                if (((eKind==HINT_OBJCHG
                    || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED
                    || eKind == HINT_CONTROLINSERTED || eKind == HINT_CONTROLREMOVED) &&
                     pSdrHint->GetPage()!=NULL) ||
                    eKind==HINT_PAGECHG)
                {
                    FASTBOOL bInv=FALSE;
                    if (pSdrHint->GetPage()==pPage) bInv=TRUE;
                    else if (pSdrHint->GetPage()->IsMasterPage()) { // ebenfalls Invalidate, wenn pPage die geaenderte Page als MasterPage referenziert
                        USHORT nMaPgAnz=pPage!=NULL?pPage->GetMasterPageCount():0;
                        for (USHORT i=0; i<nMaPgAnz && !bInv; i++) {
                            const SdrPage* pMPg=pPage->GetMasterPage(i);
                            bInv=pMPg==pSdrHint->GetPage();
                        }
                    }
                    if (bInv) {
                        InvalidateAllWin(pSdrHint->GetRect(),TRUE);
                    }
                }
            }
            if (eKind==HINT_OBJLISTCLEARED && pSdrHint->GetPage()==pPage) {
                if (GetAktGroup()!=NULL) {
                    rView.UnmarkAllObj();
                    LeaveAllGroup();
                    //pWinList ...
                }
            }
        }
    }
}


void SdrPageView::ModelHasChanged()
{
    if (GetAktGroup()!=NULL) CheckAktGroup();
}


FASTBOOL SdrPageView::IsReadOnly() const
{
    return pPage==NULL || rView.GetModel()->IsReadOnly() || pPage->IsReadOnly() || GetObjList()->IsReadOnly();
}


void SdrPageView::Show()
{
    if (!bVisible) {
        bVisible=TRUE;
        InvalidateAllWin();
        USHORT nWinAnz=rView.GetWinCount();
        for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++) {
            AddWin(rView.GetWin(nWinNum));
        }
    }
}


void SdrPageView::Hide()
{
    if (bVisible) {
        InvalidateAllWin();
        bVisible=FALSE;
        pWinList->Clear();
    }
}


void SdrPageView::SetOffset(const Point& rPnt)
{
    if (aOfs!=rPnt) {
        if (bVisible) InvalidateAllWin();
        aOfs=rPnt;
        if (bVisible) InvalidateAllWin();
    }
}


Rectangle SdrPageView::GetPageRect() const
{
    if (pPage==NULL) return Rectangle();
    return Rectangle(GetOffset(),Size(pPage->GetWdt()+1,pPage->GetHgt()+1));
}


void SdrPageView::InvalidateAllWin()
{
    if (bVisible && pPage!=NULL) {
        Rectangle aRect(Point(0,0),Size(pPage->GetWdt()+1,pPage->GetHgt()+1));
        aRect.Union(pPage->GetAllObjBoundRect());
        aRect.Move(aOfs.X(),aOfs.Y());
        rView.InvalidateAllWin(aRect);
    }
}


void SdrPageView::InvalidateAllWin(const Rectangle& rRect, FASTBOOL bPlus1Pix)
{
    if (bVisible) {
        rView.InvalidateAllWin(rRect+GetOffset(),bPlus1Pix);
    }
}


void SdrPageView::ImpPaintOutlinerView(OutputDevice* pOut, const Rectangle& rRect) const
{
    if (rView.pTextEditOutliner==NULL) return;
    const SdrObject* pTextObjTmp=rView.GetTextEditObject();
    const SdrTextObj* pText=PTR_CAST(SdrTextObj,pTextObjTmp);
    FASTBOOL bTextFrame=pText!=NULL && pText->IsTextFrame();
    ULONG nViewAnz=rView.pTextEditOutliner->GetViewCount();
    for (ULONG i=0; i<nViewAnz; i++) {
        OutlinerView* pOLV=rView.pTextEditOutliner->GetView(i);
        if (pOLV->GetWindow()==pOut) {
            rView.ImpPaintOutlinerView(*pOLV,&rRect);
            return;
        }
    }
}

FASTBOOL SdrPageView::DoCachedMasterPaint(const SdrPage* pMasterPage, ExtOutputDevice& rXOut,
                                          const SdrPaintInfoRec& InfoRec) const
{
    return DoCachedMasterPaint( pMasterPage, rXOut, InfoRec, SDR_MASTERPAGECACHE_FULL );
}

FASTBOOL SdrPageView::DoCachedMasterPaint(const SdrPage* pPg, ExtOutputDevice& rXOut,
                                          const SdrPaintInfoRec& rInfoRec, ULONG nCacheMode) const
{
    if( pPage->GetBackgroundObj() )
        return FALSE;                   // own background available

    ULONG nImplPrepareMode, nImplPaintMode;

    if( SDR_MASTERPAGECACHE_FULL != nCacheMode && SDR_MASTERPAGECACHE_BACKGROUND != nCacheMode )
    {
        DBG_ERROR( "Wrong cache mode! Assuming SDR_MASTERPAGECACHE_FULL." );
        nCacheMode = SDR_MASTERPAGECACHE_FULL;
    }

    if( SDR_MASTERPAGECACHE_FULL == nCacheMode )
        nImplPrepareMode = IMP_PAGEPAINT_PREPARE_CACHE, nImplPaintMode = IMP_PAGEPAINT_PAINT_CACHE;
    else
        nImplPrepareMode = IMP_PAGEPAINT_PREPARE_BG_CACHE, nImplPaintMode = IMP_PAGEPAINT_PAINT_BG_CACHE;

    FASTBOOL bRet=TRUE;

    OutputDevice* pWin=rXOut.GetOutDev();
    ImpMasterBmp* pBmp=rView.pMasterBmp;;
    const MapMode& rMap=pWin->GetMapMode();

    // 1. Gucken, ob vielleicht eine passende Bmp da ist
    FASTBOOL bCreate=pBmp==NULL;

    // Seitennummer der MasterPage muss gleich sein
    if (!bCreate)
        bCreate=pBmp->nMasterPageNum!=pPg->GetPageNum();
    if (!bCreate) // Scaling checken
        bCreate=rMap.GetScaleX()!=pBmp->aMapX || rMap.GetScaleY()!=pBmp->aMapY;

    // Sichtbare Layer muessen gleich sein
    if (!bCreate)
        bCreate=pBmp->aVisLayers!=rInfoRec.aPaintLayer;

    // Der sichtbare Bereich von pPg muss innerhalb des Bereiches des VD liegen
    Point aTopLeftTmp(rMap.GetOrigin()); aTopLeftTmp.X()=-aTopLeftTmp.X(); aTopLeftTmp.Y()=-aTopLeftTmp.Y();
    Size aWinSize(pWin->GetOutputSize());
    Rectangle aNeedLogRect(aTopLeftTmp,aWinSize);
    aNeedLogRect.Right()++; aNeedLogRect.Bottom()++; // Weil Rect(Pnt,Siz) unten rechts immer eins weniger ist
    Rectangle aPageBound(pPg->GetAllObjBoundRect()); // Nur den wirklich verwendeten Bereich der MasterPage
    Size aPageSize(aPageBound.GetSize()); aPageSize.Width()--; aPageSize.Height()--;
    aNeedLogRect.Intersection(aPageBound); // Nur den wirklich verwendeten Bereich der MasterPage

    // Wenn die Seite vollstaendig ins Fenster passen wuerde...
    if (aWinSize.Width()>=aPageSize.Width())
    {
        aNeedLogRect.Left()=aPageBound.Left();
        aNeedLogRect.Right()=aPageBound.Right();
    }

    // Wenn die Seite vollstaendig ins Fenster passen wuerde...
    if (aWinSize.Height()>=aPageSize.Height())
    {
        aNeedLogRect.Top()=aPageBound.Top();
        aNeedLogRect.Bottom()=aPageBound.Bottom();
    }
    if (!bCreate)
        bCreate=!pBmp->aLogBound.IsInside(aNeedLogRect);

    // #108444# Check whether the draw modes match, this is relevant
    // for cache object validity
    if (!bCreate)
        bCreate = pBmp->nDrawMode != rInfoRec.nPaintMode;

    // 2. Wenn keine passende Bmp da ist, dann versuchen eine zu erzeugen
    if (bCreate)
    {
        pPg->SwapInAll();          // Gelinkte Graphiken auf MasterPages machen sonst Probleme

        if (pBmp==NULL)
        {
            pBmp=new ImpMasterBmp(*pWin);
            rView.pMasterBmp=pBmp;
        }

        // Propagate digit language to device of bitmap.
        pBmp->aVD.SetDigitLanguage (pWin->GetDigitLanguage());

        pBmp->aVD.SetClipRegion(); // ggf. gesetztes Clipping entfernen
        pBmp->aVD.SetMapMode(rMap);
        Size aNeedLogSize(aNeedLogRect.GetSize()); aNeedLogSize.Width()--; aNeedLogSize.Height()--;
        Size a1Pix(pWin->PixelToLogic(Size(1,1)));
        aNeedLogSize.Width() +=a1Pix.Width();  // 1 Pixel fuer Toleranz drauf
        aNeedLogSize.Height()+=a1Pix.Height();

        // #103834# Use flag that impress application is using this view
        pBmp->aVD.SetBackground( Wallpaper(maBackgroundColor));

        if (pBmp->aVD.SetOutputSize(aNeedLogSize,TRUE))
        {
            // MapMode ausrechnen und am VirtDev setzen
            MapMode aNewMap(rMap);
            Point aMapOrgTmp(aNeedLogRect.TopLeft());
            aMapOrgTmp.X()=-aMapOrgTmp.X(); aMapOrgTmp.Y()=-aMapOrgTmp.Y();
            Point aMapOrgGridTmp(aMapOrgTmp);
            aMapOrgGridTmp-=aOfs; // Position der PageView abziehen fuer Border, Grid, ...
            aNewMap.SetOrigin(aMapOrgGridTmp);
            pBmp->aVD.SetMapMode(aNewMap);
            rXOut.SetOutDev(&pBmp->aVD);

            // Paper, Border etc. painten
            if (rView.bPageVisible)
            {
                ((SdrPageView*)this)->DrawPaper(pBmp->aVD);

                if (rView.bPageBorderVisible)
                    ((SdrPageView*)this)->DrawPaperBorder(pBmp->aVD);
            }

            if (rView.bBordVisible)
                ((SdrPageView*)this)->DrawBorder(pBmp->aVD);

            if (rView.bGridVisible && !rView.bGridFront)
                ((SdrPageView*)this)->DrawGrid(pBmp->aVD, rView.GetGridColor());

            if (rView.bHlplVisible && !rView.bHlplFront)
                ((SdrPageView*)this)->DrawHelplines(pBmp->aVD);

            // DrawMode vom Window uebernehmen
            const ULONG nOldDrawMode = pBmp->aVD.GetDrawMode();
            pBmp->aVD.SetDrawMode( pWin->GetDrawMode() );

            // #108444# Store draw mode, this is relevant for object validity
            pBmp->nDrawMode = rInfoRec.nPaintMode;

            // und nun endlich die MasterPage
            aNewMap.SetOrigin(aMapOrgTmp);
            pBmp->aVD.SetMapMode(aNewMap);

            Point aTopLeftVDPixelLog(-aMapOrgTmp.X(),-aMapOrgTmp.Y());
            pBmp->aVD.IntersectClipRegion(Rectangle(aTopLeftVDPixelLog,aNeedLogSize));
            SdrPaintInfoRec aInfoRec(rInfoRec);
            aInfoRec.aCheckRect=aNeedLogRect;
            aInfoRec.aDirtyRect=aNeedLogRect;
            pPg->Paint(rXOut,aInfoRec,FALSE,(sal_uInt16)nImplPrepareMode);
            pBmp->aLogBound=aNeedLogRect;
            pBmp->nMasterPageNum=pPg->GetPageNum();
            pBmp->aMapX=rMap.GetScaleX();
            pBmp->aMapY=rMap.GetScaleY();
            pBmp->aVisLayers=rInfoRec.aPaintLayer;
            rXOut.SetOutDev(pWin);
            // MapOrigin des VDev wieder auf 0 setzen
            aNewMap.SetOrigin(Point());
            pBmp->aVD.SetMapMode(aNewMap);
            pBmp->aVD.SetClipRegion(); // ggf. gesetztes Clipping entfernen

            // urspr. DrawMode wieder setzen
            pBmp->aVD.SetDrawMode( nOldDrawMode );
        }
        else
        {
            // Speicher reicht nicht
            delete pBmp;
            pBmp=NULL;
            rView.pMasterBmp=NULL;
        }
    }
    // 3. Bmp painten
    if (pBmp!=NULL)
    {
        Size aSiz(pBmp->aVD.GetOutputSize());
        pWin->DrawOutDev(pBmp->aLogBound.TopLeft(),aSiz,Point(),aSiz,pBmp->aVD);
        pPg->Paint(rXOut,rInfoRec,FALSE,(sal_uInt16)nImplPaintMode);

        // #74982# activate plugins on master page
        if(rInfoRec.pPV)
        {
            SdrView* pSdrView = (SdrView*) &rInfoRec.pPV->GetView();
            SdrObjListIter aIter(*pPg, IM_DEEPNOGROUPS);

            while(aIter.IsMore())
            {
                SdrObject* pObj = aIter.Next();

                if(pObj && pObj->ISA(SdrOle2Obj))
                {
                    pSdrView->DoConnect((SdrOle2Obj*)pObj);
                }
            }
        }
    }
    else
        bRet=FALSE; // ansonsten hat der Speicher nicht ausgereicht

//#endif

    return bRet;
}


FASTBOOL SdrPageView::MasterShouldBeCached(const SdrPage* pPg) const
{
    ULONG nObjAnz=pPg->GetObjCount();
    FASTBOOL bYes=nObjAnz>10; // bei mehr als 10 Objekten wird der Cache angeschaltet
    for (ULONG nObjNum=0; nObjNum<nObjAnz && !bYes; nObjNum++) {
        const SdrObject* pObj=pPg->GetObj(nObjNum);
        if (!pObj->IsNotVisibleAsMaster()) { // z.B. TitleText und OutlineText
            UINT32 nInv=pObj->GetObjInventor();
            UINT16 nId=pObj->GetObjIdentifier();
            // Bestimmte Objekttypen werden grundsaetzlich als langsam im Paint eingestuft:
            bYes=nInv==SdrInventor &&
                 (nId==OBJ_GRUP ||
                  nId==OBJ_PATHLINE || nId==OBJ_PATHFILL || nId==OBJ_FREELINE || nId==OBJ_FREEFILL ||
                  nId==OBJ_GRAF     || nId==OBJ_OLE2     || nId==OBJ_PATHPOLY || nId==OBJ_PATHPLIN);
            // Alles was Text hat wird gecached.
            if (!bYes) bYes=pObj->GetOutlinerParaObject()!=NULL;
            // Nochmal auf GruppenObjekt checken
            if (!bYes) bYes=pObj->GetSubList()!=NULL;
            // Und nun Attribute Checken
            if (!bYes) {
                const SfxItemSet& rSet = pObj->GetItemSet();
                XLineStyle eLineStyle=((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
                XFillStyle eFillStyle=((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
                bYes=(eLineStyle!=XLINE_NONE && eLineStyle!=XLINE_SOLID) ||
                     (eFillStyle!=XFILL_NONE && eFillStyle!=XFILL_SOLID);
            }
        }
    }
    return bYes;
}


void SdrPageView::InitRedraw(USHORT nWinNum, const Region& rReg, USHORT nPaintMode, const Link* pPaintProc)
{
    OutputDevice* pOut=rView.GetWin(nWinNum);
    if (pOut!=NULL) InitRedraw(pOut,rReg,nPaintMode,pPaintProc);
}


void SdrPageView::InitRedraw(OutputDevice* pOut_, const Region& rReg, USHORT nPaintMode, const Link* pPaintProc)
{
    if(!pPage)
        return;

    sal_uInt16 nWinAnz(pOut_!=NULL ? 1 : rView.GetWinCount());
    rView.GetModel()->SetPaintingPageView(this);

    for(sal_uInt16 nWinNum(0); nWinNum < nWinAnz; nWinNum++)
    {
        OutputDevice* pOut = pOut_!=NULL ? pOut_ : rView.GetWin(nWinNum);
        DBG_ASSERT(pOut!=NULL,"SdrPageView::InitRedraw(): pOut==NULL");

        if(!pOut)
            break;

        sal_Bool bPrinter(OUTDEV_PRINTER == pOut->GetOutDevType());
        const sal_uInt32 nOldDrawMode(pOut->GetDrawMode());

        // DrawMode temp. zuruecksetzen
        pOut->SetDrawMode(DRAWMODE_DEFAULT);

        ExtOutputDevice* pXOut = rView.pXOut;
        pXOut->SetOutDev(pOut);

        sal_Bool bDrawAll(rReg.IsEmpty());
        Rectangle aDirtyRect(rReg.GetBoundRect());
        Size a1PixSiz(pOut->PixelToLogic(Size(1, 1)));
        Rectangle aCheckRect(aDirtyRect);

        aCheckRect.Left() -= a1PixSiz.Width();
        aCheckRect.Top() -= a1PixSiz.Height();
        aCheckRect.Right() += a1PixSiz.Width();
        aCheckRect.Bottom() += a1PixSiz.Height();

        // Rect relativ zur PageView zum checken der Objekte
        aCheckRect -= aOfs;

        sal_Bool bTextEdit(rView.IsTextEdit() && rView.pTextEditPV == this);
        ImpSdrHdcMerk aHDCMerk(*pOut, SDRHDC_SAVEPENANDBRUSHANDFONT, rView.bRestoreColors);
        // Dirty, wg. DrawPager, ...
        sal_Bool bColorsDirty(sal_True);

        if(!bPrinter)
        {
            if(rView.bPageVisible)
            {
                DrawPaper(*pOut);

                if(rView.bPageBorderVisible)
                    DrawPaperBorder(*pOut);
            }

            if(rView.bBordVisible)
                DrawBorder(*pOut);

            if(rView.bGridVisible && !rView.bGridFront)
                DrawGrid(*pOut,aCheckRect, rView.GetGridColor());

            if(rView.bHlplVisible && !rView.bHlplFront)
                DrawHelplines(*pOut);
        }

        pXOut->SetOffset(aOfs);

        // eingestellten DrawMode wiederherstellen
        pOut->SetDrawMode(nOldDrawMode);

        SdrPaintInfoRec aInfoRec;
        aInfoRec.pPV = this;
        aInfoRec.bPrinter = bPrinter;
        aInfoRec.aDirtyRect = aDirtyRect;
        aInfoRec.aCheckRect = aCheckRect;
        aInfoRec.pPaintProc = pPaintProc;

        if(bPrinter)
        {
            if(rView.IsLineDraftPrn())
                nPaintMode |= SDRPAINTMODE_DRAFTLINE;

            if(rView.IsFillDraftPrn())
                nPaintMode |= SDRPAINTMODE_DRAFTFILL;

            if(rView.IsTextDraftPrn())
                nPaintMode |= SDRPAINTMODE_DRAFTTEXT;

            if(rView.IsGrafDraftPrn())
                nPaintMode |= SDRPAINTMODE_DRAFTGRAF;
        }
        else
        {
            if(rView.IsLineDraft())
                nPaintMode |= SDRPAINTMODE_DRAFTLINE;

            if(rView.IsFillDraft())
                nPaintMode |= SDRPAINTMODE_DRAFTFILL;

            if(rView.IsTextDraft())
                nPaintMode |= SDRPAINTMODE_DRAFTTEXT;

            if(rView.IsGrafDraft())
                nPaintMode |= SDRPAINTMODE_DRAFTGRAF;

            if(rView.IsHideGrafDraft())
                nPaintMode |= SDRPAINTMODE_HIDEDRAFTGRAF;
        }

        const SetOfByte& rPaintLayer = bPrinter ? aLayerPrn : aLayerVisi;

        // erstmal alle MasterPages Painten
        sal_uInt16 nMaPgAnz(pPage->GetMasterPageCount());
        sal_Bool bNeedMPagPaint(sal_True);

        if(!bPrinter && 1 == nMaPgAnz && rView.IsMasterPagePaintCaching())
        {
            // Die MasterPage ggf. mit 'ner Bitmap malen
            SdrPage* pMasterPage = pPage->GetMasterPage(0);

            if(pMasterPage && pMasterPage->GetObjCount() && MasterShouldBeCached(pMasterPage))
            {
                // Gucken, ob passende Bmp da ist. Wenn nicht, dann versuchen eine zu erzeugen. Bmp painten.
                aInfoRec.aPaintLayer = rPaintLayer;
                aInfoRec.aPaintLayer &= pPage->GetMasterPageVisibleLayers(0);
                aInfoRec.nPaintMode = nPaintMode | SDRPAINTMODE_MASTERPAGE;
                bNeedMPagPaint = !DoCachedMasterPaint(pMasterPage, *pXOut, aInfoRec, rView.GetMasterPagePaintCacheMode());
            }
        }

        if(bNeedMPagPaint)
        {
            sal_uInt16 nMaPgNum(0);

            while(nMaPgNum < nMaPgAnz)
            {
                SdrPage* pMasterPage = pPage->GetMasterPage(nMaPgNum);

                if(pMasterPage && pMasterPage->GetObjCount())
                {
                    aInfoRec.aPaintLayer = rPaintLayer;
                    aInfoRec.aPaintLayer &= pPage->GetMasterPageVisibleLayers(nMaPgNum);
                    aInfoRec.nPaintMode = nPaintMode | SDRPAINTMODE_MASTERPAGE;
                    pMasterPage->Paint(*pXOut, aInfoRec, rView.bRestoreColors);
                }
                nMaPgNum++;
            }
        }

        // Und nun die eigentliche Zeichenseite Painten
        aInfoRec.aPaintLayer = rPaintLayer;
        aInfoRec.nPaintMode = nPaintMode;

        if(GetObjList() != pPage)
            aInfoRec.pAktList = GetObjList();

        if(!bPrinter && rView.ImpIsGlueVisible())
        {
            aInfoRec.nPaintMode |= SDRPAINTMODE_GLUEPOINTS;
        }

        // Zeichnen
        pPage->Paint(*pXOut, aInfoRec, rView.bRestoreColors);
        pXOut->SetOffset(Point(0, 0));

        if(!bPrinter)
        {
            // Raster und Hilfslinien malen
            if(rView.bGridVisible && rView.bGridFront)
                DrawGrid(*pOut, aCheckRect, rView.GetGridColor());

            if(rView.bHlplVisible && rView.bHlplFront)
                DrawHelplines(*pOut);
        }

        if(bTextEdit)
        {
            ImpPaintOutlinerView(pOut, aCheckRect);
            bColorsDirty = sal_True;
        }

        if(rView.bRestoreColors)
        {
            aHDCMerk.Restore(*pOut);
        }
    }

    rView.PostPaint();
    // #37074#: fuer SolidHandles im LiveModus der praesentation
    rView.RestartAfterPaintTimer();
}


FASTBOOL SdrPageView::IsReady() const
{
    FASTBOOL bRet=TRUE;
    return bRet;
}

void SdrPageView::DrawPaper(OutputDevice& rOut)
{
    if( pPage )
    {
        // #103911# use color that was set on this view as background if present
        if( maDocumentColor != COL_AUTO )
        {
            rOut.SetFillColor( maDocumentColor );
        }
        else
        {
            const svtools::ColorConfig aColorConfig;
            rOut.SetFillColor( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
        }


        rOut.SetLineColor();
        rOut.DrawRect( GetPageRect() );
    }
}

void SdrPageView::DrawPaperBorder(OutputDevice& rOut)
{
    if( pPage )
    {
        svtools::ColorConfig aColorConfig;
        rOut.SetLineColor( Color(aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor) );
        rOut.SetFillColor();
        rOut.DrawRect( GetPageRect() );
    }
}

void SdrPageView::DrawBorder(OutputDevice& rOut)
{
    if( pPage && ( pPage->GetLftBorder() || pPage->GetUppBorder() || pPage->GetRgtBorder() || pPage->GetLwrBorder() ) )
    {
        svtools::ColorConfig    aColorConfig;
        Color               aBorderColor;

        if( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
            aBorderColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
        else
            aBorderColor = aColorConfig.GetColorValue( svtools::DOCBOUNDARIES ).nColor;

        rOut.SetLineColor( aBorderColor );
        rOut.SetFillColor();

        Rectangle aRect(GetPageRect());
        aRect.Left  ()+=pPage->GetLftBorder();
        aRect.Top   ()+=pPage->GetUppBorder();
        aRect.Right ()-=pPage->GetRgtBorder();
        aRect.Bottom()-=pPage->GetLwrBorder();
        rOut.DrawRect(aRect);
    }
}

#ifdef OS2
#define RGBCOLOR(r,g,b) ((ULONG)(((BYTE)(b) | ((USHORT)(g)<<8)) | (((ULONG)(BYTE)(r))<<16)))
#endif


void SdrPageView::DrawGrid(OutputDevice& rOut, const Rectangle& rRect, Color aColor)
{
    if (pPage==NULL)
        return;

    long nx1=rView.aGridBig.Width();
    long nx2=rView.aGridFin.Width();
    long ny1=rView.aGridBig.Height();
    long ny2=rView.aGridFin.Height();

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
        long x1=pPage->GetLftBorder()+1+nWrX;
        long x2=pPage->GetWdt()-pPage->GetRgtBorder()-1+nWrY;
        long y1=pPage->GetUppBorder()+1+nWrX;
        long y2=pPage->GetHgt()-pPage->GetLwrBorder()-1+nWrY;
        const SdrPageGridFrameList* pFrames=pPage->GetGridFrameList(this,NULL);
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


void SdrPageView::DrawHelplines(OutputDevice& rOut)
{
    aHelpLines.DrawAll(rOut,aOfs);
}


FASTBOOL SdrPageView::RedrawOne(USHORT nBrkEvent)
{
    return TRUE;
}


FASTBOOL SdrPageView::RedrawUntilInput(USHORT nBrkEvent)
{
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


void SdrPageView::RedrawOneLayer(SdrLayerID nId, const Rectangle& rRect, OutputDevice* pOut_, USHORT nPaintMode, const Link* pPaintProc) const
{
    if (pPage==NULL) return;
    USHORT nWinAnz=pOut_!=NULL ? 1 : rView.GetWinCount();
    if (pPage->GetObjCount()==0) return; // Liste ist leer!
    for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++) {
        OutputDevice* pOut=pOut_!=NULL ? pOut_ : rView.GetWin(nWinNum);
        DBG_ASSERT(pOut!=NULL,"SdrPageView::InitRedraw(): pOut==NULL");
        if (pOut==NULL) break;

        FASTBOOL bPrinter=(pOut->GetOutDevType()==OUTDEV_PRINTER);
        if (!(bPrinter?&aLayerPrn:&aLayerVisi)->IsSet(nId)) break; // der ist aber nicht druck/sichtbar!
        ExtOutputDevice* pXOut=rView.pXOut;
        pXOut->SetOutDev(pOut);

        FASTBOOL bDrawAll=rRect.IsEmpty();

        Size a1PixSiz(pOut->PixelToLogic(Size(1,1)));
        Rectangle aCheckRect(rRect);
        aCheckRect.Left()  -=a1PixSiz.Width();
        aCheckRect.Top()   -=a1PixSiz.Height();
        aCheckRect.Right() +=a1PixSiz.Width();
        aCheckRect.Bottom()+=a1PixSiz.Height();
        aCheckRect-=aOfs; // Rect relativ zur PageView zum checken der Objekte

        FASTBOOL bTextEdit=rView.IsTextEdit() && rView.pTextEditPV==this;

        ImpSdrHdcMerk aHDCMerk(*pOut,SDRHDC_SAVEPENANDBRUSHANDFONT,rView.bRestoreColors);
        FASTBOOL bColorsDirty=FALSE;

        pXOut->SetOffset(aOfs);
        SdrPaintInfoRec aInfoRec;
        aInfoRec.pPV=this;
        aInfoRec.bPrinter=bPrinter;
        aInfoRec.aDirtyRect=rRect;
        aInfoRec.aCheckRect=aCheckRect;
        aInfoRec.pPaintProc=pPaintProc;

        if (bPrinter) {
            if (rView.IsLineDraftPrn()) nPaintMode|=SDRPAINTMODE_DRAFTLINE;
            if (rView.IsFillDraftPrn()) nPaintMode|=SDRPAINTMODE_DRAFTFILL;
            if (rView.IsTextDraftPrn()) nPaintMode|=SDRPAINTMODE_DRAFTTEXT;
            if (rView.IsGrafDraftPrn()) nPaintMode|=SDRPAINTMODE_DRAFTGRAF;
        } else {
            if (rView.IsLineDraft()) nPaintMode|=SDRPAINTMODE_DRAFTLINE;
            if (rView.IsFillDraft()) nPaintMode|=SDRPAINTMODE_DRAFTFILL;
            if (rView.IsTextDraft()) nPaintMode|=SDRPAINTMODE_DRAFTTEXT;
            if (rView.IsGrafDraft()) nPaintMode|=SDRPAINTMODE_DRAFTGRAF;
            if (rView.IsHideGrafDraft()) nPaintMode|=SDRPAINTMODE_HIDEDRAFTGRAF;
        }

        aInfoRec.aPaintLayer.ClearAll();
        aInfoRec.aPaintLayer.Set((BYTE)nId);
        aInfoRec.nPaintMode=nPaintMode;
        if (GetObjList()!=pPage) aInfoRec.pAktList=GetObjList();
        if (!bPrinter && rView.ImpIsGlueVisible()) {
            aInfoRec.nPaintMode|=SDRPAINTMODE_GLUEPOINTS;
        }

        pPage->Paint(*pXOut,aInfoRec,rView.bRestoreColors);
        pXOut->SetOffset(Point(0,0));

        if (bTextEdit)
        {
            SdrObject* pObj = rView.GetTextEditObject();
            if( pObj && pObj->GetLayer() == nId )
            {
                ImpPaintOutlinerView(pOut,aCheckRect);
                bColorsDirty=TRUE;
            }
        }

        if (rView.bRestoreColors /*&& bColorsDirty*/) {
            aHDCMerk.Restore(*pOut);
        }

        const USHORT nWinNum1 = rView.FindWin( pOut );
        if( nWinNum1 != SDRVIEWWIN_NOTFOUND )
            rView.AfterInitRedraw( nWinNum1 ); // fuer SolidHandles im Writer und Calc (Joe, 13.3.1998)
    }

    // #72567# removed: rView.PostPaint();
    rView.RestartAfterPaintTimer(); // #36496#: fuer SolidHandles im Writer und Calc
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


void SdrPageView::AdjHdl()
{
    rView.AdjustMarkHdl();
}


void SdrPageView::SetLayer(const XubString& rName, SetOfByte& rBS, FASTBOOL bJa)
{
    if (pPage==NULL) return;
    SdrLayerID nID=pPage->GetLayerAdmin().GetLayerID(rName,TRUE);
    if (nID!=SDRLAYER_NOTFOUND) {
        rBS.Set(nID,bJa);
        if (&rBS == &aLayerVisi)
            LayerVisibilityChanged(nID, bJa);
    }
}


FASTBOOL SdrPageView::IsLayer(const XubString& rName, const SetOfByte& rBS) const
{
    if(!pPage)
        return FALSE;

    BOOL bRet(FALSE);

    if(rName.Len())
    {
        SdrLayerID nId = pPage->GetLayerAdmin().GetLayerID(rName, TRUE);

        if(nId != SDRLAYER_NOTFOUND)
        {
            bRet = rBS.IsSet(nId);
        }
    }
    return bRet;
}


void SdrPageView::SetAllLayers(SetOfByte& rB, FASTBOOL bJa)
{
    if (bJa) {
        rB.SetAll();
        rB.Clear(SDRLAYER_NOTFOUND);
    } else {
        rB.ClearAll();
    }
    // TODO: LayerVisibilityChanged to be called when necessary
}


void SdrPageView::LayerVisibilityChanged( const SdrLayerID _nLayerId, bool _bNewVisibility )
{
    // adjust the visibility of UNO controls, if necessary
    const SdrPageViewWinList& rWinList = GetWinList();
    const USHORT nWinCount = rWinList.GetCount();
    for ( USHORT i=0; i<nWinCount; ++i )
    {
        const SdrPageViewWinRec& rWinData = rWinList[i];
        const SdrUnoControlList& rWinControls = rWinData.GetControlList();
        const USHORT nControlCount = rWinControls.GetCount();
        for ( USHORT j=0; j<nControlCount; ++j )
        {
            SdrUnoControlRec& rControlData = const_cast< SdrUnoControlRec& >( rWinControls[j] );
                // I prefer the const_cast over using the various friend relationships
            rControlData.adjustControlVisibility( false );
        }
    }
}

void SdrPageView::ShowLayerSet(const XubString& rName, FASTBOOL bShow)
{
    if (pPage==NULL) return;
    SdrLayerSet* pSet=pPage->GetLayerAdmin().GetLayerSet(rName,TRUE);
    if (pSet!=NULL) {
        for (USHORT i=0; i<255; i++) {
            if (pSet->IsMember(BYTE(i))) {
                aLayerVisi.Set(BYTE(i),bShow);
                LayerVisibilityChanged(static_cast<SdrLayerID>(i), bShow);
            } else {
                if (bShow && pSet->IsExcluded(BYTE(i))) {
                    aLayerVisi.Clear(BYTE(i));
                    LayerVisibilityChanged(static_cast<SdrLayerID>(i), FALSE);
                }
            }
        }
    }
    if (!bShow) rView.AdjustMarkHdl();
    InvalidateAllWin();
}


FASTBOOL SdrPageView::IsLayerSetVisible(const XubString& rName) const
{
    if (pPage==NULL) return FALSE;
    FASTBOOL bRet=FALSE;
    SdrLayerSet* pSet=pPage->GetLayerAdmin().GetLayerSet(rName,TRUE);
    if (pSet!=NULL) {
        bRet=TRUE;
        USHORT i=0;
        while (bRet && i<255) {
            if (pSet->IsMember(BYTE(i))) {
                bRet=aLayerVisi.IsSet(BYTE(i));
            } else {
                if (pSet->IsExcluded(BYTE(i))) {
                    bRet=!aLayerVisi.IsSet(BYTE(i));
                }
            }
            i++;
        }
    }
    return bRet;
}


FASTBOOL SdrPageView::IsObjMarkable(SdrObject* pObj) const
{
    if(pObj)
    {
        // Vom Markieren ausgeschlossen?
        if(pObj->IsMarkProtect())
            return FALSE;

        // Der Layer muss sichtbar und darf nicht gesperrt sein
        SdrLayerID nL = pObj->GetLayer();
        return aLayerVisi.IsSet(BYTE(nL)) && !aLayerLock.IsSet(BYTE(nL));
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


void SdrPageView::SetPageOrigin(const Point& rOrg)
{
    if (rOrg!=aPgOrg) {
        aPgOrg=rOrg;
        if (rView.IsGridVisible()) {
            InvalidateAllWin();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////


void SdrPageView::ImpInvalidateHelpLineArea(USHORT nNum) const
{
    if (rView.IsHlplVisible() && nNum<aHelpLines.GetCount()) {
        const SdrHelpLine& rHL=aHelpLines[nNum];
        for (USHORT i=0; i<rView.GetWinCount(); i++) {
            OutputDevice* pOut=rView.GetWin(i);
            if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
                Rectangle aR(rHL.GetBoundRect(*pOut));
                Size aSiz(pOut->PixelToLogic(Size(1,1)));
                aR.Left  ()-=aSiz.Width();
                aR.Right ()+=aSiz.Width();
                aR.Top   ()-=aSiz.Height();
                aR.Bottom()+=aSiz.Height();
                aR.Move(aOfs.X(),aOfs.Y());
                rView.InvalidateOneWin(*(Window*)pOut,aR);
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
    if (rView.IsHlplVisible()) {
        if (rView.IsHlplFront()) {
            // Hier optimieren ...
            ImpInvalidateHelpLineArea(nNum);
         } else {
            ImpInvalidateHelpLineArea(nNum);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////


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

FASTBOOL SdrPageView::EnterGroup(SdrObject* pObj)
{
    FASTBOOL bRet=FALSE;
    if (pObj!=NULL && pObj->IsGroupObject()) {
        FASTBOOL bDisInvalidate=rView.pDisabledAttr!=NULL;
        FASTBOOL bGlueInvalidate=!bDisInvalidate && rView.ImpIsGlueVisible();
        if (bGlueInvalidate) rView.GlueInvalidate();

        // deselect all
        rView.UnmarkAll();

        // set current group and list
        SdrObjList* pNewObjList = pObj->GetSubList();
        SetAktGroupAndList(pObj, pNewObjList);

        // select contained object if only one object is contained,
        // else select nothing and let the user decide what to do next
        if(pNewObjList && pNewObjList->GetObjCount() == 1)
        {
            SdrObject* pFirstObject = pNewObjList->GetObj(0L);

            for(UINT16 nv = 0; nv < rView.GetPageViewCount(); nv++)
                rView.MarkObj(pFirstObject, rView.GetPageViewPvNum(nv));
        }

        // build new handles
        rView.AdjustMarkHdl();

        // invalidate only when view wants to visualize group entering
        if(rView.DoVisualizeEnteredGroup())
            InvalidateAllWin();

        if (bGlueInvalidate)
            rView.GlueInvalidate();
        bRet=TRUE;
    }
    return bRet;
}

void SdrPageView::LeaveOneGroup()
{
    if(GetAktGroup())
    {
        BOOL bDisInvalidate = (rView.pDisabledAttr != NULL);
        BOOL bGlueInvalidate = (!bDisInvalidate && rView.ImpIsGlueVisible());

        if(bGlueInvalidate)
            rView.GlueInvalidate();

        SdrObject* pLastGroup = GetAktGroup();
        SdrObject* pParentGroup = GetAktGroup()->GetUpGroup();
        SdrObjList* pParentList = pPage;

        if(pParentGroup)
            pParentList = pParentGroup->GetSubList();

        // Alles deselektieren
        rView.UnmarkAll();

        // Zuweisungen, pAktGroup und pAktList muessen gesetzt sein
        SetAktGroupAndList(pParentGroup, pParentList);

        // gerade verlassene Gruppe selektieren
        if(pLastGroup)
            for(UINT16 nv = 0; nv < rView.GetPageViewCount(); nv++)
                rView.MarkObj(pLastGroup, rView.GetPageViewPvNum(nv));

        rView.AdjustMarkHdl();

        // invalidate only when view wants to visualize group entering
        if(rView.DoVisualizeEnteredGroup())
            InvalidateAllWin();

        if(bGlueInvalidate)
            rView.GlueInvalidate();
    }
}


void SdrPageView::LeaveAllGroup()
{
    if(GetAktGroup())
    {
        BOOL bDisInvalidate = (rView.pDisabledAttr != NULL);
        BOOL bGlueInvalidate = (!bDisInvalidate && rView.ImpIsGlueVisible());

        if(bGlueInvalidate)
            rView.GlueInvalidate();

        SdrObject* pLastGroup = GetAktGroup();

        // Alles deselektieren
        rView.UnmarkAll();

        // Zuweisungen, pAktGroup und pAktList muessen gesetzt sein
        SetAktGroupAndList(NULL, pPage);

        // Oberste letzte Gruppe finden und selektieren
        if(pLastGroup)
        {
            while(pLastGroup->GetUpGroup())
                pLastGroup = pLastGroup->GetUpGroup();

            for(UINT16 nv = 0; nv < rView.GetPageViewCount(); nv++)
                rView.MarkObj(pLastGroup, rView.GetPageViewPvNum(nv));
        }

        rView.AdjustMarkHdl();

        // invalidate only when view wants to visualize group entering
        if(rView.DoVisualizeEnteredGroup())
            InvalidateAllWin();

        if(bGlueInvalidate)
            rView.GlueInvalidate();
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
        if (rPageView.pPage!=NULL) {
            SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_PAGVIEW);
            rOut<<BOOL(rPageView.bVisible);
            rOut<<BOOL(rPageView.pPage->IsMasterPage());
            rOut<<rPageView.pPage->GetPageNum();
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
                    BOOL bVisible;
                    BOOL bMaster;
                    USHORT nPgNum;
                    rIn>>bVisible;
                    rPageView.bVisible=bVisible;
                    rIn>>bMaster;
                    rIn>>nPgNum;
                    rIn>>rPageView.aOfs;
                    rIn>>rPageView.aPgOrg;
                    SdrModel* pMod=rPageView.GetView().GetModel();
                    if (!bMaster) rPageView.pPage=pMod->GetPage(nPgNum);
                    else rPageView.pPage=pMod->GetMasterPage(nPgNum);
                    rPageView.pAktList=rPageView.pPage;
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


