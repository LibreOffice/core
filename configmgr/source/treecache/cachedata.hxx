/*************************************************************************
 *
 *  $RCSfile: cachedata.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:41 $
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

#ifndef CONFIGMGR_CACHEDATA_HXX
#define CONFIGMGR_CACHEDATA_HXX

#ifndef CONFIGMGR_CACHELINE_HXX
#include "cacheline.hxx"
#endif

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    using ::rtl::OUString;

    namespace backend
    {
        struct NodeInstance;
        struct TemplateInstance;
        struct UpdateInstance;
        struct ConstUpdateInstance;
    }

////////////////////////////////////////////////////////////////////////////////
    /** A collection of CacheLines
    */

    class CacheData
    {
    public:
        typedef CacheLine        Module;
        typedef CacheLineRef     ModuleRef;
        typedef CacheLine::Path     Path;
        typedef CacheLine::Name     ModuleName;
    public:
        CacheData(memory::HeapManager & _rHeapManager);
        ~CacheData();

        /// retrieve the module tree name for the given path
        static ModuleName extractModuleName(Path const& _aPath);

        /// check if the given module exists already (and is not empty)
        bool hasModule(ModuleName const & _aModule) const;
        /// checks if the given module exists and has defaults available
        bool hasModuleDefaults(memory::Accessor const & _aAccessor, ModuleName const & _aModule) const;

        /// creates a new data segment reference for the given path if exists
        memory::Segment * attachDataSegment(memory::SegmentAddress const & _aLocation, ModuleName const & _aModule);
        /// gets a data segment reference for the given path if it exists
        memory::Segment * getDataSegment(ModuleName const & _aModule);
        /// gets a data segment address for the given module if it exists
        memory::SegmentAddress getDataSegmentAddress(ModuleName const & _aModule) const;

        /// checks whether a certain node exists in the tree
        bool  hasNode(memory::Accessor const & _aAccessor, Path const & _aLocation) const;

        /// retrieve the given node without changing its ref count
        data::NodeAddress   getNode(memory::Accessor const & _aAccessor, Path const & _rPath);
        /// retrieve the given template tree without changing its ref count
        data::TreeAddress   getTemplateTree(memory::Accessor const & _aAccessor, Path const & aTemplateName ) const;

        /// retrieve the subtree at _aPath and clientAcquire() it
        data::NodeAddress acquireNode(memory::Accessor const & _aAccessor, Path const & _aPath );
        /// retrieve the subtree at _aPath and clientAcquire() it
        data::TreeAddress acquireModule( ModuleName const & _aModule );
        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        CacheLine::RefCount releaseModule( ModuleName const & _aModule, bool _bKeepDeadModule = false );

        bool insertDefaults( memory::UpdateAccessor& _aAccessToken,
                             backend::NodeInstance const & _aDefaultInstance
                           ) CFG_UNO_THROW_RTE();

        /// merge the given changes into this tree - reflects old values to _anUpdate
        void applyUpdate(   memory::UpdateAccessor& _aUpdateToken,
                            backend::UpdateInstance & _anUpdate) CFG_UNO_THROW_RTE(  );

        // low-level interface for cache management
        typedef std::map<ModuleName, ModuleRef> ModuleList;
        ModuleList& accessModuleList() { return m_aModules; }

        memory::HeapManager & getHeapManager() const { return m_rHeapManager; }
    protected:
        virtual ModuleRef doCreateAttachedModule(memory::HeapManager & _rHeapManager, const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  );

        data::TreeAddress internalGetPartialTree(memory::Accessor const & _aAccessor, Path const & _aPath ) const;
        data::NodeAddress internalGetNode(memory::Accessor const & _aAccessor, const Path& _rPath) const;

        ModuleRef internalAttachModule(const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  );
        void internalAddModule(ModuleName const & _aName, ModuleRef const & _aModule);

        ModuleRef internalGetModule(const ModuleName& _aName) const;
        ModuleRef internalGetModule(const Path& _aLocation) const;

        memory::HeapManager & internalHeapManager() { return m_rHeapManager; }
    private:
        ModuleList m_aModules;

        memory::HeapManager & m_rHeapManager;
    };
////////////////////////////////////////////////////////////////////////////////
    /** A collection of CacheLines for templates
    */

    class TemplateCacheData : public CacheData
    {
    public:
        TemplateCacheData(memory::HeapManager & _rHeapManager)
        : CacheData(_rHeapManager)
        {
        }

        /// gets a data segment reference for the given path - creates if necessary
        memory::Segment * createDataSegment(ModuleName const & _aModule);

        /** add the given template tree at the given location,
            return the tree that is now pertinent and clientAcquire() it once
        */
        data::TreeAddress addTemplates( memory::UpdateAccessor& _aAccessToken,
                                        backend::ComponentData const & _aComponentInstance
                                       ) CFG_UNO_THROW_RTE();

    private:
        virtual ModuleRef doCreateAttachedModule(memory::HeapManager & _rHeapManager, const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  );

        CacheLineRef implNewCacheLine(ModuleName const & _aModule) CFG_UNO_THROW_RTE(  );
    };
//-----------------------------------------------------------------------------
    /** A collection of CacheLines
    */

    class ExtendedCacheData : public CacheData
    {
    public:
        ExtendedCacheData(memory::HeapManager & _rHeapManager)
        : CacheData(_rHeapManager)
        {
        }

        /// gets a data segment reference for the given path - creates if necessary
        memory::Segment * createDataSegment(ModuleName const & _aModule);

        /** add the given subtree at the given location,
            return the tree that is now pertinent and clientAcquire() it once
        */
        data::TreeAddress addComponentData( memory::UpdateAccessor& _aAccessToken,
                                            backend::ComponentInstance const & _aComponentInstance,
                                            bool _bWithDefaults
                                           ) CFG_UNO_THROW_RTE();

        typedef std::vector< ModuleName >  PendingModuleList;
        /// find the modules having pending changes
        bool hasPending(ModuleName const & _aModule);
        /// find the modules having pending changes
        void findPendingModules( PendingModuleList & _rPendingList );

        /// add or merge the given subtreechange at the given location
        void addPending(backend::ConstUpdateInstance const & _anUpdate) CFG_UNO_THROW_RTE(  );
        /// remove and return pending changes for the given component
        std::auto_ptr<SubtreeChange> releasePending(ModuleName const & _aModule) CFG_UNO_THROW_RTE(  );
    private:
        virtual ModuleRef doCreateAttachedModule(memory::HeapManager & _rHeapManager, const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  );

        ExtendedCacheLineRef implNewCacheLine(ModuleName const & _aModule) CFG_UNO_THROW_RTE(  );

        ExtendedCacheLineRef implExtended(ModuleRef const & _aSimpleRef) const;
    };
//-----------------------------------------------------------------------------

} // namespace configmgr

#endif

