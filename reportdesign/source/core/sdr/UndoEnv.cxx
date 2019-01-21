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
#include <UndoActions.hxx>
#include <UndoEnv.hxx>
#include "formatnormalizer.hxx"
#include <conditionupdater.hxx>
#include <strings.hxx>
#include <rptui_slotid.hrc>
#include <RptDef.hxx>
#include <RptObject.hxx>
#include <RptPage.hxx>
#include <strings.hrc>
#include <RptModel.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>

#include <connectivity/dbtools.hxx>
#include <svl/hint.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/stl_types.hxx>
#include <vcl/svapp.hxx>
#include <dbaccess/dbsubcomponentcontroller.hxx>
#include <svx/unoshape.hxx>
#include <osl/mutex.hxx>

namespace rptui
{
    using namespace ::com::sun::star;
    using namespace uno;
    using namespace lang;
    using namespace script;
    using namespace beans;
    using namespace awt;
    using namespace util;
    using namespace container;
    using namespace report;


struct PropertyInfo
{
    bool const bIsReadonlyOrTransient;

    explicit PropertyInfo( const bool i_bIsTransientOrReadOnly )
        :bIsReadonlyOrTransient( i_bIsTransientOrReadOnly )
    {
    }
};

typedef std::unordered_map< OUString, PropertyInfo >    PropertiesInfo;

struct ObjectInfo
{
    PropertiesInfo              aProperties;
    Reference< XPropertySet >   xPropertyIntrospection;

    ObjectInfo()
        :aProperties()
        ,xPropertyIntrospection()
    {
    }
};

typedef ::std::map< Reference< XPropertySet >, ObjectInfo, ::comphelper::OInterfaceCompare< XPropertySet > >    PropertySetInfoCache;


class OXUndoEnvironmentImpl
{
public:
    OReportModel&                                       m_rModel;
    PropertySetInfoCache                                m_aPropertySetCache;
    FormatNormalizer                                    m_aFormatNormalizer;
    ConditionUpdater                                    m_aConditionUpdater;
    ::osl::Mutex                                        m_aMutex;
    ::std::vector< uno::Reference< container::XChild> > m_aSections;
    Reference< XIntrospection >                         m_xIntrospection;
    oslInterlockedCount                                 m_nLocks;
    bool                                            m_bReadOnly;
    bool                                            m_bIsUndo;

    explicit OXUndoEnvironmentImpl(OReportModel& _rModel);
    OXUndoEnvironmentImpl(const OXUndoEnvironmentImpl&) = delete;
    OXUndoEnvironmentImpl& operator=(const OXUndoEnvironmentImpl&) = delete;
};

OXUndoEnvironmentImpl::OXUndoEnvironmentImpl(OReportModel& _rModel) : m_rModel(_rModel)
        ,m_aFormatNormalizer( _rModel )
        ,m_aConditionUpdater()
        ,m_nLocks(0)
        ,m_bReadOnly(false)
        ,m_bIsUndo(false)
{
}


OXUndoEnvironment::OXUndoEnvironment(OReportModel& _rModel)
                   :m_pImpl(new OXUndoEnvironmentImpl(_rModel) )
{
    StartListening(m_pImpl->m_rModel);
}


OXUndoEnvironment::~OXUndoEnvironment()
{
}

void OXUndoEnvironment::Lock()
{
    OSL_ENSURE(m_refCount,"Illegal call to dead object!");
    osl_atomic_increment( &m_pImpl->m_nLocks );
}
void OXUndoEnvironment::UnLock()
{
    OSL_ENSURE(m_refCount,"Illegal call to dead object!");

    osl_atomic_decrement( &m_pImpl->m_nLocks );
}
bool OXUndoEnvironment::IsLocked() const { return m_pImpl->m_nLocks != 0; }

void OXUndoEnvironment::RemoveSection(OReportPage const * _pPage)
{
    if ( _pPage )
    {
        Reference< XInterface > xSection(_pPage->getSection());
        if ( xSection.is() )
            RemoveElement( xSection );
    }
}

void OXUndoEnvironment::Clear(const Accessor& /*_r*/)
{
    OUndoEnvLock aLock(*this);

    m_pImpl->m_aPropertySetCache.clear();

    sal_uInt16 nCount = m_pImpl->m_rModel.GetPageCount();
    sal_uInt16 i;
    for (i = 0; i < nCount; i++)
    {
        OReportPage* pPage = dynamic_cast<OReportPage*>( m_pImpl->m_rModel.GetPage(i)  );
        RemoveSection(pPage);
    }

    nCount = m_pImpl->m_rModel.GetMasterPageCount();
    for (i = 0; i < nCount; i++)
    {
        OReportPage* pPage = dynamic_cast<OReportPage*>( m_pImpl->m_rModel.GetMasterPage(i)  );
        RemoveSection(pPage);
    }

    m_pImpl->m_aSections.clear();

    if (IsListening(m_pImpl->m_rModel))
        EndListening(m_pImpl->m_rModel);
}


void OXUndoEnvironment::ModeChanged()
{
    m_pImpl->m_bReadOnly = !m_pImpl->m_bReadOnly;

    if (!m_pImpl->m_bReadOnly)
        StartListening(m_pImpl->m_rModel);
    else
        EndListening(m_pImpl->m_rModel);
}


void OXUndoEnvironment::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::ModeChanged )
        ModeChanged();
}

//  XEventListener

void SAL_CALL OXUndoEnvironment::disposing(const EventObject& e)
{
    // check if it's an object we have cached information about
    Reference< XPropertySet > xSourceSet(e.Source, UNO_QUERY);
    if ( xSourceSet.is() )
    {
        uno::Reference< report::XSection> xSection(xSourceSet,uno::UNO_QUERY);
        if ( xSection.is() )
            RemoveSection(xSection);
        else
            RemoveElement(xSourceSet);
    }
}

// XPropertyChangeListener

void SAL_CALL OXUndoEnvironment::propertyChange( const PropertyChangeEvent& _rEvent )
{

    ::osl::ClearableMutexGuard aGuard( m_pImpl->m_aMutex );

    if ( IsLocked() )
        return;

    Reference< XPropertySet >  xSet( _rEvent.Source, UNO_QUERY );
    if (!xSet.is())
        return;

    dbaui::DBSubComponentController* pController = m_pImpl->m_rModel.getController();
    if ( !pController )
        return;

    // no Undo for transient and readonly props.
    // let's see if we know something about the set
    PropertySetInfoCache::iterator objectPos = m_pImpl->m_aPropertySetCache.find(xSet);
    if (objectPos == m_pImpl->m_aPropertySetCache.end())
    {
        objectPos = m_pImpl->m_aPropertySetCache.emplace( xSet, ObjectInfo() ).first;
        DBG_ASSERT(objectPos != m_pImpl->m_aPropertySetCache.end(), "OXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
    }
    if ( objectPos == m_pImpl->m_aPropertySetCache.end() )
        return;

    // now we have access to the cached info about the set
    // let's see what we know about the property
    ObjectInfo& rObjectInfo = objectPos->second;
    PropertiesInfo::const_iterator aPropertyPos = rObjectInfo.aProperties.find( _rEvent.PropertyName );
    if ( aPropertyPos == rObjectInfo.aProperties.end() )
    {   // nothing 'til now ... have to change this ....
        // the attributes
        Reference< XPropertySetInfo > xPSI( xSet->getPropertySetInfo(), UNO_SET_THROW );
        sal_Int32 nPropertyAttributes = 0;
        try
        {
            if ( xPSI->hasPropertyByName( _rEvent.PropertyName ) )
            {
                nPropertyAttributes = xPSI->getPropertyByName( _rEvent.PropertyName ).Attributes;
            }
            else
            {
                // it's perfectly valid for a component to notify a change in a property which it doesn't have - as long
                // as it has an attribute with this name
                if ( !rObjectInfo.xPropertyIntrospection.is() )
                {
                    if ( !m_pImpl->m_xIntrospection.is() )
                    {
                        m_pImpl->m_xIntrospection = theIntrospection::get( m_pImpl->m_rModel.getController()->getORB() );
                    }
                    Reference< XIntrospectionAccess > xIntrospection(
                        m_pImpl->m_xIntrospection->inspect( makeAny( _rEvent.Source ) ),
                        UNO_SET_THROW
                    );
                    rObjectInfo.xPropertyIntrospection.set( xIntrospection->queryAdapter( cppu::UnoType<XPropertySet>::get() ), UNO_QUERY_THROW );
                }
                if ( rObjectInfo.xPropertyIntrospection.is() )
                {
                    xPSI.set( rObjectInfo.xPropertyIntrospection->getPropertySetInfo(), UNO_SET_THROW );
                    nPropertyAttributes = xPSI->getPropertyByName( _rEvent.PropertyName ).Attributes;
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
        const bool bTransReadOnly =
                    ( ( nPropertyAttributes & PropertyAttribute::READONLY ) != 0 )
                ||  ( ( nPropertyAttributes & PropertyAttribute::TRANSIENT ) != 0 );

        // insert the new entry
        aPropertyPos = rObjectInfo.aProperties.emplace(
            _rEvent.PropertyName,
            PropertyInfo( bTransReadOnly )
        ).first;
        DBG_ASSERT(aPropertyPos != rObjectInfo.aProperties.end(), "OXUndoEnvironment::propertyChange : just inserted it ... why it's not there ?");
    }

    implSetModified();

    // now we have access to the cached info about the property affected
    // and are able to decide whether or not we need an undo action

    // no UNDO for transient/readonly properties
    if ( aPropertyPos->second.bIsReadonlyOrTransient )
        return;

    // give components with sub responsibilities a chance
    m_pImpl->m_aFormatNormalizer.notifyPropertyChange( _rEvent );
    m_pImpl->m_aConditionUpdater.notifyPropertyChange( _rEvent );

    aGuard.clear();
    // TODO: this is a potential race condition: two threads here could in theory
    // add their undo actions out-of-order

    SolarMutexGuard aSolarGuard;
    std::unique_ptr<ORptUndoPropertyAction> pUndo;
    try
    {
        uno::Reference< report::XSection> xSection( xSet, uno::UNO_QUERY );
        if ( xSection.is() )
        {
            uno::Reference< report::XGroup> xGroup = xSection->getGroup();
            if ( xGroup.is() )
                pUndo.reset(new OUndoPropertyGroupSectionAction( m_pImpl->m_rModel, _rEvent, OGroupHelper::getMemberFunction( xSection ), xGroup ));
            else
                pUndo.reset(new OUndoPropertyReportSectionAction( m_pImpl->m_rModel, _rEvent, OReportHelper::getMemberFunction( xSection ), xSection->getReportDefinition() ));
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }

    if ( pUndo == nullptr )
        pUndo.reset(new ORptUndoPropertyAction( m_pImpl->m_rModel, _rEvent ));

    m_pImpl->m_rModel.GetSdrUndoManager()->AddUndoAction( std::move(pUndo) );
    pController->InvalidateAll();
}

::std::vector< uno::Reference< container::XChild> >::const_iterator OXUndoEnvironment::getSection(const Reference<container::XChild>& _xContainer) const
{
    ::std::vector< uno::Reference< container::XChild> >::const_iterator aFind = m_pImpl->m_aSections.end();
    if ( _xContainer.is() )
    {
        aFind = ::std::find(m_pImpl->m_aSections.begin(),m_pImpl->m_aSections.end(),_xContainer);

        if ( aFind == m_pImpl->m_aSections.end() )
        {
            Reference<container::XChild> xParent(_xContainer->getParent(),uno::UNO_QUERY);
            aFind = getSection(xParent);
        }
    }
    return aFind;
}
// XContainerListener

void SAL_CALL OXUndoEnvironment::elementInserted(const ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    // new listener object
    Reference< uno::XInterface >  xIface( evt.Element, UNO_QUERY );
    if ( !IsLocked() )
    {
        Reference< report::XReportComponent >  xReportComponent( xIface, UNO_QUERY );
        if ( xReportComponent.is() )
        {
            Reference< report::XSection > xContainer(evt.Source,uno::UNO_QUERY);

            ::std::vector< uno::Reference< container::XChild> >::const_iterator aFind = getSection(xContainer.get());

            if ( aFind != m_pImpl->m_aSections.end() )
            {
                OUndoEnvLock aLock(*this);
                try
                {
                    OReportPage* pPage = m_pImpl->m_rModel.getPage(uno::Reference< report::XSection>(*aFind,uno::UNO_QUERY));
                    OSL_ENSURE(pPage,"No page could be found for section!");
                    if ( pPage )
                        pPage->insertObject(xReportComponent);
                }
                catch(uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("reportdesign");
                }

            }
        }
        else
        {
            uno::Reference< report::XFunctions> xContainer(evt.Source,uno::UNO_QUERY);
            if ( xContainer.is() )
            {
                m_pImpl->m_rModel.GetSdrUndoManager()->AddUndoAction(
                    std::make_unique<OUndoContainerAction>( m_pImpl->m_rModel, rptui::Inserted, xContainer.get(),
                        xIface, RID_STR_UNDO_ADDFUNCTION ) );
            }
        }
    }

    AddElement(xIface);

    implSetModified();
}


void OXUndoEnvironment::implSetModified()
{
    m_pImpl->m_rModel.SetModified( true );
}


void SAL_CALL OXUndoEnvironment::elementReplaced(const ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    Reference< XInterface >  xIface(evt.ReplacedElement,uno::UNO_QUERY);
    OSL_ENSURE(xIface.is(), "OXUndoEnvironment::elementReplaced: invalid container notification!");
    RemoveElement(xIface);

    xIface.set(evt.Element,uno::UNO_QUERY);
    AddElement(xIface);

    implSetModified();
}


void SAL_CALL OXUndoEnvironment::elementRemoved(const ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    Reference< uno::XInterface >  xIface( evt.Element, UNO_QUERY );
    if ( !IsLocked() )
    {
        Reference< report::XSection > xContainer(evt.Source,uno::UNO_QUERY);
        ::std::vector< uno::Reference< container::XChild> >::const_iterator aFind = getSection(xContainer.get());

        Reference< report::XReportComponent >  xReportComponent( xIface, UNO_QUERY );
        if ( aFind != m_pImpl->m_aSections.end() && xReportComponent.is() )
        {
            OXUndoEnvironment::OUndoEnvLock aLock(*this);
            try
            {
                OReportPage* pPage = m_pImpl->m_rModel.getPage(uno::Reference< report::XSection >( *aFind, uno::UNO_QUERY_THROW ) );
                OSL_ENSURE( pPage, "OXUndoEnvironment::elementRemoved: no page for the section!" );
                if ( pPage )
                    pPage->removeSdrObject(xReportComponent);
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("reportdesign");
            }
        }
        else
        {
            uno::Reference< report::XFunctions> xFunctions(evt.Source,uno::UNO_QUERY);
            if ( xFunctions.is() )
            {
                m_pImpl->m_rModel.GetSdrUndoManager()->AddUndoAction( std::make_unique<OUndoContainerAction>(
                    m_pImpl->m_rModel, rptui::Removed, xFunctions.get(), xIface, RID_STR_UNDO_ADDFUNCTION ) );
            }
        }
    }

    if ( xIface.is() )
        RemoveElement(xIface);

    implSetModified();
}


void SAL_CALL OXUndoEnvironment::modified( const EventObject& /*aEvent*/ )
{
    implSetModified();
}


void OXUndoEnvironment::AddSection(const Reference< report::XSection > & _xSection)
{
    OUndoEnvLock aLock(*this);
    try
    {
        uno::Reference<container::XChild> xChild = _xSection.get();
        m_pImpl->m_aSections.push_back(xChild);
        Reference< XInterface >  xInt(_xSection);
        AddElement(xInt);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}


void OXUndoEnvironment::RemoveSection(const Reference< report::XSection > & _xSection)
{
    OUndoEnvLock aLock(*this);
    try
    {
        uno::Reference<container::XChild> xChild(_xSection.get());
        m_pImpl->m_aSections.erase(::std::remove(m_pImpl->m_aSections.begin(),m_pImpl->m_aSections.end(),
            xChild), m_pImpl->m_aSections.end());
        Reference< XInterface >  xInt(_xSection);
        RemoveElement(xInt);
    }
    catch(uno::Exception&){}
}


void OXUndoEnvironment::switchListening( const Reference< XIndexAccess >& _rxContainer, bool _bStartListening )
{
    OSL_PRECOND( _rxContainer.is(), "OXUndoEnvironment::switchListening: invalid container!" );
    if ( !_rxContainer.is() )
        return;

    try
    {
        // also handle all children of this element
        Reference< XInterface > xInterface;
        sal_Int32 nCount = _rxContainer->getCount();
        for(sal_Int32 i = 0;i != nCount;++i)
        {
            xInterface.set(_rxContainer->getByIndex( i ),uno::UNO_QUERY);
            if ( _bStartListening )
                AddElement( xInterface );
            else
                RemoveElement( xInterface );
        }

        // be notified of any changes in the container elements
        Reference< XContainer > xSimpleContainer( _rxContainer, UNO_QUERY );
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
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}


void OXUndoEnvironment::switchListening( const Reference< XInterface >& _rxObject, bool _bStartListening )
{
    OSL_PRECOND( _rxObject.is(), "OXUndoEnvironment::switchListening: how should I listen at a NULL object?" );

    try
    {
        if ( !m_pImpl->m_bReadOnly )
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
    }
}


void OXUndoEnvironment::AddElement(const Reference< XInterface >& _rxElement )
{
    if ( !IsLocked() )
        m_pImpl->m_aFormatNormalizer.notifyElementInserted( _rxElement );

    // if it's a container, start listening at all elements
    Reference< XIndexAccess > xContainer( _rxElement, UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, true );

    switchListening( _rxElement, true );
}


void OXUndoEnvironment::RemoveElement(const Reference< XInterface >& _rxElement)
{
    uno::Reference<beans::XPropertySet> xProp(_rxElement,uno::UNO_QUERY);
    if (!m_pImpl->m_aPropertySetCache.empty())
        m_pImpl->m_aPropertySetCache.erase(xProp);
    switchListening( _rxElement, false );

    Reference< XIndexAccess > xContainer( _rxElement, UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, false );
}

void OXUndoEnvironment::SetUndoMode(bool _bUndo)
{
    m_pImpl->m_bIsUndo = _bUndo;
}

bool OXUndoEnvironment::IsUndoMode() const
{
    return m_pImpl->m_bIsUndo;
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
