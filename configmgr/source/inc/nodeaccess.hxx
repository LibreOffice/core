/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodeaccess.hxx,v $
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

#ifndef CONFIGMGR_NODEACCESS_HXX
#define CONFIGMGR_NODEACCESS_HXX

#include "node.hxx"
#include "valuenode.hxx"
#include "configpath.hxx"

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

