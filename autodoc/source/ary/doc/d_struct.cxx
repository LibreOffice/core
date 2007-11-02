/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_struct.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:38:34 $
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
