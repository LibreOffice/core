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

#ifndef ADC_CPP_TOKINTPR_HXX
#define ADC_CPP_TOKINTPR_HXX



// USED SERVICES
    // BASE CLASSES
#include <all_toks.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp {



#define DECL_TOK_HANDLER(token) \
    void                Hdl_##token( \
                            const Tok_##token & i_rTok ) { Call_Handler(i_rTok); }

class TokenInterpreter
{
  public:
    virtual             ~TokenInterpreter() {}

                        DECL_TOK_HANDLER(Identifier)
                        DECL_TOK_HANDLER(Operator)
                        DECL_TOK_HANDLER(operator)
                        DECL_TOK_HANDLER(class)
                        DECL_TOK_HANDLER(struct)
                        DECL_TOK_HANDLER(union)
                        DECL_TOK_HANDLER(enum)
                        DECL_TOK_HANDLER(typedef)
                        DECL_TOK_HANDLER(public)
                        DECL_TOK_HANDLER(protected)
                        DECL_TOK_HANDLER(private)
                        DECL_TOK_HANDLER(template)
                        DECL_TOK_HANDLER(virtual)
                        DECL_TOK_HANDLER(friend)
                        DECL_TOK_HANDLER(Tilde)
                        DECL_TOK_HANDLER(const)
                        DECL_TOK_HANDLER(volatile)
                        DECL_TOK_HANDLER(extern)
                        DECL_TOK_HANDLER(static)
                        DECL_TOK_HANDLER(mutable)
                        DECL_TOK_HANDLER(register)
                        DECL_TOK_HANDLER(inline)
                        DECL_TOK_HANDLER(explicit)
                        DECL_TOK_HANDLER(namespace)
                        DECL_TOK_HANDLER(using)
                        DECL_TOK_HANDLER(throw)
                        DECL_TOK_HANDLER(SwBracket_Left)
                        DECL_TOK_HANDLER(SwBracket_Right)
                        DECL_TOK_HANDLER(ArrayBracket_Left)
                        DECL_TOK_HANDLER(ArrayBracket_Right)
                        DECL_TOK_HANDLER(Bracket_Left)
                        DECL_TOK_HANDLER(Bracket_Right)
                        DECL_TOK_HANDLER(DoubleColon)
                        DECL_TOK_HANDLER(Semicolon)
                        DECL_TOK_HANDLER(Comma)
                        DECL_TOK_HANDLER(Colon)
                        DECL_TOK_HANDLER(Assign)
                        DECL_TOK_HANDLER(Less)
                        DECL_TOK_HANDLER(Greater)
                        DECL_TOK_HANDLER(Asterix)
                        DECL_TOK_HANDLER(AmpersAnd)
                        DECL_TOK_HANDLER(Ellipse)
                        DECL_TOK_HANDLER(typename)
                        DECL_TOK_HANDLER(DefineName)
                        DECL_TOK_HANDLER(MacroName)
                        DECL_TOK_HANDLER(MacroParameter)
                        DECL_TOK_HANDLER(PreProDefinition)
                        DECL_TOK_HANDLER(BuiltInType)
                        DECL_TOK_HANDLER(TypeSpecializer)
                        DECL_TOK_HANDLER(Constant)

  protected:
    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok ) = 0;
};

#undef DECL_TOK_HANDLER


// IMPLEMENTATION


}   // namespace cpp


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
