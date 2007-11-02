/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_explicit.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:56:44 $
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
#include "it_explicit.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include "i_strconst.hxx"
#include "it_xnameroom.hxx"



namespace ary
{
namespace idl
{


ExplicitType::ExplicitType( const String &      i_sName,
                            Type_id             i_nXNameRoom,
                            Ce_id               i_nModuleOfOccurrence,
                            Type_id             i_nTemplateType )
    :   Named_Type(i_sName),
        nXNameRoom(i_nXNameRoom),
        nModuleOfOccurrence(i_nModuleOfOccurrence),
        nTemplateType(i_nTemplateType)
{
}

ExplicitType::~ExplicitType()
{
}

ClassId
ExplicitType::get_AryClass() const
{
    return class_id;
}

void
ExplicitType::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
ExplicitType::inq_Get_Text( StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequenceCount,
                            const Gate &        i_rGate ) const
{
    const ExplicitNameRoom &
        rNameRoom = i_rGate.Types().Find_XNameRoom(nXNameRoom);
    rNameRoom.Get_Text(o_module,o_name,o_nRelatedCe,o_nSequenceCount,i_rGate);

    o_name = Name();
}

Type_id
ExplicitType::inq_TemplateParameterType() const
{
    return nTemplateType;
}


}   // namespace idl
}   // namespace ary
