/*************************************************************************
 *
 *  $RCSfile: nodechange.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 16:08:06 $
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

#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#define CONFIGMGR_CONFIGCHANGE_HXX_

#include "apitypes.hxx"
#include "configexcept.hxx"

#include <vector>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
        class Name;
        class AbsolutePath;
        class RelativePath;
        class NodeRef;
        class NodeID;
        class Tree;
//-----------------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
//-----------------------------------------------------------------------------

        class NodeChange;
        class NodeChangeImpl;
        class NodeChangeLocation;
        class NodeChangeInformation;
        class NodeChangesInformation;
//-----------------------------------------------------------------------------

        /// represents a node position in some tree
        class NodeChange
        {
        public:
            /// constructs an empty (unchanging) node change
            NodeChange();
            /// constructs a  node change with a given implementation
            NodeChange(NodeChangeImpl* pImpl);
            /// copies a node change with reference semantics
            NodeChange(NodeChange const& rOther);
            /// copies a node change with reference semantics
            NodeChange& operator=(NodeChange const& rOther);
            /// swaps the contents of this with another NodeChange
            void swap(NodeChange& rOther);
            /// destroys a node change
            ~NodeChange();

            /// checks, if this may represent an actual change (might not be tested)
            bool maybeChange() const;
            /// checks, if this represents an actual change (PRE: must be tested)
            bool isChange() const;
            /// retrieve information about the changed data
            bool getChangeInfo(NodeChangeInformation& rInfo) const;
            /// retrieve information about what node is changed
            bool getChangeLocation(NodeChangeLocation& rLoc) const;

            /// test whether this would really be a change (as close as possible)
            NodeChange& test();
            NodeChange const& test() const;

            /// apply this change and check whether the target node changed
            NodeChange& apply();
            NodeChange const& apply() const;

            // retrieve the tree where the change is actually taking place
            Tree getBaseTree() const;
            // retrieve the node where the change is actually taking place
            NodeRef getBaseNode() const;

            // retrieve the tree where the change is actually taking place
            Tree getAffectedTree() const;
            // retrieve the node where the change is actually taking place
            NodeRef getAffectedNode() const;
            // identify the node where the change is actually taking place
            NodeID getAffectedNodeID() const;

            // identify the node where the change is actually taking place
            NodeID getChangedNodeID() const;

            // Comparison
            friend bool operator==(NodeChange const& lhs, NodeChange const& rhs)
            {
                return lhs.m_pImpl == rhs.m_pImpl;
            }
            friend bool operator!=(NodeChange const& lhs, NodeChange const& rhs)
            {
                return lhs.m_pImpl != rhs.m_pImpl;
            }

            /// provides access to the internal Implementation for related classes
            NodeChangeImpl* impl() const { return m_pImpl; }
        private:
            NodeChangeImpl* m_pImpl;
            void init(), deinit();
        };

        /** represents a collection of updates to nodes (identified by <type>NodeChange</type>s) within a hierarchy of config entries
        */
        class NodeChanges
        {
            typedef vector<NodeChange> ChangesList;
        public:
            typedef ChangesList::const_iterator Iterator;
            typedef ChangesList::iterator MutatingIterator;
        public:
            /// Constructs an empty collection of changes
            NodeChanges();

            /// checks whether there are any (non-empty) changes in this
            bool isEmpty() const;

            /// retrieves the total count of changes in this collection
            ChangesList::size_type getCount() const { return m_aChanges.size(); }

            /// retrieve information about the changed data
            bool getChangesInfo(NodeChangesInformation& rInfos) const;

            /// test all changes
            NodeChanges& test()             { implTest(); return *this; }
            NodeChanges const& test() const { implTest(); return *this; }

            /// apply all changes
            NodeChanges& apply()            { implApply(); return *this; }
            NodeChanges const& apply() const{ implApply(); return *this; }

            /// remove all changes known to be doing nothing from this collection.
            NodeChanges& compact();

            /** insert a change into this collection
                <p>if there is an existing change for this element, they are combine using <method>NodeChange::combine</method>
            */
            void add(NodeChange const& aChange);

            /// returns an STL-style iterator to the first element of the collection
            Iterator begin() const      { return m_aChanges.begin(); }
            MutatingIterator begin()    { return m_aChanges.begin(); }

            /// returns an STL-style iterator to past the last element of the collection
            Iterator end() const    { return m_aChanges.end(); }
            MutatingIterator end()  { return m_aChanges.end(); }

        private:
            void implTest() const;
            void implApply() const;
            ChangesList m_aChanges;
        };

    }
}

#endif // CONFIGMGR_CONFIGCHANGE_HXX_
