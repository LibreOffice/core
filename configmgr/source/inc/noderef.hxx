/*************************************************************************
 *
 *  $RCSfile: noderef.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 16:11:15 $
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

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#define CONFIGMGR_CONFIGNODE_HXX_

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
    class INode;

    namespace data      { using memory::Accessor; class TreeAccessor; }
    namespace view      { class ViewTreeAccess; }
    namespace configapi { class Factory; }
    namespace node      { struct Attributes; }
    namespace configuration
    {
    //-------------------------------------------------------------------------
        class Name;
        class AbsolutePath;
        class RelativePath;
        namespace Path { class Component; }

        class NodeChange;
        class NodeChanges;
        class NodeChangesTree;
    //-------------------------------------------------------------------------

        namespace argument { struct NoValidate; }

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
    //-------------------------------------------------------------------------

        class NodeRef;
        class ValueRef;
        class AnyNodeRef;
        class ElementRef;

        class NodeID;
        class Tree;

        class TreeImpl;

        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;
        const TreeDepth C_TreeDepthAll = ~0u;
    //-------------------------------------------------------------------------

        /// interface for a class that can be used to do some operation on a set of <type>NodeRef</type>s and <type>ValueRef</type>s.
        struct NodeVisitor
        {
            /// returned from <method>handle</method> to indicate whether the operation is complete or should continue
            enum Result { DONE, CONTINUE };
            /// do the operation on <var>aNode</var>. needs to be implemented by concrete visitor classes
            virtual Result handle(Tree const& aTree, NodeRef const& aNode) = 0;
            /// do the operation on <var>aValue</var>. needs to be implemented by concrete visitor classes
            virtual Result handle(Tree const& aTree, ValueRef const& aValue) = 0;
        };
    //-------------------------------------------------------------------------

        /// represents a inner node position in some tree
        class NodeRef
        {
        public:
            /// constructs an empty (invalid) node
            NodeRef();

            /// copy a node (with reference semantics)
            NodeRef(NodeRef const& rOther);
            /// copy a node (with reference semantics)
            NodeRef& operator=(NodeRef const& rOther);

            void swap(NodeRef& rOther);

            ~NodeRef();

            /// checks, if this represents an existing node
            inline bool isValid() const;

            /// returns the offset of this Node
            inline NodeOffset getOffset() const;

            /// returns the offset of this Node
            inline TreeDepth getDepth() const;

        private:
            friend class Tree;
            friend class TreeRef;
            friend class TreeImplHelper;
            friend class AnyNodeRef;
            NodeRef(NodeOffset nPos, TreeDepth nDepth);
        private:
            NodeOffset  m_nPos;
            TreeDepth   m_nDepth;
        };
    //-------------------------------------------------------------------------

        /** identifies a hierarchy of config entries (identified by <type>NodeRef</type>s and <type>ValueRef</type>s)

            <p>Examples for trees include</p>
            <ulist>
                <li>A module tree (for a specific set of parameters).</li>
                <li>An updating tree (for a part of the whole).</li>
                <li>A set element (updating or not), which could be detached.</li>
            <ulist>
        */
        class TreeRef
        {
        public:
            /// create a tree with a given implementation
            TreeRef(TreeImpl* pImpl);
            /// copy a tree (with reference semantics)
            TreeRef(TreeRef const& rOther);
            /// copy a tree (with reference semantics)
            TreeRef& operator=(TreeRef const& rOther);

            ~TreeRef();

            void swap(TreeRef& rOther);

            /// checks, if this refers to an existing tree
            inline bool isValid() const;

            /// checks, if this represents a real tree
            bool isEmpty() const;

            /// retrieves the number of immediately contained (subtree) nodes
            NodeOffset getContainedInnerNodeCount() const;

            /// checks whether the node <var>aNode</var> is a valid inner node in this tree.
            bool isValidNode(NodeRef const& aNode) const;

            // releases the data this tree operates on
            void disposeData();

        // Root node
        public:
            /// checks whether <var>aNode</var> is the root node of this tree
            bool            isRootNode(NodeRef const& aNode) const;

            /// gets the root node of this tree
            NodeRef         getRootNode() const;

        // Tree context handling
        public:
            /// gets the parent tree of this tree, if available
            TreeRef getContextTree() const;
            /// gets the parent node of this tree ('s root node), if available
            NodeRef getContextNode() const;
            #if defined(_MSC_VER) && (_MSC_VER > 1300 )
            friend bool equalTreeRef(TreeRef const& lhs, TreeRef const& rhs);
            #else
            friend bool equalTreeRef(TreeRef const& lhs, TreeRef const& rhs) { return lhs.m_pImpl == rhs.m_pImpl; }
            #endif
    private:
            friend class Tree;
            friend class TreeImplHelper;
            TreeImpl* operator->() const { return  m_pImpl; }
            TreeImpl& operator* () const { return *m_pImpl; }
            TreeImpl* get()        const { return  m_pImpl; }

            TreeImpl* m_pImpl;
        };
        #if defined(_MSC_VER) && (_MSC_VER > 1300 )
        inline bool equalTreeRef(TreeRef const& lhs, TreeRef const& rhs) { return lhs.m_pImpl == rhs.m_pImpl; }
        #endif

        /** represents a hierarchy of config entries (identified by <type>NodeRef</type>s and <type>ValueRef</type>s)

            <p>Examples for trees include</p>
            <ulist>
                <li>A module tree (for a specific set of parameters).</li>
                <li>An updating tree (for a part of the whole).</li>
                <li>A set element (updating or not), which could be detached.</li>
            <ulist>
        */
        class Tree
        {
            typedef node::Attributes NodeAttributes;
        public:
            /// create a tree with a given implementation
            Tree(data::Accessor const& _accessor, TreeImpl* pImpl);
            /// create a tree with a given implementation
            Tree(data::Accessor const& _accessor, TreeRef const& _aTree);

            /// checks, if this refers to an existing tree
            bool isValid() const
            { return m_ref.isValid(); }

            /// checks, if this represents a real tree
            bool isEmpty() const
            { return m_ref.isEmpty(); }

            /// retrieves the number of immediately contained (subtree) nodes
            NodeOffset getContainedInnerNodeCount() const
            { return m_ref.getContainedInnerNodeCount(); }

            /// checks whether the node <var>aNode</var> is a valid inner node in this tree.
            bool isValidNode(NodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> is a valid inner node in this tree.
            bool isValidNode(AnyNodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> is a valid value node in this tree.
            bool isValidNode(ValueRef const& aNode) const;

            /// checks whether the node <var>aNode</var> has any element nodes (of its own).
            bool hasElements(NodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> has a element node named <var>aName</var>.
            bool hasElement(NodeRef const& aNode, Name const& aName) const;

            /// checks whether the node <var>aNode</var> has a element node named <var>aName</var>.
            bool hasElement(NodeRef const& aNode, Path::Component const& aName) const;

            /** gets the element named <var>aName</var> of node <var>aNode</var>.
                <p>PRE: <code>hasElement(aNode,aName) == true</code></p>
                <p>If there is no such element, may return an empty node or raise an exception (?)</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            ElementRef getElement(NodeRef const& aNode, Name const& aName) const;

            /** gets the element named <var>aName</var> of node <var>aNode</var>, if it is available.
                <p>PRE: <code>hasElement(aNode,aName) == true</code></p>
                <p>If there is no such element, may return an empty node or raise an exception (?)</p>
                <p>Caution: May miss existing children unless hasChild/getChild has been called before.</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            ElementRef getAvailableElement(NodeRef const& aNode, Name const& aName) const;

            /// checks whether the node <var>aNode</var> has any child value (in this tree).
            bool hasChildValues(NodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> has any child subtrees (in this tree).
            bool hasChildNodes(NodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> has any child nodes (in this tree).
            bool hasChildren(NodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> has a child value (in this tree) named <var>aName</var>.
            bool hasChildValue(NodeRef const& aNode, Name const& aName) const;

            /// checks whether the node <var>aNode</var> has a child subtree (in this tree) named <var>aName</var>.
            bool hasChildNode(NodeRef const& aNode, Name const& aName) const;

            /// checks whether the node <var>aNode</var> has a child node (in this tree) named <var>aName</var>.
            bool hasChild(NodeRef const& aNode, Name const& aName) const;

            /** gets the child value (in this tree) named <var>aName</var> of node <var>aNode</var>.
                <p>PRE: <code>hasChildValue(aNode,aName) == true</code></p>
                <P>If there is no such node, may return an empty node or raise an exception (?)</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            ValueRef getChildValue(NodeRef const& aNode, Name const& aName) const;

            /** gets the child value (in this tree) named <var>aName</var> of node <var>aNode</var>.
                <p>PRE: <code>hasChildNode(aNode,aName) == true</code></p>
                <P>If there is no such node, may return an empty node or raise an exception (?)</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            NodeRef getChildNode(NodeRef const& aNode, Name const& aName) const;

            /** gets the child value (in this tree) named <var>aName</var> of node <var>aNode</var>.
                <p>PRE: <code>hasChildNode(aNode,aName) == true</code></p>
                <P>If there is no such node, may return an empty node or raise an exception (?)</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            AnyNodeRef getAnyChild(NodeRef const& aNode, Name const& aName) const;

            /// return the local <type>Name</type> of the root node of this tree
            Path::Component getRootName() const;

            /// return the local <type>Name</type> of node <var>aNode</var> in this tree
            Name            getName(NodeRef const& aNode) const;

            /// return the local <type>Name</type> of node <var>aNode</var> in this tree
            Name            getName(AnyNodeRef const& aNode) const;

            /// return the local <type>Name</type> of value <var>aValue</var> in this tree
            Name            getName(ValueRef const& aValue) const;

            /// return the <type>Attributes</type> of node <var>aNode</var> in this tree
            NodeAttributes  getAttributes(NodeRef const& aNode) const;

            /// return the <type>Attributes</type> of node <var>aNode</var> in this tree
            NodeAttributes  getAttributes(AnyNodeRef const& aNode)  const;

            /// return the <type>Attributes</type> of value <var>aValue</var> in this tree
            NodeAttributes  getAttributes(ValueRef const& aValue)   const;

            /// get the Uno <type scope='com::sun::star::uno'>Type</type> of value <var>aValue</var> in this tree
            UnoType         getUnoType(ValueRef const& aValue) const;

        // Parent/NodeRef context handling
        public:
            /// return the parent <type>NodeRef</type> of <var>aNode</var> (or an empty node, if it is the tree root)
            NodeRef getParent(AnyNodeRef const& aNode) const;

            /// return the parent <type>NodeRef</type> of <var>aNode</var> (or an empty node, if it is the tree root)
            NodeRef getParent(NodeRef const& aNode) const;

            /// return the parent <type>NodeRef</type> of <var>aValue</var> (or an empty node, if it is the tree root)
            NodeRef getParent(ValueRef const& aValue) const;

            /// return the <type>AbsolutePath</type> of <var>aNode</var>
            AbsolutePath    getAbsolutePath(NodeRef const& aNode) const;

            /// gets the <type>AbsolutePath</type> of the root node of this tree
            AbsolutePath    getRootPath() const;

            /// gets the root node of this tree
            NodeRef         getRootNode() const
            { return m_ref.getRootNode(); }

            /// checks whether <var>aNode</var> is the root node of this tree
            bool            isRootNode(NodeRef const& aNode) const
            { return m_ref.isRootNode(aNode); }

        public:
        // value handling
            /** retrieves the current value for <var>aNode</var>, provided there is one and it
                is available.
            */
            UnoAny      getNodeValue(ValueRef const& aNode)     const; // only works for value nodes

        // default value handling
            /// checks whether <var>aNode</var> has a default value
            bool        hasNodeDefault(ValueRef const& aNode)   const; // only works for value nodes

            /// checks whether <var>aNode</var> assumes its default value
            bool        isNodeDefault(ValueRef const& aNode)        const; // only works for value nodes

            /// checks whether <var>aNode</var> has a default state
            bool        hasNodeDefault(NodeRef const& aNode)    const;

            /// checks whether <var>aNode</var> assumes its default state
            bool        isNodeDefault(NodeRef const& aNode)         const;

            /// checks whether <var>aNode</var> has a default state
            bool        hasNodeDefault(AnyNodeRef const& aNode) const;

            /// checks whether <var>aNode</var> assumes its default state
            bool        isNodeDefault(AnyNodeRef const& aNode)      const;

            /// checks whether the default values are available for the children of <var>aNode</var> (if applicable)
            bool        areValueDefaultsAvailable(NodeRef const& aNode) const;

            /** retrieves the default value for <var>aNode</var>, provided there is one and it
                is available.
                <p>call <method>Tree::ensureDefaults</method> first to achieve best results</p>
            */
            UnoAny      getNodeDefaultValue(ValueRef const& aNode)      const; // only works for value nodes

        // Tree context handling
        public:
            /// gets the parent tree of this tree, if available
            Tree getContextTree() const;
            /// gets the parent node of this tree ('s root node), if available
            NodeRef getContextNode() const;

        // Update handling
        public:
            /// checks whether there are pending changes on this tree
            bool hasChanges()  const;

            /// lists any pending changes on this tree
            bool collectChanges(NodeChanges& aChanges)  const;

            /// applies <var>aChange</var> to <var>aNode</var> within this tree
            void integrate(NodeChange& aChange, NodeRef const& aNode, bool bLocal)  const;

            /// applies <var>aChange</var> to <var>aNode</var> within this tree
            void integrate(NodeChange& aChange, ValueRef const& aNode, bool bLocal)  const;

            /// applies <var>aChanges</var> to the children or descendants of <var>aNode</var> within this tree
            void integrate(NodeChanges& aChanges, NodeRef const& aNode, bool bLocal) const;

            /// applies <var>aChanges</var> to the descendants of <var>aNode</var> within this tree
            void integrate(NodeChangesTree& aChanges, NodeRef const& aNode) const;

        // Visitor handling
        public:
            /// dispatch node <var>aNode</var> to a Visitor
            NodeVisitor::Result visit(NodeRef const& aNode, NodeVisitor& aVisitor) const
            { return aVisitor.handle(*this,aNode); }

            /// dispatch node <var>aNode</var> to a Visitor
            NodeVisitor::Result visit(ValueRef const& aNode, NodeVisitor& aVisitor) const
            { return aVisitor.handle(*this,aNode); }

            /// dispatch node <var>aNode</var> to a Visitor
            NodeVisitor::Result visit(AnyNodeRef const& aNode, NodeVisitor& aVisitor) const;

            /** lets <var>aVisitor</var> visit the child nodes of <var>aNode</var>
                <p>The order in which nodes are visited is repeatable (but currently unspecified)</p>
                <p> Visits nodes until NodeVisitor::DONE is returned, then returns NodeVisitor::DONE.<BR/>
                    If all visits return NodeVisitor::CONTINUE, returns NodeVisitor::CONTINUE.<BR/>
                    If no children are present, returns NodeVisitor::CONTINUE
                </p>
            */
            NodeVisitor::Result dispatchToChildren(NodeRef const& aNode, NodeVisitor& aVisitor) const;

            /** lets <var>aVisitor</var> visit the child nodes of <var>aNode</var>
                <p>The order in which nodes are visited is repeatable (but currently unspecified)</p>
                <p> Visits nodes until NodeVisitor::DONE is returned, then returns NodeVisitor::DONE.<BR/>
                    If all visits return NodeVisitor::CONTINUE, returns NodeVisitor::CONTINUE.<BR/>
                    If no children are present, returns NodeVisitor::CONTINUE
                </p>
            */
            NodeVisitor::Result dispatchToChildren(AnyNodeRef const& aNode, NodeVisitor& aVisitor) const;
        // More NodeRef handling
        public:
            TreeRef getRef() const { return m_ref; }

        // view & data layer binding
        public:
            data::Accessor const & getDataAccessor() const { return m_accessor; }

            view::ViewTreeAccess getView() const;

            void rebind(data::Accessor const& _aAccessor);
            void unbind();
        // Comparison
        public:
            friend bool equalTree(Tree const& lhs, Tree const& rhs) { return equalTreeRef(lhs.m_ref, rhs.m_ref); }
        private:
            friend class TreeImplHelper;
            data::Accessor  m_accessor;
            TreeRef         m_ref;
        };
    //-------------------------------------------------------------------------

        class NodeID
        {
        public:
            NodeID(Tree const& rTree, NodeRef const& rNode);
            NodeID(TreeRef const& rTree, NodeRef const& rNode);
            NodeID(TreeImpl* pImpl, NodeOffset nNode);

        // comparison
            // equality
            friend bool operator==(NodeID const& lhs, NodeID const& rhs)
            { return lhs.m_pTree == rhs.m_pTree && lhs.m_nNode == rhs.m_nNode; }
            // ordering
            friend bool operator < (NodeID const& lhs, NodeID const& rhs);
            // checking
            bool isEmpty() const;
            // checking
            bool isValidNode() const;
            // hashing
            size_t hashCode() const;
            // use as index - returns a value in the range 0..rTree.getContainedNodes() for the tree used to construct this
            NodeOffset toIndex() const;
        private:
            friend class TreeImplHelper;
            TreeImpl*   m_pTree;
            NodeOffset  m_nNode;
        };
    //-------------------------------------------------------------------------

        /** make a <type>Name</type> out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for a member of group <var>aNode</var> within <var>aTree</var>

        */
        Name validateChildName(OUString const& sName, Tree const& aTree, NodeRef const& aNode );

        /** make a <type>Name</type> out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for an element of set <var>aNode</var> within <var>aTree</var>

        */
        Name validateElementName(OUString const& sName, Tree const& aTree, NodeRef const& aNode );

        /** make a <type>Name</type> out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for a child of <var>aNode</var> within <var>aTree</var>

        */
        Name validateChildOrElementName(OUString const& sName, Tree const& aTree, NodeRef const& aNode );

        /** make one path component out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for an element of set <var>aNode</var> within <var>aTree</var>

        */
        Path::Component validateElementPathComponent(OUString const& sName, Tree const& aTree, NodeRef const& aNode );

        /** parse <var>aPath</var> into a relative path,
            valid in the context of node <var>aNode<var/> in <var>aTree<var/>.

            @returns
                <var>aPath<var/> parsed as a relative path
            @throws InvalidName
                if <var>aPath<var/> is not a relative path or not valid in the context of <var>aNode<var/>
        */
        RelativePath validateRelativePath(OUString const& aPath, Tree const& aTree, NodeRef const& aNode);

        /** parse <var>aPath</var> as a configuration path
            and reduce it to be relative to node <var>aNode<var/> in <var>aTree<var/>.

            @returns
                the result of parsing <var>aPath<var/>, if that results in a relative path, or
                the part of it relative to <var>aNode<var/>,
                  if it is an absolute path to a descendant of <var>aNode<var/>
            @throws InvalidName
                if <var>aPath<var/> is not awell-formed path or
                if it is an absolute path that is not to a descendant of <var>aNode<var/>
        */
        RelativePath validateAndReducePath(OUString const& aPath, Tree const& aTree, NodeRef const& aNode);

        /** checks whether there are any immediate children of <var>aNode</var> (which is in <var>aTree</var>)

            @return
                <TRUE/> if a child node exists
                <FALSE/> otherwise
        */
        bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode);

        /** checks whether there is an immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>

            @return
                <TRUE/> if the child node exists
                <FALSE/> otherwise
        */
        bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode, Name const& aName);

        /** checks whether there is an immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>

            @return
                <TRUE/> if the child node exists
                <FALSE/> otherwise
        */
        bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode, Path::Component const& aName);

        /** tries to find the immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>
            <p> On return <var>aNode</var> is modified to refer to the node found and
                <var>aTree</var> will then refer to the tree that node is in.
            <p/>
            <p>Caution: May miss an existing child unless the child has been accessed before.</p>

            @return
                <TRUE/> if the child node exists and is available
                (so <var>aNode</var> and <var>aTree</var> refer to the desired node),
                <FALSE/> otherwise

            @see NodeRef::getAvailableChild
        */
        bool findInnerChildOrAvailableElement(Tree& aTree, NodeRef& aNode, Name const& aName);

        /// test whether the given node is a structural (inner) node
        bool isStructuralNode(Tree const& aTree, NodeRef const& aNode);

        /// test whether the given inner node is a group node
        bool isGroupNode(Tree const& aTree, NodeRef const& aNode);

        /// get the value for a node that is a simple value (as tree element)
        UnoAny getSimpleElementValue(Tree const& aTree, NodeRef const& aNode);

        /// test whether the given inner node is a set node
        bool isSetNode(Tree const& aTree, NodeRef const& aNode);

        osl::Mutex&             getRootLock(TreeRef const& aTree);
        memory::Segment const * getRootSegment(TreeRef const& aTree);

        typedef std::vector<NodeID>     NodeIDList;

        void getAllContainedNodes(Tree const& aTree, NodeIDList& aList);
        NodeID findNodeFromIndex(TreeRef const& aTreeRef, NodeOffset nIndex);

    //-------------------------------------------------------------------------
        inline bool TreeRef::isValid() const
        {
            return m_pImpl != 0;
        }

    //-------------------------------------------------------------------------
        inline bool NodeRef::isValid() const
        {
            return m_nPos != 0;
        }

    //-------------------------------------------------------------------------
        inline NodeOffset NodeRef::getOffset() const
        {
            return m_nPos;
        }

    //-------------------------------------------------------------------------
        inline TreeDepth NodeRef::getDepth() const
        {
            return m_nDepth;
        }

    //-------------------------------------------------------------------------
        inline bool operator!=(NodeID const& lhs, NodeID const& rhs)
        { return !(lhs == rhs); }
        //---------------------------------------------------------------------

        inline bool operator>=(NodeID const& lhs, NodeID const& rhs)
        { return !(lhs < rhs); }
        //---------------------------------------------------------------------
        inline bool operator > (NodeID const& lhs, NodeID const& rhs)
        { return  (rhs < lhs); }
        inline bool operator<=(NodeID const& lhs, NodeID const& rhs)
        { return !(rhs < lhs); }
    //-------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNODE_HXX_
