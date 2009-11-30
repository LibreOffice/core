/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cmtree.cxx,v $
 * $Revision: 1.41 $
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

#include "subtree.hxx"
#include "change.hxx"
#include "treechangelist.hxx"

//#include "treeactions.hxx"
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

#ifndef INCLUDED_DEQUE
#include <deque>
#define INCLUDED_DEQUE
#endif
#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
#ifndef INCLUDED_EXCEPTION
#include <exception>
#define INCLUDED_EXCEPTION
#endif
#ifndef INCLUDED_SET
#include <set>
#define INCLUDED_SET
#endif
#include <algorithm>

namespace configmgr
{

// ------------------------ ChildListSet implementations ------------------------
    ChildListSet::ChildListSet(ChildListSet const& aSet, treeop::DeepChildCopy)
        : m_aChildList(aSet.m_aChildList.size())
    {
        for (size_t i = 0; i < aSet.m_aChildList.size(); i++)
        {
            m_aChildList[i] = aSet.m_aChildList[i]->clone().release();
        }
    }
    ChildListSet::~ChildListSet()
    {
        for (size_t i = 0; i < m_aChildList.size(); i++)
            delete m_aChildList[i];
    }

    struct ltNode
    {
        ltNode() {}

        bool operator()(const configmgr::INode* n1, const configmgr::INode* n2) const
        {
            return n1->getName().compareTo(n2->getName()) < 0;
        }
    };

    std::vector< INode* >::iterator ChildListSet::find(INode *pNode) const
    {
        std::vector< INode* > &rList = const_cast<std::vector< INode* > &>(m_aChildList);
        std::pair<std::vector< INode* >::iterator, std::vector< INode* >::iterator> aRange;
        ltNode aCompare;
        aRange = std::equal_range(rList.begin(), rList.end(), pNode, aCompare);
        if (aRange.second - aRange.first == 0)
            return rList.end();
        else
            return aRange.first;
    }

    // Keep the list sorted ...
    std::pair<std::vector< INode* >::iterator, bool> ChildListSet::insert(INode *pNode)
    {
        // Inserted records are (mostly) already in order
        if (m_aChildList.size() > 0)
        {
            sal_Int32 nCmp = pNode->getName().compareTo(
                m_aChildList.back()->getName());
            if (nCmp == 0)
            {
                return std::pair<std::vector< INode* >::iterator, bool>(m_aChildList.end(), false);
            }
            else if (nCmp < 0)
            {
                std::vector< INode* >::iterator aIns;
                ltNode aCompare;
                aIns = std::lower_bound(m_aChildList.begin(), m_aChildList.end(), pNode, aCompare);
                if (aIns != m_aChildList.end() && pNode->getName().compareTo((*aIns)->getName()) == 0)
                    return std::pair<std::vector< INode* >::iterator, bool>(m_aChildList.end(), false);
                return std::pair<std::vector< INode* >::iterator, bool>(m_aChildList.insert(aIns, pNode), true);
            }
        }
        // simple append - the common case.
        return std::pair<std::vector< INode* >::iterator, bool>(m_aChildList.insert(m_aChildList.end(), pNode), true);
    }

    INode *ChildListSet::erase(INode *pNode)
    {
        std::vector< INode* >::iterator aIter = find(pNode);

        if (aIter != m_aChildList.end())
        {
            INode *pCopy = *aIter;
            m_aChildList.erase(aIter);
            return pCopy;
        }
        else
            return NULL;
    }

// ---------------------------- Node implementation ----------------------------

    INode::INode(rtl::OUString const& aName, node::Attributes _aAttr)
          :m_aName(aName)
          ,m_aAttributes(_aAttr){}
    // CopyCTor will be create automatically

    INode::~INode() {}

    ISubtree*         INode::asISubtree(){return NULL;}
    ISubtree const*   INode::asISubtree() const {return NULL;}
    ValueNode*       INode::asValueNode() {return NULL;}
    ValueNode const* INode::asValueNode() const {return NULL;}

    void INode::modifyState(node::State _eNewState)
    {
        m_aAttributes.setState(_eNewState);
    }

    void INode::modifyAccess(node::Access _aAccessLevel)
    {
        OSL_ENSURE( node::accessWritable <= _aAccessLevel && _aAccessLevel <= node::accessReadonly,"Invalid access level for Node");

        m_aAttributes.setAccess(_aAccessLevel);
    }

    void INode::markMandatory()
    {
        m_aAttributes.markMandatory();
    }

    void INode::markRemovable()
    {
        m_aAttributes.markRemovable();
    }

    void INode::promoteAccessToDefault()
      {
        if (m_aAttributes.isFinalized())
            m_aAttributes.setAccess(node::accessReadonly);

        if ( m_aAttributes.isMandatory())
            m_aAttributes.setRemovability(false,false);
    }

// ------------------------- SearchNode implementation -------------------------
    SearchNode::SearchNode(rtl::OUString const& aName)
        :INode(aName, node::Attributes()){}

    std::auto_ptr<INode> SearchNode::clone() const {return std::auto_ptr<INode>(new SearchNode(*this));}

    SearchNode::~SearchNode(){}

    //==========================================================================
    //= OPropagateLevels
    //==========================================================================
    /** fills a subtree with the correct level informations
    */
    struct OPropagateLevels : public NodeModification
    {
    public:
        OPropagateLevels(sal_Int16 _nParentLevel, sal_Int16 _nParentDefaultLevel)
        : m_nLevel          ( childLevel(_nParentLevel) )
        , m_nDefaultLevel   ( childLevel(_nParentDefaultLevel) )
        {
        }
        virtual void handle(ValueNode&) { /* not interested in value nodes */ }
        virtual void handle(ISubtree& _rSubtree)
        {
            _rSubtree.setLevels(m_nLevel, m_nDefaultLevel);
        }

        static sal_Int16 childLevel(sal_Int16 _nLevel)
        {
            OSL_ASSERT(0 > treeop::ALL_LEVELS);
            return (_nLevel > 0) ? _nLevel-1 : _nLevel;
        }
    protected:
        sal_Int16   m_nLevel;
        sal_Int16   m_nDefaultLevel;
    };


// -------------------------- ISubtree implementation --------------------------
    ISubtree* ISubtree::asISubtree() {return this;}
    ISubtree const* ISubtree::asISubtree() const {return this;}

    //--------------------------------------------------------------------------
    static inline bool adjustLevel(sal_Int16& _rLevel, sal_Int16 _nNewLevel)
    {
        if (_rLevel == treeop::ALL_LEVELS)   return false;
        if (_nNewLevel <= _rLevel &&
            _nNewLevel != treeop::ALL_LEVELS) return false;

        _rLevel = _nNewLevel;
        return true;
    }

    //--------------------------------------------------------------------------
    void ISubtree::setLevels(sal_Int16 _nLevel, sal_Int16 _nDefaultLevels)
    {
        bool bActive = false;

        if (_nLevel && adjustLevel(m_nLevel, _nLevel))
            bActive = true;

        if (_nDefaultLevels && adjustLevel(m_nDefaultLevels, _nDefaultLevels))
            bActive = true;

        // forward the level numbers to any child subtrees we have
        if (bActive)
        {
            OPropagateLevels aPropagate(_nLevel,_nDefaultLevels);
            aPropagate.applyToChildren(*this);
        }
    }

// --------------------------- Subtree implementation ---------------------------
    std::auto_ptr<INode> Subtree::clone() const
    {
        return std::auto_ptr<INode>(new Subtree(*this, treeop::DeepChildCopy()));
    }

    INode* Subtree::doGetChild(rtl::OUString const& aName) const
    {
        SearchNode searchObj(aName);

        std::vector< INode* >::iterator aIter = m_aChildren.find(&searchObj);
        return aIter != m_aChildren.end() ? *aIter : NULL;
    }

    INode* Subtree::addChild(std::auto_ptr<INode> aNode)    // takes ownership
    {
        rtl::OUString aName = aNode->getName();
        std::pair<std::vector< INode* >::iterator, bool> aInserted =
            m_aChildren.insert(aNode.get());
        if (aInserted.second)
            aNode.release();
        return *aInserted.first;
    }

    ::std::auto_ptr<INode> Subtree::removeChild(rtl::OUString const& aName)
    {
        SearchNode searchObj(aName);
        return ::std::auto_ptr<INode>(m_aChildren.erase(&searchObj));
    }
//  // -------------------------- ValueNode implementation --------------------------

    void Subtree::forEachChild(NodeAction& anAction) const
    {
        for(std::vector< INode* >::const_iterator it = m_aChildren.begin();
            it != m_aChildren.end();
            ++it)
            (**it).dispatch(anAction);
    }

    void Subtree::forEachChild(NodeModification& anAction)
    {
        std::vector< INode* >::iterator it = m_aChildren.begin();
        while( it != m_aChildren.end() )
        {
            // modification-safe iteration
            (**it++).dispatch(anAction);
        }
      }

//  // -------------------------- ValueNode implementation --------------------------
    bool ValueNode::setValueType(uno::Type const& _aType)
    {
        if (_aType == this->getValueType()) return true;

        if (!this->isNull()) return false;

        uno::TypeClass eTC = this->getValueType().getTypeClass();
        if (eTC != uno::TypeClass_VOID && eTC != uno::TypeClass_ANY)
            return false;

        m_aValuePair = AnyPair(_aType);

        OSL_ASSERT(_aType == this->getValueType());

        return true;
    }
    bool ValueNode::setValue(com::sun::star::uno::Any const& _aValue)
    {
        sal_Bool bRet = m_aValuePair.setFirst(_aValue);
        if (bRet) this->markAsDefault(false);
        return !! bRet;
    }

    bool ValueNode::changeDefault(com::sun::star::uno::Any const& _aValue)
    {
        return !! m_aValuePair.setSecond(_aValue);
    }

    void ValueNode::setDefault()
    {
        OSL_PRECOND( hasUsableDefault(), "No default value to set for value node");
        m_aValuePair.clear( selectValue() );
        this->markAsDefault();
        OSL_POSTCOND( isDefault(), "Could not set value node to default");
    }

    void ValueNode::promoteToDefault()
    {
        if (!isDefault())
        {
            if (m_aValuePair.hasFirst())
            {
                OSL_VERIFY( m_aValuePair.setSecond(m_aValuePair.getFirst()) );
                m_aValuePair.clear( selectValue() );
            }
            else
            {
                m_aValuePair.clear( selectDeflt() );
                OSL_ASSERT( m_aValuePair.isNull() );
            }

            this->markAsDefault();

            OSL_ENSURE( !m_aValuePair.hasFirst(), "Leaving orphaned value in after promoting to default");
        }
        else
            OSL_ENSURE( !m_aValuePair.hasFirst(), "Orphaned value in default node won't be promoted");

        OSL_POSTCOND( isDefault(), "Could not promote value node to default");
    }

    std::auto_ptr<INode> ValueNode::clone() const
    {
        return std::auto_ptr<INode>(new ValueNode(*this));
    }

    ValueNode* ValueNode::asValueNode() {return this;}
    ValueNode const* ValueNode::asValueNode() const {return this;}

} // namespace configmgr


