/*************************************************************************
 *
 *  $RCSfile: mergechange.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-12 15:04:11 $
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

#ifndef CONFIGMGR_CONFNAME_HXX_
#include "confname.hxx"
#endif


#include "mergechange.hxx"

#include "change.hxx"
#include "treeprovider.hxx"
#include "treeactions.hxx"
#include "tracer.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#define ASCII(x) rtl::OUString::createFromAscii(x)

namespace configmgr
{
    using namespace com::sun::star::uno;

    void ONameCreator::init(const ConfigurationName &_aName)
    {
        // HACK!
        if (!(_aName.fullName().equals(ASCII("/"))))
        {
            // initial Name
            for(ConfigurationName::Iterator it = _aName.begin();
                it != _aName.end();
                ++it)
            {
                m_aNameList.push_back(*it);
            }
        }
    }

    rtl::OUString ONameCreator::createName(const rtl::OUString &aPlusName)
    {
        // create a name
        OUString aName;
        for (vector<OUString>::const_iterator it = m_aNameList.begin();
             it != m_aNameList.end();
             it++)
        {
            aName += *it;
            aName += OUString::createFromAscii("/");
        }
        if (aPlusName.getLength() == 0)
        {
            aName = aName.copy(0, aName.getLength() - 1);
        }
        else
        {
            aName += aPlusName;
        }

        return aName;
    }
    void ONameCreator::pushName(const rtl::OUString &aName)
    {
        m_aNameList.push_back(aName);
    }
    void ONameCreator::popName()
    {
        m_aNameList.pop_back();
    }


    // -----------------------------------------------------------------------------

    // Helperclass to search a change in a given changetree
    OMergeSearchChange::OMergeSearchChange(const rtl::OUString &_aName)
            : m_aSearchName(_aName), m_bFound(false), m_pFoundChange(NULL) {}

    void OMergeSearchChange::handle(ValueChange& _rValueNode)
    {
        rtl::OUString aName = createName(_rValueNode.getNodeName());
        if (aName.equals(m_aSearchName))
        {
            m_bFound = true;
            m_pFoundChange = &_rValueNode;
        }
    }

    void OMergeSearchChange::handle(AddNode& _rAddNode)
    {
        rtl::OUString aName = createName(_rAddNode.getNodeName());
        if (aName.equals(m_aSearchName))
        {
            m_bFound = true;
            m_pFoundChange = &_rAddNode;
        }
    }

    void OMergeSearchChange::handle(RemoveNode& _rRemoveNode)
    {
        rtl::OUString aName = createName(_rRemoveNode.getNodeName());
        if (aName.equals(m_aSearchName))
        {
            m_bFound = true;
            m_pFoundChange = &_rRemoveNode;
        }
    }

    void OMergeSearchChange::handle(SubtreeChange& _rSubtree)
    {
        rtl::OUString aName = createName(_rSubtree.getNodeName());
        if (aName.equals(m_aSearchName))
        {
            m_bFound = true;
            m_pFoundChange = &_rSubtree;
        }
        if (!m_bFound)
        {
            // recursive descent
            pushName(_rSubtree.getNodeName());
            _rSubtree.forEachChange(*this);
            popName();
        }
    }

    Change* OMergeSearchChange::searchForChange(Change &aChange)
    {
        applyToChange(aChange);
        if (isFound())
        {
            return m_pFoundChange;
        }
        return NULL;
    }
    // -----------------------------------------------------------------------------
    class OMergeValueChange : private ChangeTreeModification
    {
        const ValueChange& m_aValueChange;
    public:
        OMergeValueChange(const ValueChange& _aValueChange)
                :m_aValueChange(_aValueChange)
            {

            }
        void handleChange(Change &_rNode)
            {
                applyToChange(_rNode);
            }
    private:
        virtual void handle(ValueChange& _rValueChange)
            {
                // POST: Handle ValueChange
                _rValueChange.setNewValue(m_aValueChange.getNewValue(), m_aValueChange.getMode());
            }
        virtual void handle(RemoveNode& _rRemoveNode)
            {
                OSL_ENSHURE(false, "OMergeValueChange::handle(ValueChange): have a ValueChange for a removed node!");
                // should never happen. How did the user change a value for a node which is obviously flagged as removed?
            }
        virtual void handle(AddNode& _rAddNode)
            {
                // POST: Handle ValueChange in AddNode
                INode* pINode = _rAddNode.getAddedNode();
                ValueNode *pValueNode = pINode->asValueNode();
                if (pValueNode)
                    pValueNode->setValue(m_aValueChange.getNewValue());
                else
                    OSL_ENSURE(sal_False, "OMergeValueChange:handle(AddNode): have a ValueChange a for non-value node!");
            }
        virtual void handle(SubtreeChange& _rSubtree)
            {
                OSL_ENSHURE(false, "OMergeValueChange:handle(SubtreeChange): have a ValueChange for a sub tree!");
            }
    };

    // -----------------------------------------------------------------------------
    class OMergeRemoveNode : private ChangeTreeModification
    {
    public:
        // actions to take with the remove node change
        enum Action
        {
            RemoveCompletely,
            FlagDeleted,
            Undetermined
        };

    protected:
        Action  m_eAction;

    public:
        OMergeRemoveNode() : m_eAction(Undetermined) {  }

        Action getAction() const { return m_eAction; }

        void handleChange(Change* _pChange)
            {
                if (_pChange)
                    applyToChange(*_pChange);
                else
                    // no change -> flag as deleted
                    m_eAction = FlagDeleted;
            }

    private:
        virtual void handle(ValueChange& aValueChange)
            {
                OSL_ENSHURE(false, "OMergeRemoveNode::handle(ValueChange): remove a value node?");
            }

        virtual void handle(RemoveNode& _rRemoveNode)
            {
                OSL_ENSHURE(false, "OMergeRemoveNode::handle(RemoveNode): should never happen!");
                // how can a RemoveNode change exist if in the file we're merging it into
                // there already is such a RemoveNode change (_rRemoveNode)?
            }

        virtual void handle(AddNode& _rAddNode)
            {
                // though this is suspicious, as currently no AddNode changes are created ...
                m_eAction = RemoveCompletely;
            }

        virtual void handle(SubtreeChange& _rSubtree)
            {
                m_eAction = FlagDeleted;
                // TODO: need an extra state to distinguish between nodes which are overwritten in the user layer
                // and nodes which are added to the user layer, but not existent in the share layer
                // In the latter case m_eAction could be set to RemoveCompletely
            }
    };


    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    class OMergeSubtreeChange : private ChangeTreeModification,public ONameCreator
    {
        const SubtreeChange& m_aSubtreeChange;
    public:
        OMergeSubtreeChange(const SubtreeChange& _aSubtreeChange)
                :m_aSubtreeChange(_aSubtreeChange)
            {

            }
        void handleChange(Change& _pChange)
            {
                applyToChange(_pChange);
            }

    private:
        virtual void handle(ValueChange& aValueChange)
            {
                OSL_ENSHURE(false, "OMergeSubtreeChange::handle(ValueChange): remove a value node?");
            }

        virtual void handle(RemoveNode& _rRemoveNode)
            {
                OSL_ENSHURE(false, "OMergeSubtreeChange::handle(RemoveNode): should never happen!");
            }

        virtual void handle(AddNode& _rAddNode)
            {
            }

        virtual void handle(SubtreeChange& _rSubtree)
            {
                // we will run through the exist tree and insert the new one.
                pushName(_rSubtree.getNodeName());
                _rSubtree.forEachChange(*this);
                popName();
            }
    };

    // -----------------------------------------------------------------------------
    // Main class for merging treechangelists

    // ------- Helper for Path stack -------
    void OMergeTreeChangeList::pushTree(SubtreeChange* _pTree)
    {
        m_pCurrentParent = _pTree;
        OSL_ENSHURE(m_pCurrentParent, "OMergeTreeChangeList::pushTree: must not be NULL!");
        m_aTreePathStack.push_back(_pTree);
    }
    void OMergeTreeChangeList::popTree()
    {
        m_aTreePathStack.pop_back();
        m_pCurrentParent = m_aTreePathStack.back();
    }

    // CTor
    OMergeTreeChangeList::OMergeTreeChangeList(TreeChangeList& _aTree)
            :m_aTreeChangeList(_aTree), m_pCurrentParent(NULL)
    {
    }

    SubtreeChange* OMergeTreeChangeList::check(const ConfigurationName &_aName)
    {
        // First check, if the aName is in the treechangelist
        ONameCreator aNameCreator;

        SubtreeChange* pCurrentParent = &m_aTreeChangeList.root;

        if (!(_aName.fullName().equals(ASCII("/"))))
        {
            for(ConfigurationName::Iterator it = _aName.begin();
                it != _aName.end();
                ++it)
            {
                aNameCreator.pushName(*it);
                rtl::OUString aSearchName = aNameCreator.createName(OUString());
                OMergeSearchChange a(aSearchName);
                Change *pChange = a.searchForChange(m_aTreeChangeList.root);

                if (!pChange)
                {
                    // create a correspondens for the name, we not found.
                    auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(*it, configuration::Attributes()));
                    pCurrentParent->addChange(auto_ptr<Change>(pNewChange.release()));

                    pChange = a.searchForChange(m_aTreeChangeList.root);
                }
                pCurrentParent = SAL_STATIC_CAST(SubtreeChange*, pChange);
            }
        }
        return pCurrentParent;
    }

    // start function, with the Change we want to do.
    // WARNING this could be a big tree, because a change can contain subtreechanges!
    void OMergeTreeChangeList::handleChange(TreeChangeList &_rList)
    {
        rtl::OUString aPath = _rList.pathToRoot;
        ConfigurationName aName(aPath, ConfigurationName::Absolute());
        init(aName);                     // our Name start with pathToRoot
        m_pCurrentParent = check(aName); // pathToRoot must exist or will be created

        applyToChange(_rList.root);
    }

    // Algorithm: search the actual path in the out m_aTreeChangeList
    // if we found something, we must merge/convert the Node with our Node
    // if we found nothing, we must create a new Node with our change
    // thats it.

    // the merge is contructed with helper classes because, it's possible that we
    // are a ValueChange but in the TreeChangeList this change is an AddNode, so
    // we have something to do.

    void OMergeTreeChangeList::handle(ValueChange const& _rValueNode)
    {
        // Handle a ValueChange,
        rtl::OUString aSearchName = createName(_rValueNode.getNodeName()); // this construct is only for better debugging
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aTreeChangeList.root);
        if (pChange)
        {
            // Value found, merge content
            OMergeValueChange a(_rValueNode);
            a.handleChange(*pChange);
        }
        else
        {
            // there is no ValueChange in the List, insert new one
            auto_ptr<Change> pNewChange(new ValueChange(_rValueNode));
            m_pCurrentParent->addChange(pNewChange);
        }
    }

    void OMergeTreeChangeList::handle(AddNode const& _rAddNode)
    {
        // Handle an AddNode
        rtl::OUString aSearchName = createName(_rAddNode.getNodeName());
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aTreeChangeList.root);

        if (pChange)
        {
            OSL_ENSURE(pChange->ISA(RemoveNode) || _rAddNode.isReplacing(), "OMergeTreeChangeList::handle(AddNode): the changes tree given already contains a change for this!");

            m_pCurrentParent->removeChange(pChange->getNodeName());
        }
        // insert manually
        auto_ptr<INode> pNode = auto_ptr<INode>(_rAddNode.getAddedNode()->clone());
        auto_ptr<Change> pNewChange(new AddNode(pNode, _rAddNode.getNodeName()));
        m_pCurrentParent->addChange(pNewChange);
    }

    void OMergeTreeChangeList::handle(RemoveNode const& _rRemoveNode)
    {
        // Handle a RemoveNode
        rtl::OUString aSearchName = createName(_rRemoveNode.getNodeName());
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aTreeChangeList.root);

                // examine what to do with this change
        OMergeRemoveNode aExaminer;
        aExaminer.handleChange(pChange);

                // remove the change from it's parent (may it's re-inserted in another form below)
        if (pChange)
            m_pCurrentParent->removeChange(pChange->getNodeName());

                // insert a new change if necessary
        switch (aExaminer.getAction())
        {
        case OMergeRemoveNode::RemoveCompletely:
            // nothing to do, we already removed it
            break;
        default:
            OSL_ENSURE(sal_False, "OMergeTreeChangeList::handle(RemoveNode): don't know what to do with this!");
            // NO BREAK.
            // defaulting this so that the node will be marked as deleted
        case OMergeRemoveNode::FlagDeleted:
        {
            auto_ptr<Change> pNewChange(new RemoveNode(_rRemoveNode.getNodeName()));
            m_pCurrentParent->addChange(pNewChange);
        }
        break;
        }
    }

    void OMergeTreeChangeList::handle(SubtreeChange const& _rSubtree)
    {
        // Handle a SubtreeChange
        // we must check if exact this SubtreeChange is in the TreeChangeList, if not,
        // we must add this SubtreeChange to the TreeChangeList
        // with the pointer m_pCurrentParent we remember our SubtreeChange in witch we
        // add all other Changes.

        rtl::OUString aSearchName = createName(_rSubtree.getNodeName());
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aTreeChangeList.root);
        /*
          if (pChange)
          {
          // Value found, merge content
          OMergeSubtreeChange a(_rSubtree);
          a.handleChange(*pChange);
          }
          else
          {
          // Value not found, create a new SubtreeChange
          auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(_rSubtree, SubtreeChange::NoChildCopy()));
          // add the new SubtreeChange in m_aTreeChangeList
          m_pCurrentParent->addChange(auto_ptr<Change>(pNewChange.release()));
          }
        */

                // const sal_Char* pType = pChange ? pChange->getType() : NULL;
        SubtreeChange* pSubtreeChange = NULL;
        if (pChange == NULL || pChange->ISA(SubtreeChange))
        {
            // hard cast(!) to SubtreeChange because we are a SubtreeChange
            pSubtreeChange = SAL_STATIC_CAST(SubtreeChange*, pChange);
            if (pSubtreeChange)
            {
                // Value found, nothing to be done, because we are a SubtreeChange
                // we only must go downstairs
            }
            else
            {
                // create a new SubtreeChange
                auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(_rSubtree, SubtreeChange::NoChildCopy()));
                // add the new SubtreeChange in m_aTreeChangeList
                m_pCurrentParent->addChange(auto_ptr<Change>(pNewChange.release()));
                // check list and get this new SubtreeChange
                pChange = a.searchForChange(m_aTreeChangeList.root);
                pSubtreeChange = SAL_STATIC_CAST(SubtreeChange*, pChange);
            }
            // save this SubtreeChange so we allways have the last Subtree
            pushTree(pSubtreeChange);          // remember the SubtreeChange Pointer
            pushName(_rSubtree.getNodeName()); // pathstack
            _rSubtree.forEachChange(*this);
            popName();
            popTree();
        }
        else if (pChange->ISA(AddNode))
        {
            AddNode* pAddNode = SAL_STATIC_CAST(AddNode*, pChange);
            INode* pNode = pAddNode->getAddedNode();
            ISubtree* pSubtree = pNode ? pNode->asISubtree() : 0;

            OSL_ENSURE(pSubtree, "BLA");
            if (pSubtree)
            {
                OSL_ENSHURE(false, "DANGER, THIS CODE IS WRONG!");
                // because, the important Node is the _rSubtree, which will not insert anywhere

                // Merge _rSubtree into pSubtree using a TreeUpdate object
                TreeUpdate aTreeUpdate(pSubtree);
                TreeChangeList aMergeChangeList(m_aTreeChangeList, SubtreeChange::NoChildCopy());
                OMergeTreeAction aChangeHandler(aMergeChangeList.root, pSubtree);
                m_aTreeChangeList.root.forEachChange(aChangeHandler);
                // now check the real modifications
                OChangeActionCounter aChangeCounter;
                aChangeCounter.handle(aMergeChangeList.root);
                CFG_TRACE_INFO("cache manager: counted changes from notification : additions: %i , removes: %i, value changes: %i", aChangeCounter.nAdds, aChangeCounter.nRemoves, aChangeCounter.nValues);
                if (aChangeCounter.hasChanges())
                {
                    // aTree.updateTree(aMergeChangeList);
                    aMergeChangeList.root.forEachChange(aTreeUpdate);
                }
            }
            else
            {
                /* wrong type of node found: böse ASSERTEN/WERFEN */;
            }

        }
        else
        {
            /* wrong type of node found: böse ASSERTEN/WERFEN */;
        }
    }



    // -----------------------------------------------------------------------------

    // ------- Helper for Path stack -------
    void OMergeChanges::pushTree(SubtreeChange* _pTree)
    {
        m_pCurrentParent = _pTree;
        OSL_ENSHURE(m_pCurrentParent, "OMergeChanges::pushTree: must not be NULL!");
        m_aTreePathStack.push_back(_pTree);
    }
    void OMergeChanges::popTree()
    {
        m_aTreePathStack.pop_back();
        m_pCurrentParent = m_aTreePathStack.back();
    }

    // CTor
    OMergeChanges::OMergeChanges(SubtreeChange& _aTree)
            :m_aSubtreeChange(_aTree), m_pCurrentParent(NULL)
    {
    }

    SubtreeChange* OMergeChanges::check(const ConfigurationName &_aName)
    {
        // First check, if the aName is in the subtreechange
        ONameCreator aNameCreator;

        SubtreeChange* pCurrentParent = &m_aSubtreeChange;

        if (!(_aName.fullName().equals(ASCII("/"))))
        {
            for(ConfigurationName::Iterator it = _aName.begin();
                it != _aName.end();
                ++it)
            {
                aNameCreator.pushName(*it);
                rtl::OUString aSearchName = aNameCreator.createName(OUString());
                OMergeSearchChange a(aSearchName);
                Change *pChange = a.searchForChange(m_aSubtreeChange);

                if (!pChange)
                {
                    // create a correspondens for the name, we not found.
                    auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(*it, configuration::Attributes()));
                    pCurrentParent->addChange(auto_ptr<Change>(pNewChange.release()));

                    pChange = a.searchForChange(m_aSubtreeChange);
                }
                pCurrentParent = SAL_STATIC_CAST(SubtreeChange*, pChange);

                // HACK!
                // no descent to anything, we are flat!
                break;
            }
        }
        return pCurrentParent;
    }

    // start function, with the Change we want to do.
    // WARNING this could be a big tree, because a change can contain subtreechanges!
    void OMergeChanges::handleChange(const SubtreeChange &_rList, const rtl::OUString &_aPathToRoot)
    {
        rtl::OUString aPath = _aPathToRoot;
        ConfigurationName aName(aPath, ConfigurationName::Absolute());
        // try to use an empty List.
        // init(aName);                     // our Name start with pathToRoot
        m_pCurrentParent = check(aName); // pathToRoot must exist or will be created

        applyToChange(_rList);
    }

    // Algorithm: search the actual path in the out m_aSubtreeChange
    // if we found something, we must merge/convert the Node with our Node
    // if we found nothing, we must create a new Node with our change
    // thats it.

    // the merge is contructed with helper classes because, it's possible that we
    // are a ValueChange but in the TreeChangeList this change is an AddNode, so
    // we have something to do.

    void OMergeChanges::handle(ValueChange const& _rValueNode)
    {
        // Handle a ValueChange,
        rtl::OUString aSearchName = createName(_rValueNode.getNodeName()); // this construct is only for better debugging
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aSubtreeChange);
        if (pChange)
        {
            // Value found, merge content
            OMergeValueChange a(_rValueNode);
            a.handleChange(*pChange);
        }
        else
        {
            // there is no ValueChange in the List, insert new one
            auto_ptr<Change> pNewChange(new ValueChange(_rValueNode));
            m_pCurrentParent->addChange(pNewChange);
        }
    }

    void OMergeChanges::handle(AddNode const& _rAddNode)
    {
        // Handle an AddNode
        rtl::OUString aSearchName = createName(_rAddNode.getNodeName());
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aSubtreeChange);

        if (pChange)
        {
            OSL_ENSURE(pChange->ISA(RemoveNode) || _rAddNode.isReplacing(), "OMergeTreeChangeList::handle(AddNode): the changes tree given already contains a change for this!");

            m_pCurrentParent->removeChange(pChange->getNodeName());
        }
        // insert manually
        auto_ptr<INode> pNode = auto_ptr<INode>(_rAddNode.getAddedNode()->clone());
        auto_ptr<Change> pNewChange(new AddNode(pNode, _rAddNode.getNodeName()));
        m_pCurrentParent->addChange(pNewChange);
    }

    void OMergeChanges::handle(RemoveNode const& _rRemoveNode)
    {
        // Handle a RemoveNode
        rtl::OUString aSearchName = createName(_rRemoveNode.getNodeName());
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aSubtreeChange);

                // examine what to do with this change
        OMergeRemoveNode aExaminer;
        aExaminer.handleChange(pChange);

                // remove the change from it's parent (may it's re-inserted in another form below)
        if (pChange)
            m_pCurrentParent->removeChange(pChange->getNodeName());

                // insert a new change if necessary
        switch (aExaminer.getAction())
        {
        case OMergeRemoveNode::RemoveCompletely:
            // nothing to do, we already removed it
            break;
        default:
            OSL_ENSURE(sal_False, "OMergeChanges::handle(RemoveNode): don't know what to do with this!");
            // NO BREAK.
            // defaulting this so that the node will be marked as deleted
        case OMergeRemoveNode::FlagDeleted:
        {
            auto_ptr<Change> pNewChange(new RemoveNode(_rRemoveNode.getNodeName()));
            m_pCurrentParent->addChange(pNewChange);
        }
        break;
        }
    }


// -----------------------------------------------------------------------------
    class TreeUpdater : public ChangeTreeModification
    {
        ISubtree* m_pCurrentSubtree;
#if DEBUG
        std::vector<rtl::OString> aLog;
#endif

    public:
        TreeUpdater(ISubtree* pSubtree):m_pCurrentSubtree(pSubtree){}

        void handle(ValueChange& aValueNode);
        void handle(AddNode& aAddNode);
        void handle(RemoveNode& aRemoveNode);
        void handle(SubtreeChange& aSubtree);
    };

// -----------------------------------------------------------------------------


    void OMergeChanges::handle(SubtreeChange const& _rSubtreeChange)
    {
        // Handle a SubtreeChange
        // we must check if exact this SubtreeChange is in the TreeChangeList, if not,
        // we must add this SubtreeChange to the TreeChangeList
        // with the pointer m_pCurrentParent we remember our SubtreeChange in witch we
        // add all other Changes.

        rtl::OUString aSearchName = createName(_rSubtreeChange.getNodeName());
        OMergeSearchChange a(aSearchName);
        Change *pChange = a.searchForChange(m_aSubtreeChange);

        SubtreeChange* pSubtreeChange = NULL;
        if (pChange == NULL || pChange->ISA(SubtreeChange))
        {
            // hard cast(!) to SubtreeChange because we are a SubtreeChange
            pSubtreeChange = SAL_STATIC_CAST(SubtreeChange*, pChange);
            if (pSubtreeChange)
            {
                // Value found, nothing to be done, because we are a SubtreeChange
                // we only must go downstairs
            }
            else
            {
                // create a new SubtreeChange
                auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(_rSubtreeChange, SubtreeChange::NoChildCopy()));
                // add the new SubtreeChange in m_aTreeChangeList
                m_pCurrentParent->addChange(auto_ptr<Change>(pNewChange.release()));
                // check list and get this new SubtreeChange
                pChange = a.searchForChange(m_aSubtreeChange);
                pSubtreeChange = SAL_STATIC_CAST(SubtreeChange*, pChange);
            }
            // save this SubtreeChange so we allways have the last Subtree
            pushTree(pSubtreeChange);          // remember the SubtreeChange Pointer
            pushName(_rSubtreeChange.getNodeName()); // pathstack
            _rSubtreeChange.forEachChange(*this);
            popName();
            popTree();
        }
        else if (pChange->ISA(AddNode))
        {
            // OSL_ENSHURE(false, "sorry, no addnode in subtreechange! can't supported yet.");

            AddNode* pAddNode = SAL_STATIC_CAST(AddNode*, pChange);
            INode* pNode = pAddNode->getAddedNode();
            ISubtree* pISubtree = pNode ? pNode->asISubtree() : 0;

            OSL_ENSURE(pISubtree, "Warning: there is no Subtree in the AddNode");
            if (pISubtree)
            {
                // ----------------------------
                // pISubtree += _rSubtreeChange
                // ----------------------------

                // Merge _rSubtreeChange into pSubtree using a TreeUpdate object
                SubtreeChange *pSubtreeChange = SAL_STATIC_CAST(SubtreeChange*, _rSubtreeChange.clone());
                TreeUpdater aTreeUpdate(pISubtree);
                pSubtreeChange->forEachChange(aTreeUpdate);

//
//                 // auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(_rSubtreeChange, SubtreeChange::NoChildCopy()));
//                 // OMergeTreeAction aChangeHandler(*pNewChange.get(), pISubtree);
//                 // pNewChange.get()->forEachChange(aChangeHandler);
//
//                 TreeChangeList aMergeChangeList(NULL, _rSubtreeChange.getNodeName(), _rSubtreeChange);
//
//                 OMergeTreeAction aChangeHandler(aMergeChangeList.root, pISubtree);
//                 m_aSubtreeChange.forEachChange(aChangeHandler);
//
//                 // now check the real modifications
//                 OChangeActionCounter aChangeCounter;
//                 // aChangeCounter.handle(*pNewChange.get());
//                 aChangeCounter.handle(aMergeChangeList.root);
//
//                 CFG_TRACE_INFO("cache manager: counted changes from notification : additions: %i , removes: %i, value changes: %i", aChangeCounter.nAdds, aChangeCounter.nRemoves, aChangeCounter.nValues);
//                 if (aChangeCounter.hasChanges())
//                 {
//                     // pNewChange.get()->forEachChange(aTreeUpdate);
//                     aMergeChangeList.root.forEachChange(aTreeUpdate);
//                 }
            }
            else
            {
                // wrong type of node found: böse ASSERTEN/WERFEN
            }
        }
        else
        {
            // wrong type of node found: böse ASSERTEN/WERFEN
        }
    }



    // --------------------------------- updateTree ---------------------------------
    void TreeUpdater::handle(ValueChange& aValueNode)
    {
        // Change a Value
        OSL_ENSURE(m_pCurrentSubtree,"Cannot apply ValueChange without subtree");

        INode* pBaseNode = m_pCurrentSubtree ? m_pCurrentSubtree->getChild(aValueNode.getNodeName()) : 0;
        OSL_ENSURE(pBaseNode,"Cannot apply Change: No node to change");

        ValueNode* pValue = pBaseNode ? pBaseNode->asValueNode() : 0;
        OSL_ENSURE(pValue,"Cannot apply ValueChange: Node is not a value");

        if (pValue)
            aValueNode.applyTo(*pValue);
#ifdef DEBUG
        else
        {
            ::rtl::OString aStr("TreeUpdater: Can't find value with name:=");
            aStr += rtl::OUStringToOString(aValueNode.getNodeName(),RTL_TEXTENCODING_ASCII_US);
            OSL_ENSHURE(pValue, aStr.getStr());
            aLog.push_back(aStr);
        }
#endif
    }

    void TreeUpdater::handle(AddNode& aAddNode)
    {
        // Add a new Value
        if (m_pCurrentSubtree)
        {
            if (aAddNode.isReplacing())
            {
                std::auto_ptr<INode> aOldNode = m_pCurrentSubtree->removeChild(aAddNode.getNodeName());

#ifdef DEBUG
                OSL_ENSHURE(aOldNode.get(), "TreeUpdater:AddNode: can't recover node being replaced");
                if (aOldNode.get() == NULL)
                    aLog.push_back(rtl::OString("TreeUpdater: can't recover node being replaced (for AddNode)"));
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
#ifdef DEBUG
        else
            aLog.push_back(rtl::OString("TreeUpdater: no CurrentSubtree for AddNode"));
#endif

    }

    void TreeUpdater::handle(RemoveNode& aRemoveNode)
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

#ifdef DEBUG
            if (!bOk)
            {
                ::rtl::OString aStr("TreeUpdater: Can't remove child with name:=");
                aStr += rtl::OUStringToOString(aRemoveNode.getNodeName(),RTL_TEXTENCODING_ASCII_US);
                OSL_ENSHURE(bOk, aStr.getStr());
                aLog.push_back(aStr);
            }
#endif
        }
    }

    void TreeUpdater::handle(SubtreeChange& _aSubtree)
    {
        // handle traversion
        ISubtree *pOldSubtree = m_pCurrentSubtree;
        rtl::OUString aNodeName = _aSubtree.getNodeName();
        OSL_ENSHURE(m_pCurrentSubtree->getChild(aNodeName), "TreeUpdater::handle : invalid subtree change ... this will crash !");
        m_pCurrentSubtree = m_pCurrentSubtree->getChild(aNodeName)->asISubtree();

#if DEBUG
        ::rtl::OString aStr("TreeUpdater: there is no Subtree for name:=");
        aStr += rtl::OUStringToOString(_aSubtree.getNodeName(),RTL_TEXTENCODING_ASCII_US);
        OSL_ENSHURE(m_pCurrentSubtree, aStr.getStr());
        if (!m_pCurrentSubtree)
            aLog.push_back(aStr);
#endif

        _aSubtree.forEachChange(*this);
        m_pCurrentSubtree = pOldSubtree;
    }

} // namespace configmgr
