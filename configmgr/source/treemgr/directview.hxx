/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: directview.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:42:13 $
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

#ifndef CONFIGMGR_DIRECTVIEW_HXX_
#define CONFIGMGR_DIRECTVIEW_HXX_

#ifndef CONFIGMGR_VIEWBEHAVIOR_HXX_
#include "viewstrategy.hxx"
#endif

#ifndef CONFIGMGR_TREESEGMENT_HXX
#include "treesegment.hxx"
#endif

namespace configmgr
{
    namespace view
    {
//-----------------------------------------------------------------------------
// View behavior for direct data access
//-----------------------------------------------------------------------------

        class DirectViewStrategy : public ViewStrategy
        {
            data::TreeSegment m_aTreeSegment;
        public:
            explicit
            DirectViewStrategy(data::TreeSegment const & _aTreeSegment)
            : m_aTreeSegment(_aTreeSegment)
            {}

        protected:
            // change handling -required
            virtual bool doHasChanges(Node const& _aNode) const;
            virtual void doMarkChanged(Node const& _aNode);

            // common attributes
            virtual node::Attributes doAdjustAttributes(node::Attributes const& _aAttributes) const;

            // group member access
            virtual ValueMemberNode doGetValueMember(GroupNode const& _aNode, Name const& _aName, bool _bForUpdate) const;

            // set element access
            virtual void doInsertElement(SetNode const& _aNode, Name const& aName, SetNodeEntry const& aNewEntry);
            virtual void doRemoveElement(SetNode const& _aNode, Name const& aName);

            virtual NodeFactory& doGetNodeFactory();

        private:
            void implMarkNondefault(SetNode const& _aNode);
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_DIRECTVIEW_HXX_
