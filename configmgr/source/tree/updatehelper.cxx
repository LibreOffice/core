/*************************************************************************
 *
 *  $RCSfile: updatehelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-11-14 16:35:14 $
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

#include "updatehelper.hxx"

#ifndef CONFIGMGR_NODECONVERTER_HXX
#include "nodeconverter.hxx"
#endif
#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif
#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif
#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif
#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
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

//==========================================================================
//= OAdjustUpdateAction
//==========================================================================
//= This class tests changes on an existing tree and drops them if they
//= are not need anymore or alters add nodes in node changes and vice versa
//==========================================================================
class OAdjustUpdateAction : ChangeTreeModification
{
    SubtreeChange&      m_rChangeList;  // list which containes changes merged with the existing nodes
    const ISubtree*     m_pRefTree;     // reference node needed for merging
    OTreeNodeConverter  m_aNodeConverter;
public:
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aUpdateTree, ISubtree const& _aTargetTree)
    {
        return OAdjustUpdateAction(_rResultTree,&_aTargetTree).impl_adjust(_aUpdateTree);
    }
    static bool adjust(SubtreeChange& _rResultTree, SubtreeChange& _aUpdateTree, ISubtree const& _aTargetTree, OTreeNodeFactory& _rNodeFactory)
    {
        return OAdjustUpdateAction(_rResultTree,&_aTargetTree,_rNodeFactory).impl_adjust(_aUpdateTree);
    }

private:
    OAdjustUpdateAction(SubtreeChange& rList, const ISubtree* pTree)
        :m_rChangeList(rList)
        ,m_pRefTree(pTree)
        ,m_aNodeConverter()
    {}

    OAdjustUpdateAction(SubtreeChange& rList, const ISubtree* pTree, OTreeNodeFactory& _rNodeFactory)
        :m_rChangeList(rList)
        ,m_pRefTree(pTree)
        ,m_aNodeConverter(_rNodeFactory)
    {}

    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);

    bool impl_adjust(SubtreeChange& _aUpdateTree);
private:
    // ensuring the correct state
    bool checkTree() const;
};

// --------------------------------- TreeUpdate ---------------------------------

class TreeUpdate : public ChangeTreeModification
{
    ISubtree* m_pCurrentSubtree;
#if DBUG
    std::vector<OString> aLog;
#endif

public:
    TreeUpdate(ISubtree* pSubtree):m_pCurrentSubtree(pSubtree){}

    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);
};

// -----------------------------------------------------------------------------
    bool adjustUpdateToTree(SubtreeChange& _rUpdateTree, ISubtree const& _aTargetTree)
    {
        SubtreeChange aResultTree(_rUpdateTree, SubtreeChange::NoChildCopy());

        bool bResult = OAdjustUpdateAction::adjust(aResultTree,_rUpdateTree,_aTargetTree);

        _rUpdateTree.swap(aResultTree);

        return bResult;
    }
// -----------------------------------------------------------------------------
    void applyUpdateToTree(SubtreeChange& _anUpdateTree, ISubtree& _aTree)
    {
        TreeUpdate aTreeUpdate(&_aTree);
        _anUpdateTree.forEachChange(aTreeUpdate);
    }

// -----------------------------------------------------------------------------
    void applyUpdateWithAdjustmentToTree(SubtreeChange& _anUpdateTree, ISubtree& _aTree)
    {
        // POST: pSubtree = pSubtree + aChangeList
        SubtreeChange aActualChanges(_anUpdateTree, SubtreeChange::NoChildCopy());

        if (OAdjustUpdateAction::adjust(aActualChanges,_anUpdateTree,_aTree))
            applyUpdateToTree(aActualChanges,_aTree);
    }

//==========================================================================
//= OAdjustUpdateAction
//==========================================================================
bool OAdjustUpdateAction::impl_adjust(SubtreeChange& _aUpdateTree)
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
inline bool OAdjustUpdateAction::checkTree() const
{
    // Change a Value
    OSL_ENSURE(m_pRefTree,"OMergeTreeAction::handle: Invalid subtree");

    // if we don't have a tree or the tree level is 0 we can leave
    return m_pRefTree != NULL;
}

//--------------------------------------------------------------------------
void OAdjustUpdateAction::handle(ValueChange& _rChange)
{
    if (checkTree())
    {
        // We need to find the element in the tree
        const INode* pChild = m_pRefTree->getChild(_rChange.getNodeName());

        // We have a node so we can keep the Change and the values do not differ
        if (pChild)
        {
            const ValueNode* pValueNode = pChild ? pChild->asValueNode() : NULL;
            OSL_ENSURE(pValueNode, "OBuildChangeTree::handle : node must be a value node!");

            if (pValueNode && _rChange.isChange())
            {
                std::auto_ptr<Change> pChange( new ValueChange(_rChange) );
                m_rChangeList.addChange(pChange);
            }
        }
        else
        {
            std::auto_ptr<ValueNode> pNode = m_aNodeConverter.createCorrespondingNode(_rChange);

            // add the tree to the change list
            std::auto_ptr<Change> pChange( new AddNode( base_ptr(pNode),_rChange.getNodeName(), _rChange.isToDefault()) );
            m_rChangeList.addChange(pChange);
        }
    }
}

//--------------------------------------------------------------------------
void OAdjustUpdateAction::handle(SubtreeChange& _rChange)
{
    if (checkTree())
    {
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
                std::auto_ptr<SubtreeChange> pChange( new SubtreeChange(_rChange, SubtreeChange::NoChildCopy()) );

                // recurse
                if ( adjust(*pChange,_rChange,*pSubTree,m_aNodeConverter.nodeFactory()) )
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
            pNode->setLevels(ITreeProvider::ALL_LEVELS,ITreeProvider::ALL_LEVELS);

            // add the tree to the change list
            std::auto_ptr<Change> pChange( new AddNode(base_ptr(pNode),_rChange.getNodeName(), _rChange.isToDefault()) );
            m_rChangeList.addChange( pChange );
        }
    }
}

//--------------------------------------------------------------------------
void OAdjustUpdateAction::handle(RemoveNode& _rChange)
{
    if (checkTree())
    {
        // We need to find the element in the tree
        const INode* pChild = m_pRefTree->getChild(_rChange.getNodeName());

        // only if there is a node, we will keep the change
        if (pChild)
        {
            // generate a new change
            std::auto_ptr<Change> pChange( new RemoveNode(_rChange.getNodeName(),_rChange.isToDefault()) );
            m_rChangeList.addChange(pChange);
        }
    }
}

//--------------------------------------------------------------------------
void OAdjustUpdateAction::handle(AddNode& _rChange)
{
    if (checkTree())
    {
        // We need to find the element in the tree
        const INode* pChild = m_pRefTree->getChild(_rChange.getNodeName());

        std::auto_ptr<INode> aNewNode = _rChange.releaseAddedNode();
        std::auto_ptr<AddNode> pChange( new AddNode(aNewNode,_rChange.getNodeName(),_rChange.isToDefault()) );
        if (pChild != NULL)
        {
            pChange->setReplacing();
        }
        m_rChangeList.addChange(base_ptr(pChange));
    }
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

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

        if (m_pCurrentSubtree)
            m_pCurrentSubtree->markAsDefault( aSubtree.isToDefault() );

        aSubtree.forEachChange(*this);
        m_pCurrentSubtree = pOldSubtree;
    }

//--------------------------------------------------------------------------


//..........................................................................
}   // namespace configmgr
//..........................................................................


