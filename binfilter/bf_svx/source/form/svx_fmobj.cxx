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
#ifdef _MSC_VER
#pragma hdrstop
#endif



#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif


#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif



#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif



#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SVX_FMVIEW_HXX //autogen
#include <fmview.hxx>
#endif

#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _SVX_FMPGEIMP_HXX
#include "fmpgeimp.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::binfilter::svxform;//STRIP008 using namespace ::svxform;

/*N*/ TYPEINIT1(FmFormObj, SdrUnoObj);
/*N*/ DBG_NAME(FmFormObj)
//------------------------------------------------------------------

//------------------------------------------------------------------
/*N*/ FmFormObj::FmFormObj(sal_Int32 _nType)
/*N*/ 		  :SdrUnoObj(String(), sal_False)
/*N*/ 		  ,nEvent(0)
/*N*/ 		  ,pTempView(0)
/*N*/ 		  ,nPos(-1)
/*N*/ 		  ,m_nType(_nType)
/*N*/ {
/*N*/ 	DBG_CTOR(FmFormObj, NULL);
/*N*/ }

//------------------------------------------------------------------
/*N*/ FmFormObj::~FmFormObj()
/*N*/ {
/*N*/ 	DBG_DTOR(FmFormObj, NULL);
/*N*/ 	if (nEvent)
/*?*/ 		Application::RemoveUserEvent(nEvent);
/*N*/ 
/*N*/ 	::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xHistory(m_xEnvironmentHistory, ::com::sun::star::uno::UNO_QUERY);
/*N*/ 	if (xHistory.is())
/*?*/ 		xHistory->dispose();
/*N*/ 
/*N*/ 	m_xEnvironmentHistory = NULL;
/*N*/ 	m_aEventsHistory.realloc(0);
/*N*/ }

//------------------------------------------------------------------
/*N*/ void FmFormObj::SetPage(SdrPage* _pNewPage)
/*N*/ {
/*N*/ 	FmFormPage* pNewFormPage = PTR_CAST(FmFormPage, _pNewPage);
/*N*/ 	if (!pNewFormPage || (GetPage() == _pNewPage))
/*N*/ 	{	// Maybe it makes sense to create an environment history here : if somebody set's our page to NULL, and we have a valid page before,
/*N*/ 		// me may want to remember our place within the old page. For this we could create a new m_pEnvironmentHistory to store it.
/*N*/ 		// So the next SetPage with a valid new page would restore that environment within the new page.
/*N*/ 		// But for the original Bug (#57300#) we don't need that, so I omit it here. Maybe this will be implemented later.
/*N*/ 		SdrUnoObj::SetPage(_pNewPage);
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > 	xNewParent;
/*N*/ 	::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>	aNewEvents;
/*N*/ 
/*N*/ 	// calc the new parent for my model (within the new page's forms hierarchy)
/*N*/ 	// do we have a history ? (from :Clone)
/*N*/ 	if (m_xEnvironmentHistory.is())
/*N*/ 	{
/*N*/ 		// the element in *m_pEnvironmentHistory which is equivalent to my new parent (which (perhaps) has to be created within _pNewPage->GetForms)
/*?*/		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*N*/ 		// is the right-most element in the tree.
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!xNewParent.is())
/*N*/ 	{
/*N*/ 		// are we a valid part of our current page forms ?
/*N*/ 		FmFormPage* pOldFormPage = PTR_CAST(FmFormPage, GetPage());
/*N*/ 		::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xOldForms = pOldFormPage ? ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pOldFormPage->GetForms(), ::com::sun::star::uno::UNO_QUERY) : ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > ();
/*N*/ 		if (xOldForms.is())
/*N*/ 		{
/*N*/ 			// search (upward from our model) for xOldForms
/*?*/ 			::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xSearch(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
/*?*/ 			while (xSearch.is())
/*?*/ 			{
/*?*/ 				if (xSearch == xOldForms)
/*?*/ 					break;
/*?*/ 				xSearch = ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > (xSearch->getParent(), ::com::sun::star::uno::UNO_QUERY);
/*?*/ 			}
/*?*/ 			if (xSearch.is())	// implies xSearch == xOldForms, which means we're a valid part of our current page forms hierarchy
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xMeAsChild(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// now set the page
/*N*/ 	SdrUnoObj::SetPage(_pNewPage);
/*N*/ 
/*N*/ 	// place my model within the new parent container
/*N*/ 	if (xNewParent.is())
/*N*/ 	{
/*?*/ 		::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xMeAsFormComp(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
/*?*/ 		if (xMeAsFormComp.is())
/*?*/ 		{
/*?*/ 			// check if I have another parent (and remove me, if neccessary)
/*?*/ 			::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xOldParent(xMeAsFormComp->getParent(), ::com::sun::star::uno::UNO_QUERY);
/*?*/ 			if (xOldParent.is())
/*?*/ 			{
/*?*/ 				sal_Int32 nPos = getElementPos(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xOldParent, ::com::sun::star::uno::UNO_QUERY), xMeAsFormComp);
/*?*/ 				if (nPos > -1)
/*?*/ 					xOldParent->removeByIndex(nPos);
/*?*/ 			}
/*?*/ 			// and insert into the new container
/*?*/ 			xNewParent->insertByIndex(xNewParent->getCount(), ::com::sun::star::uno::makeAny(xMeAsFormComp));
/*?*/ 
/*?*/ 			// transfer the events
/*?*/ 			if (aNewEvents.getLength())
/*?*/ 			{
/*?*/ 				try
/*?*/ 				{
/*?*/ 					::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xEventManager(xNewParent, ::com::sun::star::uno::UNO_QUERY);
/*?*/ 					::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xManagerAsIndex(xEventManager, ::com::sun::star::uno::UNO_QUERY);
/*?*/ 					if (xManagerAsIndex.is())
/*?*/ 					{
/*?*/ 						sal_Int32 nPos = getElementPos(xManagerAsIndex, xMeAsFormComp);
/*?*/ 						DBG_ASSERT(nPos >= 0, "FmFormObj::SetPage : inserted but not present ?");
/*?*/ 						xEventManager->registerScriptEvents(nPos, aNewEvents);
/*?*/ 					}
/*?*/ 				}
/*?*/ 				catch(...)
/*?*/ 				{
/*?*/ 					DBG_ERROR("FmFormObj::SetPage : could not tranfer script events !");
/*?*/ 				}
/*?*/ 				
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// delete my history
/*N*/ 	::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xHistory(m_xEnvironmentHistory, ::com::sun::star::uno::UNO_QUERY);
/*N*/ 	if (xHistory.is())
/*?*/ 		xHistory->dispose();
/*N*/ 
/*N*/ 	m_xEnvironmentHistory = NULL;
/*N*/ 	m_aEventsHistory.realloc(0);
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_uInt32 FmFormObj::GetObjInventor()   const
/*N*/ {
/*N*/ 	if( GetModel() && ((FmFormModel*)GetModel())->IsStreamingOldVersion() )
/*?*/ 		return SdrInventor;
/*N*/ 	return FmFormInventor;
/*N*/ }

//------------------------------------------------------------------
/*N*/ sal_uInt16 FmFormObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	if( GetModel() && ((FmFormModel*)GetModel())->IsStreamingOldVersion() )
/*?*/ 		return OBJ_RECT;
/*N*/ 	return OBJ_FM_CONTROL;
/*N*/ }

//------------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------
/*N*/ void FmFormObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	FmFormModel* pModel = (FmFormModel*)GetModel();
/*N*/ 	if( pModel && pModel->IsStreamingOldVersion() )
/*N*/ 	{
/*?*/ 		SdrLayerID nOld = GetLayer();
/*?*/ 		((FmFormObj*)this)->NbcSetLayer( pModel->GetControlExportLayerId( *this ) );
/*?*/ 		SdrUnoObj::WriteData( rOut );
/*?*/ 		((FmFormObj*)this)->NbcSetLayer( nOld );
/*?*/ 		return;
/*N*/ 	}
/*N*/ 	SdrUnoObj::WriteData(rOut);
/*N*/ }

//------------------------------------------------------------------
/*N*/ void FmFormObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	SdrUnoObj::ReadData(rHead,rIn);
/*N*/ }

//------------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------



}
