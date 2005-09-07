/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_define.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:30:15 $
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
#include <ary/cpp/c_define.hxx>


// NOT FULLY DECLARED SERVICES
#include "rcids.hxx"
#include <ary/cpp/cpp_disp.hxx>
#include <ary/cpp/prpr.hxx>




namespace ary
{
namespace cpp
{


Define::Define( Did                 i_nId,
                const udmstri &     i_sName,
                const str_vector &  i_rDefinition,
                Lid                 i_nDeclaringFile )
    :   CppDefinition(i_nId, i_sName, i_nDeclaringFile),
        aDefinition(i_rDefinition)
{
}

Define::~Define()
{
}

void
Define::do_StoreAt( ary::Display & o_rOut ) const
{
    ary::cpp::Display *  pD = dynamic_cast< ary::cpp::Display* >(&o_rOut);
    if (pD != 0)
    {
         pD->Display_Define(*this);
    }
}

RCid
Define::inq_RC() const
{
    return RCID_DEFINE;
}

const CppDefinition::str_vector &
Define::inq_DefinitionText() const
{
     return aDefinition;
}

void
Define::GetText( StreamStr & o_rText ) const
{
    if ( aDefinition.begin() == aDefinition.end() )
        return;


    bool bSwitch_Stringify = false;
    bool bSwitch_Concatenate = false;

    for ( str_vector::const_iterator it = aDefinition.begin();
          it != aDefinition.end();
          ++it )
    {
        if ( HandleOperatorsBeforeTextItem( o_rText,
                                            bSwitch_Stringify,
                                            bSwitch_Concatenate,
                                            *it ) )
        {
            continue;
        }

        o_rText << (*it);

        Do_bStringify_end(o_rText, bSwitch_Stringify);
        o_rText << " ";
    }
    o_rText.seekp(-1, csv::cur);
}




}   // namespace cpp
}   // namespace ary



