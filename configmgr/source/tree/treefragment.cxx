/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treefragment.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:33:40 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "treefragment.hxx"

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
    return readString(this->header.name);
}
//-----------------------------------------------------------------------------

bool TreeFragment::isNamed(rtl::OUString const & _aName) const
{
    // TODO: optimize comparison
    return !!(this->getName() == _aName);
}
//-----------------------------------------------------------------------------

bool TreeFragment::hasDefaults() const
{
    switch (this->header.state & State::mask_state)
    {
    default: OSL_ASSERT(false); // not reachable

    case State::merged:
    case State::defaulted:  return true;

    case State::replaced:
    case State::added:      return false;
    }
}
//-----------------------------------------------------------------------------

bool TreeFragment::hasDefaultsAvailable() const
{
    return (this->header.state & State::flag_default_avail) || isDefault();
}
//-----------------------------------------------------------------------------


bool TreeFragment::isDefault() const
{
    return (this->header.state & State::mask_state) == State::defaulted;
}
//-----------------------------------------------------------------------------

bool TreeFragment::isNew() const
{
    return (this->header.state & State::mask_state) == State::added;
}
//-----------------------------------------------------------------------------

configmgr::node::Attributes TreeFragment::getAttributes() const
{
    configmgr::node::Attributes aResult;

    switch (this->header.state & State::mask_state)
    {
    case State::merged:     aResult.setState(configmgr::node::isMerged);   break;
    case State::defaulted:  aResult.setState(configmgr::node::isDefault);  break;
    case State::replaced:   aResult.setState(configmgr::node::isReplaced); break;
    case State::added:      aResult.setState(configmgr::node::isAdded);    break;
    default: OSL_ASSERT(false); break; // not reachable
    }

    aResult.setRemovability(!!(this->header.state & State::flag_removable),
                            !!(this->header.state & State::flag_mandatory));


    OSL_ASSERT( header.count != 0 );
    NodeInfo const & aRootNodeInfo = this->nodes[0].node.info;

    aResult.setAccess(  !!(this->header.state & State::flag_readonly),
                        !!(aRootNodeInfo.flags & Flags::finalized)  );

    aResult.setLocalized ( !!(aRootNodeInfo.flags & Flags::localized));

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


