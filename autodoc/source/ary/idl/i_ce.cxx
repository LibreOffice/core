/*************************************************************************
 *
 *  $RCSfile: i_ce.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:12:39 $
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
#include <ary/idl/i_ce.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/ihost_ce.hxx>
#include <ary_i/codeinf2.hxx>
#include <getncast.hxx>


namespace ary
{
namespace idl
{

namespace
{
    const Ce_2s aConstCe2sDummy;
}



CodeEntity::CodeEntity()
    :   pDocu(0),
        p2s(0)
{
}

CodeEntity::~CodeEntity()
{
}

const Ce_2s &
CodeEntity::Secondaries() const
{
    if (p2s)
        return *p2s;
    return aConstCe2sDummy;
}

Ce_2s &
CodeEntity::Secondaries()
{
    if (p2s)
        return *p2s;
    p2s = Ce_2s::Create_(ClassId());
    return *p2s;
}

void
CodeEntity::Set_Docu( DYN ary::info::CodeInformation * pass_dpDocu )
{
    pDocu = pass_dpDocu;
}

void
CodeEntity::do_Visit(::ary::Host & o_rHost) const
{
    CeHost *
        pHost = ptr_cast( &o_rHost, T2T<CeHost>() );
     if ( pHost != 0 )
        do_Visit_CeHost(*pHost);
}


}   // namespace idl
}   // namespace ary


