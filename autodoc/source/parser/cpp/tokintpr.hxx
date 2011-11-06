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
