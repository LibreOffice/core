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
#include "cx_c_pp.hxx"



// NOT FULLY DECLARED SERVICES
#include "c_dealer.hxx"
#include <tokens/parseinc.hxx>
#include <x_parse.hxx>
#include "all_toks.hxx"


namespace cpp
{

Context_Preprocessor::Context_Preprocessor( TkpContext & i_rFollowUpContext )
    :   Cx_Base(&i_rFollowUpContext),
        pContext_Parent(&i_rFollowUpContext),
        pContext_PP_MacroParams( 0 ),
        pContext_PP_Definition( new Context_PP_Definition(i_rFollowUpContext) )
{
    pContext_PP_MacroParams = new Context_PP_MacroParams(*pContext_PP_Definition);
}

void
Context_Preprocessor::ReadCharChain( CharacterSource &  io_rText )
{
    jumpOverWhite( io_rText );
    jumpToWhite( io_rText );
    const char * sPP_Keyword = io_rText.CutToken();
    if ( strcmp(sPP_Keyword, "define") == 0 )
        ReadDefine(io_rText);
    else
        ReadDefault(io_rText);
}

void
Context_Preprocessor::AssignDealer( Distributor & o_rDealer )
{
    Cx_Base::AssignDealer(o_rDealer);
    pContext_PP_MacroParams->AssignDealer(o_rDealer);
    pContext_PP_Definition->AssignDealer(o_rDealer);
}

void
Context_Preprocessor::ReadDefault( CharacterSource & io_rText )
{
    int o_rCount_BackslashedLineBreaks = 0;
    jumpToEol(io_rText,o_rCount_BackslashedLineBreaks);
    for ( ; o_rCount_BackslashedLineBreaks > 0; --o_rCount_BackslashedLineBreaks )
        Dealer().Deal_Eol();

    if (io_rText.CurChar() != NULCH)
        jumpOverEol(io_rText);
    io_rText.CutToken();
    Dealer().Deal_Eol();
    SetNewToken(0);
    SetFollowUpContext( pContext_Parent );
}

void
Context_Preprocessor::ReadDefine( CharacterSource & io_rText )
{
    jumpOverWhite( io_rText );
    io_rText.CutToken();

    char cNext = '\0';
    for ( cNext = io_rText.CurChar();
          static_cast<UINT8>(cNext) > 32 AND cNext != '(';
          cNext = io_rText.MoveOn() )
    { }

    bool bMacro = cNext == '(';

    if ( NOT bMacro )
    {
        SetNewToken( new Tok_DefineName(io_rText.CutToken()) );
        SetFollowUpContext( pContext_PP_Definition.Ptr() );
    }
    else
    {
        SetNewToken( new Tok_MacroName(io_rText.CutToken()) );
        io_rText.MoveOn();
        io_rText.CutToken();
        SetFollowUpContext( pContext_PP_MacroParams.Ptr() );
    }
}


Context_PP_MacroParams::Context_PP_MacroParams( Cx_Base & i_rFollowUpContext )
    :   Cx_Base(&i_rFollowUpContext),
        pContext_PP_Definition(&i_rFollowUpContext)
{
}

void
Context_PP_MacroParams::ReadCharChain( CharacterSource & io_rText )
{
    uintt nLen;

    jumpOverWhite( io_rText );
    // KORR_FUTURE Handling line breaks within macro parameters:
    char cSeparator = jumpTo( io_rText, ',', ')' );
    csv_assert( cSeparator != 0 );

    static char cBuf[500];
    // KORR_FUTURE, make it still safer, here:
    strcpy( cBuf, io_rText.CutToken() );    // SAFE STRCPY (#100211# - checked)
    for ( nLen = strlen(cBuf);
          nLen > 0 AND cBuf[nLen-1] < 33;
          --nLen )
    { }
    cBuf[nLen] = '\0';
    SetNewToken( new Tok_MacroParameter(cBuf) );

    io_rText.MoveOn();
    io_rText.CutToken();
    if ( cSeparator == ',')
        SetFollowUpContext( this );
    else    // Must be ')'
        SetFollowUpContext( pContext_PP_Definition );
}

Context_PP_Definition::Context_PP_Definition( TkpContext & i_rFollowUpContext )
    :   Cx_Base(&i_rFollowUpContext),
        pContext_Parent(&i_rFollowUpContext)
{
}

void
Context_PP_Definition::ReadCharChain( CharacterSource & io_rText )
{
    int o_rCount_BackslashedLineBreaks = 0;
    jumpToEol(io_rText,o_rCount_BackslashedLineBreaks);
    for ( ; o_rCount_BackslashedLineBreaks > 0; --o_rCount_BackslashedLineBreaks )
        Dealer().Deal_Eol();
    SetNewToken( new Tok_PreProDefinition(io_rText.CutToken()) );
    if (io_rText.CurChar() != NULCH)
        jumpOverEol(io_rText);
    Dealer().Deal_Eol();
}


}   // namespace cpp









