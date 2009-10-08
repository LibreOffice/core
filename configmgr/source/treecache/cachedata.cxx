/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedata.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "cachedata.hxx"
#include "node.hxx"
#include "updatehelper.hxx"
#include "tracer.hxx"
#include <osl/diagnose.h>
#include "configpath.hxx"

namespace configmgr
{
    // ---------------------------- Client Acquire helper ----------------------------

    struct CacheLineClientRef
    {
        rtl::Reference<CacheLine> xModule;

        CacheLineClientRef(rtl::Reference<CacheLine> const& _xModule)
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

        void rebind(rtl::Reference<CacheLine> const& _xModule)
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
        rtl::Reference<CacheLine> keepModule()
        {
            rtl::Reference<CacheLine> xRet = xModule;
            this->keep();
            return xRet;
        }
    private:
        CacheLineClientRef(CacheLineClientRef const& );
        CacheLineClientRef& operator=(CacheLineClientRef const& );

    };

// -----------------------------------------------------------------------------
    static inline rtl::OUString implExtractModuleName(configuration::AbsolutePath const& aConfigPath)
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
    inline
    rtl::Reference<CacheLine> CacheData::internalGetModule(rtl::OUString const & _aModuleName ) const
    {
        OSL_ASSERT(_aModuleName.getLength() != 0);

        ModuleList::const_iterator it = m_aModules.find(_aModuleName);

        return it!=m_aModules.end() ? it->second : rtl::Reference<CacheLine>();
    }
// -----------------------------------------------------------------------------
    inline
    rtl::Reference<CacheLine> CacheData::internalGetModule(const configuration::AbsolutePath&  _aPath) const
    {
        return internalGetModule( implExtractModuleName(_aPath) );
    }
// -----------------------------------------------------------------------------

    inline
    void CacheData::internalAddModule(rtl::OUString const & _aName, const rtl::Reference<CacheLine> & _aModule)
    {
        //OSL_PRECOND(m_aModules.find(_aName) == m_aModules.end(), "ERROR: Module already present in CacheData");

          m_aModules[_aName] = _aModule;

        CFG_TRACE_INFO("CacheData Data: Added new module tree for module %s", OUSTRING2ASCII(_aName) );
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    rtl::Reference<CacheLine> CacheData::internalAttachModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        rtl::Reference<CacheLine> aNewModule = doCreateAttachedModule(_aLocation,_aName);

        internalAddModule( _aName, aNewModule );

        return aNewModule;
    }

// -------------------------------------------------------------------------

    void CacheData::attachModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aModule)
    {
        this->internalAttachModule(_aLocation,_aModule);
    }
// -------------------------------------------------------------------------

    rtl::Reference<CacheLine> CacheData::doCreateAttachedModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        return CacheLine::createAttached( _aName, _aLocation );
    }
// -----------------------------------------------------------------------------

    /// gets a data segment reference for the given path if exists
    sharable::TreeFragment * CacheData::getTreeAddress(rtl::OUString const & _aModule) const
    {
        rtl::Reference<CacheLine> aModule = internalGetModule(_aModule);

        return aModule.is() ? aModule->getTreeAddress() : NULL;
    }
// -------------------------------------------------------------------------

    bool CacheData::hasModule(rtl::OUString const & _aModule) const
    {
        rtl::Reference<CacheLine> aModule = internalGetModule(_aModule);

        return aModule.is() && !aModule->isEmpty();
    }
// -------------------------------------------------------------------------

    bool CacheData::hasModuleDefaults(rtl::OUString const & _aModule) const
    {
        rtl::Reference<CacheLine> aModule = internalGetModule(_aModule);

        return aModule.is() && !aModule->hasDefaults();
    }
// -------------------------------------------------------------------------

    sharable::TreeFragment * CacheData::internalGetPartialTree(const configuration::AbsolutePath& aComponentName ) const
    {
    rtl::Reference<CacheLine> xModule = internalGetModule(aComponentName);

    if ( !xModule.is() )
            return NULL;

        sharable::TreeFragment * pSubtree = xModule->getPartialTree(aComponentName);

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
    sharable::Node * CacheData::internalGetNode(const configuration::AbsolutePath& aComponentName ) const
    {
        rtl::Reference<CacheLine> xModule = internalGetModule(aComponentName);

        if ( !xModule.is() )
            return 0;

        if ( xModule->isEmpty() )
            return 0;

        sharable::Node * pNode = xModule->getNode(aComponentName);

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
    bool CacheData::acquireModule(rtl::OUString const & _aModule)
    {
        rtl::Reference<CacheLine> xModule = internalGetModule(_aModule);

        if (xModule.is())
        {
            CacheLineClientRef aClientRef(xModule);
            aClientRef.keep();
        }

        return xModule.is();
    }
// -----------------------------------------------------------------------------
    sharable::Node * CacheData::acquireNode(configuration::AbsolutePath const& _aPath)
    {
    CacheLineClientRef aClientRef(internalGetModule(_aPath));

        sharable::Node * aNodeAddr = internalGetNode(_aPath);

        if (sharable::Node const * pNode = aNodeAddr)
    {
            if (pNode->isValue())
                aNodeAddr = 0; // invalid: cannot acquire single value
    }
        else
            OSL_ASSERT( aNodeAddr == NULL );

        if (aNodeAddr != NULL)
            aClientRef.keep();

        return aNodeAddr;
    }
// -----------------------------------------------------------------------------

    bool CacheData::insertDefaults( backend::NodeInstance const & _aDefaultInstance) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        OSL_PRECOND(_aDefaultInstance.data().get(), "insertDefaults: Data must not be NULL");
        OSL_PRECOND(_aDefaultInstance.root().getDepth() == 1, "insertDefaults: Default tree being added must be for module");

        // we should already have the module in cache !
        rtl::Reference<CacheLine> xModule = internalGetModule(_aDefaultInstance.root());

        OSL_ENSURE( xModule.is(), "CacheData::insertDefaults: No module to insert the defaults to - where did the data segment come from ?");

        if (!xModule.is()) return false;

        // make sure to keep the module alive
    CacheLineClientRef( xModule ).keep();

    sharable::TreeFragment * aResultTree = xModule->insertDefaults(_aDefaultInstance);

    return aResultTree != NULL;
    }
// -----------------------------------------------------------------------------

    void CacheData::applyUpdate( backend::UpdateInstance & _anUpdate ) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        // request the subtree, atleast one level must exist!
        sharable::Node * aNodeAddr = internalGetNode(_anUpdate.root());

        if (aNodeAddr != NULL)
        {
            applyUpdateToTree(*_anUpdate.data(),aNodeAddr);
        }
        else
        {
            OSL_ENSURE(false, "CacheData::applyUpdate called for non-existing tree");
            rtl::OUString aStr(RTL_CONSTASCII_USTRINGPARAM("CacheData: update to non-existing node: "));

            aStr += _anUpdate.root().toString();

            throw uno::RuntimeException(aStr,0);
        }
    }
// -----------------------------------------------------------------------------
    oslInterlockedCount CacheData::releaseModule( rtl::OUString const & _aModule, bool _bKeepDeadModule )
    {
        rtl::Reference<CacheLine> xModule = internalGetModule(_aModule);

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
    sharable::TreeFragment * CacheData::getTemplateTree(configuration::AbsolutePath const& aTemplateName ) const
    {
        return internalGetPartialTree(aTemplateName);
    }
// -----------------------------------------------------------------------------
    sharable::Node * CacheData::getNode(const configuration::AbsolutePath& _rPath)
    {
        return internalGetNode(_rPath);
    }
// -----------------------------------------------------------------------------
    bool CacheData::hasNode(const configuration::AbsolutePath& _rPath) const
    {
        return internalGetNode(_rPath) != NULL;
    }
// -----------------------------------------------------------------------------

    sharable::TreeFragment * TemplateCacheData::addTemplates( backend::ComponentDataStruct const & _aComponentInstance) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        OSL_PRECOND(_aComponentInstance.data.get(), "addTemplates: Data must not be NULL");
        // we should already have the module in cache !
        rtl::OUString aModuleName ( _aComponentInstance.name);
        rtl::Reference<CacheLine> xModule = internalGetModule(aModuleName);

        OSL_ENSURE( xModule.is(), "ExtendedCacheData::addTemplates: No module to add the templates to - where did the data segment come from ?");

        if (!xModule.is()) return NULL;

        // make sure to keep the module alive
        CacheLineClientRef( xModule ).keep();

        static const rtl::OUString aDummyTemplateName(RTL_CONSTASCII_USTRINGPARAM("cfg:Template"));
        static const rtl::OUString aDummyTemplateModule(RTL_CONSTASCII_USTRINGPARAM("cfg:Templates"));
        _aComponentInstance.data->makeSetNode(aDummyTemplateName,aDummyTemplateModule);

        sharable::TreeFragment * aResult = xModule->setComponentData(_aComponentInstance, true);

        OSL_ASSERT(aResult != NULL);

    return aResult;
    }
// -----------------------------------------------------------------------------

    rtl::Reference<CacheLine> TemplateCacheData::doCreateAttachedModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        rtl::Reference<CacheLine> aNewModule =  CacheLine::createAttached(_aName, _aLocation);

        return aNewModule.get();
    }
// -----------------------------------------------------------------------------

    void TemplateCacheData::createModule(rtl::OUString const & _aModule) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        rtl::Reference<CacheLine> aNewModule = CacheLine::createNew(_aModule);

        internalAddModule( _aModule, aNewModule.get() );
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    inline
    rtl::Reference<ExtendedCacheLine> ExtendedCacheData::implExtended(const rtl::Reference<CacheLine>& _aSimpleRef) const
    {
        CacheLine *         pBasic    = _aSimpleRef.get();
        ExtendedCacheLine * pExtended = static_cast<ExtendedCacheLine *>(pBasic);
        return rtl::Reference<ExtendedCacheLine>(pExtended);
    }

// -----------------------------------------------------------------------------

    rtl::Reference<CacheLine> ExtendedCacheData::doCreateAttachedModule(sharable::TreeFragment * _aLocation, rtl::OUString const & _aName) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        rtl::Reference<ExtendedCacheLine> aNewModule =
            ExtendedCacheLine::createAttached(_aName, _aLocation);

        return rtl::Reference<CacheLine>( aNewModule.get() );
    }
// -----------------------------------------------------------------------------

    void ExtendedCacheData::createModule(rtl::OUString const & _aModule) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        rtl::Reference<ExtendedCacheLine> aNewModule = ExtendedCacheLine::createNew(_aModule);

        internalAddModule( _aModule, aNewModule.get() );
    }
// -----------------------------------------------------------------------------

    sharable::TreeFragment * ExtendedCacheData::addComponentData(backend::ComponentInstance const & _aComponentInstance,
                              bool _bWithDefaults) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        OSL_PRECOND(_aComponentInstance.data().get(), "addComponentData: Data must not be NULL");
        // we should already have the module in cache !
        //rtl::Reference<CacheLine> xModule = internalGetModule(_aNodeInstance.root().location());
        rtl::Reference<CacheLine> xModule = internalGetModule(_aComponentInstance.component() );

        OSL_ENSURE( xModule.is(), "ExtendedCacheData::addComponentData: No module to add the subtree to - where did the data segment come from ?");

        if (!xModule.is()) return NULL;

        CacheLineClientRef aClientRef( xModule );

        sharable::TreeFragment * aResult = xModule->setComponentData(_aComponentInstance.componentNodeData(), _bWithDefaults);

        OSL_ASSERT(aResult != NULL);

    if (aResult != NULL) aClientRef.keep();

    return aResult;
    }
// -----------------------------------------------------------------------------

    void ExtendedCacheData::addPending(backend::ConstUpdateInstance const & _anUpdate) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        // do we already have the module in cache ?
        rtl::Reference<CacheLine> xModule = internalGetModule(_anUpdate.root());

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

    std::auto_ptr<SubtreeChange> ExtendedCacheData::releasePending(rtl::OUString const& _aModule)
    {
        rtl::Reference<ExtendedCacheLine> xModule = implExtended(internalGetModule(_aModule));

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

    bool ExtendedCacheData::hasPending(rtl::OUString const & _aModule)
    {
        rtl::Reference<ExtendedCacheLine> xModule = implExtended(internalGetModule(_aModule));

    return xModule.is() && xModule->hasPending();
    }

// -----------------------------------------------------------------------------

    void ExtendedCacheData::findPendingModules( std::vector< rtl::OUString > & _rPendingList )
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


