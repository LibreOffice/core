/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defdescr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:20:15 $
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
#include "defdescr.hxx"



// NOT FULLY DEFINED SERVICES
#include <ary/cpp/prpr.hxx>


#if 0 // should be obsolete
bool                CheckForOperator(
                        bool &              o_bStringify,
                        bool &              o_bConcatenate,
                        const udmstri &     i_sTextItem );
void                Do_bConcatenate(
                        csv::StreamStr &    o_rText,
                        bool &              io_bConcatenate );
inline void         Do_bStringify_begin(
                        csv::StreamStr &    o_rText,
                        bool                i_bStringify );
inline void         Do_bStringify_end(
                        csv::StreamStr &    o_rText,
                        bool &              io_bStringify );
inline bool         HandleOperatorsBeforeTextItem(  /// @return true, if text item is done here
                        csv::StreamStr &    o_rText,
                        bool &              io_bStringify,
                        bool &              io_bConcatenate,
                        const udmstri &     i_sTextItem );

inline void
Do_bStringify_begin( csv::StreamStr & o_rText,
                     bool             i_bStringify )
{
    if ( i_bStringify )
    {
        o_rText << "\"";
    }
}

inline void
Do_bStringify_end( csv::StreamStr & o_rText,
                   bool &           io_bStringify )
{
    if ( io_bStringify )
    {
        o_rText << "\"";
        io_bStringify = false;
    }
}


inline bool
HandleOperatorsBeforeTextItem( csv::StreamStr &    o_rText,
                               bool &              io_bStringify,
                               bool &              io_bConcatenate,
                               const udmstri &     i_sTextItem )
{
    if ( CheckForOperator( io_bStringify,
                           io_bConcatenate,
                           i_sTextItem) )
    {
        return true;
    }
    Do_bConcatenate(o_rText, io_bConcatenate);
    Do_bStringify_begin(o_rText, io_bStringify);

    return false;
}
#endif // 0, obsolete



namespace cpp
{




DefineDescription::DefineDescription( const udmstri &       i_sName,
                                      const str_vector &    i_rDefinition )
    :   sName(i_sName),
        // aParams,
        aDefinition(i_rDefinition),
        eDefineType(type_define)
{
}

DefineDescription::DefineDescription( const udmstri &       i_sName,
                                      const str_vector &    i_rParams,
                                      const str_vector &    i_rDefinition )
    :   sName(i_sName),
        aParams(i_rParams),
        aDefinition(i_rDefinition),
        eDefineType(type_macro)
{
}

DefineDescription::~DefineDescription()
{
}

void
DefineDescription::GetDefineText( csv::StreamStr & o_rText ) const
{
    if ( aDefinition.begin() == aDefinition.end() OR eDefineType != type_define )
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

void
DefineDescription::GetMacroText( csv::StreamStr &               o_rText,
                                 const StringVector & i_rGivenArguments ) const
{
    bool bSwitch_Stringify = false;
    bool bSwitch_Concatenate = false;
    intt nActiveParamNr = -1;

    if ( aDefinition.begin() == aDefinition.end() OR eDefineType != type_macro )
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



}   // end namespace cpp





