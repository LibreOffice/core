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

#include <sal/config.h>

#include <map>

#include <sal/macros.h>
#include "fmundo.hxx"
#include "fmpgeimp.hxx"
#include "svx/dbtoolsclient.hxx"
#include "svx/svditer.hxx"
#include "fmobj.hxx"
#include "fmprop.hrc"
#include "svx/fmresids.hrc"
#include "svx/fmglob.hxx"
#include "svx/dialmgr.hxx"
#include "svx/fmmodel.hxx"
#include "svx/fmpage.hxx"

#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>

#include "svx/fmtools.hxx"
#include <svl/macitem.hxx>
#include <tools/shl.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/event.hxx>
#include <osl/mutex.hxx>
#include <comphelper/property.hxx>
#include <comphelper/uno3.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::form::binding;
using namespace ::svxform;


#include <com/sun/star/script/XScriptListener.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>

typedef cppu::WeakImplHelper1< XScriptListener > ScriptEventListener_BASE;
class ScriptEventListenerWrapper : public ScriptEventListener_BASE
{
public:
    ScriptEventListenerWrapper( FmFormModel& _rModel) throw ( RuntimeException )
        :m_rModel( _rModel )
        ,m_attemptedListenerCreation( false )
    {

    }
    
    virtual void SAL_CALL disposing(const EventObject& ) throw( RuntimeException ){}

    
    virtual void SAL_CALL firing(const  ScriptEvent& evt) throw(RuntimeException)
    {
        attemptListenerCreation();
        if ( m_vbaListener.is() )
        {
            m_vbaListener->firing( evt );
        }
    }

    virtual Any SAL_CALL approveFiring(const ScriptEvent& evt) throw( com::sun::star::reflection::InvocationTargetException, RuntimeException)
    {
        attemptListenerCreation();
        if ( m_vbaListener.is() )
        {
            return m_vbaListener->approveFiring( evt );
        }
        return Any();
    }

private:
    void attemptListenerCreation()
    {
        if ( m_attemptedListenerCreation )
            return;
        m_attemptedListenerCreation = true;

        try
        {
            css::uno::Reference<css::uno::XComponentContext> context(
                comphelper::getProcessComponentContext());
            Reference< XScriptListener > const xScriptListener(
                context->getServiceManager()->createInstanceWithContext(
                    "ooo.vba.EventListener", context),
                UNO_QUERY_THROW);
            Reference< XPropertySet > const xListenerProps( xScriptListener, UNO_QUERY_THROW );
            
            SfxObjectShellRef const xObjectShell = m_rModel.GetObjectShell();
            ENSURE_OR_THROW( xObjectShell.Is(), "no object shell!" );
            xListenerProps->setPropertyValue("Model", makeAny( xObjectShell->GetModel() ) );

            m_vbaListener = xScriptListener;
        }
        catch( Exception const & )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    FmFormModel&                    m_rModel;
    Reference< XScriptListener >    m_vbaListener;
    bool                            m_attemptedListenerCreation;


};




struct PropertyInfo
{
    sal_Bool    bIsTransientOrReadOnly  : 1;    
    sal_Bool    bIsValueProperty        : 1;    
                                            
};

struct PropertySetInfo
{
    typedef std::map<OUString, PropertyInfo> AllProperties;

    AllProperties   aProps;                 
    sal_Bool            bHasEmptyControlSource; 
                                            
};

sal_Bool operator < (const Reference< XPropertySet >& lhs,
                 const Reference< XPropertySet >& rhs)
{
    return lhs.get() < rhs.get();
}

typedef std::map<Reference< XPropertySet >, PropertySetInfo> PropertySetInfoCache;



OUString static_STR_UNDO_PROPERTY;

DBG_NAME(FmXUndoEnvironment)

FmXUndoEnvironment::FmXUndoEnvironment(FmFormModel& _rModel)
                   :rModel( _rModel )
                   ,m_pPropertySetCache( NULL )
                   ,m_pScriptingEnv( ::svxform::createDefaultFormScriptingEnvironment( _rModel ) )
                   ,m_Locks( 0 )
                   ,bReadOnly( false )
                   ,m_bDisposed( false )
{
    DBG_CTOR(FmXUndoEnvironment,NULL);
    try
    {
        m_vbaListener =  new ScriptEventListenerWrapper( _rModel );
    }
    catch( Exception& )
    {
    }
}


FmXUndoEnvironment::~FmXUndoEnvironment()
{
    DBG_DTOR(FmXUndoEnvironment,NULL);
    if ( !m_bDisposed )   
        m_pScriptingEnv->dispose();

    if (m_pPropertySetCache)
        delete static_cast<PropertySetInfoCache*>(m_pPropertySetCache);
}


void FmXUndoEnvironment::dispose()
{
    OSL_ENSURE( !m_bDisposed, "FmXUndoEnvironment::dispose: disposed twice?" );
    if ( !m_bDisposed )
        return;

    Lock();

    sal_uInt16 nCount = rModel.GetPageCount();
    sal_uInt16 i;
    for (i = 0; i < nCount; i++)
    {
        FmFormPage* pPage = PTR_CAST( FmFormPage, rModel.GetPage(i) );
        if ( pPage )
        {
            Reference< css::form::XForms > xForms = pPage->GetForms( false ).get();
            if ( xForms.is() )
                RemoveElement( xForms );
        }
    }

    nCount = rModel.GetMasterPageCount();
    for (i = 0; i < nCount; i++)
    {
        FmFormPage* pPage = PTR_CAST( FmFormPage, rModel.GetMasterPage(i) );
        if ( pPage )
        {
            Reference< css::form::XForms > xForms = pPage->GetForms( false ).get();
            if ( xForms.is() )
                RemoveElement( xForms );
        }
    }

    UnLock();

    OSL_PRECOND( rModel.GetObjectShell(), "FmXUndoEnvironment::dispose: no object shell anymore!" );
    if ( rModel.GetObjectShell() )
        EndListening( *rModel.GetObjectShell() );

    if ( IsListening( rModel ) )
        EndListening( rModel );

    m_pScriptingEnv->dispose();

    m_bDisposed = true;
}


void FmXUndoEnvironment::ModeChanged()
{
    OSL_PRECOND( rModel.GetObjectShell(), "FmXUndoEnvironment::ModeChanged: no object shell anymore!" );
    if ( !rModel.GetObjectShell() )
        return;

    if (bReadOnly != (rModel.GetObjectShell()->IsReadOnly() || rModel.GetObjectShell()->IsReadOnlyUI()))
    {
        bReadOnly = !bReadOnly;

        sal_uInt16 nCount = rModel.GetPageCount();
        sal_uInt16 i;
        for (i = 0; i < nCount; i++)
        {
            FmFormPage* pPage = PTR_CAST( FmFormPage, rModel.GetPage(i) );
            if ( pPage )
            {
                Reference< css::form::XForms > xForms = pPage->GetForms( false ).get();
                if ( xForms.is() )
                    TogglePropertyListening( xForms );
            }
        }

        nCount = rModel.GetMasterPageCount();
        for (i = 0; i < nCount; i++)
        {
            FmFormPage* pPage = PTR_CAST( FmFormPage, rModel.GetMasterPage(i) );
            if ( pPage )
            {
                Reference< css::form::XForms > xForms = pPage->GetForms( false ).get();
                if ( xForms.is() )
                    TogglePropertyListening( xForms );
            }
        }

        if (!bReadOnly)
            StartListening(rModel);
        else
            EndListening(rModel);
    }
}


void FmXUndoEnvironment::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (rHint.ISA(SdrHint))
    {
        SdrHint* pSdrHint = (SdrHint*)&rHint;
        switch( pSdrHint->GetKind() )
        {
            case HINT_OBJINSERTED:
            {
                SdrObject* pSdrObj = (SdrObject*)pSdrHint->GetObject();
                Inserted( pSdrObj );
            }   break;
            case HINT_OBJREMOVED:
            {
                SdrObject* pSdrObj = (SdrObject*)pSdrHint->GetObject();
                Removed( pSdrObj );
            }
            break;
            default:
                break;
        }
    }
    else if (rHint.ISA(SfxSimpleHint))
    {
        switch ( ((SfxSimpleHint&)rHint).GetId() )
        {
            case SFX_HINT_DYING:
                dispose();
                rModel.SetObjectShell( NULL );
                break;
            case SFX_HINT_MODECHANGED:
                ModeChanged();
                break;
        }
    }
    else if (rHint.ISA(SfxEventHint))
    {
        switch (((SfxEventHint&)rHint).GetEventId())
        {
        case SFX_EVENT_CREATEDOC:
            case SFX_EVENT_OPENDOC:
                ModeChanged();
                break;
        }
    }

}


void FmXUndoEnvironment::Inserted(SdrObject* pObj)
{
    if (pObj->GetObjInventor() == FmFormInventor)
    {
        FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
        Inserted( pFormObj );
    }
    else if (pObj->IsGroupObject())
    {
        SdrObjListIter aIter(*pObj->GetSubList());
        while ( aIter.IsMore() )
            Inserted( aIter.Next() );
    }
}


namespace
{
    sal_Bool lcl_searchElement(const Reference< XIndexAccess>& xCont, const Reference< XInterface >& xElement)
    {
        if (!xCont.is() || !xElement.is())
            return sal_False;

        sal_Int32 nCount = xCont->getCount();
        Reference< XInterface > xComp;
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            try
            {
                xCont->getByIndex(i) >>= xComp;
                if (xComp.is())
                {
                    if ( xElement == xComp )
                        return sal_True;
                    else
                    {
                        Reference< XIndexAccess> xCont2(xComp, UNO_QUERY);
                        if (xCont2.is() && lcl_searchElement(xCont2, xElement))
                            return sal_True;
                    }
                }
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return sal_False;
    }
}


void FmXUndoEnvironment::Inserted(FmFormObj* pObj)
{
    DBG_ASSERT( pObj, "FmXUndoEnvironment::Inserted: invalid object!" );
    if ( !pObj )
        return;

    
    Reference< XInterface >  xModel(pObj->GetUnoControlModel(), UNO_QUERY);
    Reference< XFormComponent >  xContent(xModel, UNO_QUERY);
    if (xContent.is() && pObj->GetPage())
    {
        
        if (!xContent->getParent().is())
        {
            try
            {
                Reference< XIndexContainer > xObjectParent = pObj->GetOriginalParent();

                FmFormPage& rPage = dynamic_cast< FmFormPage& >( *pObj->GetPage() );
                Reference< XIndexAccess >  xForms( rPage.GetForms(), UNO_QUERY_THROW );

                Reference< XIndexContainer > xNewParent;
                Reference< XForm >           xForm;
                sal_Int32 nPos = -1;
                if ( lcl_searchElement( xForms, xObjectParent ) )
                {
                    
                    
                    xNewParent = xObjectParent;
                    xForm.set( xNewParent, UNO_QUERY_THROW );
                    nPos = ::std::min( pObj->GetOriginalIndex(), xNewParent->getCount() );
                }
                else
                {
                    xForm.set( rPage.GetImpl().findPlaceInFormComponentHierarchy( xContent ), UNO_SET_THROW );
                    xNewParent.set( xForm, UNO_QUERY_THROW );
                    nPos = xNewParent->getCount();
                }

                rPage.GetImpl().setUniqueName( xContent, xForm );
                xNewParent->insertByIndex( nPos, makeAny( xContent ) );

                Reference< XEventAttacherManager >  xManager( xNewParent, UNO_QUERY_THROW );
                xManager->registerScriptEvents( nPos, pObj->GetOriginalEvents() );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        
        pObj->ClearObjEnv();
    }
}


void FmXUndoEnvironment::Removed(SdrObject* pObj)
{
    if ( pObj->IsVirtualObj() )
        
        
        return;

    if (pObj->GetObjInventor() == FmFormInventor)
    {
        FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
        Removed(pFormObj);
    }
    else if (pObj->IsGroupObject())
    {
        SdrObjListIter aIter(*pObj->GetSubList());
        while ( aIter.IsMore() )
            Removed( aIter.Next() );
    }
}


void FmXUndoEnvironment::Removed(FmFormObj* pObj)
{
    DBG_ASSERT( pObj, "FmXUndoEnvironment::Removed: invalid object!" );
    if ( !pObj )
        return;

    
    Reference< XFormComponent >  xContent(pObj->GetUnoControlModel(), UNO_QUERY);
    if (xContent.is())
    {
        
        
        

        
        
        Reference< XIndexContainer >  xForm(xContent->getParent(), UNO_QUERY);
        if (xForm.is())
        {
            Reference< XIndexAccess >  xIndexAccess((XIndexContainer*)xForm.get());
            
            const sal_Int32 nPos = getElementPos(xIndexAccess, xContent);
            if (nPos >= 0)
            {
                Sequence< ScriptEventDescriptor > aEvts;
                Reference< XEventAttacherManager >  xManager(xForm, UNO_QUERY);
                if (xManager.is())
                    aEvts = xManager->getScriptEvents(nPos);

                try
                {
                    pObj->SetObjEnv(xForm, nPos, aEvts);
                    xForm->removeByIndex(nPos);
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

            }
        }
    }
}



void SAL_CALL FmXUndoEnvironment::disposing(const EventObject& e) throw( RuntimeException )
{
    
    if (m_pPropertySetCache)
    {
        Reference< XPropertySet > xSourceSet(e.Source, UNO_QUERY);
        if (xSourceSet.is())
        {
            PropertySetInfoCache* pCache = static_cast<PropertySetInfoCache*>(m_pPropertySetCache);
            PropertySetInfoCache::iterator aSetPos = pCache->find(xSourceSet);
            if (aSetPos != pCache->end())
                pCache->erase(aSetPos);
        }
    }
}



void SAL_CALL FmXUndoEnvironment::propertyChange(const PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    if (!IsLocked())
    {
        Reference< XPropertySet >  xSet(evt.Source, UNO_QUERY);
        if (!xSet.is())
            return;

        
        static const OUString pDefaultValueProperties[] = {
            OUString(FM_PROP_DEFAULT_TEXT), OUString(FM_PROP_DEFAULTCHECKED), OUString(FM_PROP_DEFAULT_DATE), OUString(FM_PROP_DEFAULT_TIME),
            OUString(FM_PROP_DEFAULT_VALUE), OUString(FM_PROP_DEFAULT_SELECT_SEQ), OUString(FM_PROP_EFFECTIVE_DEFAULT)
        };
        const OUString aValueProperties[] = {
            OUString(FM_PROP_TEXT), OUString(FM_PROP_STATE), OUString(FM_PROP_DATE), OUString(FM_PROP_TIME),
            OUString(FM_PROP_VALUE), OUString(FM_PROP_SELECT_SEQ), OUString(FM_PROP_EFFECTIVE_VALUE)
        };
        sal_Int32 nDefaultValueProps = sizeof(pDefaultValueProperties)/sizeof(pDefaultValueProperties[0]);
        OSL_ENSURE(sizeof(aValueProperties)/sizeof(aValueProperties[0]) == nDefaultValueProps,
            "FmXUndoEnvironment::propertyChange: inconsistence!");
        for (sal_Int32 i=0; i<nDefaultValueProps; ++i)
        {
            if (evt.PropertyName == pDefaultValueProperties[i])
            {
                try
                {
                    xSet->setPropertyValue(aValueProperties[i], evt.NewValue);
                }
                catch(const Exception&)
                {
                    OSL_FAIL("FmXUndoEnvironment::propertyChange: could not adjust the value property!");
                }
            }
        }

        
        
        
        
        
        
        
        
        

        if (!m_pPropertySetCache)
            m_pPropertySetCache = new PropertySetInfoCache;
        PropertySetInfoCache* pCache = static_cast<PropertySetInfoCache*>(m_pPropertySetCache);

        
        PropertySetInfoCache::iterator aSetPos = pCache->find(xSet);
        if (aSetPos == pCache->end())
        {
            PropertySetInfo aNewEntry;
            if (!::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xSet))
            {
                aNewEntry.bHasEmptyControlSource = sal_False;
            }
            else
            {
                try
                {
                    Any aCurrentControlSource = xSet->getPropertyValue(FM_PROP_CONTROLSOURCE);
                    aNewEntry.bHasEmptyControlSource = !aCurrentControlSource.hasValue() || ::comphelper::getString(aCurrentControlSource).isEmpty();
                }
                catch(const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            aSetPos = pCache->insert(PropertySetInfoCache::value_type(xSet,aNewEntry)).first;
            DBG_ASSERT(aSetPos != pCache->end(), "FmXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
        }
        else
        {   
            if (evt.PropertyName.equals(FM_PROP_CONTROLSOURCE))
            {
                aSetPos->second.bHasEmptyControlSource = !evt.NewValue.hasValue() || ::comphelper::getString(evt.NewValue).isEmpty();
            }
        }

        
        
        PropertySetInfo::AllProperties& rPropInfos = aSetPos->second.aProps;
        PropertySetInfo::AllProperties::iterator aPropertyPos = rPropInfos.find(evt.PropertyName);
        if (aPropertyPos == rPropInfos.end())
        {   
            PropertyInfo aNewEntry;

            
            sal_Int32 nAttributes = xSet->getPropertySetInfo()->getPropertyByName(evt.PropertyName).Attributes;
            aNewEntry.bIsTransientOrReadOnly = ((nAttributes & PropertyAttribute::READONLY) != 0) || ((nAttributes & PropertyAttribute::TRANSIENT) != 0);

            
            aNewEntry.bIsValueProperty = sal_False;
            try
            {
                if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCEPROPERTY, xSet))
                {
                    Any aControlSourceProperty = xSet->getPropertyValue(FM_PROP_CONTROLSOURCEPROPERTY);
                    OUString sControlSourceProperty;
                    aControlSourceProperty >>= sControlSourceProperty;

                    aNewEntry.bIsValueProperty = (sControlSourceProperty.equals(evt.PropertyName));
                }
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            
            aPropertyPos = rPropInfos.insert(PropertySetInfo::AllProperties::value_type(evt.PropertyName,aNewEntry)).first;
            DBG_ASSERT(aPropertyPos != rPropInfos.end(), "FmXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
        }

        
        

        bool bAddUndoAction = rModel.IsUndoEnabled();
        
        if ( bAddUndoAction && aPropertyPos->second.bIsTransientOrReadOnly )
            bAddUndoAction = false;

        if ( bAddUndoAction && aPropertyPos->second.bIsValueProperty )
        {
            
            
            if ( !aSetPos->second.bHasEmptyControlSource )
                bAddUndoAction = false;

            
            if ( bAddUndoAction )
            {
                Reference< XBindableValue > xBindable( evt.Source, UNO_QUERY );
                Reference< XValueBinding > xBinding;
                if ( xBindable.is() )
                    xBinding = xBindable->getValueBinding();

                Reference< XPropertySet > xBindingProps;
                Reference< XPropertySetInfo > xBindingPropsPSI;
                if ( xBindable.is() )
                    xBindingProps.set( xBinding, UNO_QUERY );
                if ( xBindingProps.is() )
                    xBindingPropsPSI = xBindingProps->getPropertySetInfo();
                
                

                static const OUString s_sExternalData( "ExternalData" );
                if ( xBindingPropsPSI.is() && xBindingPropsPSI->hasPropertyByName( s_sExternalData ) )
                {
                    sal_Bool bExternalData = sal_True;
                    OSL_VERIFY( xBindingProps->getPropertyValue( s_sExternalData ) >>= bExternalData );
                    bAddUndoAction = !bExternalData;
                }
                else
                    bAddUndoAction = !xBinding.is();
            }
        }

        if ( bAddUndoAction && ( evt.PropertyName == FM_PROP_STRINGITEMLIST ) )
        {
            Reference< XListEntrySink > xSink( evt.Source, UNO_QUERY );
            if ( xSink.is() && xSink->getListEntrySource().is() )
                
                bAddUndoAction = false;
        }

        if ( bAddUndoAction )
        {
            aGuard.clear();
            
            

            SolarMutexGuard aSolarGuard;
            rModel.AddUndo(new FmUndoPropertyAction(rModel, evt));
        }
    }
    else
    {
        
        if (m_pPropertySetCache && evt.PropertyName.equals(FM_PROP_CONTROLSOURCE))
        {
            Reference< XPropertySet >  xSet(evt.Source, UNO_QUERY);
            PropertySetInfoCache* pCache = static_cast<PropertySetInfoCache*>(m_pPropertySetCache);
            PropertySetInfo& rSetInfo = (*pCache)[xSet];
            rSetInfo.bHasEmptyControlSource = !evt.NewValue.hasValue() || ::comphelper::getString(evt.NewValue).isEmpty();
        }
    }
}



void SAL_CALL FmXUndoEnvironment::elementInserted(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    
    Reference< XInterface >  xIface;
    evt.Element >>= xIface;
    OSL_ENSURE(xIface.is(), "FmXUndoEnvironment::elementInserted: invalid container notification!");
    AddElement(xIface);

    implSetModified();
}


void FmXUndoEnvironment::implSetModified()
{
    if ( !IsLocked() && rModel.GetObjectShell() )
    {
        rModel.GetObjectShell()->SetModified( sal_True );
    }
}


void SAL_CALL FmXUndoEnvironment::elementReplaced(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XInterface >  xIface;
    evt.ReplacedElement >>= xIface;
    OSL_ENSURE(xIface.is(), "FmXUndoEnvironment::elementReplaced: invalid container notification!");
    RemoveElement(xIface);

    evt.Element >>= xIface;
    AddElement(xIface);

    implSetModified();
}


void SAL_CALL FmXUndoEnvironment::elementRemoved(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XInterface >  xIface( evt.Element, UNO_QUERY );
    OSL_ENSURE(xIface.is(), "FmXUndoEnvironment::elementRemoved: invalid container notification!");
    RemoveElement(xIface);

    implSetModified();
}


void SAL_CALL FmXUndoEnvironment::modified( const EventObject& /*aEvent*/ ) throw (RuntimeException)
{
    implSetModified();
}


void FmXUndoEnvironment::AddForms(const Reference< XNameContainer > & rForms)
{
    Lock();
    AddElement(rForms);
    UnLock();
}


void FmXUndoEnvironment::RemoveForms(const Reference< XNameContainer > & rForms)
{
    Lock();
    RemoveElement(rForms);
    UnLock();
}


void FmXUndoEnvironment::TogglePropertyListening(const Reference< XInterface > & Element)
{
    
    Reference< XIndexContainer >  xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface >  xIface;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            xContainer->getByIndex(i) >>= xIface;
            TogglePropertyListening(xIface);
        }
    }

    Reference< XPropertySet >  xSet(Element, UNO_QUERY);
    if (xSet.is())
    {
        if (!bReadOnly)
            xSet->addPropertyChangeListener( OUString(), this );
        else
            xSet->removePropertyChangeListener( OUString(), this );
    }
}



void FmXUndoEnvironment::switchListening( const Reference< XIndexContainer >& _rxContainer, bool _bStartListening ) SAL_THROW(())
{
    OSL_PRECOND( _rxContainer.is(), "FmXUndoEnvironment::switchListening: invalid container!" );
    if ( !_rxContainer.is() )
        return;

    try
    {
        
        
        Reference< XEventAttacherManager > xManager( _rxContainer, UNO_QUERY );
        if ( xManager.is() )
        {
            if ( _bStartListening )
            {
                m_pScriptingEnv->registerEventAttacherManager( xManager );
                if ( m_vbaListener.is() )
                    xManager->addScriptListener( m_vbaListener );
            }
            else
            {
                m_pScriptingEnv->revokeEventAttacherManager( xManager );
                if ( m_vbaListener.is() )
                    xManager->removeScriptListener( m_vbaListener );
            }
        }

        
        sal_uInt32 nCount = _rxContainer->getCount();
        Reference< XInterface > xInterface;
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            _rxContainer->getByIndex( i ) >>= xInterface;
            if ( _bStartListening )
                AddElement( xInterface );
            else
                RemoveElement( xInterface );
        }

        
        Reference< XContainer > xSimpleContainer( _rxContainer, UNO_QUERY );
        OSL_ENSURE( xSimpleContainer.is(), "FmXUndoEnvironment::switchListening: how are we expected to be notified of changes in the container?" );
        if ( xSimpleContainer.is() )
        {
            if ( _bStartListening )
                xSimpleContainer->addContainerListener( this );
            else
                xSimpleContainer->removeContainerListener( this );
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FmXUndoEnvironment::switchListening: caught an exception!" );
    }
}


void FmXUndoEnvironment::switchListening( const Reference< XInterface >& _rxObject, bool _bStartListening ) SAL_THROW(())
{
    OSL_PRECOND( _rxObject.is(), "FmXUndoEnvironment::switchListening: how should I listen at a NULL object?" );

    try
    {
        if ( !bReadOnly )
        {
            Reference< XPropertySet > xProps( _rxObject, UNO_QUERY );
            if ( xProps.is() )
            {
                if ( _bStartListening )
                    xProps->addPropertyChangeListener( OUString(), this );
                else
                    xProps->removePropertyChangeListener( OUString(), this );
            }
        }

        Reference< XModifyBroadcaster > xBroadcaster( _rxObject, UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            if ( _bStartListening )
                xBroadcaster->addModifyListener( this );
            else
                xBroadcaster->removeModifyListener( this );
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FmXUndoEnvironment::switchListening: caught an exception!" );
    }
}


void FmXUndoEnvironment::AddElement(const Reference< XInterface >& _rxElement )
{
    OSL_ENSURE( !m_bDisposed, "FmXUndoEnvironment::AddElement: not when I'm already disposed!" );

    
    Reference< XIndexContainer > xContainer( _rxElement, UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, true );

    switchListening( _rxElement, true );
}


void FmXUndoEnvironment::RemoveElement(const Reference< XInterface >& _rxElement)
{
    if ( m_bDisposed )
        return;

    switchListening( _rxElement, false );

    if (!bReadOnly)
    {
        
        
        
        Reference< XForm > xForm( _rxElement, UNO_QUERY );
        Reference< XPropertySet > xFormProperties( xForm, UNO_QUERY );
        if ( xFormProperties.is() )
            if ( !::svxform::OStaticDataAccessTools().isEmbeddedInDatabase( _rxElement ) )
                
                
                
                xFormProperties->setPropertyValue( FM_PROP_ACTIVE_CONNECTION, Any() );
    }

    Reference< XIndexContainer > xContainer( _rxElement, UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, false );
}



FmUndoPropertyAction::FmUndoPropertyAction(FmFormModel& rNewMod, const PropertyChangeEvent& evt)
                     :SdrUndoAction(rNewMod)
                     ,xObj(evt.Source, UNO_QUERY)
                     ,aPropertyName(evt.PropertyName)
                     ,aNewValue(evt.NewValue)
                     ,aOldValue(evt.OldValue)
{
    if (rNewMod.GetObjectShell())
        rNewMod.GetObjectShell()->SetModified(sal_True);
    if(static_STR_UNDO_PROPERTY.isEmpty())
        static_STR_UNDO_PROPERTY = SVX_RESSTR(RID_STR_UNDO_PROPERTY);
}



void FmUndoPropertyAction::Undo()
{
    FmXUndoEnvironment& rEnv = ((FmFormModel&)rMod).GetUndoEnv();

    if (xObj.is() && !rEnv.IsLocked())
    {
        rEnv.Lock();
        try
        {
            xObj->setPropertyValue( aPropertyName, aOldValue );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmUndoPropertyAction::Undo: caught an exception!" );
        }
        rEnv.UnLock();
    }
}


void FmUndoPropertyAction::Redo()
{
    FmXUndoEnvironment& rEnv = ((FmFormModel&)rMod).GetUndoEnv();

    if (xObj.is() && !rEnv.IsLocked())
    {
        rEnv.Lock();
        try
        {
            xObj->setPropertyValue( aPropertyName, aNewValue );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmUndoPropertyAction::Redo: caught an exception!" );
        }
        rEnv.UnLock();
    }
}


OUString FmUndoPropertyAction::GetComment() const
{
    OUString aStr(static_STR_UNDO_PROPERTY);

    aStr = aStr.replaceFirst( "#", aPropertyName );
    return aStr;
}


DBG_NAME(FmUndoContainerAction);

FmUndoContainerAction::FmUndoContainerAction(FmFormModel& _rMod,
                                             Action _eAction,
                                             const Reference< XIndexContainer > & xCont,
                                             const Reference< XInterface > & xElem,
                                             sal_Int32 nIdx)
                      :SdrUndoAction( _rMod )
                      ,m_xContainer( xCont )
                      ,m_nIndex( nIdx )
                      ,m_eAction( _eAction )
{
    OSL_ENSURE( nIdx >= 0, "FmUndoContainerAction::FmUndoContainerAction: invalid index!" );
        
        

    DBG_CTOR(FmUndoContainerAction,NULL);
    if ( xCont.is() && xElem.is() )
    {
        
        m_xElement = m_xElement.query( xElem );
        if ( m_eAction == Removed )
        {
            if (m_nIndex >= 0)
            {
                Reference< XEventAttacherManager >  xManager( xCont, UNO_QUERY );
                if ( xManager.is() )
                    m_aEvents = xManager->getScriptEvents(m_nIndex);
            }
            else
                m_xElement = NULL;

            
            m_xOwnElement = m_xElement;
        }
    }
}


FmUndoContainerAction::~FmUndoContainerAction()
{
    
    DisposeElement( m_xOwnElement );
    DBG_DTOR(FmUndoContainerAction,NULL);
}



void FmUndoContainerAction::DisposeElement( const Reference< XInterface > & xElem )
{
    Reference< XComponent > xComp( xElem, UNO_QUERY );
    if ( xComp.is() )
    {
        
        Reference< XChild >  xChild( xElem, UNO_QUERY );
        if ( xChild.is() && !xChild->getParent().is() )
            
            xComp->dispose();
    }
}


void FmUndoContainerAction::implReInsert( ) SAL_THROW( ( Exception ) )
{
    if ( m_xContainer->getCount() >= m_nIndex )
    {
        
        Any aVal;
        if ( m_xContainer->getElementType() == ::getCppuType( static_cast< const Reference< XFormComponent >* >( NULL ) ) )
        {
            aVal <<= Reference< XFormComponent >( m_xElement, UNO_QUERY );
        }
        else
        {
            aVal <<= Reference< XForm >( m_xElement, UNO_QUERY );
        }
        m_xContainer->insertByIndex( m_nIndex, aVal );

        OSL_ENSURE( getElementPos( m_xContainer.get(), m_xElement ) == m_nIndex, "FmUndoContainerAction::implReInsert: insertion did not work!" );

        
        Reference< XEventAttacherManager >  xManager( m_xContainer, UNO_QUERY );
        if ( xManager.is() )
            xManager->registerScriptEvents( m_nIndex, m_aEvents );

        
        m_xOwnElement = NULL;
    }
}


void FmUndoContainerAction::implReRemove( ) SAL_THROW( ( Exception ) )
{
    Reference< XInterface > xElement;
    if ( ( m_nIndex >= 0 ) && ( m_nIndex < m_xContainer->getCount() ) )
        m_xContainer->getByIndex( m_nIndex ) >>= xElement;

    if ( xElement != m_xElement )
    {
        
        
        m_nIndex = getElementPos( m_xContainer.get(), m_xElement );
        if ( m_nIndex != -1 )
            xElement = m_xElement;
    }

    OSL_ENSURE( xElement == m_xElement, "FmUndoContainerAction::implReRemove: cannot find the element which I'm responsible for!" );
    if ( xElement == m_xElement )
    {
        Reference< XEventAttacherManager >  xManager( m_xContainer, UNO_QUERY );
        if ( xManager.is() )
            m_aEvents = xManager->getScriptEvents( m_nIndex );
        m_xContainer->removeByIndex( m_nIndex );
        
        m_xOwnElement = m_xElement;
    }
}


void FmUndoContainerAction::Undo()
{
    FmXUndoEnvironment& rEnv = static_cast< FmFormModel& >( rMod ).GetUndoEnv();

    if ( m_xContainer.is() && !rEnv.IsLocked() && m_xElement.is() )
    {
        rEnv.Lock();
        try
        {
            switch ( m_eAction )
            {
            case Inserted:
                implReRemove();
                break;

            case Removed:
                implReInsert();
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmUndoContainerAction::Undo: caught an exception!" );
        }
        rEnv.UnLock();
    }
}


void FmUndoContainerAction::Redo()
{
    FmXUndoEnvironment& rEnv = static_cast< FmFormModel& >( rMod ).GetUndoEnv();
    if ( m_xContainer.is() && !rEnv.IsLocked() && m_xElement.is() )
    {
        rEnv.Lock();
        try
        {
            switch ( m_eAction )
            {
            case Inserted:
                implReInsert();
                break;

            case Removed:
                implReRemove();
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmUndoContainerAction::Redo: caught an exception!" );
        }
        rEnv.UnLock();
    }
}


FmUndoModelReplaceAction::FmUndoModelReplaceAction(FmFormModel& _rMod, SdrUnoObj* _pObject, const Reference< XControlModel > & _xReplaced)
    :SdrUndoAction(_rMod)
    ,m_xReplaced(_xReplaced)
    ,m_pObject(_pObject)
{
}


FmUndoModelReplaceAction::~FmUndoModelReplaceAction()
{
    
    DisposeElement(m_xReplaced);
}



void FmUndoModelReplaceAction::DisposeElement( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xReplaced )
{
    Reference< XComponent >  xComp(xReplaced, UNO_QUERY);
    if (xComp.is())
    {
        Reference< XChild >  xChild(xReplaced, UNO_QUERY);
        if (!xChild.is() || !xChild->getParent().is())
            xComp->dispose();
    }
}


void FmUndoModelReplaceAction::Undo()
{
    try
    {
        Reference< XControlModel > xCurrentModel( m_pObject->GetUnoControlModel() );

        
        Reference< XChild > xCurrentAsChild( xCurrentModel, UNO_QUERY );
        Reference< XNameContainer > xCurrentsParent;
        if ( xCurrentAsChild.is() )
            xCurrentsParent = xCurrentsParent.query( xCurrentAsChild->getParent() );
        DBG_ASSERT( xCurrentsParent.is(), "FmUndoModelReplaceAction::Undo: invalid current model!" );

        if ( xCurrentsParent.is() )
        {
            
            Reference< XFormComponent > xComponent( m_xReplaced, UNO_QUERY );
            DBG_ASSERT( xComponent.is(), "FmUndoModelReplaceAction::Undo: the new model is no form component !" );

            Reference< XPropertySet > xCurrentAsSet( xCurrentModel, UNO_QUERY );
            DBG_ASSERT( ::comphelper::hasProperty(FM_PROP_NAME, xCurrentAsSet ), "FmUndoModelReplaceAction::Undo : one of the models is invalid !");

            OUString sName;
            xCurrentAsSet->getPropertyValue( FM_PROP_NAME ) >>= sName;
            xCurrentsParent->replaceByName( sName, makeAny( xComponent ) );

            m_pObject->SetUnoControlModel(m_xReplaced);
            m_pObject->SetChanged();

            m_xReplaced = xCurrentModel;
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("FmUndoModelReplaceAction::Undo : could not replace the model !");
    }
}


OUString FmUndoModelReplaceAction::GetComment() const
{
    return SVX_RESSTR(RID_STR_UNDO_MODEL_REPLACE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
