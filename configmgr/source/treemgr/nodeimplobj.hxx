/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodeimplobj.hxx,v $
 * $Revision: 1.16 $
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

#ifndef CONFIGMGR_NODEIMPLOBJECTS_HXX_
#define CONFIGMGR_NODEIMPLOBJECTS_HXX_

#include "sal/config.h"

#include "salhelper/simplereferenceobject.hxx"

#include "node.hxx"
#include "nodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "setnodeimpl.hxx"
#include "valuenodeimpl.hxx"
#include "utility.hxx"

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

        class ValueMemberNode::DeferredImpl : public salhelper::SimpleReferenceObject
        {
            sharable::ValueNode * m_valueNode;

            com::sun::star::uno::Any      m_aNewValue;
            bool        m_bToDefault;
            bool        m_bChange;
        public:
            explicit DeferredImpl(sharable::ValueNode * valueNode);

            /// does this wrap a change
            bool isChange() const   { return m_bChange; }

            /// retrieve the underlying (original) node
            sharable::ValueNode * getOriginalNode() const
            { return m_valueNode; }

            /// Does this node change to default
            bool isToDefault()      const { return m_bToDefault; }

            /// retrieve the current value of this node
            com::sun::star::uno::Any    getNewValue()   const { return m_aNewValue; }

            /// Set this node to a new value
            void    setValue(com::sun::star::uno::Any const& aNewValue, sharable::ValueNode * originalNode);

            /// Set this node to assume its default value
            void    setValueToDefault(sharable::ValueNode * originalNode);

        public:
            // commit protocol
            std::auto_ptr<ValueChange> preCommitChange();
            void finishCommit(ValueChange& rChange);
            void revertCommit(ValueChange& rChange);
            void failedCommit(ValueChange& rChange);

            ValueChangeImpl* collectChange();
            ValueChangeImpl* adjustToChange(ValueChange const& rExternalChange);

            // notification protocol
            void adjustToChange(NodeChangesInformation& rLocalChange, ValueChange const& rExternalChange, Tree& rParentTree, unsigned int nParentPos, rtl::OUString const& aName);
        };
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

// Group Nodes
//-----------------------------------------------------------------------------

        class DeferredGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            DeferredGroupNodeImpl(sharable::GroupNode * const& _aNodeRef);
            explicit
            DeferredGroupNodeImpl(sharable::GroupNode * const& _aNewAddress, GroupNodeImpl& rOriginal);

            ~DeferredGroupNodeImpl();

        public:
        // commit protocol
            std::auto_ptr<SubtreeChange> preCommitValueChanges();
            void finishCommit(SubtreeChange& rChange);
            void revertCommit(SubtreeChange& rChange);
            void failedCommit(SubtreeChange& rChange);

            void collectValueChanges(NodeChanges& rChanges, Tree* pParent, unsigned int nNode) const;

        public:
        // data access
            bool hasChanges() const;
            void markChanged();

            rtl::Reference<ValueMemberNode::DeferredImpl> findValueChange(rtl::OUString const& aName);

            using GroupNodeImpl::makeValueMember;
            ValueMemberNode makeValueMember(rtl::OUString const& _aName, bool _bForUpdate);

        private:
            typedef std::map< rtl::OUString, rtl::Reference<ValueMemberNode::DeferredImpl> > MemberChanges;

            MemberChanges    m_aChanges;
        };
//-----------------------------------------------------------------------------

// Set nodes
//-----------------------------------------------------------------------------

        class DeferredSetNodeImpl : public SetNodeImpl
        {
        public:
            explicit
            DeferredSetNodeImpl(sharable::SetNode * const& _aNodeRef, Template* pTemplate);

        public:
            bool hasChanges() const;
            void markChanged();
            void collectElementChanges(NodeChanges& rChanges) const;

        public:
            std::auto_ptr<SubtreeChange> preCommitChanges(std::vector< rtl::Reference<ElementTree> >& _rRemovedElements);
            void failedCommit(SubtreeChange& rChanges);
            void finishCommit(SubtreeChange& rChanges);
            void revertCommit(SubtreeChange& rChanges);

            void insertNewElement(rtl::OUString const& aName, ElementTreeData const& aNewElement);
            void removeOldElement(rtl::OUString const& aName);
        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual bool                   doIsEmpty() const;
            virtual ElementTree*       doFindElement(rtl::OUString const& aName) ;
            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor);

            virtual void doDifferenceToDefaultState(SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree);

            virtual SetElementChangeImpl* doAdjustToAddedElement(rtl::OUString const& aName, AddNode const& aAddNodeChange, ElementTreeData const & aNewElement);
            virtual SetElementChangeImpl* doAdjustToRemovedElement(rtl::OUString const& aName, RemoveNode const& aRemoveNodeChange);

            virtual SetElementChangeImpl* doAdjustChangedElement(NodeChangesInformation& rLocalChanges, rtl::OUString const& aName, Change const& aChange);

            virtual void doTransferElements(ElementSet& rReplacement);

        // Implementation
        private:
            void rebuildElement(rtl::OUString const& aName, ElementTreeData const& _aElement);

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
