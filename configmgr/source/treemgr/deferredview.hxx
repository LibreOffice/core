/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: deferredview.hxx,v $
 * $Revision: 1.5 $
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

#ifndef CONFIGMGR_DEFERREDVIEW_HXX_
#define CONFIGMGR_DEFERREDVIEW_HXX_

#include "viewstrategy.hxx"


namespace configmgr
{
    namespace view
    {
//-----------------------------------------------------------------------------
// View behavior for direct data access
//-----------------------------------------------------------------------------

        class DeferredViewStrategy : public ViewStrategy
        {
        public:
            explicit
            DeferredViewStrategy() {}

        // ViewStrategy implementation
        private:
            // change handling -required
            virtual bool doHasChanges(Node const& _aNode) const;
            virtual void doMarkChanged(Node const& _aNode);


            // change handling
            virtual void doCollectChanges(Node const& _aNode, configuration::NodeChanges& rChanges) const;

            // commit protocol
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges(Tree const& _aTree, configuration::ElementList& _rRemovedElements);
            virtual void doFailedCommit(Tree const& _aTree, SubtreeChange& rChanges);
            virtual void doFinishCommit(Tree const& _aTree, SubtreeChange& rChanges);
            virtual void doRevertCommit(Tree const& _aTree, SubtreeChange& rChanges);

            // notification protocol
            virtual configuration::ValueChangeImpl* doAdjustToValueChange(GroupNode const& _aGroupNode, Name const& aName, ValueChange const& rExternalChange);
        //  virtual void doAdjustToElementChanges(configuration::NodeChangesInformation& rLocalChanges, SetNode const& _aNode, SubtreeChange const& rExternalChanges, TreeDepth nDepth);

            // common attributes
            virtual node::Attributes doAdjustAttributes(node::Attributes const& _aAttributes) const;

            // group member access
            virtual configuration::ValueMemberNode doGetValueMember(GroupNode const& _aNode, Name const& _aName, bool _bForUpdate) const;

            // set element access
            virtual void doInsertElement(SetNode const& _aNode, Name const& aName, configuration::SetEntry const& aNewEntry);
            virtual void doRemoveElement(SetNode const& _aNode, Name const& aName);

            virtual NodeFactory& doGetNodeFactory();
        private:
            void implCollectChangesIn(Node const& _aNode, NodeChanges& rChanges) const;
            // commit protocol
            std::auto_ptr<SubtreeChange> implPreCommitChanges(Node const& _aNode, configuration::ElementList& _rRemovedElements);
            void implFailedCommit(Node const& _aNode, SubtreeChange& rChanges);
            void implFinishCommit(Node const& _aNode, SubtreeChange& rChanges);
            void implRevertCommit(Node const& _aNode, SubtreeChange& rChanges);

            void implPreCommitSubChanges(GroupNode const & _aGroup, configuration::ElementList& _rRemovedElements, SubtreeChange& _rParentChange);
            void implFailedSubCommitted(GroupNode const & _aGroup, SubtreeChange& rChanges);
            void implFinishSubCommitted(GroupNode const & _aGroup, SubtreeChange& rChanges);
            void implRevertSubCommitted(GroupNode const & _aGroup, SubtreeChange& rChanges);

        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_DEFERREDVIEW_HXX_
