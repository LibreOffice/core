/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodeimplobj.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:45:12 $
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

#ifndef CONFIGMGR_NODEIMPLOBJECTS_HXX_
#define CONFIGMGR_NODEIMPLOBJECTS_HXX_

#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#include "setnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

// Specific types of nodes for direct or read only access
//-----------------------------------------------------------------------------

// Value Nodes
//-----------------------------------------------------------------------------

        class ValueMemberNode::DeferredImpl : public configmgr::SimpleReferenceObject
        {
            data::ValueNodeAddress m_aValueRef;

            UnoAny      m_aNewValue;
            bool        m_bToDefault;
            bool        m_bChange;
        public:
            explicit DeferredImpl(data::ValueNodeAccess const& _aValueNode);

            /// does this wrap a change
            bool isChange() const   { return m_bChange; }

            /// retrieve the underlying (original) node location
            data::ValueNodeAddress getOriginalNodeAddress() const
                { return m_aValueRef; }

            /// retrieve the underlying (original) node
            data::ValueNodeAccess getOriginalNode() const
        { return data::ValueNodeAccess(m_aValueRef); }

            /// Does this node change to default
            bool isToDefault()      const { return m_bToDefault; }

            /// retrieve the current value of this node
            UnoAny  getNewValue()   const { return m_aNewValue; }

            /// Set this node to a new value
            void    setValue(UnoAny const& aNewValue, data::ValueNodeAccess const& _aOriginalNode);

            /// Set this node to assume its default value
            void    setValueToDefault(data::ValueNodeAccess const& _aOriginalNode);

        public:
            // commit protocol
            std::auto_ptr<ValueChange> preCommitChange();
            void finishCommit(ValueChange& rChange);
            void revertCommit(ValueChange& rChange);
            void failedCommit(ValueChange& rChange);

            ValueChangeImpl* collectChange();
            ValueChangeImpl* adjustToChange(ValueChange const& rExternalChange);

            // notification protocol
            void adjustToChange(NodeChangesInformation& rLocalChange, ValueChange const& rExternalChange, TreeImpl& rParentTree, NodeOffset nParentPos, Name const& aName);
        };
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

// Group Nodes
//-----------------------------------------------------------------------------

        class DeferredGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            DeferredGroupNodeImpl(data::GroupNodeAddress const& _aNodeRef);
            explicit
            DeferredGroupNodeImpl(data::GroupNodeAddress const& _aNewAddress, GroupNodeImpl& rOriginal);

            ~DeferredGroupNodeImpl();

        public:
        // commit protocol
            std::auto_ptr<SubtreeChange> preCommitValueChanges();
            void finishCommit(SubtreeChange& rChange);
            void revertCommit(SubtreeChange& rChange);
            void failedCommit(SubtreeChange& rChange);

            void collectValueChanges(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;

        public:
        // data access
            bool hasChanges() const;
            void markChanged();

            typedef ValueMemberNode::DeferredImplRef MemberChange;
            MemberChange findValueChange(Name const& aName);

            using GroupNodeImpl::makeValueMember;
            ValueMemberNode makeValueMember(Name const& _aName, bool _bForUpdate);

        private:
            typedef std::map< Name, MemberChange > MemberChanges;

            MemberChanges    m_aChanges;
        };
//-----------------------------------------------------------------------------

// Set nodes
//-----------------------------------------------------------------------------

        class DeferredSetNodeImpl : public SetNodeImpl
        {
        public:
            explicit
            DeferredSetNodeImpl(data::SetNodeAddress const& _aNodeRef, Template* pTemplate);

        public:
            bool hasChanges() const;
            void markChanged();
            void collectElementChanges(NodeChanges& rChanges) const;

        public:
            std::auto_ptr<SubtreeChange> preCommitChanges(ElementList& _rRemovedElements);
            void failedCommit(SubtreeChange& rChanges);
            void finishCommit(SubtreeChange& rChanges);
            void revertCommit(SubtreeChange& rChanges);

            void insertNewElement(Name const& aName, Element const& aNewElement);
            void removeOldElement(Name const& aName);
        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual bool                   doIsEmpty() const;
            virtual ElementTreeImpl*       doFindElement(Name const& aName) ;
            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor);

            virtual void doDifferenceToDefaultState(SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree);

            virtual SetElementChangeImpl* doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const & aNewElement);
            virtual SetElementChangeImpl* doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange);

            virtual SetElementChangeImpl* doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange);

            virtual void doTransferElements(ElementSet& rReplacement);

        // Implementation
        private:
            void rebuildElement(Name const& aName, Element const& _aElement);

        private:
            ElementSet m_aChangedData;
            bool    m_bChanged;
            bool    m_bDefault;
        };
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_NODEIMPLOBJECTS_HXX_
