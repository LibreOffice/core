/*************************************************************************
 *
 *  $RCSfile: mergechange.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:17:49 $
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

#include "mergechange.hxx"

#ifndef CONFIGMGR_MERGEHELPER_HXX
#include "mergehelper.hxx"
#endif
#ifndef CONFIGMGR_UPDATEHELPER_HXX
#include "updatehelper.hxx"
#endif
#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif

#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif


#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#define ASCII(x) rtl::OUString::createFromAscii(x)

namespace configmgr
{
    using namespace com::sun::star::uno;
    using namespace configuration;
    using namespace std;

    // -----------------------------------------------------------------------------
    // TODO: check name match
    void mergeLayer(TreeChangeList & _aLayer, ISubtree& _aTree)
    {
        OSL_ENSURE(_aLayer.root.getNodeName() == _aTree.getName(),
                    "Layer root node does not match the module name being updated");
        mergeLayerToTree(_aLayer.root,_aTree);
    }
    // -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // TODO: check name match
    void applyUpdateWithAdjustment(TreeChangeList & _anUpdate, memory::UpdateAccessor& _aUpdateAccess, data::NodeAddress const & _aBaseAddress)
    {
        OSL_ASSERT(_aBaseAddress.is());

        applyUpdateWithAdjustmentToTree(_anUpdate.root,_aUpdateAccess,_aBaseAddress);
    }

    // -----------------------------------------------------------------------------
    // TODO: check name match
    bool adjustUpdate(TreeChangeList & _anUpdate, memory::UpdateAccessor& _aUpdateAccess, data::NodeAddress const & _aBaseAddress)
    {
        OSL_ASSERT(_aBaseAddress.is());

        return adjustUpdateToTree(_anUpdate.root,_aUpdateAccess,_aBaseAddress);
    }
    // -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    class OMergeTreeChangeList : private ChangeTreeAction, private OPathCreator<AbsolutePath>
    {
        TreeChangeList &m_aTreeChangeList;       // ChangeList, which will be grown
        SubtreeChange *m_pCurrentParent;         // our current position

        // ------- Helper for Path stack -------
        SubtreeChange* pushTree(SubtreeChange& _rTree);
        void popTree(SubtreeChange* _pSaveTree);
    public:
        // CTor
        OMergeTreeChangeList(TreeChangeList& _aTree);

        // start function, with the Change we want to do.
        // WARNING this could be a big tree, because a change can contain subtreechanges!
        void mergeChanges(TreeChangeList const&_rList);
    private:
        void initRoot(TreeChangeList const& _aChanges);

    private:
        virtual void handle(ValueChange const& _rValueNode);
        virtual void handle(AddNode const& _rAddNode);
        virtual void handle(RemoveNode const& _rRemoveNode);
        virtual void handle(SubtreeChange const& _rSubtree);
    };
    // -----------------------------------------------------------------------------
    void applyLayerUpdate(TreeChangeList const& _anUpdate, TreeChangeList& _aLayer)
    {
        OMergeTreeChangeList aMerge(_aLayer);
        aMerge.mergeChanges(_anUpdate);
    }
    // -----------------------------------------------------------------------------
    class OMergeChanges : private ChangeTreeAction, private OPathCreator<RelativePath>
    {
        SubtreeChange &m_rSubtreeChange;          // ChangeList, which will be grown
        SubtreeChange *m_pCurrentParent;          // our current position

        typedef configuration::RelativePath RelativePath;
        // ------- Helper for Path stack -------
        SubtreeChange* pushTree(SubtreeChange& _rTree);
        void popTree(SubtreeChange* _pSaveTree);

    public:
        // CTor
        OMergeChanges(SubtreeChange& _rTree);

        // start function, with the Change we want to do.
        // WARNING this could be a big tree, because a change can contain subtreechanges!
        void mergeChanges(const SubtreeChange &_rChange, const RelativePath& _aPathToChange);
        void mergeChanges(const SubtreeChange &_rChange);

    private:
        void initRoot(const SubtreeChange &_rRootChange, const RelativePath& _aPathToChange);
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

    RelativePath ONameCreator::buildPath() const
    {
        Path::Rep aAdded;

        for(NameList::const_reverse_iterator it = m_aNameList.rbegin(),
                                             stop = m_aNameList.rend();
            it != stop;
            ++it)
        {
            aAdded.prepend(*it);
        }

        return RelativePath(aAdded);
    }

    RelativePath ONameCreator::buildPath(const FullName &aPlusName) const
    {
        OSL_ENSURE( !aPlusName.isEmpty(), "To create the base name use the parameterless overload" );

        Path::Rep aAdded(aPlusName);

        for(NameList::const_reverse_iterator it = m_aNameList.rbegin(),
                                             stop = m_aNameList.rend();
            it != stop;
            ++it)
        {
            aAdded.prepend(*it);
        }

        return RelativePath(aAdded);
    }

    Path::Component ONameCreator::createName(Change const& _rChange, SubtreeChange const* _pParent)
    {
        OSL_ENSURE(_pParent, "ONameCreator: Cannot create proper name without a parent");
        if (_pParent && _pParent->isSetNodeChange())
        {
            OUString sElementName = _rChange.getNodeName();
            OUString sTypeName = _pParent->getElementTemplateName();

            return Path::makeCompositeName(sElementName, sTypeName);
        }
        else
        {
            OUString sElementName = _rChange.getNodeName();

        //    OSL_ENSURE(isSimpleName(sElementName),"Unexpected: Non-simple name in non-set node");

            return Path::wrapSafeName(sElementName);
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
        virtual void handle(RemoveNode& _rRemoveNode)
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
            std::auto_ptr<INode> pAddedNode = _rAddNode.getNewTree().cloneData(false);

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
                data::TreeSegment aNewTree = data::TreeSegment::createNew(_rAddNode.getNodeName(),pAddedNode);

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

        virtual void handle(RemoveNode& _rRemoveNode)
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
    Change* findExistingChange(SubtreeChange* pCurrentParent, Path::Component const & _aName)
    {
        OSL_ASSERT(pCurrentParent);

        Change *pChange = pCurrentParent->getChange(_aName.getName().toString());

        if (!pChange && !_aName.isSimpleName())
        {
            pChange = pCurrentParent->getChange(_aName.toPathString());
            OSL_ENSURE(!pChange, "Update trouble: Existing node found only by composite name while merging");
        }

        return pChange;
    }

    static
    inline
    Change* findExistingChange(SubtreeChange* pCurrentParent, OUString const & _aName)
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
    // -----------------------------------------------------------------------------
    // Main class for merging treechangelists

    // CTor
    OMergeTreeChangeList::OMergeTreeChangeList(TreeChangeList& _aTree)
            :m_aTreeChangeList(_aTree), m_pCurrentParent(NULL)
    {
    }

    void OMergeTreeChangeList::initRoot(TreeChangeList const& _aChanges)
    {
        AbsolutePath aThisRootPath = m_aTreeChangeList.getRootNodePath();
        AbsolutePath aStartPath    = _aChanges.getRootNodePath();

        RelativePath aEnsurePath = Path::stripPrefix(aStartPath,aThisRootPath);

        OSL_PRECOND(aStartPath.getLocalName().getName().toString() == _aChanges.root.getNodeName(),
                        "Treechangelist has incorrect root path" );

        SubtreeChange* pCurrentParent = &m_aTreeChangeList.root;

        if (!aEnsurePath.isEmpty())
        {
            RelativePath::Iterator const firstEnsure = aEnsurePath.begin();
            RelativePath::Iterator lastEnsure = aEnsurePath.end();

            OSL_ASSERT( firstEnsure != lastEnsure );
            --lastEnsure; // last to ensure is the actual root

            for(RelativePath::Iterator it = firstEnsure; it != lastEnsure; ++it)
            {
                OSL_ASSERT( it != aEnsurePath.end() );

                Change *pChange = findExistingChange(pCurrentParent,*it);

                if (!pChange)
                {
                    OSL_ASSERT( it+1 != aEnsurePath.end());
                    Name const aElementTypeName = (it+1)->getTypeName();

                    // create a correspondens for the name, we did not find.
                    auto_ptr<SubtreeChange> pNewChange =
                        OTreeChangeFactory::createDummyChange(it->getName(), aElementTypeName);

                    pChange = pNewChange.get();

                    pCurrentParent->addChange(base_ptr(pNewChange));

                    OSL_ENSURE(pChange == findExistingChange(pCurrentParent,*it),
                                "ERROR: Newly added change cannot be found in parent change");
                }

                if (!pChange->ISA(SubtreeChange))
                {
                    OSL_ENSURE(false, "Change to merge does not point to a Subtree Change");
                    throw InvalidName(aEnsurePath.toString(), "points to a non- subtree change in this changes list, but a subtree change is required as root.");
                }
                pCurrentParent = static_cast<SubtreeChange*>( pChange);
            }

            Change *pRootChange = findExistingChange(pCurrentParent,*lastEnsure);

            if (!pRootChange)
            {
                // create a correspondens for the name, we did not find.
                auto_ptr<SubtreeChange> pNewChange(
                        new SubtreeChange(_aChanges.root, SubtreeChange::NoChildCopy()) );

                pRootChange = pNewChange.get();

                pCurrentParent->addChange(base_ptr(pNewChange));

                OSL_ENSURE(pRootChange == findExistingChange(pCurrentParent,*it),
                            "ERROR: Newly added change cannot be found in parent change");
            }

            if (!pRootChange->ISA(SubtreeChange))
            {
                OSL_ENSURE(false, "Change to merge does not point to a Subtree Change");
                throw InvalidName(aEnsurePath.toString(), "points to a non- subtree change in this changes list, but a subtree change is required as root.");
            }
            pCurrentParent = static_cast<SubtreeChange*>( pRootChange);
        }

        OSL_ENSURE(pCurrentParent->getNodeName() == _aChanges.root.getNodeName(),
                        "Change being merged has a different name" );

        adjustElementTemplate(*pCurrentParent,_aChanges.root);

        this->init(aStartPath);

        m_pCurrentParent = pCurrentParent;
    }

    // ------- Helper for Path stack -------
    SubtreeChange* OMergeTreeChangeList::pushTree(SubtreeChange& _rTree)
    {
        pushName( ONameCreator::createName(_rTree,m_pCurrentParent) );

        SubtreeChange* pSave = m_pCurrentParent;
        m_pCurrentParent = &_rTree;
        return pSave;
    }
    void OMergeTreeChangeList::popTree(SubtreeChange* _pSaveTree)
    {
        m_pCurrentParent = _pSaveTree;

        popName();
    }

    // start function, with the Change we want to do.
    // WARNING this could be a big tree, because a change can contain subtreechanges!
    void OMergeTreeChangeList::mergeChanges(TreeChangeList const&_rList)
    {
        initRoot(_rList ); // path location being merged must exist

        this->applyToChildren(_rList.root);
    }

    // Algorithm: search the actual path in the out m_aTreeChangeList
    // if we found something, we must merge/convert the Node with our Node
    // if we found nothing, we must create a new Node with our change
    // thats it.

    // the merge is contructed with helper classes because, it's possible that we
    // are a ValueChange but in the TreeChangeList this change is an AddNode, so
    // we have something to do.
    // New: For a value change we may also encounter
    void OMergeTreeChangeList::handle(ValueChange const& _rValueNode)
    {
        // Handle a ValueChange,
        OUString aNodeName = _rValueNode.getNodeName();

        if (Change *pChange = findExistingChange(m_pCurrentParent,aNodeName))
        {
            // Value found, merge content
            OMergeValueChange aMergeValue(*m_pCurrentParent,_rValueNode);
            aMergeValue.handleChange(*pChange);
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
        bool bReplacing = _rAddNode.isReplacing();

        OUString aNodeName = _rAddNode.getNodeName();

        if (Change *pChange = findExistingChange(m_pCurrentParent,aNodeName))
        {
            OSL_ENSURE(pChange->ISA(RemoveNode) || bReplacing, "OMergeTreeChangeList::handle(AddNode): the changes tree given already contains a change for this!");

            m_pCurrentParent->removeChange(pChange->getNodeName());

            bReplacing = true;
        }

        // insert manually
        data::TreeSegment aAddedTree = _rAddNode.getNewTree().cloneSegment();

        auto_ptr<AddNode> pNewAdd(new AddNode(aAddedTree, _rAddNode.getNodeName(), _rAddNode.isToDefault()));
        if (bReplacing)
            pNewAdd->setReplacing();

        auto_ptr<Change> pNewChange( pNewAdd.release() );
        m_pCurrentParent->addChange(pNewChange);
    }

    void OMergeTreeChangeList::handle(RemoveNode const& _rRemoveNode)
    {
        // Handle a RemoveNode
        OUString aNodeName = _rRemoveNode.getNodeName();

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
            OSL_ENSURE(sal_False, "OMergeTreeChangeList::handle(RemoveNode): don't know what to do with this!");
            // NO BREAK.
            // defaulting this so that the node will be marked as deleted
        case OMergeRemoveNode::FlagDeleted:
        {
            auto_ptr<Change> pNewChange(new RemoveNode(_rRemoveNode.getNodeName(),_rRemoveNode.isToDefault()));
            m_pCurrentParent->addChange(pNewChange);
        }
        break;
        }
    }
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    void OMergeTreeChangeList::handle(SubtreeChange const& _rSubtree)
    {
        // Handle a SubtreeChange
        // we must check if exact this SubtreeChange is in the TreeChangeList, if not,
        // we must add this SubtreeChange to the TreeChangeList
        // with the pointer m_pCurrentParent we remember our SubtreeChange in witch we
        // add all other Changes.

        OUString aNodeName = _rSubtree.getNodeName();

        Change *pChange = findExistingChange(m_pCurrentParent,aNodeName);

        // const sal_Char* pType = pChange ? pChange->getType() : NULL;
        SubtreeChange* pSubtreeChange = NULL;
        if (pChange == NULL || pChange->ISA(SubtreeChange))
        {
            // need to create a new Subtreechange
            if (!pChange)
            {
                // create a new SubtreeChange
                auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(_rSubtree, SubtreeChange::NoChildCopy()));
                pSubtreeChange = pNewChange.get();

                // add the new SubtreeChange in m_aTreeChangeList
                m_pCurrentParent->addChange(auto_ptr<Change>(pNewChange.release()));
                // check list for this new SubtreeChange
                OSL_ASSERT(pSubtreeChange == findExistingChange(m_pCurrentParent,aNodeName));
            }
            else // hard cast(!) to SubtreeChange because we are a SubtreeChange
            {
                pSubtreeChange = static_cast<SubtreeChange*>(pChange);
                adjustElementTemplate(*pSubtreeChange,_rSubtree);
            }

            // save this SubtreeChange so we allways have the last Subtree
            SubtreeChange* pSaveParent = pushTree(*pSubtreeChange);
            this->applyToChildren(_rSubtree);
            popTree( pSaveParent );
        }
        else if (pChange->ISA(AddNode))
        {
            // in this AddNode should be a subtree, NOT a subtreechange
            AddNode* pAddNode = static_cast<AddNode*>(pChange);

            if (_rSubtree.isToDefault())
            {
                std::auto_ptr<Change> aChangeToDefault(_rSubtree.clone());

                m_pCurrentParent->removeChange(pAddNode->getNodeName());
                m_pCurrentParent->addChange( aChangeToDefault );
            }
            else
            {

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
                std::auto_ptr<INode> pAddedNode = pAddNode->getNewTree().cloneData(false);

                ISubtree* pSubtree = pAddedNode.get() != NULL ? pAddedNode->asISubtree() : 0;
                if (pSubtree)
                {
                    // pSubtree = pSubtree + _rSubtree;
                    AbsolutePath aSubtreePath = this->createPath( ONameCreator::createName(_rSubtree,m_pCurrentParent) );
                    OSL_ASSERT( aSubtreePath.getLocalName().getName().toString() == pSubtree->getName() );

                    // Now apply _rSubtree to the subtree
                    TreeChangeList aChangeList(m_aTreeChangeList.getOptions(), aSubtreePath, _rSubtree, SubtreeChange::DeepChildCopy()); // expensive!
                    mergeLayer(aChangeList, *pSubtree);

                    // make a new subtree with the changed data
                    data::TreeSegment aNewTree = data::TreeSegment::createNew(pAddNode->getNodeName(), pAddedNode);

                    std::auto_ptr<AddNode> pNewAdd( new AddNode(aNewTree, pAddNode->getNodeName(), pAddNode->isToDefault()) );
                    if (pAddNode->isReplacing())
                        pNewAdd->setReplacing();

                    std::auto_ptr<Change> pNewChange( pNewAdd.release() );

                    m_pCurrentParent->removeChange(pAddNode->getNodeName());
                    m_pCurrentParent->addChange( pNewChange );
                }
                else
                {
                    OSL_ENSURE(false, "OMergeTreeChangeList: Unexpected node type found in an AddNode.");
                    /* wrong type of node found: böse ASSERTEN/WERFEN */;
                }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
            }
        }
        else
        {
            OSL_ENSURE(false, "OMergeTreeChangeList: Unexpected change type found for a subtree.");
            /* wrong type of node found: böse ASSERTEN/WERFEN */;
        }
    }

    // -----------------------------------------------------------------------------

    // CTor
    OMergeChanges::OMergeChanges(SubtreeChange& _rTree)
    : m_rSubtreeChange(_rTree), m_pCurrentParent(NULL)
    {
    }
    // -----------------------------------------------------------------------------

    void OMergeChanges::initRoot(const SubtreeChange &_rRootChange, const RelativePath& _aPathToChange)
    {
        SubtreeChange* pCurrentParent = &m_rSubtreeChange;

        if (!_aPathToChange.isEmpty())
        {
            OSL_PRECOND(_aPathToChange.getLocalName().getName().toString() == _rRootChange.getNodeName(),
                            "Path to change root does not match change being merged" );

            RelativePath::Iterator const firstEnsure = _aPathToChange.begin();
            RelativePath::Iterator lastEnsure = _aPathToChange.end();

            OSL_ASSERT( firstEnsure != lastEnsure );
            --lastEnsure; // last to ensure is the actual root

            for(RelativePath::Iterator it = firstEnsure; it != lastEnsure; ++it)
            {
                OSL_ASSERT( it != _aPathToChange.end() );

                Change *pChange = findExistingChange(pCurrentParent,*it);

                if (!pChange)
                {
                    OSL_ASSERT( it+1 != _aPathToChange.end());
                    Name const aElementTypeName = (it+1)->getTypeName();

                    // create a correspondens for the name, we did not find.
                    auto_ptr<SubtreeChange> pNewChange =
                        OTreeChangeFactory::createDummyChange(it->getName(), aElementTypeName);

                    pChange = pNewChange.get();

                    pCurrentParent->addChange(base_ptr(pNewChange));

                    OSL_ENSURE(pChange == findExistingChange(pCurrentParent,*it),
                                "ERROR: Newly added change cannot be found in parent change");
                }

                if (!pChange->ISA(SubtreeChange))
                {
                    OSL_ENSURE(false, "Change to merge does not point to a Subtree Change");
                    throw InvalidName(_aPathToChange.toString(), "points to a non- subtree change in this changes list, but a subtree change is required as root.");
                }
                pCurrentParent = static_cast<SubtreeChange*>( pChange);
            }

            Change *pRootChange = findExistingChange(pCurrentParent,*lastEnsure);

            if (!pRootChange)
            {
                // create a correspondens for the name, we did not find.
                auto_ptr<SubtreeChange> pNewChange(
                        new SubtreeChange(_rRootChange, SubtreeChange::NoChildCopy()) );

                pRootChange = pNewChange.get();

                pCurrentParent->addChange(base_ptr(pNewChange));

                OSL_ENSURE(pRootChange == findExistingChange(pCurrentParent,*it),
                            "ERROR: Newly added change cannot be found in parent change");
            }

            if (!pRootChange->ISA(SubtreeChange))
            {
                OSL_ENSURE(false, "Change to merge does not point to a Subtree Change");
                throw InvalidName(_aPathToChange.toString(), "points to a non-subtree change in this changes list, but a subtree change is required as root.");
            }
            pCurrentParent = static_cast<SubtreeChange*>( pRootChange);
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
        mergeChanges(_rChange, RelativePath());
    }
    // -----------------------------------------------------------------------------

    // start function, with the Change we want to do.
    // WARNING this could be a big tree, because a change can contain subtreechanges!
    void OMergeChanges::mergeChanges(const SubtreeChange &_rChange, const RelativePath& _aPathToChange)
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
        OUString aNodeName = _rValueNode.getNodeName();

        if (Change *pChange = findExistingChange(m_pCurrentParent,aNodeName))
        {
            // Value found, merge content
            OMergeValueChange aMergeValue(*m_pCurrentParent,_rValueNode);
            aMergeValue.handleChange(*pChange);
        }
        else
        {
            // there is no ValueChange in the List, insert new one
            auto_ptr<Change> pNewChange(new ValueChange(_rValueNode));
            m_pCurrentParent->addChange(pNewChange);
        }
    }
    // -----------------------------------------------------------------------------

    void OMergeChanges::handle(AddNode const& _rAddNode)
    {
        // Handle an AddNode
        bool bReplacing = _rAddNode.isReplacing();

        OUString aNodeName = _rAddNode.getNodeName();

        if (Change *pChange = findExistingChange(m_pCurrentParent,aNodeName))
        {
            OSL_ENSURE(pChange->ISA(RemoveNode) || bReplacing, "OMergeChanges::handle(AddNode): the changes tree given already contains a change for this!");

            m_pCurrentParent->removeChange(pChange->getNodeName());

            bReplacing = true;
        }

        // insert manually
        data::TreeSegment aAddedTree = _rAddNode.getNewTree().cloneSegment();

        auto_ptr<AddNode> pNewAdd(new AddNode(aAddedTree, _rAddNode.getNodeName(), _rAddNode.isToDefault()));
        if (bReplacing)
            pNewAdd->setReplacing();

        auto_ptr<Change> pNewChange( pNewAdd.release() );
        m_pCurrentParent->addChange(pNewChange);
    }
    // -----------------------------------------------------------------------------

    void OMergeChanges::handle(RemoveNode const& _rRemoveNode)
    {
        // Handle a RemoveNode
        OUString aNodeName = _rRemoveNode.getNodeName();

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
            auto_ptr<Change> pNewChange(new RemoveNode(_rRemoveNode.getNodeName(),_rRemoveNode.isToDefault()));
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
            data::TreeSegment::TreeDataPtr pAdded = _rAddNode.getNewTreeData();
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
        typedef SubtreeChange::MutatingChildIterator Iter;

        Iter it = m_rParent.begin_changes(), stop = m_rParent.end_changes();

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

        OUString aNodeName = _rSubtree.getNodeName();

        Change *pChange = findExistingChange(m_pCurrentParent,aNodeName);

        // const sal_Char* pType = pChange ? pChange->getType() : NULL;
        SubtreeChange* pSubtreeChange = NULL;
        if (pChange == NULL || pChange->ISA(SubtreeChange))
        {
            // need to create a new Subtreechange
            if (!pChange)
            {
                // create a new SubtreeChange
                auto_ptr<SubtreeChange> pNewChange(new SubtreeChange(_rSubtree, SubtreeChange::NoChildCopy()));
                pSubtreeChange = pNewChange.get();

                // add the new SubtreeChange in m_aTreeChangeList
                m_pCurrentParent->addChange(auto_ptr<Change>(pNewChange.release()));
                // check list for this new SubtreeChange
                OSL_ASSERT(pSubtreeChange == findExistingChange(m_pCurrentParent,aNodeName));
            }
            else // hard cast(!) to SubtreeChange because we are a SubtreeChange
            {
                pSubtreeChange = static_cast<SubtreeChange*>(pChange);
                adjustElementTemplate(*pSubtreeChange,_rSubtree);
            }

            // save this SubtreeChange so we allways have the last Subtree
            SubtreeChange* pSaveParent = pushTree(*pSubtreeChange);
            this->applyToChildren(_rSubtree);
            popTree( pSaveParent );
        }
        else if (pChange->ISA(AddNode))
        {
            AddNode* pAddNode = static_cast<AddNode*>(pChange);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
            std::auto_ptr<INode> pAddedNode = pAddNode->getNewTree().cloneData(false);
            ISubtree* pSubtree = pAddedNode.get() ? pAddedNode->asISubtree() : 0;
            if (pSubtree)
            {
                pSubtree->markAsDefault( _rSubtree.isToDefault() );

                // Now apply _rSubtree to the subtree
                TreeUpdater aTreeUpdate(pSubtree);
                aTreeUpdate.applyToChildren(_rSubtree);

                // make a new subtree with the changed data
                data::TreeSegment aNewTree = data::TreeSegment::createNew(pAddNode->getNodeName(), pAddedNode);

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

            auto_ptr<INode> pNode = auto_ptr<INode>(aAddNode.getNewTree().cloneData(true));

            m_pCurrentSubtree->addChild(pNode);

            OIdPropagator::propagateIdToChildren(*m_pCurrentSubtree);
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
