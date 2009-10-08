/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updatehelper.cxx,v $
 * $Revision: 1.14 $
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

#include <stdio.h>

#include "updatehelper.hxx"
#include "change.hxx"
#include "nodeconverter.hxx"
#include "treeactions.hxx"
#include "treechangefactory.hxx"
#include "treenodefactory.hxx"

// -----------------------------------------------------------------------------
#include "node.hxx"
#include "treefragment.hxx"
#include "builddata.hxx"
#include "nodevisitor.hxx"
// -----------------------------------------------------------------------------
#include "tracer.hxx"
#include <osl/diagnose.h>


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
    sharable::Node * m_refNode; // reference node needed for merging
    OTreeNodeConverter  m_aNodeConverter;
public:
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aUpdateTree,
                       sharable::Node * targetNode)
    {
        return AdjustUpdate(_rResultTree, targetNode).impl_adjust(_aUpdateTree);
    }
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aUpdateTree,
                       sharable::Node * targetNode,
                        OTreeNodeFactory& _rNodeFactory)
    {
        return AdjustUpdate(_rResultTree, targetNode, _rNodeFactory).impl_adjust(_aUpdateTree);
    }
private:
    AdjustUpdate(SubtreeChange& rList, sharable::Node * node)
        :m_rChangeList(rList)
        ,m_refNode(node)
        ,m_aNodeConverter()
    {}

    AdjustUpdate(SubtreeChange& rList, sharable::Node * node, OTreeNodeFactory& _rNodeFactory)
        :m_rChangeList(rList)
        ,m_refNode(node)
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
    sharable::Node *           m_aCurrentNode;
public:
    ApplyUpdate(sharable::Node * _aNode)
    : m_aCurrentNode(_aNode)
    {}

    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);
};
//--------------------------------------------------------------------------
class ApplyValueChange
{
    static
    void adjust(uno::Any& aActual, uno::Any const& aTarget);

public:
    static
    void apply(ValueChange& _rValueChange, sharable::ValueNode * valueNode);
};
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

// apply a already matching set of changes to the target tree
    void applyUpdateToTree(SubtreeChange& _anUpdateTree, sharable::Node * _aRootNode)
    {
        ApplyUpdate aUpdater(_aRootNode);
        _anUpdateTree.forEachChange(aUpdater);
    }
//--------------------------------------------------------------------------
// apply a set of changes to the target tree
    void applyUpdateWithAdjustmentToTree(SubtreeChange& _anUpdateTree, sharable::Node * _aRootNode)
    {
    // POST: pSubtree = pSubtree + aChangeList
    SubtreeChange aActualChanges(_anUpdateTree, treeop::NoChildCopy());

        if ( AdjustUpdate::adjust(aActualChanges,_anUpdateTree, _aRootNode) )
        {
        applyUpdateToTree(aActualChanges, _aRootNode);
        }
        _anUpdateTree.swap(aActualChanges);

    }
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

static inline
rtl::OUString getNodeName(INode const & _aNode)
{
    return _aNode.getName();
}
//--------------------------------------------------------------------------

static inline
rtl::OUString getChangeNodeName(Change const & _aChange)
{
    return _aChange.getNodeName();
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
    OSL_ENSURE(m_refNode != 0, "AdjustUpdate: no data");

    return m_refNode != 0;
}
//--------------------------------------------------------------------------

void AdjustUpdate::handle(ValueChange& _rChange)
{
    if (checkNode())
    {
        // We need to find the element in the tree
        sharable::Node * childNode = m_refNode->getSubnode(getChangeNodeName(_rChange));

        // We have a node so we can keep the Change and the values do not differ
        if (childNode != 0)
        {
            bool bIsValue = childNode->isValue();
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
            rtl::OUString sTypeName = m_rChangeList.getElementTemplateName();

            rtl::Reference< data::TreeSegment > aNewTree = data::TreeSegment::create(base_ptr(pNode),sTypeName);
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
        sharable::Node * childNode = m_refNode->getSubnode(getChangeNodeName(_rChange));

        // if there is a node we continue
        if (childNode != 0)
        {
            bool bIsSubtree = childNode->isGroup() || childNode->isSet();
            OSL_ENSURE(bIsSubtree, "AdjustUpdate : node must be a inner node!");

            if (bIsSubtree)
            {
                // generate a new change
                std::auto_ptr<SubtreeChange> pChange( new SubtreeChange(_rChange, treeop::NoChildCopy()) );

                // recurse
                if ( adjust(*pChange,_rChange,childNode,m_aNodeConverter.nodeFactory()) )
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
            rtl::OUString sTypeName = m_rChangeList.getElementTemplateName();

            rtl::Reference< data::TreeSegment > aNewTree = data::TreeSegment::create(base_ptr(pNode), sTypeName);

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
        sharable::Node * childNode = m_refNode->getSubnode(getChangeNodeName(_rChange));

        // only if there is a node, we will keep the change
        if (childNode != 0)
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
        sharable::Node * childNode = m_refNode->getSubnode(getChangeNodeName(_rChange));

        rtl::Reference< data::TreeSegment > aNewNode = _rChange.getNewTree();
        std::auto_ptr<AddNode> pChange( new AddNode(aNewNode,_rChange.getNodeName(),_rChange.isToDefault()) );
        if (childNode != 0)
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
void ApplyValueChange::apply(ValueChange& _rValueChange, sharable::ValueNode * valueNode)
{
    switch (_rValueChange.getMode())
    {
    case ValueChange::wasDefault:
        OSL_ASSERT(valueNode->info.isDefault());

    case ValueChange::changeValue:
        adjust( _rValueChange.m_aOldValue, valueNode->getValue());
        valueNode->setValue(_rValueChange.getNewValue());
        break;

    case ValueChange::setToDefault:
        adjust(_rValueChange.m_aOldValue, valueNode->getValue());
        adjust(_rValueChange.m_aValue, valueNode->getDefaultValue());
        valueNode->setToDefault();
        break;

    case ValueChange::changeDefault:
        adjust(_rValueChange.m_aOldValue, valueNode->getDefaultValue());
        valueNode->changeDefault(_rValueChange.getNewValue());
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

    sharable::Node * childNode = m_aCurrentNode->getSubnode(getChangeNodeName(_rChange));
    OSL_ENSURE(childNode != 0, "Cannot apply Change: No node to change");

    sharable::ValueNode * aValueAddr = childNode->valueData();
    OSL_ENSURE(aValueAddr != NULL,"Cannot apply ValueChange: Node is not a value");

    if (aValueAddr != NULL)
    ApplyValueChange::apply(_rChange,aValueAddr);
}
//--------------------------------------------------------------------------

void ApplyUpdate::handle(SubtreeChange& _rChange)
{
    // handle traversion
    OSL_ENSURE(m_aCurrentNode != NULL,"Cannot apply SubtreeChange without node");

    sharable::Node * childNode = m_aCurrentNode->getSubnode(getChangeNodeName(_rChange));
    OSL_ENSURE(childNode != 0, "Cannot apply Change: No node to change");

    OSL_ENSURE( childNode->isGroup() || childNode->isSet(),
                "Cannot Apply SubtreeChange: Node is not an inner node");

    if (childNode != 0)
    {
        childNode->info.markAsDefault( _rChange.isToDefault() );

        sharable::Node * aOldNode = m_aCurrentNode;
        m_aCurrentNode = childNode;

        _rChange.forEachChange(*this);

        m_aCurrentNode = aOldNode;
    }
}
//--------------------------------------------------------------------------

void ApplyUpdate::handle(AddNode& _rChange)
{
    OSL_ENSURE(m_aCurrentNode != NULL,"Cannot apply AddNode without node");

    sharable::SetNode * aSetNodeAddr = sharable::SetNode::from(m_aCurrentNode);
    OSL_ENSURE(aSetNodeAddr != NULL,"Cannot apply AddNode: Node is not a set node");

    // Add a new element
    if (aSetNodeAddr != NULL)
    {
    if (_rChange.isReplacing())
    {
        sharable::TreeFragment * old = aSetNodeAddr->removeElement(getChangeNodeName(_rChange));
        OSL_ASSERT(old != 0);
        _rChange.takeReplacedTree(data::TreeSegment::create(old));
    }

    sharable::TreeFragment * aNewAddress = data::buildTree(_rChange.getNewTree()->fragment);
        OSL_ENSURE(aNewAddress != NULL, "ApplyUpdate: AddNode: could not create new element");

        aSetNodeAddr->addElement(aNewAddress);

        _rChange.setInsertedAddress( aNewAddress );
    }
}
//--------------------------------------------------------------------------

void ApplyUpdate::handle(RemoveNode& _rChange)
{
    OSL_ENSURE(m_aCurrentNode != NULL,"Cannot apply RemoveNode without node");

    sharable::SetNode * aSetNodeAddr = sharable::SetNode::from(m_aCurrentNode);
    OSL_ENSURE(aSetNodeAddr != NULL,"Cannot apply RemoveNode: Node is not a set node");

    // Remove an element
    if (aSetNodeAddr != NULL)
    {
        sharable::TreeFragment * old = aSetNodeAddr->removeElement(getChangeNodeName(_rChange));
        OSL_ASSERT(old != 0);
        _rChange.takeRemovedTree(data::TreeSegment::create(old));
    }
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    struct ForwardTreeDifferenceBuilder : NodeAction
    {
    protected:
        SubtreeChange&      m_rChangeList;
        sharable::Node * m_cacheNode;

    public:
        ForwardTreeDifferenceBuilder(SubtreeChange& rList, sharable::Node * cacheNode)
        : m_rChangeList(rList)
        , m_cacheNode(cacheNode)
        {
        }

        virtual void handle(ValueNode const& _aNewNode)
        {
            sharable::Node * childNode = m_cacheNode->getSubnode(getNodeName(_aNewNode));

            OSL_ENSURE(childNode != 0, "TreeDifferenceBuilder: could not find expected node !");

            sharable::ValueNode * valueNode = childNode->valueData();

            OSL_ENSURE(valueNode != 0, "TreeDifferenceBuilder: node must be a value node!");

            // if the values differ add a new change
            if (_aNewNode.getValue() != valueNode->getValue())
            {
                bool bNewDefault = _aNewNode.isDefault();
                bool bOldDefault = valueNode->info.isDefault();

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
                                    _aNewNode.getValue(), valueNode->getValue()) );

                m_rChangeList.addChange(pChange);
            }
        }
        virtual void handle(ISubtree const& _aNewNode)
        {
            sharable::Node * childNode = m_cacheNode->getSubnode(getNodeName(_aNewNode));

            if (childNode != 0)
            {
                OSL_ENSURE( childNode->isGroup() || childNode->isSet(),
                            "ForwardTreeDifferenceBuilder: Node must be an inner node");

                // generate a new change
                std::auto_ptr<SubtreeChange> pNewChange( new SubtreeChange(_aNewNode) );

                // .. and recurse
                ForwardTreeDifferenceBuilder aNextLevel(*pNewChange, childNode);
                aNextLevel.applyToChildren(_aNewNode);

                // now count if there are any changes
                OChangeActionCounter aCounter;
                aCounter.applyToChange(*pNewChange);

                if (aCounter.hasChanges())
                    m_rChangeList.addChange(base_ptr(pNewChange));
            }
            else if (m_cacheNode != 0 && m_cacheNode->isSet())
            {
                // Subtree not in Cache, add in TreeChangeList
                // SubtreeChange* pChange = new SubtreeChange(_rSubtree);
                OSL_ENSURE(m_rChangeList.isSetNodeChange(), "Found newly added node in non-set node");
                rtl::OUString sTypeName = m_rChangeList.getElementTemplateName();

                std::auto_ptr<INode> pSubtree( _aNewNode.clone() );
                rtl::Reference< data::TreeSegment > aNewTree = data::TreeSegment::create(pSubtree,sTypeName);

                std::auto_ptr<Change> pAdd(new AddNode(aNewTree, _aNewNode.getName(), _aNewNode.isDefault()));

                m_rChangeList.addChange(pAdd);
            }
            else
                OSL_ENSURE(false, "Found newly added node in group");

        }
    };
// -----------------------------------------------------------------------------

class BackwardTreeDifferenceBuilder: public data::SetVisitor {
public:
    BackwardTreeDifferenceBuilder(SubtreeChange & list, ISubtree const * node):
        m_changeList(list), m_newNode(node) {}

    void applyToChildren(sharable::Node * cacheNode) {
        OSL_ASSERT(cacheNode != 0);
        if (cacheNode->isGroup()) {
            OSL_ASSERT(!m_changeList.isSetNodeChange());
            visitChildren(&cacheNode->group);
        } else if (cacheNode->isSet()) {
            OSL_ASSERT(m_changeList.isSetNodeChange());
            visitElements(&cacheNode->set);
        } else {
            OSL_ASSERT(false);
        }
    }

private:
    using NodeVisitor::handle;

    virtual bool handle(sharable::Node * node) {
        OSL_ASSERT(!node->isValue());
        INode const * newChild = m_newNode->getChild(node->getName());
        ISubtree const * newTree = newChild == 0 ? 0 : newChild->asISubtree();
        if (newTree != 0) {
            // Traverse down to next change:
            Change * change = m_changeList.getChange(node->getName());
            std::auto_ptr< Change > newChange;
            SubtreeChange * groupChange = 0;
            if (change == 0) {
                groupChange = new SubtreeChange(*newTree);
                newChange.reset(groupChange);
            } else {
                groupChange = dynamic_cast< SubtreeChange * >(change);
                OSL_ASSERT(groupChange != 0);
            }
            if (groupChange != 0) {
                BackwardTreeDifferenceBuilder(*groupChange, newTree).
                    applyToChildren(node);
                if (newChange.get() != 0) {
                    // Now count if there are any real changes:
                    OChangeActionCounter counter;
                    counter.applyToChange(*newChange);
                    if (counter.hasChanges()) {
                        m_changeList.addChange(newChange);
                    }
                }
            }
        }
        return false;
    }

    virtual bool handle(sharable::ValueNode *) {
        return false;
    }

    virtual bool handle(sharable::TreeFragment * tree) {
        INode const * newElement = m_newNode->getChild(tree->getName());
        if (newElement == 0) {
            // Remove node:
            std::auto_ptr< Change > remove(
                new RemoveNode(tree->getName(), tree->isNew()));
            m_changeList.addChange(remove);
            return false;
        } else {
            // Handle the root node:
            return SetVisitor::handle(tree);
        }
    }

    SubtreeChange & m_changeList;
    ISubtree const * m_newNode;
};

//--------------------------------------------------------------------------

// apply a set of changes to the target tree, return true, if there are changes found
    bool createUpdateFromDifference(SubtreeChange& _rResultingUpdateTree, sharable::Node * existingData, ISubtree const & _aNewData)
    {
        OSL_ENSURE(existingData != 0, "Trying to create diffrence for empty data");
    // create the differences
        ForwardTreeDifferenceBuilder aForwardTreeDifference(_rResultingUpdateTree, existingData);
        aForwardTreeDifference.applyToChildren(_aNewData);

        BackwardTreeDifferenceBuilder aBackwardTreeDifference(_rResultingUpdateTree, & _aNewData);
        aBackwardTreeDifference.applyToChildren(existingData);

        return true;
    }
//--------------------------------------------------------------------------

//..........................................................................
}   // namespace configmgr
//..........................................................................


