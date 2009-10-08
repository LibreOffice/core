/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cacheaccess.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_CACHEACCESS_HXX
#define CONFIGMGR_CACHEACCESS_HXX

#include "cachedata.hxx"
#include "timestamp.hxx"
#include "utility.hxx"
#include <boost/utility.hpp>
#include <osl/mutex.hxx>
#ifndef _CONFIGMGR_UTILITY_HXX_
#include <utility.hxx>
#endif
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    class ConfigChangeBroadcastHelper;
    namespace backend
    {
        class CacheController;
    }
//-----------------------------------------------------------------------------

    class CacheClientAccess: private boost::noncopyable, public salhelper::SimpleReferenceObject
    {
    private:
        CacheData   m_aData;

        ConfigChangeBroadcastHelper* m_pBroadcastHelper;
    public:
        explicit
        CacheClientAccess(ConfigChangeBroadcastHelper* _pBroadcastHelper);

        ~CacheClientAccess();

        /// gets a helper to broadcast changes for
        ConfigChangeBroadcastHelper * getBroadcaster() const
        { return m_pBroadcastHelper; }

        /// removes an existing broadcast helper
        ConfigChangeBroadcastHelper * releaseBroadcaster();

        // attach a module with a given name
        void attachModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aModule);
        /// check if the given module exists already (and is not empty)
        bool hasModule(const configuration::AbsolutePath& _aLocation);
        /// checks if the given module exists and has defaults available
        bool hasModuleDefaults(configuration::AbsolutePath const & _aLocation);

        /// retrieve the subtree at _aPath (maybe if it has the requested defaults) and clientAcquire() it
        sharable::Node * acquireNode(configuration::AbsolutePath const& _aPath);

        /** add or merge the given subtree at the given location,
            return <TRUE/> if the tree has defaults then
        */
        bool insertDefaults( backend::NodeInstance const & _aDefaultData ) SAL_THROW((com::sun::star::uno::RuntimeException));

        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        oslInterlockedCount releaseNode( configuration::AbsolutePath const& _aPath );

        /// retrieve the given subtree without changing its ref count
        sharable::Node *    findInnerNode(configuration::AbsolutePath const& _aPath );

        /// merge the given change list into this tree - reflects old data to _aUpdate
        void applyUpdate(backend::UpdateInstance & _aUpdate) SAL_THROW((com::sun::star::uno::RuntimeException));
    };


////////////////////////////////////////////////////////////////////////////////

    class CacheLoadingAccess: private boost::noncopyable, public salhelper::SimpleReferenceObject
    {
    public:
        friend class backend::CacheController;
    private:
        friend class CacheDisposeScheduler;

        ExtendedCacheData   m_aData;
        std::map< rtl::OUString, TimeStamp >        m_aDeadModules;         /// list of nodes which are registered for throwing away
    public:
        explicit
        CacheLoadingAccess();
        ~CacheLoadingAccess();

        /// gets a tree address for the given module if it exists
        sharable::TreeFragment * getTreeAddress(rtl::OUString const & _aModule);

        /// return TRUE if there is no data (left) in this object's cache data
        bool isEmpty();

        // create a module with a given name
        void createModule(rtl::OUString const & _aModule);
        /// check if the given module exists already (and is not empty)
        bool hasModule(rtl::OUString const & _aLocation);
        /// retrieve the subtree at aComponentName and clientAcquire() it, true if succeeded
        bool acquireModule(rtl::OUString const & _aModule);

        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        oslInterlockedCount releaseModule( rtl::OUString const & _aModule );

        /// collect the modules that can be disposed now (i.e. released after _rLimitReleaseTime)
        TimeStamp collectDisposeList(std::vector< rtl::Reference<CacheLine> > & _rList,
                                        TimeStamp const & _aLimitTime,
                                        TimeInterval const & _aDelay);

        /// clear the contained tree, return all remaining modules
        void clearData( std::vector< rtl::Reference<CacheLine> >& _rDisposeList) SAL_THROW(());

    // stuff that is particular for CacheLoadingAccess
        /** add the given subtree at the given location,
            return the tree that is then pertinent and clientAcquire() it once
        */
        sharable::TreeFragment * addComponentData( backend::ComponentInstance const & _aComponentInstance,
                                            bool _bIncludesDefaults
                                           ) SAL_THROW((com::sun::star::uno::RuntimeException));

        /// merge the given change list into the pending change list of this tree
        void addChangesToPending( backend::ConstUpdateInstance const& _anUpdate ) SAL_THROW((com::sun::star::uno::RuntimeException));
        /// retrieve accumulated pending changes
        std::auto_ptr<SubtreeChange> releasePendingChanges(rtl::OUString const& _aModule);

        /// find the modules having pending changes
        bool findPendingChangedModules( std::vector< rtl::OUString > & _rPendingList );
    };


////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

