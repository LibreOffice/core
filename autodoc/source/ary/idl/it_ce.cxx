/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_ce.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:52:28 $
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
#include "it_ce.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ip_ce.hxx>
#include "i_strconst.hxx"



namespace ary
{
namespace idl
{


Ce_Type::Ce_Type( Ce_id     i_nRelatedCe,
                  Type_id   i_nTemplateType )
    :   nRelatedCe(i_nRelatedCe),
        nTemplateType(i_nTemplateType)
{
}

Ce_Type::~Ce_Type()
{
}

void
Ce_Type::do_Visit( Host & ) const
{
    // yet unused.
}

RCid
Ce_Type::inq_ClassId() const
{
    return class_id;
}

void
Ce_Type::inq_Get_Text( StringVector &      o_module,
                       String &            o_name,
                       Ce_id &             o_nRelatedCe,
                       int &               ,
                       const Gate &        i_rGate ) const
{
    String sDummyMember;

    const CodeEntity &
        rCe = i_rGate.Ces().Find_Ce(nRelatedCe);
    i_rGate.Ces().Get_Text( o_module,
                            o_name,
                            sDummyMember,
                            rCe );
    o_nRelatedCe = nRelatedCe;
}

Type_id
Ce_Type::inq_TemplateParameterType() const
{
    return nTemplateType;
}


}   // namespace idl
}   // namespace ary
