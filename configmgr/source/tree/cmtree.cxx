/*************************************************************************
 *
 *  $RCSfile: cmtree.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 09:01:03 $
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
/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#include "cmtree.hxx"

#include <deque>
#include <vector>
#include <iostream>
#include <exception>
#include <sal/types.h>
#include <set>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

#include "confname.hxx"                          // Iterator for PathName scans
#include "cmtreemodel.hxx"
#include "treeactions.hxx"

#include <com/sun/star/uno/Any.hxx>

// WISDOM
// !!Never write same code twice!!

using namespace std;
using namespace rtl;
using namespace com::sun::star::uno;

namespace configmgr
{

// ------------------------ ChildListSet implementations ------------------------
    ChildListSet::ChildListSet(ChildListSet const& aSet)
    {
        for(ChildList::iterator it = aSet.GetSet().begin();
            it != aSet.GetSet().end();
            ++it)
            m_aChildList.insert(m_aChildList.end(), (*it)->clone());
    }
    ChildListSet::~ChildListSet()
    {
        for(ChildList::iterator it = m_aChildList.begin();
            it != m_aChildList.end();
            ++it)
            delete *it;
    }


// ---------------------------- Node implementation ----------------------------

    INode::INode(configuration::Attributes _aAttr):m_aAttributes(_aAttr){}
    INode::INode(OUString const& aName, configuration::Attributes _aAttr)
          :m_aName(aName)
          ,m_aAttributes(_aAttr){}
    // CopyCTor will be create automatically

    INode::~INode() {}

    ISubtree*         INode::asISubtree(){return NULL;}
    ISubtree const*   INode::asISubtree() const {return NULL;}
    ValueNode*       INode::asValueNode() {return NULL;}
    ValueNode const* INode::asValueNode() const {return NULL;}


// ------------------------- SearchNode implementation -------------------------
    SearchNode::SearchNode():INode(configuration::Attributes()){}
    SearchNode::SearchNode(OUString const& aName)
        :INode(aName, configuration::Attributes()){}

    INode* SearchNode::clone() const {return new SearchNode(*this);}

    SearchNode::~SearchNode(){}

    //==========================================================================
    //= OPropagateLevels
    //==========================================================================
    /** fills a subtree with the correct level informations
    */
    struct OPropagateLevels : public NodeModification
    {
    protected:
        sal_Int32   nChildLevel;
    public:
        OPropagateLevels(sal_Int32 _nParentLevel)
        {
            nChildLevel = (ITreeProvider::ALL_LEVELS == _nParentLevel) ? ITreeProvider::ALL_LEVELS : _nParentLevel - 1;
        }
        virtual void handle(ValueNode&) { /* not interested in value nodes */ }
        virtual void handle(ISubtree& _rSubtree)
        {
            if ((ITreeProvider::ALL_LEVELS == nChildLevel) || nChildLevel > _rSubtree.getLevel())
                _rSubtree.setLevel(nChildLevel);
        }
    };


// -------------------------- ISubtree implementation --------------------------
    ISubtree* ISubtree::asISubtree() {return this;}
    ISubtree const* ISubtree::asISubtree() const {return this;}

    //--------------------------------------------------------------------------
    void ISubtree::setLevel(sal_Int16 _nLevel)
    {
        m_nLevel = _nLevel;
        if (0 == _nLevel)
            // nothing more to do, this means "nothing known about any children"
            return;

        // forward the level number to any child subtrees we have
        OPropagateLevels aDeeperInto(_nLevel);
        aDeeperInto.applyToChildren(*this);
    }

// --------------------------- Subtree implementation ---------------------------
    INode* Subtree::clone() const {return new Subtree(*this);}

    INode* Subtree::doGetChild(OUString const& aName) const
    {
        SearchNode searchObj(aName);

#ifdef DEBUG
        for (ChildList::iterator it2 = m_aChildren.GetSet().begin();
            it2 != m_aChildren.GetSet().end();
            ++it2)
        {
            INode* pINode = *it2;
            OUString aName2 = pINode->getName();
            volatile int dummy = 0;
        }
#endif

        ChildList::iterator it = m_aChildren.GetSet().find(&searchObj);
        if (it == m_aChildren.GetSet().end())
            return NULL;
        else
            return *it;
    }

    INode* Subtree::addChild(std::auto_ptr<INode> aNode)    // takes ownership
    {
        OUString aName = aNode->getName();
        std::pair<ChildList::iterator, bool> aInserted =
            m_aChildren.GetSet().insert(aNode.get());
        if (aInserted.second)
            aNode.release();
        return *aInserted.first;
    }

    ::std::auto_ptr<INode> Subtree::removeChild(OUString const& aName)
    {
        SearchNode searchObj(aName);
        ChildList::const_iterator it = m_aChildren.GetSet().find(&searchObj);

        ::std::auto_ptr<INode> aReturn;
        if (m_aChildren.GetSet().end() != it)
        {
            aReturn = ::std::auto_ptr<INode>(*it);
            m_aChildren.GetSet().erase(it);
        }
        return aReturn;
    }

    //==========================================================================
    //= OBuildChangeTree - historic
    //==========================================================================
    /** generates a change tree by comparing two trees
    */
/*  struct OBuildChangeTree : public NodeModification
    {
    protected:
        SubtreeChange&  m_rChangeList;
        INode*          m_pCacheNode;

    public:
        OBuildChangeTree(SubtreeChange& rList, INode* pNode)
            :m_rChangeList(rList)
            ,m_pCacheNode(pNode)
        {
        }

        virtual void handle(ValueNode& _nNode)
        {
            OUString aNodeName = _nNode.getName();
            ISubtree* pTree = m_pCacheNode->asISubtree();
            OSL_ENSHURE(pTree, "OBuildChangeTree::handle : node must be a inner node!");
            if (pTree)
            {
                INode* pChild = pTree->getChild(aNodeName);
                ValueNode* pValueNode = pChild ? pChild->asValueNode() : NULL;
                OSL_ENSHURE(pValueNode, "OBuildChangeTree::handle : node must be a value node!");

                // if the values differ add a new change
                if (pValueNode && _nNode.getValue() != pValueNode->getValue())
                {
                    ValueChange* pChange = new ValueChange(_nNode.getValue(), *pValueNode);
                    m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
                }
            }
        }

        virtual void handle(ISubtree& _rSubtree)
        {
            OUString aNodeName = _rSubtree.getName();
            ISubtree* pTree = m_pCacheNode->asISubtree();
            OSL_ENSHURE(pTree, "OBuildChangeTree::handle : node must be a inner node!");
            if (pTree)
            {
                INode* pChild = pTree->getChild(aNodeName);
                // node not in cache, so ignore it
                // later, when we get additions and removements within on transaction, then we have to care about
                if (pChild)
                {
                    ISubtree* pSubTree = pChild->asISubtree();
                    OSL_ENSHURE(pSubTree, "OBuildChangeTree::handle : node must be a inner node!");
                    // generate a new change

                    SubtreeChange* pChange = new SubtreeChange(_rSubtree);
                    OBuildChangeTree aNextLevel(*pChange, pSubTree);
                    aNextLevel.applyToChildren(_rSubtree);

                    // now count if there are any changes
                    OChangeCounter aCounter;
                    pChange->dispatch(aCounter);

                    if (aCounter.nCount != 0)
                        m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
                    else
                        delete pChange;
                }
            }
        }
    };

*/
    void Subtree::forEachChild(NodeAction& anAction) const {
        for(ChildList::const_iterator it = m_aChildren.GetSet().begin();
            it != m_aChildren.GetSet().end();
            ++it)
            (**it).dispatch(anAction);
    }

    void Subtree::forEachChild(NodeModification& anAction) {
        for(ChildList::iterator it = m_aChildren.GetSet().begin();
            it != m_aChildren.GetSet().end();
            ++it)
            (**it).dispatch(anAction);
    }

// -------------------------- ValueNode implementation --------------------------
    void ValueNode::check_init()    // may throw in the future
    {
        if (m_aValue.hasValue())
        {
            OSL_ASSERT(m_aType != ::getVoidCppuType());
            OSL_ASSERT(m_aType == m_aValue.getValueType());
        }
        else OSL_ASSERT(getVoidCppuType() == m_aValue.getValueType());

        if (m_aDefaultValue.hasValue())
        {
            OSL_ASSERT(m_aType != ::getVoidCppuType());
            OSL_ASSERT(m_aType == m_aDefaultValue.getValueType());
        }
        else OSL_ASSERT(getVoidCppuType() == m_aDefaultValue.getValueType());
    }

    void ValueNode::init()
    {
        OSL_ASSERT(m_aType == ::getVoidCppuType());

        if (m_aDefaultValue.hasValue())
        {
            m_aType = m_aDefaultValue.getValueType();
            OSL_ASSERT(m_aType != ::getVoidCppuType());
        }
        else if (m_aValue.hasValue())
        {
            m_aType = m_aValue.getValueType();
            OSL_ASSERT(m_aType != ::getVoidCppuType());
        }
    }


    void ValueNode::setValue(Any aValue)
    {
        m_aValue = aValue;
        // flip the type if necessary
        if  (   (m_aType.getTypeClass() == TypeClass_ANY)
            &&  (aValue.getValueType().getTypeClass() != TypeClass_ANY)
            &&  (aValue.getValueType().getTypeClass() != TypeClass_VOID)
            )
            m_aType = aValue.getValueType();
    }

    void ValueNode::changeDefault(Any aValue)
    {
        m_aDefaultValue = aValue;
        // flip the type if necessary
        if  (   (m_aType.getTypeClass() == TypeClass_ANY)
            &&  (aValue.getValueType().getTypeClass() != TypeClass_ANY)
            &&  (aValue.getValueType().getTypeClass() != TypeClass_VOID)
            )
            m_aType = aValue.getValueType();
    }

    void ValueNode::setDefault()
    {
        // PRE: ????
        // POST: isDefault() == true
        m_aValue = Any();
    }

    INode* ValueNode::clone() const
    {
        return new ValueNode(*this);
    }

    ValueNode* ValueNode::asValueNode() {return this;}
    ValueNode const* ValueNode::asValueNode() const {return this;}

} // namespace configmgr


