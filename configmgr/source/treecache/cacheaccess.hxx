/*************************************************************************
 *
 *  $RCSfile: cacheaccess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    using ::rtl::OUString;

    class ConfigChangeBroadcastHelper;

//-----------------------------------------------------------------------------

    class CacheClientAccess : public salhelper::SimpleReferenceObject, Noncopyable
    {
    public:
        typedef CacheData   Data;
        typedef Data::Module       Module;
        typedef Data::ModuleRef    ModuleRef;
        typedef Data::ModuleName   ModuleName;
        typedef Data::Path         Path;
    private:
        osl::Mutex          m_aMutex;
        Data                m_aData;

        ConfigChangeBroadcastHelper* m_pBroadcastHelper;
    public:
        explicit
        CacheClientAccess(memory::HeapManager & _rHeapManager,
                          ConfigChangeBroadcastHelper* _pBroadcastHelper);

        ~CacheClientAccess();

        /// gets a helper to broadcast changes for
        ConfigChangeBroadcastHelper * getBroadcaster() const
        { return m_pBroadcastHelper; }

        /// removes an existing broadcast helper
        ConfigChangeBroadcastHelper * releaseBroadcaster();

        /// gets a data segment reference for the given path - creates if necessary
        memory::Segment * attachDataSegment(const memory::SegmentAddress & _aSegment, const Path& _aLocation);
        /// gets a data segment reference for the given path if exists
        memory::Segment * getDataSegment(const Path& _aLocation);

        /// return TRUE if there is no data (left) in this object's cache data
        bool isEmpty();

        /// check if the given module exists already (and is not empty)
        bool hasModule(const Path& _aLocation);
        /// checks if the given module exists and has defaults available
        bool hasModuleDefaults(memory::Accessor const & _aAccessor, Path const & _aLocation);

        /// retrieve the subtree at _aPath (maybe if it has the requested defaults) and clientAcquire() it
        data::NodeAddress acquireNode(memory::Accessor const& _aAccessToken, Path const& _aPath);

        /** add or merge the given subtree at the given location,
            return <TRUE/> if the tree has defaults then
        */
        bool insertDefaults(memory::UpdateAccessor& _aUpdateToken, backend::NodeInstance const & _aDefaultData ) CFG_UNO_THROW_RTE(  );

        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        CacheLine::RefCount releaseNode( Path const& _aPath );

        /// retrieve the given subtree without changing its ref count
        data::NodeAddress   findInnerNode(memory::Accessor const& _aAccessToken, Path const& _aPath );

        /// merge the given change list into this tree - reflects old data to _aUpdate
        void applyUpdate(memory::UpdateAccessor& _aUpdateToken,  backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( );
    };


////////////////////////////////////////////////////////////////////////////////

    class CacheLoadingAccess : public salhelper::SimpleReferenceObject, Noncopyable
    {
    public:
        typedef ExtendedCacheData  Data;
        typedef Data::Module       Module;
        typedef Data::ModuleRef    ModuleRef;
        typedef Data::ModuleName   ModuleName;
        typedef Data::Path         Path;

        typedef std::vector< ModuleRef >  DisposeList;
    private:
        friend class CacheDisposeScheduler;
        typedef std::map< ModuleName, TimeStamp > DeadModuleList;

        osl::Mutex          m_aMutex;
        Data                m_aData;
        DeadModuleList      m_aDeadModules;         /// list of nodes which are registered for throwing away
    public:
        explicit
        CacheLoadingAccess(memory::HeapManager & _rHeapManager);
        ~CacheLoadingAccess();

        /// gets a data segment reference for the given path if exists
        memory::Segment * createNewDataSegment(ModuleName const & _aModule);
        /// gets a data segment reference for the given path - creates if necessary
        memory::Segment * attachDataSegment(const memory::SegmentAddress & _aSegment, ModuleName const & _aModule);
        /// gets a data segment reference for the given path if exists
        memory::Segment * getDataSegment(ModuleName const & _aModule);
        /// gets a data segment address for the given module if it exists
        memory::SegmentAddress getDataSegmentAddress(ModuleName const & _aModule);

        /// return TRUE if there is no data (left) in this object's cache data
        bool isEmpty();

        /// check if the given module exists already (and is not empty)
        bool hasModule(ModuleName const & _aLocation);

        /// retrieve the subtree at aComponentName and clientAcquire() it
        data::TreeAddress acquireModule(ModuleName const & _aModule);

        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        CacheLine::RefCount releaseModule( ModuleName const & _aModule );

        /// retrieve the given subtree without changing its ref count
        data::NodeAddress   findNode(memory::Accessor const& _aAccessToken, Path const& _aPath );

        /// merge the given change list into this tree - reflects old data to _aUpdate
        void applyUpdate(memory::UpdateAccessor& _aUpdateToken,  backend::UpdateInstance & _aUpdate) CFG_UNO_THROW_RTE( );

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
        data::TreeAddress addComponentData( memory::UpdateAccessor& _aAccessToken,
                                            backend::ComponentInstance const & _aComponentInstance,
                                            bool _bIncludesDefaults
                                           ) CFG_UNO_THROW_RTE();

        /// merge the given change list into the pending change list of this tree
        void addChangesToPending( backend::ConstUpdateInstance const& _anUpdate ) CFG_UNO_THROW_RTE(  );
        /// retrieve accumulated pending changes
        std::auto_ptr<SubtreeChange> releasePendingChanges(ModuleName const& _aModule);

        /// find the modules having pending changes
        bool findPendingChangedModules( Data::PendingModuleList & _rPendingList );

        /// get a local lock for this cache line
        osl::Mutex & mutex() { return m_aMutex; }
    };


////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

