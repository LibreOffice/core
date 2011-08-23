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

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif


#include "svdpagv.hxx"
#include "svdouno.hxx"
#include "svdpage.hxx"
#include "svdio.hxx"
#include "svdview.hxx"
#include "svditer.hxx"
#include "svdogrp.hxx"

#include "svdoole2.hxx"
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

/*N*/ TYPEINIT1(SdrPageView, SfxListener);


// Klasse muﬂ als listener fungieren, um den Zustand, ob ein Object sichtbar ist oder nicht
// festzuhalten
//------------------------------------------------------------------------------
/*N*/ SdrUnoControlRec::SdrUnoControlRec(SdrUnoControlList* _pParent, SdrUnoObj* _pObj, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > _xControl) throw()
/*N*/ 				 :pObj(_pObj)
/*N*/ 				 ,xControl(_xControl)
/*N*/ 				 ,bVisible(TRUE)
/*N*/ 				 ,bIsListening(FALSE)
/*N*/ 				 ,bDisposed(FALSE)
/*N*/ 				 ,pParent(_pParent)
/*N*/                  ,mnPaintLevel( 0 )
/*N*/ {
/*N*/     DBG_ASSERT( xControl.is(), "SdrUnoControlRec::SdrUnoControlRec: invalid control, this will crash!" );
/*N*/ 
/*N*/     bVisible = xControl.is() ? !xControl->isDesignMode() : TRUE;
/*N*/     bool bOldVisible = bVisible;
/*N*/ 
/*N*/     // if bVisible is TRUE here, then switchControlListening will also start
/*N*/     // DesignModeListening
/*N*/     switchControlListening( true );
/*N*/ 
/*N*/     // adjust the initial visibility according to the visibility of the layer
/*N*/     // 2003-06-03 - #110592# - fs@openoffice.org
/*N*/     adjustControlVisibility( true );
/*N*/ 
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ SdrUnoControlRec::~SdrUnoControlRec() throw()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------------
/*?*/ void SdrUnoControlRec::adjustControlVisibility( bool _bForce )
/*?*/ {{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 
/*?*/ }

//------------------------------------------------------------------------------
/*?*/ void SdrUnoControlRec::switchControlListening( bool _bStart )
/*?*/ {{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 
/*?*/ }

//------------------------------------------------------------------------------
/*N*/ void SAL_CALL SdrUnoControlRec::disposing( const ::com::sun::star::lang::EventObject& Source )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	uno::Reference< awt::XControl > xSource( Source.Source, uno::UNO_QUERY);
/*N*/ 	if (xSource.is())
/*?*/ 	{  {DBG_BF_ASSERT(0, "STRIP");}//STRIP001  // it's the control we're responsible for
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------------

// XWindowListener
/*N*/ void SAL_CALL SdrUnoControlRec::windowResized( const ::com::sun::star::awt::WindowEvent& e )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SdrUnoControlRec::windowMoved( const ::com::sun::star::awt::WindowEvent& e )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SdrUnoControlRec::windowShown( const ::com::sun::star::lang::EventObject& e )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ void SAL_CALL SdrUnoControlRec::windowHidden( const ::com::sun::star::lang::EventObject& e )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// XPropertyChangeListener
//------------------------------------------------------------------------------
/*N*/ void SAL_CALL SdrUnoControlRec::propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// XImageConsumer
//------------------------------------------------------------------------------
/*N*/ void SAL_CALL SdrUnoControlRec::complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ void SAL_CALL SdrUnoControlRec::init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SdrUnoControlRec::setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SdrUnoControlRec::setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ }

/*N*/ void SAL_CALL SdrUnoControlRec::setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------------
/*?*/ void SAL_CALL SdrUnoControlRec::modeChanged( const util::ModeChangeEvent& _rSource ) throw (uno::RuntimeException)
/*?*/ {{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 
/*?*/ }

//------------------------------------------------------------------------------
/*N*/ void SdrUnoControlRec::Clear(BOOL bDispose)
/*N*/ {
/*N*/ 	if (xControl.is())
/*N*/ 	{
/*N*/         switchControlListening( false );
/*N*/ 
/*N*/ 		if (bDispose)
/*?*/ 			xControl->dispose();
/*N*/ 		xControl = NULL;
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ SV_IMPL_OP_PTRARR_SORT( SdrUnoControlAccessArr, SdrUnoControlAccessPtr )

//------------------------------------------------------------------------
/*N*/ SdrUnoControlRec* SdrUnoControlList::GetObject(USHORT i) const
/*N*/ {
/*N*/ 	SdrUnoControlAccess* pAccess = aAccessArr[i];
/*N*/ 	return pAccess->pControlRec;
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void SdrUnoControlList::Clear(BOOL bDispose)
/*N*/ {
/*N*/ 	for( USHORT i=aAccessArr.Count(); i > 0; )
/*N*/ 		Delete(--i, bDispose);
/*N*/ 
/*N*/ 	DBG_ASSERT(!aList.Count(), "SdrUnoControlList::Clear(BOOL bDispose): List not empty");
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void SdrUnoControlList::Insert(SdrUnoControlRec* pRec)
/*N*/ {
/*N*/ 	aList.Insert( pRec, LIST_APPEND );
/*N*/ 	pRec->acquire();
/*N*/ 
/*N*/ 	SdrUnoControlAccess* pAccess = new SdrUnoControlAccess(pRec->GetControl()->getModel(), pRec);
/*N*/ 	aAccessArr.Insert( pAccess );
/*N*/ }

//------------------------------------------------------------------------
/*N*/ void SdrUnoControlList::Delete(USHORT nPos, BOOL bDispose)
/*N*/ {
/*N*/ 	SdrUnoControlAccess* pAccess = aAccessArr[nPos];
/*N*/ 	SdrUnoControlRec* pRec = pAccess->pControlRec;
/*N*/ 
/*N*/ 	aList.Remove( pRec );
/*N*/ 	aAccessArr.Remove( nPos );
/*N*/ 
/*N*/ 	// Rec laeﬂt alles los und wird anschlieﬂend zerstoert
/*N*/ 	pRec->Clear(bDispose);
/*N*/ 
/*N*/ 	pRec->release();
/*N*/ 	delete pAccess;
/*N*/ }

//------------------------------------------------------------------------

//------------------------------------------------------------------------

//------------------------------------------------------------------------

//------------------------------------------------------------------------
/*N*/ USHORT SdrUnoControlList::Find(uno::Reference< awt::XControlModel > rUnoControlModel) const
/*N*/ {
/*N*/ 	SdrUnoControlAccess aAccess(rUnoControlModel);
/*N*/ 	USHORT nPos;
/*N*/ 	if( !aAccessArr.Seek_Entry(&aAccess, &nPos) )
/*N*/ 		return SDRUNOCONTROL_NOTFOUND;
/*N*/ 
/*N*/ 	return nPos;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ void SdrPageViewWinList::Clear()
/*N*/ {
/*N*/ 	USHORT nAnz=GetCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		delete GetObject(i);
/*N*/ 	}
/*N*/ 	aList.Clear();
/*N*/ }

/*N*/ USHORT SdrPageViewWinList::Find(OutputDevice* pOut) const
/*N*/ {
/*N*/ 	USHORT nAnz=GetCount();
/*N*/ 	USHORT nRet=SDRPAGEVIEWWIN_NOTFOUND;
/*N*/ 	for (USHORT nNum=0; nNum<nAnz && nRet==SDRPAGEVIEWWIN_NOTFOUND; nNum++) {
/*N*/ 		if (GetObject(nNum)->MatchOutputDevice(pOut))
/*N*/ 			nRet=nNum;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ SdrPageViewWinRec::SdrPageViewWinRec(SdrPageView& rNewPageView, OutputDevice* pOut)
/*N*/     :rView( rNewPageView.GetView() )
/*N*/ 	,pOutDev( pOut )
/*N*/     ,aControlList( rNewPageView )
/*N*/ {
/*N*/ }

/*N*/ SdrPageViewWinRec::~SdrPageViewWinRec()
/*N*/ {
/*N*/ 	if (xControlContainer.is())
/*N*/ 	{
/*N*/ 		// notify derived views
/*N*/ 		rView.RemoveControlContainer(xControlContainer);
/*N*/ 
/*N*/ 		// clear the control place holders
/*N*/ 		aControlList.Clear(FALSE);
/*N*/ 
/*N*/ 		// dispose the control container
/*N*/ 		uno::Reference< lang::XComponent > xComponent(xControlContainer, uno::UNO_QUERY);
/*N*/ 		xComponent->dispose();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPageViewWinRec::CreateControlContainer()
/*N*/ {
/*N*/ 	if (!xControlContainer.is())
/*N*/ 	{
/*N*/ 		if (pOutDev && pOutDev->GetOutDevType() == OUTDEV_WINDOW &&
/*N*/ 			!rView.IsPrintPreview())
/*N*/ 		{
/*N*/ 			Window* pWindow = (Window*) pOutDev;
/*N*/ 			xControlContainer = VCLUnoHelper::CreateControlContainer( pWindow );

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
            // 	pWindow->Hide();
            //	if( !mxContext.is() && bVisible )
            //		// Es ist ein TopWindow, also automatisch anzeigen
            //		createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (), ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > () );

/*N*/ 			uno::Reference< awt::XControl > xControl(xControlContainer, uno::UNO_QUERY);
/*N*/ 			if(xControl.is())
/*N*/ 			{
/*N*/ 				uno::Reference< uno::XInterface > xContext = xControl->getContext();
/*N*/ 				if(!xContext.is())
/*N*/ 				{
/*N*/ 					xControl->createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (),
/*N*/ 						::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > () );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			// Printer und VirtualDevice, bzw. kein OutDev
/*?*/ 			uno::Reference< lang::XMultiServiceFactory > xFactory( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*?*/ 			if( xFactory.is() )
/*?*/ 			{
/*?*/ 				xControlContainer = uno::Reference< awt::XControlContainer >(xFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainer")), uno::UNO_QUERY);
/*?*/ 				uno::Reference< awt::XControlModel > xModel(xFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlContainerModel")), uno::UNO_QUERY);
/*?*/ 				uno::Reference< awt::XControl > xControl(xControlContainer, uno::UNO_QUERY);
/*?*/ 				if (xControl.is())
/*?*/ 					xControl->setModel(xModel);
/*?*/ 
/*?*/ 				Point aPosPix;
/*?*/ 				Size aSizePix;
/*?*/ 
/*?*/ 				if ( pOutDev )
/*?*/ 				{
/*?*/ 					aPosPix = pOutDev->GetMapMode().GetOrigin();
/*?*/ 					aSizePix = pOutDev->GetOutputSizePixel();
/*?*/ 				}
/*?*/ 
/*?*/ 				uno::Reference< awt::XWindow > xContComp(xControlContainer, uno::UNO_QUERY);
/*?*/ 				if( xContComp.is() )
/*?*/ 					xContComp->setPosSize(aPosPix.X(), aPosPix.Y(), aSizePix.Width(), aSizePix.Height(), awt::PosSize::POSSIZE);
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		rView.InsertControlContainer(xControlContainer);
/*N*/ 	}
/*N*/ }

/*N*/ BOOL SdrPageViewWinRec::MatchOutputDevice(OutputDevice* pOut) const
/*N*/ {
/*N*/ 	BOOL bMatch = FALSE;
/*N*/ 
/*N*/ 	if (pOutDev == pOut)
/*N*/ 	{
/*N*/ 		bMatch = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return (bMatch);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ DBG_NAME(SdrPageView)


/*N*/ SdrPageView::SdrPageView(SdrPage* pPage1, const Point& rOffs, SdrView& rNewView):
/*N*/ 	rView(rNewView),
/*N*/ 	//aRedraw(1024,16,16),
/*N*/ 	aOfs(rOffs),
/*N*/ 	pPaintingPageObj( NULL ),
/*N*/ 	maDocumentColor( COL_AUTO )		// #103911# col_auto color lets the view takes the default SvxColorConfig entry
/*N*/ {
/*N*/ 	DBG_CTOR(SdrPageView,NULL);
/*N*/ 	pDragPoly0=new XPolyPolygon;
/*N*/ 	pDragPoly=new XPolyPolygon;
/*N*/ 	pWinList=new SdrPageViewWinList;
/*N*/ 	pPage=pPage1;
/*N*/ 	if (pPage!=NULL) {
/*N*/ 		aPgOrg.X()=pPage->GetLftBorder();
/*N*/ 		aPgOrg.Y()=pPage->GetUppBorder();
/*N*/ 	}
/*N*/ //	  aOut.SetOutDev(rView.pWin);
/*N*/ //	  aOut.SetOffset(rOffs);
/*N*/ //	  eDrwStat=RS_READY;
/*N*/ 	bHasMarked=FALSE;
/*N*/ 	//aDragPoly.Clear();
/*N*/ 	aLayerVisi.SetAll();
/*N*/ 	aLayerPrn.SetAll();
/*N*/ 	bVisible=FALSE;
/*N*/ 
/*N*/ 	pAktList = NULL;
/*N*/ 	pAktGroup = NULL;
/*N*/ 	SetAktGroupAndList(NULL, pPage);
/*N*/ 
/*N*/ 	StartListening(*rNewView.GetModel());
/*N*/ 	USHORT nWinAnz=rNewView.GetWinCount();
/*N*/ 	for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++) {
/*N*/ 		AddWin(rNewView.GetWin(nWinNum));
/*N*/ 	}
/*N*/ }


/*N*/ SdrPageView::~SdrPageView()
/*N*/ {
/*N*/ 	if (rView.GetModel()->GetPaintingPageView() == this)
/*N*/ 	{
        // Abmelden
/*?*/ 		rView.GetModel()->SetPaintingPageView(NULL);
/*N*/ 	}
/*N*/ 
/*N*/ 	DBG_DTOR(SdrPageView,NULL);
/*N*/ 	delete pWinList;
/*N*/ 	delete pDragPoly0;
/*N*/ 	delete pDragPoly;
/*N*/ }

/*N*/ SdrPageViewWinRec* SdrPageView::ImpMakePageViewWinRec(OutputDevice* pOut)
/*N*/ {
/*N*/ 	// MIB 3.7.08: Das WinRec muss sofort in die Liste eingetragen werden,
/*N*/ 	// weil sich das InsertControlContainer darauf verlaesst
/*N*/ 	SdrPageViewWinRec* pRec = new SdrPageViewWinRec( *this, pOut );
/*N*/ 	pWinList->Insert(pRec);
/*N*/ 
/*N*/ 	ULONG nObjAnz=pPage!=NULL?pPage->GetObjCount():0;
/*N*/ 
/*N*/ 	for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++)
/*N*/ 	{
/*N*/ 		SdrObject* pObj = pPage->GetObj(nObjNum);
/*N*/ 
/*N*/ 		if (pObj->IsUnoObj())
/*N*/ 		{
/*N*/ 			SdrUnoObj* pSdrUnoObj = PTR_CAST(SdrUnoObj, pObj);
/*N*/ 			ImpInsertControl(pSdrUnoObj, pRec);
/*N*/ 		}
/*N*/ 		else if (pObj->GetObjIdentifier() == OBJ_GRUP &&
/*N*/ 				 pObj->GetObjInventor() == SdrInventor)
/*N*/ 		{
/*N*/ 			// Gruppenobjekt: sind Uno-Objekte enthalten?
/*N*/ 			SdrObjListIter aIter(*((SdrObjGroup*) pObj)->GetSubList(), IM_DEEPNOGROUPS);
/*N*/ 
/*N*/ 			SdrObject* pObj = NULL;
/*N*/ 
/*N*/ 			while (aIter.IsMore())
/*N*/ 			{
/*N*/ 				pObj = aIter.Next();
/*N*/ 
/*N*/ 				if (pObj && pObj->IsUnoObj())
/*N*/ 				{
/*?*/ 					SdrUnoObj* pSdrUnoObj = PTR_CAST(SdrUnoObj, pObj);
/*?*/ 					ImpInsertControl(pSdrUnoObj, pRec);
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRec;
/*N*/ }

/*N*/ void SdrPageView::AddWin(OutputDevice* pOutDev)
/*N*/ {
/*N*/ 	USHORT nPos = pWinList->Find(pOutDev);
/*N*/ 
/*N*/ 	if (nPos == SDRPAGEVIEWWIN_NOTFOUND)
/*N*/ 		SdrPageViewWinRec* pWinRec = ImpMakePageViewWinRec(pOutDev);
/*N*/ }

/*N*/ void SdrPageView::DelWin(OutputDevice* pOutDev)
/*N*/ {
/*N*/ 	USHORT nPos=pWinList->Find(pOutDev);
/*N*/ 
/*N*/ 	if (nPos != SDRPAGEVIEWWIN_NOTFOUND)
/*N*/ 	{
/*N*/ 		pWinList->Delete(nPos);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPageView::ImpInsertControl(const SdrUnoObj* pSdrUnoObj,
/*N*/ 								   SdrPageViewWinRec* pRec)
/*N*/ {
/*N*/ 	if (pSdrUnoObj)
/*N*/ 	{
/*N*/ 		uno::Reference< awt::XControlModel > xUnoControlModel(pSdrUnoObj->GetUnoControlModel());
/*N*/ 		if (!xUnoControlModel.is())
/*N*/ 			return;
/*N*/ 
/*N*/ 		USHORT nCtrlNum = pRec->aControlList.Find(xUnoControlModel);
/*N*/ 		if (nCtrlNum == SDRUNOCONTROL_NOTFOUND)
/*N*/ 		{
/*N*/ 			// Control fuer die View erzeugen
/*N*/ 			uno::Reference< lang::XMultiServiceFactory > xFactory( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/ 			uno::Reference< awt::XControl > xUnoControl;
/*N*/ 			if( xFactory.is() )
/*N*/ 				xUnoControl = uno::Reference< awt::XControl >(xFactory->createInstance(pSdrUnoObj->GetUnoControlTypeName()), uno::UNO_QUERY);
/*N*/ 
/*N*/ 			if (xUnoControl.is())
/*N*/ 			{
/*N*/ 				xUnoControl->setModel(xUnoControlModel);
/*N*/ 				if (pRec->pOutDev->GetOutDevType() != OUTDEV_WINDOW)
/*N*/ 				{
/*?*/ 					uno::Reference< awt::XView > xView = xUnoControl->getView();
/*?*/ 					if (xView.is())
/*?*/ 					{
/*?*/ //						  xView->setGraphics(pRec->pOutDev->GetGraphicsInterface(TRUE));
/*?*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				// ControlContainer ggf. erzeugen
/*N*/ 				pRec->CreateControlContainer();
/*N*/ 
/*N*/ 				// xControl in ControlContainer einfuegen (Eigentuemer-Uebergang)
/*N*/ 				if (pRec->GetControlContainerRef().is())
/*N*/ 				{
/*N*/ 					uno::Reference< awt::XWindow > xWindow(xUnoControl, uno::UNO_QUERY);
/*N*/ 					if (xWindow.is())
/*N*/ 					{
/*N*/ 						Rectangle aRect(pSdrUnoObj->GetLogicRect());
/*N*/ 						Point aPixPos(pRec->pOutDev->LogicToPixel(aRect.TopLeft()));
/*N*/ 						Size aPixSize(pRec->pOutDev->LogicToPixel(aRect.GetSize()));
/*N*/ 						xWindow->setPosSize(aPixPos.X(), aPixPos.Y(), aPixSize.Width(), aPixSize.Height(), awt::PosSize::POSSIZE);
/*N*/ 					}
/*N*/ 
/*N*/ 					// set the correct zoom
/*N*/ 					if (!GetView().IsDesignMode())
/*N*/ 					{
/*N*/ 						uno::Reference< awt::XView > xView(xUnoControl, uno::UNO_QUERY);
/*N*/ 						if (xView.is())
/*N*/ 						{
/*N*/ 							const MapMode& rMap = pRec->pOutDev->GetMapMode();
/*N*/ 							xView->setZoom((float) double(rMap.GetScaleX()),
/*N*/ 										   (float) double(rMap.GetScaleY()));
/*N*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					// #107049# set design mode before peer is created,
/*N*/ 					// this is also needed for accessibility
/*N*/ 					xUnoControl->setDesignMode(GetView().IsDesignMode());
/*N*/ 
/*N*/ 					SdrUnoControlRec* pUCR = new SdrUnoControlRec(&pRec->aControlList, (SdrUnoObj*)pSdrUnoObj, xUnoControl);
/*N*/ 					pRec->aControlList.Insert(pUCR);
/*N*/ 
/*N*/                     // #108327# do this last - the addition of the control triggeres processes which rely
/*N*/                     // on the control already being inserted into the aControlList
/*N*/ 					pRec->GetControlContainerRef()->addControl(pSdrUnoObj->GetUnoControlTypeName(), xUnoControl);
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SdrPageView::ImpUnoInserted(const SdrObject* pObj)
/*N*/ {
/*N*/ 	SdrUnoObj* pSdrUnoObj = (SdrUnoObj*)pObj;
/*N*/ 	USHORT nPos = pWinList->GetCount();
/*N*/ 
/*N*/ 	for (; nPos; )
/*N*/ 		ImpInsertControl(pSdrUnoObj, &(*pWinList)[--nPos]);
/*N*/ 
/*N*/ }



/*N*/ void __EXPORT SdrPageView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	if (bVisible) {
/*N*/ 		SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
/*N*/ 		if (pSdrHint!=NULL) {
/*N*/ 			SdrHintKind eKind=pSdrHint->GetKind();
/*N*/ 			const SdrObject* pObj=pSdrHint->GetObject();
/*N*/ 			if ( pObj!=NULL && pObj->GetPage() == pPage )
/*N*/ 			{
/*N*/ 				if (pObj->IsUnoObj())
/*N*/ 				{
/*N*/ 					if (eKind == HINT_OBJINSERTED ||
/*N*/ 						eKind == HINT_CONTROLINSERTED)
/*N*/ 					{
/*N*/ 						ImpUnoInserted(pObj);
/*N*/ 					}
/*N*/ 					else if (eKind == HINT_OBJREMOVED ||
/*N*/ 							 eKind == HINT_CONTROLREMOVED)
/*N*/ 
/*N*/ 					{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	ImpUnoRemoved(pObj);
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if (pObj->GetObjIdentifier() == OBJ_GRUP &&
/*N*/ 						 pObj->GetObjInventor() == SdrInventor)
/*N*/ 				{
/*N*/ 					// Gruppenobjekt: sind Uno-Objekte enthalten?
/*N*/ 					SdrObjListIter aIter(*((SdrObjGroup*) pObj)->GetSubList(), IM_DEEPNOGROUPS);
/*N*/ 
/*N*/ 					SdrObject* pObj = NULL;
/*N*/ 
/*N*/ 					while (aIter.IsMore())
/*N*/ 					{
/*N*/ 						pObj = aIter.Next();
/*N*/ 
/*N*/ 						if (pObj && pObj->IsUnoObj())
/*N*/ 						{
/*?*/ 							if (eKind == HINT_OBJINSERTED ||
/*?*/ 								eKind == HINT_CONTROLINSERTED)
/*?*/ 
/*?*/ 							{
/*?*/ 								ImpUnoInserted(pObj);
/*?*/ 							}
/*?*/ 							else if (eKind == HINT_OBJREMOVED ||
/*?*/ 									 eKind == HINT_CONTROLREMOVED)
/*?*/ 
/*?*/ 							{
/*?*/ 								DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ImpUnoRemoved(pObj);
/*?*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if (pSdrHint->IsNeedRepaint()) {
/*N*/ 				if (((eKind==HINT_OBJCHG
/*N*/ 					|| eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED
/*N*/ 					|| eKind == HINT_CONTROLINSERTED || eKind == HINT_CONTROLREMOVED) &&
/*N*/ 					 pSdrHint->GetPage()!=NULL) ||
/*N*/ 					eKind==HINT_PAGECHG)
/*N*/ 				{
/*N*/ 					FASTBOOL bInv=FALSE;
/*N*/ 					if (pSdrHint->GetPage()==pPage) bInv=TRUE;
/*N*/ 					else if (pSdrHint->GetPage()->IsMasterPage()) { // ebenfalls Invalidate, wenn pPage die geaenderte Page als MasterPage referenziert
/*N*/ 						USHORT nMaPgAnz=pPage!=NULL?pPage->GetMasterPageCount():0;
/*N*/ 						for (USHORT i=0; i<nMaPgAnz && !bInv; i++) {
/*N*/ 							const SdrPage* pMPg=pPage->GetMasterPage(i);
/*N*/ 							bInv=pMPg==pSdrHint->GetPage();
/*N*/ 						}
/*N*/ 					}
/*N*/ 					if (bInv) {
/*N*/ 						InvalidateAllWin(pSdrHint->GetRect(),TRUE);
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if (eKind==HINT_OBJLISTCLEARED && pSdrHint->GetPage()==pPage) {
/*?*/ 				if (GetAktGroup()!=NULL) {
/*?*/ 					rView.UnmarkAllObj();
/*?*/ 					LeaveAllGroup();
/*?*/ 					//pWinList ...
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SdrPageView::ModelHasChanged()
/*N*/ {
/*N*/ 	if (GetAktGroup()!=NULL) CheckAktGroup();
/*N*/ }




/*N*/ void SdrPageView::Show()
/*N*/ {
/*N*/ 	if (!bVisible) {
/*N*/ 		bVisible=TRUE;
/*N*/ 		InvalidateAllWin();
/*N*/ 		USHORT nWinAnz=rView.GetWinCount();
/*N*/ 		for (USHORT nWinNum=0; nWinNum<nWinAnz; nWinNum++) {
/*N*/ 			AddWin(rView.GetWin(nWinNum));
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SdrPageView::Hide()
/*N*/ {
/*N*/ 	if (bVisible) {
/*N*/ 		InvalidateAllWin();
/*N*/ 		bVisible=FALSE;
/*N*/ 		pWinList->Clear();
/*N*/ 	}
/*N*/ }




/*N*/ Rectangle SdrPageView::GetPageRect() const
/*N*/ {
/*N*/ 	if (pPage==NULL) return Rectangle();
/*N*/ 	return Rectangle(GetOffset(),Size(pPage->GetWdt()+1,pPage->GetHgt()+1));
/*N*/ }


/*N*/ void SdrPageView::InvalidateAllWin()
/*N*/ {
/*N*/ 	if (bVisible && pPage!=NULL) {
/*N*/ 		Rectangle aRect(Point(0,0),Size(pPage->GetWdt()+1,pPage->GetHgt()+1));
/*N*/ 		aRect.Union(pPage->GetAllObjBoundRect());
/*N*/ 		aRect.Move(aOfs.X(),aOfs.Y());
/*N*/ 		rView.InvalidateAllWin(aRect);
/*N*/ 	}
/*N*/ }


/*N*/ void SdrPageView::InvalidateAllWin(const Rectangle& rRect, FASTBOOL bPlus1Pix)
/*N*/ {
/*N*/ 	if (bVisible) {
/*N*/ 		rView.InvalidateAllWin(rRect+GetOffset(),bPlus1Pix);
/*N*/ 	}
/*N*/ }

#ifdef OS2
#define RGBCOLOR(r,g,b) ((ULONG)(((BYTE)(b) | ((USHORT)(g)<<8)) | (((ULONG)(BYTE)(r))<<16)))
#endif

/*N*/ FASTBOOL SdrPageView::IsLayer(const XubString& rName, const SetOfByte& rBS) const
/*N*/ {
/*N*/ 	if(!pPage)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	BOOL bRet(FALSE);
/*N*/ 
/*N*/ 	if(rName.Len())
/*N*/ 	{
/*N*/ 		SdrLayerID nId = pPage->GetLayerAdmin().GetLayerID(rName, TRUE);
/*N*/ 
/*N*/ 		if(nId != SDRLAYER_NOTFOUND)
/*N*/ 		{
/*N*/ 			bRet = rBS.IsSet(nId);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ FASTBOOL SdrPageView::IsObjMarkable(SdrObject* pObj) const
/*N*/ {
/*N*/ 	if(pObj)
/*N*/ 	{
/*N*/ 		// Vom Markieren ausgeschlossen?
/*N*/ 		if(pObj->IsMarkProtect())
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 		// Der Layer muss sichtbar und darf nicht gesperrt sein
/*N*/ 		SdrLayerID nL = pObj->GetLayer();
/*N*/ 		return aLayerVisi.IsSet(BYTE(nL)) && !aLayerLock.IsSet(BYTE(nL));
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


// Betretene Gruppe und Liste setzen
/*N*/ void SdrPageView::SetAktGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList)
/*N*/ {
/*N*/ 	if(pAktGroup != pNewGroup)
/*N*/ 	{
/*?*/ 		pAktGroup = pNewGroup;
/*N*/ 	}
/*N*/ 	if(pAktList != pNewList)
/*N*/ 	{
/*N*/ 		pAktList = pNewList;
/*N*/ 	}
/*N*/ }




/*N*/ void SdrPageView::LeaveAllGroup()
/*N*/ {
/*N*/ 	if(GetAktGroup())
/*N*/ 	{
/*?*/ 		BOOL bDisInvalidate = (rView.pDisabledAttr != NULL);
/*?*/ 		BOOL bGlueInvalidate = (!bDisInvalidate && rView.ImpIsGlueVisible());
/*?*/ 
/*?*/ 		if(bGlueInvalidate)
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 rView.GlueInvalidate();
/*?*/ 
/*?*/ 		SdrObject* pLastGroup = GetAktGroup();
/*?*/ 
/*?*/ 		// Alles deselektieren
/*?*/ 		rView.UnmarkAll();
/*?*/ 
/*?*/ 		// Zuweisungen, pAktGroup und pAktList muessen gesetzt sein
/*?*/ 		SetAktGroupAndList(NULL, pPage);
/*?*/ 
/*?*/ 		// Oberste letzte Gruppe finden und selektieren
/*?*/ 		if(pLastGroup)
/*?*/ 		{
/*?*/ 			while(pLastGroup->GetUpGroup())
/*?*/ 				pLastGroup = pLastGroup->GetUpGroup();
/*?*/ 
/*?*/ 			for(UINT16 nv = 0; nv < rView.GetPageViewCount(); nv++)
/*?*/ 				rView.MarkObj(pLastGroup, rView.GetPageViewPvNum(nv));
/*?*/ 		}
/*?*/ 
/*?*/ 		rView.AdjustMarkHdl();
/*?*/ 
/*?*/ 		// invalidate only when view wants to visualize group entering
/*?*/ 		if(rView.DoVisualizeEnteredGroup())
/*?*/ 			InvalidateAllWin();
/*?*/ 
/*?*/ 		if(bGlueInvalidate)
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 rView.GlueInvalidate();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPageView::CheckAktGroup()
/*N*/ {
/*N*/ 	SdrObject* pGrp=GetAktGroup();
/*N*/ 	while (pGrp!=NULL &&
/*N*/ 		   (!pGrp->IsInserted() || pGrp->GetObjList()==NULL ||
/*N*/ 			pGrp->GetPage()==NULL || pGrp->GetModel()==NULL)) { // irgendwas daneben?
/*N*/ 		pGrp=pGrp->GetUpGroup();
/*N*/ 	}
/*N*/ 	if (pGrp!=GetAktGroup()) {
/*?*/ 		if (pGrp!=NULL) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 EnterGroup(pGrp);
/*N*/ 		else LeaveAllGroup();
/*N*/ 	}
/*N*/ }


/*N*/ SvStream& operator<<(SvStream& rOut, const SdrPageView& rPageView)
/*N*/ {
/*N*/ 	SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOPgVwID);
/*N*/ 	{
/*N*/ 		if (rPageView.pPage!=NULL) {
/*N*/ 			SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_PAGVIEW);
/*N*/ 			rOut<<BOOL(rPageView.bVisible);
/*N*/ 			rOut<<BOOL(rPageView.pPage->IsMasterPage());
/*N*/ 			rOut<<rPageView.pPage->GetPageNum();
/*N*/ 			rOut<<rPageView.aOfs;
/*N*/ 			rOut<<rPageView.aPgOrg;
/*N*/ 		}
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_PAGVLAYER);
/*N*/ 		rOut<<rPageView.aLayerVisi;
/*N*/ 		rOut<<rPageView.aLayerLock;
/*N*/ 		rOut<<rPageView.aLayerPrn;
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_PAGVHELPLINES);
/*N*/ 		rOut<<rPageView.aHelpLines;
/*N*/ 	}
/*N*/ 	//if (GetAktGroup()!=NULL) {
/*N*/ 	//	  // ...
/*N*/ 	//	  //rOut<<aAktGroup;
/*N*/ 	//}
/*N*/ 	return rOut;
/*N*/ }


/*N*/ SvStream& operator>>(SvStream& rIn, SdrPageView& rPageView)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return rIn;
/*N*/ 	SdrIOHeader aHead(rIn,STREAM_READ);
/*N*/ 	while (aHead.GetBytesLeft()>0 && rIn.GetError()==0 && !rIn.IsEof()) {
/*N*/ 		SdrNamedSubRecord aSubRecord(rIn,STREAM_READ);
/*N*/ 		if (aSubRecord.GetInventor()==SdrInventor) {
/*N*/ 			switch (aSubRecord.GetIdentifier()) {
/*N*/ 				case SDRIORECNAME_PAGVIEW: {
/*N*/ 					BOOL bVisible;
/*N*/ 					BOOL bMaster;
/*N*/ 					USHORT nPgNum;
/*N*/ 					rIn>>bVisible;
/*N*/ 					rPageView.bVisible=bVisible;
/*N*/ 					rIn>>bMaster;
/*N*/ 					rIn>>nPgNum;
/*N*/ 					rIn>>rPageView.aOfs;
/*N*/ 					rIn>>rPageView.aPgOrg;
/*N*/ 					SdrModel* pMod=rPageView.GetView().GetModel();
/*N*/ 					if (!bMaster) rPageView.pPage=pMod->GetPage(nPgNum);
/*N*/ 					else rPageView.pPage=pMod->GetMasterPage(nPgNum);
/*N*/ 					rPageView.pAktList=rPageView.pPage;
/*N*/ 				} break;
/*N*/ 				case SDRIORECNAME_PAGVLAYER: {
/*N*/ 					rIn>>rPageView.aLayerVisi;
/*N*/ 					rIn>>rPageView.aLayerLock;
/*N*/ 					rIn>>rPageView.aLayerPrn;
/*N*/ 				} break;
/*N*/ 				case SDRIORECNAME_PAGVHELPLINES: {
/*N*/ 					rIn>>rPageView.aHelpLines;
/*N*/ 				} break;
/*N*/ 				case SDRIORECNAME_PAGVAKTGROUP: {
/*N*/ 					//rIn>>aAktGroup; fehlende Implementation!
/*N*/ 				} break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return rIn;
/*N*/ }

// #103834# Set background color for svx at SdrPageViews

////////////////////////////////////////////////////////////////////////////////////////////////////


}
