/*************************************************************************
 *
 *  $RCSfile: cmtree.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: lla $ $Date: 2000-11-13 13:14:49 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#include <stl/deque>
#include <stl/vector>
#include <iostream>
#include <exception>
#include <sal/types.h>
#include <stl/set>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

#include "cmtree.hxx"
#include "confname.hxx"                          // Iterator for PathName scans
#include "cmtreemodel.hxx"

#include <com/sun/star/uno/Any.hxx>

#ifndef _CONFIGMGR_TREEACCESS_HXX_
#include "treeaccess.hxx"
#endif
#ifndef CONFIGMGR_API_CHANGESSET_HXX_
#include "confchangesset.hxx"
#endif

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

    INode::INode(){}
    INode::INode(OUString const& aName)
            :m_aName(aName){}
    // CopyCTor will be create automatically

    INode::~INode() {}

    // Node* Node::clone() {}

    OUString INode::getName() const {return m_aName;}
    NodeAttributes INode::getAttributes() const {return m_aNodeAttribute;}

    ISubtree*         INode::asISubtree(){return NULL;}
    ISubtree const*   INode::asISubtree() const {return NULL;}
    ValueNode*       INode::asValueNode() {return NULL;}
    ValueNode const* INode::asValueNode() const {return NULL;}


// ------------------------- SearchNode implementation -------------------------
    SearchNode::SearchNode(){}
    SearchNode::SearchNode(OUString const& aName)
            :INode(aName){}

    INode* SearchNode::clone() const {return new SearchNode(*this);}

    OUString SearchNode::getName() const {return getName();}
    SearchNode::~SearchNode(){}


// -------------------------- ISubtree implementation --------------------------
    ISubtree* ISubtree::asISubtree() {return this;}
    ISubtree const* ISubtree::asISubtree() const {return this;}

// --------------------------- Subtree implementation ---------------------------

    Subtree::Subtree(OUString const& aName)
        :ISubtree(aName)
        ,m_nLevel(0)
    {
    }
    Subtree::Subtree()
        :m_nLevel(0)
    {
    }
    Subtree::~Subtree() {}

// #pragma message("__FILE__(__LINE__) Subtree::clone() has empty implementation")
    INode* Subtree::clone() const {return new Subtree(*this);}

// #define MAX_NODE_NAME_LENGTH 256

// RECURSIVE:     Node* Subtree::createChild(OUString const& aPath)
// RECURSIVE:     {
// RECURSIVE:         //?   configmgr::Node searchObj(aName);
// RECURSIVE:         //?   ChildList::iterator it = m_aNode->GetSet().find(&searchObj);
// RECURSIVE:         //?   if (it == m_aNode->GetSet().end())
// RECURSIVE:         //?       return NULL;
// RECURSIVE:         //?   else
// RECURSIVE:         //?       return *it;
// RECURSIVE:         sal_Unicode aName[MAX_NODE_NAME_LENGTH];
// RECURSIVE:         int nStartPos = 0;
// RECURSIVE:         int nPos = 0;
// RECURSIVE:
// RECURSIVE:         if (aPath[0] == '/')
// RECURSIVE:         {
// RECURSIVE:             nStartPos++;
// RECURSIVE:         }
// RECURSIVE:         if (aPath[nStartPos] == '\0')
// RECURSIVE:         {
// RECURSIVE:             return this;
// RECURSIVE:         }
// RECURSIVE:
// RECURSIVE:         int nEndPos = nStartPos;
// RECURSIVE:         while ((aPath[nEndPos] != '\0') && (aPath[nEndPos] != '/')) {
// RECURSIVE:             aName[nPos++] = aPath[nEndPos++];
// RECURSIVE:         }
// RECURSIVE:         aName[nPos] = '\0';
// RECURSIVE:
// RECURSIVE: #ifdef DEBUG
// RECURSIVE:         // list(cout << " searching for [" << aName << "] in\n");
// RECURSIVE: #endif
// RECURSIVE:
// RECURSIVE:         // search Node
// RECURSIVE:         SearchNode searchObj(aName);
// RECURSIVE:         ChildList::iterator it = m_aChildren->GetSet().find(&searchObj);
// RECURSIVE:         if (it == m_aChildren->GetSet().end())
// RECURSIVE:         {
// RECURSIVE:             // create new Child
// RECURSIVE:             auto_ptr<Node> pSubtree( new Subtree(aName));
// RECURSIVE:             Node *pN = addChild(pSubtree);
// RECURSIVE:             Subtree *pS = (Subtree*)pN;
// RECURSIVE:             return pS->createChild(aPath + nEndPos);
// RECURSIVE:         }
// RECURSIVE:         else {
// RECURSIVE:             // call recursive the inner child
// RECURSIVE:             Node *pN = *it;
// RECURSIVE:             Subtree *pP = (Subtree*)pN;
// RECURSIVE:             return pP->createChild(aPath + nEndPos);
// RECURSIVE:         }
// RECURSIVE:     }

    INode* Subtree::createChild(OUString const& aName)
    {
        // POST: create Subtree if not found

        // search Node
        SearchNode searchObj(aName);
        ChildList::iterator it = m_aChildren.GetSet().find(&searchObj);
        if (it == m_aChildren.GetSet().end())
        {
            // create new Child
            auto_ptr<INode> pSubtree( new Subtree(aName));
            return addChild(pSubtree);
        }
        return *it;
    }

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

    //==========================================================================
    //= OCompleteTree
    //==========================================================================
    /** completes the cache with missing subelements
    */
    struct OCompleteTree : public NodeModification
    {
    protected:
        ISubtree* m_pCacheSubtree;
        sal_Int32 m_nChildLevel;

    public:
        OCompleteTree(ISubtree* pSubtree, sal_Int32 _nParentLevel)
            :m_pCacheSubtree(pSubtree)
        {
            m_nChildLevel = (ITreeProvider::ALL_LEVELS == _nParentLevel) ? ITreeProvider::ALL_LEVELS : _nParentLevel - 1;
        }
        virtual void handle(ValueNode&) { /* not interested in value nodes */ }
        virtual void handle(ISubtree& _rSubtree)
        {
            OUString aNodeName = _rSubtree.getName();
            INode*   pChild    = m_pCacheSubtree->getChild(aNodeName);
            // now we have different possibilites
            // a.) the node does not exist than clone the subtree and add it to the cache tree
            if (!pChild)
            {
                pChild = _rSubtree.clone();
                ISubtree* pSubTree = pChild->asISubtree();
                m_pCacheSubtree->addChild(::std::auto_ptr<INode>(pSubTree));
            }
            else
            {
                ISubtree* pSubTree = pChild->asISubtree();
                OSL_ENSHURE(pSubTree, "OCompleteTree::handle : node must be a inner node!");

                // b.) the node does exist with level all or greater level -> nothing to do
                // c.) the node does exist but with smaller level
                if (pSubTree && ITreeProvider::ALL_LEVELS != pSubTree->getLevel() &&
                    (ITreeProvider::ALL_LEVELS == m_nChildLevel ||
                     m_nChildLevel > pSubTree->getLevel()))
                {
                    OCompleteTree aNextLevel(pSubTree, m_nChildLevel);
                    aNextLevel.applyToChildren(_rSubtree);
                }
            }
        }
    };

    //==========================================================================
    //= OBuildChangeTree
    //==========================================================================
    /** generates a change tree by comparing two trees
    */
    struct OBuildChangeTree : public NodeModification
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

                    SubtreeChange* pChange = new SubtreeChange(aNodeName);
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

    // --------------------------------- updateTree ---------------------------------
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
            OSL_ENSHURE(pValue, aStr.getStr());
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
                OSL_ENSHURE(aOldNode.get(), "TreeUpdate:AddNode: can't recover node being replaced");
                if (aOldNode.get() == NULL)
                    aLog.push_back(OString("TreeUpdate: can't recover node being replaced (for AddNode)"));
#endif

                aAddNode.takeReplacedNode( aOldNode );
            }

            m_pCurrentSubtree->addChild(aAddNode.releaseAddedNode());
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
            aRemoveNode.takeRemovedNode( aOldNode );

#ifdef DBUG
            if (!bOk)
            {
                ::rtl::OString aStr("TreeUpdate: Can't remove child with name:=");
                aStr += rtl::OUStringToOString(aRemoveNode.getNodeName(),RTL_TEXTENCODING_ASCII_US);
                OSL_ENSHURE(bOk, aStr.getStr());
                aLog.push_back(aStr);
            }
#endif
        }
    }


    void TreeUpdate::handle(SubtreeChange& aSubtree)
    {
        // handle traversion
        ISubtree *pOldSubtree = m_pCurrentSubtree;
        OSL_ENSHURE(m_pCurrentSubtree->getChild(aSubtree.getNodeName()), "TreeUpdate::handle : invalid subtree change ... this will crash !");
        m_pCurrentSubtree = m_pCurrentSubtree->getChild(aSubtree.getNodeName())->asISubtree();

#if DBUG
        ::rtl::OString aStr("TreeUpdate: there is no Subtree for name:=");
        aStr += rtl::OUStringToOString(aSubtree.getNodeName(),RTL_TEXTENCODING_ASCII_US);
        OSL_ENSHURE(m_pCurrentSubtree, aStr.getStr());
        if (!m_pCurrentSubtree)
            aLog.push_back(aStr);
#endif

        aSubtree.forEachChange(*this);
        m_pCurrentSubtree = pOldSubtree;
    }


    //--------------------------------------------------------------------------
    void Subtree::setLevel(sal_Int16 _nLevel)
    {
        m_nLevel = _nLevel;
        if (0 == _nLevel)
            // nothing more to do, this means "nothing known about any children"
            return;

        // forward the level number to any child subtrees we have
        OPropagateLevels aDeeperInto(_nLevel);
        aDeeperInto.applyToChildren(*this);
    }

    //--------------------------------------------------------------------------
    sal_Int16 Subtree::getLevel() const {return m_nLevel;}

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

    bool ValueNode::isDefault() const
    {
        // POST: true, if only m_aDefaultValue is set.
        return !m_aValue.hasValue() && hasDefault();
    }

    bool ValueNode::hasDefault() const
    {
        // POST: true, if only m_aDefaultValue is set.
        return getAttributes().optional || m_aDefaultValue.hasValue();
    }

    bool ValueNode::isNull() const
    {
        // POST: true, if neither Any is set.
        return !m_aValue.hasValue() && !m_aDefaultValue.hasValue();
    }


    Type ValueNode::getValueType() const
    {
        // POST: return Type of Any
        return m_aType;
    }

    Any ValueNode::getValue() const
    {
        // POST: getValue, if not set, get DefaultValue
        if (isDefault())
            return m_aDefaultValue;
        return m_aValue;
    }

    Any ValueNode::getDefault() const
    {
        return m_aDefaultValue;
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

// ---------------------------- Tree implementation ----------------------------

    Tree::Tree()
            : m_pRoot(NULL), m_pLock(NULL)
    {
        m_pRoot = new Subtree();
        m_pLock = new OTreeAccessor;
    }

    Tree::~Tree()
    {
        delete m_pLock;
    }

    void Tree::acquireReadAccess() const
    {
        m_pLock->acquireReadAccess();
    }

    void Tree::releaseReadAccess() const
    {
        m_pLock->releaseReadAccess();
    }

    void Tree::acquireWriteAccess()
    {
        m_pLock->acquireWriteAccess();
    }

    void Tree::releaseWriteAccess()
    {
        m_pLock->releaseWriteAccess();
    }

// -----------------------------------------------------------------------------
    ISubtree* Tree::requestSubtree( OUString const& aComponentName, sal_Int16 nLevel ) throw (container::NoSuchElementException)
    {
        // OLD:
        // INode* pResult = m_pRoot->getChild(aComponentName);
        // return pResult->asISubtree();

        // looking for the requested subtree: results are
        // a.) tree not found, tree isn't already cached
        // b.) tree found, but not complete
        // c.) tree found and complete

        sal_Bool bCompleteForRequest = sal_False;

        // Build SearchName
        OSL_ENSHURE(m_pRoot, "Tree::requestSubtree : m_pRoot MUST NOT BE ZERO");
            // hey, don't cry that loud ....

        ISubtree* pSubtree = m_pRoot;
        ConfigurationName aConfigName(aComponentName,ConfigurationName::Absolute() );
        for(ConfigurationName::Iterator it = aConfigName.begin();
            it != aConfigName.end();
            ++it)
        {
            if (pSubtree)
            {
                INode* pNode = pSubtree->getChild(*it);
                if (pNode)
                    pSubtree = pNode->asISubtree();
                else
                    pSubtree = 0;
            }
            else
                break;
        }

        // if the tree is not complete: ALL_LEVELS != pSubtree->getLevel()
        // or not fetched with all requested levels, we have to refetch
        bCompleteForRequest = pSubtree && (ALL_LEVELS == pSubtree->getLevel() ||
                                          (ALL_LEVELS != nLevel && nLevel <= pSubtree->getLevel()));

        if (!bCompleteForRequest)
            pSubtree = 0;

        return pSubtree;
    }

// -----------------------------------------------------------------------------
    const INode* Tree::getNode(const OUString& _rPath)
    {
        OSL_ENSHURE(m_pRoot, "Tree::getNode : invalid root !");

        ConfigurationName aPath(_rPath, ConfigurationName::Absolute() );

        INode* pNodeLoop = m_pRoot;
        for (   ConfigurationName::Iterator aSearch = aPath.begin();
                (aSearch != aPath.end()) && pNodeLoop;
                ++aSearch)
        {
            ISubtree* pNodeContainer = pNodeLoop->asISubtree();
            if (pNodeContainer)
                pNodeLoop = pNodeContainer->getChild(*aSearch);
            else
                return NULL;
        }

        return pNodeLoop;
    }

// -----------------------------------------------------------------------------
    ISubtree const* Tree::getSubtree( OUString const& aComponentName ) const
    {
        // Build SearchName
        OSL_ENSHURE(m_pRoot, "m_pRoot MUST NOT BE ZERO");

        ISubtree const* pSubtree = m_pRoot;
        ConfigurationName aConfigName(aComponentName,ConfigurationName::Absolute() );
        for(ConfigurationName::Iterator it = aConfigName.begin();
            it != aConfigName.end();
            ++it)
        {
            if (pSubtree)
            {
                const INode* pNode = pSubtree->getChild(*it);
                if (!pNode)
                    return NULL;

                pSubtree = pNode->asISubtree();
            }
            else
                break;
        }
        return pSubtree;
        /*
          sal_Unicode aName[MAX_NODE_NAME_LENGTH];
          int nStartPos = 0;
          int nPos = 0;

          if (aPath[0] == '/')
          {
          nStartPos++;
          }
          if (aPath[nStartPos] == '\0')
          {
          return this;
          }

          int nEndPos = nStartPos;
          while ((aPath[nEndPos] != '\0') && (aPath[nEndPos] != '/')) {
          aName[nPos++] = aPath[nEndPos++];
          }
          aName[nPos] = '\0';

          ISubtree const* pSubtree = m_pRoot->getChild(aName)->asISubtree();


        if (pSubtree)
        {
            // call recursive the inner child
            return pSubtree->getSubtree(aPath + nEndPos);
        }
        return NULL;
        */
    }

    ISubtree* Tree::addSubtree(const ConfigurationName& _rLocation, std::auto_ptr<ISubtree> _pSubtree, sal_Int16 nLevels)
    {
        OSL_ENSHURE(nLevels != 0, "Tree::addSubtree : invalid level count !");
        // there was a time where 0 meant "all levels", but we changed the according enum in ITReeProvider
        // so that it is -1 now. Since this time, 0 isn't valid as level depth anymore !

        // if we notice that a subtree already exists we have to verify
        // if the subtree is already complete populated

        // do we already have the subtree in cache, but not completely populated?
        // so find the subtree
        Subtree* pEntry = m_pRoot;
        for (ConfigurationName::Iterator i = _rLocation.begin(); i != _rLocation.end() && pEntry != NULL; i++)
            pEntry = static_cast<Subtree*>(pEntry->getChild(*i));

        // we already have a tree, so we fill the subtree with the neccessary fragments
        if (pEntry)
        {
            OSL_ENSHURE(pEntry->getLevel() != ITreeProvider::ALL_LEVELS, "Tree::addSubtree : node already complete in cache, why adding it again? !");
            if (pEntry->getLevel() != ITreeProvider::ALL_LEVELS)
            {
                // release the ownership and delete that unnecessary tree
                OCompleteTree aTreeCompletion(pEntry, nLevels);
                aTreeCompletion.applyToChildren(*_pSubtree.get());

                // now adjust the levels
                pEntry->setLevel(nLevels);
            }

            // release the ownership and delete that unnecessary tree
            _pSubtree.release();
            delete _pSubtree.get();
            return pEntry;
        }
        else
        {
            // insert the complete subtree because it is not part of the cache yet
            Subtree* pInsertInto = m_pRoot;
            ConfigurationName::Iterator i = _rLocation.begin();
            while (i != _rLocation.end())
            {
                // increment the iterator here, as we later may need the next position
                OUString aNodeName = *i++;
                Subtree* pEntry = static_cast<Subtree*>(m_pRoot->getChild(aNodeName));
                if (!pEntry)
                {
                    Subtree* pNewChild = NULL;
                    if (i != _rLocation.end())
                    {
                        // do we still have (at least) one level to go than we need a new temporary node
                        pNewChild = new Subtree(aNodeName);
                        pNewChild = static_cast<Subtree*>(pInsertInto->addChild(::std::auto_ptr<INode>(pNewChild)));
                        pNewChild->setLevel(0); // which means "we know nothing about any children"
                    }
                    else
                    {
                        // at this last level, we don't need an intermediate node, instead we have to insert _pSubtree here
                        break;
                    }
                    pEntry = pNewChild;
                }
                // one level down
                pInsertInto = pEntry;
            }

            Subtree* pNewSubtree = static_cast<Subtree*>(pInsertInto->addChild(::std::auto_ptr<INode>(_pSubtree.release())));
            pNewSubtree->setLevel(nLevels);
            return pNewSubtree;
        }
    }


    std::auto_ptr<TreeChangeList> Tree::mergeSubTree(const ConfigurationName& _rLocation,  std::auto_ptr<ISubtree> pSubtree)
    {
        // first look for the subtree
        Subtree* pEntry = m_pRoot;
        for (ConfigurationName::Iterator i = _rLocation.begin(); i != _rLocation.end() && pEntry != NULL; i++)
            pEntry = static_cast<Subtree*>(pEntry->getChild(*i));

#ifdef DEBUG
        ::rtl::OString aStr("Tree: there is no Subtree for name:=");
        aStr += rtl::OUStringToOString(_rLocation.fullName(), RTL_TEXTENCODING_ASCII_US);
        OSL_ENSHURE(pEntry, aStr.getStr());
#endif
        if (pEntry)
        {
            std::auto_ptr<TreeChangeList> pChangeList(new TreeChangeList(_rLocation.getParentName().fullName(), _rLocation.localName()));
            // now fill the change list
            OBuildChangeTree aTreeChange(pChangeList->root, pEntry);
            aTreeChange.applyToChildren(*pSubtree.get());

            // now count if there are any changes
            OChangeCounter aCounter;
            pChangeList->root.dispatch(aCounter);

            if (aCounter.nCount == 0)
                pChangeList.reset();
            else
            {
                TreeUpdate aTreeUpdate(pEntry);
                pChangeList->root.forEachChange(aTreeUpdate);
            }
            return pChangeList;

        }
        else
            return std::auto_ptr<TreeChangeList>(0);
    }


    void Tree::updateTree( TreeChangeList& aTree) throw (lang::WrappedTargetException, uno::RuntimeException)
    {
        ConfigurationName aSubtreeName(aTree.pathToRoot, aTree.root.getNodeName());
        ISubtree *pSubtree = NULL;
        try
        {
            // request the subtree, atleast one level must exist!
            pSubtree = requestSubtree(aSubtreeName.fullName(), 1);
        }
        catch(container::NoSuchElementException&e)
        {
            ::rtl::OUString aStr(RTL_CONSTASCII_USTRINGPARAM("Tree: there is no Subtree for name:="));

            aStr += aSubtreeName.fullName();

            lang::WrappedTargetException aError;
            aError.Message = aStr;
            aError.TargetException <<= e;
            throw aError;
        }

        if (pSubtree)
        {
            TreeUpdate aTreeUpdate(pSubtree);
            aTree.root.forEachChange(aTreeUpdate);
        }
        else
        {
            ::rtl::OUString aStr(RTL_CONSTASCII_USTRINGPARAM("Tree: there is no Subtree for name:="));

            aStr += aSubtreeName.fullName();

            throw uno::RuntimeException(aStr,0);
        }
        // Better:
        // ISubtree *pCloneTree = m_pRoot->clone();
        // ISubtree *pSubtree = pCloneTree->requestSubtree(aTree.pathToRoot, ITreeProvider::ALL_LEVELS);
        // TreeUpdate aTreeUpdate(pSubtree);
        // aTreeUpdate.handle(aTree.root);
        // if (aTreeUpdate.isOk())
        // {
        //    delete m_pRoot;
        //    m_pRoot = pSubtree;
        // }
    }
} // namespace configmgr


