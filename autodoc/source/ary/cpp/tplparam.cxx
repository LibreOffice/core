/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tplparam.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:35:49 $
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
#include "tplparam.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_type.hxx>


namespace ary
{
namespace cpp
{
namespace ut
{

TplParameter_Type::TplParameter_Type( Tid i_nType )
    :   nType(i_nType)
{
}

TplParameter_Type::~TplParameter_Type()
{
}

intt
TplParameter_Type::Compare( const TemplateParameter & i_rOther ) const
{
    const TplParameter_Type * pOther
            = dynamic_cast< const TplParameter_Type* >( &i_rOther );
    if (pOther == 0)
        return -1;

    return static_cast<long>(nType.Value())
           - static_cast<long>(pOther->nType.Value());
}

void
TplParameter_Type::Get_Text( StreamStr &                    o_rOut,
                             const ary::cpp::Gate & i_rGate ) const
{
    i_rGate.Types().Get_TypeText( o_rOut, nType );
}

TplParameter_Const::TplParameter_Const( const String  & i_sConst )
    :   sConstant(i_sConst)
{
}

TplParameter_Const::~TplParameter_Const()
{
}

intt
TplParameter_Const::Compare( const TemplateParameter & i_rOther ) const
{
    const TplParameter_Const * pOther
            = dynamic_cast< const TplParameter_Const* >( &i_rOther );
    if (pOther == 0)
        return +1;

    return strcmp( sConstant.c_str(), pOther->sConstant.c_str() );
}

void
TplParameter_Const::Get_Text( StreamStr &               o_out,
                              const ary::cpp::Gate &    ) const
{
    o_out << sConstant;
}



}   // namespace ut
}   // namespace cpp
}   // namespace ary
