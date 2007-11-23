/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treesegment.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:26:23 $
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

#ifndef CONFIGMGR_TREESEGMENT_HXX
#define CONFIGMGR_TREESEGMENT_HXX

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif
#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif // INCLUDED_MEMORY


// -----------------------------------------------------------------------------
namespace rtl { class OUString; }
// -----------------------------------------------------------------------------
namespace configmgr
{
// -----------------------------------------------------------------------------
    class INode; // for RawTreeData
    namespace sharable { struct TreeFragment; union Node; } // for TreeData (sharable)
// -----------------------------------------------------------------------------
    namespace configuration { class Name; }
    // -------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class TreeAccessor;
    // -------------------------------------------------------------------------
        class TreeSegment
        {
            struct Impl;
            rtl::Reference<Impl> m_pImpl;
        public:
            typedef configuration::Name  Name;
            typedef std::auto_ptr<INode> RawTreeData;
            typedef rtl::OUString        RawName;

            typedef sharable::Node const         * NodeDataPtr;
            typedef sharable::TreeFragment const * TreeDataPtr;
            typedef sharable::TreeFragment       * TreeDataUpdatePtr;

            static TreeSegment createNew(RawTreeData _aTree, RawName const & _aTypeName)
            { return createNewSegment(_aTree,_aTypeName); }

            static TreeSegment createNew(RawName const & _aTreeName, RawTreeData _aTree)
            { return createNewSegment(_aTreeName, _aTree); }

            static TreeSegment createNew(TreeAccessor const & _aTree)
            { return createNewSegment(_aTree); }

            TreeSegment();
            TreeSegment(TreeSegment const & );
            TreeSegment& operator=(TreeSegment const & );
            ~TreeSegment();

            TreeSegment cloneSegment() const;

            bool is() const;
            void clear();

            Name getName() const;
            void setName(Name const & _aNewName);
            void markRemovable();

            TreeAddress     getBaseAddress() const;
            TreeAccessor    getTreeAccess() const;

         //   RawTreeData releaseData();
            void clearData() { clear(); }
            RawTreeData cloneData(bool bUseTreeName) const;

            TreeDataPtr getTreeData() const;
            NodeDataPtr getSegmentRootNode() const;
        private:
            TreeDataUpdatePtr getTreeDataForUpdate() const;

        private:
            bool hasData() const { return !!m_pImpl.is(); }
            static Impl* createNewSegment(RawName const & _aTreeName, RawTreeData& _aTree);
            static Impl* createNewSegment(RawTreeData& _aTree, RawName const & _aTypeName);
            static Impl* createNewSegment(TreeAccessor const & _aTree);
            TreeSegment(Impl *);
        };

    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_SEGMENT_HXX

