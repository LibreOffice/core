/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedata.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:35:36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "cachedata.hxx"

#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
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
    static inline CacheLine::Name implExtractModuleName(CacheLine::Path const& aConfigPath)
    {
        return aConfigPath.getModuleName();
    }

// -----------------------------------------------------------------------------

    CacheData::CacheData()
    {
    }
// -----------------------------------------------------------------------------

    CacheData::~CacheData()
    {
        CFG_TRACE_INFO("Discarding CacheData (Still holding %d module trees)", int (m_aModules.size()) );
    }
// -----------------------------------------------------------------------------

    CacheLine::Name CacheData::extractModuleName(CacheLine::Path const& _aPath)
    {
        return implExtractModuleName(_aPath);
    }
// -----------------------------------------------------------------------------
    inline
    CacheLineRef CacheData::internalGetModule(const CacheLine::Name& _aModuleName ) const
    {
        OSL_ASSERT(!_aModuleName.isEmpty());

        ModuleList::const_iterator it = m_aModules.find(_aModuleName);

        return it!=m_aModules.end() ? it->second : CacheLineRef();
    }
// -----------------------------------------------------------------------------
    inline
    CacheLineRef CacheData::internalGetModule(const CacheLine::Path&  _aPath) const
    {
        return internalGetModule( implExtractModuleName(_aPath) );
    }
// -----------------------------------------------------------------------------

    inline
    void CacheData::internalAddModule(const CacheLine::Name& _aName, const CacheLineRef & _aModule)
    {
        //OSL_PRECOND(m_aModules.find(_aName) == m_aModules.end(), "ERROR: Module already present in CacheData");

          m_aModules[_aName] = _aModule;

        CFG_TRACE_INFO("CacheData Data: Added new module tree for module %s", OUSTRING2ASCII(_aName.toString()) );
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    CacheLineRef CacheData::internalAttachModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  )
    {
        CacheLineRef aNewModule = doCreateAttachedModule(_aLocation,_aName);

        internalAddModule( _aName, aNewModule );

        return aNewModule;
    }

// -------------------------------------------------------------------------

    void CacheData::attachModule(data::TreeAddress _aLocation, CacheLine::Name const & _aModule)
    {
        this->internalAttachModule(_aLocation,_aModule);
    }
// -------------------------------------------------------------------------

    CacheLineRef CacheData::doCreateAttachedModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  )
    {
        return CacheLine::createAttached( _aName, _aLocation );
    }
// -----------------------------------------------------------------------------

    /// gets a data segment reference for the given path if exists
    data::TreeAddress CacheData::getTreeAddress(const CacheLine::Name & _aModule) const
    {
        CacheLineRef aModule = internalGetModule(_aModule);

        return aModule.is() ? aModule->getTreeAddress() : NULL;
    }
// -------------------------------------------------------------------------

    bool CacheData::hasModule(const CacheLine::Name & _aModule) const
    {
        CacheLineRef aModule = internalGetModule(_aModule);

        return aModule.is() && !aModule->isEmpty();
    }
// -------------------------------------------------------------------------

    bool CacheData::hasModuleDefaults(const CacheLine::Name & _aModule) const
    {
        CacheLineRef aModule = internalGetModule(_aModule);

        return aModule.is() && !aModule->hasDefaults();
    }
// -------------------------------------------------------------------------

    data::TreeAddress CacheData::internalGetPartialTree(const CacheLine::Path& aComponentName ) const
    {
    CacheLineRef xModule = internalGetModule(aComponentName);

    if ( !xModule.is() )
            return NULL;

        data::TreeAddress pSubtree = xModule->getPartialTree(aComponentName);

        OSL_ENSURE( pSubtree == NULL || xModule->clientReferences() != 0 ,
                    "WARNING: returning subtree from module without clients\n" );
#ifdef CFG_ENABLE_TRACING
        if( pSubtree != NULL && xModule->clientReferences() == 0)
        {
            CFG_TRACE_WARNING("CacheData data: returning subtree %s from module without clients", OUSTRING2ASCII( aComponentName.toString() ) );
        }
#endif // CFG_ENABLE_TRACING

        return pSubtree;
    }
// -----------------------------------------------------------------------------
    data::NodeAddress CacheData::internalGetNode(const CacheLine::Path& aComponentName ) const
    {
        CacheLineRef xModule = internalGetModule(aComponentName);

        if ( !xModule.is() )
            return data::NodeAddress();

        if ( xModule->isEmpty() )
            return data::NodeAddress();

        data::NodeAddress pNode = xModule->getNode(aComponentName);

        OSL_ENSURE( pNode == NULL || xModule->clientReferences() != 0,
                    "WARNING: returning node from module without clients\n" );
    #ifdef CFG_ENABLE_TRACING
        if( pNode != NULL && xModule->clientReferences() == 0)
        {
            CFG_TRACE_WARNING("CacheData data: returning node %s from module without clients", OUSTRING2ASCII( aComponentName.toString() ) );
        }
#endif // CFG_ENABLE_TRACING

        return pNode;
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    bool CacheData::acquireModule(CacheLine::Name const & _aModule)
    {
        CacheLineRef xModule = internalGetModule(_aModule);

        if (xModule.is())
        {
            CacheLineClientRef aClientRef(xModule);
            aClientRef.keep();
        }

        return xModule.is();
    }
// -----------------------------------------------------------------------------
    data::NodeAddress CacheData::acquireNode(CacheLine::Path const& _aPath)
    {
    CacheLineClientRef aClientRef(internalGetModule(_aPath));

        data::NodeAddress aNodeAddr = internalGetNode(_aPath);

        if (sharable::Node const * pNode = aNodeAddr)
    {
            if (pNode->isValue())
                aNodeAddr = data::NodeAddress(); // invalid: cannot acquire single value
    }
        else
            OSL_ASSERT( aNodeAddr == NULL );

        if (aNodeAddr != NULL)
            aClientRef.keep();

        return aNodeAddr;
    }
// -----------------------------------------------------------------------------

    bool CacheData::insertDefaults( backend::NodeInstance const & _aDefaultInstance) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aDefaultInstance.data().get(), "insertDefaults: Data must not be NULL");
        OSL_PRECOND(_aDefaultInstance.root().isModuleRoot(), "insertDefaults: Default tree being added must be for module");

        // we should already have the module in cache !
        CacheLineRef xModule = internalGetModule(_aDefaultInstance.root().location());

        OSL_ENSURE( xModule.is(), "CacheData::insertDefaults: No module to insert the defaults to - where did the data segment come from ?");

        if (!xModule.is()) return false;

        // make sure to keep the module alive
    CacheLineClientRef( xModule ).keep();

    data::TreeAddress aResultTree = xModule->insertDefaults(_aDefaultInstance);

    return aResultTree != NULL;
    }
// -----------------------------------------------------------------------------

    void CacheData::applyUpdate( backend::UpdateInstance & _anUpdate ) CFG_UNO_THROW_RTE(  )
    {
        // request the subtree, atleast one level must exist!
        data::NodeAddress aNodeAddr = internalGetNode(_anUpdate.root().location());

        if (aNodeAddr != NULL)
        {
            applyUpdateToTree(*_anUpdate.data(),aNodeAddr);
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
    oslInterlockedCount CacheData::releaseModule( CacheLine::Name const & _aModule, bool _bKeepDeadModule )
    {
        CacheLineRef xModule = internalGetModule(_aModule);

        const oslInterlockedCount c_nErrorCount = -1;

        OSL_ENSURE( xModule.is(), "ERROR: Releasing non-existent subtree");
        if ( !xModule.is()) return c_nErrorCount;

        OSL_ENSURE( xModule->clientReferences() > 0, "ERROR: Releasing non-referenced subtree");

        oslInterlockedCount nResult = xModule->clientRelease();

        if (nResult == 0 && !_bKeepDeadModule)
        {
              m_aModules.erase( _aModule );
        }
        return nResult;
    }

// -----------------------------------------------------------------------------
    data::TreeAddress CacheData::getTemplateTree(CacheLine::Path const& aTemplateName ) const
    {
        return internalGetPartialTree(aTemplateName);
    }
// -----------------------------------------------------------------------------
    data::NodeAddress CacheData::getNode(const CacheLine::Path& _rPath)
    {
        return internalGetNode(_rPath);
    }
// -----------------------------------------------------------------------------
    bool CacheData::hasNode(const CacheLine::Path& _rPath) const
    {
        return internalGetNode(_rPath) != NULL;
    }
// -----------------------------------------------------------------------------

    data::TreeAddress TemplateCacheData::addTemplates( backend::ComponentData const & _aComponentInstance) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aComponentInstance.data.get(), "addTemplates: Data must not be NULL");
        // we should already have the module in cache !
        CacheLine::Name aModuleName ( _aComponentInstance.name);
        CacheLineRef xModule = internalGetModule(aModuleName);

        OSL_ENSURE( xModule.is(), "ExtendedCacheData::addTemplates: No module to add the templates to - where did the data segment come from ?");

        if (!xModule.is()) return NULL;

        // make sure to keep the module alive
        CacheLineClientRef( xModule ).keep();

        static const OUString aDummyTemplateName(RTL_CONSTASCII_USTRINGPARAM("cfg:Template"));
        static const OUString aDummyTemplateModule(RTL_CONSTASCII_USTRINGPARAM("cfg:Templates"));
        _aComponentInstance.data->makeSetNode(aDummyTemplateName,aDummyTemplateModule);

        data::TreeAddress aResult = xModule->setComponentData(_aComponentInstance, true);

        OSL_ASSERT(aResult != NULL);

    return aResult;
    }
// -----------------------------------------------------------------------------

    CacheLineRef TemplateCacheData::doCreateAttachedModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  )
    {
        CacheLineRef aNewModule =  CacheLine::createAttached(_aName, _aLocation);

        return aNewModule.get();
    }
// -----------------------------------------------------------------------------

    void TemplateCacheData::createModule(const CacheLine::Name& _aModule) CFG_UNO_THROW_RTE()
    {
        CacheLineRef aNewModule = CacheLine::createNew(_aModule);

        internalAddModule( _aModule, aNewModule.get() );
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

    CacheLineRef ExtendedCacheData::doCreateAttachedModule(data::TreeAddress _aLocation, const CacheLine::Name& _aName) CFG_UNO_THROW_RTE(  )
    {
        ExtendedCacheLineRef aNewModule =
            ExtendedCacheLine::createAttached(_aName, _aLocation);

        return CacheLineRef( aNewModule.get() );
    }
// -----------------------------------------------------------------------------

    void ExtendedCacheData::createModule(const CacheLine::Name& _aModule) CFG_UNO_THROW_RTE()
    {
        ExtendedCacheLineRef aNewModule = ExtendedCacheLine::createNew(_aModule);

        internalAddModule( _aModule, aNewModule.get() );
    }
// -----------------------------------------------------------------------------

    data::TreeAddress ExtendedCacheData::addComponentData(backend::ComponentInstance const & _aComponentInstance,
                              bool _bWithDefaults) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aComponentInstance.data().get(), "addComponentData: Data must not be NULL");
        // we should already have the module in cache !
        //CacheLineRef xModule = internalGetModule(_aNodeInstance.root().location());
        CacheLineRef xModule = internalGetModule(_aComponentInstance.component() );

        OSL_ENSURE( xModule.is(), "ExtendedCacheData::addComponentData: No module to add the subtree to - where did the data segment come from ?");

        if (!xModule.is()) return NULL;

        CacheLineClientRef aClientRef( xModule );

        data::TreeAddress aResult = xModule->setComponentData(_aComponentInstance.componentNodeData(), _bWithDefaults);

        OSL_ASSERT(aResult != NULL);

    if (aResult != NULL) aClientRef.keep();

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

    std::auto_ptr<SubtreeChange> ExtendedCacheData::releasePending(CacheLine::Name const& _aModule)
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

    bool ExtendedCacheData::hasPending(CacheLine::Name const & _aModule)
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


