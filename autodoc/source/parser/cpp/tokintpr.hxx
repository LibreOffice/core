/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokintpr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:34:19 $
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
