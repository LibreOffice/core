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

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif


#ifndef _SVDOUNO_HXX
#include "svdouno.hxx"
#endif
#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif
#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif
#include "svdstr.hrc"   // Objektname
#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif
#ifndef _SVDVITER_HXX
#include "svdviter.hxx"
#endif

#include <set>

using namespace ::rtl;
using namespace ::com::sun::star;

//************************************************************
//   Defines
//************************************************************

//************************************************************
//   Hilfsklasse SdrControlEventListenerImpl
//************************************************************


#include <cppuhelper/implbase1.hxx>
namespace binfilter {

/*N*/ class SdrControlEventListenerImpl : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
/*N*/ {
/*N*/ protected:
/*N*/ 	SdrUnoObj*					pObj;
/*N*/ 
/*N*/ public:
/*N*/ 	SdrControlEventListenerImpl(SdrUnoObj* _pObj)
/*N*/ 	:	pObj(_pObj)
/*N*/ 	{}
/*N*/ 
/*N*/ 	// XEventListener
/*N*/     virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
/*N*/ 
/*N*/ 	void StopListening(const uno::Reference< lang::XComponent >& xComp);
/*N*/ 	void StartListening(const uno::Reference< lang::XComponent >& xComp);
/*N*/ };

// XEventListener
/*N*/ void SAL_CALL SdrControlEventListenerImpl::disposing( const ::com::sun::star::lang::EventObject& Source )
/*N*/ 	throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	if (pObj)
/*N*/ 	{
/*N*/ 		pObj->xUnoControlModel = NULL;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrControlEventListenerImpl::StopListening(const uno::Reference< lang::XComponent >& xComp)
/*N*/ {
/*N*/ 	if (xComp.is())
/*N*/ 		xComp->removeEventListener(this);
/*N*/ }

/*N*/ void SdrControlEventListenerImpl::StartListening(const uno::Reference< lang::XComponent >& xComp)
/*N*/ {
/*N*/ 	if (xComp.is())
/*N*/ 		xComp->addEventListener(this);
/*N*/ }

// -----------------------------------------------------------------------------
/*?*/ namespace
/*?*/ {
/*?*/     void lcl_ensureControlVisibility( SdrView* _pView, const SdrUnoObj* _pObject, bool _bVisible )
/*?*/     {
/*?*/         SdrPageView* pPageView = _pView ? _pView->GetPageView( _pObject->GetPage() ) : NULL;
/*?*/         DBG_ASSERT( pPageView, "lcl_ensureControlVisibility: no view found!" );
/*?*/ 
/*?*/         if ( pPageView )
/*?*/         {
/*?*/             // loop through all the views windows
/*?*/             const SdrPageViewWinList& rViewWins = pPageView->GetWinList();
/*?*/             USHORT nWins = rViewWins.GetCount();
/*?*/             for ( USHORT i=0; i<nWins; ++i )
/*?*/             {
/*?*/                 const SdrPageViewWinRec& rWinData = rViewWins[i];
/*?*/ 
/*?*/                 // loop through all controls in this window
/*?*/                 const SdrUnoControlList& rControlsInThisWin = rWinData.GetControlList();
/*?*/                 USHORT nControlsInThisWin = rControlsInThisWin.GetCount();
/*?*/                 for ( USHORT j=0; j<nControlsInThisWin; ++j )
/*?*/                 {
/*?*/                     const SdrUnoControlRec& rControlData = rControlsInThisWin[j];
/*?*/                     if ( rControlData.GetUnoObj() == _pObject )
/*?*/                     {
/*?*/                         // yep - this control is the representation of the given FmFormObj in the
/*?*/                         // given view
/*?*/                         // is the control in alive mode?
/*?*/                         uno::Reference< awt::XControl > xControl( rControlData.GetControl(), uno::UNO_QUERY );
/*?*/                         DBG_ASSERT( xControl.is(), "lcl_ensureControlVisibility: no control!" );
/*?*/                         if ( xControl.is() && !xControl->isDesignMode() )
/*?*/                         {
/*?*/                             // yes, alive mode. Is the visibility correct?
/*?*/                             if ( (bool)rControlData.IsVisible() != _bVisible )
/*?*/                             {
/*?*/                                 // no -> adjust it
/*?*/                                 uno::Reference< awt::XWindow > xControlWindow( xControl, uno::UNO_QUERY );
/*?*/                                 DBG_ASSERT( xControlWindow.is(), "lcl_ensureControlVisibility: the control is no window!" );
/*?*/                                 if ( xControlWindow.is() )
/*?*/                                 {
/*?*/                                     xControlWindow->setVisible( _bVisible );
/*?*/                                     DBG_ASSERT( (bool)rControlData.IsVisible() == _bVisible, "lcl_ensureControlVisibility: this didn't work!" );
/*?*/                                         // now this would mean that either IsVisible is not reliable (which would
/*?*/                                         // be bad 'cause we used it above) or that showing/hiding the window
/*?*/                                         // did not work as intended.
/*?*/                                 }
/*?*/                             }
/*?*/                         }
/*?*/                     }
/*?*/                 }
/*?*/             }
/*?*/         }
/*?*/     }
/*?*/ }

//************************************************************
//   SdrUnoObj
//************************************************************

/*N*/ TYPEINIT1(SdrUnoObj, SdrRectObj);

/*N*/ SdrUnoObj::SdrUnoObj(const String& rModelName, BOOL _bOwnUnoControlModel)
/*N*/ :	bOwnUnoControlModel(_bOwnUnoControlModel)
/*N*/ {
/*N*/ 	bIsUnoObj = TRUE;
/*N*/ 
/*N*/ 	pEventListener = new SdrControlEventListenerImpl(this);
/*N*/ 	pEventListener->acquire();
/*N*/ 
/*N*/ 	// nur ein owner darf eigenstaendig erzeugen
/*N*/ 	if (rModelName.Len())
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	CreateUnoControlModel(rModelName);
/*N*/ }

/*N*/ SdrUnoObj::~SdrUnoObj()
/*N*/ {
/*N*/ 	uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
/*N*/ 	if (xComp.is())
/*N*/ 	{
/*N*/ 		// gehoert das Control seiner Umgebung?
/*N*/ 		uno::Reference< container::XChild > xContent(xUnoControlModel, uno::UNO_QUERY);
/*N*/ 		if (xContent.is() && !xContent->getParent().is())
/*N*/ 			xComp->dispose();
/*N*/ 		else
/*N*/ 			pEventListener->StopListening(xComp);
/*N*/ 	}
/*N*/ 	pEventListener->release();
/*N*/ }

/*N*/ void SdrUnoObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	SdrRectObj::SetModel(pNewModel);
/*N*/ }

/*N*/ void SdrUnoObj::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/ 	SdrRectObj::SetPage(pNewPage);
/*N*/ }



/** helper class to restore graphics at <awt::XView> object after <SdrUnoObj::Paint>

    OD 08.05.2003 #109432#
    Restoration of graphics necessary to assure that paint on a window

    @author OD
*/
/*?*/ class RestoreXViewGraphics
/*?*/ {
/*?*/     private:
/*?*/         uno::Reference< awt::XView >        m_rXView;
/*?*/         uno::Reference< awt::XGraphics >    m_rXGraphics;
/*?*/ 
/*?*/     public:
/*?*/         RestoreXViewGraphics( const uno::Reference< awt::XView >& _rXView )
/*?*/         {
/*?*/              m_rXView = _rXView;
/*?*/              m_rXGraphics = m_rXView->getGraphics();
/*?*/         }
/*?*/         ~RestoreXViewGraphics()
/*?*/         {
/*?*/             m_rXView->setGraphics( m_rXGraphics );
/*?*/         }
/*?*/ };







/*N*/ void SdrUnoObj::VisAreaChanged(const OutputDevice* pOut)
/*N*/ {
/*N*/ 	if (!xUnoControlModel.is())
/*N*/ 		return;
/*N*/ 
/*N*/ 	if (pOut)
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
        // Nur dieses eine OutDev beruecksichtigen
/*N*/ 	}
/*N*/ 	else if (pModel)
/*N*/ 	{
/*N*/ 		// Controls aller PageViews beruecksichtigen
/*N*/ 		USHORT nLstPos = pModel->GetListenerCount();
/*N*/ 		uno::Reference< awt::XWindow > xWindow;
/*N*/ 		Point aPixPos;
/*N*/ 		Size aPixSize;
/*N*/ 		SfxListener* pListener;
/*N*/ 		SdrPageView* pPV;
/*N*/ 		const SdrUnoControlRec* pControlRec = NULL;
/*N*/ 
/*N*/ 		for (; nLstPos ;)
/*N*/ 		{
/*N*/ 			pListener = pModel->GetListener(--nLstPos);
/*N*/ 
/*N*/ 			if (pListener && pListener->ISA(SdrPageView))
/*N*/ 			{
/*N*/ 				pPV = (SdrPageView*) pListener;
/*N*/ 				const SdrPageViewWinList& rWL = pPV->GetWinList();
/*N*/ 				USHORT nPos = rWL.GetCount();
/*N*/ 
/*N*/ 				for (; nPos ; )
/*N*/ 				{
/*N*/ 					// Controls aller OutDevs beruecksichtigen
/*N*/ 					const SdrPageViewWinRec& rWR = rWL[--nPos];
/*N*/ 					const SdrUnoControlList& rControlList = rWR.GetControlList();
/*N*/ 					USHORT nCtrlNum = rControlList.Find(xUnoControlModel);
/*N*/ 					pControlRec = (nCtrlNum != SDRUNOCONTROL_NOTFOUND) ? &rControlList[nCtrlNum] : NULL;
/*N*/ 					if (pControlRec)
/*N*/ 					{
/*N*/ 						xWindow = uno::Reference< awt::XWindow >(pControlRec->GetControl(), uno::UNO_QUERY);
/*N*/ 						if (xWindow.is())
/*N*/ 						{
/*N*/ 							// #62560 Pixelverschiebung weil mit einem Rechteck
/*N*/ 							// und nicht mit Point, Size gearbeitet wurde
/*N*/ 							aPixPos = rWR.GetOutputDevice()->LogicToPixel(aRect.TopLeft());
/*N*/ 							aPixSize = rWR.GetOutputDevice()->LogicToPixel(aRect.GetSize());
/*N*/ 							xWindow->setPosSize(aPixPos.X(), aPixPos.Y(),
/*N*/ 												aPixSize.Width(), aPixSize.Height(),
/*N*/ 												awt::PosSize::POSSIZE);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrUnoObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	SdrRectObj::NbcResize(rRef,xFact,yFact);
/*N*/ 
/*N*/ 	if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0)
/*N*/ 	{
/*N*/ 		// kleine Korrekturen
/*?*/ 		if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000)
/*?*/ 		{
/*?*/ 			aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
/*?*/ 		}
/*?*/ 
/*?*/ 		aGeo.nDrehWink	= 0;
/*?*/ 		aGeo.nShearWink = 0;
/*?*/ 		aGeo.nSin		= 0.0;
/*?*/ 		aGeo.nCos		= 1.0;
/*?*/ 		aGeo.nTan		= 0.0;
/*?*/ 		SetRectsDirty();
/*N*/ 	}
/*N*/ 
/*N*/ 	VisAreaChanged();
/*N*/ }

/*N*/ void SdrUnoObj::NbcMove(const Size& rSize)
/*N*/ {
/*N*/ 	SdrRectObj::NbcMove(rSize);
/*N*/ 	VisAreaChanged();
/*N*/ }

/*N*/ void SdrUnoObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetSnapRect(rRect);
/*N*/ 	VisAreaChanged();
/*N*/ }

/*N*/ void SdrUnoObj::NbcSetLogicRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetLogicRect(rRect);
/*N*/ 	VisAreaChanged();
/*N*/ }

// -----------------------------------------------------------------------------
/*N*/ void SdrUnoObj::NbcSetLayer( SdrLayerID _nLayer )
/*N*/ {
/*N*/     if ( GetLayer() == _nLayer )
/*N*/     {   // redundant call -> not interested in doing anything here
/*N*/         SdrRectObj::NbcSetLayer( _nLayer );
/*N*/         return;
/*N*/     }
/*N*/ 
/*N*/     // we need some special handling here in case we're moved from an invisible layer
/*N*/     // to a visible one, or vice versa
/*N*/     // (relative to a layer. Remember that the visibility of a layer is a view attribute
/*N*/     // - the same layer can be visible in one view, and invisible in another view, at the
/*N*/     // same time)
/*N*/     // 2003-06-03 - #110592# - fs@openoffice.org
/*N*/ 
/*N*/     // collect all views in which our old layer is visible
/*N*/     ::std::set< SdrView* > aPreviouslyVisible;
/*N*/ 
/*N*/     {
/*N*/         SdrViewIter aIter( this );
/*N*/         for ( SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView() )
/*N*/             aPreviouslyVisible.insert( pView );
/*N*/     }
/*N*/ 
/*N*/     SdrRectObj::NbcSetLayer( _nLayer );
/*N*/ 
/*N*/     // collect all views in which our new layer is visible
/*N*/     ::std::set< SdrView* > aNewlyVisible;
/*N*/ 
/*N*/     {
/*N*/         SdrViewIter aIter( this );
/*N*/ 	    for ( SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView() )
/*N*/         {
/*N*/             ::std::set< SdrView* >::const_iterator aPrevPos = aPreviouslyVisible.find( pView );
/*N*/             if ( aPreviouslyVisible.end() != aPrevPos )
/*N*/             {   // in pView, we were visible _before_ the layer change, and are
/*N*/                 // visible _after_ the layer change, too
/*N*/                 // -> we're not interested in this view at all
/*N*/                 aPreviouslyVisible.erase( aPrevPos );
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 // in pView, we were visible _before_ the layer change, and are
/*N*/                 // _not_ visible after the layer change
/*N*/                 // => remember this view, as our visibility there changed
/*N*/                 aNewlyVisible.insert( pView );
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     // now aPreviouslyVisible contains all views where we became invisible
/*N*/     ::std::set< SdrView* >::const_iterator aLoopViews;
/*N*/     for (   aLoopViews = aPreviouslyVisible.begin();
/*N*/             aLoopViews != aPreviouslyVisible.end();
/*N*/             ++aLoopViews
/*N*/         )
/*N*/     {
/*N*/         lcl_ensureControlVisibility( *aLoopViews, this, false );
/*N*/     }
/*N*/ 
/*N*/     // and aNewlyVisible all views where we became visible
/*N*/     for (   aLoopViews = aNewlyVisible.begin();
/*N*/             aLoopViews != aNewlyVisible.end();
/*N*/             ++aLoopViews
/*N*/         )
/*N*/     {
/*N*/         lcl_ensureControlVisibility( *aLoopViews, this, true );
/*N*/     }
/*N*/ }



/*N*/ void SdrUnoObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrRectObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrUnoObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	if (bOwnUnoControlModel)					// nur als besitzt des Models dieses auch schreiben
/*N*/ 	{
/*?*/ 		// UNICODE: rOut << aUnoControlModelTypeName;
/*?*/ 		rOut.WriteByteString(aUnoControlModelTypeName);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrUnoObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError() != 0)
/*N*/ 		return;
/*N*/ 
/*N*/ 	SdrRectObj::ReadData(rHead,rIn);
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rIn, STREAM_READ);	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrUnoObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	if (bOwnUnoControlModel)					// nur als besitzt des Models dieses auch lesen
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ }

/*N*/ void SdrUnoObj::SetUnoControlModel( uno::Reference< awt::XControlModel > xModel)
/*N*/ {
/*N*/ 	if (xUnoControlModel.is())
/*N*/ 	{
/*?*/ 		uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
/*?*/ 		if (xComp.is())
/*?*/ 			pEventListener->StopListening(xComp);
/*?*/ 
/*?*/ 		if (pModel)
/*?*/ 		{
/*?*/ 			SdrHint aHint(*this);
/*?*/ 			aHint.SetKind(HINT_CONTROLREMOVED);
/*?*/ 			pModel->Broadcast(aHint);
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	xUnoControlModel = xModel;
/*N*/ 
/*N*/ 	// control model muss servicename des controls enthalten
/*N*/ 	if (xUnoControlModel.is())
/*N*/ 	{
/*N*/ 		uno::Reference< beans::XPropertySet > xSet(xUnoControlModel, uno::UNO_QUERY);
/*N*/ 		if (xSet.is())
/*N*/ 		{
/*N*/ 			uno::Any aValue( xSet->getPropertyValue(String("DefaultControl", gsl_getSystemTextEncoding())) );
/*N*/ 			OUString aStr;
/*N*/ 			if( aValue >>= aStr )
/*N*/ 				aUnoControlTypeName = String(aStr);
/*N*/ 		}
/*N*/ 
/*N*/ 		uno::Reference< lang::XComponent > xComp(xUnoControlModel, uno::UNO_QUERY);
/*N*/ 		if (xComp.is())
/*N*/ 			pEventListener->StartListening(xComp);
/*N*/ 
/*N*/ 		if (pModel)
/*N*/ 		{
/*N*/ 			SdrHint aHint(*this);
/*N*/ 			aHint.SetKind(HINT_CONTROLINSERTED);
/*N*/ 			pModel->Broadcast(aHint);
/*N*/ 		}
/*N*/ 	}
/*N*/ }
}
