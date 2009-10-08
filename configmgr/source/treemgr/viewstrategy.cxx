/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewstrategy.cxx,v $
 * $Revision: 1.13 $
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

#include "viewstrategy.hxx"
#include "valuenodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "setnodeimpl.hxx"
#include "change.hxx"
#include "nodevisitor.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "nodeconverter.hxx"
//-----------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace view
    {
//-----------------------------------------------------------------------------
        static
        inline
        sharable::ValueNode * getMemberValueAccess( GroupNode const & _aGroupNode, rtl::OUString const & _aName )
        {
            configuration::GroupNodeImpl* pGroupData = _aGroupNode.get_impl();
            return pGroupData->getOriginalValueNode(_aName);
        }

//-----------------------------------------------------------------------------
        void ViewStrategy::checkInstance(configuration::Tree * tree) const
        {
            (void) tree; // avoid warnings
            OSL_ENSURE( getViewBehavior(tree).get() == this,
                        "Tree operation dispatched to wrong strategy instance");
        }
//-----------------------------------------------------------------------------
        void ViewStrategy::collectChanges(configuration::Tree * tree, configuration::NodeChanges& rChanges) const
        {
            checkInstance(tree);
            doCollectChanges( getRootNode(tree), rChanges );
        }

        bool ViewStrategy::hasChanges(configuration::Tree * tree) const
        {
            checkInstance(tree);
            return hasChanges( getRootNode(tree) );
        }

        // mark the given node and all its ancestors (we can stop when we hit a node that already is marked)
        void ViewStrategy::markChanged(Node const& _aNode)
        {
            configuration::Tree * tree = _aNode.tree();
            checkInstance(tree);

            Node aNode = _aNode;
            if (aNode.is())
            {
                do
                {
                    this->doMarkChanged(aNode);

                    aNode = aNode.getParent();
                }
                while (aNode.is() && !this->hasChanges( aNode ));
            }

            if (!aNode.is()) // just marked the root
            {
                configuration::Tree* pContext = tree->getContextTree();
                unsigned int nContext = tree->getContextNode();
                if (pContext)
                {
                    OSL_ASSERT(pContext->isValidNode(nContext));

                    view::Node aContextNode(pContext,nContext);
                    pContext->getViewBehavior()->markChanged(aContextNode);
                }
            }
        }

//-----------------------------------------------------------------------------
        std::auto_ptr<SubtreeChange> ViewStrategy::preCommitChanges(configuration::Tree * tree, std::vector< rtl::Reference<configuration::ElementTree> >& _rRemovedElements)
        {
            checkInstance(tree);
            return doPreCommitChanges( tree, _rRemovedElements);
        }

        void ViewStrategy::finishCommit(configuration::Tree * tree, SubtreeChange& rRootChange)
        {
            checkInstance(tree);
            doFinishCommit(tree, rRootChange);
        }

        void ViewStrategy::revertCommit(configuration::Tree * tree, SubtreeChange& rRootChange)
        {
            checkInstance(tree);
            doRevertCommit(tree, rRootChange);
        }

        void ViewStrategy::recoverFailedCommit(configuration::Tree * tree, SubtreeChange& rRootChange)
        {
            checkInstance(tree);
            doFailedCommit(tree, rRootChange);
        }

//-----------------------------------------------------------------------------
        void ViewStrategy::adjustToChanges(configuration::NodeChangesInformation& rLocalChanges, Node const& _aNode, SubtreeChange const& aExternalChange)
        {
            OSL_PRECOND( isValidNode(_aNode), "ERROR: Valid node required for adjusting to changes" );
            OSL_PRECOND( getSimpleNodeName(_aNode) == aExternalChange.getNodeName(), "name of change does not match actual node" );

            checkInstance(_aNode.tree());

            configuration::Tree * pTreeData = _aNode.tree();

            if (_aNode.isSetNode())
            {
                OSL_ENSURE(aExternalChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

                   unsigned int nDepth = pTreeData->getRemainingDepth(_aNode.get_offset());

                implAdjustToElementChanges( rLocalChanges, SetNode(_aNode), aExternalChange, nDepth);
            }
            else if (_aNode.isGroupNode())
            {
                OSL_ENSURE(!aExternalChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

                GroupNode aGroupNode(_aNode);

                implAdjustToValueChanges(rLocalChanges, aGroupNode, aExternalChange);
                implAdjustToSubChanges(  rLocalChanges, aGroupNode, aExternalChange);
            }
            else // might occur on external change (?)
            {
                OSL_ENSURE(_aNode.isValueNode(), "Tree: Unknown node type to adjust to changes");

                OSL_ENSURE(_aNode.get_offset() == configuration::Tree::ROOT, "Tree: Unexpected node type - non-root value element");

                OSL_ENSURE(false,"ERROR: Change type does not match node: Trying to apply subtree change to value element.");
            }
        }

//-----------------------------------------------------------------------------

        void ViewStrategy::addLocalChangeHelper( configuration::NodeChangesInformation& rLocalChanges_, configuration::NodeChange const& aChange_)
        {
            aChange_.getChangeInfos(rLocalChanges_);
        }

//-----------------------------------------------------------------------------
        // TO DO: create CommitAction class, which is returned by precommit (if applicable)

        std::auto_ptr<SubtreeChange> ViewStrategy::doPreCommitChanges(configuration::Tree * tree, std::vector< rtl::Reference<configuration::ElementTree> >& )
        {
            (void) tree; // avoid warnings
            OSL_ENSURE(!hasChanges(getRootNode(tree)),"Unexpected changes in View");
            return std::auto_ptr<SubtreeChange>();
        }

        void ViewStrategy::doFinishCommit(configuration::Tree * tree, SubtreeChange& )
        {
            (void) tree; // avoid warnings
            OSL_ENSURE(!hasChanges(getRootNode(tree)),"Unexpected changes in View");
            OSL_ENSURE(false,"ERROR: Cannot finish commit for unexpected changes");
        }

        void ViewStrategy::doRevertCommit(configuration::Tree * tree, SubtreeChange& )
        {
            (void) tree; // avoid warnings
            OSL_ENSURE(!hasChanges(getRootNode(tree)),"Unexpected changes in View");
            OSL_ENSURE(false,"ERROR: Cannot revert commit for unexpected changes");
        }

        void ViewStrategy::doFailedCommit(configuration::Tree * tree, SubtreeChange& )
        {
            (void) tree; // avoid warnings
            OSL_ENSURE(!hasChanges(getRootNode(tree)),"Unexpected changes in View");
            OSL_ENSURE(false,"ERROR: Cannot recover commit for unexpected changes");
        }

//-----------------------------------------------------------------------------
        void ViewStrategy::implAdjustToElementChange(configuration::NodeChangesInformation& rLocalChanges, SetNode const& _aSetNode, Change const& rElementChange, unsigned int nDepth)
        {
            configuration::SetNodeImpl * pSetData = _aSetNode.get_impl();

            OSL_ENSURE( pSetData->implHasLoadedElements() , "Unexpected call: Processing element change in uninitialized set");

            rtl::OUString aName( rElementChange.getNodeName() );

            configuration::SetElementChangeImpl* pThisChange = 0;
            if (AddNode const * addNode = dynamic_cast< AddNode const *>(&rElementChange))
            {
                configuration::ElementTreeData aNewElement = pSetData->makeAdditionalElement(this, *addNode, nDepth);

                pThisChange = pSetData->doAdjustToAddedElement(aName, *addNode, aNewElement);
            }
            else if (RemoveNode const * removeNode = dynamic_cast< RemoveNode const * >(&rElementChange))
            {
                pThisChange = pSetData->doAdjustToRemovedElement(aName, *removeNode);
            }
            else
            {
                if (nDepth > 0 || (NULL != pSetData->doFindElement(aName)) )// found even beyond nDepth ?
                {
                    pThisChange = pSetData->doAdjustChangedElement(rLocalChanges,aName, rElementChange);
                }
            }

            if (pThisChange)
            {
                addLocalChangeHelper( rLocalChanges, configuration::NodeChange(pThisChange) );
            }
        }

        void ViewStrategy::implAdjustToElementChanges(configuration::NodeChangesInformation& rLocalChanges, SetNode const& _aSetNode, SubtreeChange const& rExternalChanges, unsigned int nDepth)
        {

            if (nDepth > 0)
            {
                configuration::SetNodeImpl * pSetData = _aSetNode.get_impl();

                OSL_ENSURE( pSetData->getTemplateProvider().isValid(), "Cannot adjust SetNode to changes - node was never initialized" );

                if (pSetData->implHasLoadedElements())
                {
                    unsigned int const nElementDepth = configuration::childDepth(nDepth);
                    for (SubtreeChange::ChildIterator it = rExternalChanges.begin(); it != rExternalChanges.end(); ++it)
                    {
                        this->implAdjustToElementChange(rLocalChanges, _aSetNode, *it, nElementDepth);
                    }
                }
                else
                {
                    OSL_ENSURE( !hasChanges(_aSetNode.node()),"Cannot have changes to consider when no elements are loaded");

                    pSetData->convertChanges( rLocalChanges, rExternalChanges, nDepth);
                }
            }
        }

        configuration::ValueChangeImpl* ViewStrategy::doAdjustToValueChange(GroupNode const& _aGroupNode, rtl::OUString const& _aName, ValueChange const& _rExternalChange)
        {
            configuration::ValueChangeImpl* pChangeImpl = NULL;

            sharable::ValueNode * localNode = getMemberValueAccess(_aGroupNode,_aName);
            if (localNode != 0)
            {
                switch( _rExternalChange. getMode() )
                {
                case ValueChange::wasDefault:
                case ValueChange::changeValue:
                    pChangeImpl = new configuration::ValueReplaceImpl( _rExternalChange.getNewValue(), _rExternalChange.getOldValue() );
                    break;

                case ValueChange::setToDefault:
                    pChangeImpl = new configuration::ValueResetImpl( _rExternalChange.getNewValue(), _rExternalChange.getOldValue() );
                    break;

                default: OSL_ENSURE(false, "Unknown change mode");
                    // fall thru to next case for somewhat meaningful return value
                case ValueChange::changeDefault:
                    {
                        com::sun::star::uno::Any aLocalValue = localNode->getValue();

                        pChangeImpl = new configuration::ValueReplaceImpl( aLocalValue, aLocalValue );
                    }
                    break;
                }
                OSL_ASSERT( pChangeImpl );
            }
            else
            {
                OSL_ENSURE(false, "ERROR: Notification tries to change nonexistent value within group");
            }

            return pChangeImpl;
        }

        void ViewStrategy::implAdjustToValueChanges(configuration::NodeChangesInformation& rLocalChanges, GroupNode const& _aGroupNode, SubtreeChange const& rExternalChanges)
        {
            for (SubtreeChange::ChildIterator it = rExternalChanges.begin(); it != rExternalChanges.end(); ++it)
            {
                if (ValueChange const * valueChange = dynamic_cast< ValueChange const * >(&*it))
                {
                    rtl::OUString aValueName( valueChange->getNodeName() );

                    if (configuration::ValueChangeImpl* pThisChange = doAdjustToValueChange(_aGroupNode, aValueName, *valueChange))
                    {
                        pThisChange->setTarget(_aGroupNode,aValueName);
                        addLocalChangeHelper(rLocalChanges, configuration::NodeChange(pThisChange));
                    }
                    else
                        OSL_TRACE("WARNING: Configuration: derived class hides an external value member change from listeners");
                }
                else
                    OSL_ENSURE(dynamic_cast< SubtreeChange const * >(&*it) != 0, "Unexpected change type within group");
            }
        }

        void ViewStrategy::implAdjustToSubChanges(configuration::NodeChangesInformation& rLocalChanges, GroupNode const& _aGroupNode, SubtreeChange const& rExternalChanges)
        {
#if (OSL_DEBUG_LEVEL > 0)
            configuration::Tree * pTreeData = _aGroupNode.tree();
#endif
            for(SubtreeChange::ChildIterator it = rExternalChanges.begin(); it != rExternalChanges.end(); ++it)
            {
                if (SubtreeChange const * subtreeChange = dynamic_cast< SubtreeChange const * >(&*it))
                {
                    Node aSubNode = _aGroupNode.findChild( it->getNodeName() );
                    OSL_ENSURE( aSubNode.is() || pTreeData->depthTo(_aGroupNode.node().get_offset()) >= pTreeData->getAvailableDepth(), "Changed node not found in tree");

                    if (aSubNode.is())
                    {
                        OSL_ENSURE( pTreeData->getRemainingDepth(_aGroupNode.node().get_offset()) > 0, "Depth is smaller than expected for tree");
                        this->adjustToChanges(rLocalChanges, aSubNode, *subtreeChange);
                    }
                }
                else
                {
                    OSL_ENSURE(dynamic_cast< ValueChange const * >(&*it) != 0, "Unexpected change type for child of group node; change is ignored");
                    OSL_ENSURE( !_aGroupNode.findChild(it->getNodeName()).is(),
                                "Found sub(tree) node where a value was expected");
                }
            }
        }
//-----------------------------------------------------------------------------
        void ViewStrategy::doCollectChanges(Node const& _aNode, configuration::NodeChanges& ) const
        {
            { (void)_aNode; }
            // no-op: there are no changes to collect
            OSL_ENSURE(!hasChanges(_aNode),"Unexpected changes in View");
        }


//-----------------------------------------------------------------------------
        com::sun::star::uno::Any    ViewStrategy::getValue(ValueNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getValue();
        }
#if OSL_DEBUG_LEVEL > 0
        com::sun::star::uno::Type ViewStrategy::getValueType(ValueNode const& _aNode)   const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getValueType();
        }
#endif
//-----------------------------------------------------------------------------
// group member access

//-----------------------------------------------------------------------------
    namespace { // helpers
        struct GroupMemberDispatch : data::NodeVisitor
        {
            GroupMemberDispatch(ViewStrategy& _rStrategy, GroupNode const& _aGroup, configuration::GroupMemberVisitor& rVisitor)
            : m_rStrategy(_rStrategy)
            , m_aGroup(_aGroup)
            , m_rVisitor(rVisitor)
            {}

            static bool mapResult(configuration::GroupMemberVisitor::Result _aResult)
            {
                return _aResult == configuration::GroupMemberVisitor::DONE;
            }

            static configuration::GroupMemberVisitor::Result unmapResult(bool done)
            {
                return done
                    ? configuration::GroupMemberVisitor::DONE
                    : configuration::GroupMemberVisitor::CONTINUE;
            }

            using NodeVisitor::handle;
            virtual bool handle(sharable::Node * node);
            virtual bool handle(sharable::ValueNode * node);
#if (OSL_DEBUG_LEVEL > 0)
            bool test_value(sharable::Node * node) const;
#endif
            ViewStrategy&       m_rStrategy;
            GroupNode           m_aGroup;
            configuration::GroupMemberVisitor& m_rVisitor;

            configuration::GroupMemberVisitor::Result m_aResult;
        };

#if (OSL_DEBUG_LEVEL > 0)
        bool GroupMemberDispatch::test_value(sharable::Node * node) const
        {
            return m_rStrategy.hasValue(m_aGroup, node->getName());
        }
#endif

        bool GroupMemberDispatch::handle(sharable::ValueNode * node)
        {
            OSL_ENSURE( test_value(sharable::node(node)), "ERROR: Group MemberDispatch:Did not find a ValueMember for a value child.");

            rtl::OUString aValueName = node->info.getName();

            return mapResult( m_rVisitor.visit( m_rStrategy.getValue(m_aGroup,aValueName) ) );
        }

        bool GroupMemberDispatch::handle(sharable::Node * node)
        {
            (void) node; // avoid warnings
            OSL_ENSURE( !test_value(node), "ERROR: Group MemberDispatch:Found a ValueMember for a subtree child.");

            return false;
        }
    }
//-----------------------------------------------------------------------------
        configuration::ValueMemberNode ViewStrategy::doGetValueMember(GroupNode const& _aNode, rtl::OUString const& _aName, bool ) const
        {
            sharable::ValueNode * valueData = getMemberValueAccess(_aNode,_aName);
            return _aNode.get_impl()->makeValueMember(valueData);
        }

        bool ViewStrategy::hasValue(GroupNode const& _aNode, rtl::OUString const& _aName) const
        {
            checkInstance(_aNode.tree());
            return getMemberValueAccess(_aNode,_aName) != 0;
        }

        bool ViewStrategy::hasValue(GroupNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            configuration::GroupNodeImpl* pGroupNode=_aNode.get_impl();
            sharable::GroupNode * group = pGroupNode->getDataAccess();
            return group->numDescendants > 0;
        }


        bool ViewStrategy::areValueDefaultsAvailable(GroupNode const& _aNode) const
        {
            checkInstance(_aNode.tree());

            return  _aNode.get_impl()->areValueDefaultsAvailable();
        }

        configuration::ValueMemberNode ViewStrategy::getValue(GroupNode const& _aNode, rtl::OUString const& _aName) const
        {
            checkInstance(_aNode.tree());
            return doGetValueMember(_aNode,_aName,false);
        }

        configuration::ValueMemberUpdate ViewStrategy::getValueForUpdate(GroupNode const & _aNode, rtl::OUString const& _aName)
        {
            checkInstance(_aNode.tree());
            return configuration::ValueMemberUpdate( doGetValueMember(_aNode,_aName,true), *this );
        }

        configuration::GroupMemberVisitor::Result ViewStrategy::dispatchToValues(GroupNode const& _aNode, configuration::GroupMemberVisitor& _aVisitor)
        {
            checkInstance(_aNode.tree());

            GroupMemberDispatch aDispatch(*this,_aNode,_aVisitor);

            bool done = aDispatch.visitChildren( _aNode.getAccess() );

            return aDispatch.unmapResult(done);
        }

//-----------------------------------------------------------------------------
        configuration::ElementTreeData ViewStrategy::implMakeElement(SetNode const& _aNode, configuration::SetEntry const& anEntry) const
        {
            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();
            return pNodeData->implValidateElement(pNodeData->entryToElement(anEntry));
        }
//-----------------------------------------------------------------------------
        configuration::SetEntry ViewStrategy::implFindElement(SetNode const& _aNode, rtl::OUString const& aName) const
        {
            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();

            OSL_ENSURE(pNodeData->implHasLoadedElements(),"Cannot find elements in set that is not loaded");
            configuration::ElementTree * pElement = pNodeData->doFindElement(aName);

            return configuration::SetEntry(pElement);
        }

        configuration::SetEntry ViewStrategy::findElement(SetNode const& _aNode, rtl::OUString const& aName) const
        {
            checkInstance(_aNode.tree());
            _aNode.get_impl()->implEnsureElementsLoaded();
            return implFindElement(_aNode,aName);
        }

        configuration::SetEntry ViewStrategy::findAvailableElement(SetNode const& _aNode, rtl::OUString const& aName) const
        {
            checkInstance(_aNode.tree());
            if (_aNode.get_impl()->implHasLoadedElements())
                return implFindElement(_aNode,aName);
            else
                return configuration::SetEntry(0);
        }

        static
        inline
        std::auto_ptr<SubtreeChange> makeChangeToDefault(sharable::SetNode * setNode)
        {
            return std::auto_ptr<SubtreeChange>(
                        new SubtreeChange(
                            setNode->info.getName(),
                            setNode->getElementTemplateName(),
                            setNode->getElementTemplateModule(),
                            sharable::node(setNode)->getAttributes(),
                            true // to default
                   )    );
        }

        std::auto_ptr<SubtreeChange> ViewStrategy::differenceToDefaultState(SetNode const& _aNode, ISubtree& _rDefaultTree) const
        {
            checkInstance(_aNode.tree());
            std::auto_ptr<SubtreeChange> aResult;

            sharable::SetNode * originalSetNode = _aNode.getAccess();
            OSL_ASSERT(originalSetNode != 0);
            if (!originalSetNode->info.isDefault())
            {
                aResult = makeChangeToDefault(originalSetNode);

                configuration::SetNodeImpl * pNodeData = _aNode.get_impl();
                if (this->hasChanges(_aNode.node()))
                {
                    OSL_ENSURE(pNodeData->implHasLoadedElements(),"Unexpected: Found set with changes but elements are not loaded");
                    pNodeData->doDifferenceToDefaultState(*aResult,_rDefaultTree);
                }
                else
                    pNodeData->implDifferenceToDefaultState(*aResult,_rDefaultTree);
            }
            return aResult;
        }

        rtl::Reference<configuration::Template> ViewStrategy::getElementTemplate(SetNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getElementTemplate();
        }

        configuration::TemplateProvider ViewStrategy::getTemplateProvider(SetNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getTemplateProvider();
        }

        node::Attributes ViewStrategy::getNodeAttributes(Node const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.getAccessRef()->getAttributes();
        }

//-----------------------------------------------------------------------------
        configuration::SetNodeVisitor::Result ViewStrategy::dispatchToElements(SetNode const& _aNode, configuration::SetNodeVisitor& _aVisitor)
        {
            checkInstance(_aNode.tree());

            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();

            if (pNodeData->implLoadElements())
                return pNodeData->doDispatchToElements(_aVisitor);

            else
                return configuration::SetNodeVisitor::CONTINUE;
        }

        bool ViewStrategy::isEmpty(SetNode const& _aNode) const
        {
            checkInstance(_aNode.tree());

            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();

            return !pNodeData->implLoadElements() || pNodeData->doIsEmpty();
        }
//-----------------------------------------------------------------------------

        void ViewStrategy::insertElement(SetNode const& _aNode, rtl::OUString const& _aName, configuration::SetEntry const& _aNewEntry)
        {
            // cannot insert, if we cannot check for collisions
            checkInstance(_aNode.tree());
            _aNode.get_impl()->implEnsureElementsLoaded();
            doInsertElement(_aNode,_aName,_aNewEntry);
        }

        void ViewStrategy::removeElement(SetNode const& _aNode, rtl::OUString const& _aName)
        {
            // cannot remove, if we cannot check for existance
            checkInstance(_aNode.tree());
            _aNode.get_impl()->implEnsureElementsLoaded();
            doRemoveElement(_aNode,_aName);
        }

//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

