/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedata.hxx,v $
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

#ifndef CONFIGMGR_CACHEDATA_HXX
#define CONFIGMGR_CACHEDATA_HXX

#include "cacheline.hxx"

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

        // attach a module with a given name
        void attachModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aModule);
        /// check if the given module exists already (and is not empty)
        bool hasModule(rtl::OUString const & _aModule) const;
        /// checks if the given module exists and has defaults available
        bool hasModuleDefaults(rtl::OUString const & _aModule) const;

        /// gets a tree address for the given module if it exists
        sharable::TreeFragment * getTreeAddress(rtl::OUString const & _aModule) const;

        /// checks whether a certain node exists in the tree
        bool  hasNode(configuration::AbsolutePath const & _aLocation) const;

        /// retrieve the given node without changing its ref count
        sharable::Node *   getNode(configuration::AbsolutePath const & _rPath);
        /// retrieve the given template tree without changing its ref count
        sharable::TreeFragment *    getTemplateTree( configuration::AbsolutePath const & aTemplateName ) const;

        /// retrieve the subtree at _aPath and clientAcquire() it
        sharable::Node * acquireNode(configuration::AbsolutePath const & _aPath );
        /// retrieve the subtree at _aPath and clientAcquire() it, return true on success
        bool acquireModule( rtl::OUString const & _aModule );
        /// clientRelease() the tree at aComponentName, and return the resulting reference count
        oslInterlockedCount releaseModule( rtl::OUString const & _aModule, bool _bKeepDeadModule = false );

        bool insertDefaults( backend::NodeInstance const & _aDefaultInstance
                           ) SAL_THROW((com::sun::star::uno::RuntimeException));

        /// merge the given changes into this tree - reflects old values to _anUpdate
        void applyUpdate( backend::UpdateInstance & _anUpdate) SAL_THROW((com::sun::star::uno::RuntimeException));

        // low-level interface for cache management
        typedef std::map<rtl::OUString, rtl::Reference<CacheLine> > ModuleList;
        ModuleList& accessModuleList() { return m_aModules; }

    protected:
        virtual rtl::Reference<CacheLine> doCreateAttachedModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException));

        sharable::TreeFragment * internalGetPartialTree(configuration::AbsolutePath const & _aPath ) const;
        sharable::Node * internalGetNode(const configuration::AbsolutePath& _rPath) const;

        rtl::Reference<CacheLine> internalAttachModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException));
        void internalAddModule(rtl::OUString const & _aName, rtl::Reference<CacheLine> const & _aModule);

        rtl::Reference<CacheLine> internalGetModule(rtl::OUString const & _aName) const;
        rtl::Reference<CacheLine> internalGetModule(const configuration::AbsolutePath& _aLocation) const;

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
        sharable::TreeFragment * addTemplates( backend::ComponentDataStruct const & _aComponentInstance
                                       ) SAL_THROW((com::sun::star::uno::RuntimeException));

        // create a new module with the given name
        void createModule(rtl::OUString const & _aModule) SAL_THROW((com::sun::star::uno::RuntimeException));
    private:
        virtual rtl::Reference<CacheLine> doCreateAttachedModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException));
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
        sharable::TreeFragment * addComponentData( backend::ComponentInstance const & _aComponentInstance,
                                            bool _bWithDefaults
                                           ) SAL_THROW((com::sun::star::uno::RuntimeException));

        /// find the modules having pending changes
        bool hasPending(rtl::OUString const & _aModule);
        /// find the modules having pending changes
        void findPendingModules( std::vector< rtl::OUString > & _rPendingList );

        /// add or merge the given subtreechange at the given location
        void addPending(backend::ConstUpdateInstance const & _anUpdate) SAL_THROW((com::sun::star::uno::RuntimeException));
        /// remove and return pending changes for the given component
        std::auto_ptr<SubtreeChange> releasePending(rtl::OUString const & _aModule) SAL_THROW((com::sun::star::uno::RuntimeException));

        // create a new module with the given name
        void createModule(rtl::OUString const & _aModule) SAL_THROW((com::sun::star::uno::RuntimeException));
    private:
        virtual rtl::Reference<CacheLine> doCreateAttachedModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException));

        rtl::Reference<ExtendedCacheLine> implExtended(rtl::Reference<CacheLine> const & _aSimpleRef) const;
    };
//-----------------------------------------------------------------------------

} // namespace configmgr

#endif

