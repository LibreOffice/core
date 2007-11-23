/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedata.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:35:48 $
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
        CacheData();
        virtual ~CacheData();

        /// retrieve the module tree name for the given path
        static CacheLine::Name extractModuleName(CacheLine::Path const& _aPath);

        // attach a module with a given name
        void attachModule(data::TreeAddress _aLocation, CacheLine::Name const & _aModule);
        /// check if the given module exists already (and is not empty)
        bool hasModule(CacheLine::Name const & _aModule) const;
        /// checks if the given module exists and has defaults available
        bool hasModuleDefaults(CacheLine::Name const & _aModule) const;

        /// gets a tree address for the given module if it exists
        data::TreeAddress getTreeAddress(CacheLine::Name const & _aModule) const;

        /// checks whether a certain node exists in the tree
        bool  hasNode(CacheLine::Path const & _aLocation) const;

        /// retrieve the given node without changing its ref count
        data::NodeAddress   getNode(CacheLine::Path const & _rPath);
        /// retrieve the given template tree without changing its ref count
        data::TreeAddress   getTemplateTree( CacheLine::Path const & aTemplateName ) const;

        /// retrieve the subtree at _aPath and clientAcquire() it
        data::NodeAddress acquireNode(CacheLine::Path const & _aPath );
        /// retrieve the subtree at _aPath and clientAcquire() it, return true on success
        bool acquireModule( CacheLine::Name const & _aModule );
        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        oslInterlockedCount releaseModule( CacheLine::Name const & _aModule, bool _bKeepDeadModule = false );

        bool insertDefaults( backend::NodeInstance const & _aDefaultInstance
                           ) CFG_UNO_THROW_RTE();

        /// merge the given changes into this tree - reflects old values to _anUpdate
        void applyUpdate( backend::UpdateInstance & _anUpdate) CFG_UNO_THROW_RTE(  );

        // low-level interface for cache management
        typedef std::map<CacheLine::Name, CacheLineRef> ModuleList;
        ModuleList& accessModuleList() { return m_aModules; }

    protected:
        virtual CacheLineRef doCreateAttachedModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  );

        data::TreeAddress internalGetPartialTree(CacheLine::Path const & _aPath ) const;
        data::NodeAddress internalGetNode(const CacheLine::Path& _rPath) const;

        CacheLineRef internalAttachModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  );
        void internalAddModule(CacheLine::Name const & _aName, CacheLineRef const & _aModule);

        CacheLineRef internalGetModule(const CacheLine::Name& _aName) const;
        CacheLineRef internalGetModule(const CacheLine::Path& _aLocation) const;

    private:
        ModuleList m_aModules;
    };
////////////////////////////////////////////////////////////////////////////////
    /** A collection of CacheLines for templates
    */

    class TemplateCacheData : public CacheData
    {
    public:
        TemplateCacheData() : CacheData()
        {
        }

        /** add the given template tree at the given location,
            return the tree that is now pertinent and clientAcquire() it once
        */
        data::TreeAddress addTemplates( backend::ComponentData const & _aComponentInstance
                                       ) CFG_UNO_THROW_RTE();

        // create a new module with the given name
        void createModule(CacheLine::Name const & _aModule) CFG_UNO_THROW_RTE();
    private:
        virtual CacheLineRef doCreateAttachedModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  );
    };
//-----------------------------------------------------------------------------
    /** A collection of CacheLines
    */

    class ExtendedCacheData : public CacheData
    {
    public:
        ExtendedCacheData() : CacheData()
        {
        }

        /** add the given subtree at the given location,
            return the tree that is now pertinent and clientAcquire() it once
        */
        data::TreeAddress addComponentData( backend::ComponentInstance const & _aComponentInstance,
                                            bool _bWithDefaults
                                           ) CFG_UNO_THROW_RTE();

        typedef std::vector< CacheLine::Name >  PendingModuleList;

        /// find the modules having pending changes
        bool hasPending(CacheLine::Name const & _aModule);
        /// find the modules having pending changes
        void findPendingModules( PendingModuleList & _rPendingList );

        /// add or merge the given subtreechange at the given location
        void addPending(backend::ConstUpdateInstance const & _anUpdate) CFG_UNO_THROW_RTE(  );
        /// remove and return pending changes for the given component
        std::auto_ptr<SubtreeChange> releasePending(CacheLine::Name const & _aModule) CFG_UNO_THROW_RTE(  );

        // create a new module with the given name
        void createModule(CacheLine::Name const & _aModule) CFG_UNO_THROW_RTE();
    private:
        virtual CacheLineRef doCreateAttachedModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  );

        ExtendedCacheLineRef implExtended(CacheLineRef const & _aSimpleRef) const;
    };
//-----------------------------------------------------------------------------

} // namespace configmgr

#endif

