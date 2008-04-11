/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treeaccessor.hxx,v $
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

#ifndef CONFIGMGR_TREEACCESSOR_HXX
#define CONFIGMGR_TREEACCESSOR_HXX

#include "nodeaccess.hxx"
#include "treefragment.hxx"

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#include <builddata.hxx>

namespace configmgr
{
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        /** class that mediates access to the data of a tree fragment
        */
        class TreeAccessor
        {
        public:
            TreeAccessor(sharable::TreeFragment *_aTreeRef)
                : m_pTree(_aTreeRef) {}
            TreeAccessor(const sharable::TreeFragment * _pTree)
                : m_pTree((sharable::TreeFragment *)_pTree) {}

            inline configuration::Name getName() const
                { return configuration::makeName( m_pTree->getName(),
                                                  configuration::Name::NoValidate() ); }

            NodeAccess getRootNode() const
                { return NodeAccess(m_pTree ? const_cast<sharable::Node *>(m_pTree->nodes) : NULL); }

            TreeAddress copyTree() const
                { return data::buildTree(*this); }
            static void freeTree(TreeAddress _aTree)
                { data::destroyTree(_aTree); }

            // make it look like a pointer ...
            operator sharable::TreeFragment *() const { return (sharable::TreeFragment *)m_pTree; }
            sharable::TreeFragment* operator->() const { return m_pTree; }
            bool operator == (sharable::TreeFragment *pTree) const { return pTree == m_pTree; }
            bool operator != (sharable::TreeFragment *pTree) const { return pTree != m_pTree; }

        private:
            TreeAddress  m_pTree;
        };
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_TREEACCESSOR_HXX

