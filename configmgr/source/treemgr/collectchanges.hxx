/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: collectchanges.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_COLLECTCHANGES_HXX_
#define CONFIGMGR_COLLECTCHANGES_HXX_

// low.level (cache model) changes (needed for change tree action class)
#include "change.hxx"

// pathes for accessors
#include "configpath.hxx"

// need c_TreeDepthAll
#include "tree.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------
        class NodeChangeData;
        class NodeChangeLocation;
        class NodeChangeInformation;
        class NodeChangesInformation;
//-----------------------------------------------------------------------------

        /// Change model translation: Create a NodeChangeData from a ValueChange object
        bool convertNodeChange(NodeChangeData& aData_, ValueChange const& aChange_);
        /// Change model translation: Create a NodeChangeData from an AddNode change object
        bool convertNodeChange(NodeChangeData& aData_, AddNode const& aChange_);
        /// Change model translation: Create a NodeChangeData from a RemoveNode change object
        bool convertNodeChange(NodeChangeData& aData_, RemoveNode const& aChange_);

        /** Change Tree Visitor that appends all changes in the changes tree
         to a NodeChanges list, optionally restricted to a given depth
        */
        class CollectChanges : private ChangeTreeAction
        {
            NodeChangesInformation& m_rTargetList;
            RelativePath            m_aAccessor;
            rtl::OUString m_aContextTypeName;
            Tree*               m_pBaseTree;
            unsigned int                m_nBaseNode;
            unsigned int                m_nDepthLeft;

        public:
            /// Constructs a Visitor object, sets the output target list and context
            CollectChanges( NodeChangesInformation& rTargetList_,
                            Tree& rStartTree_, unsigned int nStartNode_,
                            rtl::Reference<Template> aElementTemplate_,
                            unsigned int nMaxDepth = c_TreeDepthAll);

            /// Adds a (translated) ValueChange to the target list
            void collectFrom(ValueChange const& aChange_);

            /// Adds a (translated) AddNode to the target list
            void collectFrom(AddNode const& aChange_);

            /// Adds a (translated) RemoveNode to the target list
            void collectFrom(RemoveNode const& aChange_);

            /// Appends (translated) Changes from the subtree to the target list, possibly with a depth limit
            void collectFrom(SubtreeChange const& aChange_);

            /// Appends the given Change and its descendants to the target list, possibly with a depth limit
            void collectFrom(Change const& aChange_)
            {
                this->applyToChange( aChange_ );
            }

            /// Appends the descendants of the given Change to the target list, possibly with a depth limit
            void collectFromChildren(SubtreeChange const& aParent_)
            {
                this->applyToChildren( aParent_ );
            }

        protected:
            /// Constructs a Visitor object for a child of another one's context
            CollectChanges( CollectChanges const& rBase, Path::Component const& rChildName, rtl::OUString const& aSubTypeName );

        private:
            // ChangeTreeAction implementations
            virtual void handle(ValueChange const& aValueNode);
            virtual void handle(AddNode const& aAddNode);
            virtual void handle(RemoveNode const& aRemoveNode);
            virtual void handle(SubtreeChange const& aSubtree);

            bool implSetLocation(NodeChangeLocation& rLocation_, Change const& aOriginal_, bool bSet_) const;
            void implAdd(NodeChangeInformation const& aChangeInfo_);
            Path::Component implGetNodeName(Change const& _aChange_) const;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_COLLECTCHANGES_HXX_
