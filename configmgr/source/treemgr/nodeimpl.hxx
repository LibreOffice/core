/*************************************************************************
 *
 *  $RCSfile: nodeimpl.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 17:20:54 $
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

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#define CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_

#ifndef __SGI_STL_MEMORY
#include <memory>
#endif

#include "apitypes.hxx"
#include "template.hxx"

#include <vos/refernce.hxx>
#include <vos/ref.hxx>
#include <osl/diagnose.h>

namespace configmgr
{
    class INode;
    class ISubtree;
    class ValueNode;

    class SubtreeChange;
    class ValueChange;

    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Any UnoAny;
        typedef com::sun::star::uno::Type UnoType;

        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;

        class Name;
        class Node;
        class TreeImpl;
        class ElementTreeImpl;

        struct NodeInfo;
        class NodeChangeImpl;
        class NodeChange;
        class NodeChangeInformation;
        class NodeChanges;
        class NodeChangesInformation;
//-----------------------------------------------------------------------------

// Specific types of nodes
//-----------------------------------------------------------------------------

        namespace NodeType
        {
        //---------------------------------------------------------------------
            enum Enum { eVALUE, eGROUP, eSET, eVALUESET, eTREESET };
            inline bool isSet(Enum e)   { return e >= eSET; }
            inline bool isGroup(Enum e) { return e == eGROUP; }
            inline bool isValue(Enum e) { return e == eVALUE; }

        //---------------------------------------------------------------------
        }

//-----------------------------------------------------------------------------
        class NodeImpl;
        typedef vos::ORef<NodeImpl> NodeImplHolder;

        struct INodeHandler;

        // Almost an interface, but derives from concrete OReference
        class NodeImpl : public vos::OReference
        {
        public:
            void collectChanges(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const
                { doCollectChangesWithTarget(rChanges,pParent,nNode); }

            bool hasChanges()               const   { return doHasChanges(); }
            void markChanged()                      { doMarkChanged(); }
            void commitChanges()                    { doCommitChanges(); }

            static void makeIndirect(NodeImplHolder&    aThis, bool bIndirect);

            NodeType::Enum  getType()         const { return doGetType(); }
            void getNodeInfo(NodeInfo& rInfo) const { doGetNodeInfo(rInfo); }
            void setNodeName(Name const& rName_)    { doSetNodeName(rName_); }
            void dispatch(INodeHandler& rHandler_)  { doDispatch(rHandler_); }

        private:
            virtual NodeType::Enum  doGetType()                     const = 0;
            virtual void            doGetNodeInfo(NodeInfo& rInfo_) const = 0;
            virtual void            doSetNodeName(Name const& rName_)    = 0;
            virtual void            doDispatch(INodeHandler& rHandler_) = 0;

            virtual bool doHasChanges() const = 0;
            virtual void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const = 0;
            virtual void doMarkChanged() = 0;
            virtual void doCommitChanges() = 0;
            virtual NodeImplHolder doCloneIndirect(bool bIndirect) = 0;

        protected:
            //helper for migration to new (info based) model for adjusting to changes
            static void addLocalChangeHelper( NodeChangesInformation& rLocalChanges, NodeChange const& aChange);

        };

//-----------------------------------------------------------------------------
        class ValueNodeImpl;
        class GroupNodeImpl;
        class SetNodeImpl;
//-----------------------------------------------------------------------------

        class GroupNodeImpl : public NodeImpl
        {
            ISubtree& m_rOriginal;
        public:
            explicit GroupNodeImpl(ISubtree& rOriginal);
            explicit GroupNodeImpl(GroupNodeImpl& rOriginal); // only for makeIndirect

            std::auto_ptr<SubtreeChange> preCommitChanges();
            void finishCommit(SubtreeChange& rChanges) { doFinishCommit(rChanges); }
            void revertCommit(SubtreeChange& rChanges) { doRevertCommit(rChanges); }
            void failedCommit(SubtreeChange& rChanges) { doFailedCommit(rChanges); }

        protected:
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges();
            virtual void doFinishCommit(SubtreeChange& rChanges);
            virtual void doRevertCommit(SubtreeChange& rChanges);
            virtual void doFailedCommit(SubtreeChange& rChanges);

        // MoreNodeImpl implementation - direct clients don't need it
        protected:
            virtual void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;

        // NodeImpl implementation
            virtual void            doGetNodeInfo(NodeInfo& rInfo) const;
            virtual void            doSetNodeName(Name const& rName) = 0;

        private:
            virtual NodeType::Enum  doGetType() const;
            virtual void            doDispatch(INodeHandler& rHandler);

        };

//-----------------------------------------------------------------------------

        struct SetEntry
        {
            explicit SetEntry(ElementTreeImpl* pTree_);

            bool isValid()  const { return m_pTree != 0; }

            ElementTreeImpl* tree() const { return m_pTree; };
        private:
            ElementTreeImpl* m_pTree;
        };
    //-------------------------------------------------------------------------

        struct SetNodeVisitor
        {
            enum Result { DONE, CONTINUE };
            virtual Result visit(SetEntry const& anEntry) = 0;
        };
    //-------------------------------------------------------------------------

        class SetNodeImpl : public NodeImpl
        {
            ISubtree&       m_rOriginal;
            TemplateHolder  m_aTemplate;
            TreeImpl*       m_pParentTree;
            NodeOffset      m_nContextPos;

            struct InitHelper;
            std::auto_ptr<InitHelper> m_pInit;
        public:
            /// construct a set node referring to rOriginal as data node with the given element-template
            explicit SetNodeImpl(ISubtree& rOriginal, Template* pTemplate);
            /// 'Moving constructor': Constructs a set that takes the data from rOriginal, leaves rOriginal empty
            explicit SetNodeImpl(SetNodeImpl& rOriginal); // only for makeIndirect

        // the following willmostly  be implemented by derived classes (using the virtual equivalents)
            /// does this set contain any elements (loads elements if needed)
            bool        isEmpty();
            /// does this set contain an element named <var>aName</var> (loads elements if needed)
            SetEntry    findElement(Name const& aName);
            /// does this set contain an element named <var>aName</var> (and is that element loaded ?)
            SetEntry    findAvailableElement(Name const& aName);

            /// insert a new entry into this set
            void        insertElement(Name const& aName, SetEntry const& aNewEntry);
            /// remove an existing entry into this set
            void        removeElement(Name const& aName);

            /// Initialize the set data: Set context information, and build the view (actually loading the elements may be deferred)
            void        initElements(TemplateProvider const& aTemplateProvider, TreeImpl& rParentTree, NodeOffset nPos, TreeDepth nDepth);

            /// Call <code>aVisitor.visit(aElement)</code> for each element in this set until SetNodeVisitor::DONE is returned.
            SetNodeVisitor::Result dispatchToElements(SetNodeVisitor& aVisitor);

            /// Get the template that describes elements of this set
            TemplateHolder getElementTemplate() const { return m_aTemplate; }

            /// Prepare committing the changes in this set and its descendants, if any - builds a SubtreeChange describing pending changes.
            std::auto_ptr<SubtreeChange> preCommitChanges();
            /// Finalize committing the changes - patch the original state wrapper, reset the pending changes
            void finishCommit(SubtreeChange& rChanges);
            /// Back out precommited changes - restore original state wrapper, rebuild the pending changes
            void revertCommit(SubtreeChange& rChanges);
            /// Fix up precommited changes after failure - try to detect failures, patch the original state wrapper, reset the pending changes
            void failedCommit(SubtreeChange& rChanges);

        // does not load elements:
            /// Adjust the internal representation after external changes to the original data - build NodeChangeInformation objects for notification
            void adjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChanges, TemplateProvider const& aTemplateProvider, TreeDepth nDepth);

        // legacy commit - default is 'Not supported'
        protected:
            ~SetNodeImpl();
            virtual std::auto_ptr<SubtreeChange>doPreCommitChanges();
            virtual void doFinishCommit(SubtreeChange& rChanges);
            virtual void doRevertCommit(SubtreeChange& rChanges);
            virtual void doFailedCommit(SubtreeChange& rChanges);
        protected:
            TreeImpl*   getParentTree() const;
            NodeOffset  getContextOffset() const;

            ISubtree&   getOriginalSetNode() { return m_rOriginal; };

        // New Overrideables
        private:
            virtual bool        doIsEmpty() const = 0;
            virtual SetEntry    doFindElement(Name const& aName) = 0;
            virtual void        doInsertElement(Name const& aName, SetEntry const& aNewEntry) = 0;
            virtual void        doRemoveElement(Name const& aName) = 0;

            virtual void        doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth) = 0;
            virtual void        doClearElements() = 0;

            virtual void        doAdjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChanges, TemplateProvider const& aTemplateProvider, TreeDepth nDepth) = 0;

            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor) = 0;

            virtual void        doCollectChanges(NodeChanges& rChanges) const = 0;

        // NodeImpl implementation - direct clients don't need it
        protected:
            virtual void            doGetNodeInfo(NodeInfo& rInfo) const;
            virtual void            doSetNodeName(Name const& rName) = 0;
            virtual NodeType::Enum  doGetType() const;

        private:
            virtual void            doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;
            virtual void            doDispatch(INodeHandler& rHandler);

        protected:
            bool implHasLoadedElements() const;
        private:
            bool implLoadElements();
            void implEnsureElementsLoaded();
            bool implInitElements(InitHelper const& aInit);
            void implCollectChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChange, TreeDepth nDepth);
        };

//-----------------------------------------------------------------------------
        class ValueNodeImpl : public NodeImpl
        {
            ValueNode& m_rOriginal;
        public:
            explicit ValueNodeImpl(ValueNode& rOriginal) ;
            explicit ValueNodeImpl(ValueNodeImpl& rOriginal) ; // only for makeIndirect

        // the following are implemented, though pure
        // they delegate directly to m_rOriginal

            /// Does this node assume its default value
            virtual bool    isDefault()     const = 0;
            /// is the default value of this node available
            virtual bool canGetDefaultValue() const = 0;
            /// retrieve the current value of this node
            virtual UnoAny  getValue()      const = 0;
            /// retrieve the default value of this node
            virtual UnoAny getDefaultValue() const = 0;

            /// Does this node assume its default value
            virtual UnoType getValueType()  const = 0;

            virtual void    setValue(UnoAny const& aNewValue) = 0;
            virtual void    setDefault() = 0;

        // legacy commit - default is 'Not supported'
            std::auto_ptr<ValueChange> preCommitChange();
            void finishCommit(ValueChange& rChange) { doFinishCommit(rChange); }
            void revertCommit(ValueChange& rChange) { doRevertCommit(rChange); }
            void failedCommit(ValueChange& rChange) { doFailedCommit(rChange); }

            void adjustToChange(NodeChangesInformation& rLocalChanges, ValueChange const& rExternalChange, TreeImpl& rParentTree, NodeOffset nPos);
        protected:
            virtual std::auto_ptr<ValueChange> doPreCommitChange();
            virtual void doFinishCommit(ValueChange& rChange);
            virtual void doRevertCommit(ValueChange& rChange);
            virtual void doFailedCommit(ValueChange& rChange);


        protected:
            virtual void            doGetNodeInfo(NodeInfo& rInfo) const;
            virtual void            doSetNodeName(Name const& rName) = 0;

        // NodeImpl implementation - direct clients don't need it
            virtual NodeChangeImpl* doAdjustToChange(ValueChange const& rExternalChange);
            virtual NodeChangeImpl* doCollectChange() const;

            virtual void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;
        private:
            virtual NodeType::Enum  doGetType() const;
            virtual void            doDispatch(INodeHandler& rHandler);
        };
//-----------------------------------------------------------------------------

        struct INodeHandler
        {
            virtual void handle( ValueNodeImpl& rNode) = 0;
            virtual void handle( GroupNodeImpl& rNode) = 0;
            virtual void handle( SetNodeImpl& rNode) = 0;
        };

//-----------------------------------------------------------------------------
        // domain-specific 'dynamic_cast' replacements
        ValueNodeImpl&  AsValueNode(NodeImpl& rNode);
        GroupNodeImpl&  AsGroupNode(NodeImpl& rNode);
        SetNodeImpl&    AsSetNode  (NodeImpl& rNode);

//      ValueNodeImpl const& AsValueNode(NodeImpl const& rNode);
//      GroupNodeImpl const& AsGroupNode(NodeImpl const& rNode);
//      SetNodeImpl   const& AsSetNode  (NodeImpl const& rNode);
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
