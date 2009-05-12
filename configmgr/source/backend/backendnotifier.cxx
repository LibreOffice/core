/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backendnotifier.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "backendnotifier.hxx"

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#include "configpath.hxx"

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {


//----------------------------------------------------------------------------
BackendChangeNotifier::BackendChangeNotifier(const uno::Reference<backenduno::XBackend>& _xBackend)
: m_aMutex()
 ,m_aListeners()
 ,m_aBackend(_xBackend, uno::UNO_QUERY)
{

}
// ---------------------------------------------------------------------------

BackendChangeNotifier::~BackendChangeNotifier()
{
    m_aListeners.clear();
}
// ---------------------------------------------------------------------------
void SAL_CALL BackendChangeNotifier::componentDataChanged(const backenduno::ComponentChangeEvent& _aEvent)
   throw (::com::sun::star::uno::RuntimeException)
{
    rtl::OUString aComponentName = _aEvent.Component;
    std::map<rtl::OUString, ComponentNotifier>::iterator aIter =    m_aListeners.find(aComponentName);
    if(aIter != m_aListeners.end())
    {
        aIter->second.notifyListeners(aComponentName);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL BackendChangeNotifier::disposing( lang::EventObject const & /*rSource*/ )
throw (uno::RuntimeException)
{
    osl::MutexGuard aListGuard(m_aMutex);
    if (m_aBackend.is())
    {
        m_aBackend.clear();
    }
    m_aListeners.clear();
}
// -----------------------------------------------------------------------------
void BackendChangeNotifier::addListener(INodeDataListener * _xListener,  const ComponentRequest& _aRequest) SAL_THROW(())
{
    osl::MutexGuard aListGuard(m_aMutex);

    OSL_PRECOND(_xListener, "ERROR: trying to register a NULL listener");
    ComponentListener aComponentListener(_xListener, _aRequest.getOptions());

    const rtl::OUString aComponentName = _aRequest.getComponentName();

    //Check if we have a Listener registered for that Component
    std::map<rtl::OUString, ComponentNotifier>::iterator aIter;
    aIter = m_aListeners.find(aComponentName);
    if (aIter == m_aListeners.end())
    {
        ComponentNotifier aComponentNotifier;
        aComponentNotifier.addListenerToList(aComponentListener);
        m_aListeners[aComponentName] = aComponentNotifier;

        //Now need to register Listener with MultiStratumBackend for that Component
        if (m_aBackend.is())
        {
            m_aBackend->addChangesListener(this, aComponentName);
        }
    }
    else
    {
        //Add Listener and Option to ComponentNotifier list
        aIter->second.addListenerToList(aComponentListener);

    }

}
// ---------------------------------------------------------------------------

void BackendChangeNotifier::removeListener(INodeDataListener * _xListener, const ComponentRequest& _aRequest) SAL_THROW(())
{
    osl::MutexGuard aListGuard(m_aMutex);
    OSL_PRECOND(!m_aListeners.empty(),
        "BackendChangeNotifier:Cannot Remove Listener, no Listeners Registered");
    OSL_PRECOND(_xListener, "ERROR: trying to remove a NULL listener");

    std::map<rtl::OUString, ComponentNotifier>::iterator aIter;
    rtl::OUString aComponentName = _aRequest.getComponentName();

    aIter = m_aListeners.find(aComponentName);
    if (aIter == m_aListeners.end())
    {
        OSL_TRACE("BackendChangeNotifier: removeListener: no listener registered for component %s",
                   aComponentName.getStr());
    }
    else
    {
        ComponentListener aComponentListener(_xListener, _aRequest.getOptions());
        aIter->second.removeListenerFromList(aComponentListener);
        if (aIter->second.isListEmpty())
        {
            m_aListeners.erase(aIter);
            //Need all to de-register listeners from the lower layers
            if (m_aBackend.is())
            {
                m_aBackend->removeChangesListener(this, aComponentName);
            }
        }
    }

}
// ---------------------------------------------------------------------------
ComponentNotifier::ComponentNotifier():m_aListenerList()
{
}
// ---------------------------------------------------------------------------


void ComponentNotifier::addListenerToList(const ComponentListener& _aListener)
{
    m_aListenerList.push_back(_aListener);
    //REM TO REMOVE
    ComponentListener aListener = m_aListenerList.front();

}
// ---------------------------------------------------------------------------
void ComponentNotifier::removeListenerFromList(const ComponentListener& _aListener)
{
    OSL_PRECOND(!m_aListenerList.empty(),
        "ComponentNotifier:Cannot Remove Listener, no Listeners Registered");

    std::list<ComponentListener>::iterator aIter;

    //Remove first instance that matches in the list
    aIter = std::find(m_aListenerList.begin(), m_aListenerList.end(),_aListener);
    if(aIter != m_aListenerList.end())
    {
        m_aListenerList.erase(aIter);
    }
}
// ---------------------------------------------------------------------------
void ComponentNotifier::notifyListeners(const rtl::OUString& _aComponent)
{

    for( std::list<ComponentListener>::iterator aIter = m_aListenerList.begin();
        aIter != m_aListenerList.end(); aIter++)
    {
        ComponentRequest aRequest(_aComponent, (*aIter).m_aOptions);

        (*aIter).m_aListener->dataChanged(aRequest);
    }

}
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr
