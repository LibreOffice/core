/*************************************************************************
 *
 *  $RCSfile: anynoderef.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:53 $
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

#ifndef CONFIGMGR_CONFIGANYNODE_HXX_
#define CONFIGMGR_CONFIGANYNODE_HXX_

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif

namespace configmgr
{
    namespace configapi { class Factory; }
    namespace node { struct Attributes; }

    namespace configuration
    {
    //-------------------------------------------------------------------------
        class Name;
    //-------------------------------------------------------------------------

        namespace argument { struct NoValidate; }

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
    //-------------------------------------------------------------------------

        class NodeRef;
        class ValueRef;
        class AnyNodeRef;
        class NodeID;

        class Tree;

        class TreeImpl;

        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;
    //-------------------------------------------------------------------------

        /// represents any node in some tree
        class AnyNodeRef
        {
        public:
            /// constructs an empty (invalid) node
            AnyNodeRef();

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

        private:
            friend class Tree;
            friend class TreeImplHelper;
            AnyNodeRef(NodeOffset nParentPos, TreeDepth m_nDepth);
            AnyNodeRef(Name const& aName, NodeOffset nParentPos);

            bool checkValidState() const;
        private:
            Name        m_sNodeName;
            NodeOffset  m_nUsedPos;
            TreeDepth   m_nDepth;
        };
    //-------------------------------------------------------------------------

        /** checks whether there are any immediate children of <var>aNode</var> (which is in <var>aTree</var>)

            @return
                <TRUE/> if a child node exists
                <FALSE/> otherwise
        */
        inline
        bool hasChildOrElement(Tree const& aTree, AnyNodeRef const& aNode)
        { return aNode.isNode() && hasChildOrElement(aTree,aNode.toNode()); }

        /** checks whether there is an immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>

            @return
                <TRUE/> if the child node exists
                <FALSE/> otherwise
        */
        inline
        bool hasChildOrElement(Tree const& aTree, AnyNodeRef const& aNode, Name const& aName)
        { return aNode.isNode() && hasChildOrElement(aTree,aNode.toNode(),aName); }

        /** tries to find the immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>
            <p> On return <var>aNode</var> is modified to refer to the node found and
                <var>aTree</var> will then refer to the tree that node is in.
            <p/>

            @return The requested child node, if it exists
                (then <var>aTree</var> refers to the tree containing the desired node),
        */
        AnyNodeRef getChildOrElement(Tree& aTree, NodeRef const& aParentNode, Name const& aName);

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
        AnyNodeRef getLocalDescendant(Tree const& aTree, NodeRef const& aNode, RelativePath const& aPath);

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
        AnyNodeRef getDeepDescendant(Tree& aTree, NodeRef& aNode, RelativePath& aPath);

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
