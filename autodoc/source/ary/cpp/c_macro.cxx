/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_macro.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 13:25:51 $
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
#include <ary/cpp/c_macro.hxx>


// NOT FULLY DECLARED SERVICES
#include "rcids.hxx"
#include <ary/cpp/cpp_disp.hxx>
#include <ary/cpp/prpr.hxx>





namespace ary
{
namespace cpp
{


Macro::Macro( Did                 i_nId,
              const udmstri &     i_sName,
              const str_vector &  i_rParams,
              const str_vector &  i_rDefinition,
              Lid                 i_nDeclaringFile )
    :   CppDefinition(i_nId, i_sName, i_nDeclaringFile),
        aParams(i_rParams),
        aDefinition(i_rDefinition)
{
}

Macro::~Macro()
{
}

void
Macro::do_StoreAt( ary::Display & o_rOut ) const
{
    ary::cpp::Display *  pD = dynamic_cast< ary::cpp::Display* >(&o_rOut);
    if (pD != 0)
    {
         pD->Display_Macro(*this);
    }
}

RCid
Macro::inq_RC() const
{
    return RCID_MACRO;
}

const CppDefinition::str_vector &
Macro::inq_DefinitionText() const
{
     return aDefinition;
}

void
Macro::GetText( StreamStr &                     o_rText,
                const StringVector &  i_rGivenArguments ) const
{
    bool bSwitch_Stringify = false;
    bool bSwitch_Concatenate = false;
    intt nActiveParamNr = -1;

    if ( aDefinition.begin() == aDefinition.end() )
        return;

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

        for ( str_vector::const_iterator param_it = aParams.begin();
              param_it != aParams.end() AND nActiveParamNr == -1;
              ++param_it )
        {
             if ( strcmp(*it, *param_it) == 0 )
                nActiveParamNr = param_it - aParams.begin();
        }
        if ( nActiveParamNr == -1 )
        {
            o_rText << (*it);
        }
        else
        {
            o_rText << i_rGivenArguments[nActiveParamNr];
            nActiveParamNr = -1;
        }

        Do_bStringify_end(o_rText, bSwitch_Stringify);
        o_rText << " ";
    }
    o_rText.seekp(-1, csv::cur);
}




}   // namespace cpp
}   // namespace ary



