/*************************************************************************
 *
 *  $RCSfile: viewstrategy.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 14:01:26 $
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

#ifndef CONFIGMGR_VIEWBEHAVIOR_HXX_
#define CONFIGMGR_VIEWBEHAVIOR_HXX_

#ifndef CONFIGMGR_VIEWNODE_HXX_
#include "viewnode.hxx"
#endif

#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#include "setnodeimpl.hxx"
#endif

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace memory { class Segment; }
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
        using configuration::Name;
        using configuration::NodeOffset;
        using configuration::TreeDepth;

        typedef com::sun::star::uno::Any    UnoAny;
        typedef com::sun::star::uno::Type   UnoType;
//-----------------------------------------------------------------------------
        class ViewStrategy : public salhelper::SimpleReferenceObject
        {
        // node attributes
        public:
            /// retrieve the attributes of the node
            Name getName(Node const& _aNode)    const;

            /// retrieve the attributes of the node
            node::Attributes getAttributes(Node const& _aNode)  const;

        // tracking pending changes
        public:
            typedef configuration::NodeChanges NodeChanges;

            void collectChanges(Tree const& _aTree, NodeChanges& rChanges) const;

            bool hasChanges(Tree const& _aTree) const;

            bool hasChanges(Node const& _aNode) const;

            void markChanged(Node const& _aNode);

        // commit protocol
        public:
            std::auto_ptr<SubtreeChange> preCommitChanges(Tree const& _aTree, configuration::ElementList& _rRemovedElements);

            void finishCommit(Tree const& _aTree, SubtreeChange& rRootChange);

            void revertCommit(Tree const& _aTree, SubtreeChange& rRootChange);

            void recoverFailedCommit(Tree const& _aTree, SubtreeChange& rRootChange);

        // notification protocol
        public:
            typedef configuration::NodeChangesInformation NodeChangesInformation;

            /// Adjust the internal representation after external changes to the original data - build NodeChangeInformation objects for notification
            void    adjustToChanges(NodeChangesInformation& rLocalChanges, Node const & _aNode, SubtreeChange const& aExternalChange);

        // visitor dispatch
        public:
            typedef configuration::GroupMemberVisitor GroupMemberVisitor;
            typedef configuration::SetNodeVisitor SetNodeVisitor;

            GroupMemberVisitor::Result dispatchToValues(GroupNode const& _aNode, GroupMemberVisitor& _aVisitor);

            /// Call <code>aVisitor.visit(aElement)</code> for each element in this set until SetNodeVisitor::DONE is returned.
            SetNodeVisitor::Result dispatchToElements(SetNode const& _aNode, SetNodeVisitor& _aVisitor);

        // value (element) node specific operations
        public:
            /// Does this node assume its default value
            /// retrieve the current value of this node
            UnoAny  getValue(ValueNode const& _aNode) const;

            /// get the type of this value
            UnoType getValueType(ValueNode const& _aNode)   const;


        // group node specific operations
        public:
            typedef configuration::ValueMemberNode   ValueMemberNode;
            typedef configuration::ValueMemberUpdate ValueMemberUpdate;

            /// does this hold a child value of the given name
            bool hasValue(GroupNode const& _aNode, Name const& _aName) const;

            /// does this hold a child value
            bool hasValue(GroupNode const& _aNode) const;

            /// are defaults for this node available ?
            bool areValueDefaultsAvailable(GroupNode const& _aNode) const;

            /// retrieve data for the child value of the given name
            ValueMemberNode getValue(GroupNode const& _aNode, Name const& _aName) const;

            /// retrieve data for updating the child value of the given name
            ValueMemberUpdate getValueForUpdate(GroupNode const & _aNode, Name const& _aName);

        // set node specific operations
        public:
            typedef configuration::ElementTreeData  SetNodeElement;
            typedef configuration::SetEntry         SetNodeEntry;

            /// does this set contain any elements (loads elements if needed)
            bool isEmpty(SetNode const& _aNode) const;

            /// does this set contain an element named <var>aName</var> (loads elements if needed)
            SetNodeEntry    findElement(SetNode const& _aNode, Name const& aName) const;

            /// does this set contain an element named <var>aName</var> (and is that element loaded ?)
            SetNodeEntry    findAvailableElement(SetNode const& _aNode, Name const& aName) const;

            /// insert a new entry into this set
            void        insertElement(SetNode const& _aNode, Name const& aName, SetNodeEntry const& aNewEntry);

            /// remove an existing entry into this set
            void        removeElement(SetNode const& _aNode, Name const& aName);

            /** Create a Subtree change as 'diff' which allows transforming the set to its default state
                (given that <var>_rDefaultTree</var> points to a default instance of this set)
                <p>Ownership of added trees should be transferred to the SubtreeChange.</p>
            */
            std::auto_ptr<SubtreeChange> differenceToDefaultState(SetNode const& _aNode, ISubtree& _rDefaultTree) const;

            /// Get the template that describes elements of this set
            configuration::TemplateHolder getElementTemplate(SetNode const& _aNode) const;

            /// Get a template provider that can create new elements for this set
            configuration::TemplateProvider getTemplateProvider(SetNode const& _aNode) const;

            // create a view::Tree from a configuration::SetEntry
            Tree extractTree(SetNodeEntry const& _anEntry);

        // creating/changing state/strategy
        public:
            NodeFactory& getNodeFactory();

        // direct update access to data
        public:
            void releaseDataSegment();

            memory::Segment const * getDataSegment() const;
            memory::Segment  * getDataSegmentForUpdate();

            data::NodeAddress   ::DataType * getDataForUpdate(data::NodeAccessRef const & _aNode);
            data::SetNodeAddress::DataType * getDataForUpdate(data::SetNodeAccess const & _aNode);
            data::GroupNodeAddress::DataType * getDataForUpdate(data::GroupNodeAccess const & _aNode);
            data::ValueNodeAddress::DataType * getDataForUpdate(data::ValueNodeAccess const & _aNode);

        // access to node innards
        protected:
            /// provide access to the data of the underlying node
            data::NodeAccessRef getNodeAccessRef(Node const& _aNode) const;

            /// provide access to the address of the underlying node
            data::NodeAddress getNodeAddress(Node const& _aNode) const;

            /// retrieve the name of the underlying node
            Name getNodeName(Node const& _aNode) const;

            /// retrieve the attributes of the underlying node
            node::Attributes getNodeAttributes(Node const& _aNode) const;

        protected:
            //helper for migration to new (info based) model for adjusting to changes
            static void addLocalChangeHelper( NodeChangesInformation& rLocalChanges, configuration::NodeChange const& aChange);

        private:
            void implAdjustToValueChanges(NodeChangesInformation& rLocalChanges, GroupNode const& _aGroupNode, SubtreeChange const& rExternalChanges);
            void implAdjustToSubChanges(NodeChangesInformation& rLocalChanges, GroupNode const & _aGroupNode, SubtreeChange const& rExternalChanges);
            void implAdjustToElementChanges(NodeChangesInformation& rLocalChanges, SetNode const& _aNode, SubtreeChange const& rExternalChanges, TreeDepth nDepth);
            void implAdjustToElementChange (NodeChangesInformation& rLocalChanges, SetNode const& _aNode, Change const& rElementChange, TreeDepth nElementDepth);
            void implCommitDirectIn(data::TreeAccessor const& _aPlaceHolder, Node const& _aNode);

        protected:
            void            checkInstance(Tree const& _aTreeForThis) const;
            SetNodeEntry    implFindElement(SetNode const& _aNode, Name const& aName) const;
            SetNodeElement  implMakeElement(SetNode const& _aNode, SetNodeEntry const& anEntry) const;

        // virtual interface - these functions must be provided
        private:
            // change handling
            virtual bool doHasChanges(Node const& _aNode) const     = 0;
            virtual void doMarkChanged(Node const& _aNode)          = 0;

            virtual NodeFactory& doGetNodeFactory()  = 0;

        // virtual interface - these functions all have default implementations without support for pending changes
        protected:
            virtual void doReleaseDataSegment() = 0;

            // special support for direct changes to underlying data - default is no support
            virtual data::NodeAddress::DataType * implAccessForUpdate(data::NodeAccessRef const & _aDataAccess);
            virtual memory::Segment const * doGetDataSegment() const = 0;
            virtual memory::Segment       * doGetDataSegmentForUpdate();

            // change handling
            virtual void doCollectChanges(Node const& _aNode, NodeChanges& rChanges) const;

            // commit protocol
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges(Tree const& _aTree, configuration::ElementList& _rRemovedElements);
            virtual void doFailedCommit(Tree const& _aTree, SubtreeChange& rChanges);
            virtual void doFinishCommit(Tree const& _aTree, SubtreeChange& rChanges);
            virtual void doRevertCommit(Tree const& _aTree, SubtreeChange& rChanges);

            // notification protocol
            virtual configuration::ValueChangeImpl* doAdjustToValueChange(GroupNode const& _aGroupNode, Name const& aName, ValueChange const& rExternalChange);

            // common attributes
            virtual node::Attributes doAdjustAttributes(node::Attributes const& _aAttributes) const = 0;

            // group member access
            virtual ValueMemberNode doGetValueMember(GroupNode const& _aNode, Name const& _aName, bool _bForUpdate) const = 0;

            // set element access
            virtual void doInsertElement(SetNode const& _aNode, Name const& aName, SetNodeEntry const& aNewEntry) = 0;
            virtual void doRemoveElement(SetNode const& _aNode, Name const& aName) = 0;

            // strategy change support
/*          virtual void doCommitChanges(Node const& _aNode);
            virtual rtl::Reference<ViewStrategy> doCloneDirect() = 0;
            virtual rtl::Reference<ViewStrategy> doCloneIndirect() = 0;
*/      };

//-----------------------------------------------------------------------------
        inline Name ViewStrategy::getName(Node const& _aNode)   const
        { return getNodeName(_aNode); }

        inline node::Attributes ViewStrategy::getAttributes(Node const& _aNode) const
        { return doAdjustAttributes(getNodeAttributes(_aNode)); }

        inline bool ViewStrategy::hasChanges(Node const& _aNode) const
        { return doHasChanges(_aNode); }

        inline NodeFactory& ViewStrategy::getNodeFactory()
        { return doGetNodeFactory(); }

        inline void ViewStrategy::releaseDataSegment()
        { doReleaseDataSegment(); }

        inline memory::Segment const * ViewStrategy::getDataSegment()   const
        { return doGetDataSegment(); }

        inline memory::Segment       * ViewStrategy::getDataSegmentForUpdate()
        { return doGetDataSegmentForUpdate(); }

//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
