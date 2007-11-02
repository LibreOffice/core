/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_node.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:37:47 $
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
#include <ary/doc/d_node.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_hypertext.hxx>



namespace ary
{
namespace doc
{



Node::~Node()
{
}

Node::Node(nodetype::id  i_type)
    :   nType(i_type),
        pNext(0)
{
}

void
Node::Add_toChain( DYN Node & pass_nextNode )
{
    if (NOT pNext)
        pNext = &pass_nextNode;
    else
        pNext->Add_toChain(pass_nextNode);
}

uintt
Node::ListSize() const
{
    return pNext
            ?   pNext->ListSize() + 1
            :   1;
}



}   // namespace doc
}   // namespace ary
