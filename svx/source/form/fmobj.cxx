/*************************************************************************
 *
 *  $RCSfile: fmobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-20 14:18:56 $
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
#pragma hdrstop

#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
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
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _SVDIO_HXX //autogen
#include <svdio.hxx>
#endif

#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
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
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

TYPEINIT1(FmFormObj, SdrUnoObj);
DBG_NAME(FmFormObj);
//------------------------------------------------------------------
FmFormObj::FmFormObj(const ::rtl::OUString& rModelName)
          :SdrUnoObj(rModelName, sal_False)
          ,pTempView(0)
          ,nEvent(0)
          ,nPos(-1)
{
    DBG_CTOR(FmFormObj, NULL);
}

//------------------------------------------------------------------
FmFormObj::FmFormObj()
          :SdrUnoObj(String(), sal_False)
          ,nEvent(0)
          ,pTempView(0)
          ,nPos(-1)
{
    DBG_CTOR(FmFormObj, NULL);
}

//------------------------------------------------------------------
FmFormObj::~FmFormObj()
{
    DBG_DTOR(FmFormObj, NULL);
    if (nEvent)
        Application::RemoveUserEvent(nEvent);

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xHistory(m_xEnvironmentHistory, ::com::sun::star::uno::UNO_QUERY);
    if (xHistory.is())
        xHistory->dispose();

    m_xEnvironmentHistory = NULL;
    m_aEventsHistory.realloc(0);
}

//------------------------------------------------------------------
void FmFormObj::SetObjEnv(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > & xForm, sal_Int32 nIdx,
                          const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& rEvts)
{
    xParent = xForm;
    aEvts   = rEvts;
    nPos    = nIdx;
}

//------------------------------------------------------------------
void FmFormObj::SetPage(SdrPage* _pNewPage)
{
    FmFormPage* pNewFormPage = PTR_CAST(FmFormPage, _pNewPage);
    if (!pNewFormPage || (GetPage() == _pNewPage))
    {   // Maybe it makes sense to create an environment history here : if somebody set's our page to NULL, and we have a valid page before,
        // me may want to remember our place within the old page. For this we could create a new m_pEnvironmentHistory to store it.
        // So the next SetPage with a valid new page would restore that environment within the new page.
        // But for the original Bug (#57300#) we don't need that, so I omit it here. Maybe this will be implemented later.
        SdrUnoObj::SetPage(_pNewPage);
        return;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >    xNewParent;
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>   aNewEvents;

    // calc the new parent for my model (within the new page's forms hierarchy)
    // do we have a history ? (from :Clone)
    if (m_xEnvironmentHistory.is())
    {
        // the element in *m_pEnvironmentHistory which is equivalent to my new parent (which (perhaps) has to be created within _pNewPage->GetForms)
        // is the right-most element in the tree.
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xLoop = m_xEnvironmentHistory;
        do
        {
            if (xLoop->getCount() == 0)
                break;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xRightMostChild(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xLoop->getByIndex(xLoop->getCount() - 1).getValue(), ::com::sun::star::uno::UNO_QUERY);
            if (!xRightMostChild.is())
            {
                DBG_ERROR("FmFormObj::SetPage : invalid elements in environment history !");
                break;
            }
            xLoop = xRightMostChild;
        }
        while (sal_True);

        xNewParent = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (ensureModelEnv(xLoop, ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pNewFormPage->GetForms(), ::com::sun::star::uno::UNO_QUERY)), ::com::sun::star::uno::UNO_QUERY);
        if (xNewParent.is())
            // we successfully clone the environment in m_pEnvironmentHistory, so we can use m_aEventsHistory
            // (which describes the events of our model at the moment m_pEnvironmentHistory was created)
            aNewEvents = m_aEventsHistory;
    }

    if (!xNewParent.is())
    {
        // are we a valid part of our current page forms ?
        FmFormPage* pOldFormPage = PTR_CAST(FmFormPage, GetPage());
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xOldForms = pOldFormPage ? ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pOldFormPage->GetForms(), ::com::sun::star::uno::UNO_QUERY) : ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > ();
        if (xOldForms.is())
        {
            // search (upward from our model) for xOldForms
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xSearch(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
            while (xSearch.is())
            {
                if (xSearch == xOldForms)
                    break;
                xSearch = ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > (xSearch->getParent(), ::com::sun::star::uno::UNO_QUERY);
            }
            if (xSearch.is())   // implies xSearch == xOldForms, which means we're a valid part of our current page forms hierarchy
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xMeAsChild(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
                xNewParent = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (ensureModelEnv(xMeAsChild->getParent(), ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pNewFormPage->GetForms(), ::com::sun::star::uno::UNO_QUERY)), ::com::sun::star::uno::UNO_QUERY);

                if (xNewParent.is())
                {
                    try
                    {
                        // transfer the events from our (model's) parent to the new (model's) parent, too
                        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xEventManager(xMeAsChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xManagerAsIndex(xEventManager, ::com::sun::star::uno::UNO_QUERY);
                        if (xManagerAsIndex.is())
                        {
                            sal_Int32 nPos = getElementPos(xManagerAsIndex, xMeAsChild);
                            if (nPos >= 0)
                                aNewEvents = xEventManager->getScriptEvents(nPos);
                        }
                        else
                            aNewEvents = aEvts;
                    }
                    catch(...)
                    {
                        DBG_ERROR("FmFormObj::SetPage : could not retrieve script events !");
                    }

                }
            }
        }
    }

    // now set the page
    SdrUnoObj::SetPage(_pNewPage);

    // place my model within the new parent container
    if (xNewParent.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xMeAsFormComp(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xMeAsFormComp.is())
        {
            // check if I have another parent (and remove me, if neccessary)
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xOldParent(xMeAsFormComp->getParent(), ::com::sun::star::uno::UNO_QUERY);
            if (xOldParent.is())
            {
                sal_Int32 nPos = getElementPos(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xOldParent, ::com::sun::star::uno::UNO_QUERY), xMeAsFormComp);
                if (nPos > -1)
                    xOldParent->removeByIndex(nPos);
            }
            // and insert into the new container
            xNewParent->insertByIndex(xNewParent->getCount(), ::com::sun::star::uno::makeAny(xMeAsFormComp));

            // transfer the events
            if (aNewEvents.getLength())
            {
                try
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xEventManager(xNewParent, ::com::sun::star::uno::UNO_QUERY);
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xManagerAsIndex(xEventManager, ::com::sun::star::uno::UNO_QUERY);
                    if (xManagerAsIndex.is())
                    {
                        sal_Int32 nPos = getElementPos(xManagerAsIndex, xMeAsFormComp);
                        DBG_ASSERT(nPos >= 0, "FmFormObj::SetPage : inserted but not present ?");
                        xEventManager->registerScriptEvents(nPos, aNewEvents);
                    }
                }
                catch(...)
                {
                    DBG_ERROR("FmFormObj::SetPage : could not tranfer script events !");
                }

            }
        }
    }

    // delete my history
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xHistory(m_xEnvironmentHistory, ::com::sun::star::uno::UNO_QUERY);
    if (xHistory.is())
        xHistory->dispose();

    m_xEnvironmentHistory = NULL;
    m_aEventsHistory.realloc(0);
}

//------------------------------------------------------------------
sal_uInt32 FmFormObj::GetObjInventor()   const
{
    if( GetModel() && ((FmFormModel*)GetModel())->IsStreamingOldVersion() )
        return SdrInventor;
    return FmFormInventor;
}

//------------------------------------------------------------------
sal_uInt16 FmFormObj::GetObjIdentifier() const
{
    if( GetModel() && ((FmFormModel*)GetModel())->IsStreamingOldVersion() )
        return OBJ_RECT;
    return OBJ_FM_CONTROL;
}

//------------------------------------------------------------------
void FmFormObj::clonedFrom(const FmFormObj* _pSource)
{
    DBG_ASSERT(_pSource != NULL, "FmFormObj::clonedFrom : invalid source !");
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xHistory(m_xEnvironmentHistory, ::com::sun::star::uno::UNO_QUERY);
    if (xHistory.is())
        xHistory->dispose();

    m_xEnvironmentHistory = NULL;
    m_aEventsHistory.realloc(0);

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xSourceAsChild(_pSource->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
    if (!xSourceAsChild.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xSourceContainer = xSourceAsChild->getParent();

    m_xEnvironmentHistory = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >(
        ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.FormsCollection")),
        ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(m_xEnvironmentHistory.is(), "FmFormObj::clonedFrom : could not create a forms collection !");

    if (m_xEnvironmentHistory.is())
    {
        ensureModelEnv(xSourceContainer, m_xEnvironmentHistory);
        m_aEventsHistory = aEvts;
            // if we we're clone there was a call to operator=, so aEvts are excatly the events we need here ...
    }
}

//------------------------------------------------------------------
SdrObject* FmFormObj::Clone() const
{
    SdrObject* pReturn = SdrUnoObj::Clone();

    FmFormObj* pFormObject = PTR_CAST(FmFormObj, pReturn);
    DBG_ASSERT(pFormObject != NULL, "FmFormObj::Clone : invalid clone !");
    if (pFormObject)
        pFormObject->clonedFrom(this);

    return pReturn;
}

//------------------------------------------------------------------
SdrObject* FmFormObj::Clone(SdrPage* _pPage, SdrModel* _pModel) const
{
    SdrObject* pReturn = SdrUnoObj::Clone(_pPage, _pModel);
    if (!pReturn)
        return pReturn;

    FmFormObj* pCloneAsFormObj = PTR_CAST(FmFormObj, pReturn);
    if (!pCloneAsFormObj)
        return pReturn;

    FmFormPage* pClonesPage = PTR_CAST(FmFormPage, pReturn->GetPage());
    if (!pClonesPage || !pClonesPage->GetForms().is())
        return pReturn;

    // build an form environment equivalent to my own withín the destination page
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xMeAsChild(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
    if (!xMeAsChild.is())
        return pReturn;

    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xMyParent = xMeAsChild->getParent();
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xClonesParent = ensureModelEnv(xMyParent, ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (pClonesPage->GetForms(), ::com::sun::star::uno::UNO_QUERY));
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xNewParentContainer(xClonesParent, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xCloneAsFormComponent(PTR_CAST(FmFormObj, pReturn)->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xNewParentContainer.is() && xCloneAsFormComponent.is())
        {
            sal_Int32 nPos = xNewParentContainer->getCount();
            xNewParentContainer->insertByIndex(nPos, ::com::sun::star::uno::makeAny(xCloneAsFormComponent));
            // transfer the events, too
            ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xEventManager(xNewParentContainer, ::com::sun::star::uno::UNO_QUERY);
            if (xEventManager.is())
                xEventManager->registerScriptEvents(nPos, pCloneAsFormObj->GetEvents());
        }
    }
    catch(...)
    {
        DBG_ERROR("FmFormObj::Clone : error while placing the model within it's new env");
    }


    return pReturn;
}

//------------------------------------------------------------------
void FmFormObj::operator= (const SdrObject& rObj)
{
    SdrUnoObj::operator= (rObj);

    FmFormObj* pFormObj = PTR_CAST(FmFormObj, &rObj);
    if (pFormObj)
    {
        // liegt das ::com::sun::star::awt::UnoControlModel in einer Eventumgebung,
        // dann koennen noch Events zugeordnet sein
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xContent(pFormObj->xUnoControlModel, ::com::sun::star::uno::UNO_QUERY);
        if (xContent.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xManager(xContent->getParent(), ::com::sun::star::uno::UNO_QUERY);
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xManagerAsIndex(xManager, ::com::sun::star::uno::UNO_QUERY);
            if (xManagerAsIndex.is())
            {
                sal_Int32 nPos = getElementPos(xManagerAsIndex, xContent);
                if (nPos >= 0)
                    aEvts = xManager->getScriptEvents(nPos);
            }
        }
        else
            aEvts = pFormObj->aEvts;
    }
}

//------------------------------------------------------------------
void FmFormObj::WriteData(SvStream& rOut) const
{
    FmFormModel* pModel = (FmFormModel*)GetModel();
    if( pModel && pModel->IsStreamingOldVersion() )
    {
        SdrLayerID nOld = GetLayer();
        ((FmFormObj*)this)->NbcSetLayer( pModel->GetControlExportLayerId( *this ) );
        SdrUnoObj::WriteData( rOut );
        ((FmFormObj*)this)->NbcSetLayer( nOld );
        return;
    }
    SdrUnoObj::WriteData(rOut);
}

//------------------------------------------------------------------
void FmFormObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    SdrUnoObj::ReadData(rHead,rIn);
}

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  FmFormObj::ensureModelEnv(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & _rSourceContainer, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  _rTopLevelDestContainer)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xTopLevelSouce;
    String sAccessPath = getFormComponentAccessPath(_rSourceContainer, xTopLevelSouce);
    if (!xTopLevelSouce.is())
        // somthing went wrong, maybe _rSourceContainer isn't part of a valid forms hierarchy
        return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xDestContainer(_rTopLevelDestContainer);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xSourceContainer(xTopLevelSouce, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xSourceContainer.is(), "FmFormObj::ensureModelEnv : the top level source is invalid !");

    for (xub_StrLen i=0; i<sAccessPath.GetTokenCount('\\'); ++i)
    {
        sal_uInt16 nIndex = sAccessPath.GetToken(i, '\\').ToInt32();

        // get the DSS of the source form (we have to find an aquivalent for)
        DBG_ASSERT(nIndex<xSourceContainer->getCount(), "FmFormObj::ensureModelEnv : invalid access path !");
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSourceForm = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xSourceContainer->getByIndex(nIndex).getValue(), ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xSourceForm.is(), "FmFormObj::ensureModelEnv : invalid source form !");

        ::com::sun::star::uno::Any aSrcCursorSource, aSrcCursorSourceType, aSrcDataSource;
        DBG_ASSERT(::comphelper::hasProperty(FM_PROP_COMMAND, xSourceForm) && ::comphelper::hasProperty(FM_PROP_COMMANDTYPE, xSourceForm)
            && ::comphelper::hasProperty(FM_PROP_DATASOURCE, xSourceForm), "FmFormObj::ensureModelEnv : invalid access path or invalid form (missing props) !");
            // the parent access path should refer to a row set
        try
        {
            aSrcCursorSource = xSourceForm->getPropertyValue(FM_PROP_COMMAND);
            aSrcCursorSourceType = xSourceForm->getPropertyValue(FM_PROP_COMMANDTYPE);
            aSrcDataSource = xSourceForm->getPropertyValue(FM_PROP_DATASOURCE);
        }
        catch(...)
        {
            DBG_ERROR("FmFormObj::ensureModelEnv : could not retrieve a source DSS !");
        }


        // calc the number of (source) form siblings with the same DSS
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xCurrentSourceForm, xCurrentDestForm;
        sal_Int16 nCurrentSourceIndex = 0, nCurrentDestIndex = 0;
        while (nCurrentSourceIndex <= nIndex)
        {
            sal_Bool bEqualDSS = sal_False;
            while (!bEqualDSS)  // (we don't have to check nCurrentSourceIndex here : it's bounded by nIndex)
            {
                xCurrentSourceForm = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xSourceContainer->getByIndex(nCurrentSourceIndex).getValue(), ::com::sun::star::uno::UNO_QUERY);
                DBG_ASSERT(xCurrentSourceForm.is(), "FmFormObj::ensureModelEnv : invalid form ancestor (2) !");
                bEqualDSS = sal_False;
                if (::comphelper::hasProperty(FM_PROP_DATASOURCE, xCurrentSourceForm))
                {   // it is a form
                    try
                    {
                        if  (   ::comphelper::compare(xCurrentSourceForm->getPropertyValue(FM_PROP_COMMAND), aSrcCursorSource)
                            &&  ::comphelper::compare(xCurrentSourceForm->getPropertyValue(FM_PROP_COMMANDTYPE), aSrcCursorSourceType)
                            &&  ::comphelper::compare(xCurrentSourceForm->getPropertyValue(FM_PROP_DATASOURCE), aSrcDataSource)
                            )
                        {
                            bEqualDSS = sal_True;
                        }
                    }
                    catch(...)
                    {
                        DBG_ERROR("FmFormObj::ensureModelEnv : exception while getting a sibling's DSS !");
                    }

                }
                ++nCurrentSourceIndex;
            }

            DBG_ASSERT(bEqualDSS, "FmFormObj::ensureModelEnv : found no source form !");
            // ??? at least the nIndex-th one should have been found ???

            // now search the next one with the given DSS (within the destination container)
            bEqualDSS = sal_False;
            while (!bEqualDSS && (nCurrentDestIndex < xDestContainer->getCount()))
            {
                xCurrentDestForm = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)xDestContainer->getByIndex(nCurrentDestIndex).getValue(), ::com::sun::star::uno::UNO_QUERY);
                DBG_ASSERT(xCurrentDestForm.is(), "FmFormObj::ensureModelEnv : invalid destination form !");
                bEqualDSS = sal_False;
                if (::comphelper::hasProperty(FM_PROP_DATASOURCE, xCurrentDestForm))
                {   // it is a form
                    try
                    {
                        if  (   ::comphelper::compare(xCurrentDestForm->getPropertyValue(FM_PROP_COMMAND), aSrcCursorSource)
                            &&  ::comphelper::compare(xCurrentDestForm->getPropertyValue(FM_PROP_COMMANDTYPE), aSrcCursorSourceType)
                            &&  ::comphelper::compare(xCurrentDestForm->getPropertyValue(FM_PROP_DATASOURCE), aSrcDataSource)
                            )
                        {
                            bEqualDSS = sal_True;
                        }
                    }
                    catch(...)
                    {
                        DBG_ERROR("FmFormObj::ensureModelEnv : exception while getting a destination DSS !");
                    }

                }
                ++nCurrentDestIndex;
            }

            if (!bEqualDSS)
            {   // There is at least one more source form with the given DSS than destination forms are.
                // correct this ...
                try
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xSourcePersist(xCurrentSourceForm, ::com::sun::star::uno::UNO_QUERY);
                    DBG_ASSERT(xSourcePersist.is(), "FmFormObj::ensureModelEnv : invalid form (no persist object) !");

                    // create and insert (into the destination) a clone of the form
                    xCurrentDestForm = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (cloneUsingProperties(xSourcePersist), ::com::sun::star::uno::UNO_QUERY);
                    DBG_ASSERT(xCurrentDestForm.is(), "FmFormObj::ensureModelEnv : invalid cloned form !");

                    DBG_ASSERT(nCurrentDestIndex == xDestContainer->getCount(), "FmFormObj::ensureModelEnv : something went wrong with the numbers !");
                    xDestContainer->insertByIndex(nCurrentDestIndex, ::com::sun::star::uno::makeAny(xCurrentDestForm));

                    ++nCurrentDestIndex;
                        // like nCurrentSourceIndex, nCurrentDestIndex now points 'behind' the form it actally means
                }
                catch(...)
                {
                    DBG_ERROR("FmFormObj::ensureModelEnv : something went seriously wrong while creating a new form !");
                    // no more options anymore ...
                    return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ();
                }

            }
        }

        // now xCurrentDestForm is a form aequivalent to xSourceForm (which means they have the same DSS and the same number
        // of left siblings with the same DSS, which counts for all their ancestors, too)

        // go down
        xDestContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (xCurrentDestForm, ::com::sun::star::uno::UNO_QUERY);
        xSourceContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (xSourceForm, ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xDestContainer.is() && xSourceContainer.is(), "FmFormObj::ensureModelEnv : invalid container !");
    }

    return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > (xDestContainer, ::com::sun::star::uno::UNO_QUERY);
}

//------------------------------------------------------------------
FASTBOOL FmFormObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    sal_Bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);
    if (bResult && SDRCREATE_FORCEEND == eCmd && rStat.GetView())
    {
        // ist das Object teil einer ::com::sun::star::form::Form?
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xContent(xUnoControlModel, ::com::sun::star::uno::UNO_QUERY);
        if (xContent.is() && pPage)
        {
            // Komponente gehoert noch keiner ::com::sun::star::form::Form an
            if (!xContent->getParent().is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xTemp = ((FmFormPage*)pPage)->GetImpl()->SetDefaults(xContent);
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xForm(xTemp, ::com::sun::star::uno::UNO_QUERY);

                // Position des Elements
                sal_Int32 nPos = xForm->getCount();
                xForm->insertByIndex(nPos, ::com::sun::star::uno::makeAny(xContent));
            }
        }

        if (nEvent)
            Application::RemoveUserEvent(nEvent);

        pTempView = (FmFormView*)rStat.GetView();
        nEvent = Application::PostUserEvent(LINK(this,FmFormObj,OnCreate));
    }
    return bResult;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmFormObj, OnCreate, void*, EMPTYTAG)
{
    nEvent = 0;
    if (pTempView)
        pTempView->ObjectCreated(this);
    return 0;
}



