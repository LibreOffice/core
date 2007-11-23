/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmtree.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:31:21 $
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

#include "subtree.hxx"
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif

#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif

//#include "treeactions.hxx"

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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

using namespace std;
using namespace rtl;
using namespace com::sun::star::uno;

namespace configmgr
{

// ------------------------ ChildListSet implementations ------------------------
    ChildListSet::ChildListSet(ChildListSet const& aSet, treeop::DeepChildCopy)
        : m_aChildList(0)
    {
        for (size_t i = 0; i < aSet.m_aChildList.size(); i++)
        {
            m_aChildList.insert(m_aChildList.end(),
                                aSet.m_aChildList[i]->clone().release());
        }
    }
    ChildListSet::~ChildListSet()
    {
        for (size_t i = 0; i < m_aChildList.size(); i++)
            delete m_aChildList[i];
    }

    struct ltNode
    {
        bool operator()(const configmgr::INode* n1, const configmgr::INode* n2) const
        {
            return n1->getName().compareTo(n2->getName()) < 0;
        }
    };

    ChildList::iterator ChildListSet::find(INode *pNode) const
    {
        ChildList &rList = const_cast<ChildList &>(m_aChildList);
        std::pair<ChildList::iterator, ChildList::iterator> aRange;
        ltNode aCompare;
        aRange = equal_range(rList.begin(), rList.end(), pNode, aCompare);
        if (aRange.second - aRange.first == 0)
            return rList.end();
        else
            return aRange.first;
    }

    // Keep the list sorted ...
    std::pair<ChildList::iterator, bool> ChildListSet::insert(INode *pNode)
    {
        // Inserted records are (mostly) already in order
        if (m_aChildList.size() > 0)
        {
            sal_Int32 nCmp = pNode->getName().compareTo(
                m_aChildList.back()->getName());
            if (nCmp == 0)
            {
                return std::pair<ChildList::iterator, bool>(m_aChildList.end(), false);
            }
            else if (nCmp < 0)
            {
                ChildList::iterator aIns;
                ltNode aCompare;
                aIns = lower_bound(m_aChildList.begin(), m_aChildList.end(), pNode, aCompare);
                if (aIns != m_aChildList.end() && pNode->getName().compareTo((*aIns)->getName()) == 0)
                    return std::pair<ChildList::iterator, bool>(m_aChildList.end(), false);
                return std::pair<ChildList::iterator, bool>(m_aChildList.insert(aIns, pNode), true);
            }
        }
        // simple append - the common case.
        return std::pair<ChildList::iterator, bool>(m_aChildList.insert(m_aChildList.end(), pNode), true);
    }

    INode *ChildListSet::erase(INode *pNode)
    {
        ChildList::iterator aIter = find(pNode);

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

    INode::INode(node::Attributes _aAttr):m_aAttributes(_aAttr){}
    INode::INode(OUString const& aName, node::Attributes _aAttr)
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

    void INode::forceReadonlyToFinalized()
      {
        if (m_aAttributes.isReadonly())
        {
            m_aAttributes.setAccess(node::accessFinal);
        }
    }

// ------------------------- SearchNode implementation -------------------------
    SearchNode::SearchNode():INode(node::Attributes()){}
    SearchNode::SearchNode(OUString const& aName)
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
        typedef sal_Int16 Level;
        OPropagateLevels(Level _nParentLevel, Level _nParentDefaultLevel)
        : m_nLevel          ( childLevel(_nParentLevel) )
        , m_nDefaultLevel   ( childLevel(_nParentDefaultLevel) )
        {
        }
        virtual void handle(ValueNode&) { /* not interested in value nodes */ }
        virtual void handle(ISubtree& _rSubtree)
        {
            _rSubtree.setLevels(m_nLevel, m_nDefaultLevel);
        }

        static Level childLevel(Level _nLevel)
        {
            OSL_ASSERT(0 > treeop::ALL_LEVELS);
            return (_nLevel > 0) ? _nLevel-1 : _nLevel;
        }
    protected:
        Level   m_nLevel;
        Level   m_nDefaultLevel;
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

    INode* Subtree::doGetChild(OUString const& aName) const
    {
        SearchNode searchObj(aName);

        ChildList::iterator aIter = m_aChildren.find(&searchObj);
        return aIter != m_aChildren.end() ? *aIter : NULL;
    }

    INode* Subtree::addChild(std::auto_ptr<INode> aNode)    // takes ownership
    {
        OUString aName = aNode->getName();
        std::pair<ChildList::iterator, bool> aInserted =
            m_aChildren.insert(aNode.get());
        if (aInserted.second)
            aNode.release();
        return *aInserted.first;
    }

    ::std::auto_ptr<INode> Subtree::removeChild(OUString const& aName)
    {
        SearchNode searchObj(aName);
        return ::std::auto_ptr<INode>(m_aChildren.erase(&searchObj));
    }
//  // -------------------------- ValueNode implementation --------------------------

    void Subtree::forEachChild(NodeAction& anAction) const
    {
        for(ChildList::const_iterator it = m_aChildren.begin();
            it != m_aChildren.end();
            ++it)
            (**it).dispatch(anAction);
    }

    void Subtree::forEachChild(NodeModification& anAction)
    {
        ChildList::iterator it = m_aChildren.begin();
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
    bool ValueNode::setValue(Any const& _aValue)
    {
        sal_Bool bRet = m_aValuePair.setFirst(_aValue);
        if (bRet) this->markAsDefault(false);
        return !! bRet;
    }

    bool ValueNode::changeDefault(Any const& _aValue)
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


