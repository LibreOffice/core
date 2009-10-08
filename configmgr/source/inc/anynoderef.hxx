/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anynoderef.hxx,v $
 * $Revision: 1.8 $
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

#ifndef CONFIGMGR_CONFIGANYNODE_HXX_
#define CONFIGMGR_CONFIGANYNODE_HXX_

#include "noderef.hxx"

namespace configmgr
{
    namespace configapi { class Factory; }
    namespace node { struct Attributes; }

    namespace configuration
    {
    //-------------------------------------------------------------------------
        class NodeRef;
        class ValueRef;
        class AnyNodeRef;
        class NodeID;
        class Tree;
    //-------------------------------------------------------------------------

        /// represents any node in some tree
        class AnyNodeRef
        {
        public:
            /// constructs an empty (invalid) node
            AnyNodeRef();

            AnyNodeRef(unsigned int nParentPos, unsigned int m_nDepth);
            AnyNodeRef(rtl::OUString const& aName, unsigned int nParentPos);

            /// converts an inner node
            explicit AnyNodeRef(NodeRef const& aInnerNode);
            /// converts a value node
            explicit AnyNodeRef(ValueRef const& aValueNode);

            /// copy a node (with reference semantics)
            AnyNodeRef(AnyNodeRef const& rOther);
            /// copy a node (with reference semantics)
            AnyNodeRef& operator=(AnyNodeRef const& rOther);

            void swap(AnyNodeRef& rOther);

            ~AnyNodeRef();

            /// checks, if this represents an existing node
            inline bool isValid() const;

            /// checks if this a node (rather than a value only)
            bool isNode() const;

            /// converts this, if it is a value
            ValueRef toValue() const;

            /// converts this, if it is a inner node
            NodeRef toNode() const;

#if OSL_DEBUG_LEVEL > 0
            bool checkValidState() const;
#endif

            rtl::OUString m_sNodeName;
            unsigned int    m_nUsedPos;
            unsigned int    m_nDepth;
        };
    //-------------------------------------------------------------------------

        /** checks whether there is an immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>

            @return
                <TRUE/> if the child node exists
                <FALSE/> otherwise
        */
        inline
        bool hasChildOrElement(rtl::Reference< Tree > const& aTree, AnyNodeRef const& aNode, rtl::OUString const& aName)
        { return aNode.isNode() && hasChildOrElement(aTree,aNode.toNode(),aName); }

        /** tries to find the immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>
            <p> On return <var>aNode</var> is modified to refer to the node found and
                <var>aTree</var> will then refer to the tree that node is in.
            <p/>

            @return The requested child node, if it exists
                (then <var>aTree</var> refers to the tree containing the desired node),
        */
        AnyNodeRef getChildOrElement(rtl::Reference< Tree > & aTree, NodeRef const& aParentNode, rtl::OUString const& aName);

        /** tries to find the descendant of <var>aNode</var> specified by <var>aPath</var> within <var>aTree</var>
            <p> This function follows the given path stepwise, until a requested node is missing in the tree.</p>
            <p> On return <var>aNode</var> is modified to refer to the last inner node found
                and <var>aTree</var> will be unchanged (except for deprecated usage).
            <p/>
            <p> Also, <var>aPath</var> is modified to contain the unresolved part of the original path.
            </p>

            @return the requested node, if the path could be resolved completely
                (so <var>aNode</var> refers to the desired node or its parent,
                and <var>aPath</var> is empty)<BR/>
                an invalid node otherwise
        */
        AnyNodeRef getLocalDescendant(rtl::Reference< Tree > const& aTree, NodeRef const& aNode, RelativePath const& aPath);

        /** tries to find the descendant of <var>aNode</var> (which is in <var>aTree</var>) specified by <var>aPath</var>
            <p> This function follows the given path stepwise, until a requested node is missing in the tree.</p>
            <p> On return <var>aNode</var> is modified to refer to the last inner node found and
                <var>aTree</var> will then refer to the tree that node is in.
            <p/>
            <p> Also, <var>aPath</var> is modified to contain the unresolved part of the original path.
            </p>

            @return the requested node, if the path could be resolved completely
                (so <var>aNode</var> and <var>aTree</var> refer to the desired node or its parent,
                and <var>aPath</var> is empty)<BR/>
                an invalid node otherwise
        */
        AnyNodeRef getDeepDescendant(rtl::Reference< Tree > & aTree, NodeRef& aNode, RelativePath& aPath);

    //-------------------------------------------------------------------------
        inline bool AnyNodeRef::isValid() const
        {
            OSL_ASSERT( m_nUsedPos == 0 || checkValidState() );
            return m_nUsedPos != 0;
        }

    //-------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGANYNODE_HXX_
