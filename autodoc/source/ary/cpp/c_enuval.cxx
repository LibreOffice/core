/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_enuval.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:30:50 $
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
#include <ary/cpp/c_enuval.hxx>


// NOT FULLY DECLARED SERVICES
#include "rcids.hxx"
#include <ary/cpp/cpp_disp.hxx>




namespace ary
{
namespace cpp
{


EnumValue::EnumValue()
//  :   aEssentials,
           // sInitialisation
{
}

EnumValue::EnumValue( Cid                 i_nId,
                      const udmstri &     i_sLocalName,
                      Cid                 i_nOwner,
                      udmstri             i_sInitialisation )
    :   aEssentials( i_nId,
                     i_sLocalName,
                     i_nOwner,
                     0 ),
           sInitialisation(i_sInitialisation)
{
}

EnumValue::~EnumValue()
{
}

Cid
EnumValue::inq_Id() const
{
    return aEssentials.Id();
}

const udmstri &
EnumValue::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
EnumValue::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
EnumValue::inq_Location() const
{
    return aEssentials.Location();
}

void
EnumValue::do_StoreAt( ary::Display & o_rOut ) const
{
    ary::cpp::Display * pD = dynamic_cast< ary::cpp::Display* >(&o_rOut);
    if (pD != 0)
    {
         pD->Display_EnumValue(*this);
    }
}

RCid
EnumValue::inq_RC() const
{
    return RC_();
}


const ary::Documentation &
EnumValue::inq_Info() const
{
    return aEssentials.Info();
}

void
EnumValue::do_Add_Documentation( DYN ary::Documentation & let_drInfo )
{
    aEssentials.SetInfo(let_drInfo);
}


}   // namespace cpp
}   // namespace ary



