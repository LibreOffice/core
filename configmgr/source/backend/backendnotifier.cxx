/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backendnotifier.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:17:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "backendnotifier.hxx"

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

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
    ListenerList::iterator aIter =  m_aListeners.find(aComponentName);
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
void BackendChangeNotifier::addListener(INodeDataListener * _xListener,  const ComponentRequest& _aRequest) CFG_NOTHROW()
{
    osl::MutexGuard aListGuard(m_aMutex);

    OSL_PRECOND(_xListener, "ERROR: trying to register a NULL listener");
    ComponentListener aComponentListener(_xListener, _aRequest.getOptions());

    const rtl::OUString aComponentName = _aRequest.getComponentName().toString() ;

    //Check if we have a Listener registered for that Component
    ListenerList::iterator aIter;
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

void BackendChangeNotifier::removeListener(INodeDataListener * _xListener, const ComponentRequest& _aRequest) CFG_NOTHROW()
{
    osl::MutexGuard aListGuard(m_aMutex);
    OSL_PRECOND(!m_aListeners.empty(),
        "BackendChangeNotifier:Cannot Remove Listener, no Listeners Registered");
    OSL_PRECOND(_xListener, "ERROR: trying to remove a NULL listener");

    ListenerList::iterator aIter;
    rtl::OUString aComponentName = _aRequest.getComponentName().toString() ;

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
        ComponentRequest aRequest( configuration::makeName
                                 (_aComponent,configuration::Name::NoValidate()),
                                 (*aIter).m_aOptions );

        (*aIter).m_aListener->dataChanged(aRequest);
    }

}
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr
