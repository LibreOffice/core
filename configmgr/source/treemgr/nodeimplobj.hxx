/*************************************************************************
 *
 *  $RCSfile: nodeimplobj.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-20 11:01:51 $
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

#ifndef CONFIGMGR_NODEIMPLOBJECTS_HXX_
#define CONFIGMGR_NODEIMPLOBJECTS_HXX_

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_SETNODEIMPL_HXX_
#include "setnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
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

        class ReadOnlyValueElementNodeImpl : public ValueElementNodeImpl
        {
        public:
            explicit
            ReadOnlyValueElementNodeImpl(ValueNode& rOriginal)
            : ValueElementNodeImpl(rOriginal)
            {}

        protected:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredValueElementNodeImpl;

        class DirectValueElementNodeImpl : public ValueElementNodeImpl
        {
        public:
            explicit
            DirectValueElementNodeImpl(ValueNode& rOriginal);

            explicit
            DirectValueElementNodeImpl(DeferredValueElementNodeImpl& rOriginal);

        protected:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class DeferredValueElementNodeImpl : public ValueElementNodeImpl
        {
        public:
            explicit
            DeferredValueElementNodeImpl(ValueNode& rOriginal);
            explicit
            DeferredValueElementNodeImpl(DirectValueElementNodeImpl& rOriginal);

        protected:
        // NodeImpl implementation
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class ValueMemberNode::DeferredImpl : public vos::OReference
        {
            ValueNode& m_rOriginal;

            UnoAny      m_aNewValue;
            bool        m_bToDefault;
            bool        m_bChange;
        public:
            explicit DeferredImpl(ValueNode& rOriginal) ;

            /// does this wrap a change
            bool isChange() const   { return m_bChange; }

            /// retrieve the underlying (original) node
            ValueNode& getOriginalNode() const { return m_rOriginal; }

            /// Does this node change to default
            bool isToDefault()      const { return m_bToDefault; }

            /// retrieve the current value of this node
            UnoAny  getNewValue()   const { return m_aNewValue; }

            /// Set this node to a new value
            void    setValue(UnoAny const& aNewValue);

            /// Set this node to assume its default value
            void    setValueToDefault();

        public:
            // commit protocol
            std::auto_ptr<ValueChange> preCommitChange();
            void finishCommit(ValueChange& rChange);
            void revertCommit(ValueChange& rChange);
            void failedCommit(ValueChange& rChange);

            void commitDirect();

            ValueChangeImpl* collectChange();
            ValueChangeImpl* adjustToChange(ValueChange const& rExternalChange);

            // notification protocol
            void adjustToChange(NodeChangesInformation& rLocalChange, ValueChange const& rExternalChange, TreeImpl& rParentTree, NodeOffset nParentPos, Name const& aName);
        };
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

// Group Nodes
//-----------------------------------------------------------------------------

        class ReadOnlyGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            ReadOnlyGroupNodeImpl(ISubtree& rOriginal)
            : GroupNodeImpl(rOriginal)
            {}

        // Base obverrideables
        private:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;
            virtual ValueMemberNode doGetValueMember(Name const& aName, bool bForUpdate);

            virtual bool doHasChanges() const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredGroupNodeImpl;

        class DirectGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            DirectGroupNodeImpl(ISubtree& rOriginal);

            explicit
            DirectGroupNodeImpl(DeferredGroupNodeImpl& rOriginal);

        // Base obverrideables
        private:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;
            virtual ValueMemberNode doGetValueMember(Name const& aName, bool bForUpdate);

            virtual bool doHasChanges() const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class DeferredGroupNodeImpl : public GroupNodeImpl
        {
        public:
            explicit
            DeferredGroupNodeImpl(ISubtree& rOriginal);
            explicit
            DeferredGroupNodeImpl(DirectGroupNodeImpl& rOriginal);

            ~DeferredGroupNodeImpl();

        protected:
        // commit protocol
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges();
            virtual void doFinishCommit(SubtreeChange& rChange);
            virtual void doRevertCommit(SubtreeChange& rChange);
            virtual void doFailedCommit(SubtreeChange& rChange);

        // notification protocol
            virtual ValueChangeImpl* doAdjustToValueChange(Name const& aName, ValueChange const& rExternalChange);

        // Base obverrideables
        private:
        // NodeImpl implementation
            virtual ValueMemberNode doGetValueMember(Name const& aName, bool bForUpdate);

            virtual bool doHasChanges() const;
            virtual void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);

        private:
            typedef ValueMemberNode::DeferredImplRef DeferredValueImplRef;
            typedef std::map< Name, DeferredValueImplRef > ValueChanges;

            ValueChanges    m_aChanges;
        };
//-----------------------------------------------------------------------------

// Set nodes
//-----------------------------------------------------------------------------

        class ReadOnlyTreeSetNodeImpl : public TreeSetNodeImpl
        {
        public:
            explicit
            ReadOnlyTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
            : TreeSetNodeImpl(rOriginal,pTemplate)
            {}


        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual void doInsertElement(Name const& aName, SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements( ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class ReadOnlyValueSetNodeImpl : public ValueSetNodeImpl
        {
        public:
            explicit
            ReadOnlyValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
            : ValueSetNodeImpl(rOriginal,pTemplate)
            {}


        // Base Overrideables
        private:
            virtual void doInsertElement(Name const& aName,  SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth);

        // NodeImpl implementation
            virtual Attributes doGetAttributes() const;

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredTreeSetNodeImpl;

        class DirectTreeSetNodeImpl : public TreeSetNodeImpl
        {
        public:
            explicit
            DirectTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);

            explicit
            DirectTreeSetNodeImpl(DeferredTreeSetNodeImpl& rOriginal);


        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual void doInsertElement(Name const& aName, SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements( ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth);

            virtual Attributes doGetAttributes() const;

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------
        class DeferredValueSetNodeImpl;

        class DirectValueSetNodeImpl : public ValueSetNodeImpl
        {
        public:
            explicit
            DirectValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);

            explicit
            DirectValueSetNodeImpl(DeferredValueSetNodeImpl& rOriginal);

        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual void doInsertElement(Name const& aName,  SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth);

            virtual Attributes doGetAttributes() const;

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);
        };
//-----------------------------------------------------------------------------

        class DeferredTreeSetNodeImpl : public TreeSetNodeImpl
        {
        public:
            explicit
            DeferredTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);
            explicit
            DeferredTreeSetNodeImpl(DirectTreeSetNodeImpl& rOriginal);


        protected:
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges(ElementList& _rRemovedElements);
            virtual void doFinishCommit(SubtreeChange& rChanges);
            virtual void doRevertCommit(SubtreeChange& rChanges);
            virtual void doFailedCommit(SubtreeChange& rChanges);

        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual bool        doIsEmpty() const;
            virtual SetEntry    doFindElement(Name const& aName) ;
            virtual void        doClearElements();
            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor);

            virtual void doInsertElement(Name const& aName, SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth);

            virtual void doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange);

            virtual NodeChangeImpl* doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement);
            virtual NodeChangeImpl* doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);

        // Implementation
        private:
            void implInsertNewElement(Name const& aName, Element const& aNewElement);
            void implRemoveOldElement(Name const& aName);

            ElementSet m_aChangedData;
            bool    m_bChanged;
        };
//-----------------------------------------------------------------------------

        class DeferredValueSetNodeImpl : public ValueSetNodeImpl
        {
        public:
            explicit
            DeferredValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate);

            explicit
            DeferredValueSetNodeImpl(DirectValueSetNodeImpl& rOriginal);


        protected:
        // legacy commit support
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges(ElementList& _rRemovedElements);
            virtual void doFinishCommit(SubtreeChange& rChanges);
            virtual void doRevertCommit(SubtreeChange& rChanges);
            virtual void doFailedCommit(SubtreeChange& rChanges);

        // Base Overrideables
        private:
        // NodeImpl implementation
            virtual bool        doIsEmpty() const;
            virtual SetEntry    doFindElement(Name const& aName) ;
            virtual void        doClearElements();
            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor);

            virtual void doInsertElement(Name const& aName,  SetEntry const& aNewEntry);
            virtual void doRemoveElement(Name const& aName);

            virtual void doInitElements(ISubtree& rTree, TreeDepth nDepth);
            virtual Element doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth);

            virtual void doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange);

            virtual NodeChangeImpl* doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement);
            virtual NodeChangeImpl* doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange);

            virtual bool doHasChanges() const;
            virtual void doCollectChanges(NodeChanges& rChanges) const;
            virtual void doCommitChanges();
            virtual void doMarkChanged();
            virtual NodeImplHolder doCloneIndirect(bool bIndirect);

        // Implementation
        private:
            void implInsertNewElement(Name const& aName, Element const& aNewElement);
            void implRemoveOldElement(Name const& aName);

            ElementSet m_aChangedData;
            bool    m_bChanged;
        };
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_NODEIMPLOBJECTS_HXX_
