/*************************************************************************
 *
 *  $RCSfile: viewstrategy.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:33:39 $
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

#include "viewstrategy.hxx"

#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#include "setnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif
#ifndef CONFIGMGR_NODECONVERTER_HXX
#include "nodeconverter.hxx"
#endif
//-----------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace view
    {
//-----------------------------------------------------------------------------
        using configuration::SetEntry;
        using configuration::ElementList;
        using configuration::GroupMemberVisitor;
        using configuration::SetNodeVisitor;
//-----------------------------------------------------------------------------
// virtual void doInitElements( data::SetNodeAccess const& _aNode, TreeDepth nDepth);

        static
        inline
        data::ValueNodeAccess getMemberValueAccess( GroupNode const & _aGroupNode, Name const & _aName )
        {
            configuration::GroupNodeImpl* pGroupData = _aGroupNode.get_impl();
            return pGroupData->getOriginalValueNode(_aGroupNode.accessor(),_aName);
        }

//-----------------------------------------------------------------------------
        void ViewStrategy::checkInstance(Tree const& _aTreeForThis) const
        {
            OSL_ENSURE( getViewBehavior(_aTreeForThis).get() == this,
                        "Tree operation dispatched to wrong strategy instance");
        }
//-----------------------------------------------------------------------------
        void ViewStrategy::collectChanges(Tree const& _aTree, NodeChanges& rChanges)    const
        {
            checkInstance(_aTree);
            doCollectChanges( getRootNode(_aTree), rChanges );
        }

        bool ViewStrategy::hasChanges(Tree const& _aTree) const
        {
            checkInstance(_aTree);
            return hasChanges( getRootNode(_aTree) );
        }

        // mark the given node and all its ancestors (we can stop when we hit a node that already is marked)
        void ViewStrategy::markChanged(Node const& _aNode)
        {
            Tree aTree = _aNode.tree();
            checkInstance(aTree);

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
                configuration::TreeImpl* pTreeData = aTree.get_impl();
                configuration::TreeImpl* pContext = pTreeData->getContextTree();
                NodeOffset nContext = pTreeData->getContextNode();
                if (pContext)
                {
                    OSL_ASSERT(pContext->isValidNode(nContext));

                    view::Node aContextNode(aNode.accessor(),*pContext,nContext);
                    pContext->getViewBehavior()->markChanged(aContextNode);
                }
            }
        }

//-----------------------------------------------------------------------------
        data::NodeAddress::DataType * ViewStrategy::getDataForUpdate(data::NodeAccessRef const & _aNode)
        {
            typedef data::NodeAddress::DataType DataType;
            DataType * pResult = implAccessForUpdate(_aNode);
            OSL_ASSERT(!pResult || _aNode.getDataPtr() == pResult);
            return pResult;
        }

        data::SetNodeAddress::DataType * ViewStrategy::getDataForUpdate(data::SetNodeAccess const & _aNode)
        {
            typedef data::SetNodeAddress::DataType DataType;

            sharable::Node * pNode = implAccessForUpdate(_aNode);
            DataType * pResult = pNode ? pNode->setData() : 0;

            OSL_ASSERT(!pResult || &_aNode.data() == pResult);
            return pResult;
        }

        data::GroupNodeAddress::DataType * ViewStrategy::getDataForUpdate(data::GroupNodeAccess const & _aNode)
        {
            typedef data::GroupNodeAddress::DataType DataType;

            sharable::Node * pNode = implAccessForUpdate(_aNode);
            DataType * pResult = pNode ? pNode->groupData() : 0;

            OSL_ASSERT(!pResult || &_aNode.data() == pResult);
            return pResult;
        }

        data::ValueNodeAddress::DataType * ViewStrategy::getDataForUpdate(data::ValueNodeAccess const & _aNode)
        {
            typedef data::ValueNodeAddress::DataType DataType;

            sharable::Node * pNode = implAccessForUpdate(_aNode);
            DataType * pResult = pNode ? pNode->valueData() : 0;

            OSL_ASSERT(!pResult || &_aNode.data() == pResult);
            return pResult;
        }

//-----------------------------------------------------------------------------
        data::NodeAddress::DataType * ViewStrategy::implAccessForUpdate(data::NodeAccessRef const & _aNode)
        {
            if (memory::Segment * pUpdatableSegment = doGetDataSegmentForUpdate())
            {
                void * p = memory::UpdateAccessor(pUpdatableSegment).validate(_aNode.rawAddress());

                OSL_ASSERT(const_cast<const void *>(p) == _aNode.getDataPtr());

                return static_cast<data::NodeAddress::DataType*>(p);
            }

            else
                return NULL;
        }

//-----------------------------------------------------------------------------
        memory::Segment * ViewStrategy::doGetDataSegmentForUpdate()
        {
            return NULL;
        }


//-----------------------------------------------------------------------------
        std::auto_ptr<SubtreeChange> ViewStrategy::preCommitChanges(Tree const& _aTree, ElementList& _rRemovedElements)
        {
            checkInstance(_aTree);
            return doPreCommitChanges( _aTree, _rRemovedElements);
        }

        void ViewStrategy::finishCommit(Tree const& _aTree, SubtreeChange& rRootChange)
        {
            checkInstance(_aTree);
            doFinishCommit(_aTree, rRootChange);
        }

        void ViewStrategy::revertCommit(Tree const& _aTree, SubtreeChange& rRootChange)
        {
            checkInstance(_aTree);
            doRevertCommit(_aTree, rRootChange);
        }

        void ViewStrategy::recoverFailedCommit(Tree const& _aTree, SubtreeChange& rRootChange)
        {
            checkInstance(_aTree);
            doFailedCommit(_aTree, rRootChange);
        }

//-----------------------------------------------------------------------------
        void ViewStrategy::adjustToChanges(NodeChangesInformation& rLocalChanges, Node const& _aNode, SubtreeChange const& aExternalChange)
        {
            OSL_PRECOND( isValidNode(_aNode), "ERROR: Valid node required for adjusting to changes" );
            OSL_PRECOND( getSimpleNodeName(_aNode).toString() == aExternalChange.getNodeName(), "Name of change does not match actual node" );

            checkInstance(_aNode.tree());

            configuration::TreeImpl * pTreeData = _aNode.tree().get_impl();

            if (_aNode.isSetNode())
            {
                OSL_ENSURE(aExternalChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

                   TreeDepth nDepth = pTreeData->getRemainingDepth(_aNode.get_offset());

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
                OSL_ENSURE(_aNode.isValueNode(), "TreeImpl: Unknown node type to adjust to changes");

                OSL_ENSURE(_aNode.get_offset() == pTreeData->root_(), "TreeImpl: Unexpected node type - non-root value element");

                OSL_ENSURE(false,"ERROR: Change type does not match node: Trying to apply subtree change to value element.");
            }
        }

//-----------------------------------------------------------------------------

        void ViewStrategy::addLocalChangeHelper( NodeChangesInformation& rLocalChanges_, configuration::NodeChange const& aChange_)
        {
            aChange_.getChangeInfos(rLocalChanges_);
        }

//-----------------------------------------------------------------------------
        // TO DO: create CommitAction class, which is returned by precommit (if applicable)

        std::auto_ptr<SubtreeChange> ViewStrategy::doPreCommitChanges(Tree const& _aTree, configuration::ElementList& )
        {
            OSL_ENSURE(!hasChanges(getRootNode(_aTree)),"Unexpected changes in View");
            return std::auto_ptr<SubtreeChange>();
        }

        void ViewStrategy::doFinishCommit(Tree const& _aTree, SubtreeChange& )
        {
            OSL_ENSURE(!hasChanges(getRootNode(_aTree)),"Unexpected changes in View");
            OSL_ENSURE(false,"ERROR: Cannot finish commit for unexpected changes");
        }

        void ViewStrategy::doRevertCommit(Tree const& _aTree, SubtreeChange& )
        {
            OSL_ENSURE(!hasChanges(getRootNode(_aTree)),"Unexpected changes in View");
            OSL_ENSURE(false,"ERROR: Cannot revert commit for unexpected changes");
        }

        void ViewStrategy::doFailedCommit(Tree const& _aTree, SubtreeChange& )
        {
            OSL_ENSURE(!hasChanges(getRootNode(_aTree)),"Unexpected changes in View");
            OSL_ENSURE(false,"ERROR: Cannot recover commit for unexpected changes");
        }

//-----------------------------------------------------------------------------
        void ViewStrategy::implAdjustToElementChange(NodeChangesInformation& rLocalChanges, SetNode const& _aSetNode, Change const& rElementChange, TreeDepth nDepth)
        {
            using namespace configuration;

            SetNodeImpl * pSetData = _aSetNode.get_impl();

            OSL_ENSURE( pSetData->implHasLoadedElements() , "Unexpected call: Processing element change in uninitialized set");

            Name aName = makeElementName( rElementChange.getNodeName(), Name::NoValidate() );

            SetElementChangeImpl* pThisChange = 0;
            if (rElementChange.ISA(AddNode))
            {
                AddNode const& aAddNode = static_cast<AddNode const&>(rElementChange);

                SetNodeElement aNewElement = pSetData->makeAdditionalElement(_aSetNode.accessor(),this,aAddNode,nDepth);

                pThisChange = pSetData->doAdjustToAddedElement(_aSetNode.accessor(), aName, aAddNode,aNewElement);
            }
            else if (rElementChange.ISA(RemoveNode))
            {
                RemoveNode const& aRemoveNode = static_cast<RemoveNode const&>(rElementChange);

                pThisChange = pSetData->doAdjustToRemovedElement(_aSetNode.accessor(), aName, aRemoveNode);
            }
            else
            {
                if (nDepth > 0 || (NULL != pSetData->doFindElement(aName)) )// found even beyond nDepth ?
                {
                    pThisChange = pSetData->doAdjustChangedElement(_aSetNode.accessor(),rLocalChanges,aName, rElementChange);
                }
            }

            if (pThisChange)
            {
                addLocalChangeHelper( rLocalChanges, NodeChange(pThisChange) );
            }
        }

        void ViewStrategy::implAdjustToElementChanges(NodeChangesInformation& rLocalChanges, SetNode const& _aSetNode, SubtreeChange const& rExternalChanges, TreeDepth nDepth)
        {

            if (nDepth > 0)
            {
                configuration::SetNodeImpl * pSetData = _aSetNode.get_impl();

                OSL_ENSURE( pSetData->getTemplateProvider().isValid(), "Cannot adjust SetNode to changes - node was never initialized" );

                if (pSetData->implHasLoadedElements())
                {
                    TreeDepth const nElementDepth = configuration::childDepth(nDepth);
                    for (SubtreeChange::ChildIterator it = rExternalChanges.begin(); it != rExternalChanges.end(); ++it)
                    {
                        this->implAdjustToElementChange(rLocalChanges, _aSetNode, *it, nElementDepth);
                    }
                }
                else
                {
                    OSL_ENSURE( !hasChanges(_aSetNode.node()),"Cannot have changes to consider when no elements are loaded");

                    pSetData->convertChanges( rLocalChanges, _aSetNode.accessor(), rExternalChanges, nDepth);
                }
            }
        }

        configuration::ValueChangeImpl* ViewStrategy::doAdjustToValueChange(GroupNode const& _aGroupNode, Name const& _aName, ValueChange const& _rExternalChange)
        {
            using namespace configuration;

            ValueChangeImpl* pChangeImpl = NULL;

            data::ValueNodeAccess aLocalNode = getMemberValueAccess(_aGroupNode,_aName);
            if (aLocalNode.isValid())
            {
                switch( _rExternalChange. getMode() )
                {
                case ValueChange::wasDefault:
                case ValueChange::changeValue:
                    pChangeImpl = new ValueReplaceImpl( _rExternalChange.getNewValue(), _rExternalChange.getOldValue() );
                    break;

                    break;

                case ValueChange::setToDefault:
                    pChangeImpl = new ValueResetImpl( _rExternalChange.getNewValue(), _rExternalChange.getOldValue() );
                    break;

                default: OSL_ENSURE(false, "Unknown change mode");
                    // fall thru to next case for somewhat meaningful return value
                case ValueChange::changeDefault:
                    {
                        UnoAny aLocalValue = aLocalNode.getValue();

                        pChangeImpl = new ValueReplaceImpl( aLocalValue, aLocalValue );
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

        void ViewStrategy::implAdjustToValueChanges(NodeChangesInformation& rLocalChanges, GroupNode const& _aGroupNode, SubtreeChange const& rExternalChanges)
        {
            for (SubtreeChange::ChildIterator it = rExternalChanges.begin(); it != rExternalChanges.end(); ++it)
            {
                using namespace configuration;

                if (it->ISA(ValueChange))
                {
                    ValueChange const& rValueChange = static_cast<ValueChange const&>(*it);

                    Name aValueName = makeNodeName( rValueChange.getNodeName(), Name::NoValidate() );

                    if (ValueChangeImpl* pThisChange = doAdjustToValueChange(_aGroupNode, aValueName, rValueChange))
                    {
                        pThisChange->setTarget(_aGroupNode,aValueName);
                        addLocalChangeHelper(rLocalChanges, NodeChange(pThisChange));
                    }
                    else
                        OSL_TRACE("WARNING: Configuration: derived class hides an external value member change from listeners");
                }
                else
                    OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected change type within group");
            }
        }

        void ViewStrategy::implAdjustToSubChanges(NodeChangesInformation& rLocalChanges, GroupNode const& _aGroupNode, SubtreeChange const& rExternalChanges)
        {
            using namespace configuration;

            for(SubtreeChange::ChildIterator it = rExternalChanges.begin(); it != rExternalChanges.end(); ++it)
            {
                TreeImpl * pTreeData = _aGroupNode.tree().get_impl();

                if ( it->ISA(SubtreeChange) )
                {
                    Node aSubNode = _aGroupNode.findChild( makeNodeName(it->getNodeName(), Name::NoValidate()) );
                    OSL_ENSURE( aSubNode.is() || pTreeData->depthTo(_aGroupNode.node().get_offset()) >= pTreeData->getAvailableDepth(), "Changed node not found in tree");

                    if (aSubNode.is())
                    {
                        OSL_ENSURE( pTreeData->getRemainingDepth(_aGroupNode.node().get_offset()) > 0, "Depth is smaller than expected for tree");
                        this->adjustToChanges(rLocalChanges, aSubNode, static_cast<SubtreeChange const&>(*it));
                    }
                }
                else
                {
                    OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for child of group node; change is ignored");
                    OSL_ENSURE( !_aGroupNode.findChild(makeNodeName(it->getNodeName(), Name::NoValidate())).is(),
                                "Found sub(tree) node where a value was expected");
                }
            }
        }
//-----------------------------------------------------------------------------
        void ViewStrategy::doCollectChanges(Node const& _aNode, NodeChanges& ) const
        {
            // no-op: there are no changes to collect
            OSL_ENSURE(!hasChanges(_aNode),"Unexpected changes in View");
        }


//-----------------------------------------------------------------------------
        UnoAny  ViewStrategy::getValue(ValueNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getValue(_aNode.accessor());
        }

        UnoType ViewStrategy::getValueType(ValueNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getValueType(_aNode.accessor());
        }

//-----------------------------------------------------------------------------
// group member access

//-----------------------------------------------------------------------------
    namespace { // helpers
        using configuration::GroupMemberVisitor;

        struct GroupMemberDispatch : data::NodeVisitor
        {
            GroupMemberDispatch(ViewStrategy& _rStrategy, GroupNode const& _aGroup, GroupMemberVisitor& rVisitor)
            : m_aGroup(_aGroup)
            , m_rVisitor(rVisitor)
            , m_rStrategy(_rStrategy)
            {}

            static Result mapResult(GroupMemberVisitor::Result _aResult)
            {

                OSL_ASSERT( DONE == GroupMemberVisitor::DONE );
                OSL_ASSERT( CONTINUE == GroupMemberVisitor::CONTINUE );
        register int nResultValue = _aResult;
                return static_cast<Result>(nResultValue);
            }

            static GroupMemberVisitor::Result unmapResult(Result _aResult)
            {
                OSL_ASSERT( DONE == GroupMemberVisitor::DONE );
                OSL_ASSERT( CONTINUE == GroupMemberVisitor::CONTINUE );
        register int nResultValue = _aResult;
                return static_cast<GroupMemberVisitor::Result>(nResultValue);
            }

            virtual Result handle(data::ValueNodeAccess const& _aValue);
            virtual Result handle(data::NodeAccessRef const& _aNonValue);

            bool test_value(data::NodeAccessRef const & _aNode) const;

            ViewStrategy&       m_rStrategy;
            GroupNode           m_aGroup;
            GroupMemberVisitor& m_rVisitor;

            GroupMemberVisitor::Result m_aResult;
        };

        bool GroupMemberDispatch::test_value(data::NodeAccessRef const& _aNode) const
        {
            Name aName = _aNode.getName();

            return m_rStrategy.hasValue( m_aGroup, aName );
        }

        GroupMemberDispatch::Result GroupMemberDispatch::handle(data::ValueNodeAccess const& _aValue)
        {
            OSL_ENSURE( test_value(_aValue), "ERROR: Group MemberDispatch:Did not find a ValueMember for a value child.");

            Name aValueName = _aValue.getName();

            return mapResult( m_rVisitor.visit( m_rStrategy.getValue(m_aGroup,aValueName) ) );
        }

        GroupMemberDispatch::Result GroupMemberDispatch::handle(data::NodeAccessRef const& _aNonValue)
        {
            OSL_ENSURE( !test_value(_aNonValue), "ERROR: Group MemberDispatch:Found a ValueMember for a subtree child.");

            return CONTINUE;
        }
    }
//-----------------------------------------------------------------------------
        configuration::ValueMemberNode ViewStrategy::doGetValueMember(GroupNode const& _aNode, Name const& _aName, bool ) const
        {
            using data::ValueNodeAccess;

            ValueNodeAccess aValueData = getMemberValueAccess(_aNode,_aName);

            return _aNode.get_impl()->makeValueMember( aValueData );
        }

        bool ViewStrategy::hasValue(GroupNode const& _aNode, Name const& _aName) const
        {
            checkInstance(_aNode.tree());
            return getMemberValueAccess(_aNode,_aName).isValid();
        }

        bool ViewStrategy::hasValue(GroupNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            configuration::GroupNodeImpl* pGroupNode=_aNode.get_impl();
            data::GroupNodeAccess aGroupNodeAccess = pGroupNode->getDataAccess( _aNode.accessor() );
            return aGroupNodeAccess.hasChildren();
        }


        bool ViewStrategy::areValueDefaultsAvailable(GroupNode const& _aNode) const
        {
            checkInstance(_aNode.tree());

            return  _aNode.get_impl()->areValueDefaultsAvailable( _aNode.accessor() );
        }

        configuration::ValueMemberNode ViewStrategy::getValue(GroupNode const& _aNode, Name const& _aName) const
        {
            checkInstance(_aNode.tree());
            return doGetValueMember(_aNode,_aName,false);
        }

        configuration::ValueMemberUpdate ViewStrategy::getValueForUpdate(GroupNode const & _aNode, Name const& _aName)
        {
            checkInstance(_aNode.tree());
            return ValueMemberUpdate( doGetValueMember(_aNode,_aName,true), *this );
        }

        GroupMemberVisitor::Result ViewStrategy::dispatchToValues(GroupNode const& _aNode, GroupMemberVisitor& _aVisitor)
        {
            checkInstance(_aNode.tree());

            GroupMemberDispatch aDispatch(*this,_aNode,_aVisitor);

            GroupMemberDispatch::Result eResult = aDispatch.visitChildren( _aNode.getAccess() );

            return aDispatch.unmapResult(eResult);
        }

//-----------------------------------------------------------------------------
        ViewStrategy::SetNodeElement ViewStrategy::implMakeElement(SetNode const& _aNode, SetNodeEntry const& anEntry) const
        {
            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();
            return pNodeData->implValidateElement(anEntry.accessor(), pNodeData->entryToElement(anEntry));
        }
//-----------------------------------------------------------------------------
        SetEntry ViewStrategy::implFindElement(SetNode const& _aNode, Name const& aName) const
        {
            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();

            OSL_ENSURE(pNodeData->implHasLoadedElements(),"Cannot find elements in set that is not loaded");
            configuration::ElementTreeImpl * pElement = pNodeData->doFindElement(aName);

            return SetEntry(_aNode.accessor(), pElement);
        }

        SetEntry ViewStrategy::findElement(SetNode const& _aNode, Name const& aName) const
        {
            checkInstance(_aNode.tree());
            _aNode.get_impl()->implEnsureElementsLoaded(_aNode.accessor());
            return implFindElement(_aNode,aName);
        }

        SetEntry ViewStrategy::findAvailableElement(SetNode const& _aNode, Name const& aName) const
        {
            checkInstance(_aNode.tree());
            if (_aNode.get_impl()->implHasLoadedElements())
                return implFindElement(_aNode,aName);
            else
                return SetEntry(_aNode.accessor(),0);
        }

        static
        inline
        std::auto_ptr<SubtreeChange> makeChangeToDefault(data::SetNodeAccess const & _aSetNode)
        {
            return std::auto_ptr<SubtreeChange>(
                        new SubtreeChange(
                            _aSetNode.getName().toString(),
                            _aSetNode.getElementTemplateName().toString(),
                            _aSetNode.getElementTemplateModule().toString(),
                            _aSetNode.getAttributes(),
                            true // to default
                   )    );
        }

        std::auto_ptr<SubtreeChange> ViewStrategy::differenceToDefaultState(SetNode const& _aNode, ISubtree& _rDefaultTree) const
        {
            checkInstance(_aNode.tree());
            std::auto_ptr<SubtreeChange> aResult;

            data::SetNodeAccess aOriginalSetNode = _aNode.getAccess();
            if (!aOriginalSetNode.isDefault())
            {
                OSL_ASSERT(aOriginalSetNode.isValid());

                aResult = makeChangeToDefault( aOriginalSetNode );

                configuration::SetNodeImpl * pNodeData = _aNode.get_impl();
                if (this->hasChanges(_aNode.node()))
                {
                    OSL_ENSURE(pNodeData->implHasLoadedElements(),"Unexpected: Found set with changes but elements are not loaded");
                    pNodeData->doDifferenceToDefaultState(_aNode.accessor(),*aResult,_rDefaultTree);
                }
                else
                    pNodeData->implDifferenceToDefaultState(_aNode.accessor(),*aResult,_rDefaultTree);
            }
            return aResult;
        }

        configuration::TemplateHolder ViewStrategy::getElementTemplate(SetNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getElementTemplate();
        }

        configuration::TemplateProvider ViewStrategy::getTemplateProvider(SetNode const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.get_impl()->getTemplateProvider();
        }

//-----------------------------------------------------------------------------
/*       //  virtual rtl::Reference<ViewStrategy> doCloneIndirect(); // fails* /
        rtl::Reference<ViewStrategy> ViewStrategy::makeIndirect(Tree const& _aTree)
        {
            _aTree->makeIndirect(true);
            return this;
        }

        void ViewStrategy::doCommitChanges(Node const& _aNode)
        {
            // nothing to do
        }

        // TODO: move this to deferred impl
        void ViewStrategy::implCommitDirectIn(data::TreeAccessor const& _aPlaceHolder, Node const& _aNode)
        {
            if (this->hasChanges(_aNode) )
            {
                this->doCommitChanges(_aNode);

                GroupNode aGroup(_aNode);

                for (Node aChild = aGroup.getFirstChild(); aChild.is(); aChild = aGroup.getNextChild(aChild) )
                {
                    implCommitDirectIn(_aPlaceHolder, aChild);
                }
            }
        }

        void ViewStrategy::commitDirectly(data::TreeAccessor const& _aPlaceHolder, Tree const& _aTree)
        {
            implCommitDirectIn( _aPlaceHolder, getRootNode(_aTree) );
        }

        //  virtual rtl::Reference<ViewStrategy> doCloneDirect();   // returns 'this'
        rtl::Reference<ViewStrategy> ViewStrategy::makeDirect  (Tree const& _aTree)
        {
            commitDirectly(_aTree.accessor(), _aTree);
            _aTree->makeIndirect(false);
            return this;
        }
*/
//-----------------------------------------------------------------------------
        data::NodeAccessRef ViewStrategy::getNodeAccessRef(Node const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.getAccessRef();
        }

        Name ViewStrategy::getNodeName(Node const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.data().getName();
        }

        node::Attributes ViewStrategy::getNodeAttributes(Node const& _aNode) const
        {
            checkInstance(_aNode.tree());
            return _aNode.getAccessRef().getAttributes();
        }

//-----------------------------------------------------------------------------
        SetNodeVisitor::Result ViewStrategy::dispatchToElements(SetNode const& _aNode, SetNodeVisitor& _aVisitor)
        {
            checkInstance(_aNode.tree());

            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();

            if (pNodeData->implLoadElements(_aNode.accessor()))
                return pNodeData->doDispatchToElements(_aNode.accessor(), _aVisitor);

            else
                return SetNodeVisitor::CONTINUE;
        }

        bool ViewStrategy::isEmpty(SetNode const& _aNode) const
        {
            checkInstance(_aNode.tree());

            configuration::SetNodeImpl * pNodeData = _aNode.get_impl();

            return !pNodeData->implLoadElements(_aNode.accessor()) || pNodeData->doIsEmpty();
        }
//-----------------------------------------------------------------------------

        void ViewStrategy::insertElement(SetNode const& _aNode, Name const& _aName, SetEntry const& _aNewEntry)
        {
            // cannot insert, if we cannot check for collisions
            checkInstance(_aNode.tree());
            _aNode.get_impl()->implEnsureElementsLoaded(_aNode.accessor());
            doInsertElement(_aNode,_aName,_aNewEntry);
        }

        void ViewStrategy::removeElement(SetNode const& _aNode, Name const& _aName)
        {
            // cannot remove, if we cannot check for existance
            checkInstance(_aNode.tree());
            _aNode.get_impl()->implEnsureElementsLoaded(_aNode.accessor());
            doRemoveElement(_aNode,_aName);
        }

//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

