/*************************************************************************
 *
 *  $RCSfile: groupnodeimpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-03 12:55:06 $
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

#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#define CONFIGMGR_GROUPNODEBEHAVIOR_HXX_

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif

#ifndef CONFIGMGR_VALUEMEMBERNODE_HXX_
#include "valuemembernode.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
    class ISubtree;
    class ValueNode;
    class SubtreeChange;
    class ValueChange;

    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Any UnoAny;
        typedef com::sun::star::uno::Type UnoType;

        class Name;
        class ValueChangeImpl;
//-----------------------------------------------------------------------------
// a visitor
//-----------------------------------------------------------------------------
        struct GroupMemberVisitor
        {
            enum Result { DONE, CONTINUE };
            virtual Result visit(ValueMemberNode const& aValue) = 0;
        };


// Specific type of nodes
//-----------------------------------------------------------------------------

        class GroupNodeImpl : public NodeImpl
        {
            ISubtree& m_rOriginal;
        public:
            explicit GroupNodeImpl(ISubtree& rOriginal);
            explicit GroupNodeImpl(GroupNodeImpl& rOriginal); // only for makeIndirect

            /// retrieve the name of the underlying node
            OUString getOriginalNodeName() const;

            /// does this hold a child value of the given name
            bool hasValue(Name const& aName) const;

            /// retrieve data for the child value of the given name
            ValueMemberNode getValue(Name const& aName)
            { return doGetValueMember(aName,false); }

            /// retrieve data for updating the child value of the given name
            ValueMemberUpdate getValueForUpdate(Name const& aName)
            { return ValueMemberUpdate( doGetValueMember(aName,true) ); }

            // visitor support
            GroupMemberVisitor::Result dispatchToValues(GroupMemberVisitor& aVisitor);

            // commit protocol
            std::auto_ptr<SubtreeChange> preCommitChanges();
            void finishCommit(SubtreeChange& rChanges) { doFinishCommit(rChanges); }
            void revertCommit(SubtreeChange& rChanges) { doRevertCommit(rChanges); }
            void failedCommit(SubtreeChange& rChanges) { doFailedCommit(rChanges); }

            /// Adjust the internal representation after external changes to the original data - build NodeChangeInformation objects for notification
            void adjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChanges, TreeImpl& rParentTree, NodeOffset nPos);
        protected:
            virtual std::auto_ptr<SubtreeChange> doPreCommitChanges();
            virtual void doFinishCommit(SubtreeChange& rChanges);
            virtual void doRevertCommit(SubtreeChange& rChanges);
            virtual void doFailedCommit(SubtreeChange& rChanges);

            virtual void doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const;
            virtual ValueChangeImpl* doAdjustToValueChange(Name const& aName, ValueChange const& rExternalChange);

        protected:
            virtual ValueMemberNode doGetValueMember(Name const& aName, bool bForUpdate) = 0;

            ValueNode* getOriginalValueNode(Name const& aName) const;
        protected:
        // NodeImpl implementation
            virtual Attributes      doGetAttributes() const;

        private:
            virtual NodeType::Enum  doGetType() const;
            virtual void            doDispatch(INodeHandler& rHandler);

        };

//-----------------------------------------------------------------------------

        // domain-specific 'dynamic_cast' replacements
        GroupNodeImpl&  AsGroupNode(NodeImpl& rNode);

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
