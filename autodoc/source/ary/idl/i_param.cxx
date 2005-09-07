/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_param.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:44:15 $
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
#include <ary/idl/i_param.hxx>


// NOT FULLY DEFINED SERVICES
#if ENABLE_UDM
#include <udm/tt_incl.hxx>
#endif // ENABLE_UDM



namespace ary
{
namespace idl
{


Parameter::Parameter()
    :   sName(),
        nType(0),
        eDirection(param_in)
{
}


Parameter::Parameter( const String &        i_sName,
                      Type_id               i_nType,
                      E_ParameterDirection  i_eDirection )
    :   sName(i_sName),
        nType(i_nType),
        eDirection(i_eDirection)
{
}

Parameter::~Parameter()
{
}


#if ENABLE_UDM
namespace
{
enum E_Data_Parameter
{
    mid_Name,
    mid_Type,
    mid_Direction,
    mid_MAX
};
}

void
Parameter::SetupUdmTraits_( udm::struct_traits<Parameter> & o_rTraits )
{
    o_rTraits.reserve( mid_MAX );
    udm::add_traits_member( o_rTraits, &Parameter::sName, mid_Name );
    udm::add_traits_member( o_rTraits, &Parameter::nType, mid_Type );
    udm::add_traits_member( o_rTraits, &Parameter::eDirection, mid_Direction );
}
#endif // ENABLE_UDM


}   //  namespace   idl
}   //  namespace   ary



#if ENABLE_UDM
IMPL_UDM_GET_TRAITS( ary::idl::Parameter );
#endif // ENABLE_UDM
