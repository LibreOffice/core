/*************************************************************************
 *
 *  $RCSfile: treefragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:38:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "treefragment.hxx"

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
//-----------------------------------------------------------------------------
rtl::OUString TreeFragment::getName(memory::Accessor const & _aAccessor) const
{
    return readString(_aAccessor,this->header.name);
}
//-----------------------------------------------------------------------------

bool TreeFragment::isNamed(rtl::OUString const & _aName, memory::Accessor const & _aAccessor) const
{
    // TODO: optimize comparison
    return !!(this->getName(_aAccessor) == _aName);
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

//-----------------------------------------------------------------------------
    } // namespace sharable
//-----------------------------------------------------------------------------
} // namespace configmgr


