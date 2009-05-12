/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treefragment.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "treefragment.hxx"
#include "utility.hxx"
#include "attributes.hxx"
#include <rtl/ustring.hxx>

// memset
#include <string.h>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
//-----------------------------------------------------------------------------
rtl::OUString TreeFragment::getName() const
{
    return rtl::OUString(this->header.name);
}

void TreeFragment::setName(rtl::OUString const & name) {
    rtl_uString * old = header.name;
    header.name = acquireString(name);
    rtl_uString_release(old);
}

//-----------------------------------------------------------------------------

bool TreeFragment::isNamed(rtl::OUString const & _aName) const
{
    // TODO: optimize comparison
    return !!(this->getName() == _aName);
}
//-----------------------------------------------------------------------------

bool TreeFragment::hasDefaultsAvailable() const
{
    return (this->header.state & data::State::flag_default_avail) || isDefault();
}
//-----------------------------------------------------------------------------


bool TreeFragment::isDefault() const
{
    return (this->header.state & data::State::mask_state) == data::State::defaulted;
}
//-----------------------------------------------------------------------------

bool TreeFragment::isNew() const
{
    return (this->header.state & data::State::mask_state) == data::State::added;
}
//-----------------------------------------------------------------------------

configmgr::node::Attributes TreeFragment::getAttributes() const
{
    configmgr::node::Attributes aResult;

    switch (this->header.state & data::State::mask_state)
    {
    case data::State::merged:     aResult.setState(configmgr::node::isMerged);   break;
    case data::State::defaulted:  aResult.setState(configmgr::node::isDefault);  break;
    case data::State::replaced:   aResult.setState(configmgr::node::isReplaced); break;
    case data::State::added:      aResult.setState(configmgr::node::isAdded);    break;
    default: OSL_ASSERT(false); break; // not reachable
    }

    aResult.setRemovability(!!(this->header.state & data::State::flag_removable),
                            !!(this->header.state & data::State::flag_mandatory));


    OSL_ASSERT( header.count != 0 );
    NodeInfo const & aRootNodeInfo = this->nodes[0].info;

    aResult.setAccess(  !!(this->header.state & data::State::flag_readonly),
                        !!(aRootNodeInfo.flags & data::Flags::finalized)  );

    aResult.setLocalized ( !!(aRootNodeInfo.flags & data::Flags::localized));

    return aResult;
}

TreeFragment *TreeFragment::allocate(sal_uInt32 nFragments)
{
    sal_uInt32 nSize = sizeof(TreeFragment) + sizeof(Node) * (nFragments-1);
    sal_uInt8 *pMem = new sal_uInt8 [nSize];
    memset (pMem, 0, nSize);
    return reinterpret_cast<TreeFragment *>(pMem);
}

void TreeFragment::free_shallow(TreeFragment *pFragment )
{
    delete[] (sal_uInt8 *) pFragment;
}

//-----------------------------------------------------------------------------
    } // namespace sharable
//-----------------------------------------------------------------------------
} // namespace configmgr


