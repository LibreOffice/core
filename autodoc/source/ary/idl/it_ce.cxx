/*************************************************************************
 *
 *  $RCSfile: it_ce.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 13:30:19 $
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
Ce_Type::do_Visit( Host & io_rHost ) const
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
                       int &               o_nSequemceCount,
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
