/*************************************************************************
 *
 *  $RCSfile: mergehelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:37:09 $
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

#include <stdio.h>

#include "mergehelper.hxx"

#ifndef CONFIGMGR_NODECONVERTER_HXX
#include "nodeconverter.hxx"
#endif
#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif

#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif
#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


//..........................................................................
namespace configmgr
{

namespace
{
//==========================================================================
//= OCleanupLayerAction
//==========================================================================
//= This class cleans up a layer to be merged into an existing tree
//==========================================================================
class OCleanupLayerAction : ChangeTreeModification
{
    SubtreeChange&      m_rResultTree;  // list which containes changes merged with the existing nodes
    ISubtree const &    m_rTargetTree;      // reference node needed for merging
    OTreeNodeConverter  m_aNodeConverter;
public:
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aLayerTree, ISubtree const& _aTargetTree)
    {
        return OCleanupLayerAction(_rResultTree,_aTargetTree).impl_cleanup(_aLayerTree);
    }
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aLayerTree, ISubtree const& _aTargetTree, OTreeNodeFactory& _rNodeFactory)
    {
        return OCleanupLayerAction(_rResultTree,_aTargetTree,_rNodeFactory).impl_cleanup(_aLayerTree);
    }

private:
    OCleanupLayerAction(SubtreeChange& _rResult, ISubtree const& _rTree)
        :m_rResultTree(_rResult)
        ,m_rTargetTree(_rTree)
        ,m_aNodeConverter()
    {}

    OCleanupLayerAction(SubtreeChange& _rResult, ISubtree const& _rTree, OTreeNodeFactory& _rNodeFactory)
        :m_rResultTree(_rResult)
        ,m_rTargetTree(_rTree)
        ,m_aNodeConverter(_rNodeFactory)
    {}

    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);

    bool impl_cleanup(SubtreeChange& _aUpdateTree);

    void add(std::auto_ptr<Change> _pChange);
    void addReplacedNode(data::TreeSegment const & _aReplacedTree);
    void addReplacedNode(std::auto_ptr<INode> _aReplacedNode);
};

// --------------------------------- MergeLayerToTree ---------------------------------

class MergeLayerToTree : ChangeTreeModification
{
    ISubtree& m_rTree;

public:
    explicit
    MergeLayerToTree(ISubtree& _rTree):m_rTree(_rTree) {}

    MergeLayerToTree& merge(SubtreeChange & _rLayerTree);
private:
    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);

    static void mergeAttributes(INode& _rNode, node::Attributes const& _aChangeAttributes);
};
// --------------------------------- AttributeSetter ---------------------------------

class AttributeSetter : NodeModification
{
    node::State m_state;
    bool        m_bPromoteFinalized;
public:
    explicit
    AttributeSetter(node::State _state, bool _bPromoteFinalized)
    : m_state(_state)
    , m_bPromoteFinalized(_bPromoteFinalized)
    {}

    void setNodeAttributes(INode& _rNode);

    using NodeModification::applyToNode;
private:
    void handle(ValueNode& _rValueNode);
    void handle(ISubtree& _rSubtree);
};
// -----------------------------------------------------------------------------

} // anon namepsace
// -----------------------------------------------------------------------------
// this is our 'exported' function
void mergeLayerToTree(SubtreeChange & _aLayerTree,ISubtree& _aTree)
{
    // coarse: _aTree += _aLayerTree;

    AttributeSetter(node::isDefault, true).applyToNode(_aTree);

    SubtreeChange aMergeChange(_aLayerTree, SubtreeChange::NoChildCopy());

    if (OCleanupLayerAction::adjust(aMergeChange,_aLayerTree,_aTree))
        MergeLayerToTree(_aTree).merge(aMergeChange);
}

// -----------------------------------------------------------------------------
namespace
{
//==========================================================================
//= OCleanupLayerAction
//==========================================================================
static inline bool isFinal(node::Attributes const& _aAttributes)
{
    return _aAttributes.isFinalized() || _aAttributes.isReadonly();
}
//==========================================================================
bool OCleanupLayerAction::impl_cleanup(SubtreeChange& _aUpdateTree)
{
    OSL_ENSURE(!_aUpdateTree.isReplacedNode(), "Layer cleanup: A replaced tree should not be merged");

    if (isFinal(m_rTargetTree.getAttributes()))
    {
        CFG_TRACE_WARNING("Layer cleanup : Ignoring change to write-protected tree '%s'",OUSTRING2ASCII(m_rTargetTree.getName()));
        return false;
    }

    // first check the changes
    this->applyToChildren(_aUpdateTree);

    return m_rResultTree.size() != 0;
}
//--------------------------------------------------------------------------
inline void OCleanupLayerAction::add(std::auto_ptr<Change> _aChange)
{
    m_rResultTree.addChange(_aChange);
}
//--------------------------------------------------------------------------
void OCleanupLayerAction::handle(ValueChange& _rChange)
{
    OUString const sNodeName = _rChange.getNodeName();

    OSL_ENSURE(!_rChange.isToDefault(),"Found change to default in layer being merged");

    // replaced state -> should be a full (added/replaced) node
    //if (_rChange.isReplacedNode() && m_rTargetTree.isSetNodeChange())
    if ( _rChange.isReplacedValue() )
    {
        std::auto_ptr<ValueNode> pNode( m_aNodeConverter.createCorrespondingNode(_rChange) );

        this->addReplacedNode( base_ptr(pNode) );

    }
    else if (INode const * const pTargetNode = m_rTargetTree.getChild(sNodeName))
    {
        // this mismatch is not discarded here (should be ignored while merging though)
        OSL_ENSURE(pTargetNode->asValueNode(), "Layer cleanup : Node type mismatch: Value change applied to non-value node !");

        if (!isFinal(pTargetNode->getAttributes()))
        {
            std::auto_ptr<Change> pResult( new ValueChange(_rChange) );
            this->add(pResult);
        }
        else
        {
            CFG_TRACE_WARNING("Layer cleanup : Ignoring change to write-protected value '%s'",OUSTRING2ASCII(sNodeName));
        }
    }
    else
    {
        OSL_TRACE("Layer cleanup : Found orphaned node (value) '%s'",OUSTRING2ASCII(sNodeName));
        CFG_TRACE_INFO("Layer cleanup : Found orphaned node (value) '%s'",OUSTRING2ASCII(sNodeName));
        OSL_ENSURE(false, "Layer cleanup : Found orphaned Value");
    }
}

//--------------------------------------------------------------------------
void OCleanupLayerAction::handle(SubtreeChange& _rChange)
{
    OUString const sNodeName = _rChange.getNodeName();

    OSL_ENSURE(!_rChange.isToDefault(),"Found change to default in layer being merged");

    // replaced state -> should be a full (added/replaced) node
    if (_rChange.isReplacedNode())
    {
        std::auto_ptr<ISubtree> pNode = m_aNodeConverter.createCorrespondingTree(_rChange);

        // mark as complete with defaults)
        pNode->setLevels(treeop::ALL_LEVELS,treeop::ALL_LEVELS);

        this->addReplacedNode( base_ptr(pNode) );
    }
    else if ( INode const * const pTargetNode = m_rTargetTree.getChild(sNodeName) )
    {
        ISubtree const * pTargetTree = pTargetNode->asISubtree();

        if (pTargetTree)
        {
            // generate a new change
            std::auto_ptr<SubtreeChange> pResult( new SubtreeChange(_rChange, SubtreeChange::NoChildCopy()) );

            // recurse
            if ( adjust(*pResult,_rChange,*pTargetTree,m_aNodeConverter.nodeFactory()) )
                this->add(base_ptr(pResult));

            else
                CFG_TRACE_INFO("Layer cleanup : Found void modification tree '%s'",OUSTRING2ASCII(sNodeName));
        }
        else
        {
            OSL_ENSURE(false, "Layer cleanup : Node type mismatch: Tree change applied to non-tree node !");
            CFG_TRACE_ERROR("Layer cleanup : Discarding schema violation for node '%s'",OUSTRING2ASCII(sNodeName));
            //throw Whatever();
        }
    }
    else
    {
        OSL_TRACE("Layer cleanup : Found orphaned node (subtree) '%s'",OUSTRING2ASCII(sNodeName));
        CFG_TRACE_INFO("Layer cleanup : Found orphaned node (subtree) '%s'",OUSTRING2ASCII(sNodeName));
    }
}

//--------------------------------------------------------------------------
void OCleanupLayerAction::handle(RemoveNode& _rChange)
{
    OUString const sNodeName = _rChange.getNodeName();

    OSL_ENSURE(!_rChange.isToDefault(),"Found change to default in layer being merged");

    OSL_ENSURE(m_rTargetTree.isSetNode(),"Found RemoveNode for non-set-element in layer being merged");

    // look up the corresponding node
    INode const * const pTargetNode = m_rTargetTree.getChild(sNodeName);

    if (pTargetNode)
    {
        // generate the same change
        std::auto_ptr<Change> pResult( new RemoveNode(sNodeName,false) );
        this->add(pResult);
    }
    else
    {
        OSL_TRACE("Layer cleanup : Found orphaned node (removal) '%s'",OUSTRING2ASCII(sNodeName));
        CFG_TRACE_INFO("Layer cleanup : Found orphaned node (removal) '%s'",OUSTRING2ASCII(sNodeName));
    }
}

//--------------------------------------------------------------------------
void OCleanupLayerAction::handle(AddNode& _rChange)
{
    OSL_ENSURE(!_rChange.isToDefault(),"Found change to default in layer being merged");

// generate the same change
    this->addReplacedNode( _rChange.getNewTree() );
}

//--------------------------------------------------------------------------
void OCleanupLayerAction::addReplacedNode(std::auto_ptr<INode> _aReplacedNode)
{
    OSL_ENSURE(m_rTargetTree.isSetNode(),"Found replaced node for non-set-element in layer being merged");

    OSL_ASSERT(_aReplacedNode.get());

    OUString sTypeName = m_rTargetTree.getElementTemplateName();

    this->addReplacedNode( data::TreeSegment::createNew(_aReplacedNode,sTypeName) );
}

//--------------------------------------------------------------------------
void OCleanupLayerAction::addReplacedNode(data::TreeSegment const & _aReplacedTree)
{
    OSL_ENSURE(m_rTargetTree.isSetNode(),"Found replaced node for non-set-element in layer being merged");

    OSL_ASSERT(_aReplacedTree.is());

    OUString sNodeName = _aReplacedTree.getName().toString();

    // add the tree to the change list
    std::auto_ptr<AddNode> pResult( new AddNode(_aReplacedTree,sNodeName, false) );

    // look up the corresponding existing node
    INode const * const pTargetNode = m_rTargetTree.getChild(sNodeName);

    if (pTargetNode) pResult->setReplacing();

    this->add( base_ptr(pResult) );
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void MergeLayerToTree::mergeAttributes(INode& _rNode, node::Attributes const& _aChangeAttributes)
{
    OSL_ENSURE(!isFinal(_rNode.getAttributes()),"Layer merge: Node being merged is READONLY - cleanup broken");
    OSL_ENSURE(_aChangeAttributes.state() == node::isMerged,"Layer merge: Found unexpected state for change being merged");

    _rNode.modifyState(node::isMerged);
    _rNode.modifyAccess(_aChangeAttributes.getAccess());
}
//--------------------------------------------------------------------------

MergeLayerToTree& MergeLayerToTree::merge(SubtreeChange & _rLayerTree)
{
    OSL_ENSURE(_rLayerTree.getNodeName() == m_rTree.getName(),"Layer merge: change does not match tree");

    this->applyToChildren(_rLayerTree);
    mergeAttributes(m_rTree,_rLayerTree.getAttributes());

    return *this;
}
//--------------------------------------------------------------------------

void MergeLayerToTree::handle(ValueChange& _rChange)
{
    OSL_ENSURE(!_rChange.isToDefault(),"Layer merge: Found change to default - cleanup broken");

    OUString const sNodeName = _rChange.getNodeName();

    INode * const pTargetNode = m_rTree.getChild(sNodeName);

    OSL_ENSURE(pTargetNode,"Layer merge: Found NULL value - cleanup broken");
    OSL_ENSURE(!isFinal(pTargetNode->getAttributes()),"Layer merge: Found READONLY value - cleanup broken");

    if (ValueNode* pTargetValue = pTargetNode->asValueNode() )
    {
        _rChange.applyChangeNoRecover(*pTargetValue);
        mergeAttributes(*pTargetValue,_rChange.getAttributes());
    }
    else
    {
        OSL_ENSURE(false, "Layer merge : Node type mismatch: Value change applied to non-value node !");
        CFG_TRACE_ERROR("Layer merge : Discarding schema violation for value '%s'",OUSTRING2ASCII(sNodeName));
        //throw Whatever();
    }
}
//--------------------------------------------------------------------------

void MergeLayerToTree::handle(AddNode& _rChange)
{
    OUString const sNodeName = _rChange.getNodeName();

    node::State eNodeState = node::isAdded;

    if (_rChange.isReplacing())
    {
        OSL_VERIFY( m_rTree.removeChild(sNodeName).get() );
        eNodeState = node::isReplaced;
    }

    OSL_ENSURE( !m_rTree.getChild(sNodeName),"Layer merge: Found conflicting data on insert - cleanup broken");

    data::TreeSegment aAddedTree = _rChange.getNewTree();

    OSL_ENSURE( aAddedTree.is(),"Layer merge: Found empty data on insert - cleanup broken");

    // clean up the attributes of the added node
    data::TreeSegment::RawTreeData aAddedData = aAddedTree.cloneData(true);

    AttributeSetter(eNodeState,false).applyToNode(*aAddedData);

    m_rTree.addChild(aAddedData);
}
//--------------------------------------------------------------------------

void MergeLayerToTree::handle(RemoveNode& _rChange)
{
    OUString const sNodeName = _rChange.getNodeName();

    // should have such a node when removing
    OSL_VERIFY( m_rTree.removeChild(sNodeName).get() );
}
//--------------------------------------------------------------------------


void MergeLayerToTree::handle(SubtreeChange& _rChange)
{
    OSL_ENSURE(!_rChange.isToDefault(),"Layer merge: Found change to default - cleanup broken");

    OUString const sNodeName = _rChange.getNodeName();

    INode * const pTargetNode = m_rTree.getChild(sNodeName);

    OSL_ENSURE(pTargetNode,"Layer merge: Found NULL subtree - cleanup broken");
    OSL_ENSURE(!isFinal(pTargetNode->getAttributes()),"Layer merge: Found READONLY subtree - cleanup broken");

    ISubtree * const pTargetTree = pTargetNode->asISubtree();
    OSL_ENSURE(pTargetTree,"Layer merge: Found non-tree for SubtreeChange - cleanup broken");

    // recurse
    MergeLayerToTree(*pTargetTree).merge(_rChange);
}
//--------------------------------------------------------------------------

void AttributeSetter::setNodeAttributes(INode& _rNode)
{
    node::Attributes const aOldAttributes = _rNode.getAttributes();

    _rNode.modifyState(m_state);
    if (m_bPromoteFinalized && isFinal(aOldAttributes))
        _rNode.modifyAccess(node::accessReadonly);
}
// -----------------------------------------------------------------------------

void AttributeSetter::handle(ValueNode& _rValueNode)
{
    setNodeAttributes(_rValueNode);
}
// -----------------------------------------------------------------------------

void AttributeSetter::handle(ISubtree& _rSubtree)
{
    setNodeAttributes(_rSubtree);

    this->applyToChildren(_rSubtree);
}
//--------------------------------------------------------------------------

} // anonymous namespace
//..........................................................................
}   // namespace configmgr
//..........................................................................


