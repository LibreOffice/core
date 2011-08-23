/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <precomp.h>
#include <s2_luidl/tk_punct.hxx>


// NOT FULLY DECLARED SERVICES
#include <parser/parserinfo.hxx>
#include <s2_luidl/tokintpr.hxx>


using csi::uidl::TokPunctuation;


lux::EnumValueMap			G_aTokPunctuation_EV_TokenId_Values;
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
TokPunctuation::EV_TokenId::Values_()		{ return G_aTokPunctuation_EV_TokenId_Values; }
}




namespace csi
{
namespace uidl
{

void
TokPunctuation::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_Punctuation(*this);
}

const char *
TokPunctuation::Text() const
{
    return eTag.Text();
}

void
Tok_EOL::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_EOL();
}

const char *
Tok_EOL::Text() const
{
    return "\r\n";
}

void
Tok_EOF::Trigger( TokenInterpreter & ) const
{
    csv_assert(false);
//	io_rInterpreter.Process_EOF();
}

const char *
Tok_EOF::Text() const
{
    return "";
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
