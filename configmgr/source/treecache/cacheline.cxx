/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cacheline.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:36:37 $
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

#include "cacheline.hxx"

#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_BUILDDATA_HXX
#include "builddata.hxx"
#endif

#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
#endif
#ifndef INCLUDED_CONFIGMGR_MERGECHANGE_HXX
#include "mergechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    static inline CacheLine::Name implExtractModuleName(CacheLine::Path const& _aConfigPath)
    {
        return _aConfigPath.getModuleName();
    }


// -----------------------------------------------------------------------------
// class CacheLine
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

    CacheLine::CacheLine(Name const & _aModuleName)
    : m_base(NULL)
    , m_name(_aModuleName)
    , m_nDataRefs(0)
    {
    }
// -----------------------------------------------------------------------------

    CacheLine::CacheLine(Name const & _aModuleName, data::TreeAddress _pSegment)
    : m_base(_pSegment)
    , m_name(_aModuleName)
    , m_nDataRefs(0)
    {
    }

// -----------------------------------------------------------------------------

    void CacheLine::setBase(data::TreeAddress _base)
    {
        OSL_PRECOND(m_base == NULL, "CacheLine: Data base address was already set");
        OSL_PRECOND( _base != NULL, "CacheLine: Cannot set NULL base address");
        m_base = _base;
    }
// -----------------------------------------------------------------------------

    CacheLineRef CacheLine::createAttached( Name const & _aModuleName,
                                            data::TreeAddress _aSegment
                                          ) CFG_UNO_THROW_RTE(  )
    {
        if (_aModuleName.isEmpty())
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }
        if (_aSegment == NULL)
        {
            OSL_ENSURE(false, "Cannot attach a cache line to a NULL segment");
            return NULL;
        }

        CacheLineRef xResult = new CacheLine(_aModuleName,_aSegment);

        return xResult;
    }
// -----------------------------------------------------------------------------

    CacheLineRef CacheLine::createNew(  Name const & _aModuleName
                                      ) CFG_UNO_THROW_RTE(  )
    {
        if (_aModuleName.isEmpty())
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }

        CacheLineRef xResult = new CacheLine(_aModuleName);

        return xResult;
    }
// -------------------------------------------------------------------------

    CacheLine::Name CacheLine::getModuleName() const
    {
        return m_name;
    }
// -----------------------------------------------------------------------------

    data::TreeAddress CacheLine::getPartialTree(Path const& aConfigName) const
    {
        data::SetNodeAccess aParentSet( internalGetNode(aConfigName.getParentPath()) );

        if (aParentSet.isValid())
            return aParentSet.getElementTree(aConfigName.getLocalName().getName());
        else
            return NULL;
    }
// -----------------------------------------------------------------------------

    bool CacheLine::hasDefaults() const
    {
    if ( m_base != NULL ) return false; // cannot get defaults without data

        data::TreeAccessor aModuleTree(m_base);
    OSL_ASSERT( aModuleTree != NULL);

        return aModuleTree->hasDefaultsAvailable();
    }
// -----------------------------------------------------------------------------
    data::NodeAccess CacheLine::internalGetNode(Path const& aConfigName) const
    {
    OSL_ENSURE( m_base != NULL, "Cannot get a node from a dataless module");

        data::TreeAccessor aModuleTree(m_base);
    OSL_ASSERT(aModuleTree != NULL);

        data::NodeAccess aNode = aModuleTree.getRootNode();
    OSL_ENSURE( aNode.isValid(), "CacheLine contains no nodes");

    Path::Iterator it = aConfigName.begin();
    OSL_ENSURE( it != aConfigName.end(), "Empty Path can't match any module");
    OSL_ENSURE( aNode.isValid() && aNode.getName() == it->getInternalName(), "Module part in config path does not match selected CacheLine");

        // find child of node
        // might be done using a visitor
    while(aNode.isValid() && ++it != aConfigName.end())
    {
            aNode = data::getSubnode(aNode,it->getName());
    }
        return aNode;
    }
// -----------------------------------------------------------------------------

    data::NodeAddress CacheLine::getNode(Path const& aConfigName) const
    {
        data::NodeAccess aNode = internalGetNode(aConfigName);
        return aNode;
    }
// -------------------------------------------------------------------------

    data::TreeAddress CacheLine::setComponentData( backend::ComponentData const & _aComponentInstance,
                                                   bool _bWithDefaults
                                                   ) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aComponentInstance.data.get(), "CacheLine::insertDefaults: inserting NULL defaults !");
        OSL_PRECOND(_aComponentInstance.name == this->getModuleName(),"Data location does not match module");

        OSL_PRECOND(base() == NULL, "Data is already loaded");

        if (base() == NULL) // no data yet
        {
            this->setBase( data::buildTree(_aComponentInstance.data->getName(), *_aComponentInstance.data, _bWithDefaults) );
        }

        return this->base();
    }
// -----------------------------------------------------------------------------

    data::TreeAddress CacheLine::insertDefaults( backend::NodeInstance const & _aDefaultInstance
                                               ) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aDefaultInstance.data().get(), "CacheLine::insertDefaults: inserting NULL defaults !");
        OSL_PRECOND(_aDefaultInstance.root().isModuleRoot(), "Should have complete component to fill tree with defaults");
        OSL_PRECOND(_aDefaultInstance.root().getModuleName() == this->getModuleName(),"Data location does not match module");

        OSL_PRECOND(m_base != NULL, "Data must already be loaded to insert defaults");

        if (m_base != NULL)
        {
            data::mergeDefaults(m_base,*_aDefaultInstance.data());
        }

        return m_base;
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    ExtendedCacheLine::ExtendedCacheLine(Name const & _aModuleName)
    : CacheLine(_aModuleName)
    , m_pPending()
    {
    }

// -----------------------------------------------------------------------------

    ExtendedCacheLine::ExtendedCacheLine(Name const & _aModuleName,
                                         data::TreeAddress _aSegment)
    : CacheLine(_aModuleName,_aSegment)
    , m_pPending()
    {
    }

// -----------------------------------------------------------------------------

    ExtendedCacheLineRef ExtendedCacheLine::createAttached( Name const & _aModuleName,
                                                            data::TreeAddress _aSegment
                                                          ) CFG_UNO_THROW_RTE(  )
    {
        if (_aModuleName.isEmpty())
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }
        if (_aSegment == NULL)
        {
            OSL_ENSURE(false, "Cannot attach a cache line to a NULL segment");
            return NULL;
        }

        ExtendedCacheLineRef xResult = new ExtendedCacheLine(_aModuleName,_aSegment);

        return xResult;
    }
// -----------------------------------------------------------------------------

ExtendedCacheLineRef ExtendedCacheLine::createNew(  Name const & _aModuleName
                                                      ) CFG_UNO_THROW_RTE(  )
    {
        if (_aModuleName.isEmpty())
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }

        ExtendedCacheLineRef xResult = new ExtendedCacheLine(_aModuleName);

        return xResult;
    }
// -------------------------------------------------------------------------

    void ExtendedCacheLine::addPending(backend::ConstUpdateInstance const & _anUpdate) CFG_UNO_THROW_RTE(  )
    {
        Path aRootLocation = _anUpdate.root().location();

        OSL_PRECOND(!aRootLocation.isRoot(),"Pending change cannot be located at root");
        OSL_PRECOND(aRootLocation.getModuleName() == this->getModuleName(),"Pending change location does not match module");

        OSL_PRECOND(_anUpdate.data() != NULL,"Adding NULL 'pending' change");
        OSL_PRECOND(_anUpdate.data()->getNodeName() == aRootLocation.getLocalName().getName().toString(),
                    "Path to pending change does not match change name");

        using std::auto_ptr;

        // first make the _pSubtreeChange a full tree starting at the module root
        auto_ptr<SubtreeChange> pRootChange;
        SubtreeChange *pExistingEntry = NULL;

        Path::Iterator last = aRootLocation.end();

        OSL_ASSERT(last != aRootLocation.begin());
        --last;

        for (Path::Iterator it = aRootLocation.begin();
             it != last;
             ++it)
        {
            OSL_ASSERT( it   != aRootLocation.end());
            OSL_ASSERT( it+1 != aRootLocation.end());
            // We need to create a new SubtreeChange
            Name const aChangeName      = it->getName();
            Name const aElementTypeName = (it+1)->getTypeName();

            auto_ptr<SubtreeChange> pNewChange =
                        OTreeChangeFactory::createDummyChange(aChangeName, aElementTypeName);

            if (pExistingEntry == NULL)
            {
                OSL_ASSERT(pRootChange.get() == NULL);

                pRootChange     = pNewChange;
                pExistingEntry  = pRootChange.get();
            }
            else
            {
                OSL_ASSERT(pRootChange.get() != NULL);

                pExistingEntry->addChange(base_ptr(pNewChange));

                Change* pChange = pExistingEntry->getChange(aChangeName.toString());
                pExistingEntry = static_cast<SubtreeChange*>(pChange);

                OSL_ENSURE(pChange && pChange->ISA(SubtreeChange), "ERROR: Cannot recover change just added");
            }
        }

        auto_ptr<SubtreeChange> pAddedChange( new SubtreeChange(*_anUpdate.data(), SubtreeChange::DeepChildCopy()) );

        if (aRootLocation.getDepth() > 1)
        {
            OSL_ASSERT(pRootChange.get() != NULL && pExistingEntry != NULL);

            // the _pSubtreeChange did not start at root, so add its clone to the built dummies
            pExistingEntry->addChange(base_ptr(pAddedChange));
        }
        else
        {
            OSL_ASSERT(pRootChange.get() == NULL && pExistingEntry == NULL);

            // the _pSubtreeChange starts at root, so just reset pRootChange
            pRootChange = pAddedChange;
        }
        OSL_ASSERT(pRootChange.get() != NULL);

        if (m_pPending.get() == NULL)
        {
            // no merge is need, because the existing pending changes are empty
            m_pPending = pRootChange;
        }
        else
        {
            try
            {
                // We need to merge the new rebased changes into the m_pPending
                combineUpdates(*pRootChange,*m_pPending);
            }
            catch (configuration::Exception& e)
            {
                OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("Update cache for module: Could not add pending changes at"));

                sMessage += aRootLocation.toString();

                sMessage += OUString(RTL_CONSTASCII_USTRINGPARAM(". Internal Exception:")) + e.message();

                throw uno::RuntimeException(sMessage,0);
            }
        }

        OSL_POSTCOND(m_pPending.get() != NULL, "Could not insert new pending changes");
    }
// -----------------------------------------------------------------------------


} // namespace configmgr


