/*************************************************************************
 *
 *  $RCSfile: treeactions.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-03 16:29:28 $
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
//= OIdPropagator
//==========================================================================

void OIdPropagator::propagateIdToChildren(ISubtree& rTree)
{
    if (!rTree.hasId())
    {
        OIdPropagator aAction(rTree.getId());
        aAction.applyToChildren(rTree);
    }
}
//--------------------------------------------------------------------------

void OIdPropagator::propagateIdToTree(OUString const& aId, ISubtree& rTree)
{
    OSL_ENSURE(!rTree.hasId(), "OIdPropagator::propagateIdToTree: Tree already has an Id, propagating may not work");
    rTree.setId(aId);
    propagateIdToChildren(rTree);
}
//--------------------------------------------------------------------------

void OIdPropagator::handle(ValueNode& _rValueNode)
{ /* not interested in value nodes */ }
//--------------------------------------------------------------------------

void OIdPropagator::handle(ISubtree& _rSubtree)
{
    if (!_rSubtree.hasId())
    {
        _rSubtree.setId(sId);
        applyToChildren(_rSubtree);
    }
}

//==========================================================================
//= OIdRemover
//==========================================================================

void OIdRemover::removeIds(INode& rNode)
{
    OIdRemover().applyToNode(rNode);
}
//--------------------------------------------------------------------------

void OIdRemover::handle(ValueNode& _rValueNode)
{ /* not interested in value nodes */ }
//--------------------------------------------------------------------------

void OIdRemover::handle(ISubtree& _rSubtree)
{
    if (_rSubtree.hasId())
    {
        _rSubtree.setId(OUString());
        applyToChildren(_rSubtree);
    }
}

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
        OSL_ENSURE(pValueNode, "OBuildChangeTree::handle : node must be a value node!");

        if (pValueNode && _rChange.getNewValue() != pValueNode->getValue())
        {
            ValueChange* pChange = new ValueChange(_rChange);
            m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
        }
    }
    else
    {
        ValueNode* pNode = new ValueNode(_rChange.getNodeName(), _rChange.getNewValue(), _rChange.getAttributes());

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
        OSL_ENSURE(pSubTree, "OMergeTreeAction::handle : node must be a inner node!");

        if (pSubTree)
        {
            // generate a new change
            SubtreeChange* pChange = new SubtreeChange(_rChange, SubtreeChange::NoChildCopy());

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
        Subtree* pNode = new Subtree(_rChange.getNodeName(),
                                     _rChange.getElementTemplateName(),
                                     _rChange.getElementTemplateModule(),
                                     _rChange.getAttributes());

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
        OSL_ENSURE(pValueNode && aNewNode.get() && aNewNode.get()->asValueNode(), "OMergeTreeAction::handle : node must be a value node!");
        if (!pValueNode || !aNewNode.get() || !aNewNode.get()->asValueNode())
            return;

        ValueNode* pNewValueNode = aNewNode.get()->asValueNode();
        if (pNewValueNode->getValue() != pValueNode->getValue())
        {
            ValueChange* pChange = new ValueChange(_rChange.getNodeName(),
                                                   pNewValueNode->getValue(),
                                                   pNewValueNode->getAttributes(),
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
//= OChangeCounter
//==========================================================================
void OChangeCounter::handle(ValueChange const& aValueNode)
{
    ++nCount;
}

void OChangeCounter::handle(AddNode const& aAddNode)
{
    ++nCount;
}

void OChangeCounter::handle(RemoveNode const& aRemoveNode)
{
    ++nCount;
}

void OChangeCounter::handle(SubtreeChange const& aSubtree) { applyToChildren(aSubtree); }


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
    ValueNode* pNode = new ValueNode(_rChange.getNodeName(), _rChange.getNewValue(), _rChange.getAttributes());
    m_pTree->addChild(std::auto_ptr<INode>(pNode));
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(SubtreeChange& _rChange)
{
    ensure();

    // create a node by a ValueChange
    Subtree* pNode = new Subtree(_rChange.getNodeName(),
                                 _rChange.getElementTemplateName(),
                                 _rChange.getElementTemplateModule(),
                                 _rChange.getAttributes());

    // add it to the tree
    m_pTree->addChild(auto_ptr<INode>(pNode));

    // and continue
    OCreateSubtreeAction aNextLevel(pNode);
    _rChange.forEachChange(aNextLevel);
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(RemoveNode& _rChange)
{
    // we have nothing to do
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(AddNode& _rChange)
{
    ensure();

    // free the node and add it to the subtree
    std::auto_ptr<INode> aNewNode = _rChange.releaseAddedNode();
    m_pTree->addChild(aNewNode);
}

    // --------------------------------- updateTree ---------------------------------
    void TreeUpdate::handle(ValueChange& aValueNode)
    {
        // Change a Value
        OSL_ENSURE(m_pCurrentSubtree,"Cannot apply ValueChange without subtree");

        INode* pBaseNode = m_pCurrentSubtree ? m_pCurrentSubtree->getChild(aValueNode.getNodeName()) : 0;
        OSL_ENSURE(pBaseNode,"Cannot apply Change: No node to change");

        ValueNode* pValue = pBaseNode ? pBaseNode->asValueNode() : 0;
        OSL_ENSURE(pValue,"Cannot apply ValueChange: Node is not a value");

        if (pValue)
            aValueNode.applyTo(*pValue);
#ifdef DBUG
        else
        {
            ::rtl::OString aStr("TreeUpdate: Can't find value with name:=");
            aStr += rtl::OUStringToOString(aValueNode.getNodeName(),RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(pValue, aStr.getStr());
            aLog.push_back(aStr);
        }
#endif
    }

    void TreeUpdate::handle(AddNode& aAddNode)
    {
        // Add a new Value
        if (m_pCurrentSubtree)
        {
            if (aAddNode.isReplacing())
            {
                std::auto_ptr<INode> aOldNode = m_pCurrentSubtree->removeChild(aAddNode.getNodeName());

#ifdef DBUG
                OSL_ENSURE(aOldNode.get(), "TreeUpdate:AddNode: can't recover node being replaced");
                if (aOldNode.get() == NULL)
                    aLog.push_back(OString("TreeUpdate: can't recover node being replaced (for AddNode)"));
#endif
                if (aOldNode.get() != NULL)
                {
                    OIdRemover::removeIds(*aOldNode);
                }

                aAddNode.takeReplacedNode( aOldNode );
            }

            m_pCurrentSubtree->addChild(aAddNode.releaseAddedNode());

            OIdPropagator::propagateIdToChildren(*m_pCurrentSubtree);
        }
#ifdef DBUG
        else
            aLog.push_back(OString("TreeUpdate: no CurrentSubtree for AddNode"));
#endif

    }

    void TreeUpdate::handle(RemoveNode& aRemoveNode)
    {
        // remove a Value
        if (m_pCurrentSubtree)
        {
            std::auto_ptr<INode> aOldNode = m_pCurrentSubtree->removeChild(aRemoveNode.getNodeName());

            sal_Bool bOk = (NULL != aOldNode.get());
            if (bOk)
            {
                OIdRemover::removeIds(*aOldNode);
            }
            aRemoveNode.takeRemovedNode( aOldNode );

#ifdef DBUG
            if (!bOk)
            {
                ::rtl::OString aStr("TreeUpdate: Can't remove child with name:=");
                aStr += rtl::OUStringToOString(aRemoveNode.getNodeName(),RTL_TEXTENCODING_ASCII_US);
                OSL_ENSURE(bOk, aStr.getStr());
                aLog.push_back(aStr);
            }
#endif
        }
    }


    void TreeUpdate::handle(SubtreeChange& aSubtree)
    {
        // handle traversion
        ISubtree *pOldSubtree = m_pCurrentSubtree;
        OSL_ENSURE(m_pCurrentSubtree->getChild(aSubtree.getNodeName()), "TreeUpdate::handle : invalid subtree change ... this will crash !");
        m_pCurrentSubtree = m_pCurrentSubtree->getChild(aSubtree.getNodeName())->asISubtree();

#if DBUG
        ::rtl::OString aStr("TreeUpdate: there is no Subtree for name:=");
        aStr += rtl::OUStringToOString(aSubtree.getNodeName(),RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(m_pCurrentSubtree, aStr.getStr());
        if (!m_pCurrentSubtree)
            aLog.push_back(aStr);
#endif

        aSubtree.forEachChange(*this);
        m_pCurrentSubtree = pOldSubtree;
    }

//..........................................................................
}   // namespace configmgr
//..........................................................................


