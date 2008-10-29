/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: builddata.hxx,v $
 * $Revision: 1.4 $
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

#ifndef CONFIGMGR_BUILDDATA_HXX
#define CONFIGMGR_BUILDDATA_HXX

#include "treefragment.hxx"
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
        sharable::TreeFragment * buildTree(sharable::TreeFragment * tree);
//-----------------------------------------------------------------------------
        sharable::TreeFragment * buildTree(rtl::OUString const & _aTreeName, INode const& _aNode, bool _bWithDefaults);
//-----------------------------------------------------------------------------
        sharable::TreeFragment * buildElementTree(INode const& _aNode, rtl::OUString const & _aTypeName, bool _bWithDefaults);
//-----------------------------------------------------------------------------
        void mergeDefaults(sharable::TreeFragment * _aBaseAddress, INode const& _aDefaultNode);
//-----------------------------------------------------------------------------
        void destroyTree(sharable::TreeFragment * _aBaseAddress);
//-----------------------------------------------------------------------------
        std::auto_ptr<INode> convertTree(sharable::TreeFragment * tree, bool _bUseTreeName);
//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}   // namespace configmgr
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_BUILDDATA_HXX


