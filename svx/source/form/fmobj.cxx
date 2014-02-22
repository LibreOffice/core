/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "fmobj.hxx"
#include "fmprop.hrc"
#include "fmvwimp.hxx"
#include "fmpgeimp.hxx"
#include "svx/fmresids.hrc"
#include "svx/fmview.hxx"
#include "svx/fmglob.hxx"
#include "svx/fmpage.hxx"
#include "editeng/editeng.hxx"
#include "svx/svdovirt.hxx"
#include "svx/fmmodel.hxx"
#include "svx/dialmgr.hxx"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/form/Forms.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include "svx/fmtools.hxx"

#include <tools/shl.hxx>
#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <vcl/svapp.hxx>
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

TYPEINIT1(FmFormObj, SdrUnoObj);
DBG_NAME(FmFormObj);

FmFormObj::FmFormObj(const OUString& rModelName)
          :SdrUnoObj                ( rModelName    )
          ,m_nPos                   ( -1            )
          ,m_pLastKnownRefDevice    ( NULL          )
{
    DBG_CTOR(FmFormObj, NULL);

    
    
    impl_checkRefDevice_nothrow( true );
}


FmFormObj::FmFormObj()
          :SdrUnoObj                ( ""  )
          ,m_nPos                   ( -1        )
          ,m_pLastKnownRefDevice    ( NULL      )
{
    DBG_CTOR(FmFormObj, NULL);
}


FmFormObj::~FmFormObj()
{
    DBG_DTOR(FmFormObj, NULL);

    if (m_xEnvironmentHistory.is())
        m_xEnvironmentHistory->dispose();

    m_xEnvironmentHistory = NULL;
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
    const FmFormModel* pFormModel = PTR_CAST( FmFormModel, GetModel() );
    if ( !pFormModel || !pFormModel->ControlsUseRefDevice() )
        return;

    OutputDevice* pCurrentRefDevice = pFormModel ? pFormModel->GetRefDevice() : NULL;
    if ( ( m_pLastKnownRefDevice == pCurrentRefDevice ) && !_force )
        return;

    Reference< XControlModel > xControlModel( GetUnoControlModel() );
    if ( !xControlModel.is() )
        return;

    m_pLastKnownRefDevice = pCurrentRefDevice;
    if ( m_pLastKnownRefDevice == NULL )
        return;

    try
    {
        Reference< XPropertySet > xModelProps( GetUnoControlModel(), UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xPropertyInfo( xModelProps->getPropertySetInfo(), UNO_SET_THROW );

        static const OUString sRefDevicePropName( "ReferenceDevice" );
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
        DBG_UNHANDLED_EXCEPTION();
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
        DBG_UNHANDLED_EXCEPTION();
    }
}


void FmFormObj::SetPage(SdrPage* _pNewPage)
{
    if ( GetPage() == _pNewPage )
    {
        SdrUnoObj::SetPage(_pNewPage);
        return;
    }

    FmFormPage* pOldFormPage = PTR_CAST( FmFormPage, GetPage() );
    if ( pOldFormPage )
        pOldFormPage->GetImpl().formObjectRemoved( *this );

    FmFormPage* pNewFormPage = PTR_CAST( FmFormPage, _pNewPage );
    if ( !pNewFormPage )
    {   
        
        
        
        impl_isolateControlModel_nothrow();
        SdrUnoObj::SetPage(_pNewPage);
        return;
    }

    Reference< css::form::XForms >      xNewPageForms = pNewFormPage->GetForms( true );
    Reference< XIndexContainer >        xNewParent;
    Sequence< ScriptEventDescriptor>    aNewEvents;

    
    
    if ( m_xEnvironmentHistory.is() )
    {
        
        
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

            
            
            aNewEvents = m_aEventsHistory;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    if ( !xNewParent.is() )
    {
        
        Reference< XIndexContainer > xOldForms;
        if ( pOldFormPage )
            xOldForms.set( pOldFormPage->GetForms(), UNO_QUERY_THROW );

        if ( xOldForms.is() )
        {
            
            Reference< XChild > xSearch( GetUnoControlModel(), UNO_QUERY );
            while (xSearch.is())
            {
                if ( xSearch == xOldForms )
                    break;
                xSearch = Reference< XChild >( xSearch->getParent(), UNO_QUERY );
            }
            if ( xSearch.is() ) 
            {
                Reference< XChild >  xMeAsChild( GetUnoControlModel(), UNO_QUERY );
                xNewParent.set( ensureModelEnv( xMeAsChild->getParent(), xNewPageForms ), UNO_QUERY );

                if ( xNewParent.is() )
                {
                    try
                    {
                        
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
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }
    }

    
    SdrUnoObj::SetPage(_pNewPage);

    
    if (xNewParent.is())
    {
        Reference< XFormComponent >  xMeAsFormComp(GetUnoControlModel(), UNO_QUERY);
        if (xMeAsFormComp.is())
        {
            
            Reference< XIndexContainer >  xOldParent(xMeAsFormComp->getParent(), UNO_QUERY);
            if (xOldParent.is())
            {
                sal_Int32 nPos = getElementPos(xOldParent, xMeAsFormComp);
                if (nPos > -1)
                    xOldParent->removeByIndex(nPos);
            }
            
            xNewParent->insertByIndex(xNewParent->getCount(), makeAny(xMeAsFormComp));

            
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
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

            }
        }
    }

    
    if (m_xEnvironmentHistory.is())
        m_xEnvironmentHistory->dispose();

    m_xEnvironmentHistory = NULL;
    m_aEventsHistory.realloc(0);

    if ( pNewFormPage )
        pNewFormPage->GetImpl().formObjectInserted( *this );
}


sal_uInt32 FmFormObj::GetObjInventor()   const
{
    return FmFormInventor;
}


sal_uInt16 FmFormObj::GetObjIdentifier() const
{
    return OBJ_UNO;
}


void FmFormObj::clonedFrom(const FmFormObj* _pSource)
{
    DBG_ASSERT(_pSource != NULL, "FmFormObj::clonedFrom : invalid source !");
    if (m_xEnvironmentHistory.is())
       m_xEnvironmentHistory->dispose();

    m_xEnvironmentHistory = NULL;
    m_aEventsHistory.realloc(0);

    Reference< XChild >  xSourceAsChild(_pSource->GetUnoControlModel(), UNO_QUERY);
    if (!xSourceAsChild.is())
        return;

    Reference< XInterface >  xSourceContainer = xSourceAsChild->getParent();

    m_xEnvironmentHistory = css::form::Forms::create( comphelper::getProcessComponentContext() );

    ensureModelEnv(xSourceContainer, m_xEnvironmentHistory);
    m_aEventsHistory = aEvts;
        
}


FmFormObj* FmFormObj::Clone() const
{
    FmFormObj* pFormObject = CloneHelper< FmFormObj >();
    DBG_ASSERT(pFormObject != NULL, "FmFormObj::Clone : invalid clone !");
    if (pFormObject)
        pFormObject->clonedFrom(this);

    return pFormObject;
}


void FmFormObj::NbcReformatText()
{
    impl_checkRefDevice_nothrow( false );
    SdrUnoObj::NbcReformatText();
}


FmFormObj& FmFormObj::operator= (const FmFormObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrUnoObj::operator= (rObj);

    
    
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


namespace
{
    OUString lcl_getFormComponentAccessPath(const Reference< XInterface >& _xElement, Reference< XInterface >& _rTopLevelElement)
    {
        Reference< ::com::sun::star::form::XFormComponent> xChild(_xElement, UNO_QUERY);
        Reference< ::com::sun::star::container::XIndexAccess> xParent;
        if (xChild.is())
            xParent = Reference< ::com::sun::star::container::XIndexAccess>(xChild->getParent(), UNO_QUERY);

        
        OUString sReturn;
        OUString sCurrentIndex;
        while (xChild.is())
        {
            
            sal_Int32 nPos = getElementPos(xParent, xChild);

            
            sCurrentIndex = OUString::number(nPos);
            if (!sReturn.isEmpty())
            {
                sCurrentIndex += "\\";
                sCurrentIndex += sReturn;
            }

            sReturn = sCurrentIndex;

            
            if (::comphelper::query_interface((Reference< XInterface >)xParent,xChild))
                xParent = Reference< ::com::sun::star::container::XIndexAccess>(xChild->getParent(), UNO_QUERY);
        }

        _rTopLevelElement = xParent;
        return sReturn;
    }
}


Reference< XInterface >  FmFormObj::ensureModelEnv(const Reference< XInterface > & _rSourceContainer, const Reference<css::form::XForms>& _rTopLevelDestContainer)
{
    Reference< XInterface >  xTopLevelSouce;
    OUString sAccessPath = lcl_getFormComponentAccessPath(_rSourceContainer, xTopLevelSouce);
    if (!xTopLevelSouce.is())
        
        return Reference< XInterface > ();

    Reference< XIndexContainer >  xDestContainer(_rTopLevelDestContainer, UNO_QUERY_THROW);
    Reference< XIndexContainer >  xSourceContainer(xTopLevelSouce, UNO_QUERY);
    DBG_ASSERT(xSourceContainer.is(), "FmFormObj::ensureModelEnv : the top level source is invalid !");

    sal_Int32 nTokIndex = 0;
    do
    {
        OUString aToken = sAccessPath.getToken( 0, '\\', nTokIndex );
        sal_uInt16 nIndex = (sal_uInt16)aToken.toInt32();

        
        DBG_ASSERT(nIndex<xSourceContainer->getCount(), "FmFormObj::ensureModelEnv : invalid access path !");
        Reference< XPropertySet >  xSourceForm;
        xSourceContainer->getByIndex(nIndex) >>= xSourceForm;
        DBG_ASSERT(xSourceForm.is(), "FmFormObj::ensureModelEnv : invalid source form !");

        Any aSrcCursorSource, aSrcCursorSourceType, aSrcDataSource;
        DBG_ASSERT(::comphelper::hasProperty(FM_PROP_COMMAND, xSourceForm) && ::comphelper::hasProperty(FM_PROP_COMMANDTYPE, xSourceForm)
            && ::comphelper::hasProperty(FM_PROP_DATASOURCE, xSourceForm), "FmFormObj::ensureModelEnv : invalid access path or invalid form (missing props) !");
            
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


        
        Reference< XPropertySet >  xCurrentSourceForm, xCurrentDestForm;
        sal_Int16 nCurrentSourceIndex = 0, nCurrentDestIndex = 0;
        while (nCurrentSourceIndex <= nIndex)
        {
            sal_Bool bEqualDSS = sal_False;
            while (!bEqualDSS)  
            {
                xSourceContainer->getByIndex(nCurrentSourceIndex) >>= xCurrentSourceForm;
                DBG_ASSERT(xCurrentSourceForm.is(), "FmFormObj::ensureModelEnv : invalid form ancestor (2) !");
                bEqualDSS = sal_False;
                if (::comphelper::hasProperty(FM_PROP_DATASOURCE, xCurrentSourceForm))
                {   
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
                        OSL_FAIL("FmFormObj::ensureModelEnv : exception while getting a sibling's DSS !");
                    }

                }
                ++nCurrentSourceIndex;
            }

            DBG_ASSERT(bEqualDSS, "FmFormObj::ensureModelEnv : found no source form !");
            

            
            bEqualDSS = sal_False;
            while (!bEqualDSS && (nCurrentDestIndex < xDestContainer->getCount()))
            {
                xDestContainer->getByIndex(nCurrentDestIndex) >>= xCurrentDestForm;
                DBG_ASSERT(xCurrentDestForm.is(), "FmFormObj::ensureModelEnv : invalid destination form !");
                bEqualDSS = sal_False;
                if (::comphelper::hasProperty(FM_PROP_DATASOURCE, xCurrentDestForm))
                {   
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
                        OSL_FAIL("FmFormObj::ensureModelEnv : exception while getting a destination DSS !");
                    }

                }
                ++nCurrentDestIndex;
            }

            if (!bEqualDSS)
            {   
                
                try
                {
                    
                    xCurrentDestForm.set(
                        ::comphelper::getProcessServiceFactory()->createInstance("com.sun.star.form.component.DataForm"),
                        UNO_QUERY_THROW );
                    ::comphelper::copyProperties( xCurrentSourceForm, xCurrentDestForm );

                    DBG_ASSERT(nCurrentDestIndex == xDestContainer->getCount(), "FmFormObj::ensureModelEnv : something went wrong with the numbers !");
                    xDestContainer->insertByIndex(nCurrentDestIndex, makeAny(xCurrentDestForm));

                    ++nCurrentDestIndex;
                        
                }
                catch(Exception&)
                {
                    OSL_FAIL("FmFormObj::ensureModelEnv : something went seriously wrong while creating a new form !");
                    
                    return Reference< XInterface > ();
                }

            }
        }

        
        

        
        xDestContainer = Reference< XIndexContainer > (xCurrentDestForm, UNO_QUERY);
        xSourceContainer = Reference< XIndexContainer > (xSourceForm, UNO_QUERY);
        DBG_ASSERT(xDestContainer.is() && xSourceContainer.is(), "FmFormObj::ensureModelEnv : invalid container !");
    }
    while ( nTokIndex >= 0 );

    return xDestContainer;
}


void FmFormObj::SetModel( SdrModel* _pNewModel )
{
    SdrUnoObj::SetModel( _pNewModel );
    impl_checkRefDevice_nothrow();
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


void FmFormObj::SetUnoControlModel( const Reference< com::sun::star::awt::XControlModel >& _rxModel )
{
    SdrUnoObj::SetUnoControlModel( _rxModel );

    FmFormPage* pFormPage = PTR_CAST( FmFormPage, GetPage() );
    if ( pFormPage )
        pFormPage->GetImpl().formModelAssigned( *this );

    impl_checkRefDevice_nothrow( true );
}


bool FmFormObj::EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd )
{
    bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);
    if ( bResult && SDRCREATE_FORCEEND == eCmd && rStat.GetView() )
    {
        if ( pPage )
        {
            FmFormPage& rPage = dynamic_cast< FmFormPage& >( *pPage );

            try
            {
                Reference< XFormComponent >  xContent( xUnoControlModel, UNO_QUERY_THROW );
                Reference< XForm > xParentForm( xContent->getParent(), UNO_QUERY );

                Reference< XIndexContainer > xFormToInsertInto;

                if ( !xParentForm.is() )
                {   
                    xParentForm.set( rPage.GetImpl().findPlaceInFormComponentHierarchy( xContent ), UNO_SET_THROW );
                    xFormToInsertInto.set( xParentForm, UNO_QUERY_THROW );
                }

                rPage.GetImpl().setUniqueName( xContent, xParentForm );

                if ( xFormToInsertInto.is() )
                    xFormToInsertInto->insertByIndex( xFormToInsertInto->getCount(), makeAny( xContent ) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        FmFormView* pView( dynamic_cast< FmFormView* >( rStat.GetView() ) );
        FmXFormView* pViewImpl = pView ? pView->GetImpl() : NULL;
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
}




SdrLayerID FmFormObj::GetLayer() const
{
    
    
    
    
    
    
    
    return SdrUnoObj::GetLayer();
}

void FmFormObj::NbcSetLayer(SdrLayerID nLayer)
{
    
    
    return SdrUnoObj::NbcSetLayer(nLayer);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
