/*************************************************************************
 *
 *  $RCSfile: tplparam.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:18 $
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
#include "tplparam.hxx"


// NOT FULLY DEFINED SERVICES
#include <cpp/c_gate.hxx>


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

    return nType - pOther->nType;
}

void
TplParameter_Type::Get_Text( StreamStr &                    o_rOut,
                             const ary::cpp::DisplayGate &  i_rGate ) const
{
    i_rGate.Get_TypeText( o_rOut, nType );
}

TplParameter_Const::TplParameter_Const( const udmstri & i_sConst )
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
TplParameter_Const::Get_Text( StreamStr &                   o_rOut,
                              const ary::cpp::DisplayGate & i_rGate ) const
{
    o_rOut << sConstant;
}



}   // namespace ut
}   // namespace cpp
}   // namespace ary


