/*************************************************************************
 *
 *  $RCSfile: collectchanges.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:12:03 $
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

#ifndef CONFIGMGR_COLLECTCHANGES_HXX_
#define CONFIGMGR_COLLECTCHANGES_HXX_

// low.level (cache model) changes (needed for change tree action class)
#include "change.hxx"

// pathes for accessors
#include "configpath.hxx"

// need TreeDepth and c_TreeDepthAll
#include "treeimpl.hxx"

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
            Name                    m_aContextTypeName;
            data::Accessor          m_aDataAccessor;
            TreeImpl*               m_pBaseTree;
            NodeOffset              m_nBaseNode;
            TreeDepth               m_nDepthLeft;

        public:
            /// Constructs a Visitor object, sets the output target list and context
            CollectChanges( NodeChangesInformation& rTargetList_,
                            data::Accessor const& _aDataAccessor,
                            TreeImpl& rStartTree_, NodeOffset nStartNode_,
                            TemplateHolder aElementTemplate_,
                            TreeDepth nMaxDepth = c_TreeDepthAll);

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
            CollectChanges( CollectChanges const& rBase, Path::Component const& rChildName, Name const& aSubTypeName );

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
