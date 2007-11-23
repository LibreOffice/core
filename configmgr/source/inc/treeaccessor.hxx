/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeaccessor.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:25:13 $
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

#ifndef CONFIGMGR_TREEACCESSOR_HXX
#define CONFIGMGR_TREEACCESSOR_HXX

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#ifndef CONFIGMGR_BUILDDATA_HXX
#include <builddata.hxx>
#endif

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

