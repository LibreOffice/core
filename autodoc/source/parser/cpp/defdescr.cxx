/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <precomp.h>
#include "defdescr.hxx"


// NOT FULLY DEFINED SERVICES
#include <prprpr.hxx>




namespace cpp
{




DefineDescription::DefineDescription( const String  &       i_sName,
                                      const str_vector &    i_rDefinition )
    :   sName(i_sName),
        // aParams,
        aDefinition(i_rDefinition),
        eDefineType(type_define)
{
}

DefineDescription::DefineDescription( const String  &       i_sName,
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





bool
CheckForOperator( bool &              o_bStringify,
                  bool &              o_bConcatenate,
                  const String &      i_sTextItem )
{
    if ( strcmp(i_sTextItem, "##") == 0 )
    {
        o_bConcatenate = true;
        return true;
    }
    else if ( strcmp(i_sTextItem, "#") == 0 )
    {
        o_bStringify = true;
        return true;
    }
    return false;
}

void
Do_bConcatenate( csv::StreamStr &    o_rText,
                 bool &              io_bConcatenate )
{
    if ( io_bConcatenate )
    {
        uintt nPos;
        for ( nPos = o_rText.tellp() - 1;
              nPos > 0 ? o_rText.c_str()[nPos] == ' ' : false;
              --nPos ) ;
        o_rText.seekp(nPos+1);
        io_bConcatenate = false;
    }
}

void
Do_bStringify_begin( csv::StreamStr & o_rText,
                     bool             i_bStringify )
{
    if ( i_bStringify )
    {
        o_rText << "\"";
    }
}

void
Do_bStringify_end( csv::StreamStr & o_rText,
                   bool &           io_bStringify )
{
    if ( io_bStringify )
    {
        o_rText << "\"";
        io_bStringify = false;
    }
}


bool
HandleOperatorsBeforeTextItem( csv::StreamStr &    o_rText,
                               bool &              io_bStringify,
                               bool &              io_bConcatenate,
                               const String  &     i_sTextItem )
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



