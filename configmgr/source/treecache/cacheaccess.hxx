/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cacheaccess.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:34:50 $
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

#ifndef CONFIGMGR_CACHEACCESS_HXX
#define CONFIGMGR_CACHEACCESS_HXX

#ifndef CONFIGMGR_CACHEDATA_HXX
#include "cachedata.hxx"
#endif
#ifndef CONFIGMGR_TIMESTAMP_HXX
#include "timestamp.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CONFIGMGR_UTILITY_HXX_
#include <utility.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    using ::rtl::OUString;

    class ConfigChangeBroadcastHelper;
    namespace backend
    {
        class CacheController;
    }
//-----------------------------------------------------------------------------

    class CacheClientAccess : public configmgr::SimpleReferenceObject, Noncopyable
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

        /// return TRUE if there is no data (left) in this object's cache data
        bool isEmpty();

        // attach a module with a given name
        void attachModule(data::TreeAddress _aLocation, CacheLine::Name const & _aModule);
        /// check if the given module exists already (and is not empty)
        bool hasModule(const CacheLine::Path& _aLocation);
        /// checks if the given module exists and has defaults available
        bool hasModuleDefaults(CacheLine::Path const & _aLocation);

        /// retrieve the subtree at _aPath (maybe if it has the requested defaults) and clientAcquire() it
        data::NodeAddress acquireNode(CacheLine::Path const& _aPath);

        /** add or merge the given subtree at the given location,
            return <TRUE/> if the tree has defaults then
        */
        bool insertDefaults( backend::NodeInstance const & _aDefaultData ) CFG_UNO_THROW_RTE(  );

        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        oslInterlockedCount releaseNode( CacheLine::Path const& _aPath );

        /// retrieve the given subtree without changing its ref count
        data::NodeAddress   findInnerNode(CacheLine::Path const& _aPath );

        /// merge the given change list into this tree - reflects old data to _aUpdate
        void applyUpdate(backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( );
    };


////////////////////////////////////////////////////////////////////////////////

    class CacheLoadingAccess : public configmgr::SimpleReferenceObject, Noncopyable
    {
    public:
        typedef std::vector< CacheLineRef >  DisposeList;
        friend class backend::CacheController;
    private:
        friend class CacheDisposeScheduler;
        typedef std::map< CacheLine::Name, TimeStamp > DeadModuleList;

        ExtendedCacheData   m_aData;
        DeadModuleList      m_aDeadModules;         /// list of nodes which are registered for throwing away
    public:
        explicit
        CacheLoadingAccess();
        ~CacheLoadingAccess();

        /// gets a tree address for the given module if it exists
        data::TreeAddress getTreeAddress(CacheLine::Name const & _aModule);

        /// return TRUE if there is no data (left) in this object's cache data
        bool isEmpty();

        // create a module with a given name
        void createModule(CacheLine::Name const & _aModule);
        /// check if the given module exists already (and is not empty)
        bool hasModule(CacheLine::Name const & _aLocation);
        /// retrieve the subtree at aComponentName and clientAcquire() it, true if succeeded
        bool acquireModule(CacheLine::Name const & _aModule);

        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        oslInterlockedCount releaseModule( CacheLine::Name const & _aModule );

        /// retrieve the given subtree without changing its ref count
        data::NodeAddress   findNode(CacheLine::Path const& _aPath );

        /// merge the given change list into this tree - reflects old data to _aUpdate
        void applyUpdate(backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( );

        /// collect the modules that can be disposed now (i.e. released after _rLimitReleaseTime)
        TimeStamp collectDisposeList(CacheLoadingAccess::DisposeList & _rList,
                                        TimeStamp const & _aLimitTime,
                                        TimeInterval const & _aDelay);

        /// clear the contained tree, return all remaining modules
        void clearData( DisposeList& _rDisposeList) CFG_NOTHROW();

    // stuff that is particular for CacheLoadingAccess
        /** add the given subtree at the given location,
            return the tree that is then pertinent and clientAcquire() it once
        */
        data::TreeAddress addComponentData( backend::ComponentInstance const & _aComponentInstance,
                                            bool _bIncludesDefaults
                                           ) CFG_UNO_THROW_RTE();

        /// merge the given change list into the pending change list of this tree
        void addChangesToPending( backend::ConstUpdateInstance const& _anUpdate ) CFG_UNO_THROW_RTE(  );
        /// retrieve accumulated pending changes
        std::auto_ptr<SubtreeChange> releasePendingChanges(CacheLine::Name const& _aModule);

        /// find the modules having pending changes
        bool findPendingChangedModules( ExtendedCacheData::PendingModuleList & _rPendingList );
    };


////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

