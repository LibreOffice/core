/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatehelper.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:34:11 $
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

#include <stdio.h>

#include "updatehelper.hxx"

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_NODECONVERTER_HXX
#include "nodeconverter.hxx"
#endif
#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif
#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
#endif
#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif
#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif

// -----------------------------------------------------------------------------
#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif
#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_BUILDDATA_HXX
#include "builddata.hxx"
#endif
#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif

#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
// -----------------------------------------------------------------------------

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


//..........................................................................
namespace configmgr
{
// -----------------------------------------------------------------------------
//==========================================================================
//= AdjustUpdate
//==========================================================================
//= This class tests changes on an existing tree and drops them if they
//= are not need anymore or alters add nodes in node changes and vice versa
//==========================================================================
class AdjustUpdate : ChangeTreeModification
{
    SubtreeChange&      m_rChangeList;  // list which containes changes merged with the existing nodes
    data::NodeAccess    m_aRefNode;     // reference node needed for merging
    OTreeNodeConverter  m_aNodeConverter;
public:
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aUpdateTree,
                        data::NodeAccess const& _aTargetNode)
    {
        return AdjustUpdate(_rResultTree,_aTargetNode).impl_adjust(_aUpdateTree);
    }
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aUpdateTree,
                        data::NodeAccess const& _aTargetNode,
                        OTreeNodeFactory& _rNodeFactory)
    {
        return AdjustUpdate(_rResultTree,_aTargetNode,_rNodeFactory).impl_adjust(_aUpdateTree);
    }
private:
    AdjustUpdate(SubtreeChange& rList, data::NodeAccess const & _aNode)
        :m_rChangeList(rList)
        ,m_aRefNode(_aNode)
        ,m_aNodeConverter()
    {}

    AdjustUpdate(SubtreeChange& rList, data::NodeAccess const & _aNode, OTreeNodeFactory& _rNodeFactory)
        :m_rChangeList(rList)
        ,m_aRefNode(_aNode)
        ,m_aNodeConverter(_rNodeFactory)
    {}

    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);

    bool impl_adjust(SubtreeChange& _aUpdateTree);
private:
    // ensuring the correct state
    bool checkNode() const;
};

// --------------------------------- ApplyUpdate ---------------------------------

class ApplyUpdate : public ChangeTreeModification
{
    data::NodeAddress           m_aCurrentNode;
public:
    ApplyUpdate(data::NodeAddress _aNode)
    : m_aCurrentNode(_aNode)
    {}

    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);

    static
    void applyChange(ValueChange& _rValueChange, data::ValueNodeAddress & _aValueNodeAddr);
};
//--------------------------------------------------------------------------
class ApplyValueChange
{
    static
    void adjust(uno::Any& aActual, uno::Any const& aTarget);

public:
    static
    data::ValueNodeAccess node(data::ValueNodeAddress & _aValueNodeAddr)
    { return data::ValueNodeAccess(_aValueNodeAddr); }

    static
    uno::Any getValue(data::ValueNodeAddress & _aValueNodeAddr)
    { return node(_aValueNodeAddr).getValue(); }

    static
    uno::Any getDefault(data::ValueNodeAddress & _aValueNodeAddr)
    { return node(_aValueNodeAddr).getDefaultValue(); }

    static
    void apply(ValueChange& _rValueChange, data::ValueNodeAddress & _aValueNodeAddr);
};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


// adjust a set of changes to the target tree, return true, if there are changes left
    bool adjustUpdateToTree(SubtreeChange & _rUpdateTree, data::NodeAccess const & _aRootNode)
    {
        SubtreeChange aResultTree(_rUpdateTree, SubtreeChange::NoChildCopy());

        bool bResult = AdjustUpdate::adjust(aResultTree,_rUpdateTree,_aRootNode);

        _rUpdateTree.swap(aResultTree);

        return bResult;
    }
//--------------------------------------------------------------------------

// adjust a set of changes to the target tree, return true, if there are changes left
    bool adjustUpdateToTree(SubtreeChange & _rUpdateTree, data::NodeAddress _aRootNode)
    {
        data::NodeAccess aTargetNode(_aRootNode);
        return adjustUpdateToTree(_rUpdateTree, aTargetNode);
    }
//--------------------------------------------------------------------------

// apply a already matching set of changes to the target tree
    void applyUpdateToTree(SubtreeChange& _anUpdateTree, data::NodeAddress _aRootNode)
    {
        ApplyUpdate aUpdater(_aRootNode);
        _anUpdateTree.forEachChange(aUpdater);
    }
//--------------------------------------------------------------------------
    static inline
    bool adjust_helper(SubtreeChange& _rResultTree, SubtreeChange& _aUpdateTree,
               data::NodeAddress _aTargetAddress )
    {
        data::NodeAccess aTargetNode(_aTargetAddress);
        return AdjustUpdate::adjust(_rResultTree, _aUpdateTree, aTargetNode);
    }
//--------------------------------------------------------------------------
// apply a set of changes to the target tree
    void applyUpdateWithAdjustmentToTree(SubtreeChange& _anUpdateTree, data::NodeAddress _aRootNode)
    {
    // POST: pSubtree = pSubtree + aChangeList
    SubtreeChange aActualChanges(_anUpdateTree, SubtreeChange::NoChildCopy());

        if ( adjust_helper(aActualChanges,_anUpdateTree, _aRootNode) )
        {
        applyUpdateToTree(aActualChanges, _aRootNode);
        }
        _anUpdateTree.swap(aActualChanges);

    }
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

static inline
configuration::Name getNodeName(INode const & _aNode)
{
    return configuration::makeName( _aNode.getName(), configuration::Name::NoValidate() );
}
//--------------------------------------------------------------------------

static inline
configuration::Name getChangeNodeName(Change const & _aChange)
{
    return configuration::makeName( _aChange.getNodeName(), configuration::Name::NoValidate() );
}
//--------------------------------------------------------------------------

bool AdjustUpdate::impl_adjust(SubtreeChange& _aUpdateTree)
{
    // first check the changes
    this->applyToChildren(_aUpdateTree);

    // now check whether there are real modifications
    OChangeActionCounter aChangeCounter;
    aChangeCounter.handle(m_rChangeList);
    CFG_TRACE_INFO_NI("cache manager: counted changes : additions: %i , removes: %i, value changes: %i", aChangeCounter.nAdds, aChangeCounter.nRemoves, aChangeCounter.nValues);

    return !! aChangeCounter.hasChanges();
}
// -----------------------------------------------------------------------------

inline bool AdjustUpdate::checkNode() const
{
    // Change a Value
    OSL_ENSURE(m_aRefNode.isValid(),"AdjustUpdate: no data");

    return m_aRefNode.isValid();
}
//--------------------------------------------------------------------------

void AdjustUpdate::handle(ValueChange& _rChange)
{
    if (checkNode())
    {
        // We need to find the element in the tree
        data::NodeAccess aChildNodeAcc = data::getSubnode(m_aRefNode, getChangeNodeName(_rChange));
        data::NodeAccess aChildNode(aChildNodeAcc);

        // We have a node so we can keep the Change and the values do not differ
        if (aChildNode.isValid())
        {
            bool bIsValue = data::ValueNodeAccess::isInstance(aChildNode);
            OSL_ENSURE(bIsValue, "AdjustUpdate : node must be a value node!");

            if (bIsValue && _rChange.isChange())
            {
                std::auto_ptr<Change> pChange( new ValueChange(_rChange) );
                m_rChangeList.addChange(pChange);
            }
        }
        else
        {
            std::auto_ptr<ValueNode> pNode = m_aNodeConverter.createCorrespondingNode(_rChange);

            OSL_ENSURE(m_rChangeList.isSetNodeChange(), "Adding a new value to a non-set node");
            OUString sTypeName = m_rChangeList.getElementTemplateName();

            data::TreeSegment aNewTree = data::TreeSegment::createNew(base_ptr(pNode),sTypeName);
            // add the tree to the change list
            std::auto_ptr<Change> pChange( new AddNode( aNewTree,_rChange.getNodeName(), _rChange.isToDefault()) );
            m_rChangeList.addChange(pChange);
        }
    }
}
//--------------------------------------------------------------------------

void AdjustUpdate::handle(SubtreeChange& _rChange)
{
    if (checkNode())
    {
        // We need to find the element in the tree
        data::NodeAccess aChildNodeAcc = data::getSubnode(m_aRefNode, getChangeNodeName(_rChange));
        data::NodeAccess aChildNode(aChildNodeAcc);

        // if there is a node we continue
        if (aChildNode.isValid())
        {
            bool bIsSubtree =   data::GroupNodeAccess::isInstance(aChildNode) ||
                                data::SetNodeAccess::isInstance(aChildNode);
            OSL_ENSURE(bIsSubtree, "AdjustUpdate : node must be a inner node!");

            if (bIsSubtree)
            {
                // generate a new change
                std::auto_ptr<SubtreeChange> pChange( new SubtreeChange(_rChange, SubtreeChange::NoChildCopy()) );

                // recurse
                if ( adjust(*pChange,_rChange,aChildNode,m_aNodeConverter.nodeFactory()) )
                    m_rChangeList.addChange(base_ptr(pChange));
            }
            else
                OSL_ENSURE(false, "Inconsistent data: Subtree Change is merged into non-subtree node.");
        }
        // otherwise we have to create the node
        else
        {
            std::auto_ptr<ISubtree> pNode = m_aNodeConverter.createCorrespondingTree(_rChange);
            OSL_ASSERT(pNode.get() != NULL);

            // set the level
            pNode->setLevels(treeop::ALL_LEVELS,treeop::ALL_LEVELS);

            OSL_ENSURE(m_rChangeList.isSetNodeChange(), "Adding a new value to a non-set node");
            OUString sTypeName = m_rChangeList.getElementTemplateName();

            data::TreeSegment aNewTree = data::TreeSegment::createNew(base_ptr(pNode), sTypeName);

            // add the tree to the change list
            std::auto_ptr<Change> pChange( new AddNode(aNewTree,_rChange.getNodeName(), _rChange.isToDefault()) );
            m_rChangeList.addChange( pChange );
        }
    }
}
//--------------------------------------------------------------------------

void AdjustUpdate::handle(RemoveNode& _rChange)
{
    if (checkNode())
    {
        // We need to find the element in the tree
        data::NodeAccess aChildNode = data::getSubnode(m_aRefNode, getChangeNodeName(_rChange));

        // only if there is a node, we will keep the change
        if (aChildNode.isValid())
        {
            // generate a new change
            std::auto_ptr<Change> pChange( new RemoveNode(_rChange.getNodeName(),_rChange.isToDefault()) );
            m_rChangeList.addChange(pChange);
        }
    }
}
//--------------------------------------------------------------------------

void AdjustUpdate::handle(AddNode& _rChange)
{
    if (checkNode())
    {
        // We need to find the element in the tree
        data::NodeAccess aChildNode = data::getSubnode(m_aRefNode, getChangeNodeName(_rChange));

        data::TreeSegment aNewNode = _rChange.getNewTree();
        std::auto_ptr<AddNode> pChange( new AddNode(aNewNode,_rChange.getNodeName(),_rChange.isToDefault()) );
        if (aChildNode.isValid())
        {
            pChange->setReplacing();
        }
        m_rChangeList.addChange(base_ptr(pChange));
    }
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

inline
void ApplyValueChange::adjust(uno::Any& aActual, uno::Any const& aTarget)
{
    // If set - it should already match
    OSL_ASSERT(!aActual.hasValue() || aTarget == aActual);
    aActual = aTarget;
}

//--------------------------------------------------------------------------
// _rValueChange.applyTo(_aValueNode)
void ApplyValueChange::apply(ValueChange& _rValueChange, data::ValueNodeAddress & _aValueNodeAddr)
{
    using data::ValueNodeAccess;

    switch (_rValueChange.getMode())
    {
    case ValueChange::wasDefault:
        OSL_ASSERT(node(_aValueNodeAddr).isDefault());

    case ValueChange::changeValue:
        adjust( _rValueChange.m_aOldValue, getValue(_aValueNodeAddr));
        ValueNodeAccess::setValue(_aValueNodeAddr,_rValueChange.getNewValue());
        break;

    case ValueChange::setToDefault:
        adjust( _rValueChange.m_aOldValue,  getValue(_aValueNodeAddr));
        adjust( _rValueChange.m_aValue,     getDefault(_aValueNodeAddr));
        ValueNodeAccess::setToDefault(_aValueNodeAddr);
        break;

    case ValueChange::changeDefault:
        adjust( _rValueChange.m_aOldValue,  getDefault(_aValueNodeAddr));
        ValueNodeAccess::changeDefault(_aValueNodeAddr,_rValueChange.getNewValue());
        break;

    default:
        OSL_ENSURE(0, "Unknown mode found for ValueChange");
        break;
    }
}
//--------------------------------------------------------------------------

void ApplyUpdate::handle(ValueChange& _rChange)
{
    // Change a Value
    OSL_ENSURE(m_aCurrentNode != NULL,"Cannot apply ValueChange without node");

    data::NodeAddress aChildNodeAddr = data::getSubnodeAddress(m_aCurrentNode, getChangeNodeName(_rChange));
    OSL_ENSURE(aChildNodeAddr != NULL,"Cannot apply Change: No node to change");

    data::ValueNodeAddress aValueAddr = aChildNodeAddr->valueData();
    OSL_ENSURE(aValueAddr != NULL,"Cannot apply ValueChange: Node is not a value");

    if (aValueAddr != NULL)
    ApplyValueChange::apply(_rChange,aValueAddr);
}
//--------------------------------------------------------------------------

void ApplyUpdate::handle(SubtreeChange& _rChange)
{
    // handle traversion
    OSL_ENSURE(m_aCurrentNode != NULL,"Cannot apply SubtreeChange without node");

    data::NodeAddress aChildNodeAddr = data::getSubnodeAddress(m_aCurrentNode, getChangeNodeName(_rChange));
    OSL_ENSURE(aChildNodeAddr != NULL,"Cannot apply Change: No node to change");

    OSL_ENSURE( data::toGroupNodeAddress(aChildNodeAddr) != NULL ||
                data::toSetNodeAddress(aChildNodeAddr) != NULL ,
                "Cannot Apply SubtreeChange: Node is not an inner node");

    if (aChildNodeAddr != NULL)
    {
        aChildNodeAddr->node.info.markAsDefault( _rChange.isToDefault() );

        data::NodeAddress aOldNode = m_aCurrentNode;
        m_aCurrentNode = aChildNodeAddr;

        _rChange.forEachChange(*this);

        m_aCurrentNode = aOldNode;
    }
}
//--------------------------------------------------------------------------

void ApplyUpdate::handle(AddNode& _rChange)
{
    OSL_ENSURE(m_aCurrentNode != NULL,"Cannot apply AddNode without node");

    data::SetNodeAddress aSetNodeAddr = data::toSetNodeAddress(m_aCurrentNode);
    OSL_ENSURE(aSetNodeAddr != NULL,"Cannot apply AddNode: Node is not a set node");

    // Add a new element
    if (aSetNodeAddr != NULL)
    {
    if (_rChange.isReplacing())
    {
            data::TreeAddress aOldNodeAddr =
                data::SetNodeAccess::removeElement(aSetNodeAddr,getChangeNodeName(_rChange));

            OSL_ENSURE(aOldNodeAddr != NULL, "ApplyUpdate: AddNode: can't recover node being replaced");

            data::TreeAccessor aOldNodeAccess(aOldNodeAddr);
            _rChange.takeReplacedTree( data::TreeSegment::createNew(aOldNodeAccess) );
        }

        data::TreeAddress aNewAddress = data::buildTree(_rChange.getNewTree().getTreeAccess());
        OSL_ENSURE(aNewAddress != NULL, "ApplyUpdate: AddNode: could not create new element");

        data::SetNodeAccess::addElement(aSetNodeAddr,aNewAddress);

        _rChange.setInsertedAddress( aNewAddress );
    }
}
//--------------------------------------------------------------------------

void ApplyUpdate::handle(RemoveNode& _rChange)
{
    OSL_ENSURE(m_aCurrentNode != NULL,"Cannot apply RemoveNode without node");

    data::SetNodeAddress aSetNodeAddr = data::toSetNodeAddress(m_aCurrentNode);
    OSL_ENSURE(aSetNodeAddr != NULL,"Cannot apply RemoveNode: Node is not a set node");

    // Remove an element
    if (aSetNodeAddr != NULL)
    {
        data::TreeAddress aOldNodeAddr =
            data::SetNodeAccess::removeElement(aSetNodeAddr,getChangeNodeName(_rChange));

        OSL_ENSURE(aOldNodeAddr != NULL, "ApplyUpdate: Remove: can't recover node being removed");

        data::TreeAccessor aOldNodeAccess(aOldNodeAddr);
        _rChange.takeRemovedTree( data::TreeSegment::createNew(aOldNodeAccess) );
    }
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    struct ForwardTreeDifferenceBuilder : NodeAction
    {
    protected:
        SubtreeChange&      m_rChangeList;
        data::NodeAccess    m_aCacheNode;

    public:
        ForwardTreeDifferenceBuilder(SubtreeChange& rList, data::NodeAccess const & _aCacheNode)
        : m_rChangeList(rList)
        , m_aCacheNode(_aCacheNode)
        {
        }

        virtual void handle(ValueNode const& _aNewNode)
        {
            data::NodeAccess aChildNode = data::getSubnode(m_aCacheNode,getNodeName(_aNewNode));

            OSL_ENSURE(aChildNode.isValid(), "TreeDifferenceBuilder: could not find expected node !");

            data::ValueNodeAccess aValueNode( aChildNode );

            OSL_ENSURE(aValueNode.isValid(), "TreeDifferenceBuilder: node must be a value node!");

            // if the values differ add a new change
            if (aValueNode.isValid() && _aNewNode.getValue() != aValueNode.getValue())
            {
                bool bNewDefault = _aNewNode.isDefault();
                bool bOldDefault = aValueNode.isDefault();

                ValueChange::Mode eMode;
                if (bNewDefault)
                    if (bOldDefault)
                        eMode =  ValueChange::changeDefault;
                    else
                        eMode =  ValueChange::setToDefault;
                else
                    if (bOldDefault)
                        eMode =  ValueChange::wasDefault;
                    else
                        eMode =  ValueChange::changeValue;

                std::auto_ptr<Change> pChange(
                    new ValueChange(_aNewNode.getName(), _aNewNode.getAttributes(), eMode,
                                    _aNewNode.getValue(), aValueNode.getValue()) );

                m_rChangeList.addChange(pChange);
            }
        }
        virtual void handle(ISubtree const& _aNewNode)
        {
            data::NodeAccess aChildNode = data::getSubnode(m_aCacheNode,getNodeName(_aNewNode));

            if (aChildNode.isValid())
            {
                OSL_ENSURE( data::GroupNodeAccess::isInstance(aChildNode) ||
                            data::SetNodeAccess::isInstance(aChildNode) ,
                            "ForwardTreeDifferenceBuilder: Node must be an inner node");

                // generate a new change
                std::auto_ptr<SubtreeChange> pNewChange( new SubtreeChange(_aNewNode) );

                // .. and recurse
                ForwardTreeDifferenceBuilder aNextLevel(*pNewChange, aChildNode);
                aNextLevel.applyToChildren(_aNewNode);

                // now count if there are any changes
                OChangeActionCounter aCounter;
                aCounter.applyToChange(*pNewChange);

                if (aCounter.hasChanges())
                    m_rChangeList.addChange(base_ptr(pNewChange));
            }
            else if (data::SetNodeAccess::isInstance(m_aCacheNode))
            {
                // Subtree not in Cache, add in TreeChangeList
                // SubtreeChange* pChange = new SubtreeChange(_rSubtree);
                OSL_ENSURE(m_rChangeList.isSetNodeChange(), "Found newly added node in non-set node");
                OUString sTypeName = m_rChangeList.getElementTemplateName();

                std::auto_ptr<INode> pSubtree( _aNewNode.clone() );
                data::TreeSegment aNewTree = data::TreeSegment::createNew(pSubtree,sTypeName);

                std::auto_ptr<Change> pAdd(new AddNode(aNewTree, _aNewNode.getName(), _aNewNode.isDefault()));

                m_rChangeList.addChange(pAdd);
            }
            else
                OSL_ENSURE(false, "Found newly added node in group");

        }
    };
// -----------------------------------------------------------------------------

    struct BackwardTreeDifferenceBuilder : data::SetVisitor
    {
    protected:
        SubtreeChange&      m_rChangeList;
        ISubtree const *    m_pNewNode;

              using NodeVisitor::handle;
    public:
        BackwardTreeDifferenceBuilder(SubtreeChange& rList, ISubtree const* pNode)
        : m_rChangeList(rList)
        , m_pNewNode(pNode)
        {
        }

        Result applyToChildren(data::NodeAccess const & _aCacheNode)
        {
            if (data::GroupNodeAccess::isInstance(_aCacheNode))
            {
                OSL_ENSURE( !m_rChangeList.isSetNodeChange(), "Building a set change for a group node" );
                return visitChildren( data::GroupNodeAccess(_aCacheNode) );
            }
            else if (data::SetNodeAccess::isInstance(_aCacheNode))
            {
                OSL_ENSURE(  m_rChangeList.isSetNodeChange(), "Building a group change for a set node" );
                return visitElements( data::SetNodeAccess(_aCacheNode) );
            }
            else
            {
                OSL_ENSURE(  m_rChangeList.ISA(ValueChange), "BackwardTreeDifferenceBuilder: Unknown node type" );
                OSL_ENSURE( !m_rChangeList.ISA(ValueChange), "Trying to build a change tree for a value node" );
                return DONE;
            }
        }

    private:
        virtual Result handle(data::ValueNodeAccess const & /*_aCacheNode*/)
        {
#if 0 // do we really need to do nothing here?
            OUString aNodeName = _aCacheNode.getName().toString();

            INode const* pNewChild = m_pNewNode->getChild(aNodeName);

            //OSL_ENSURE(pNewChild, "BackwardTreeDifferenceBuilder: New (value) node is missing !");

            // if (!pNewChild) return DONE; // error stop !
#endif

            return CONTINUE;
        }

        virtual Result handle(data::NodeAccess const & _aCacheNode)
        {
            // value nodes are handled separately
            OSL_ASSERT(!data::ValueNodeAccess::isInstance(_aCacheNode));

            OUString aNodeName = _aCacheNode.getName().toString();

            INode const* pNewChild = m_pNewNode->getChild(aNodeName);
            //OSL_ENSURE(pNewChild, "BackwardTreeDifferenceBuilder: New node is missing !");

            ISubtree const * pNewTree = pNewChild ? pNewChild->asISubtree() : NULL;
            //OSL_ENSURE(pNewChild, "BackwardTreeDifferenceBuilder: Inner node expected !");

            if (pNewTree)
            {
                // Traverse down to next change
                Change* pChange = m_rChangeList.getChange(aNodeName);

                std::auto_ptr<Change> pNewChange;
                SubtreeChange * pGroupChange = NULL;

                if (pChange)
                {
                    OSL_ENSURE(pChange->ISA(SubtreeChange),"BackwardTreeDifferenceBuilder: Found wrong change for this Group Node");
                    if (pChange->ISA(SubtreeChange))
                        pGroupChange = static_cast<SubtreeChange*>(pChange);
                }
                else
                {
                    pGroupChange = new SubtreeChange(*pNewTree);
                    pNewChange.reset( pGroupChange );
                }

                if (pGroupChange)
                {
                    BackwardTreeDifferenceBuilder aNextLevel(*pGroupChange, pNewTree);
                    aNextLevel.applyToChildren(_aCacheNode);

                    if (pNewChange.get())
                    {
                        // now count if there are any real changes
                        OChangeActionCounter aCounter;
                        aCounter.applyToChange(*pNewChange);

                        if (aCounter.hasChanges())
                            m_rChangeList.addChange(pNewChange);
                    }
                }
            }
            else
            {
                // return DONE; // error stop !
            }

            return CONTINUE;
        }

        virtual Result handle(data::TreeAccessor const & _aCacheElement)
        {
            OUString aElementName = _aCacheElement.getName().toString();

            INode const* pNewElement = m_pNewNode->getChild(aElementName);

            if (pNewElement)
            {
                // continue: handle the root node
                return SetVisitor::handle(_aCacheElement);
            }
            else
            {
                // Remove Node
                std::auto_ptr<Change> pRemove(new RemoveNode(aElementName,
                                 _aCacheElement->isNew()));

                m_rChangeList.addChange(pRemove);

                return CONTINUE;
            }
        }
    };
// -----------------------------------------------------------------------------
//--------------------------------------------------------------------------

// apply a set of changes to the target tree, return true, if there are changes found
    bool createUpdateFromDifference(SubtreeChange& _rResultingUpdateTree, data::NodeAccess const & _aExistingData, ISubtree const & _aNewData)
    {
        OSL_ENSURE( _aExistingData.isValid(), "Trying to create diffrence for empty data" );
    // create the differences
        ForwardTreeDifferenceBuilder aForwardTreeDifference(_rResultingUpdateTree, _aExistingData);
        aForwardTreeDifference.applyToChildren(_aNewData);

        BackwardTreeDifferenceBuilder aBackwardTreeDifference(_rResultingUpdateTree, & _aNewData);
        aBackwardTreeDifference.applyToChildren(_aExistingData);

        return true;
    }
//--------------------------------------------------------------------------

//..........................................................................
}   // namespace configmgr
//..........................................................................


