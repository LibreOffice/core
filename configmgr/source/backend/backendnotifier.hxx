/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backendnotifier.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:23:54 $
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
#ifndef CONFIGMGR_BACKEND_BACKENDNOTIFIER_HXX
#define CONFIGMGR_BACKEND_BACKENDNOTIFIER_HXX

#ifndef CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX
#include "mergeddataprovider.hxx"
#endif // CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackend.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESNOTIFIER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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
    typedef struct ComponentListener
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
    } aComponentListener;

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
    typedef ::cppu::WeakImplHelper1<backenduno::XBackendChangesListener> BackendListener_Base;
    /** Interface providing a multicasting service for changes in the backend
     */
    class BackendChangeNotifier :public BackendListener_Base
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
        void addListener(INodeDataListener * _xListener, const ComponentRequest& _aRequest) CFG_NOTHROW();
        /// unregister a listener previously registered
        void removeListener(INodeDataListener * _xListener, const ComponentRequest& _aRequest) CFG_NOTHROW();
    private:
        typedef std::map<rtl::OUString, ComponentNotifier> ListenerList;
        osl::Mutex   m_aMutex;
        ListenerList m_aListeners;

        /** Backend being accessed */
        uno::Reference<backenduno::XBackendChangesNotifier> m_aBackend ;
    };
// ---------------------------------------------------------------------------
 } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

