/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodechange.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#define CONFIGMGR_CONFIGCHANGE_HXX_

#include "rtl/ref.hxx"

#include "configexcept.hxx"

#include <vector>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
        class AbsolutePath;
        class RelativePath;
        class NodeRef;
        class NodeID;
        class SubNodeID;
        class Tree;
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
            /// retrieve information about the changed data, appending to a sequence, returning the count
            sal_uInt32 getChangeInfos(NodeChangesInformation& rInfo) const;
            /// retrieve information about what node is changed
            bool getChangeLocation(NodeChangeLocation& rLoc) const;

            /// test whether this would really be a change (as close as possible)
            NodeChange& test();
            NodeChange const& test() const;

            /// apply this change and check whether the target node changed
            NodeChange& apply();
            NodeChange const& apply() const;

            // retrieve the tree where the change is actually taking place
            rtl::Reference< Tree > getAffectedTree() const;
            // retrieve the node where the change is actually taking place
            NodeRef getAffectedNode() const;

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
        public:
            /// Constructs an empty collection of changes
            NodeChanges();

            /// checks whether there are any (non-empty) changes in this
            bool isEmpty() const;

            /// retrieves the total count of changes in this collection
            std::vector<NodeChange>::size_type getCount() const { return m_aChanges.size(); }

            /// retrieve information about the changed data, appending to a sequence, returning the count
            sal_uInt32 getChangesInfos(NodeChangesInformation& rInfos) const;

            /// test all changes
            NodeChanges& test()             { implTest(); return *this; }
            NodeChanges const& test() const { implTest(); return *this; }

            /// remove all changes known to be doing nothing from this collection.
            NodeChanges& compact();

            /** insert a change into this collection
            */
            void add(NodeChange const& aChange);

            /// returns an STL-style iterator to the first element of the collection
            std::vector<NodeChange>::const_iterator begin() const       { return m_aChanges.begin(); }
            std::vector<NodeChange>::iterator begin()   { return m_aChanges.begin(); }

            /// returns an STL-style iterator to past the last element of the collection
            std::vector<NodeChange>::const_iterator end() const { return m_aChanges.end(); }
            std::vector<NodeChange>::iterator end() { return m_aChanges.end(); }

        private:
            void implTest() const;
            std::vector<NodeChange> m_aChanges;
        };

    }
}

#endif // CONFIGMGR_CONFIGCHANGE_HXX_
