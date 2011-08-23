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
#include <all_toks.hxx>


// NOT FULLY DEFINED SERVICES
#include <cpp/ctokdeal.hxx>
#include "c_dealer.hxx"
#include "tokintpr.hxx"



namespace cpp {


void
Token::DealOut( ::TokenDealer & o_rDealer )
{
    // KORR_FUTURE HACK (casting to derivation cpp::TokenDealer)
    o_rDealer.AsDistributor()->Deal_CppCode(*this);
}


#define DEF_TOKEN_CLASS(name) \
void \
Tok_##name::Trigger( TokenInterpreter & io_rInterpreter ) const \
{ io_rInterpreter.Hdl_##name(*this); } \
INT16 \
Tok_##name::TypeId() const { return Tid_##name; } \
const char * \
Tok_##name::Text() const { return #name; }

#define DEF_TOKEN_CLASS_WITHTEXT(name, text ) \
void \
Tok_##name::Trigger( TokenInterpreter & io_rInterpreter ) const \
{ io_rInterpreter.Hdl_##name(*this); } \
INT16 \
Tok_##name::TypeId() const { return Tid_##name; } \
const char * \
Tok_##name::Text() const { return text; }


DEF_TOKEN_CLASS_WITHTEXT(Identifier,sText)
DEF_TOKEN_CLASS_WITHTEXT(Operator,sText)

DEF_TOKEN_CLASS(operator)
DEF_TOKEN_CLASS(class)
DEF_TOKEN_CLASS(struct)
DEF_TOKEN_CLASS(union)
DEF_TOKEN_CLASS(enum)
DEF_TOKEN_CLASS(typedef)
DEF_TOKEN_CLASS(public)
DEF_TOKEN_CLASS(protected)
DEF_TOKEN_CLASS(private)
DEF_TOKEN_CLASS(template)
DEF_TOKEN_CLASS(virtual)
DEF_TOKEN_CLASS(friend)
DEF_TOKEN_CLASS_WITHTEXT(Tilde,"~")
DEF_TOKEN_CLASS(const)
DEF_TOKEN_CLASS(volatile)
DEF_TOKEN_CLASS(extern)
DEF_TOKEN_CLASS(static)
DEF_TOKEN_CLASS(mutable)
DEF_TOKEN_CLASS(register)
DEF_TOKEN_CLASS(inline)
DEF_TOKEN_CLASS(explicit)
DEF_TOKEN_CLASS(namespace)
DEF_TOKEN_CLASS(using)
DEF_TOKEN_CLASS(throw)
DEF_TOKEN_CLASS_WITHTEXT(SwBracket_Left,"{")
DEF_TOKEN_CLASS_WITHTEXT(SwBracket_Right,"}")
DEF_TOKEN_CLASS_WITHTEXT(ArrayBracket_Left,"[")
DEF_TOKEN_CLASS_WITHTEXT(ArrayBracket_Right,"]")
DEF_TOKEN_CLASS_WITHTEXT(Bracket_Left,"(")
DEF_TOKEN_CLASS_WITHTEXT(Bracket_Right,")")
DEF_TOKEN_CLASS_WITHTEXT(DoubleColon,"::")
DEF_TOKEN_CLASS_WITHTEXT(Semicolon,";")
DEF_TOKEN_CLASS_WITHTEXT(Comma,",")
DEF_TOKEN_CLASS_WITHTEXT(Colon,":")
DEF_TOKEN_CLASS_WITHTEXT(Assign,"=")
DEF_TOKEN_CLASS_WITHTEXT(Less,"<")
DEF_TOKEN_CLASS_WITHTEXT(Greater,">")
DEF_TOKEN_CLASS_WITHTEXT(Asterix,"*")
DEF_TOKEN_CLASS_WITHTEXT(AmpersAnd,"&")
DEF_TOKEN_CLASS_WITHTEXT(Ellipse,"...")
DEF_TOKEN_CLASS(typename)

DEF_TOKEN_CLASS_WITHTEXT(DefineName,sText)
DEF_TOKEN_CLASS_WITHTEXT(MacroName,sText)
DEF_TOKEN_CLASS_WITHTEXT(MacroParameter,sText)
// DEF_TOKEN_CLASS_WITHTEXT(PreProDefinition,sText)

void
Tok_PreProDefinition::Trigger( TokenInterpreter &   io_rInterpreter ) const
{ io_rInterpreter.Hdl_PreProDefinition(*this); }

INT16
Tok_PreProDefinition::TypeId() const { return Tid_PreProDefinition; }

const char *
Tok_PreProDefinition::Text() const
{
    return sText;
}



DEF_TOKEN_CLASS_WITHTEXT(BuiltInType,sText)
DEF_TOKEN_CLASS_WITHTEXT(TypeSpecializer,sText)
DEF_TOKEN_CLASS_WITHTEXT(Constant,sText)

const char *
Tok_UnblockMacro::Text() const
{
    return sMacroName;
}

void
Tok_UnblockMacro::DealOut( ::TokenDealer & o_rDealer )
{
    // KORR_FUTURE HACK (casting to derivation cpp::TokenDealer)
    o_rDealer.AsDistributor()->Deal_Cpp_UnblockMacro(*this);
}

}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
