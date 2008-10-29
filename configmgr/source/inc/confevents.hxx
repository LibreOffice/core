/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confevents.hxx,v $
 * $Revision: 1.10 $
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

#ifndef CONFIGMGR_API_EVENTS_HXX_
#define CONFIGMGR_API_EVENTS_HXX_

#include <hash_set>
#include <map>
#include <set>

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "configpath.hxx"
#include "utility.hxx"

namespace rtl { class OUString; }

namespace configmgr
{
    class Change;
    struct TreeChangeList;
    class TreeManager;

    namespace configuration { class AbsolutePath; }

    struct IConfigListener : public salhelper::SimpleReferenceObject
    {
        virtual void disposing(TreeManager * pSource) = 0;
    };
    struct INodeListener : IConfigListener
    {
        virtual void nodeChanged(Change const& aChange, configuration::AbsolutePath const& aPath, TreeManager * pSource) = 0;
        virtual void nodeDeleted(configuration::AbsolutePath const& aPath, TreeManager * pSource) = 0;
    };

    namespace internal
    {

    ////////////////////////////////////////////////////////////////////////
        template <class ListenerRef>
        class BroadcastImplHelper
        {
        public:
            osl::Mutex mutex;

        public:
            BroadcastImplHelper()
            {}

            ~BroadcastImplHelper()
            {
                OSL_ENSURE(m_aInterfaces.empty(), "Configuration Broadcaster was not disposed properly");
            }

        public:
            typedef std::set<ListenerRef> Interfaces;

        public:
            typename Interfaces::iterator addListener(ListenerRef aListener)
            {
                return m_aInterfaces.insert(aListener).first;
            }
            void removeListener(ListenerRef aListener)
            {
                m_aInterfaces.erase(aListener);
            }

            void disposing(TreeManager * pSource);

        public:
            typename Interfaces::const_iterator begin() const { return m_aInterfaces.begin(); }
            typename Interfaces::const_iterator end() const { return m_aInterfaces.end(); }

            typename Interfaces::const_iterator find(ListenerRef aListener) const   { return m_aInterfaces.find(aListener); }
            typename Interfaces::iterator findFull(ListenerRef aListener) { return m_aInterfaces.find(aListener); }
        private:
            Interfaces m_aInterfaces;

            // no implementation - not copyable
            BroadcastImplHelper(BroadcastImplHelper&);
            void operator=(BroadcastImplHelper&);
        };

        ////////////////////////////////////////////////////////////////////////
        template <class Listener>
        void BroadcastImplHelper<Listener>::disposing(TreeManager * pSource)
        {
            osl::ClearableMutexGuard aGuard(this->mutex);   // ensure that no notifications are running

            Interfaces aTargets;
            aTargets.swap(m_aInterfaces);

            aGuard.clear();
            for(typename Interfaces::iterator it = aTargets.begin(); it != aTargets.end(); )
            {
                typename Interfaces::iterator cur = it++;
                if (*cur)
                    (*cur)->disposing(pSource);
            }
        }


    /////////////////////////////////////////////////////////////////////////

        class NodeListenerInfo
        {
        public:
            typedef std::hash_set<configuration::AbsolutePath, configuration::Path::Hash, configuration::Path::Equiv> Pathes;

        public:
            NodeListenerInfo(rtl::Reference<INodeListener> const&   pListener)
                : m_pListener(pListener)
            {
            }

        // path handling
            Pathes const& pathList() const { return m_aPathes; }

            void addPath(configuration::AbsolutePath const& sPath) const { m_aPathes.insert(sPath); }
            void removePath(configuration::AbsolutePath const& sPath) const { m_aPathes.erase(sPath); }
            //void removeChildPathes(OUString const& sPath);

        // behave as pointer for use as a 'reference' class
            rtl::Reference<INodeListener> get() const { return m_pListener; }
            rtl::Reference<INodeListener> operator->() const { return get(); }
            INodeListener& operator*() const { return *m_pListener; }
        // needed to allow if (info) ...
            struct HasListener;
            operator HasListener const*() const { return reinterpret_cast<HasListener*>(m_pListener.get()); }

            bool operator < (NodeListenerInfo const& aInfo) const
            { return std::less<INodeListener*>()(m_pListener.get(), aInfo.m_pListener.get()); }

            bool operator == (NodeListenerInfo const& aInfo) const
            { return !!( m_pListener == aInfo.m_pListener); }

            bool operator > (NodeListenerInfo const& aInfo) const
            { return aInfo.operator < (*this); }
            bool operator >= (NodeListenerInfo const& aInfo) const
            { return !operator<(aInfo); }
            bool operator <= (NodeListenerInfo const& aInfo) const
            { return !operator>(aInfo); }

            bool operator != (NodeListenerInfo const& aInfo) const
            { return !operator==(aInfo); }

        private:
            rtl::Reference<INodeListener> m_pListener;
            mutable Pathes m_aPathes; // hack to be mutable even as set element
        };
    } // namespace

    /////////////////////////////////////////////////////////////////////////
    class ConfigChangeBroadcastHelper // broadcasts changes for a given set of options
    {
    public:
        ConfigChangeBroadcastHelper();
        ~ConfigChangeBroadcastHelper();

        void broadcast(TreeChangeList const& anUpdate, sal_Bool bError, TreeManager * pSource);

        void addListener(configuration::AbsolutePath const& aName, rtl::Reference<INodeListener> const& );
        void removeListener(rtl::Reference<INodeListener> const&);

        void dispose(TreeManager * pSource);

    private:
        void add(configuration::AbsolutePath const& aPath, rtl::Reference<INodeListener> const& pListener);
        void remove(rtl::Reference<INodeListener> const& pListener);

        void dispatch(Change const& rBaseChange, configuration::AbsolutePath const& sChangeLocation, sal_Bool _bError, TreeManager * pSource);
        void dispatch(TreeChangeList const& rList_, sal_Bool _bError, TreeManager * pSource);
        void disposing(TreeManager * pSource);

        void dispatchInner(rtl::Reference<INodeListener> const& pTarget, configuration::AbsolutePath const& sTargetPath, Change const& rBaseChange, configuration::AbsolutePath const& sChangeLocation, sal_Bool _bError, TreeManager * pSource);
        void dispatchOuter(rtl::Reference<INodeListener> const& pTarget, configuration::AbsolutePath const& sTargetPath, Change const& rBaseChange, configuration::AbsolutePath const& sChangeLocation, sal_Bool _bError, TreeManager * pSource);

        typedef std::multimap<configuration::AbsolutePath, internal::BroadcastImplHelper<internal::NodeListenerInfo>::Interfaces::iterator, configuration::Path::Before> PathMap;
        internal::BroadcastImplHelper<internal::NodeListenerInfo> m_aListeners;
        PathMap m_aPathMap;
    };
} // namespace

#endif // CONFIGMGR_API_EVENTS_HXX_



