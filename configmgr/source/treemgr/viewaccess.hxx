/*************************************************************************
 *
 *  $RCSfile: viewaccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:33:29 $
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

#ifndef CONFIGMGR_VIEWACCESS_HXX_
#define CONFIGMGR_VIEWACCESS_HXX_

#ifndef CONFIGMGR_VIEWNODE_HXX_
#include "viewnode.hxx"
#endif
#ifndef CONFIGMGR_VIEWBEHAVIOR_HXX_
#include "viewstrategy.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

//-----------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration { class NodeRef; }
//-----------------------------------------------------------------------------
    namespace view
    {
//-----------------------------------------------------------------------------

        class ViewTreeAccess
        {
            rtl::Reference< ViewStrategy >  m_xStrategy;
            Tree                            m_aTree;

        private:
            data::Accessor accessor() const { return m_aTree.accessor(); }

        public:
            explicit ViewTreeAccess(Tree const & _aTree);
            ViewTreeAccess(data::Accessor const & _aAccessor, configuration::TreeImpl& _rTree);

            rtl::Reference< view::ViewStrategy > getViewBehavior() { return m_xStrategy; }
        public:
            typedef configuration::NodeRef const& NodeArg;
            typedef configuration::NodeOffset NodeOffset;
            configuration::NodeData* nodeData(NodeArg    _aNodeArg) const;
            configuration::NodeData* nodeData(NodeOffset _aNodePos) const;
            bool isValid(NodeArg _aNodeArg) const;

            Node makeNode(NodeArg    _aNodeArg) const { return Node(m_aTree,nodeData(_aNodeArg)); }
            Node makeNode(NodeOffset _aNodePos) const { return Node(m_aTree,nodeData(_aNodePos)); }

            bool isSetNode  (NodeArg _aNodeArg) const { return makeNode(_aNodeArg).isSetNode(); }
            bool isGroupNode(NodeArg _aNodeArg) const { return makeNode(_aNodeArg).isGroupNode(); }
            bool isValueNode(NodeArg _aNodeArg) const { return makeNode(_aNodeArg).isValueNode(); }

            bool isSetNodeAt  (NodeOffset _aNodeArg) const { return makeNode(_aNodeArg).isSetNode(); }
            bool isGroupNodeAt(NodeOffset _aNodeArg) const { return makeNode(_aNodeArg).isGroupNode(); }
            bool isValueNodeAt(NodeOffset _aNodeArg) const { return makeNode(_aNodeArg).isValueNode(); }

            SetNode   toSetNode  (NodeArg _aNodeArg) const
            { return SetNode  (makeNode(_aNodeArg)); }

            GroupNode toGroupNode(NodeArg _aNodeArg) const
            { return GroupNode(makeNode(_aNodeArg)); }

            ValueNode toValueNode(NodeArg _aNodeArg) const
            { return ValueNode(makeNode(_aNodeArg)); }

            SetNode   getSetNodeAt  (NodeOffset _aNodeArg) const
            { return SetNode  (makeNode(_aNodeArg)); }

            GroupNode getGroupNodeAt(NodeOffset _aNodeArg) const
            { return GroupNode(makeNode(_aNodeArg)); }

            ValueNode getValueNodeAt(NodeOffset _aNodeArg) const
            { return ValueNode(makeNode(_aNodeArg)); }
        // node attributes
        public:
            /// retrieve the name of the node
            Name getName(NodeArg _aNode)    const
            { return m_xStrategy->getName(makeNode(_aNode)); }

            /// retrieve the attributes of the node
            node::Attributes getAttributes(NodeArg _aNode)  const
            { return m_xStrategy->getAttributes(makeNode(_aNode)); }

            /// retrieve the name of the tree root
            Name getRootName()  const
            { return m_xStrategy->getName( getRootNode(m_aTree) ); }

            /// retrieve the attributes of the tree root
            node::Attributes getRootAttributes()    const
            { return m_xStrategy->getAttributes( getRootNode(m_aTree) ); }

        // tracking pending changes
        public:
            void collectChanges(configuration::NodeChanges& rChanges)   const
            { m_xStrategy->collectChanges(m_aTree,rChanges); }

            bool hasChanges() const
            { return m_xStrategy->hasChanges(m_aTree); }

            bool hasChanges(NodeArg _aNode) const
            { return m_xStrategy->hasChanges(makeNode(_aNode)); }

            void markChanged(NodeArg _aNode)
            { m_xStrategy->markChanged(makeNode(_aNode)); }

        // commit protocol
        public:
            std::auto_ptr<SubtreeChange> preCommitChanges(configuration::ElementList& _rRemovedElements)
            { return m_xStrategy->preCommitChanges(m_aTree,_rRemovedElements); }

            void finishCommit(SubtreeChange& rRootChange)
            { m_xStrategy->finishCommit(m_aTree,rRootChange); }

            void revertCommit(SubtreeChange& rRootChange)
            { m_xStrategy->revertCommit(m_aTree,rRootChange); }

            void recoverFailedCommit(SubtreeChange& rRootChange)
            { m_xStrategy->recoverFailedCommit(m_aTree,rRootChange); }

        // notification protocol
        public:
            /// Adjust the internal representation after external changes to the original data - build NodeChangeInformation objects for notification
            void    adjustToChanges(configuration::NodeChangesInformation& rLocalChanges, NodeArg _aNode, SubtreeChange const& aExternalChange)
            { m_xStrategy->adjustToChanges(rLocalChanges,makeNode(_aNode), aExternalChange); }

        // visitor dispatch
        public:
            configuration::GroupMemberVisitor::Result dispatchToValues(GroupNode const& _aNode, configuration::GroupMemberVisitor& _aVisitor) const
            { return m_xStrategy->dispatchToValues(_aNode,_aVisitor); }

            /// Call <code>aVisitor.visit(aElement)</code> for each element in this set until SetNodeVisitor::DONE is returned.
            configuration::SetNodeVisitor::Result dispatchToElements(SetNode const& _aNode, configuration::SetNodeVisitor& _aVisitor) const
            { return m_xStrategy->dispatchToElements(_aNode,_aVisitor); }

        // value (element) node specific operations
        public:
            /// Does this node assume its default value
            /// retrieve the current value of this node
            UnoAny  getValue(ValueNode const& _aNode) const
            { return m_xStrategy->getValue(_aNode); }

            /// get the type of this value
            UnoType getValueType(ValueNode const& _aNode)   const
            { return m_xStrategy->getValueType(_aNode); }


        // group node specific operations
        public:
            /// does this hold a child value of the given name
            bool hasValue(GroupNode const& _aNode, Name const& _aName) const
            { return m_xStrategy->hasValue(_aNode,_aName); }

            /// does this hold a child value
            bool hasValue(GroupNode const& _aNode) const
            { return m_xStrategy->hasValue(_aNode); }

            /// are defaults for this node available ?
            bool areValueDefaultsAvailable(GroupNode const& _aNode) const
            { return m_xStrategy->areValueDefaultsAvailable(_aNode); }

            /// retrieve data for the child value of the given name
            configuration::ValueMemberNode getValue(GroupNode const& _aNode, Name const& _aName) const
            { return m_xStrategy->getValue(_aNode,_aName); }

            /// retrieve data for updating the child value of the given name
            configuration::ValueMemberUpdate getValueForUpdate(GroupNode const & _aNode, Name const& _aName) const
            { return m_xStrategy->getValueForUpdate(_aNode,_aName); }

        // set node specific operations
        public:
            /// does this set contain any elements (loads elements if needed)
            bool isEmpty(SetNode const& _aNode) const
            { return m_xStrategy->isEmpty(_aNode); }

            /// does this set contain an element named <var>aName</var> (loads elements if needed)
            configuration::SetEntry findElement(SetNode const& _aNode, Name const& aName) const
            { return m_xStrategy->findElement(_aNode,aName); }

            /// does this set contain an element named <var>aName</var> (and is that element loaded ?)
            configuration::SetEntry findAvailableElement(SetNode const& _aNode, Name const& aName) const
            { return m_xStrategy->findAvailableElement(_aNode,aName); }

            /// insert a new entry into this set
            void        insertElement(SetNode const& _aNode, Name const& aName, configuration::SetEntry const& aNewEntry)
            { m_xStrategy->insertElement(_aNode,aName,aNewEntry); }

            /// remove an existing entry into this set
            void        removeElement(SetNode const& _aNode, Name const& aName)
            { m_xStrategy->removeElement(_aNode,aName); }

            /** Create a Subtree change as 'diff' which allows transforming the set to its default state
                (given that <var>_rDefaultTree</var> points to a default instance of this set)
                <p>Ownership of added trees should be transferred to the SubtreeChange.</p>
            */
            std::auto_ptr<SubtreeChange> differenceToDefaultState(SetNode const& _aNode, ISubtree& _rDefaultTree)
            { return m_xStrategy->differenceToDefaultState(_aNode,_rDefaultTree); }

            /// Get the template that describes elements of this set
            configuration::TemplateHolder getElementTemplate(SetNode const& _aNode) const
            { return m_xStrategy->getElementTemplate(_aNode); }

            /// Get a template provider that can create new elements for this set
            configuration::TemplateProvider getTemplateProvider(SetNode const& _aNode) const
            { return m_xStrategy->getTemplateProvider(_aNode); }

        // changing state/strategy
        public:
            // replace m_xStrategy by a direct ViewStrategy (commiting changes to the data), if possible
         //   void makeDirect  ();

        };

//-----------------------------------------------------------------------------
        inline
        ViewTreeAccess::ViewTreeAccess(Tree const & _aTree)
        : m_xStrategy()
        , m_aTree(_aTree)
        {
            m_xStrategy = _aTree.get_impl()->getViewBehavior();
        }

//-----------------------------------------------------------------------------
        inline
        ViewTreeAccess::ViewTreeAccess(data::Accessor const & _aAccessor, configuration::TreeImpl& _rTree)
        : m_xStrategy(_rTree.getViewBehavior())
        , m_aTree(_aAccessor,_rTree)
        {
        }

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_VIEWACCESS_HXX_
