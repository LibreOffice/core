/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodeaccess.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:20:54 $
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

#ifndef CONFIGMGR_NODEACCESS_HXX
#define CONFIGMGR_NODEACCESS_HXX

#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
        // -------------------------------------------------------------------------
        class NodeAccess
        {
        public:
             NodeAccess(const sharable::Node *_pNode)
                 : m_pData((sharable::Node *)_pNode) {}

             bool isValid() const { return m_pData != NULL; }

             configuration::Name getName() const { return wrapName( m_pData->getName() ); }
             static configuration::Name wrapName(rtl::OUString const& _aNameString)
                 { return configuration::makeName( _aNameString, configuration::Name::NoValidate() ); }

            // make it look like a pointer ...
            operator sharable::Node *() const { return (sharable::Node *)m_pData; }
            operator const sharable::Node * () const { return m_pData; }
            sharable::Node* operator->() const { return (sharable::Node *)m_pData; }
            bool operator == (const sharable::Node *pData) const { return pData == m_pData; }
            bool operator != (const sharable::Node *pData) const { return pData != m_pData; }

        private:
            const sharable::Node * m_pData;
        };

    // -------------------------------------------------------------------------
    // helper - finds child or element
    NodeAccess  getSubnode(NodeAccess const & _aNode, configuration::Name const & _aName);
    NodeAddress getSubnodeAddress(NodeAddress const & _aNodeAddress, configuration::Name const & _aName);
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_NODEACCESS_HXX

