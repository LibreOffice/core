/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: noderef.hxx,v $
 * $Revision: 1.21 $
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

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#define CONFIGMGR_CONFIGNODE_HXX_

#include "rtl/ref.hxx"

#include "configexcept.hxx"
#include "configpath.hxx"
#include "tree.hxx"

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
    class INode;

    namespace view      { class ViewTreeAccess; }
    namespace configapi { class Factory; }
    namespace node      { struct Attributes; }
    namespace configuration
    {
    //-------------------------------------------------------------------------
    class AbsolutePath;
    class RelativePath;
        namespace Path { class Component; }

        class NodeChange;
        class NodeChanges;
        class NodeRef;
        class ValueRef;
        class AnyNodeRef;

        class NodeID;

        const unsigned int C_TreeDepthAll = ~0u;
    //-------------------------------------------------------------------------

        /// represents a inner node position in some tree
        class NodeRef
        {
        public:
            /// constructs an empty (invalid) node
            NodeRef();

            NodeRef(unsigned int nPos, unsigned int nDepth);

            /// copy a node (with reference semantics)
            NodeRef(NodeRef const& rOther);
            /// copy a node (with reference semantics)
            NodeRef& operator=(NodeRef const& rOther);

            void swap(NodeRef& rOther);

            ~NodeRef();

            /// checks, if this represents an existing node
            inline bool isValid() const;

            /// returns the offset of this Node
            inline unsigned int getOffset() const;

            /// returns the offset of this Node
            inline unsigned int getDepth() const;

        private:
            friend class AnyNodeRef;
        private:
            unsigned int    m_nPos;
            unsigned int    m_nDepth;
        };
    //-------------------------------------------------------------------------

        class NodeID
        {
        public:
            NodeID(rtl::Reference< Tree > const& rTree, NodeRef const& rNode);
            NodeID(Tree* pImpl, unsigned int nNode);

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
            unsigned int toIndex() const;

            Tree * getTree() const { return m_pTree; }

            unsigned int getOffset() const { return m_nNode; }

            NodeRef getNode() const;

        private:
            Tree*   m_pTree;
            unsigned int    m_nNode;
        };
    //-------------------------------------------------------------------------

        /** make a name out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for a member of group <var>aNode</var> within <var>aTree</var>

        */
        rtl::OUString validateChildName(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode );

        /** make a name out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for an element of set <var>aNode</var> within <var>aTree</var>

        */
        rtl::OUString validateElementName(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode );

        /** make a name out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for a child of <var>aNode</var> within <var>aTree</var>

        */
        rtl::OUString validateChildOrElementName(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode );

        /** make one path component out of <var>sName</var>.
            @throws InvalidName
                if <var>sName</var> is not a valid name for an element of set <var>aNode</var> within <var>aTree</var>

        */
        Path::Component validateElementPathComponent(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode );

        /** parse <var>aPath</var> into a relative path,
            valid in the context of node <var>aNode<var/> in <var>aTree<var/>.

            @returns
                <var>aPath<var/> parsed as a relative path
            @throws InvalidName
                if <var>aPath<var/> is not a relative path or not valid in the context of <var>aNode<var/>
        */
        RelativePath validateRelativePath(rtl::OUString const& aPath, rtl::Reference< Tree > const& aTree, NodeRef const& aNode);

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
        RelativePath validateAndReducePath(rtl::OUString const& aPath, rtl::Reference< Tree > const& aTree, NodeRef const& aNode);
        /** checks whether there is an immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>

            @return
                <TRUE/> if the child node exists
                <FALSE/> otherwise
        */
        bool hasChildOrElement(rtl::Reference< Tree > const& aTree, NodeRef const& aNode, rtl::OUString const& aName);

        /** checks whether there is an immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>

            @return
                <TRUE/> if the child node exists
                <FALSE/> otherwise
        */
        bool hasChildOrElement(rtl::Reference< Tree > const& aTree, NodeRef const& aNode, Path::Component const& aName);

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
        bool findInnerChildOrAvailableElement(rtl::Reference< Tree >& aTree, NodeRef& aNode, rtl::OUString const& aName);

        /// test whether the given node is a structural (inner) node
        bool isStructuralNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode);

        /// test whether the given inner node is a group node
        bool isGroupNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode);

        /// get the value for a node that is a simple value (as tree element)
        com::sun::star::uno::Any getSimpleElementValue(rtl::Reference< Tree > const& aTree, NodeRef const& aNode);

        /// test whether the given inner node is a set node
        bool isSetNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode);

        void getAllContainedNodes(rtl::Reference< Tree > const& aTree, std::vector<NodeID>& aList);
        NodeID findNodeFromIndex(rtl::Reference< Tree > const& aTreeRef, unsigned int nIndex);

    //-------------------------------------------------------------------------
        inline bool NodeRef::isValid() const
        {
            return m_nPos != 0;
        }

    //-------------------------------------------------------------------------
        inline unsigned int NodeRef::getOffset() const
        {
            return m_nPos;
        }

    //-------------------------------------------------------------------------
        inline unsigned int NodeRef::getDepth() const
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
