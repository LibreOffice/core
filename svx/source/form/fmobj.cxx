/*************************************************************************
 *
 *  $RCSfile: fmobj.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 15:47:04 $
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

#ifndef _MyEDITENG_HXX
#include "editeng.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
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
/** === end UNO includes === **/

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

#ifndef _TOOLKIT_AWT_VCLXDEVICE_HXX_
#include <toolkit/awt/vclxdevice.hxx>
#endif

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;
using namespace ::svxform;

TYPEINIT1(FmFormObj, SdrUnoObj);
DBG_NAME(FmFormObj);
//------------------------------------------------------------------
FmFormObj::FmFormObj(const ::rtl::OUString& rModelName,sal_Int32 _nType)
          :SdrUnoObj                ( rModelName, sal_False )
          ,m_pControlCreationView   ( 0                     )
          ,m_nControlCreationEvent  ( 0                     )
          ,m_nPos                   ( -1                    )
          ,m_nType                  ( _nType                )
          ,m_pLastKnownRefDevice    ( NULL                  )
{
    DBG_CTOR(FmFormObj, NULL);
}

//------------------------------------------------------------------
FmFormObj::FmFormObj( sal_Int32 _nType )
          :SdrUnoObj                ( String(), sal_False   )
          ,m_pControlCreationView   ( 0                     )
          ,m_nControlCreationEvent  ( 0                     )
          ,m_nPos                   ( -1                    )
          ,m_nType                  ( _nType                )
          ,m_pLastKnownRefDevice    ( NULL                  )
{
    DBG_CTOR(FmFormObj, NULL);
}

//------------------------------------------------------------------
FmFormObj::~FmFormObj()
{
    DBG_DTOR(FmFormObj, NULL);
    if (m_nControlCreationEvent)
        Application::RemoveUserEvent(m_nControlCreationEvent);

    Reference< XComponent> xHistory(m_xEnvironmentHistory, UNO_QUERY);
    if (xHistory.is())
        xHistory->dispose();

    m_xEnvironmentHistory = NULL;
    m_aEventsHistory.realloc(0);
}

//------------------------------------------------------------------
void FmFormObj::SetObjEnv(const Reference< XIndexContainer > & xForm, sal_Int32 nIdx,
                          const Sequence< ScriptEventDescriptor >& rEvts)
{
    m_xParent = xForm;
    aEvts     = rEvts;
    m_nPos    = nIdx;
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

    Reference< XIndexContainer >    xNewParent;
    Sequence< ScriptEventDescriptor>    aNewEvents;

    // calc the new parent for my model (within the new page's forms hierarchy)
    // do we have a history ? (from :Clone)
    if (m_xEnvironmentHistory.is())
    {
        // the element in *m_pEnvironmentHistory which is equivalent to my new parent (which (perhaps) has to be created within _pNewPage->GetForms)
        // is the right-most element in the tree.
        Reference< XIndexContainer >  xLoop = m_xEnvironmentHistory;
        do
        {
            if (xLoop->getCount() == 0)
                break;
            Reference< XIndexContainer >  xRightMostChild;
            xLoop->getByIndex(xLoop->getCount() - 1) >>= xRightMostChild;
            if (!xRightMostChild.is())
            {
                DBG_ERROR("FmFormObj::SetPage : invalid elements in environment history !");
                break;
            }
            xLoop = xRightMostChild;
        }
        while (sal_True);

        xNewParent = Reference< XIndexContainer > (ensureModelEnv(xLoop, Reference< XIndexContainer > (pNewFormPage->GetForms(), ::com::sun::star::uno::UNO_QUERY)), ::com::sun::star::uno::UNO_QUERY);
        if (xNewParent.is())
            // we successfully clone the environment in m_pEnvironmentHistory, so we can use m_aEventsHistory
            // (which describes the events of our model at the moment m_pEnvironmentHistory was created)
            aNewEvents = m_aEventsHistory;
    }

    if (!xNewParent.is())
    {
        // are we a valid part of our current page forms ?
        FmFormPage* pOldFormPage = PTR_CAST(FmFormPage, GetPage());
        Reference< XIndexContainer >  xOldForms = pOldFormPage ? Reference< XIndexContainer > (pOldFormPage->GetForms(), ::com::sun::star::uno::UNO_QUERY) : ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > ();
        if (xOldForms.is())
        {
            // search (upward from our model) for xOldForms
            Reference< XChild >  xSearch(GetUnoControlModel(), UNO_QUERY);
            while (xSearch.is())
            {
                if (xSearch == xOldForms)
                    break;
                xSearch = Reference< XChild > (xSearch->getParent(), UNO_QUERY);
            }
            if (xSearch.is())   // implies xSearch == xOldForms, which means we're a valid part of our current page forms hierarchy
            {
                Reference< XChild >  xMeAsChild(GetUnoControlModel(), UNO_QUERY);
                xNewParent = Reference< XIndexContainer > (ensureModelEnv(xMeAsChild->getParent(), Reference< XIndexContainer > (pNewFormPage->GetForms(), ::com::sun::star::uno::UNO_QUERY)), ::com::sun::star::uno::UNO_QUERY);

                if (xNewParent.is())
                {
                    try
                    {
                        // transfer the events from our (model's) parent to the new (model's) parent, too
                        Reference< XEventAttacherManager >  xEventManager(xMeAsChild->getParent(), UNO_QUERY);
                        Reference< XIndexAccess >  xManagerAsIndex(xEventManager, UNO_QUERY);
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
        Reference< XFormComponent >  xMeAsFormComp(GetUnoControlModel(), UNO_QUERY);
        if (xMeAsFormComp.is())
        {
            // check if I have another parent (and remove me, if neccessary)
            Reference< XIndexContainer >  xOldParent(xMeAsFormComp->getParent(), UNO_QUERY);
            if (xOldParent.is())
            {
                sal_Int32 nPos = getElementPos(Reference< XIndexAccess > (xOldParent, UNO_QUERY), xMeAsFormComp);
                if (nPos > -1)
                    xOldParent->removeByIndex(nPos);
            }
            // and insert into the new container
            xNewParent->insertByIndex(xNewParent->getCount(), makeAny(xMeAsFormComp));

            // transfer the events
            if (aNewEvents.getLength())
            {
                try
                {
                    Reference< XEventAttacherManager >  xEventManager(xNewParent, UNO_QUERY);
                    Reference< XIndexAccess >  xManagerAsIndex(xEventManager, UNO_QUERY);
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
    Reference< XComponent> xHistory(m_xEnvironmentHistory, UNO_QUERY);
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
    Reference< XComponent> xHistory(m_xEnvironmentHistory, UNO_QUERY);
    if (xHistory.is())
        xHistory->dispose();

    m_xEnvironmentHistory = NULL;
    m_aEventsHistory.realloc(0);

    Reference< XChild >  xSourceAsChild(_pSource->GetUnoControlModel(), UNO_QUERY);
    if (!xSourceAsChild.is())
        return;

    Reference< XInterface >  xSourceContainer = xSourceAsChild->getParent();

    m_xEnvironmentHistory = Reference< XIndexContainer >(
        ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.Forms")),
        UNO_QUERY);
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
    Reference< XChild >  xMeAsChild(GetUnoControlModel(), UNO_QUERY);
    if (!xMeAsChild.is())
        return pReturn;

    try
    {
        Reference< XInterface >  xMyParent = xMeAsChild->getParent();
        Reference< XInterface >  xClonesParent = ensureModelEnv(xMyParent, Reference< XIndexContainer > (pClonesPage->GetForms(), ::com::sun::star::uno::UNO_QUERY));
        Reference< XIndexContainer >  xNewParentContainer(xClonesParent, UNO_QUERY);
        Reference< XFormComponent >  xCloneAsFormComponent(PTR_CAST(FmFormObj, pReturn)->GetUnoControlModel(), UNO_QUERY);
        if (xNewParentContainer.is() && xCloneAsFormComponent.is())
        {
            sal_Int32 nPos = xNewParentContainer->getCount();
            xNewParentContainer->insertByIndex(nPos, makeAny(xCloneAsFormComponent));
            // transfer the events, too
            Reference< XEventAttacherManager >  xEventManager(xNewParentContainer, UNO_QUERY);
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
void FmFormObj::ReformatText()
{
    const FmFormModel* pModel = PTR_CAST( FmFormModel, GetModel() );
    OutputDevice* pCurrentRefDevice = pModel ? pModel->GetRefDevice() : NULL;

    if ( m_pLastKnownRefDevice != pCurrentRefDevice )
    {
        m_pLastKnownRefDevice = pCurrentRefDevice;

        try
        {
            Reference< XPropertySet > xModelProps( GetUnoControlModel(), UNO_QUERY );
            Reference< XPropertySetInfo > xPropertyInfo;
            if ( xModelProps.is() )
                xPropertyInfo = xModelProps->getPropertySetInfo();

            const ::rtl::OUString sRefDevicePropName( RTL_CONSTASCII_USTRINGPARAM( "ReferenceDevice" ) );
            if ( xPropertyInfo.is() && xPropertyInfo->hasPropertyByName( sRefDevicePropName ) )
            {
                VCLXDevice* pUnoRefDevice = new VCLXDevice;
                pUnoRefDevice->SetOutputDevice( m_pLastKnownRefDevice );
                Reference< XDevice > xRefDevice( pUnoRefDevice );
                xModelProps->setPropertyValue( sRefDevicePropName, makeAny( xRefDevice ) );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FmFormObj::ReformatText: caught an exception!" );
        }
    }

    SdrUnoObj::ReformatText();
}

//------------------------------------------------------------------
void FmFormObj::operator= (const SdrObject& rObj)
{
    SdrUnoObj::operator= (rObj);

    FmFormObj* pFormObj = PTR_CAST(FmFormObj, &rObj);
    if (pFormObj)
    {
        // liegt das UnoControlModel in einer Eventumgebung,
        // dann koennen noch Events zugeordnet sein
        Reference< XFormComponent >  xContent(pFormObj->xUnoControlModel, UNO_QUERY);
        if (xContent.is())
        {
            Reference< XEventAttacherManager >  xManager(xContent->getParent(), UNO_QUERY);
            Reference< XIndexAccess >  xManagerAsIndex(xManager, UNO_QUERY);
            if (xManagerAsIndex.is())
            {
                sal_Int32 nPos = getElementPos( xManagerAsIndex, xContent );
                if ( nPos >= 0 )
                    aEvts = xManager->getScriptEvents( nPos );
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
Reference< XInterface >  FmFormObj::ensureModelEnv(const Reference< XInterface > & _rSourceContainer, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  _rTopLevelDestContainer)
{
    Reference< XInterface >  xTopLevelSouce;
    String sAccessPath = getFormComponentAccessPath(_rSourceContainer, xTopLevelSouce);
    if (!xTopLevelSouce.is())
        // somthing went wrong, maybe _rSourceContainer isn't part of a valid forms hierarchy
        return Reference< XInterface > ();

    Reference< XIndexContainer >  xDestContainer(_rTopLevelDestContainer);
    Reference< XIndexContainer >  xSourceContainer(xTopLevelSouce, UNO_QUERY);
    DBG_ASSERT(xSourceContainer.is(), "FmFormObj::ensureModelEnv : the top level source is invalid !");

    for (xub_StrLen i=0; i<sAccessPath.GetTokenCount('\\'); ++i)
    {
        sal_uInt16 nIndex = (sal_uInt16)sAccessPath.GetToken(i, '\\').ToInt32();

        // get the DSS of the source form (we have to find an aquivalent for)
        DBG_ASSERT(nIndex<xSourceContainer->getCount(), "FmFormObj::ensureModelEnv : invalid access path !");
        Reference< XPropertySet >  xSourceForm;
        xSourceContainer->getByIndex(nIndex) >>= xSourceForm;
        DBG_ASSERT(xSourceForm.is(), "FmFormObj::ensureModelEnv : invalid source form !");

        Any aSrcCursorSource, aSrcCursorSourceType, aSrcDataSource;
        DBG_ASSERT(::comphelper::hasProperty(FM_PROP_COMMAND, xSourceForm) && ::comphelper::hasProperty(FM_PROP_COMMANDTYPE, xSourceForm)
            && ::comphelper::hasProperty(FM_PROP_DATASOURCE, xSourceForm), "FmFormObj::ensureModelEnv : invalid access path or invalid form (missing props) !");
            // the parent access path should refer to a row set
        try
        {
            aSrcCursorSource        = xSourceForm->getPropertyValue(FM_PROP_COMMAND);
            aSrcCursorSourceType    = xSourceForm->getPropertyValue(FM_PROP_COMMANDTYPE);
            aSrcDataSource          = xSourceForm->getPropertyValue(FM_PROP_DATASOURCE);
        }
        catch(Exception&)
        {
            DBG_ERROR("FmFormObj::ensureModelEnv : could not retrieve a source DSS !");
        }


        // calc the number of (source) form siblings with the same DSS
        Reference< XPropertySet >  xCurrentSourceForm, xCurrentDestForm;
        sal_Int16 nCurrentSourceIndex = 0, nCurrentDestIndex = 0;
        while (nCurrentSourceIndex <= nIndex)
        {
            sal_Bool bEqualDSS = sal_False;
            while (!bEqualDSS)  // (we don't have to check nCurrentSourceIndex here : it's bounded by nIndex)
            {
                xSourceContainer->getByIndex(nCurrentSourceIndex) >>= xCurrentSourceForm;
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
                    catch(Exception&)
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
                xDestContainer->getByIndex(nCurrentDestIndex) >>= xCurrentDestForm;
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
                    catch(Exception&)
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
                    Reference< XPersistObject >  xSourcePersist(xCurrentSourceForm, UNO_QUERY);
                    DBG_ASSERT(xSourcePersist.is(), "FmFormObj::ensureModelEnv : invalid form (no persist object) !");

                    // create and insert (into the destination) a clone of the form
                    xCurrentDestForm = Reference< XPropertySet > (cloneUsingProperties(xSourcePersist), UNO_QUERY);
                    DBG_ASSERT(xCurrentDestForm.is(), "FmFormObj::ensureModelEnv : invalid cloned form !");

                    DBG_ASSERT(nCurrentDestIndex == xDestContainer->getCount(), "FmFormObj::ensureModelEnv : something went wrong with the numbers !");
                    xDestContainer->insertByIndex(nCurrentDestIndex, makeAny(xCurrentDestForm));

                    ++nCurrentDestIndex;
                        // like nCurrentSourceIndex, nCurrentDestIndex now points 'behind' the form it actally means
                }
                catch(Exception&)
                {
                    DBG_ERROR("FmFormObj::ensureModelEnv : something went seriously wrong while creating a new form !");
                    // no more options anymore ...
                    return Reference< XInterface > ();
                }

            }
        }

        // now xCurrentDestForm is a form aequivalent to xSourceForm (which means they have the same DSS and the same number
        // of left siblings with the same DSS, which counts for all their ancestors, too)

        // go down
        xDestContainer = Reference< XIndexContainer > (xCurrentDestForm, UNO_QUERY);
        xSourceContainer = Reference< XIndexContainer > (xSourceForm, UNO_QUERY);
        DBG_ASSERT(xDestContainer.is() && xSourceContainer.is(), "FmFormObj::ensureModelEnv : invalid container !");
    }

    return Reference< XInterface > (xDestContainer, UNO_QUERY);
}

//------------------------------------------------------------------
FASTBOOL FmFormObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    sal_Bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);
    if (bResult && SDRCREATE_FORCEEND == eCmd && rStat.GetView())
    {
        // ist das Object teil einer Form?
        Reference< XFormComponent >  xContent(xUnoControlModel, UNO_QUERY);
        if (xContent.is() && pPage)
        {
            // Komponente gehoert noch keiner Form an
            if (!xContent->getParent().is())
            {
                Reference< XForm >  xTemp = ((FmFormPage*)pPage)->GetImpl()->SetDefaults(xContent);
                Reference< XIndexContainer >  xForm(xTemp, UNO_QUERY);

                // Position des Elements
                sal_Int32 nPos = xForm->getCount();
                xForm->insertByIndex(nPos, makeAny(xContent));
            }
        }

        if ( m_nControlCreationEvent )
            Application::RemoveUserEvent( m_nControlCreationEvent );

        m_pControlCreationView = static_cast< FmFormView* >( rStat.GetView() );
        m_nControlCreationEvent = Application::PostUserEvent( LINK( this, FmFormObj, OnCreate ) );
    }
    return bResult;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmFormObj, OnCreate, void*, EMPTYTAG)
{
    m_nControlCreationEvent = 0;
    if ( m_pControlCreationView )
        m_pControlCreationView->ObjectCreated( this );
    return 0;
}
// -----------------------------------------------------------------------------
sal_Int32 FmFormObj::getType() const
{
    return m_nType;
}
// -----------------------------------------------------------------------------



