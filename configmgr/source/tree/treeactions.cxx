/*************************************************************************
 *
 *  $RCSfile: treeactions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-23 12:03:33 $
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

#include "treeactions.hxx"
#include "cmtree.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


//..........................................................................
namespace configmgr
{

//==========================================================================
//= OChangeActionCounter
//==========================================================================

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(ValueChange const& aValueNode){ ++nValues; }

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(AddNode const& aAddNode){ ++nAdds; }

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(RemoveNode const& aRemoveNode){ ++nRemoves; }

//--------------------------------------------------------------------------
void OChangeActionCounter::handle(SubtreeChange const& aSubtree)
{
    applyToChildren(aSubtree);
}

//==========================================================================
//= OMergeTreeAction
//==========================================================================
void OMergeTreeAction::ensure()
{
    // Change a Value
    OSL_ENSURE(m_pRefTree,"OMergeTreeAction::handle: Invalid subtree");

    // if we don't have a tree or the tree level is 0 we can leave
    if (!m_pRefTree || 0 == m_pRefTree->getLevel())
        return;
}

//--------------------------------------------------------------------------
void OMergeTreeAction::handle(ValueChange& _rChange)
{
    ensure();

    // We need to find the element in the tree
    const INode* pChild = m_pRefTree->getChild(_rChange.getNodeName());

    // We have a node so we can keep the Change and the values do not differ
    if (pChild)
    {
        const ValueNode* pValueNode = pChild ? pChild->asValueNode() : NULL;
        OSL_ENSHURE(pValueNode, "OBuildChangeTree::handle : node must be a value node!");

        if (pValueNode && _rChange.getNewValue() != pValueNode->getValue())
        {
            ValueChange* pChange = new ValueChange(_rChange);
            m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
        }
    }
    else
    {
        ValueNode* pNode = new ValueNode(_rChange.getNodeName(), _rChange.getNewValue());

        // add the tree to the change list
        AddNode* pChange = new AddNode(auto_ptr<INode>(pNode),_rChange.getNodeName());
        m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
    }
}

//--------------------------------------------------------------------------
void OMergeTreeAction::handle(SubtreeChange& _rChange)
{
    ensure();

    // We need to find the element in the tree
    const INode* pChild = m_pRefTree->getChild(_rChange.getNodeName());

    // if there is a node we continue
    if (pChild)
    {
        const ISubtree* pSubTree = pChild->asISubtree();
        OSL_ENSHURE(pSubTree, "OMergeTreeAction::handle : node must be a inner node!");

        if (pSubTree)
        {
            // generate a new change
            SubtreeChange* pChange = new SubtreeChange(_rChange.getNodeName());
            pChange->setChildTemplateName(_rChange.getChildTemplateName());

            OMergeTreeAction aNextLevel(*pChange, pSubTree);
            _rChange.forEachChange(aNextLevel);

            // now count if there are any changes
            OChangeActionCounter aCounter;
            aCounter.handle(*pChange);

            if (aCounter.hasChanges())
                m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
            else
                delete pChange;
        }
    }
    // otherwise we have to create the node
    else
    {
        Subtree* pNode = new Subtree(_rChange.getNodeName());
        pNode->setChildTemplateName(_rChange.getChildTemplateName());

        // add the subnodes
        OCreateSubtreeAction aNextLevel(pNode);
        _rChange.forEachChange(aNextLevel);

        // set the level
        pNode->setLevel(ITreeProvider::ALL_LEVELS);

        // add the tree to the change list
        AddNode* pChange = new AddNode(auto_ptr<INode>(pNode),_rChange.getNodeName());
        m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
    }
}

//--------------------------------------------------------------------------
void OMergeTreeAction::handle(RemoveNode& _rChange)
{
    ensure();

    // We need to find the element in the tree
    const INode* pChild = m_pRefTree->getChild(_rChange.getNodeName());

    // only if there is a node, we will keep the change
    if (pChild)
    {
        // generate a new change
        RemoveNode* pChange = new RemoveNode(_rChange.getNodeName());
        m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
    }
}

//--------------------------------------------------------------------------
void OMergeTreeAction::handle(AddNode& _rChange)
{
    ensure();

    // We need to find the element in the tree
    const INode* pChild = m_pRefTree->getChild(_rChange.getNodeName());

    // if the node exists, this must be a ValueNode,
    // otherwise we are not able to clone
    if (pChild)
    {
        const ValueNode* pValueNode = pChild->asValueNode();

        std::auto_ptr<INode> aNewNode = _rChange.releaseAddedNode();
        OSL_ENSHURE(pValueNode && aNewNode.get() && aNewNode.get()->asValueNode(), "OMergeTreeAction::handle : node must be a value node!");
        if (!pValueNode || !aNewNode.get() || !aNewNode.get()->asValueNode())
            return;

        ValueNode* pNewValueNode = aNewNode.get()->asValueNode();
        if (pNewValueNode->getValue() != pValueNode->getValue())
        {
            ValueChange* pChange = new ValueChange(_rChange.getNodeName(),
                                                   pNewValueNode->getValue(),
                                                   ValueChange::changeValue,
                                                   pValueNode->getValue());
            m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
        }
    }
    else
    {
        std::auto_ptr<INode> aNewNode = _rChange.releaseAddedNode();
        AddNode* pChange = new AddNode(aNewNode,_rChange.getNodeName());
        m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
    }
}

//==========================================================================
//= OCreateSubtreeAction
//==========================================================================
void OCreateSubtreeAction::ensure()
{
    // Change a Value
    OSL_ENSURE(m_pTree,"OCreateSubtreeAction::handle: Invalid Subtree");

    // if we don't have a tree or the tree level is 0 we can leave
    if (!m_pTree)
        return;
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(ValueChange& _rChange)
{
    ensure();

    // create a node by a ValueChange
    ValueNode* pNode = new ValueNode(_rChange.getNodeName(), _rChange.getNewValue());
    m_pTree->addChild(std::auto_ptr<INode>(pNode));
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(SubtreeChange& _rChange)
{
    ensure();

    // create a node by a ValueChange
    Subtree* pNode = new Subtree(_rChange.getNodeName());
    pNode->setChildTemplateName(_rChange.getChildTemplateName());

    // add it to the tree
    m_pTree->addChild(auto_ptr<INode>(pNode));

    // and continue
    OCreateSubtreeAction aNextLevel(pNode);
    _rChange.forEachChange(aNextLevel);
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(RemoveNode& _rChange)
{
    OSL_ENSURE(false,"OCreateSubtreeAction::handle: RemoveNode isn't valid");
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(AddNode& _rChange)
{
    ensure();

    // free the node and add it to the subtree
    std::auto_ptr<INode> aNewNode = _rChange.releaseAddedNode();
    m_pTree->addChild(aNewNode);
}


//..........................................................................
}   // namespace configmgr
//..........................................................................


