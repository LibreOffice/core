/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treesegment.hxx,v $
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

#ifndef CONFIGMGR_TREESEGMENT_HXX
#define CONFIGMGR_TREESEGMENT_HXX

#include "valuenode.hxx"
#include <rtl/ref.hxx>
#include "treefragment.hxx"
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

