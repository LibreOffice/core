/*************************************************************************
 *
 *  $RCSfile: i_constgroup.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:12:43 $
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
#include <ary/idl/i_constgroup.hxx>
#include <ary/idl/ik_constgroup.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/ihost_ce.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{


ConstantsGroup::ConstantsGroup( const String &      i_sName,
                                Ce_id               i_nModule )
    :   sName(i_sName),
        nModule(i_nModule),
        aConstants()
{
}

ConstantsGroup::~ConstantsGroup()
{
}

void
ConstantsGroup::do_Visit_CeHost( CeHost & o_rHost ) const
{
    o_rHost.Do_ConstantsGroup(*this);
}

RCid
ConstantsGroup::inq_ClassId() const
{
    return class_id;
}

const String &
ConstantsGroup::inq_LocalName() const
{
    return sName;
}

Ce_id
ConstantsGroup::inq_NameRoom() const
{
    return nModule;
}

Ce_id
ConstantsGroup::inq_Owner() const
{
    return nModule;
}

E_SightLevel
ConstantsGroup::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_constgroup
{

inline const ConstantsGroup &
constgroup_cast( const CodeEntity & i_ce )
{
    csv_assert( i_ce.ClassId() == ConstantsGroup::class_id );
    return static_cast< const ConstantsGroup& >(i_ce);
}

void
attr::Get_Constants( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(constgroup_cast(i_ce).aConstants);
}

} // namespace ifc_constgroup


}   //  namespace   idl
}   //  namespace   ary
