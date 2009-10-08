/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backendnotifier.hxx,v $
 * $Revision: 1.4 $
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
#ifndef CONFIGMGR_BACKEND_BACKENDNOTIFIER_HXX
#define CONFIGMGR_BACKEND_BACKENDNOTIFIER_HXX

#include "mergeddataprovider.hxx"
#include <com/sun/star/configuration/backend/XBackend.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

#ifndef INCLUDED_LIST
#include <list>
#define INCLUDED_LIST
#endif


namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
    namespace uno       = ::com::sun::star::uno;
    namespace lang      = ::com::sun::star::lang;
    namespace backenduno = ::com::sun::star::configuration::backend;
// ---------------------------------------------------------------------------
    struct ComponentListener
    {
        explicit
        ComponentListener(INodeDataListener * _xListener, RequestOptions _aOptions):
        m_aListener( _xListener),
        m_aOptions(_aOptions)
        {}

        bool operator==(const ComponentListener& _aListener)const
        {
            return ( (&m_aListener == &_aListener.m_aListener)&&
                     (compareRequestOptions(m_aOptions, _aListener.m_aOptions)== 0) );

        }
        INodeDataListener * m_aListener;
        RequestOptions m_aOptions;
    };

    /** Class used to store ComponentListener(listener and options)
    */
    class ComponentNotifier
    {
    public:
        ComponentNotifier();
        void addListenerToList(const ComponentListener& _aListener);
        void removeListenerFromList(const ComponentListener& _aListener);
        bool isListEmpty(){ return m_aListenerList.empty();}
        void notifyListeners(const rtl::OUString& _aComponent);
    private:
        std::list<ComponentListener> m_aListenerList;
    };
    // ---------------------------------------------------------------------------
    /** Interface providing a multicasting service for changes in the backend
     */
    class BackendChangeNotifier :public cppu::WeakImplHelper1<backenduno::XBackendChangesListener>
    {
    public:
        BackendChangeNotifier(const uno::Reference<backenduno::XBackend>& _xBackend);
        ~BackendChangeNotifier();

        virtual void SAL_CALL componentDataChanged(const backenduno::ComponentChangeEvent& aEvent)
        throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL disposing( lang::EventObject const & rSource )
            throw (uno::RuntimeException);
      // notification support.
        /// register a listener for observing changes to the cached data
        void addListener(INodeDataListener * _xListener, const ComponentRequest& _aRequest) SAL_THROW(());
        /// unregister a listener previously registered
        void removeListener(INodeDataListener * _xListener, const ComponentRequest& _aRequest) SAL_THROW(());
    private:
        osl::Mutex   m_aMutex;
        std::map<rtl::OUString, ComponentNotifier> m_aListeners;

        /** Backend being accessed */
        uno::Reference<backenduno::XBackendChangesNotifier> m_aBackend ;
    };
// ---------------------------------------------------------------------------
 } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

