/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: d_struct.cxx,v $
 * $Revision: 1.3 $
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

#include <precomp.h>
#include <ary/doc/d_struct.hxx>


// NOT FULLY DEFINED SERVICES
#include <algorithm>


namespace ary
{
namespace doc
{





Struct::Struct( nodetype::id        i_id )
    :   Node(i_id),
        aElements()
{
}

Struct::~Struct()
{
}


struct GetSlotId
{
                        GetSlotId(
                            Struct::slot_id         i_slot )
                            :   nSlot(i_slot) {}

    bool                operator()(
                            const Node &            i_node ) const
                            { return i_node.Type() == nSlot; }

  private:
    Struct::slot_id     nSlot;
};

Node &
Struct::Add_Node( DYN Node & pass_node )
{
    NodeList::iterator
        itFind = std::find_if( aElements.begin(),
                               aElements.end(),
                               GetSlotId(pass_node.Type()) );
    if ( itFind == aElements.end() )
        aElements.push_back(pass_node);
    else
        (*itFind).Add_toChain(pass_node);
    return pass_node;
}

const Node *
Struct::Slot( slot_id i_slot ) const
{
    NodeList::const_iterator
        itFind = std::find_if( aElements.begin(),
                               aElements.end(),
                               GetSlotId(i_slot) );
    if ( itFind != aElements.end() )
        return &(*itFind);
    return 0;
}

Node *
Struct::Slot(slot_id i_slot)
{
    NodeList::iterator
        itFind = std::find_if( aElements.begin(),
                               aElements.end(),
                               GetSlotId(i_slot) );
    if ( itFind != aElements.end() )
        return &(*itFind);
    return 0;
}

void
Struct::do_Accept(csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor,*this);
}




}   // namespace doc
}   // namespace ary
