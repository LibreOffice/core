/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <fmobj.hxx>
#include <fmprop.hxx>
#include <fmvwimp.hxx>
#include <fmpgeimp.hxx>
#include <svx/fmview.hxx>
#include <svx/fmpage.hxx>
#include <svx/svdovirt.hxx>
#include <svx/fmmodel.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/Forms.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <svx/fmtools.hxx>

#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

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


FmFormObj::FmFormObj(
    SdrModel& rSdrModel,
    const OUString& rModelName)
:   SdrUnoObj(rSdrModel, rModelName)
    ,m_nPos(-1)
    ,m_pLastKnownRefDevice(nullptr)
{
    // normally, this is done in SetUnoControlModel, but if the call happened in the base class ctor,
    // then our incarnation of it was not called (since we were not constructed at this time).
    impl_checkRefDevice_nothrow( true );
}

FmFormObj::FmFormObj(SdrModel& rSdrModel)
:   SdrUnoObj(rSdrModel, "")
    ,m_nPos(-1)
    ,m_pLastKnownRefDevice(nullptr)
{
    // Stuff that old SetModel also did:
    impl_checkRefDevice_nothrow();
}

FmFormObj::~FmFormObj()
{

    if (m_xEnvironmentHistory.is())
        m_xEnvironmentHistory->dispose();

    m_xEnvironmentHistory = nullptr;
    m_aEventsHistory.realloc(0);
}


void FmFormObj::SetObjEnv(const Reference< XIndexContainer > & xForm, const sal_Int32 nIdx,
                          const Sequence< ScriptEventDescriptor >& rEvts)
{
    m_xParent = xForm;
    aEvts     = rEvts;
    m_nPos    = nIdx;
}


void FmFormObj::ClearObjEnv()
{
    m_xParent.clear();
    aEvts.realloc( 0 );
    m_nPos = -1;
}


void FmFormObj::impl_checkRefDevice_nothrow( bool _force )
{
    const FmFormModel* pFormModel = dynamic_cast<FmFormModel*>(&getSdrModelFromSdrObject());
    if ( !pFormModel || !pFormModel->ControlsUseRefDevice() )
        return;

    OutputDevice* pCurrentRefDevice = pFormModel->GetRefDevice();
    if ( ( m_pLastKnownRefDevice.get() == pCurrentRefDevice ) && !_force )
        return;

    Reference< XControlModel > xControlModel( GetUnoControlModel() );
    if ( !xControlModel.is() )
        return;

    m_pLastKnownRefDevice = pCurrentRefDevice;
    if ( !m_pLastKnownRefDevice )
        return;

    try
    {
        Reference< XPropertySet > xModelProps( GetUnoControlModel(), UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xPropertyInfo( xModelProps->getPropertySetInfo(), UNO_SET_THROW );

        static constexpr OUStringLiteral sRefDevicePropName = u"ReferenceDevice";
        if ( xPropertyInfo->hasPropertyByName( sRefDevicePropName ) )
        {
            VCLXDevice* pUnoRefDevice = new VCLXDevice;
            pUnoRefDevice->SetOutputDevice( m_pLastKnownRefDevice );
            Reference< XDevice > xRefDevice( pUnoRefDevice );
            xModelProps->setPropertyValue( sRefDevicePropName, makeAny( xRefDevice ) );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


void FmFormObj::impl_isolateControlModel_nothrow()
{
    try
    {
        Reference< XChild > xControlModel( GetUnoControlModel(), UNO_QUERY );
        if ( xControlModel.is() )
        {
            Reference< XIndexContainer> xParent( xControlModel->getParent(), UNO_QUERY );
            if ( xParent.is() )
            {
                sal_Int32 nPos = getElementPos( xParent.get(), xControlModel );
                xParent->removeByIndex( nPos );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


void FmFormObj::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    FmFormPage* pOldFormPage(dynamic_cast< FmFormPage* >(getSdrPageFromSdrObject()));
    if ( pOldFormPage )
        pOldFormPage->GetImpl().formObjectRemoved( *this );

    FmFormPage* pNewFormPage = dynamic_cast<FmFormPage*>( pNewPage  );
    if ( !pNewFormPage )
    {
        // Maybe it makes sense to create an environment history here : if somebody set's our page to NULL, and we have a valid page before,
        // me may want to remember our place within the old page. For this we could create a new m_xEnvironmentHistory to store it.
        // So the next SetPage with a valid new page would restore that environment within the new page.
        // But for the original Bug (#57300#) we don't need that, so I omit it here. Maybe this will be implemented later.
        impl_isolateControlModel_nothrow();
        SdrUnoObj::handlePageChange(pOldPage, pNewPage);
        return;
    }

    Reference< css::form::XForms >      xNewPageForms = pNewFormPage->GetForms();
    Reference< XIndexContainer >        xNewParent;
    Sequence< ScriptEventDescriptor>    aNewEvents;

    // calc the new parent for my model (within the new page's forms hierarchy)
    // do we have a history ? (from :Clone)
    if ( m_xEnvironmentHistory.is() )
    {
        // the element in m_xEnvironmentHistory which is equivalent to my new parent (which (perhaps) has to be created within pNewPage->GetForms)
        // is the right-most element in the tree.
        Reference< XIndexContainer > xRightMostLeaf( m_xEnvironmentHistory, UNO_QUERY_THROW );
        try
        {
            while ( xRightMostLeaf->getCount() )
            {
                xRightMostLeaf.set(
                    xRightMostLeaf->getByIndex( xRightMostLeaf->getCount() - 1 ),
                    UNO_QUERY_THROW
                );
            }

            xNewParent.set( ensureModelEnv( xRightMostLeaf, xNewPageForms ), UNO_QUERY_THROW );

            // we successfully cloned the environment in m_xEnvironmentHistory, so we can use m_aEventsHistory
            // (which describes the events of our model at the moment m_xEnvironmentHistory was created)
            aNewEvents = m_aEventsHistory;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }

    if ( !xNewParent.is() )
    {
        // are we a valid part of our current page forms ?
        Reference< XIndexContainer > xOldForms;
        if ( pOldFormPage )
            xOldForms.set( pOldFormPage->GetForms(), UNO_QUERY_THROW );

        if ( xOldForms.is() )
        {
            // search (upward from our model) for xOldForms
            Reference< XChild > xSearch( GetUnoControlModel(), UNO_QUERY );
            while (xSearch.is())
            {
                if ( xSearch == xOldForms )
                    break;
                xSearch.set( xSearch->getParent(), UNO_QUERY );
            }
            if ( xSearch.is() ) // implies xSearch == xOldForms, which means we're a valid part of our current page forms hierarchy
            {
                Reference< XChild >  xMeAsChild( GetUnoControlModel(), UNO_QUERY );
                xNewParent.set( ensureModelEnv( xMeAsChild->getParent(), xNewPageForms ), UNO_QUERY );

                if ( xNewParent.is() )
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
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION("svx");
                    }
                }
            }
        }
    }

    // now set the page
    SdrUnoObj::handlePageChange(pOldPage, pNewPage);

    // place my model within the new parent container
    if (xNewParent.is())
    {
        Reference< XFormComponent >  xMeAsFormComp(GetUnoControlModel(), UNO_QUERY);
        if (xMeAsFormComp.is())
        {
            // check if I have another parent (and remove me, if necessary)
            Reference< XIndexContainer >  xOldParent(xMeAsFormComp->getParent(), UNO_QUERY);
            if (xOldParent.is())
            {
                sal_Int32 nPos = getElementPos(xOldParent, xMeAsFormComp);
                if (nPos > -1)
                    xOldParent->removeByIndex(nPos);
            }

            // and insert into the new container
            xNewParent->insertByIndex(xNewParent->getCount(), makeAny(xMeAsFormComp));

            // transfer the events
            if (aNewEvents.hasElements())
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
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("svx");
                }

            }
        }
    }

    // delete my history
    if (m_xEnvironmentHistory.is())
        m_xEnvironmentHistory->dispose();

    m_xEnvironmentHistory = nullptr;
    m_aEventsHistory.realloc(0);

    pNewFormPage->GetImpl().formObjectInserted( *this );
}

SdrInventor FmFormObj::GetObjInventor()   const
{
    return SdrInventor::FmForm;
}

SdrObjKind FmFormObj::GetObjIdentifier() const
{
    return OBJ_UNO;
}

void FmFormObj::clonedFrom(const FmFormObj* _pSource)
{
    DBG_ASSERT(_pSource != nullptr, "FmFormObj::clonedFrom : invalid source !");
    if (m_xEnvironmentHistory.is())
       m_xEnvironmentHistory->dispose();

    m_xEnvironmentHistory = nullptr;
    m_aEventsHistory.realloc(0);

    Reference< XChild >  xSourceAsChild(_pSource->GetUnoControlModel(), UNO_QUERY);
    if (!xSourceAsChild.is())
        return;

    Reference< XInterface >  xSourceContainer = xSourceAsChild->getParent();

    m_xEnvironmentHistory = css::form::Forms::create( comphelper::getProcessComponentContext() );

    ensureModelEnv(xSourceContainer, m_xEnvironmentHistory);
    m_aEventsHistory = aEvts;
        // if we were clone there was a call to operator=, so aEvts are exactly the events we need here...
}


FmFormObj* FmFormObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    FmFormObj* pFormObject = CloneHelper< FmFormObj >(rTargetModel);
    DBG_ASSERT(pFormObject != nullptr, "FmFormObj::Clone : invalid clone !");
    if (pFormObject)
        pFormObject->clonedFrom(this);

    return pFormObject;
}


FmFormObj& FmFormObj::operator= (const FmFormObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrUnoObj::operator= (rObj);

    // If UnoControlModel is part of an event environment,
    // events may assigned to it.
    Reference< XFormComponent >  xContent(rObj.xUnoControlModel, UNO_QUERY);
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
        aEvts = rObj.aEvts;
    return *this;
}


void FmFormObj::NbcReformatText()
{
    impl_checkRefDevice_nothrow();
    SdrUnoObj::NbcReformatText();
}


namespace
{
    OUString lcl_getFormComponentAccessPath(const Reference< XInterface >& _xElement, Reference< XInterface >& _rTopLevelElement)
    {
        Reference< css::form::XFormComponent> xChild(_xElement, UNO_QUERY);
        Reference< css::container::XIndexAccess> xParent;
        if (xChild.is())
            xParent.set(xChild->getParent(), UNO_QUERY);

        // while the current content is a form
        OUString sReturn;
        while (xChild.is())
        {
            // get the content's relative pos within its parent container
            sal_Int32 nPos = getElementPos(xParent, xChild);

            // prepend this current relative pos
            OUString sCurrentIndex = OUString::number(nPos);
            if (!sReturn.isEmpty())
            {
                sCurrentIndex += "\\" + sReturn;
            }

            sReturn = sCurrentIndex;

            // travel up
            xChild.set(xParent, css::uno::UNO_QUERY);
            if (xChild.is())
                xParent.set(xChild->getParent(), UNO_QUERY);
        }

        _rTopLevelElement = xParent;
        return sReturn;
    }
}


Reference< XInterface >  FmFormObj::ensureModelEnv(const Reference< XInterface > & _rSourceContainer, const Reference<css::form::XForms>& _rTopLevelDestContainer)
{
    Reference< XInterface >  xTopLevelSource;
    OUString sAccessPath = lcl_getFormComponentAccessPath(_rSourceContainer, xTopLevelSource);
    if (!xTopLevelSource.is())
        // something went wrong, maybe _rSourceContainer isn't part of a valid forms hierarchy
        return Reference< XInterface > ();

    Reference< XIndexContainer >  xDestContainer(_rTopLevelDestContainer, UNO_QUERY_THROW);
    Reference< XIndexContainer >  xSourceContainer(xTopLevelSource, UNO_QUERY);
    DBG_ASSERT(xSourceContainer.is(), "FmFormObj::ensureModelEnv : the top level source is invalid !");

    sal_Int32 nTokIndex = 0;
    do
    {
        OUString aToken = sAccessPath.getToken( 0, '\\', nTokIndex );
        sal_uInt16 nIndex = static_cast<sal_uInt16>(aToken.toInt32());

        // get the DSS of the source form (we have to find an equivalent for)
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
            OSL_FAIL("FmFormObj::ensureModelEnv : could not retrieve a source DSS !");
        }


        // calc the number of (source) form siblings with the same DSS
        Reference< XPropertySet >  xCurrentSourceForm, xCurrentDestForm;
        sal_Int16 nCurrentSourceIndex = 0;
        sal_Int32 nCurrentDestIndex = 0;
        while (nCurrentSourceIndex <= nIndex)
        {
            bool bEqualDSS = false;
            while (!bEqualDSS)  // (we don't have to check nCurrentSourceIndex here : it's bound by nIndex)
            {
                xSourceContainer->getByIndex(nCurrentSourceIndex) >>= xCurrentSourceForm;
                DBG_ASSERT(xCurrentSourceForm.is(), "FmFormObj::ensureModelEnv : invalid form ancestor (2) !");
                bEqualDSS = false;
                if (::comphelper::hasProperty(FM_PROP_DATASOURCE, xCurrentSourceForm))
                {   // it is a form
                    try
                    {
                        if  (   xCurrentSourceForm->getPropertyValue(FM_PROP_COMMAND) == aSrcCursorSource
                            &&  xCurrentSourceForm->getPropertyValue(FM_PROP_COMMANDTYPE) == aSrcCursorSourceType
                            &&  xCurrentSourceForm->getPropertyValue(FM_PROP_DATASOURCE) == aSrcDataSource
                            )
                        {
                            bEqualDSS = true;
                        }
                    }
                    catch(Exception&)
                    {
                        TOOLS_WARN_EXCEPTION("svx.form",
                                             "exception while getting a sibling's DSS !");
                    }

                }
                ++nCurrentSourceIndex;
            }

            DBG_ASSERT(bEqualDSS, "FmFormObj::ensureModelEnv : found no source form !");
            // ??? at least the nIndex-th one should have been found ???

            // now search the next one with the given DSS (within the destination container)
            bEqualDSS = false;
            while (!bEqualDSS && (nCurrentDestIndex < xDestContainer->getCount()))
            {
                xDestContainer->getByIndex(nCurrentDestIndex) >>= xCurrentDestForm;
                DBG_ASSERT(xCurrentDestForm.is(), "FmFormObj::ensureModelEnv : invalid destination form !");
                bEqualDSS = false;
                if (::comphelper::hasProperty(FM_PROP_DATASOURCE, xCurrentDestForm))
                {   // it is a form
                    try
                    {
                        if  (   xCurrentDestForm->getPropertyValue(FM_PROP_COMMAND) == aSrcCursorSource
                            &&  xCurrentDestForm->getPropertyValue(FM_PROP_COMMANDTYPE) == aSrcCursorSourceType
                            &&  xCurrentDestForm->getPropertyValue(FM_PROP_DATASOURCE) == aSrcDataSource
                            )
                        {
                            bEqualDSS = true;
                        }
                    }
                    catch(Exception&)
                    {
                        TOOLS_WARN_EXCEPTION("svx.form",
                                             "exception while getting a destination DSS !");
                    }

                }
                ++nCurrentDestIndex;
            }

            if (!bEqualDSS)
            {   // There is at least one more source form with the given DSS than destination forms are.
                // correct this ...
                try
                {
                    // create and insert (into the destination) a copy of the form
                    xCurrentDestForm.set(
                        ::comphelper::getProcessServiceFactory()->createInstance("com.sun.star.form.component.DataForm"),
                        UNO_QUERY_THROW );
                    ::comphelper::copyProperties( xCurrentSourceForm, xCurrentDestForm );

                    DBG_ASSERT(nCurrentDestIndex == xDestContainer->getCount(), "FmFormObj::ensureModelEnv : something went wrong with the numbers !");
                    xDestContainer->insertByIndex(nCurrentDestIndex, makeAny(xCurrentDestForm));

                    ++nCurrentDestIndex;
                        // like nCurrentSourceIndex, nCurrentDestIndex now points 'behind' the form it actually means
                }
                catch(Exception&)
                {
                    OSL_FAIL("FmFormObj::ensureModelEnv : something went seriously wrong while creating a new form !");
                    // no more options anymore ...
                    return Reference< XInterface > ();
                }

            }
        }

        // now xCurrentDestForm is a form equivalent to xSourceForm (which means they have the same DSS and the same number
        // of left siblings with the same DSS, which counts for all their ancestors, too)

        // go down
        xDestContainer.set(xCurrentDestForm, UNO_QUERY);
        xSourceContainer.set(xSourceForm, UNO_QUERY);
        DBG_ASSERT(xDestContainer.is() && xSourceContainer.is(), "FmFormObj::ensureModelEnv : invalid container !");
    }
    while ( nTokIndex >= 0 );

    return Reference<XInterface>( xDestContainer, UNO_QUERY );
}

FmFormObj* FmFormObj::GetFormObject( SdrObject* _pSdrObject )
{
    FmFormObj* pFormObject = dynamic_cast< FmFormObj* >( _pSdrObject );
    if ( !pFormObject )
    {
        SdrVirtObj* pVirtualObject = dynamic_cast< SdrVirtObj* >( _pSdrObject );
        if ( pVirtualObject )
            pFormObject = dynamic_cast< FmFormObj* >( &pVirtualObject->ReferencedObj() );
    }
    return pFormObject;
}


const FmFormObj* FmFormObj::GetFormObject( const SdrObject* _pSdrObject )
{
    const FmFormObj* pFormObject = dynamic_cast< const FmFormObj* >( _pSdrObject );
    if ( !pFormObject )
    {
        const SdrVirtObj* pVirtualObject = dynamic_cast< const SdrVirtObj* >( _pSdrObject );
        if ( pVirtualObject )
            pFormObject = dynamic_cast< const FmFormObj* >( &pVirtualObject->GetReferencedObj() );
    }
    return pFormObject;
}


void FmFormObj::SetUnoControlModel( const Reference< css::awt::XControlModel >& _rxModel )
{
    SdrUnoObj::SetUnoControlModel( _rxModel );

    FmFormPage* pFormPage(dynamic_cast< FmFormPage* >(getSdrPageFromSdrObject()));
    if ( pFormPage )
        pFormPage->GetImpl().formModelAssigned( *this );

    impl_checkRefDevice_nothrow( true );
}


bool FmFormObj::EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd )
{
    bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);
    if ( bResult && SdrCreateCmd::ForceEnd == eCmd && rStat.GetView() )
    {
        FmFormPage* pFormPage(dynamic_cast< FmFormPage* >(getSdrPageFromSdrObject()));

        if (nullptr != pFormPage)
        {
            try
            {
                Reference< XFormComponent >  xContent( xUnoControlModel, UNO_QUERY_THROW );
                Reference< XForm > xParentForm( xContent->getParent(), UNO_QUERY );

                Reference< XIndexContainer > xFormToInsertInto;

                if ( !xParentForm.is() )
                {   // model is not yet part of a form component hierarchy
                    xParentForm.set( pFormPage->GetImpl().findPlaceInFormComponentHierarchy( xContent ), UNO_SET_THROW );
                    xFormToInsertInto.set( xParentForm, UNO_QUERY_THROW );
                }

                FmFormPageImpl::setUniqueName( xContent, xParentForm );

                if ( xFormToInsertInto.is() )
                    xFormToInsertInto->insertByIndex( xFormToInsertInto->getCount(), makeAny( xContent ) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }

        FmFormView* pView( dynamic_cast< FmFormView* >( rStat.GetView() ) );
        FmXFormView* pViewImpl = pView ? pView->GetImpl() : nullptr;
        OSL_ENSURE( pViewImpl, "FmFormObj::EndCreate: no view!?" );
        if ( pViewImpl )
            pViewImpl->onCreatedFormObject( *this );
    }
    return bResult;
}


void FmFormObj::BrkCreate( SdrDragStat& rStat )
{
    SdrUnoObj::BrkCreate( rStat );
    impl_isolateControlModel_nothrow();

    FmFormView* pView( dynamic_cast< FmFormView* >( rStat.GetView() ) );
    FmXFormView* pViewImpl = pView ? pView->GetImpl() : nullptr;
    OSL_ENSURE( pViewImpl, "FmFormObj::EndCreate: no view!?" );
    if ( pViewImpl )
        pViewImpl->breakCreateFormObject();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
