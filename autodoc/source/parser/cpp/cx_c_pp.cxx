/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_c_pp.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:49:27 $
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









