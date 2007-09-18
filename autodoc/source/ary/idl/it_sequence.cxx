/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_sequence.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 13:39:22 $
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
#include "it_sequence.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_type.hxx>
#include "i_strconst.hxx"



namespace ary
{
namespace idl
{


Sequence::Sequence( Type_id i_nRelatedType )
    :   nRelatedType(i_nRelatedType)
{
}

Sequence::~Sequence()
{
}

void
Sequence::do_Visit( Host & ) const
{
    // yet unused.
}

RCid
Sequence::inq_ClassId() const
{
    return class_id;
}

void
Sequence::inq_Get_Text( StringVector &      o_module,
                        String &            o_name,
                        Ce_id &             o_nRelatedCe,
                        int &               o_nSequenceCount,
                        const Gate &        i_rGate ) const
{
    ++o_nSequenceCount;

    i_rGate.Types().Find_Type(nRelatedType)
            .Get_Text(  o_module,
                        o_name,
                        o_nRelatedCe,
                        o_nSequenceCount,
                        i_rGate );
}



}   // namespace idl
}   // namespace ary

