/*************************************************************************
 *
 *  $RCSfile: cacheline.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-15 11:48:53 $
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
/*    using namespace configuration;
    // =========================================================================
    struct ONodeIdCollector : public NodeAction
    {
        typedef std::set< rtl::OUString, UStringLess > NodeIdSet;
        NodeIdSet ids;

        ONodeIdCollector() : ids() { }

        virtual void handle(ValueNode const& _rValueNode) { /* not interested in value nodes * / }
        virtual void handle(ISubtree const& _rSubtree) { ids.insert(_rSubtree.getId()); applyToChildren(_rSubtree); }
    };

    //==========================================================================
    static
    inline
    INode* findExistingChild(ISubtree* pCurrentParent, Path::Component const & _aName)
    {
        OSL_ASSERT(pCurrentParent);

        INode * pChild = pCurrentParent->getChild(_aName.getName().toString());

        if (!pChild && !_aName.isSimpleName())
        {
            pChild = pCurrentParent->getChild(_aName.toPathString());
            OSL_ENSURE(!pChild, "TreeData trouble: Existing node found only by composite name");
        }

        return pChild;
    }
    //==========================================================================
    static
    inline
    INode* findExistingChild(ISubtree* pCurrentParent, OUString const & _aName)
    {
        OSL_ASSERT(pCurrentParent);

        INode * pChild = pCurrentParent->getChild(_aName );

        return pChild;
    }
    //==========================================================================

    static sal_Int16 childLevel(sal_Int16 _nLevel)
    {
        OSL_ASSERT(0 > ITreeProvider::ALL_LEVELS);
        return (_nLevel > 0) ? _nLevel-1 : _nLevel;
    }

    //==========================================================================
    static bool deeper(sal_Int16 lhs, sal_Int16  rhs)
    {
        if (rhs == ITreeProvider::ALL_LEVELS) return false;
        if (lhs == ITreeProvider::ALL_LEVELS) return true;
        return lhs > rhs;
    }
    */
    //==========================================================================
    //= OCompleteTree
    //==========================================================================
    /** completes the cache with missing subelements
    */
/*  class OCompleteTree : private NodeModification
    {
    public:
        typedef sal_Int16 Level;

        static bool complete(ISubtree& _rExistingSubtree, ISubtree& _rNewSubtree,
                                Level _nNewDepth, Level _nDefaultDepth);
    private:
        OCompleteTree(ISubtree* pSubtree, Level _nParentLevel, Level _nParentDefaultLevel)
            :m_pCacheSubtree(pSubtree)
            ,m_nChildLevel( childLevel(_nParentLevel) )
            ,m_nDefaultLevel( childLevel(_nParentDefaultLevel) )
        {
            OSL_ENSURE(m_pCacheSubtree,"OCompleteTree: Need a tree to complete");

            OSL_ENSURE(m_pCacheSubtree->hasId(), "WARNING: Completing subtree without ID");
        }

        virtual void handle(ValueNode& _rNode);
        virtual void handle(ISubtree& _rSubtree);

    private:
        ISubtree* m_pCacheSubtree;
        sal_Int16 m_nChildLevel;
        sal_Int16 m_nDefaultLevel;
    };
    //----------------------------------------------------------------------
    void OCompleteTree::handle(ValueNode& _rNode)
    {
        OSL_ASSERT(m_pCacheSubtree);

        OUString aNodeName = _rNode.getName();
        INode*   pChild    = m_pCacheSubtree->getChild(aNodeName);
        // only not existing nodes are interesting other should be in the cache
        if (!pChild)
        {
            std::auto_ptr<INode> pNewChild( _rNode.clone() );

            pChild = m_pCacheSubtree->addChild(pNewChild);
        }
        else
        {
            OSL_ENSURE(pChild->ISA(ValueNode), "OCompleteTree: Node type mismatch");
            if (ValueNode* pValue = pChild->asValueNode())
            {
                if (m_nDefaultLevel != 0)
                    pValue->changeDefault(_rNode.getDefault());

                OSL_ENSURE(!m_nChildLevel || pValue->getValue() == _rNode.getValue(),
                            "WARNING: Cached value differs from data being spliced");
            }
        }
    }
    //----------------------------------------------------------------------

    void OCompleteTree::handle(ISubtree& _rSubtree)
    {
        OSL_ASSERT(m_pCacheSubtree);

        OUString aNodeName = _rSubtree.getName();
        INode*   pChild    = m_pCacheSubtree->getChild(aNodeName);

        // now we have different possibilites
        // a.) the node does not exist than clone the subtree and add it to the cache tree
        if (!pChild)
        {
            std::auto_ptr<INode> pNewChild( _rSubtree.clone() );

            ISubtree* pChildTree = pNewChild->asISubtree();

            OSL_ASSERT(pChildTree);
            OSL_ASSERT(!pChildTree->hasId()); // should be cleared by cloning a subtree

            OSL_ENSURE(_rSubtree.hasId() || m_pCacheSubtree->hasId(), "WARNING: OCompleteTree: completing a tree without ID");
            OUString const aTreeId = _rSubtree.hasId() ? _rSubtree.getId() : m_pCacheSubtree->getId();

            OIdPropagator::propagateIdToTree(aTreeId, *pChildTree);

            pChild = m_pCacheSubtree->addChild(pNewChild);

            OSL_ASSERT(pChild && pChild->asISubtree());
            OSL_ENSURE(pChild->asISubtree()->getId()== aTreeId, "OCompleteTree: Tree lost ID while being added");
        }
        else
        {
            ISubtree* pSubTree = pChild->asISubtree();
            OSL_ENSURE(pSubTree, "OCompleteTree::handle : node must be a inner node!");

            // b.) the node does exist with level all or greater level -> nothing to do
            // c.) the node does exist but with smaller level
            if (pSubTree)
            {
                complete(*pSubTree, _rSubtree, m_nChildLevel, m_nDefaultLevel);

                OSL_ENSURE(pSubTree->hasId(), "OCompleteTree: Completed Tree has no ID");
            }
            // else throw something (?)
        }
    }
    //----------------------------------------------------------------------
    bool OCompleteTree::complete(ISubtree& _rExistingSubtree, ISubtree& _rNewSubtree,
                                    Level _nNewDepth, Level _nDefaultDepth)
    {

        OSL_ENSURE(_rNewSubtree.hasId(),"WARNING: Completing subtree without providing an ID");

        Level const nExistingLevels     = _rExistingSubtree.getLevel();
        Level const nExistingDefaults   = _rExistingSubtree.getDefaultsLevel();
        if (!_rExistingSubtree.hasId())
        {
            // if it has levels, it surely was really loaded (and therefore has an ID)
            OSL_ENSURE(nExistingLevels   == 0,"ERROR: Dummy subtree has nonzero levels");
            OSL_ENSURE(nExistingDefaults == 0,"ERROR: Dummy subtree has nonzero default levels");
            OSL_ENSURE(_nNewDepth != 0,"ERROR: Merging default for data that isn't loaded");

            // the tree wasn't loaded before
            OSL_ENSURE(!_rExistingSubtree.getElementTemplateName().getLength(),
                        "WARNING: Dummy tree has child template");
            OSL_ENSURE(!_rExistingSubtree.getElementTemplateName().getLength() ||  _rNewSubtree.getElementTemplateName().getLength(),
                        "ERROR: Dummy tree with child template replaced by node without child template");
            OSL_ENSURE(_rExistingSubtree.getName() ==  _rNewSubtree.getName(),
                        "ERROR: Dummy tree renamed by completion (?!)");

            // bad hack: abuse of assignment operator to update a slice of _rExistingSubtree
            _rExistingSubtree = _rNewSubtree;

            // id is copied by assignment (but not by copy ctor)
            OSL_ASSERT(_rExistingSubtree.getId() ==  _rNewSubtree.getId());
            // _rExistingSubtree.setId(_rNewSubtree.getId();
        }
        else
        {
            OSL_ENSURE(_rExistingSubtree.getName() == _rNewSubtree.getName(),
                        "ERROR: Existing tree replaced by node with different name");

            OSL_ENSURE(_rExistingSubtree.getElementTemplateName() == _rNewSubtree.getElementTemplateName(),
                        "ERROR: Existing tree replaced by node with different child template");
        }

        bool bAddingData        = deeper(_nNewDepth,nExistingLevels);
        bool bAddingDefaults    = deeper(_nDefaultDepth,nExistingDefaults);
        bool bAdding = bAddingData || bAddingDefaults;

        if (bAdding)
        {
            // now do it
            OCompleteTree aCompleter(&_rExistingSubtree, _nNewDepth, _nDefaultDepth);
            aCompleter.applyToChildren(_rNewSubtree);
            //_rExistingSubtree.setLevel(_nNewDepth); // don't do it here, as this is recursive
        }
        // else nothing to do (well, we should check/refresh - at least in DEBUG)

        OSL_ENSURE(_rExistingSubtree.hasId(), "OCompleteTree: Completed Tree lost ID");

        return bAdding; // true only if we actually did something
    }
*/
// -----------------------------------------------------------------------------
    static inline CacheLine::Name implExtractModuleName(CacheLine::Path const& _aConfigPath)
    {
        return _aConfigPath.getModuleName();
    }


// -----------------------------------------------------------------------------
// class CacheLine
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

    CacheLine::CacheLine(Name const & _aModuleName, memory::HeapManager & _rHeapImpl)
    : m_storage(_rHeapImpl  )
    , m_name(_aModuleName)
    , m_base()
    , m_nDataRefs(0)
    {
    }
// -----------------------------------------------------------------------------

    CacheLine::CacheLine(Name const & _aModuleName, memory::HeapManager & _rHeapImpl,
                            memory::SegmentAddress const & _aLocation)
    : m_storage(_rHeapImpl,_aLocation.segment)
    , m_name(_aModuleName)
    , m_base( memory::Pointer(_aLocation.address) )
    , m_nDataRefs(0)
    {
    }
// -----------------------------------------------------------------------------

    void CacheLine::setBase(data::TreeAddress _base)
    {
        OSL_PRECOND(!m_base.is(), "CacheLine: Data base address was already set");
        OSL_PRECOND(  _base.is(), "CacheLine: Cannot set NULL base address");
        m_base = _base;
    }
// -----------------------------------------------------------------------------

    CacheLineRef CacheLine::createAttached( Name const & _aModuleName,
                                            memory::HeapManager & _rHeapImpl,
                                            memory::SegmentAddress const & _aLocation
                                          ) CFG_UNO_THROW_RTE(  )
    {
        if (_aModuleName.isEmpty())
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }
        if (_aLocation.address == 0)
        {
            OSL_ENSURE(false, "Cannot attach a cache line to a NULL address");
            return NULL;
        }

        CacheLineRef xResult = new CacheLine(_aModuleName,_rHeapImpl,_aLocation);

        return xResult;
    }
// -----------------------------------------------------------------------------

    CacheLine::Name CacheLine::getModuleName() const
    {
        return m_name;
    }
// -----------------------------------------------------------------------------

    data::TreeAddress CacheLine::getPartialTree(memory::Accessor const & _aAccessor, Path const& aConfigName) const
    {
        data::SetNodeAccess aParentSet( internalGetNode(_aAccessor, aConfigName.getParentPath()) );

        if (aParentSet.isValid())
            return aParentSet.getElementTree(aConfigName.getLocalName().getName()).address();
        else
            return data::TreeAddress();
    }
// -----------------------------------------------------------------------------

    data::NodeAccess CacheLine::internalGetNode(memory::Accessor const & _anAccessor, Path const& aConfigName) const
    {
        OSL_ENSURE( m_base.is(), "Cannot get a node from a dataless module");

        data::TreeAccessor aModuleTree(_anAccessor, m_base);
        OSL_ASSERT( aModuleTree.isValid());

        data::NodeAccess aNode = aModuleTree.getRootNode();
        OSL_ENSURE( aNode.isValid(), "CacheLine contains no nodes");

        Path::Iterator it = aConfigName.begin();
        OSL_ENSURE( it != aConfigName.end(), "Empty Path can't match any module");
        OSL_ENSURE( aNode.getName() == it->getInternalName(), "Module part in config path does not match selected CacheLine");

        // find child of node
        // might be done using a visitor
        while(aNode.isValid() && ++it != aConfigName.end())
        {
            aNode = data::getSubnode(aNode,it->getName());
        }
        return aNode;
    }
// -----------------------------------------------------------------------------

    data::NodeAddress CacheLine::getNode(memory::Accessor const & _aAccessor, Path const& aConfigName) const
    {
        data::NodeAccess aNode = internalGetNode(_aAccessor, aConfigName);
        return aNode.address();
    }
// -------------------------------------------------------------------------

    data::TreeAddress CacheLine::setComponentData( memory::UpdateAccessor& _aAccessToken,
                                                           backend::NodeInstance & _aNodeInstance,
                                                           bool _bWithDefaults
                                                         ) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aNodeInstance.node.get(), "CacheLine::insertDefaults: inserting NULL defaults !");
        OSL_PRECOND(_aNodeInstance.root.isModuleRoot(), "Should have complete component to fill cache");
        OSL_PRECOND(_aNodeInstance.root.getModuleName() == this->getModuleName(),"Data location does not match module");

        OSL_PRECOND(!base().is(), "Data is already loaded");

        if (!base().is()) // no data yet
        {
            this->setBase( data::buildTree(_aAccessToken, _aNodeInstance.node->getName(), *_aNodeInstance.node, _bWithDefaults) );
        }

        return this->base();
    }
// -----------------------------------------------------------------------------

    data::TreeAddress CacheLine::insertDefaults( memory::UpdateAccessor& _aAccessToken,
                                                 backend::NodeInstance & _aDefaultInstance
                                               ) CFG_UNO_THROW_RTE(  )
    {
        OSL_PRECOND(_aDefaultInstance.node.get(), "CacheLine::insertDefaults: inserting NULL defaults !");
        OSL_PRECOND(_aDefaultInstance.root.isModuleRoot(), "Should have complete component to fill tree with defaults");
        OSL_PRECOND(_aDefaultInstance.root.getModuleName() == this->getModuleName(),"Data location does not match module");

        OSL_PRECOND(m_base.is(), "Data must already be loaded to insert defaults");

        if (m_base.is())
        {
            data::mergeDefaults(_aAccessToken,m_base,*_aDefaultInstance.node);
        }

        return m_base;
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    ExtendedCacheLine::ExtendedCacheLine(Name const & _aModuleName, memory::HeapManager & _rHeapImpl)
    : CacheLine(_aModuleName,_rHeapImpl)
    , m_pPending()
    {
    }

// -----------------------------------------------------------------------------

    ExtendedCacheLine::ExtendedCacheLine(Name const & _aModuleName, memory::HeapManager & _rHeapImpl,
                                            memory::SegmentAddress const & _aLocation)
    : CacheLine(_aModuleName,_rHeapImpl,_aLocation)
    , m_pPending()
    {
    }

// -----------------------------------------------------------------------------

    ExtendedCacheLineRef ExtendedCacheLine::createAttached( Name const & _aModuleName,
                                                            memory::HeapManager & _rHeapImpl,
                                                            memory::SegmentAddress const & _aLocation
                                                          ) CFG_UNO_THROW_RTE(  )
    {
        if (_aModuleName.isEmpty())
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }
        if (_aLocation.address == 0)
        {
            OSL_ENSURE(false, "Cannot attach a cache line to a NULL address");
            return NULL;
        }

        ExtendedCacheLineRef xResult = new ExtendedCacheLine(_aModuleName,_rHeapImpl,_aLocation);

        return xResult;
    }
// -----------------------------------------------------------------------------

    ExtendedCacheLineRef ExtendedCacheLine::createNew(  Name const & _aModuleName,
                                                        memory::HeapManager & _rHeapImpl
                                                      ) CFG_UNO_THROW_RTE(  )
    {
        if (_aModuleName.isEmpty())
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }

        ExtendedCacheLineRef xResult = new ExtendedCacheLine(_aModuleName,_rHeapImpl);

        return xResult;
    }
// -------------------------------------------------------------------------

    void ExtendedCacheLine::addPending(backend::UpdateInstance const & _anUpdate) CFG_UNO_THROW_RTE(  )
    {
        Path aRootLocation = _anUpdate.root.location();

        OSL_PRECOND(!aRootLocation.isRoot(),"Pending change cannot be located at root");
        OSL_PRECOND(aRootLocation.getModuleName() == this->getModuleName(),"Pending change location does not match module");

        OSL_PRECOND(_anUpdate.update.get() != NULL,"Adding NULL 'pending' change");
        OSL_PRECOND(_anUpdate.update->getNodeName() == aRootLocation.getLocalName().getName().toString(),
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

        auto_ptr<SubtreeChange> pAddedChange( new SubtreeChange(*_anUpdate.update, SubtreeChange::DeepChildCopy()) );

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


