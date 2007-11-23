/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: builddata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:15:56 $
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

#ifndef CONFIGMGR_BUILDDATA_HXX
#define CONFIGMGR_BUILDDATA_HXX

#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif
#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY
#include <memory>
#endif
//-----------------------------------------------------------------------------
namespace rtl { class OUString; }
//-----------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------
    class INode;
    class ISubtree;
    class ValueNode;
//-----------------------------------------------------------------------------
    namespace sharable { struct TreeFragment; }
    namespace data
    {
//-----------------------------------------------------------------------------
        class TreeAccessor;
//-----------------------------------------------------------------------------
        TreeAddress buildTree(TreeAccessor const& _aTree);
//-----------------------------------------------------------------------------
        TreeAddress buildTree(rtl::OUString const & _aTreeName, INode const& _aNode, bool _bWithDefaults);
//-----------------------------------------------------------------------------
        TreeAddress buildElementTree(INode const& _aNode, rtl::OUString const & _aTypeName, bool _bWithDefaults);
//-----------------------------------------------------------------------------
        void mergeDefaults(TreeAddress _aBaseAddress, INode const& _aDefaultNode);
//-----------------------------------------------------------------------------
        void destroyTree(TreeAddress _aBaseAddress);
//-----------------------------------------------------------------------------
        std::auto_ptr<INode> convertTree(TreeAccessor const & _aTree, bool _bUseTreeName);
//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}   // namespace configmgr
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_BUILDDATA_HXX


