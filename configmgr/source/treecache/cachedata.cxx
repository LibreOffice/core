/*************************************************************************
 *
 *  $RCSfile: cachedata.cxx,v $
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

#include "cachedata.hxx"

#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif
#ifndef CONFIGMGR_NODEADDRESS_HXX
#include "nodeaddress.hxx"
#endif
#ifndef CONFIGMGR_TREEADDRESS_HXX
#include "treeaddress.hxx"
#endif
#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_UPDATEHELPER_HXX
#include "updatehelper.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{
    using namespace configuration;
    // ---------------------------- Client Acquire helper ----------------------------

    struct CacheLineClientRef
    {
        CacheLineRef xModule;

        CacheLineClientRef(CacheLineRef const& _xModule)
        : xModule(_xModule)
        {
            if (xModule.is())
                xModule->clientAcquire();
        }

        ~CacheLineClientRef() // release the client reference unless it was 'kept'
        {
            if (xModule.is())
                xModule->clientRelease();
        }

        void rebind(CacheLineRef const& _xModule)
        {
            if (_xModule.is())
                _xModule->clientAcquire();
            if (xModule.is())
                xModule->clientRelease();
            xModule = _xModule;
        }

        /// release the contained module so that the client reference will be kept active
        void keep()
        {
            xModule.clear();
            OSL_ASSERT(!xModule.is());
        }

        /// return the contained module so that the client reference will be kept active
        CacheLineRef keepModule()
        {
            CacheLineRef xRet = xModule;
            this->keep();
            return xRet;
        }
    private:
        CacheLineClientRef(CacheLineClientRef const& );
        CacheLineClientRef& operator=(CacheLineClientRef const& );

    };

// -----------------------------------------------------------------------------
    static inline CacheData::ModuleName implExtractModuleName(CacheData::Path const& aConfigPath)
    {
        return aConfigPath.getModuleName();
    }

// -----------------------------------------------------------------------------

    CacheData::CacheData(memory::HeapManager & _rHeapManager)
    : m_rHeapManager(_rHeapManager)
    {
    }
// -----------------------------------------------------------------------------

    CacheData::~CacheData()
    {
        CFG_TRACE_INFO("Discarding CacheData (Still holding %d module trees)", int (m_aModules.size()) );
    }
// -----------------------------------------------------------------------------

    CacheData::ModuleName CacheData::extractModuleName(Path const& _aPath)
    {
        return implExtractModuleName(_aPath);
    }
// -----------------------------------------------------------------------------
    inline
    CacheLineRef CacheData::internalGetModule(const ModuleName& _aModuleName ) const
    {
        OSL_ASSERT(!_aModuleName.isEmpty());

        ModuleList::const_iterator it = m_aModules.find(_aModuleName);

        return it!=m_aModules.end() ? it->second : CacheLineRef();
    }
// -----------------------------------------------------------------------------
    inline
    CacheLineRef CacheData::internalGetModule(const Path&  _aPath) const
    {
        return internalGetModule( implExtractModuleName(_aPath) );
    }
// -----------------------------------------------------------------------------

    inline
    void CacheData::internalAddModule(const ModuleName& _aName, const ModuleRef & _aModule)
    {
        //OSL_PRECOND(m_aModules.find(_aName) == m_aModules.end(), "ERROR: Module already present in CacheData");

          m_aModules[_aName] = _aModule;

        CFG_TRACE_INFO("CacheData Data: Added new module tree for module %s", OUSTRING2ASCII(_aName.toString()) );
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    CacheLineRef CacheData::internalAttachModule(const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  )
    {
        CacheLineRef aNewModule = doCreateAttachedModule(this->internalHeapManager(), _aLocation,_aName);

        internalAddModule( _aName, aNewModule );

        return aNewModule;
    }
// -----------------------------------------------------------------------------

    /// creates a new data segment reference for the given path - creates if necessary
    memory::Segment * CacheData::attachDataSegment(memory::SegmentAddress const & _aLocation, ModuleName const & _aModule)
    {
        CacheLineRef aModule = this->internalAttachModule(_aLocation,_aModule);

        return aModule.is() ? aModule->getDataSegment() : NULL;
    }
// -------------------------------------------------------------------------

    CacheLineRef CacheData::doCreateAttachedModule(memory::HeapManager & _rHeapManager, const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  )
    {
        return CacheLine::createAttached( _aName, _rHeapManager, _aLocation );
    }
// -----------------------------------------------------------------------------

    /// gets a data segment reference for the given path if exists
    memory::Segment * CacheData::getDataSegment(const ModuleName & _aModule)
    {
        CacheLineRef aModule = internalGetModule(_aModule);

        return aModule.is() ? aModule->getDataSegment() : NULL;
    }
// -------------------------------------------------------------------------

    /// gets a data segment reference for the given path if exists
    memory::SegmentAddress CacheData::getDataSegmentAddress(const ModuleName & _aModule) const
    {
        CacheLineRef aModule = internalGetModule(_aModule);

        return aModule.is() ? aModule->getDataSegmentAddress() : memory::SegmentAddress();
    }
// -------------------------------------------------------------------------

    bool CacheData::hasModule(const ModuleName & _aModule) const
    {
        CacheLineRef aModule = internalGetModule(_aModule);

        return aModule.is() && !aModule->isEmpty();
    }
// -------------------------------------------------------------------------

    bool CacheData::hasModuleDefaults(memory::Accessor const & _aAccessor, const ModuleName & _aModule) const
    {
        CacheLineRef aModule = internalGetModule(_aModule);

        return aModule.is() && !aModule->hasDefaults(_aAccessor);
    }
// -------------------------------------------------------------------------

    data::TreeAddress CacheData::internalGetPartialTree(memory::Accessor const & _aAccessor, const Path& aComponentName ) const
    {
        CacheLineRef xModule = internalGetModule(aComponentName);

        if ( !xModule.is() )
            return data::TreeAddress();

        data::TreeAddress pSubtree = xModule->getPartialTree(_aAccessor,aComponentName);

        OSL_ENSURE( pSubtree.isNull() || xModule->clientReferences() != 0 ,
                    "WARNING: returning subtree from module without clients\n" );
#ifdef CFG_ENABLE_TRACING
        if( pSubtree.is() && xModule->clientReferences() == 0)
        {
            CFG_TRACE_WARNING("CacheData data: returning subtree %s from module without clients", OUSTRING2ASCII( aComponentName.toString() ) );
        }
#endif // CFG_ENABLE_TRACING

        return pSubtree;
    }
// -----------------------------------------------------------------------------
    data::NodeAddress CacheData::internalGetNode(memory::Accessor const & _aAccessor, const Path& aComponentName ) const
    {
        CacheLineRef xModule = internalGetModule(aComponentName);

        if ( !xModule.is() )
            return data::NodeAddress();

        if ( xModule->isEmpty() )
            return data::NodeAddress();

        data::NodeAddress pNode = xModule->getNode(_aAccessor,aComponentName);

        OSL_ENSURE( pNode.isNull() || xModule->clientReferences() != 0,
                    "WARNING: returning node from module without clients\n" );
    #ifdef CFG_ENABLE_TRACING
        if( pNode.is() && xModule->clientReferences() == 0)
        {
            CFG_TRACE_WARNING("CacheData data: returning node %s from module without clients", OUSTRING2ASCII( aComponentName.toString() ) );
        }
#endif // CFG_ENABLE_TRACING

        return pNode;
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    data::TreeAddress CacheData::acquireModule(ModuleName const & _aModule)
    {
        CacheLineRef xModule = internalGetModule(_aModule);

        CacheLineClientRef aClientRef(xModule);

        data::TreeAddress aModuleAddr = xModule.is() ? xModule->getModuleRootAddress() : data::TreeAddress();

        if (aModuleAddr.is())
            aClientRef.keep();

        return aModuleAddr;
    }
// -----------------------------------------------------------------------------
    data::NodeAddress CacheData::acquireNode(memory::Accessor const & _aAccessor, Path const& _aPath)
    {
        CacheLineClientRef aClientRef(internalGetModule(_aPath));

        data::NodeAddress aNodeAddr = internalGetNode(_aAccessor,_aPath);

        if (sharable::Node const * pNode = data::NodeAccess::access(aNodeAddr,_aAccessor))
        {
            if (pNode->isValue())
                aNodeAddr = data::NodeAddress(); // invalid: cannot acquire single value
        }
        else
            OSL_ASSERT( !aNodeAddr.is() );

        if (aNodeAddr.is())
            aClientRef.keep();

        return aNodeAddr;
    }
// -----------------------------------------------------------------------------

    bool CacheData::insertDefaults( memory::UpdateAccessor& _aAccessToken,
                                    backend::NodeInstance const & _aDefaultInstance) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aDefaultInstance.data().get(), "insertDefaults: Data must not be NULL");
        OSL_PRECOND(_aDefaultInstance.root().isModuleRoot(), "insertDefaults: Default tree being added must be for module");

        // we should already have the module in cache !
        CacheLineRef xModule = internalGetModule(_aDefaultInstance.root().location());

        OSL_ENSURE( xModule.is(), "CacheData::insertDefaults: No module to insert the defaults to - where did the data segment come from ?");

        if (!xModule.is()) return false;

        // make sure to keep the module alive
        CacheLineClientRef( xModule ).keep();

        data::TreeAddress aResultTree = xModule->insertDefaults(_aAccessToken, _aDefaultInstance);

        return aResultTree.is();
    }
// -----------------------------------------------------------------------------

    void CacheData::applyUpdate(memory::UpdateAccessor & _aAccessToken, backend::UpdateInstance & _anUpdate ) CFG_UNO_THROW_RTE(  )
    {
        // request the subtree, atleast one level must exist!
        data::NodeAddress aNodeAddr = internalGetNode(_aAccessToken.accessor(),_anUpdate.root().location());

        if (aNodeAddr.is())
        {
            applyUpdateToTree(*_anUpdate.data(),_aAccessToken,aNodeAddr);
        }
        else
        {
            OSL_ENSURE(false, "CacheData::applyUpdate called for non-existing tree");
            OUString aStr(RTL_CONSTASCII_USTRINGPARAM("CacheData: update to non-existing node: "));

            aStr += _anUpdate.root().toString();

            throw uno::RuntimeException(aStr,0);
        }
    }
// -----------------------------------------------------------------------------
    CacheLine::RefCount CacheData::releaseModule( ModuleName const & _aModule, bool _bKeepDeadModule )
    {
        CacheLineRef xModule = internalGetModule(_aModule);

        const CacheLine::RefCount c_nErrorCount = -1;

        OSL_ENSURE( xModule.is(), "ERROR: Releasing non-existent subtree");
        if ( !xModule.is()) return c_nErrorCount;

        OSL_ENSURE( xModule->clientReferences() > 0, "ERROR: Releasing non-referenced subtree");

        CacheLine::RefCount nResult = xModule->clientRelease();

        if (nResult == 0 && !_bKeepDeadModule)
        {
              m_aModules.erase( _aModule );
        }
        return nResult;
    }

// -----------------------------------------------------------------------------
    data::TreeAddress CacheData::getTemplateTree( memory::Accessor const & _aAccessor, Path const& aTemplateName ) const
    {
        return internalGetPartialTree(_aAccessor,aTemplateName);
    }
// -----------------------------------------------------------------------------
    data::NodeAddress CacheData::getNode(memory::Accessor const & _aAccessor, const Path& _rPath)
    {
        return internalGetNode(_aAccessor,_rPath);
    }
// -----------------------------------------------------------------------------
    bool CacheData::hasNode(memory::Accessor const & _aAccessor, const Path& _rPath) const
    {
        return internalGetNode(_aAccessor,_rPath).is();
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    /// gets a data segment reference for the given path - creates if necessary
    memory::Segment * TemplateCacheData::createDataSegment(ModuleName const & _aModule)
    {
        CacheLineRef aModule = implNewCacheLine(_aModule);

        return aModule.is() ? aModule->getDataSegment() : NULL;
    }
// -------------------------------------------------------------------------

    data::TreeAddress TemplateCacheData::addTemplates(  memory::UpdateAccessor& _aUpdateToken,
                                                        backend::ComponentData const & _aComponentInstance) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aComponentInstance.data.get(), "addTemplates: Data must not be NULL");
        // we should already have the module in cache !
        ModuleName aModuleName ( _aComponentInstance.name);
        CacheLineRef xModule = internalGetModule(aModuleName);

        OSL_ENSURE( xModule.is(), "ExtendedCacheData::addTemplates: No module to add the templates to - where did the data segment come from ?");

        if (!xModule.is()) return data::TreeAddress();

        // make sure to keep the module alive
        CacheLineClientRef( xModule ).keep();

        static const OUString aDummyTemplateName(RTL_CONSTASCII_USTRINGPARAM("cfg:Template"));
        static const OUString aDummyTemplateModule(RTL_CONSTASCII_USTRINGPARAM("cfg:Templates"));
        _aComponentInstance.data->makeSetNode(aDummyTemplateName,aDummyTemplateModule);

        data::TreeAddress aResult = xModule->setComponentData(_aUpdateToken, _aComponentInstance, true);

        OSL_ASSERT(aResult.is());

        return aResult;
    }
// -----------------------------------------------------------------------------

    CacheLineRef TemplateCacheData::doCreateAttachedModule(memory::HeapManager & _rHeapManager, const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  )
    {
        CacheLineRef aNewModule =
            CacheLine::createAttached(_aName, _rHeapManager, _aLocation);

        return aNewModule.get();
    }
// -----------------------------------------------------------------------------

    CacheLineRef TemplateCacheData::implNewCacheLine(const ModuleName& _aModule) CFG_UNO_THROW_RTE(  )
    {
        CacheLineRef aNewModule =
            CacheLine::createNew(_aModule, this->internalHeapManager());

        internalAddModule( _aModule, aNewModule.get() );

        return aNewModule;
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    inline
    ExtendedCacheLineRef ExtendedCacheData::implExtended(const CacheLineRef& _aSimpleRef) const
    {
        CacheLine *         pBasic    = _aSimpleRef.get();
        ExtendedCacheLine * pExtended = static_cast<ExtendedCacheLine *>(pBasic);
        return ExtendedCacheLineRef(pExtended);
    }

// -----------------------------------------------------------------------------

    CacheLineRef ExtendedCacheData::doCreateAttachedModule(memory::HeapManager & _rHeapManager, const memory::SegmentAddress & _aLocation, const ModuleName& _aName) CFG_UNO_THROW_RTE(  )
    {
        ExtendedCacheLineRef aNewModule =
            ExtendedCacheLine::createAttached(_aName, _rHeapManager, _aLocation);

        return CacheLineRef( aNewModule.get() );
    }
// -----------------------------------------------------------------------------

    ExtendedCacheLineRef ExtendedCacheData::implNewCacheLine(const ModuleName& _aModule) CFG_UNO_THROW_RTE(  )
    {
        ExtendedCacheLineRef aNewModule =
            ExtendedCacheLine::createNew(_aModule, this->internalHeapManager());

        internalAddModule( _aModule, aNewModule.get() );

        return aNewModule;
    }
// -----------------------------------------------------------------------------

    /// gets a data segment reference for the given path - creates if necessary
    memory::Segment * ExtendedCacheData::createDataSegment(ModuleName const & _aModule)
    {
        ExtendedCacheLineRef aModule = implNewCacheLine(_aModule);

        return aModule.is() ? aModule->getDataSegment() : NULL;
    }
// -------------------------------------------------------------------------

    data::TreeAddress ExtendedCacheData::addComponentData(memory::UpdateAccessor& _aUpdateToken,
                                                    backend::ComponentInstance const & _aComponentInstance,
                                                    bool _bWithDefaults) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aComponentInstance.data().get(), "addComponentData: Data must not be NULL");
        // we should already have the module in cache !
        //CacheLineRef xModule = internalGetModule(_aNodeInstance.root().location());
        CacheLineRef xModule = internalGetModule(_aComponentInstance.component() );

        OSL_ENSURE( xModule.is(), "ExtendedCacheData::addComponentData: No module to add the subtree to - where did the data segment come from ?");

        if (!xModule.is()) return data::TreeAddress();

        CacheLineClientRef aClientRef( xModule );

        data::TreeAddress aResult = xModule->setComponentData(_aUpdateToken,_aComponentInstance.componentNodeData(), _bWithDefaults);

        OSL_ASSERT(aResult.is());

        if (aResult.is()) aClientRef.keep();

        return aResult;
    }
// -----------------------------------------------------------------------------

    void ExtendedCacheData::addPending(backend::ConstUpdateInstance const & _anUpdate) CFG_UNO_THROW_RTE(  )
    {
        // do we already have the module in cache ?
        CacheLineRef xModule = internalGetModule(_anUpdate.root().location());

        if (xModule.is())
        {
            implExtended(xModule)->addPending(_anUpdate);
        }
        else
        {
            OSL_ENSURE(false, "We can only change Nodes if we already know something about it. So this must be a big bug.");
        }
    }
// -----------------------------------------------------------------------------

    std::auto_ptr<SubtreeChange> ExtendedCacheData::releasePending(ModuleName const& _aModule)
    {
        ExtendedCacheLineRef xModule = implExtended(internalGetModule(_aModule));

        if (xModule.is())
        {
            if (xModule->hasPending())
                return xModule->releasePending();
            else
                return std::auto_ptr<SubtreeChange>();
        }
        else
        {
            OSL_ENSURE(false, "We can only get Nodes if we already know something about it.");
        }
        return std::auto_ptr<SubtreeChange>();
    }
// -----------------------------------------------------------------------------

    bool ExtendedCacheData::hasPending(ModuleName const & _aModule)
    {
        ExtendedCacheLineRef xModule = implExtended(internalGetModule(_aModule));

        return xModule.is() && xModule->hasPending();
    }
// -----------------------------------------------------------------------------

    void ExtendedCacheData::findPendingModules( PendingModuleList & _rPendingList )
    {
        ModuleList& rModules = CacheData::accessModuleList();
        for (ModuleList::iterator it = rModules.begin();
                it != rModules.end();
                ++it)
        {
            OSL_ASSERT( it->second.is() );
            if ( implExtended(it->second)->hasPending() )
                _rPendingList.push_back( it->first );
        }
    }
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
} // namespace configmgr


