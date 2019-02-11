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

#include <sal/config.h>

#include <map>

#include <ReportControllerObserver.hxx>
#include <ReportController.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <FormattedFieldBeautifier.hxx>

#include <svx/unopage.hxx>

// DBG_*
#include <tools/debug.hxx>
// DBG_UNHANDLED_EXCEPTION
#include <tools/diagnose_ex.h>

namespace rptui
{

    using namespace ::com::sun::star;

typedef std::map<OUString, bool> AllProperties;
typedef std::map<uno::Reference< beans::XPropertySet >, AllProperties> PropertySetInfoCache;

class OXReportControllerObserverImpl
{
public:
    ::std::vector< uno::Reference< container::XChild> > m_aSections;
    ::osl::Mutex                                        m_aMutex;
    oslInterlockedCount                                 m_nLocks;

    explicit OXReportControllerObserverImpl();
    OXReportControllerObserverImpl(const OXReportControllerObserverImpl&) = delete;
    OXReportControllerObserverImpl& operator=(const OXReportControllerObserverImpl&) = delete;
};


    OXReportControllerObserverImpl::OXReportControllerObserverImpl()
            :m_nLocks(0)
    {
    }


    OXReportControllerObserver::OXReportControllerObserver(const OReportController& _rController)
            :m_pImpl(new OXReportControllerObserverImpl )
            ,m_aFormattedFieldBeautifier(_rController)
            ,m_aFixedTextColor(_rController)
    {

        Application::AddEventListener(LINK( this, OXReportControllerObserver, SettingsChanged ) );
    }

    OXReportControllerObserver::~OXReportControllerObserver()
    {
        Application::RemoveEventListener(LINK( this, OXReportControllerObserver, SettingsChanged ) );
    }


    IMPL_LINK(OXReportControllerObserver, SettingsChanged, VclSimpleEvent&, _rEvt, void)
    {
            VclEventId nEvent = _rEvt.GetId();

            if (nEvent == VclEventId::ApplicationDataChanged )
            {
                DataChangedEvent* pData = static_cast<DataChangedEvent*>(static_cast<VclWindowEvent&>(_rEvt).GetData());
                if ( pData && ((( pData->GetType() == DataChangedEventType::SETTINGS  )   ||
                                ( pData->GetType() == DataChangedEventType::DISPLAY   ))  &&
                               ( pData->GetFlags() & AllSettingsFlags::STYLE     )))
                {
                    OEnvLock aLock(*this);

                    // send all Section Objects a 'tingle'
                    // maybe they need a change in format, color, etc
                    for (const uno::Reference<container::XChild>& xChild : m_pImpl->m_aSections)
                    {
                        if (xChild.is())
                        {
                            uno::Reference<report::XSection> xSection(xChild, uno::UNO_QUERY);
                            if (xSection.is())
                            {
                                const sal_Int32 nCount = xSection->getCount();
                                for (sal_Int32 i = 0; i < nCount; ++i)
                                {
                                    const uno::Any aObj = xSection->getByIndex(i);
                                    uno::Reference < report::XReportComponent > xReportComponent(aObj, uno::UNO_QUERY);
                                    if (xReportComponent.is())
                                    {
                                        m_aFormattedFieldBeautifier.handle(xReportComponent);
                                        m_aFixedTextColor.handle(xReportComponent);
                                    }
                                }
                            }
                        }
                    }
                }
            }
    }

    // XEventListener
    void SAL_CALL OXReportControllerObserver::disposing(const lang::EventObject& e)
    {
        // check if it's an object we have cached information about
        uno::Reference< beans::XPropertySet > xSourceSet(e.Source, uno::UNO_QUERY);
        if ( xSourceSet.is() )
        {
            uno::Reference< report::XSection> xSection(xSourceSet,uno::UNO_QUERY);
            if ( xSection.is() )
                RemoveSection(xSection);
            else
                RemoveElement(xSourceSet);
        }
    }

    void OXReportControllerObserver::Clear()
    {
        OEnvLock aLock(*this);
        m_pImpl->m_aSections.clear();
    }

    // XPropertyChangeListener
    void SAL_CALL OXReportControllerObserver::propertyChange(const beans::PropertyChangeEvent& _rEvent)
    {
        ::osl::ClearableMutexGuard aGuard( m_pImpl->m_aMutex );

        if ( m_pImpl->m_nLocks != 0 )
            return;

        m_aFormattedFieldBeautifier.notifyPropertyChange(_rEvent);
        m_aFixedTextColor.notifyPropertyChange(_rEvent);
    }


void OXReportControllerObserver::Lock()
{
    OSL_ENSURE(m_refCount,"Illegal call to dead object!");
    osl_atomic_increment( &m_pImpl->m_nLocks );
}

void OXReportControllerObserver::UnLock()
{
    OSL_ENSURE(m_refCount,"Illegal call to dead object!");

    osl_atomic_decrement( &m_pImpl->m_nLocks );
}

void OXReportControllerObserver::AddSection(const uno::Reference< report::XSection > & _xSection)
{
    OEnvLock aLock(*this);
    try
    {
        uno::Reference<container::XChild> xChild = _xSection.get();
        m_pImpl->m_aSections.push_back(xChild);
        uno::Reference< uno::XInterface >  xInt(_xSection);
        AddElement(xInt);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}


void OXReportControllerObserver::RemoveSection(const uno::Reference< report::XSection > & _xSection)
{
    OEnvLock aLock(*this);
    try
    {
        uno::Reference<container::XChild> xChild(_xSection.get());
        m_pImpl->m_aSections.erase(::std::remove(m_pImpl->m_aSections.begin(),m_pImpl->m_aSections.end(),
            xChild), m_pImpl->m_aSections.end());
        uno::Reference< uno::XInterface >  xInt(_xSection);
        RemoveElement(xInt);
    }
    catch(uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}


void OXReportControllerObserver::switchListening( const uno::Reference< container::XIndexAccess >& _rxContainer, bool _bStartListening )
{
    OSL_PRECOND( _rxContainer.is(), "OXReportControllerObserver::switchListening: invalid container!" );
    if ( !_rxContainer.is() )
        return;

    try
    {
        // also handle all children of this element
        uno::Reference< uno::XInterface > xInterface;
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
        uno::Reference< container::XContainer > xSimpleContainer( _rxContainer, uno::UNO_QUERY );
        if ( xSimpleContainer.is() )
        {
            if ( _bStartListening )
                xSimpleContainer->addContainerListener( this );
            else
                xSimpleContainer->removeContainerListener( this );
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}


void OXReportControllerObserver::switchListening( const uno::Reference< uno::XInterface >& _rxObject, bool _bStartListening )
{
    OSL_PRECOND( _rxObject.is(), "OXReportControllerObserver::switchListening: how should I listen at a NULL object?" );

    try
    {
        uno::Reference< beans::XPropertySet > xProps( _rxObject, uno::UNO_QUERY );
        if ( xProps.is() )
        {
            if ( _bStartListening )
                xProps->addPropertyChangeListener( OUString(), this );
            else
                xProps->removePropertyChangeListener( OUString(), this );
        }

        uno::Reference< util::XModifyBroadcaster > xBroadcaster( _rxObject, uno::UNO_QUERY );
        if ( xBroadcaster.is() )
        {
            if ( _bStartListening )
                xBroadcaster->addModifyListener( this );
            else
                xBroadcaster->removeModifyListener( this );
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}


void SAL_CALL OXReportControllerObserver::modified( const lang::EventObject& /*aEvent*/ )
{
}


void OXReportControllerObserver::AddElement(const uno::Reference< uno::XInterface >& _rxElement )
{
    m_aFormattedFieldBeautifier.notifyElementInserted(_rxElement);
    m_aFixedTextColor.notifyElementInserted(_rxElement);

    // if it's a container, start listening at all elements
    uno::Reference< container::XIndexAccess > xContainer( _rxElement, uno::UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, true );

    switchListening( _rxElement, true );
}


void OXReportControllerObserver::RemoveElement(const uno::Reference< uno::XInterface >& _rxElement)
{
    switchListening( _rxElement, false );

    uno::Reference< container::XIndexAccess > xContainer( _rxElement, uno::UNO_QUERY );
    if ( xContainer.is() )
        switchListening( xContainer, false );
}


// XContainerListener

void SAL_CALL OXReportControllerObserver::elementInserted(const container::ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    // new listener object
    uno::Reference< uno::XInterface >  xIface( evt.Element, uno::UNO_QUERY );
    if ( xIface.is() )
    {
        AddElement(xIface);
    }
}


void SAL_CALL OXReportControllerObserver::elementReplaced(const container::ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    uno::Reference< uno::XInterface >  xIface(evt.ReplacedElement,uno::UNO_QUERY);
    OSL_ENSURE(xIface.is(), "OXReportControllerObserver::elementReplaced: invalid container notification!");
    RemoveElement(xIface);

    xIface.set(evt.Element,uno::UNO_QUERY);
    AddElement(xIface);
}


void SAL_CALL OXReportControllerObserver::elementRemoved(const container::ContainerEvent& evt)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_pImpl->m_aMutex );

    uno::Reference< uno::XInterface >  xIface( evt.Element, uno::UNO_QUERY );
    if ( xIface.is() )
    {
        RemoveElement(xIface);
    }
}


} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
