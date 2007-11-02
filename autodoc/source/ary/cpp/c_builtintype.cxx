/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_builtintype.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:23:39 $
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
#include <ary/cpp/c_builtintype.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_type.hxx>


namespace ary
{
namespace cpp
{




//**********************        Type        **************************//
Rid
Type::inq_RelatedCe() const
{
     return 0;
}


//**********************        BuiltInType        **************************//

BuiltInType::BuiltInType( const String  &       i_sName,
                          E_TypeSpecialisation  i_eSpecialisation )
    :   sName( i_sName ),
        eSpecialisation( i_eSpecialisation )
{
}

String
BuiltInType::SpecializedName_( const char *         i_sName,
                               E_TypeSpecialisation i_eTypeSpecialisation )
{
    StreamLock
        aStrLock(60);
    StreamStr &
        ret = aStrLock();

    switch ( i_eTypeSpecialisation )
    {
        case TYSP_unsigned:
                    ret << "u_";
                    break;
        case TYSP_signed:
                    if (strcmp(i_sName,"char") == 0)
                        ret << "s_";
                    break;
        default:
                    ;

    }   // end switch

    ret << i_sName;
    return String(ret.c_str());
}

void
BuiltInType::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ary::ClassId
BuiltInType::get_AryClass() const
{
    return class_id;
}

bool
BuiltInType::inq_IsConst() const
{
    return false;
}

void
BuiltInType::inq_Get_Text( StreamStr &      ,               // o_rPreName
                           StreamStr &      o_rName,
                           StreamStr &      ,               // o_rPostName
                           const Gate &     ) const         // i_rGate
{
    switch (eSpecialisation)
    {
        case TYSP_unsigned: o_rName << "unsigned "; break;
        case TYSP_signed:   o_rName << "signed ";   break;

        default:            // Does nothing.
                            ;
    }
    o_rName << sName;
}




}   // namespace cpp
}   // namespace ary
