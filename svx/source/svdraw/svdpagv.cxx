/*************************************************************************
 *
 *  $RCSfile: svdpagv.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-13 16:48:43 $
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

#ifdef JOEDEBUG
#include "joedebug.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

TYPEINIT1(SdrPageView, SfxListener);

#ifdef JOEDEBUG
class UnoControl
{
public:
    Window* pWin;
public:
    UnoControl(OutputDevice* pParent);
    ~UnoControl() { delete pWin; }
    void SetRect(const SdrObject* pObj, const Point& rPvOfs); // nur fuer intern zum testen!
};

UnoControl::UnoControl(OutputDevice* pParent)
{
    if (pParent->GetOutDevType()==OUTDEV_WINDOW)
    {
        pWin=new Window((Window*)pParent);
        pWin->Show();
    } else {
        pWin=NULL;
    }
}

// nur fuer intern zum testen!
void UnoControl::SetRect(const SdrObject* pObj, const Point& rPvOfs)
{
    if (pWin!=NULL)
    {
        Rectangle aRect(pObj->GetBoundRect());
        aRect.Move(rPvOfs.X(),rPvOfs.Y());
        Window* pParent=pWin->GetParent();
        aRect=pParent->LogicToPixel(aRect);
        pWin->SetPosSizePixel(aRect.TopLeft(),aRect.GetSize());
    }
}

class UnoControlModel
{
public:
    UnoControl* CreateUnoControl(OutputDevice* pOutDev1) const { return new UnoControl(pOutDev1); }
};
#endif

// Klasse muﬂ als listener fungieren, um den Zustand, ob ein Object sichtbar ist oder nicht
// festzuhalten
//------------------------------------------------------------------------------
SdrUnoControlRec::SdrUnoControlRec(SdrUnoControlList* _pParent, SdrUnoObj* _pObj, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > _xControl) throw()
                 :pObj(_pObj)
                 ,xControl(_xControl)
                 ,bVisible(TRUE)
                 ,bIsListening(FALSE)
                 ,bDisposed(FALSE)
                 ,nEvent(0)
                 ,pParent(_pParent)
{
    uno::Reference< awt::XWindow> xWindow(xControl, uno::UNO_QUERY);
    if (xWindow.is())
    {
        xWindow->addWindowListener((awt::XWindowListener*)this);

        StartListening();

        // Am Property fuer das DefaultControl lauschen um das Control eventuell auszutauschen
        uno::Reference< beans::XPropertySet > xSet(xControl->getModel(), uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropInfo( xSet->getPropertySetInfo() );
            if (xPropInfo.is() && xPropInfo->hasPropertyByName( rtl::OUString::createFromAscii("DefaultControl")))
                xSet->addPropertyChangeListener( rtl::OUString::createFromAscii("DefaultControl"), this);
        }
    }
}

//------------------------------------------------------------------------------
SdrUnoControlRec::~SdrUnoControlRec() throw()
{
    {
        ::osl::MutexGuard aEventGuard( m_aEventSafety );
        if (nEvent)
            Application::RemoveUserEvent(nEvent);
        nEvent = 0;
    }

    ::osl::MutexGuard aDestructionGuard(m_aDestructionSafety);
    // this is just for the case we're deleted while another thread just handled the event :
    // if this other thread called our link while we were deleting the event here, the
    // link handler blocked. With leaving the above block it continued, but now we are prevented
    // to leave this destructor 'til the link handler recognized that nEvent == 0 and left.
}

//------------------------------------------------------------------------------
void SAL_CALL SdrUnoControlRec::disposing( const ::com::sun::star::lang::EventObject& Source )
    throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XControl > xSource( Source.Source, uno::UNO_QUERY);
    if (xSource.is())
    {
        if (!IsVisible())
            StopListening();

        uno::Reference< beans::XPropertySet > xSet(xControl->getModel(), uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropInfo( xSet->getPropertySetInfo() );
            if (xPropInfo.is() && xPropInfo->hasPropertyByName( rtl::OUString::createFromAscii("DefaultControl")))
                xSet->removePropertyChangeListener( rtl::OUString::createFromAscii("DefaultControl"), this);
        }

        if (pParent)
        {
            // now that the control is disposed, cancel any async events
            // 11/24/2000 - 80508 - FS
            {
                ::osl::MutexGuard aEventGuard( m_aEventSafety );
                if (nEvent)
                    Application::RemoveUserEvent(nEvent);
                nEvent = 0;
            }

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
    if (!IsVisible())
        StopListening();

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
    if (!bDisposed)
        StartListening();
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
    if (!pObj)
        return;
    osl::MutexGuard aEventGuard( m_aEventSafety );
    // bild fertig gelesen, dann nochmals neuzeichnen (async)
    if (nEvent)
        Application::RemoveUserEvent(nEvent);
    nEvent = Application::PostUserEvent(LINK(this,SdrUnoControlRec,OnComplete));
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
void SdrUnoControlRec::Clear(BOOL bDispose)
{
    if (xControl.is())
    {
        if (!IsVisible())
            StopListening();

        uno::Reference< awt::XWindow > xOldWindow(xControl, uno::UNO_QUERY);
        if (xOldWindow.is())
            xOldWindow->removeWindowListener((awt::XWindowListener*)this);

        uno::Reference< beans::XPropertySet > xSet(xControl->getModel(), uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropInfo = xSet->getPropertySetInfo();
            if (xPropInfo.is() && xPropInfo->hasPropertyByName( rtl::OUString::createFromAscii("DefaultControl")))
                xSet->removePropertyChangeListener( rtl::OUString::createFromAscii("DefaultControl"), this);
        }

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
        {
            xWindow->addWindowListener((awt::XWindowListener*)this);

            uno::Reference< beans::XPropertySet > xSet(xControl->getModel(), uno::UNO_QUERY);
            if (xSet.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropInfo = xSet->getPropertySetInfo();
                if (xPropInfo.is() && xPropInfo->hasPropertyByName( rtl::OUString::createFromAscii("DefaultControl")))
                    xSet->addPropertyChangeListener(rtl::OUString::createFromAscii("DefaultControl"), this);
            }

            // wieder lauschen falls noetig
            if (!bVisible)
                StartListening();
        }
    }
}

//------------------------------------------------------------------------------
void SdrUnoControlRec::StartListening()
{
    if (!IsListening())
    {
        bIsListening = TRUE;

        if (xControl.is())
        {
            uno::Reference< beans::XPropertySet > xSet(xControl->getModel(), uno::UNO_QUERY);
            if (xSet.is())
                xSet->addPropertyChangeListener(String(), this);

            uno::Reference< form::XImageProducerSupplier > xImg(xSet, uno::UNO_QUERY);
            if (xImg.is())
            {
                uno::Reference< awt::XImageProducer > xProducer = xImg->getImageProducer();
                if (xProducer.is())
                    xProducer->addConsumer(this);
            }
        }
    }
}

//------------------------------------------------------------------------------
void SdrUnoControlRec::StopListening()
{
    if (IsListening())
    {
        bIsListening = FALSE;

        if (xControl.is())
        {
            uno::Reference< beans::XPropertySet > xSet(xControl->getModel(), uno::UNO_QUERY);
            if (xSet.is())
                xSet->removePropertyChangeListener(String(), this);

            uno::Reference< form::XImageProducerSupplier > xImg(xSet, uno::UNO_QUERY);
            if (xImg.is())
            {
                uno::Reference< awt::XImageProducer > xProducer = xImg->getImageProducer();
                if (xProducer.is())
                    xProducer->removeConsumer(this);
            }
        }
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(SdrUnoControlRec, OnComplete, void*, EMPTYTAG)
{
    {
        ::osl::MutexGuard aDestructionGuard( m_aDestructionSafety );
        {
            ::osl::MutexGuard aEventGuard( m_aEventSafety );
            if (!nEvent)
                // our destructor deleted the event just while we we're waiting for m_aEventSafety
                // -> get outta here
                return 0;
            nEvent = 0;
        }
    }

    // Bereich neu Zeichnen
    OutputDevice* pOut = pObj->GetOutputDevice(xControl);
    if (pOut && pOut->GetOutDevType() == OUTDEV_WINDOW)
        ((Window*)pOut)->Invalidate(pObj->GetBoundRect());

    return 0;
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
        }
        break;
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
SdrPageViewWinRec::SdrPageViewWinRec(SdrView& rNewView, OutputDevice* pOut)
:   rView(rNewView),
    pOutDev(pOut)
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

            // UnoContainerModel erzeugen
            uno::Reference< awt::XWindow > xC(xControlContainer, uno::UNO_QUERY);

            // #58917# Das Show darf nicht am VCL-Fenster landen, weil dann Assertion vom SFX
            BOOL bVis = pWindow->IsVisible();
            xC->setVisible(TRUE);
            if ( !bVis )
                pWindow->Hide();
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
    pPaintingPageObj( NULL )
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
    SdrPageViewWinRec* pRec = new SdrPageViewWinRec(rView, pOut);
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

                    pRec->GetControlContainerRef()->addControl(pSdrUnoObj->GetUnoControlTypeName(), xUnoControl);

                    // Designmodus erst setzen, wenn Peer bereits existiert!
                    xUnoControl->setDesignMode(GetView().IsDesignMode());

                    SdrUnoControlRec* pUCR = new SdrUnoControlRec(&pRec->aControlList, (SdrUnoObj*)pSdrUnoObj, xUnoControl);
                    pRec->aControlList.Insert(pUCR);
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

#ifdef nixJOEDEBUG
    DebWrite("SdrPageView("); DebWrite(GetStr((void*)this)); DebWrite(")::DoCachedMasterPaint(");
    DebWrite(GetStr((void*)pPg)); DebWrite(",pWin="); DebWrite(GetStr(rXOut.GetOutDev()));
    DebWrite("("); DebWrite(GetStr(rXOut.GetOutDev()->GetOutDevType())); DebWriteln("),...)");
    DebIncIndent();
    DebWrite("pWin->GetMapMode()="); DebWriteln(GetStr(rXOut.GetMapMode()));
#endif

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

#ifdef nixJOEDEBUG
    Rectangle aVisibleArea(aTopLeftTmp,aWinSize);
    aVisibleArea.Right()++; aVisibleArea.Bottom()++; // Weil Rect(Pnt,Siz) unten rechts immer eins weniger ist
    DEBOUT(aVisibleArea);
    DEBOUT(aPageBound);
    DEBOUT(aNeedLogRect);
    DEBOUT(pBmp->aLogBound);
#endif

    // 2. Wenn keine passende Bmp da ist, dann versuchen eine zu erzeugen
    if (bCreate)
    {
#ifdef nixJOEDEBUG
        DebWriteln("*** Bitmap wird neu erzeugt ***");
#endif
        pPg->SwapInAll();          // Gelinkte Graphiken auf MasterPages machen sonst Probleme

        if (pBmp==NULL)
        {
            pBmp=new ImpMasterBmp(*pWin);
            rView.pMasterBmp=pBmp;
        }
        pBmp->aVD.SetClipRegion(); // ggf. gesetztes Clipping entfernen
        pBmp->aVD.SetMapMode(rMap);
        Size aNeedLogSize(aNeedLogRect.GetSize()); aNeedLogSize.Width()--; aNeedLogSize.Height()--;
        Size a1Pix(pWin->PixelToLogic(Size(1,1)));
        aNeedLogSize.Width() +=a1Pix.Width();  // 1 Pixel fuer Toleranz drauf
        aNeedLogSize.Height()+=a1Pix.Height();

#ifdef nixJOEDEBUG
        DEBOUT(aNeedLogSize);
#endif

     // Die Wiese im Hintergrund fuer StarDraw
#ifdef MAC
        Color aWiesenColor( RGB_COLORDATA( 223, 223, 223 ) );
#else
        Color aWiesenColor( RGB_COLORDATA( 234, 234, 234 ) );
#endif
        pBmp->aVD.SetBackground( Wallpaper( aWiesenColor ) );

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
            if (rView.bPageVisible) ((SdrPageView*)this)->DrawPaper(pBmp->aVD);
            if (rView.bBordVisible) ((SdrPageView*)this)->DrawBorder(pBmp->aVD);
            if (rView.bGridVisible && !rView.bGridFront) ((SdrPageView*)this)->DrawGrid(pBmp->aVD);
            if (rView.bHlplVisible && !rView.bHlplFront) ((SdrPageView*)this)->DrawHelplines(pBmp->aVD);

            // DrawMode vom Window uebernehmen
            const ULONG nOldDrawMode = pBmp->aVD.GetDrawMode();
            pBmp->aVD.SetDrawMode( pWin->GetDrawMode() );

            // und nun endlich die MasterPage
            aNewMap.SetOrigin(aMapOrgTmp);
            pBmp->aVD.SetMapMode(aNewMap);

#ifdef nixJOEDEBUG
            DebWrite("pBmp->aVD.SetMapMode("); DebWrite(GetStr(aNewMap)); DebWriteln(");");
#endif
            Point aTopLeftVDPixelLog(-aMapOrgTmp.X(),-aMapOrgTmp.Y());
            pBmp->aVD.IntersectClipRegion(Rectangle(aTopLeftVDPixelLog,aNeedLogSize));
            SdrPaintInfoRec aInfoRec(rInfoRec);
            aInfoRec.aCheckRect=aNeedLogRect;
            aInfoRec.aDirtyRect=aNeedLogRect;
            pPg->Paint(rXOut,aInfoRec,FALSE,nImplPrepareMode);
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
#ifdef nixJOEDEBUG
            DebWriteln("pBmp->aVD.SetOutputSize() ging daneben");
#endif
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
        pPg->Paint(rXOut,rInfoRec,FALSE,nImplPaintMode);
#ifdef nixJOEDEBUG
        DebWrite("Bitmap gepaintet: Size="); DebWrite(GetStr(aSiz));
        DebWrite(", Position="); DebWrite(GetStr(pBmp->aLogBound.TopLeft()));
        DebWriteln("");
#endif

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

#ifdef nixJOEDEBUG
    DebWrite("Returniert mit "); DebOut(bRet);
    DebDecIndent();
#endif

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
    if (pPage==NULL) return;
    USHORT nWinAnz=pOut_!=NULL ? 1 : rView.GetWinCount();
    rView.GetModel()->SetPaintingPageView(this);
    //SdrPageWin*   pRed=GetWin(nWinNum);
    for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++) {
        OutputDevice* pOut=pOut_!=NULL ? pOut_ : rView.GetWin(nWinNum);
        DBG_ASSERT(pOut!=NULL,"SdrPageView::InitRedraw(): pOut==NULL");
        if (pOut==NULL) break;
//    if (pOut!=NULL /*&& pRed!=NULL*/) {

        FASTBOOL bPrinter=(pOut->GetOutDevType()==OUTDEV_PRINTER);
        const ULONG nOldDrawMode = pOut->GetDrawMode();

        // DrawMode temp. zuruecksetzen
        pOut->SetDrawMode( DRAWMODE_DEFAULT );

        //pRed->bPrinter=bPrinter;
        ExtOutputDevice* pXOut=rView.pXOut;
        pXOut->SetOutDev(pOut);
        //pRed->eDrwStat=SDRREDRAW_BEGIN;
        //pRed->aDrwReg=rReg;
        //pRed->aCheckRect=rReg.GetBoundRect();
        FASTBOOL bDrawAll=rReg.IsEmpty();
        Rectangle aDirtyRect(rReg.GetBoundRect());
        Size a1PixSiz(pOut->PixelToLogic(Size(1,1)));
        Rectangle aCheckRect(aDirtyRect);
        aCheckRect.Left()  -=a1PixSiz.Width();
        aCheckRect.Top()   -=a1PixSiz.Height();
        aCheckRect.Right() +=a1PixSiz.Width();
        aCheckRect.Bottom()+=a1PixSiz.Height();
        aCheckRect-=aOfs; // Rect relativ zur PageView zum checken der Objekte

        FASTBOOL bTextEdit=rView.IsTextEdit() && rView.pTextEditPV==this;
        FASTBOOL bOnlyTextEdit=FALSE;
        if (bTextEdit && !bDrawAll && !bPrinter && rView.IsTextEditFrame() &&
            rView.pTextEditOutlinerView)
        {
            Rectangle aTmpRect(rView.pTextEditOutlinerView->GetOutputArea());

            // Da nicht mehr der gesamte Hintergrund der EditArea mit einem DrawRect()
            // vollflaechig gepainted wird, entfaellt das folgende Union()
//          aTmpRect.Union(rView.aMinTextEditArea);

            aTmpRect.Left()  -=2*a1PixSiz.Width();
            aTmpRect.Top()   -=2*a1PixSiz.Height();
            aTmpRect.Right() +=2*a1PixSiz.Width();
            aTmpRect.Bottom()+=2*a1PixSiz.Height();
            // wenn der Invalidierte Bereich vollstaendig von der OutlinerView
            // verdeckt wird, male ich nur die OutlinerView und sonst nix.
            bOnlyTextEdit=aTmpRect.IsInside(aDirtyRect);

            // Da nicht mehr der gesamte Hintergrund der EditArea mit einem DrawRect()
            // vollflaechig gepainted wird, muss bOnlyTextEdit ggf. auf FALSE gesetzt werden
            if( bOnlyTextEdit && rView.pTextEditOutliner )
            {
                Paragraph* p1stPara = rView.pTextEditOutliner->GetParagraph( 0 );
                if ( !p1stPara )
                {
                    // Kein Text vorhanden
                    bOnlyTextEdit = FALSE;
                }
                else
                {
                    if ( rView.pTextEditOutliner->GetParagraphCount() == 1 )
                    {
                        // bei nur einem Para nachsehen ob da ueberhaupt was drin steht
                        XubString aStr( rView.pTextEditOutliner->GetText( p1stPara ) );
                        if ( !aStr.Len() )
                            bOnlyTextEdit = FALSE; // Kein Text vorhanden
                    }
                }
            }

            // Funkt noch nicht:
            //if (!bOnlyTextEdit) { // aTmpRect ggf. von aCheckRect abziehen
              //  if (aTmpRect2.Left()<=pRed->aCheckRect.Left() && aTmpRect2.Right()>=pRed->aCheckRect.Right()) {
              //      if (aTmpRect2.Top()<=pRed->aCheckRect.Top() && aTmpRect2.Bottom()>pRed->aCheckRect.Top()) {
              //          pRed->aCheckRect.Top()=aTmpRect2.Bottom();
              //      }
              //  }
                //if (aTmpRect2.Top()<=pRed->aCheckRect.Top() && aTmpRect2.Bottom()>=pRed->aCheckRect.Bottom()) {
                //    ...
                //}
            //}
        }
#ifdef neeJOEDEBUG
        if (bTextEdit && !bOnlyTextEdit) {
            String aMsg("PageView::InitRedraw: bOnlyTextEdit=");
            if (bOnlyTextEdit) aMsg+="TRUE"; else aMsg+="FALSE";
            DebWriteln(aMsg);
        }
#endif
        ImpSdrHdcMerk aHDCMerk(*pOut,SDRHDC_SAVEPENANDBRUSHANDFONT,rView.bRestoreColors);
        FASTBOOL bColorsDirty=TRUE; // Dirty, wg. DrawPager, ...
        if (!bOnlyTextEdit) {
            if (!bPrinter) { // Papier, Seitenraender, Raster und Hilfslinien
                if (rView.bPageVisible) DrawPaper(*pOut);
                if (rView.bBordVisible) DrawBorder(*pOut);
                if (rView.bGridVisible && !rView.bGridFront) DrawGrid(*pOut,aCheckRect);
                if (rView.bHlplVisible && !rView.bHlplFront) DrawHelplines(*pOut);
            }
        }

        pXOut->SetOffset(aOfs);

        // eingestellten DrawMode wiederherstellen
        pOut->SetDrawMode( nOldDrawMode );

        if (!bOnlyTextEdit) {
            SdrPaintInfoRec aInfoRec;
            aInfoRec.pPV=this;
            aInfoRec.bPrinter=bPrinter;
            aInfoRec.aDirtyRect=aDirtyRect;
            aInfoRec.aCheckRect=aCheckRect;
            aInfoRec.pPaintProc=pPaintProc;
    #ifdef nixJOEDEBUG
            String aMsg("SdrPageView::InitRedraw(): "); if (pPage->IsMasterPage()) aMsg+="MasterPage "; else aMsg+="Seite ";
            aMsg+=(pPage->GetPageNum()+1); aMsg+=" auf OutDev "; aMsg+=...nWinNum...; aMsg+=" ("; aMsg+=GetStr(pOut->GetOutDevType()); aMsg+=").";
            DebWriteln(aMsg);
            DebOut(*pPage);
    #endif
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
            const SetOfByte& rPaintLayer=bPrinter ? aLayerPrn : aLayerVisi;

            // erstmal alle MasterPages Painten
            USHORT nMaPgAnz=pPage->GetMasterPageCount();
            FASTBOOL bNeedMPagPaint=TRUE;
            if (!bPrinter && nMaPgAnz==1 && rView.IsMasterPagePaintCaching()) { // Die MasterPage ggf. mit 'ner Bitmap malen
                SdrPage* pMasterPage=pPage->GetMasterPage(0);
                if (pMasterPage!=NULL && pMasterPage->GetObjCount()!=0 && MasterShouldBeCached(pMasterPage)) {
                    // Gucken, ob passende Bmp da ist. Wenn nicht, dann versuchen eine zu erzeugen. Bmp painten.
                    aInfoRec.aPaintLayer=rPaintLayer;
                    aInfoRec.aPaintLayer&=pPage->GetMasterPageVisibleLayers(0);
                    aInfoRec.nPaintMode=nPaintMode|SDRPAINTMODE_MASTERPAGE;
                    bNeedMPagPaint=!DoCachedMasterPaint(pMasterPage,*pXOut,aInfoRec,rView.GetMasterPagePaintCacheMode());
                }
            }
            if (bNeedMPagPaint) {
                USHORT nMaPgNum=0;
                while (nMaPgNum<nMaPgAnz) {
                    SdrPage* pMasterPage=pPage->GetMasterPage(nMaPgNum);
                    if (pMasterPage!=NULL && pMasterPage->GetObjCount()!=0) {
                        aInfoRec.aPaintLayer=rPaintLayer;
                        aInfoRec.aPaintLayer&=pPage->GetMasterPageVisibleLayers(nMaPgNum);
                        aInfoRec.nPaintMode=nPaintMode|SDRPAINTMODE_MASTERPAGE;
                        pMasterPage->Paint(*pXOut,aInfoRec,rView.bRestoreColors);
                    }
                    nMaPgNum++;
                }
            }
            // Und nun die eigentliche Zeichenseite Painten
            aInfoRec.aPaintLayer=rPaintLayer;
            aInfoRec.nPaintMode=nPaintMode;
            if (GetObjList()!=pPage) aInfoRec.pAktList=GetObjList();
            if (!bPrinter && rView.ImpIsGlueVisible()) {
                aInfoRec.nPaintMode|=SDRPAINTMODE_GLUEPOINTS;
            }

            // Zeichnen
            pPage->Paint(*pXOut,aInfoRec,rView.bRestoreColors);
            pXOut->SetOffset(Point(0,0));

            if (!bPrinter) { // Raster und Hilfslinien malen
                if (rView.bGridVisible && rView.bGridFront) DrawGrid(*pOut,aCheckRect);
                if (rView.bHlplVisible && rView.bHlplFront) DrawHelplines(*pOut);
            }
        }
        if (bTextEdit) {
            ImpPaintOutlinerView(pOut,aCheckRect);
            bColorsDirty=TRUE;
        }
        //pRed->eDrwStat=SDRREDRAW_READY;
        if (rView.bRestoreColors /*&& bColorsDirty*/) {
            aHDCMerk.Restore(*pOut);
        }
    }

    rView.PostPaint();
    rView.RestartAfterPaintTimer(); // #37074#: fuer SolidHandles im LiveModus der praesentation
}


FASTBOOL SdrPageView::IsReady() const
{
    FASTBOOL bRet=TRUE;
/*    for (USHORT i=0; i<GetWinCount() && bRet; i++) {
        if (GetWin(i)->eDrwStat!=SDRREDRAW_READY) bRet=FALSE;
    }*/
    return bRet;
}


void SdrPageView::DrawPaper(OutputDevice& rOut)
{
    if (pPage==NULL)
        return;

#ifdef nixJOEDEBUG
    DebWriteln("SdrPageView::DrawPaper()");
    DebIncIndent();
    DebWrite("MapMode des OutDev: ");
    DebWriteln(GetStr(rOut.GetMapMode()));
    DebDecIndent();
#endif

    rOut.SetLineColor( Color( COL_GRAY ) );
    rOut.SetFillColor( Color( COL_WHITE ) );
    Rectangle aRect(GetPageRect());
    rOut.DrawRect(aRect);
}


void SdrPageView::DrawBorder(OutputDevice& rOut)
{
    if (pPage==NULL)
        return;

    Color aGrayColor( COL_GRAY );
    rOut.SetLineColor( aGrayColor );
    rOut.SetFillColor();

    Rectangle aRect(GetPageRect());
    aRect.Left  ()+=pPage->GetLftBorder();
    aRect.Top   ()+=pPage->GetUppBorder();
    aRect.Right ()-=pPage->GetRgtBorder();
    aRect.Bottom()-=pPage->GetLwrBorder();
    rOut.DrawRect(aRect);

#ifdef neinJOEDEBUG // BoundRect und SnapRect der Page sichtbar machen
    rOut.SetFillColor();
    Rectangle aBnd(pPage->GetAllObjBoundRect());
    Rectangle aSnp(pPage->GetAllObjSnapRect());
    if (!aBnd.IsEmpty()) {
        aBnd.Move(aOfs.X(),aOfs.Y());
        rOut.SetLineColor( Color(COL_BLUE) );
        rOut.DrawRect(aBnd);
    }
    if (!aSnp.IsEmpty()) {
        aSnp.Move(aOfs.X(),aOfs.Y());
        rOut.SetLineCOlor( Color(COL_RED) );
        rOut.DrawRect(aSnp);
    }
#endif
}

#ifdef OS2
#define RGBCOLOR(r,g,b) ((ULONG)(((BYTE)(b) | ((USHORT)(g)<<8)) | (((ULONG)(BYTE)(r))<<16)))
#endif


void SdrPageView::DrawGrid(OutputDevice& rOut, const Rectangle& rRect)
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

        Color aCol(COL_GRAY);
        Color aColorMerk( rOut.GetLineColor() );
        rOut.SetLineColor( aCol );

        FASTBOOL bMap0=rOut.IsMapModeEnabled();
#ifdef WIN // SetPixel-Profiling fuer Windows
       COLORREF aWinColRef=PALETTERGB(aCol.GetRed()>>8,aCol.GetGreen()>>8,aCol.GetBlue()>>8);
       HDC aWinhDC=Sysdepen::GethDC(rOut);
#endif
#ifdef OS2 // SetPixel-Profiling fuer OS/2
        HPS aOS2hPS=Sysdepen::GethPS(rOut);
        LINEBUNDLE aOS2BundleMerker;
        memset(&aOS2BundleMerker,0,sizeof(aOS2BundleMerker));
        GpiQueryAttrs(aOS2hPS,PRIM_LINE,LBB_COLOR,&aOS2BundleMerker);
        LINEBUNDLE aOS2Bundle;
        memset(&aOS2Bundle,0,sizeof(aOS2Bundle));
        aOS2Bundle.lColor=RGBCOLOR(aCol.GetRed()>>8,aCol.GetGreen()>>8,aCol.GetBlue()>>8);
        GpiSetAttrs(aOS2hPS,PRIM_LINE,LBB_COLOR,0,&aOS2Bundle);
        long nOS2MaxYPix=rOut.GetOutputSizePixel().Height()-1;
#endif

//#ifdef JOEDEBUG
//        aPgOrg.X()=0;
//        aPgOrg.Y()=0;
//#endif

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
        long* pBuf = NULL;
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
                    UINT16 nSteps = nx1 / nx2;
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
                    UINT16 nSteps = ny1 / ny2;
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
        delete [] pBuf;
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
        FASTBOOL bOnlyTextEdit=FALSE;
        if (bTextEdit && !bDrawAll && !bPrinter && rView.IsTextEditFrame()) {
            Rectangle aTmpRect(rView.pTextEditOutlinerView->GetOutputArea());
            aTmpRect.Union(rView.aMinTextEditArea);
            //Rectangle aTmpRect(rView.aTextEditArea);
            aTmpRect.Left()  -=2*a1PixSiz.Width();
            aTmpRect.Top()   -=2*a1PixSiz.Height();
            aTmpRect.Right() +=2*a1PixSiz.Width();
            aTmpRect.Bottom()+=2*a1PixSiz.Height();
            // wenn der Invalidierte Bereich vollstaendig von der OutlinerView
            // verdeckt wird, male ich nur die OutlinerView und sonst nix.
            bOnlyTextEdit=aTmpRect.IsInside(rRect);
        }

        ImpSdrHdcMerk aHDCMerk(*pOut,SDRHDC_SAVEPENANDBRUSHANDFONT,rView.bRestoreColors);
        FASTBOOL bColorsDirty=FALSE;

        pXOut->SetOffset(aOfs);
        if (!bOnlyTextEdit) {
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
        }
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
}


void SdrPageView::ShowLayerSet(const XubString& rName, FASTBOOL bShow)
{
    if (pPage==NULL) return;
    SdrLayerSet* pSet=pPage->GetLayerAdmin().GetLayerSet(rName,TRUE);
    if (pSet!=NULL) {
        for (USHORT i=0; i<255; i++) {
            if (pSet->IsMember(BYTE(i))) {
                aLayerVisi.Set(BYTE(i),bShow);
            } else {
                if (bShow && pSet->IsExcluded(BYTE(i))) {
                    aLayerVisi.Clear(BYTE(i));
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
        aStr += GetEnteredLevel();
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
#ifdef JOEDEBUG
                    String aStr("Lesen der PageView fuer ");
                    if (bMaster) aStr+="Master";
                    aStr+="Page ";
                    aStr+=nPgNum+1;
                    DebWriteln(aStr);
#endif
                } break;
                case SDRIORECNAME_PAGVLAYER: {
                    rIn>>rPageView.aLayerVisi;
                    rIn>>rPageView.aLayerLock;
                    rIn>>rPageView.aLayerPrn;
#ifdef JOEDEBUG
                    DebWriteln("Lesen der Layerstati");
#endif
                } break;
                case SDRIORECNAME_PAGVHELPLINES: {
                    rIn>>rPageView.aHelpLines;
#ifdef JOEDEBUG
                    String aStr("Lesen der Hilfsliniendefinitionen (");
                    aStr+=rPageView.aHelpLines.GetCount();
                    aStr+=" Stueck)";
                    DebWriteln(aStr);
#endif
                } break;
                case SDRIORECNAME_PAGVAKTGROUP: {
                    //rIn>>aAktGroup; fehlende Implementation!
#ifdef JOEDEBUG
                    DebWriteln("Lesen der EnteredGroup - nicht implementiert");
#endif
                } break;
            }
        }
    }
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


