/*************************************************************************
 *
 *  $RCSfile: nodeimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-13 11:54:51 $
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
#include <stl/memory>
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
        class NodeChanges;
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
            void collectChanges(NodeChanges& rChanges)  const
                { doCollectChanges(rChanges); }

            bool hasChanges()                   const { return doHasChanges(); }
            void markChanged()                        { doMarkChanged(); }
            void commitChanges()                      { doCommitChanges(); }

            static void makeIndirect(NodeImplHolder&    aThis, bool bIndirect);

            virtual NodeType::Enum  getType()                       const = 0;
            virtual void            getNodeInfo(NodeInfo& rInfo)    const = 0;
            virtual void            setNodeName(Name const& rName)   = 0;
            virtual void            dispatch(INodeHandler& rHandler) = 0;

        private:
            virtual bool doHasChanges() const = 0;
            virtual void doCollectChanges(NodeChanges& rChanges) const = 0;
            virtual void doMarkChanged() = 0;
            virtual void doCommitChanges() = 0;
            virtual NodeImplHolder doCloneIndirect(bool bIndirect) = 0;
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

        // NodeImpl implementation
            virtual void            getNodeInfo(NodeInfo& rInfo) const;
            virtual void            setNodeName(Name const& rName) = 0;

            virtual std::auto_ptr<SubtreeChange> preCommitChanges();
            virtual void finishCommit(SubtreeChange& rChanges);
            virtual void revertCommit(SubtreeChange& rChanges);

        // MoreNodeImpl implementation - direct clients don't need it
        private:
            virtual NodeType::Enum  getType() const;
            virtual void            dispatch(INodeHandler& rHandler);
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
        public:
            explicit SetNodeImpl(ISubtree& rOriginal, Template* pTemplate);
            explicit SetNodeImpl(SetNodeImpl& rOriginal); // only for makeIndirect

        // the following wiil be implemented by derived classes
            bool        isEmpty() const                 { return doIsEmpty(); }
            SetEntry    findElement(Name const& aName)  { return doFindElement(aName); };

            void        insertElement(Name const& aName, SetEntry const& aNewEntry) { doInsertElement(aName,aNewEntry); }
            void        removeElement(Name const& aName)                            { doRemoveElement(aName); }

            void        initElements(TreeImpl& rParentTree, NodeOffset nPos, TreeDepth nDepth);

            SetNodeVisitor::Result dispatchToElements(SetNodeVisitor& aVisitor)     { return doDispatchToElements(aVisitor); }

            TemplateHolder getElementTemplate() const { return m_aTemplate; }

        // NodeImpl implementation
            virtual void            getNodeInfo(NodeInfo& rInfo) const;
            virtual void            setNodeName(Name const& rName) = 0;
            virtual NodeType::Enum  getType() const;

        // legacy commit - default is 'Not supported'
            virtual std::auto_ptr<SubtreeChange> preCommitChanges();
            virtual void finishCommit(SubtreeChange& rChanges);
            virtual void revertCommit(SubtreeChange& rChanges);

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

            virtual void        doInitElements(ISubtree& rTree, TreeDepth nDepth) = 0;
            virtual void        doClearElements() = 0;

            virtual SetNodeVisitor::Result doDispatchToElements(SetNodeVisitor& aVisitor) = 0;

        // More NodeImpl implementation - direct clients don't need it
        private:
            virtual void            dispatch(INodeHandler& rHandler);
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

        // NodeImpl implementation
            virtual void            getNodeInfo(NodeInfo& rInfo) const;
            virtual void            setNodeName(Name const& rName) = 0;
            virtual NodeType::Enum  getType() const;

        // legacy commit - default is 'Not supported'
            virtual std::auto_ptr<ValueChange> preCommitChange();
            virtual void finishCommit(ValueChange& rChange);
            virtual void revertCommit(ValueChange& rChange);

        // More NodeImpl implementation - direct clients don't need it
        private:
            virtual void            dispatch(INodeHandler& rHandler);
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
