/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: valueref.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:28:07 $
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

#ifndef CONFIGMGR_CONFIGVALUEREF_HXX_
#define CONFIGMGR_CONFIGVALUEREF_HXX_

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif

namespace configmgr
{
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

        class TreeImpl;

        typedef unsigned int NodeOffset;
    //-------------------------------------------------------------------------

        /// represents a value node in some tree
        class ValueRef
        {
        public:
            /// constructs an empty (invalid) node
            ValueRef();

            /// copy a node (with reference semantics)
            ValueRef(ValueRef const& rOther);
            /// copy a node (with reference semantics)
            ValueRef& operator=(ValueRef const& rOther);

            void swap(ValueRef& rOther);

            ~ValueRef();

            /// checks, if this represents an existing node
            inline bool isValid() const;

        private:
            friend class Tree;
            friend class TreeImplHelper;
            ValueRef(Name const& aName, NodeOffset nParentPos);

            bool checkValidState() const;
        private:
            Name        m_sNodeName;
            NodeOffset  m_nParentPos;
        };
    //-------------------------------------------------------------------------

        /** extract the value from a plain value
        */
        inline
        UnoAny getSimpleValue(Tree const& aTree, ValueRef const& aNode)
        { return aTree.getNodeValue( aNode ); }

    //-------------------------------------------------------------------------
        inline bool ValueRef::isValid() const
        {
            OSL_ASSERT( m_nParentPos == 0 || checkValidState() );
            return m_nParentPos != 0;
        }

    //-------------------------------------------------------------------------

        class SubNodeID
        {
        public:
            static SubNodeID createEmpty() { return SubNodeID(); }
            SubNodeID(Tree const& rTree, ValueRef const& rNode);
            SubNodeID(Tree const& rTree, NodeRef const& rParentNode, Name const& aName);
            SubNodeID(TreeRef const& rTree, NodeRef const& rParentNode, Name const& aName);
            SubNodeID(NodeID const& rParentNodeID, Name const& aName);

        // comparison
            // equality
            friend bool operator==(SubNodeID const& lhs, SubNodeID const& rhs)
            { return lhs.m_aParentID == rhs.m_aParentID && lhs.m_sNodeName == rhs.m_sNodeName; }
            // ordering
            friend bool operator < (SubNodeID const& lhs, SubNodeID const& rhs);
            // checking
            bool isEmpty() const;
            // checking
            bool isValidNode() const;
            // hashing
            size_t hashCode() const;
            // containing node this
            NodeID getParentID() const { return m_aParentID; }
            // containing node this
            Name getNodeName() const { return m_sNodeName; }
        private:
            SubNodeID(); // create an empty one
            friend class TreeImplHelper;
            Name    m_sNodeName;
            NodeID  m_aParentID;
        };
    //-------------------------------------------------------------------------

        typedef std::vector<SubNodeID>      SubNodeIDList;
        void getAllChildrenHelper(NodeID const& aNode, SubNodeIDList& aList);

    //-------------------------------------------------------------------------
        inline bool operator!=(SubNodeID const& lhs, SubNodeID const& rhs)
        { return !(lhs == rhs); }
        //---------------------------------------------------------------------

        inline bool operator>=(SubNodeID const& lhs, SubNodeID const& rhs)
        { return !(lhs < rhs); }
        //---------------------------------------------------------------------
        inline bool operator > (SubNodeID const& lhs, SubNodeID const& rhs)
        { return  (rhs < lhs); }
        inline bool operator<=(SubNodeID const& lhs, SubNodeID const& rhs)
        { return !(rhs < lhs); }
    //-------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGVALUENODE_HXX_
