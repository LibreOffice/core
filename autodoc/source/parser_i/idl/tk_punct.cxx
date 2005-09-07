/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_punct.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:52:27 $
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
#include <s2_luidl/tk_punct.hxx>


// NOT FULLY DECLARED SERVICES
#include <parser/parserinfo.hxx>
#include <s2_luidl/tokintpr.hxx>


using csi::uidl::TokPunctuation;


lux::EnumValueMap           G_aTokPunctuation_EV_TokenId_Values;
TokPunctuation::EV_TokenId  ev_none(TokPunctuation::e_none,"");
TokPunctuation::EV_TokenId  BracketOpen(TokPunctuation::BracketOpen,"(");
TokPunctuation::EV_TokenId  BracketClose(TokPunctuation::BracketClose,")");
TokPunctuation::EV_TokenId  ArrayBracketOpen(TokPunctuation::ArrayBracketOpen,"[");
TokPunctuation::EV_TokenId  ArrayBracketClose(TokPunctuation::ArrayBracketClose,"]");
TokPunctuation::EV_TokenId  CurledBracketOpen(TokPunctuation::CurledBracketOpen,"{");
TokPunctuation::EV_TokenId  CurledBracketClose(TokPunctuation::CurledBracketClose,"}");
TokPunctuation::EV_TokenId  Semicolon(TokPunctuation::Semicolon,";");
TokPunctuation::EV_TokenId  Colon(TokPunctuation::Colon,":");
TokPunctuation::EV_TokenId  DoubleColon(TokPunctuation::DoubleColon,"::");
TokPunctuation::EV_TokenId  Comma(TokPunctuation::Comma,",");
TokPunctuation::EV_TokenId  Minus(TokPunctuation::Minus,"-");
TokPunctuation::EV_TokenId  Fullstop(TokPunctuation::Fullstop,".");
TokPunctuation::EV_TokenId  Lesser(TokPunctuation::Lesser,"<");
TokPunctuation::EV_TokenId  Greater(TokPunctuation::Greater,">");




namespace lux
{
template<> EnumValueMap &
TokPunctuation::EV_TokenId::Values_()       { return G_aTokPunctuation_EV_TokenId_Values; }
}




namespace csi
{
namespace uidl
{

void
TokPunctuation::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Process_Punctuation(*this);
}

const char *
TokPunctuation::Text() const
{
    return eTag.Text();
}

void
Tok_EOL::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    io_rInterpreter.Process_EOL();
}

const char *
Tok_EOL::Text() const
{
    return "\r\n";
}

void
Tok_EOF::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    csv_assert(false);
//  io_rInterpreter.Process_EOF();
}

const char *
Tok_EOF::Text() const
{
    return "";
}


}   // namespace uidl
}   // namespace csi
