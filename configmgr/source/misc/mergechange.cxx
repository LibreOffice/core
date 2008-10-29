/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mergechange.cxx,v $
 * $Revision: 1.27 $
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

#include "builddata.hxx"
#include "mergechange.hxx"
#include "updatehelper.hxx"
#include "treeactions.hxx"
#include "treefragment.hxx"
#include "change.hxx"
#include "treechangefactory.hxx"
#include "treechangelist.hxx"
#include "configexcept.hxx"
#include "tracer.hxx"

#define ASCII(x) rtl::OUString::createFromAscii(x)

namespace configmgr
{
    // -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    class OMergeChanges : private ChangeTreeAction, private OPathCreator<configuration::RelativePath>
    {
        SubtreeChange &m_rSubtreeChange;          // ChangeList, which will be grown
        SubtreeChange *m_pCurrentParent;          // our current position

        // ------- Helper for Path stack -------
        SubtreeChange* pushTree(SubtreeChange& _rTree);
        void popTree(SubtreeChange* _pSaveTree);

    public:
        // CTor
        OMergeChanges(SubtreeChange& _rTree);

        // start function, with the Change we want to do.
        // WARNING this could be a big tree, because a change can contain subtreechanges!
        void mergeChanges(const SubtreeChange &_rChange, const configuration::RelativePath& _aPathToChange);
        void mergeChanges(const SubtreeChange &_rChange);

    private:
        void initRoot(const SubtreeChange &_rRootChange, const configuration::RelativePath& _aPathToChange);
    private:
        virtual void handle(ValueChange const& _rValueNode);
        virtual void handle(AddNode const& _rAddNode);
        virtual void handle(RemoveNode const& _rRemoveNode);
        virtual void handle(SubtreeChange const& _rSubtree);

    };
    // -----------------------------------------------------------------------------
    void combineUpdates(SubtreeChange  const& _anUpdate, SubtreeChange& _aCombinedUpdate)
    {
        OMergeChanges aCombined(_aCombinedUpdate);
        aCombined.mergeChanges(_anUpdate);
    }
    // -----------------------------------------------------------------------------

    configuration::Path::Component ONameCreator::createName(Change const& _rChange, SubtreeChange const* _pParent)
    {
        OSL_ENSURE(_pParent, "ONameCreator: Cannot create proper name without a parent");
        if (_pParent && _pParent->isSetNodeChange())
        {
            rtl::OUString sElementName = _rChange.getNodeName();
            rtl::OUString sTypeName = _pParent->getElementTemplateName();

            return configuration::Path::makeCompositeName(sElementName, sTypeName);
        }
        else
        {
            rtl::OUString sElementName = _rChange.getNodeName();

        //    OSL_ENSURE(isSimpleName(sElementName),"Unexpected: Non-simple name in non-set node");

            return configuration::Path::wrapSafeName(sElementName);
        }
    }


    // -----------------------------------------------------------------------------
    class OMergeValueChange : private ChangeTreeModification
    {
        SubtreeChange&      m_rTargetParent;
        const ValueChange&  m_aValueChange;
    public:
        OMergeValueChange(SubtreeChange& _rTargetParent, const ValueChange& _aValueChange)
                :m_rTargetParent(_rTargetParent)
                ,m_aValueChange(_aValueChange)
        {

        }
        void handleChange(Change &_rNode)
        {
            this->applyToChange(_rNode);
        }
    private:
        virtual void handle(ValueChange& _rValueChange)
        {
            // POST: Handle ValueChange
            _rValueChange.setNewValue(m_aValueChange.getNewValue(), m_aValueChange.getMode());
        }
              virtual void handle(RemoveNode& /*_rRemoveNode*/)
        {
            OSL_ENSURE(false, "OMergeValueChange::handle(ValueChange): have a ValueChange for a removed node!");
            // should never happen. How did the user change a value for a node which is obviously flagged as removed?
        }
        virtual void handle(AddNode& _rAddNode);
        virtual void handle(SubtreeChange& _rSubtree)
        {
            if ( isLocalizedValueSet(_rSubtree) )
            {
                std::auto_ptr<ValueChange> pNewValueChange( new ValueChange(m_aValueChange) );
                OSL_ENSURE(pNewValueChange->isLocalizedValue(), "OMergeValueChange:handle(SubtreeChange): have a non-localized ValueChange a for a localized node!");

                std::auto_ptr<Change> pNewChange( pNewValueChange.release() );

                replaceExistingEntry(pNewChange);
            }
            else
                OSL_ENSURE(false, "OMergeValueChange:handle(SubtreeChange): have a ValueChange for a sub tree!");
        }

        void replaceExistingEntry(std::auto_ptr<Change> pNewChange)
        {
            m_rTargetParent.removeChange(pNewChange->getNodeName());
            m_rTargetParent.addChange(pNewChange);
        }
        static std::auto_ptr<ValueNode> createNodeFromChange(ValueChange const& rChange)
        {
            std::auto_ptr<ValueNode> aRet;

            uno::Any aNewValue = rChange.getNewValue();

            // currently not supporting change modes !
            if (aNewValue.hasValue())
                aRet.reset( new ValueNode(rChange.getNodeName(), aNewValue, rChange.getAttributes()) );

            else  // NULL
            {
                aRet.reset( new ValueNode(rChange.getNodeName(), rChange.getValueType(), rChange.getAttributes()) );

                OSL_ENSURE(aRet->isValid(), "Cannot recover type for change to NULL");
            }
            return aRet;
        }

    };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    // TODO: operate directly on new data structures

    void OMergeValueChange::handle(AddNode& _rAddNode)
    {
        if (m_aValueChange.isToDefault())
        {
            std::auto_ptr<Change> aChangeToDefault(m_aValueChange.clone());
            m_rTargetParent.removeChange(_rAddNode.getNodeName());
            m_rTargetParent.addChange( aChangeToDefault );
        }
        else
        {
            // POST: Handle ValueChange in AddNode
            rtl::Reference< data::TreeSegment > seg(_rAddNode.getNewTree());
            std::auto_ptr<INode> pAddedNode = data::convertTree(seg.is() ? seg->fragment : 0, false);

            if (ValueNode *pValueNode = pAddedNode->asValueNode())
            {
                m_aValueChange.applyChangeNoRecover(*pValueNode);
            }

            else if (ISubtree* pValueSetNode = pAddedNode->asISubtree() )
            {
                if ( isLocalizedValueSet(*pValueSetNode) )
                {
                    std::auto_ptr<ValueNode> pNewValueNode = createNodeFromChange(m_aValueChange);
                    if (pNewValueNode.get())
                    {
                        OSL_ENSURE(pNewValueNode->isLocalized(), "OMergeValueChange:handle(AddNode): have a non-localized ValueChange a for a localized node!");
                        pNewValueNode->setName(pAddedNode->getName());
                    }
                    else
                        OSL_ENSURE(false, "OMergeValueChange:handle(SubtreeChange): Creating a NULL node to replace a localized value set not yet supported");


                    pAddedNode.reset(pNewValueNode.release());
                }
                else
                {
                    OSL_ENSURE(sal_False, "OMergeValueChange:handle(AddNode): have a ValueChange a for non-value node!");
                    pAddedNode.reset(); // leave unchanged
                }

            }
            else
            {
                OSL_ENSURE(sal_False, "OMergeValueChange:handle(AddNode): Found unknown node type!");
                pAddedNode.reset(); // leave unchanged
            }

            if (pAddedNode.get() != NULL)
            {
                rtl::Reference< data::TreeSegment > aNewTree = data::TreeSegment::create(_rAddNode.getNodeName(),pAddedNode);

                std::auto_ptr<AddNode> pNewAdd( new AddNode(aNewTree,m_aValueChange.getNodeName(), m_aValueChange.isToDefault()) );
                if (_rAddNode.isReplacing())
                    pNewAdd->setReplacing();

                std::auto_ptr<Change> pNewChange( pNewAdd.release() );
                replaceExistingEntry(pNewChange);
            }
        }
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
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
                if (aValueChange.getAttributes().existsInDefault())
                    m_eAction = FlagDeleted;
                else
                    m_eAction = RemoveCompletely;
            }

              virtual void handle(RemoveNode& /*_rRemoveNode*/)
            {
                OSL_ENSURE(false, "OMergeRemoveNode::handle(RemoveNode): should never happen!");
                // how can a RemoveNode change exist if in the file we're merging it into
                // there already is such a RemoveNode change (_rRemoveNode)?
            }

        virtual void handle(AddNode& _rAddNode)
            {
                if (_rAddNode.isReplacing())
                    m_eAction = FlagDeleted;
                else
                    m_eAction = RemoveCompletely;
            }

        virtual void handle(SubtreeChange& _rSubtree)
            {
                if (_rSubtree.getAttributes().existsInDefault())
                    m_eAction = FlagDeleted;
                else
                    m_eAction = RemoveCompletely;
            }
    };
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------

    static
    inline
    Change* findExistingChange(SubtreeChange* pCurrentParent, configuration::Path::Component const & _aName)
    {
        OSL_ASSERT(pCurrentParent);

        Change *pChange = pCurrentParent->getChange(_aName.getName());

        if (!pChange && !_aName.isSimpleName())
        {
            pChange = pCurrentParent->getChange(_aName.toPathString());
            OSL_ENSURE(!pChange, "Update trouble: Existing node found only by composite name while merging");
        }

        return pChange;
    }

    static
    inline
    Change* findExistingChange(SubtreeChange* pCurrentParent, rtl::OUString const & _aName)
    {
        OSL_ASSERT(pCurrentParent);

        Change *pChange = pCurrentParent->getChange(_aName);

        return pChange;
    }
    // -----------------------------------------------------------------------------

    void adjustElementTemplate(SubtreeChange& _rChange, const rtl::OUString& _rName, const rtl::OUString& _rModule)
    {
        if (!_rChange.isSetNodeChange() || isGenericSetElementType(_rChange.getElementTemplateName()))
        {
            _rChange.setElementTemplate(_rName,_rModule);
        }
        else if ( isDummySetElementModule(_rChange.getElementTemplateModule()) &&
                 !isGenericSetElementType(_rName) && !isDummySetElementModule(_rModule))
        {
            OSL_ENSURE(_rChange.getElementTemplateName() == _rName,   "Adjusting: Template modules do not match");

            _rChange.setElementTemplate(_rName,_rModule);
        }
        OSL_POSTCOND(_rChange.getElementTemplateName()   == _rName   || isGenericSetElementType(_rName),
                     "Adjusting: Template modules do not match");
        OSL_POSTCOND(_rChange.getElementTemplateModule() == _rModule || isDummySetElementModule(_rModule),
                     "Adjusting: Template modules do not match");
    }
    // -----------------------------------------------------------------------------

    inline void adjustElementTemplate(SubtreeChange& _rChange, SubtreeChange const& _rSource)
    {
        if (_rSource.isSetNodeChange())
            adjustElementTemplate(_rChange, _rSource.getElementTemplateName(), _rSource.getElementTemplateModule());
    }

    // -----------------------------------------------------------------------------

    // CTor
    OMergeChanges::OMergeChanges(SubtreeChange& _rTree)
    : m_rSubtreeChange(_rTree), m_pCurrentParent(NULL)
    {
    }
    // -----------------------------------------------------------------------------

    void OMergeChanges::initRoot(const SubtreeChange &_rRootChange, const configuration::RelativePath& _aPathToChange)
    {
        SubtreeChange* pCurrentParent = &m_rSubtreeChange;

        if (!_aPathToChange.isEmpty())
        {
            OSL_PRECOND(_aPathToChange.getLocalName().getName() == _rRootChange.getNodeName(),
                            "Path to change root does not match change being merged" );

            std::vector<configuration::Path::Component>::const_reverse_iterator const firstEnsure = _aPathToChange.begin();
            std::vector<configuration::Path::Component>::const_reverse_iterator lastEnsure = _aPathToChange.end();
            std::vector<configuration::Path::Component>::const_reverse_iterator it;

            OSL_ASSERT( firstEnsure != lastEnsure );
            --lastEnsure; // last to ensure is the actual root

            for( it = firstEnsure; it != lastEnsure; ++it)
            {
                OSL_ASSERT( it != _aPathToChange.end() );

                Change *pChange = findExistingChange(pCurrentParent,*it);

                if (!pChange)
                {
                    OSL_ASSERT( it+1 != _aPathToChange.end());
                    rtl::OUString const aElementTypeName = (it+1)->getTypeName();

                    // create a correspondens for the name, we did not find.
                    std::auto_ptr<SubtreeChange> pNewChange =
                        OTreeChangeFactory::createDummyChange(it->getName(), aElementTypeName);

                    pChange = pNewChange.get();

                    pCurrentParent->addChange(base_ptr(pNewChange));

                    OSL_ENSURE(pChange == findExistingChange(pCurrentParent,*it),
                                "ERROR: Newly added change cannot be found in parent change");
                }

                pCurrentParent = dynamic_cast<SubtreeChange*>( pChange);
                if (pCurrentParent == 0)
                {
                    OSL_ENSURE(false, "Change to merge does not point to a Subtree Change");
                    throw configuration::InvalidName(_aPathToChange.toString(), "points to a non- subtree change in this changes list, but a subtree change is required as root.");
                }
            }

            Change *pRootChange = findExistingChange(pCurrentParent,*lastEnsure);

            if (!pRootChange)
            {
                // create a correspondens for the name, we did not find.
                std::auto_ptr<SubtreeChange> pNewChange(
                        new SubtreeChange(_rRootChange, treeop::NoChildCopy()) );

                pRootChange = pNewChange.get();

                pCurrentParent->addChange(base_ptr(pNewChange));

                OSL_ENSURE(pRootChange == findExistingChange(pCurrentParent,*it),
                            "ERROR: Newly added change cannot be found in parent change");
            }

            pCurrentParent = dynamic_cast<SubtreeChange*>( pRootChange);
            if (pCurrentParent == 0)
            {
                OSL_ENSURE(false, "Change to merge does not point to a Subtree Change");
                throw configuration::InvalidName(_aPathToChange.toString(), "points to a non-subtree change in this changes list, but a subtree change is required as root.");
            }
        }

        OSL_ENSURE(pCurrentParent->getNodeName() == _rRootChange.getNodeName(),
                        "Change being merged has a different name");

        adjustElementTemplate(*pCurrentParent,_rRootChange);

        this->init(_aPathToChange);

        m_pCurrentParent = pCurrentParent;
    }
    // -----------------------------------------------------------------------------

    // ------- Helper for Path stack -------
    SubtreeChange* OMergeChanges::pushTree(SubtreeChange& _rTree)
    {
        pushName( ONameCreator::createName(_rTree,m_pCurrentParent) );

        SubtreeChange* pSave = m_pCurrentParent;
        m_pCurrentParent = &_rTree;
        return pSave;
    }
    void OMergeChanges::popTree(SubtreeChange* _pSaveTree)
    {
        m_pCurrentParent = _pSaveTree;

        popName();
    }
    // -----------------------------------------------------------------------------

    // start function, with the Change we want to do.
    // WARNING this could be a big tree, because a change can contain subtreechanges!
    void OMergeChanges::mergeChanges(const SubtreeChange &_rChange)
    {
        mergeChanges(_rChange, configuration::RelativePath());
    }
    // -----------------------------------------------------------------------------

    // start function, with the Change we want to do.
    // WARNING this could be a big tree, because a change can contain subtreechanges!
    void OMergeChanges::mergeChanges(const SubtreeChange &_rChange, const configuration::RelativePath& _aPathToChange)
    {
        initRoot(_rChange, _aPathToChange); // path location being merged must exist

        this->applyToChildren(_rChange); //- semantics ?
    }
    // -----------------------------------------------------------------------------

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
        rtl::OUString aNodeName = _rValueNode.getNodeName();

        if (Change *pChange = findExistingChange(m_pCurrentParent,aNodeName))
        {
            // Value found, merge content
            OMergeValueChange aMergeValue(*m_pCurrentParent,_rValueNode);
            aMergeValue.handleChange(*pChange);
        }
        else
        {
            // there is no ValueChange in the List, insert new one
            std::auto_ptr<Change> pNewChange(new ValueChange(_rValueNode));
            m_pCurrentParent->addChange(pNewChange);
        }
    }
    // -----------------------------------------------------------------------------

    void OMergeChanges::handle(AddNode const& _rAddNode)
    {
        // Handle an AddNode
        bool bReplacing = _rAddNode.isReplacing();

        rtl::OUString aNodeName = _rAddNode.getNodeName();

        if (Change *pChange = findExistingChange(m_pCurrentParent,aNodeName))
        {
            OSL_ENSURE(dynamic_cast< RemoveNode * >(pChange) != 0 || bReplacing, "OMergeChanges::handle(AddNode): the changes tree given already contains a change for this!");

            m_pCurrentParent->removeChange(pChange->getNodeName());

            bReplacing = true;
        }

        // insert manually
        rtl::Reference< data::TreeSegment > aAddedTree = data::TreeSegment::create(_rAddNode.getNewTree());

        std::auto_ptr<AddNode> pNewAdd(new AddNode(aAddedTree, _rAddNode.getNodeName(), _rAddNode.isToDefault()));
        if (bReplacing)
            pNewAdd->setReplacing();

        std::auto_ptr<Change> pNewChange( pNewAdd.release() );
        m_pCurrentParent->addChange(pNewChange);
    }
    // -----------------------------------------------------------------------------

    void OMergeChanges::handle(RemoveNode const& _rRemoveNode)
    {
        // Handle a RemoveNode
        rtl::OUString aNodeName = _rRemoveNode.getNodeName();

        Change *pChange = findExistingChange(m_pCurrentParent,aNodeName);

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
            std::auto_ptr<Change> pNewChange(new RemoveNode(_rRemoveNode.getNodeName(),_rRemoveNode.isToDefault()));
            m_pCurrentParent->addChange(pNewChange);
        }
        break;
        }
    }

// -----------------------------------------------------------------------------
    inline
    void OStripDefaults::stripOne(Change& _rChange)
    {
        m_rParent.removeChange(_rChange.getNodeName());
    }

    void OStripDefaults::handle(ValueChange& _rValueNode)
    {
        if (_rValueNode.isToDefault())
            stripOne(_rValueNode);
    }
    void OStripDefaults::handle(AddNode& _rAddNode)
    {
        if (_rAddNode.isToDefault())
        {
            sharable::TreeFragment const * pAdded = _rAddNode.getNewTreeData();
            OSL_ENSURE(pAdded,"No Data in AddNode");
            if (pAdded == NULL || pAdded->getAttributes().isDefault())
                stripOne(_rAddNode);

            // else we should strip the defaults from the added node
        }
    }
    void OStripDefaults::handle(RemoveNode& _rRemoveNode)
    {
        if (_rRemoveNode.isToDefault())
            stripOne(_rRemoveNode);
    }
    void OStripDefaults::handle(SubtreeChange& _rSubtree)
    {
        if ( strip(_rSubtree) )
            if (_rSubtree.isToDefault() || !_rSubtree.isSetNodeChange())
                stripOne(_rSubtree);
    }

    OStripDefaults& OStripDefaults::strip()
    {
        SubtreeChange::MutatingChildIterator it = m_rParent.begin_changes(), stop = m_rParent.end_changes();

        while (it != stop)
        {
            this->applyToChange(*it++);
        }

        return *this;
    }


// -----------------------------------------------------------------------------
    class TreeUpdater : public ChangeTreeAction
    {
        ISubtree* m_pCurrentSubtree;
#if OSL_DEBUG_LEVEL > 1
        std::vector<rtl::OString> aLog;
#endif

    public:
        TreeUpdater(ISubtree* pSubtree):m_pCurrentSubtree(pSubtree){}

        void handle(ValueChange const& aValueNode);
        void handle(AddNode const& aAddNode);
        void handle(RemoveNode const& aRemoveNode);
        void handle(SubtreeChange const& aSubtree);
    };

// -----------------------------------------------------------------------------


    void OMergeChanges::handle(SubtreeChange const& _rSubtree)
    {
        // Handle a SubtreeChange
        // we must check if exact this SubtreeChange is in the TreeChangeList, if not,
        // we must add this SubtreeChange to the TreeChangeList
        // with the pointer m_pCurrentParent we remember our SubtreeChange in witch we
        // add all other Changes.

        rtl::OUString aNodeName = _rSubtree.getNodeName();

        Change *pChange = findExistingChange(m_pCurrentParent,aNodeName);

        // const sal_Char* pType = pChange ? pChange->getType() : NULL;
        SubtreeChange* pSubtreeChange = NULL;
        if (pChange == NULL || dynamic_cast< SubtreeChange * >(pChange) != 0)
        {
            // need to create a new Subtreechange
            if (!pChange)
            {
                // create a new SubtreeChange
                std::auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(_rSubtree, treeop::NoChildCopy()));
                pSubtreeChange = pNewChange.get();

                // add the new SubtreeChange in m_aTreeChangeList
                m_pCurrentParent->addChange(std::auto_ptr<Change>(pNewChange.release()));
                // check list for this new SubtreeChange
                OSL_ASSERT(pSubtreeChange == findExistingChange(m_pCurrentParent,aNodeName));
            }
            else
            {
                pSubtreeChange = dynamic_cast<SubtreeChange*>(pChange);
                OSL_ASSERT(pSubtreeChange != 0);
                adjustElementTemplate(*pSubtreeChange,_rSubtree);
            }

            // save this SubtreeChange so we allways have the last Subtree
            SubtreeChange* pSaveParent = pushTree(*pSubtreeChange);
            this->applyToChildren(_rSubtree);
            popTree( pSaveParent );
        }
        else if (AddNode* pAddNode = dynamic_cast<AddNode*>(pChange))
        {
            rtl::Reference< data::TreeSegment > seg(pAddNode->getNewTree());
            std::auto_ptr<INode> pAddedNode = data::convertTree(seg.is() ? seg->fragment : 0, false);
            ISubtree* pSubtree = pAddedNode.get() ? pAddedNode->asISubtree() : 0;
            if (pSubtree)
            {
                pSubtree->markAsDefault( _rSubtree.isToDefault() );

                // Now apply _rSubtree to the subtree
                TreeUpdater aTreeUpdate(pSubtree);
                aTreeUpdate.applyToChildren(_rSubtree);

                // make a new subtree with the changed data
                rtl::Reference< data::TreeSegment > aNewTree = data::TreeSegment::create(pAddNode->getNodeName(), pAddedNode);

                std::auto_ptr<AddNode> pNewAdd( new AddNode(aNewTree, pAddNode->getNodeName(), pAddNode->isToDefault()) );
                if (pAddNode->isReplacing())
                    pNewAdd->setReplacing();

                std::auto_ptr<Change> pNewChange( pNewAdd.release() );

                m_pCurrentParent->removeChange(pAddNode->getNodeName());
                m_pCurrentParent->addChange( pNewChange );
            }
            else
            {
                OSL_ENSURE(false, "OMergeChanges: Unexpected node type found in an AddNode.");
                /* wrong type of node found: böse ASSERTEN/WERFEN */;
            }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
        }
        else
        {
            OSL_ENSURE(false, "OMergeChanges: Unexpected change type found for a subtree.");
            /* wrong type of node found: böse ASSERTEN/WERFEN */;
        }
    }

    // --------------------------------- updateTree ---------------------------------
    void TreeUpdater::handle(ValueChange const& aValueNode)
    {
        // Change a Value
        OSL_ENSURE(m_pCurrentSubtree,"Cannot apply ValueChange without subtree");

        INode* pBaseNode = m_pCurrentSubtree ? m_pCurrentSubtree->getChild(aValueNode.getNodeName()) : 0;
        OSL_ENSURE(pBaseNode,"Cannot apply Change: No node to change");

        ValueNode* pValue = pBaseNode ? pBaseNode->asValueNode() : 0;
        OSL_ENSURE(pValue,"Cannot apply ValueChange: Node is not a value");

        if (pValue)
            aValueNode.applyChangeNoRecover(*pValue);
#if OSL_DEBUG_LEVEL > 1
        else
        {
            ::rtl::OString aStr("TreeUpdater: Can't find value with name:=");
            aStr += rtl::OUStringToOString(aValueNode.getNodeName(),RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(pValue, aStr.getStr());
            aLog.push_back(aStr);
        }
#endif
    }

    void TreeUpdater::handle(AddNode const& aAddNode)
    {
        // Add a new Value
        if (m_pCurrentSubtree)
        {
            if (aAddNode.isReplacing())
            {
                std::auto_ptr<INode> aOldNode = m_pCurrentSubtree->removeChild(aAddNode.getNodeName());

#if OSL_DEBUG_LEVEL > 1
                OSL_ENSURE(aOldNode.get(), "TreeUpdater:AddNode: can't recover node being replaced");
                if (aOldNode.get() == NULL)
                    aLog.push_back(rtl::OString("TreeUpdater: can't recover node being replaced (for AddNode)"));
#endif
            }

            rtl::Reference< data::TreeSegment > seg(aAddNode.getNewTree());
            std::auto_ptr<INode> pNode(data::convertTree(seg.is() ? seg->fragment : 0, true));

            m_pCurrentSubtree->addChild(pNode);
        }
#if OSL_DEBUG_LEVEL > 1
        else
            aLog.push_back(rtl::OString("TreeUpdater: no CurrentSubtree for AddNode"));
#endif

    }

    void TreeUpdater::handle(RemoveNode const& aRemoveNode)
    {
        // remove a Value
        if (m_pCurrentSubtree)
        {
            std::auto_ptr<INode> aOldNode = m_pCurrentSubtree->removeChild(aRemoveNode.getNodeName());

#if OSL_DEBUG_LEVEL > 1
            if (NULL == aOldNode.get())
            {
                ::rtl::OString aStr("TreeUpdater: Can't remove child with name:=");
                aStr += rtl::OUStringToOString(aRemoveNode.getNodeName(),RTL_TEXTENCODING_ASCII_US);
                OSL_ENSURE(false, aStr.getStr());
                aLog.push_back(aStr);
            }
#endif
        }
    }

    void TreeUpdater::handle(SubtreeChange const& _aSubtree)
    {
        // handle traversion
        ISubtree *pOldSubtree = m_pCurrentSubtree;
        rtl::OUString aNodeName = _aSubtree.getNodeName();

        INode* pChild = m_pCurrentSubtree->getChild(aNodeName);
        OSL_ENSURE(pChild, "TreeUpdater::handle : invalid subtree change ... no child for change !");
        m_pCurrentSubtree = pChild ? pChild->asISubtree() : NULL;

#if OSL_DEBUG_LEVEL > 1
        if (!m_pCurrentSubtree)
        {
            ::rtl::OString aStr("TreeUpdater: there is no Subtree for name:=");
            aStr += rtl::OUStringToOString(_aSubtree.getNodeName(),RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(false, aStr.getStr());
            aLog.push_back(aStr);
        }
#endif
        // recurse
        if (m_pCurrentSubtree)
        {
            m_pCurrentSubtree->markAsDefault( _aSubtree.isToDefault() );
            _aSubtree.forEachChange(*this);
        }

        m_pCurrentSubtree = pOldSubtree;
    }

} // namespace configmgr
