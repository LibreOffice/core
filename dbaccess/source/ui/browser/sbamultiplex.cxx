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

#include <sbamultiplex.hxx>
using namespace dbaui;

// the listener multiplexers

// XStatusListener
SbaXStatusMultiplexer::SbaXStatusMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXStatusMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::frame::XStatusListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::frame::XStatusListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXStatusMultiplexer::disposing(const css::lang::EventObject& )
{
}


void SAL_CALL SbaXStatusMultiplexer::statusChanged(const css::frame::FeatureStateEvent& e)
{
    m_aLastKnownStatus = e;
    m_aLastKnownStatus.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt( *this );
    while ( aIt.hasMoreElements() )
        aIt.next()->statusChanged( m_aLastKnownStatus );
}

// LoadListener
SbaXLoadMultiplexer::SbaXLoadMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXLoadMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::form::XLoadListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::form::XLoadListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXLoadMultiplexer::disposing(const css::lang::EventObject& )
{
}

void SAL_CALL SbaXLoadMultiplexer::loaded(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->loaded(aMulti);
}
void SAL_CALL SbaXLoadMultiplexer::unloaded(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->unloaded(aMulti);
}

void SAL_CALL SbaXLoadMultiplexer::unloading(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->unloading(aMulti);
}

void SAL_CALL SbaXLoadMultiplexer::reloading(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->reloading(aMulti);
}

void SAL_CALL SbaXLoadMultiplexer::reloaded(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->reloaded(aMulti);
}


// css::sdbc::XRowSetListener
SbaXRowSetMultiplexer::SbaXRowSetMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXRowSetMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::sdbc::XRowSetListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::sdbc::XRowSetListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXRowSetMultiplexer::disposing(const css::lang::EventObject& )
{
}

void SAL_CALL SbaXRowSetMultiplexer::cursorMoved(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->cursorMoved(aMulti);
}

void SAL_CALL SbaXRowSetMultiplexer::rowChanged(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->rowChanged(aMulti);
}

void SAL_CALL SbaXRowSetMultiplexer::rowSetChanged(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->rowSetChanged(aMulti);
}

// css::sdb::XRowSetApproveListener
SbaXRowSetApproveMultiplexer::SbaXRowSetApproveMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXRowSetApproveMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::sdb::XRowSetApproveListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::sdb::XRowSetApproveListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXRowSetApproveMultiplexer::disposing(const css::lang::EventObject& )
{
}

sal_Bool SAL_CALL SbaXRowSetApproveMultiplexer::approveCursorMove(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    bool bResult = true;
    while (bResult && aIt.hasMoreElements())
        bResult = aIt.next()->approveCursorMove(aMulti);
    return bResult;
}

sal_Bool SAL_CALL SbaXRowSetApproveMultiplexer::approveRowChange(const css::sdb::RowChangeEvent& e)
{
    css::sdb::RowChangeEvent aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    bool bResult = true;
    while (bResult && aIt.hasMoreElements())
        bResult = aIt.next()->approveRowChange(aMulti);
    return bResult;
}

sal_Bool SAL_CALL SbaXRowSetApproveMultiplexer::approveRowSetChange(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    bool bResult = true;
    while (bResult && aIt.hasMoreElements())
        bResult = aIt.next()->approveRowSetChange(aMulti);
    return bResult;
}

// css::sdb::XSQLErrorListener
SbaXSQLErrorMultiplexer::SbaXSQLErrorMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXSQLErrorMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::sdb::XSQLErrorListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::sdb::XSQLErrorListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXSQLErrorMultiplexer::disposing(const css::lang::EventObject& )
{
}

void SAL_CALL SbaXSQLErrorMultiplexer::errorOccured(const css::sdb::SQLErrorEvent& e)
{
    css::sdb::SQLErrorEvent aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->errorOccured(aMulti);
}

// css::form::XDatabaseParameterListener
SbaXParameterMultiplexer::SbaXParameterMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXParameterMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::form::XDatabaseParameterListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::form::XDatabaseParameterListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXParameterMultiplexer::disposing(const css::lang::EventObject& )
{
}

sal_Bool SAL_CALL SbaXParameterMultiplexer::approveParameter(const css::form::DatabaseParameterEvent& e)
{
    css::form::DatabaseParameterEvent aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    bool bResult = true;
    while (bResult && aIt.hasMoreElements())
        bResult = aIt.next()->approveParameter(aMulti);
    return bResult;
}

// css::form::XSubmitListener
SbaXSubmitMultiplexer::SbaXSubmitMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXSubmitMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::form::XSubmitListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::form::XSubmitListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXSubmitMultiplexer::disposing(const css::lang::EventObject& )
{
}



sal_Bool SAL_CALL SbaXSubmitMultiplexer::approveSubmit(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    bool bResult = true;
    while (bResult && aIt.hasMoreElements())
        bResult = aIt.next()->approveSubmit(aMulti);
    return bResult;
}

// css::form::XResetListener
SbaXResetMultiplexer::SbaXResetMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXResetMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::form::XResetListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::form::XResetListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXResetMultiplexer::disposing(const css::lang::EventObject& )
{
}


sal_Bool SAL_CALL SbaXResetMultiplexer::approveReset(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    bool bResult = true;
    while (bResult && aIt.hasMoreElements())
        bResult = aIt.next()->approveReset(aMulti);
    return bResult;
}

void SAL_CALL SbaXResetMultiplexer::resetted(const css::lang::EventObject& e)
{
    css::lang::EventObject aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->resetted(aMulti);
}

// css::beans::XPropertyChangeListener
SbaXPropertyChangeMultiplexer::SbaXPropertyChangeMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex)
    :OSbaWeakSubObject(rSource)
    ,m_aListeners(rMutex)
{
}

css::uno::Any  SAL_CALL SbaXPropertyChangeMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::beans::XPropertyChangeListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::beans::XPropertyChangeListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXPropertyChangeMultiplexer::disposing(const css::lang::EventObject& )
{
}
void SAL_CALL SbaXPropertyChangeMultiplexer::propertyChange(const css::beans::PropertyChangeEvent& e)
{
    ::comphelper::OInterfaceContainerHelper3<XPropertyChangeListener>* pListeners = m_aListeners.getContainer(e.PropertyName);
    if (pListeners)
        Notify(*pListeners, e);

    /* do the notification for the unspecialized listeners, too */
    pListeners = m_aListeners.getContainer(OUString());
    if (pListeners)
        Notify(*pListeners, e);
}

void SbaXPropertyChangeMultiplexer::addInterface(const OUString& rName,
    const css::uno::Reference< css::beans::XPropertyChangeListener > & rListener)
{
    m_aListeners.addInterface(rName, rListener);
}

void SbaXPropertyChangeMultiplexer::removeInterface(const OUString& rName,
    const css::uno::Reference< css::beans::XPropertyChangeListener > & rListener)
{
    m_aListeners.removeInterface(rName, rListener);
}

void SbaXPropertyChangeMultiplexer::disposeAndClear()
{
    css::lang::EventObject aEvt(m_rParent);
    m_aListeners.disposeAndClear(aEvt);
}

sal_Int32 SbaXPropertyChangeMultiplexer::getOverallLen() const
{
    sal_Int32 nLen = 0;
    const std::vector< OUString > aContained = m_aListeners.getContainedTypes();
    for ( OUString const & s : aContained)
    {
        ::comphelper::OInterfaceContainerHelper3<XPropertyChangeListener>* pListeners = m_aListeners.getContainer(s);
        if (!pListeners)
            continue;
        nLen += pListeners->getLength();
    }
    return nLen;
}

void SbaXPropertyChangeMultiplexer::Notify(::comphelper::OInterfaceContainerHelper3<XPropertyChangeListener>& rListeners, const css::beans::PropertyChangeEvent& e)
{
    css::beans::PropertyChangeEvent aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper3 aIt(rListeners);
    while (aIt.hasMoreElements())
        aIt.next()->propertyChange(aMulti);
}

// css::beans::XVetoableChangeListener
SbaXVetoableChangeMultiplexer::SbaXVetoableChangeMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex)
    :OSbaWeakSubObject(rSource)
    ,m_aListeners(rMutex)
{
}

css::uno::Any  SAL_CALL SbaXVetoableChangeMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::beans::XVetoableChangeListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::beans::XVetoableChangeListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXVetoableChangeMultiplexer::disposing(const css::lang::EventObject& )
{
}
void SAL_CALL SbaXVetoableChangeMultiplexer::vetoableChange(const css::beans::PropertyChangeEvent& e)
{
    ::comphelper::OInterfaceContainerHelper2* pListeners = m_aListeners.getContainer(e.PropertyName);
    if (pListeners)
        Notify(*pListeners, e);

    /* do the notification for the unspecialized listeners, too */
    pListeners = m_aListeners.getContainer(OUString());
    if (pListeners)
        Notify(*pListeners, e);
}

void SbaXVetoableChangeMultiplexer::addInterface(const OUString& rName,
    const css::uno::Reference< css::uno::XInterface > & rListener)
{
    m_aListeners.addInterface(rName, rListener);
}

void SbaXVetoableChangeMultiplexer::removeInterface(const OUString& rName,
    const css::uno::Reference< css::uno::XInterface > & rListener)
{
    m_aListeners.removeInterface(rName, rListener);
}

void SbaXVetoableChangeMultiplexer::disposeAndClear()
{
    css::lang::EventObject aEvt(m_rParent);
    m_aListeners.disposeAndClear(aEvt);
}

sal_Int32 SbaXVetoableChangeMultiplexer::getOverallLen() const
{
    sal_Int32 nLen = 0;
    const std::vector< OUString > aContained = m_aListeners.getContainedTypes();
    for ( OUString const & s : aContained)
    {
        ::comphelper::OInterfaceContainerHelper2* pListeners = m_aListeners.getContainer(s);
        if (!pListeners)
            continue;
        nLen += pListeners->getLength();
    }
    return nLen;
}

void SbaXVetoableChangeMultiplexer::Notify(::comphelper::OInterfaceContainerHelper2& rListeners, const css::beans::PropertyChangeEvent& e)
{
    css::beans::PropertyChangeEvent aMulti(e);
    aMulti.Source = &m_rParent;
    ::comphelper::OInterfaceIteratorHelper2 aIt(rListeners);
    while (aIt.hasMoreElements())
        static_cast< css::beans::XVetoableChangeListener*>(aIt.next())->vetoableChange(aMulti);
}

// css::beans::XPropertiesChangeListener
SbaXPropertiesChangeMultiplexer::SbaXPropertiesChangeMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)
    :OSbaWeakSubObject(rSource)
    ,OInterfaceContainerHelper3(_rMutex)
{
}

css::uno::Any  SAL_CALL SbaXPropertiesChangeMultiplexer::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = OSbaWeakSubObject::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast< css::beans::XPropertiesChangeListener* >(this),
            static_cast< css::lang::XEventListener* >(static_cast< css::beans::XPropertiesChangeListener* >(this))
        );

    return aReturn;
}
void SAL_CALL SbaXPropertiesChangeMultiplexer::disposing(const css::lang::EventObject& )
{
}

void SbaXPropertiesChangeMultiplexer::propertiesChange(const css::uno::Sequence< css::beans::PropertyChangeEvent>& aEvts)
{
    // the SbaXPropertiesChangeMultiplexer doesn't care about the property names a listener logs on for, it simply
    // forwards _all_ changes to _all_ listeners

    css::uno::Sequence< css::beans::PropertyChangeEvent> aMulti(aEvts);
    for (css::beans::PropertyChangeEvent & rEvent : asNonConstRange(aMulti))
        rEvent.Source = &m_rParent;

    ::comphelper::OInterfaceIteratorHelper3 aIt(*this);
    while (aIt.hasMoreElements())
        aIt.next()->propertiesChange(aMulti);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
