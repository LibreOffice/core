/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewstrategy.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_VIEWBEHAVIOR_HXX_
#define CONFIGMGR_VIEWBEHAVIOR_HXX_

#include "viewnode.hxx"
#include "groupnodeimpl.hxx"
#include "setnodeimpl.hxx"
#include "utility.hxx"
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
        class SetElementChangeImpl;
        class ValueChangeImpl;
    }
//-----------------------------------------------------------------------------
    namespace view
    {
//-----------------------------------------------------------------------------
        struct NodeFactory;
//-----------------------------------------------------------------------------
        class ViewStrategy : public salhelper::SimpleReferenceObject
        {
        // node attributes
        public:
            /// retrieve the attributes of the node
            rtl::OUString getName(Node const& _aNode)   const;

            /// retrieve the attributes of the node
            node::Attributes getAttributes(Node const& _aNode)  const;

        // tracking pending changes
        public:
            void collectChanges(configuration::Tree * tree, configuration::NodeChanges& rChanges) const;

            bool hasChanges(configuration::Tree * tree) const;

            bool hasChanges(Node const& _aNode) const;

            void markChanged(Node const& _aNode);

        // commit protocol
        public:
            std::auto_ptr<SubtreeChange> preCommitChanges(configuration::Tree * tree, std::vector< rtl::Reference<configuration::ElementTree> >& _rRemovedElements);

            void finishCommit(configuration::Tree * tree, SubtreeChange& rRootChange);

            void revertCommit(configuration::Tree * tree, SubtreeChange& rRootChange);

            void recoverFailedCommit(configuration::Tree * tree, SubtreeChange& rRootChange);

        // notification protocol
        public:
            /// Adjust the internal representation after external changes to the original data - build NodeChangeInformation objects for notification
            void    adjustToChanges(configuration::NodeChangesInformation& rLocalChanges, Node const & _aNode, SubtreeChange const& aExternalChange);

        // visitor dispatch
        public:
            configuration::GroupMemberVisitor::Result dispatchToValues(GroupNode const& _aNode, configuration::GroupMemberVisitor& _aVisitor);

            /// Call <code>aVisitor.visit(aElement)</code> for each element in this set until SetNodeVisitor::DONE is returned.
            configuration::SetNodeVisitor::Result dispatchToElements(SetNode const& _aNode, configuration::SetNodeVisitor& _aVisitor);

        // value (element) node specific operations
        public:
            /// Does this node assume its default value
            /// retrieve the current value of this node
            com::sun::star::uno::Any    getValue(ValueNode const& _aNode) const;
#if OSL_DEBUG_LEVEL > 0
            /// get the type of this value
            com::sun::star::uno::Type   getValueType(ValueNode const& _aNode)   const;
#endif

        // group node specific operations
        public:
            /// does this hold a child value of the given name
            bool hasValue(GroupNode const& _aNode, rtl::OUString const& _aName) const;

            /// does this hold a child value
            bool hasValue(GroupNode const& _aNode) const;

            /// are defaults for this node available ?
            bool areValueDefaultsAvailable(GroupNode const& _aNode) const;

            /// retrieve data for the child value of the given name
            configuration::ValueMemberNode getValue(GroupNode const& _aNode, rtl::OUString const& _aName) const;

            /// retrieve data for updating the child value of the given name
            configuration::ValueMemberUpdate getValueForUpdate(GroupNode const & _aNode, rtl::OUString const& _aName);

        // set node specific operations
        public:
            /// does this set contain any elements (loads elements if needed)
            bool isEmpty(SetNode const& _aNode) const;

            /// does this set contain an element named <var>aName</var> (loads elements if needed)
            configuration::SetEntry findElement(SetNode const& _aNode, rtl::OUString const& aName) const;

            /// does this set contain an element named <var>aName</var> (and is that element loaded ?)
            configuration::SetEntry findAvailableElement(SetNode const& _aNode, rtl::OUString const& aName) const;

            /// insert a new entry into this set
            void        insertElement(SetNode const& _aNode, rtl::OUString const& aName, configuration::SetEntry const& aNewEntry);

            /// remove an existing entry into this set
            void        removeElement(SetNode const& _aNode, rtl::OUString const& aName);

            /** Create a Subtree change as 'diff' which allows transforming the set to its default state
                (given that <var>_rDefaultTree</var> points to a default instance of this set)
                <p>Ownership of added trees should be transferred to the SubtreeChange.</p>
            */
            std::auto_ptr<SubtreeChange> differenceToDefaultState(SetNode const& _aNode, ISubtree& _rDefaultTree) const;

            /// Get the template that describes elements of this set
            rtl::Reference<configuration::Template> getElementTemplate(SetNode const& _aNode) const;

            /// Get a template provider that can create new elements for this set
            configuration::TemplateProvider getTemplateProvider(SetNode const& _aNode) const;

            // create a configuration::Tree * from a configuration::SetEntry
            configuration::Tree * extractTree(configuration::SetEntry const& _anEntry);

        // creating/changing state/strategy
        public:
            NodeFactory& getNodeFactory();

        // access to node innards
        protected:
            /// provide access to the address of the underlying node
            sharable::Node * getNodeAddress(Node const& _aNode) const;

            /// retrieve the attributes of the underlying node
            node::Attributes getNodeAttributes(Node const& _aNode) const;

        protected:
            //helper for migration to new (info based) model for adjusting to changes
            static void addLocalChangeHelper( configuration::NodeChangesInformation& rLocalChanges, configuration::NodeChange const& aChange);

        private:
            void implAdjustToValueChanges(configuration::NodeChangesInformation& rLocalChanges, GroupNode const& _aGroupNode, SubtreeChange const& rExternalChanges);
            void implAdjustToSubChanges(configuration::NodeChangesInformation& rLocalChanges, GroupNode const & _aGroupNode, SubtreeChange const& rExternalChanges);
            void implAdjustToElementChanges(configuration::NodeChangesInformation& rLocalChanges, SetNode const& _aNode, SubtreeChange const& rExternalChanges, unsigned int nDepth);
            void implAdjustToElementChange (configuration::NodeChangesInformation& rLocalChanges, SetNode const& _aNode, Change const& rElementChange, unsigned int nElementDepth);
            void implCommitDirectIn(sharable::TreeFragment * placeHolder, Node const& _aNode);

        protected:
            void            checkInstance(configuration::Tree * tree) const;
            configuration::SetEntry    implFindElement(SetNode const& _aNode, rtl::OUString const& aName) const;
            configuration::ElementTreeData  implMakeElement(SetNode const& _aNode, configuration::SetEntry const& anEntry) const;

        // virtual interface - these functions must be provided
        private:
            // change handling
            virtual bool doHasChanges(Node const& _aNode) const     = 0;
            virtual void doMarkChanged(Node const& _aNode)          = 0;

            virtual NodeFactory& doGetNodeFactory()  = 0;

        // virtual interface - these functions all have default implementations without support for pending changes
        protected:
            // change handling
            virtual void doCollectChanges(Node const& _aNode, configuration::NodeChanges& rChanges) const;

            // commit protocol
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges(configuration::Tree * tree, std::vector< rtl::Reference<configuration::ElementTree> >& _rRemovedElements);
            virtual void doFailedCommit(configuration::Tree * tree, SubtreeChange& rChanges);
            virtual void doFinishCommit(configuration::Tree * tree, SubtreeChange& rChanges);
            virtual void doRevertCommit(configuration::Tree * tree, SubtreeChange& rChanges);

            // notification protocol
            virtual configuration::ValueChangeImpl* doAdjustToValueChange(GroupNode const& _aGroupNode, rtl::OUString const& aName, ValueChange const& rExternalChange);

            // common attributes
            virtual node::Attributes doAdjustAttributes(node::Attributes const& _aAttributes) const = 0;

            // group member access
            virtual configuration::ValueMemberNode doGetValueMember(GroupNode const& _aNode, rtl::OUString const& _aName, bool _bForUpdate) const = 0;

            // set element access
            virtual void doInsertElement(SetNode const& _aNode, rtl::OUString const& aName, configuration::SetEntry const& aNewEntry) = 0;
            virtual void doRemoveElement(SetNode const& _aNode, rtl::OUString const& aName) = 0;
        };

        inline node::Attributes ViewStrategy::getAttributes(Node const& _aNode) const
        { return doAdjustAttributes(getNodeAttributes(_aNode)); }

        inline bool ViewStrategy::hasChanges(Node const& _aNode) const
        { return doHasChanges(_aNode); }

        inline NodeFactory& ViewStrategy::getNodeFactory()
        { return doGetNodeFactory(); }

//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
