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

#ifndef ADC_CPP_ALL_TOKS_HXX
#define ADC_CPP_ALL_TOKS_HXX

// USED SERVICES
    // BASE CLASSES
#include "cpp_tok.hxx"
    // COMPONENTS
    // PARAMETERS

namespace cpp {

class Tok_Identifier : public cpp::Token
{
  public:
                        Tok_Identifier(
                            const char *        i_sText ) : sText(i_sText) {}
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual INT16       TypeId() const;
    virtual const char *
                        Text() const;
  private:
    String              sText;
};
const INT16 Tid_Identifier = 1;

/** == != <= >=  && || !

    new delete sizeof typeid
    + - / %  ^ | << >>
    . ->  ?
    += -= *= /= %= &= |= ^= <<= >>=
*/
class Tok_Operator : public cpp::Token
{
  public:
                        Tok_Operator(
                            const char *        i_sText ) : sText(i_sText) {}
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual INT16       TypeId() const;
    virtual const char *
                        Text() const;
  private:
    String              sText;
};
const INT16 Tid_Operator = 2;



#define DECL_TOKEN_CLASS(name,tid) \
class Tok_##name : public cpp::Token \
{ public: \
    virtual void        Trigger( \
                            TokenInterpreter &  io_rInterpreter ) const; \
    virtual INT16       TypeId() const; \
    virtual const char * \
                        Text() const; \
}; \
const INT16 Tid_##name = tid

DECL_TOKEN_CLASS(operator,3);
DECL_TOKEN_CLASS(class,4);
DECL_TOKEN_CLASS(struct,5);
DECL_TOKEN_CLASS(union,6);
DECL_TOKEN_CLASS(enum,7);
DECL_TOKEN_CLASS(typedef,8);
DECL_TOKEN_CLASS(public,9);
DECL_TOKEN_CLASS(protected,10);
DECL_TOKEN_CLASS(private,11);
DECL_TOKEN_CLASS(template,12);
DECL_TOKEN_CLASS(virtual,13);
DECL_TOKEN_CLASS(friend,14);
DECL_TOKEN_CLASS(Tilde,15);
DECL_TOKEN_CLASS(const,16);
DECL_TOKEN_CLASS(volatile,17);
DECL_TOKEN_CLASS(extern,18);
DECL_TOKEN_CLASS(static,19);
DECL_TOKEN_CLASS(mutable,20);
DECL_TOKEN_CLASS(register,21);
DECL_TOKEN_CLASS(inline,22);
DECL_TOKEN_CLASS(explicit,23);
DECL_TOKEN_CLASS(namespace,24);
DECL_TOKEN_CLASS(using,25);
DECL_TOKEN_CLASS(throw,26);
DECL_TOKEN_CLASS(SwBracket_Left,27);
DECL_TOKEN_CLASS(SwBracket_Right,28);
DECL_TOKEN_CLASS(ArrayBracket_Left,29);
DECL_TOKEN_CLASS(ArrayBracket_Right,30);
DECL_TOKEN_CLASS(Bracket_Left,31);
DECL_TOKEN_CLASS(Bracket_Right,32);
DECL_TOKEN_CLASS(DoubleColon,33);
DECL_TOKEN_CLASS(Semicolon,34);
DECL_TOKEN_CLASS(Comma,35);
DECL_TOKEN_CLASS(Colon,36);
DECL_TOKEN_CLASS(Assign,37);
DECL_TOKEN_CLASS(Less,38);
DECL_TOKEN_CLASS(Greater,39);
DECL_TOKEN_CLASS(Asterix,40);
DECL_TOKEN_CLASS(AmpersAnd,41);
DECL_TOKEN_CLASS(Ellipse,42);
DECL_TOKEN_CLASS(typename,43);

#undef DECL_TOKEN_CLASS

#define DECL_TOKEN_CLASS_WITHTEXT(name,tid) \
class Tok_##name : public cpp::Token \
{ public: \
                        Tok_##name( \
                            const char *        i_sText ) : sText(i_sText) {} \
    virtual void        Trigger( \
                            TokenInterpreter &  io_rInterpreter ) const; \
    virtual INT16       TypeId() const; \
    virtual const char * \
                        Text() const; \
  private: \
    String              sText; \
}; \
const INT16 Tid_##name = tid



DECL_TOKEN_CLASS_WITHTEXT(DefineName,44);
DECL_TOKEN_CLASS_WITHTEXT(MacroName,45);
DECL_TOKEN_CLASS_WITHTEXT(MacroParameter,46);
DECL_TOKEN_CLASS_WITHTEXT(PreProDefinition,47);

/** char short int long float double wchar_t size_t
*/
DECL_TOKEN_CLASS_WITHTEXT(BuiltInType, 48);

/** signed unsigned
*/
DECL_TOKEN_CLASS_WITHTEXT(TypeSpecializer, 49);
DECL_TOKEN_CLASS_WITHTEXT(Constant, 50);



/** This token does nothing in C++ code. It is added by the
    internal macro-replacer to mark the position, where a
    define or macro becomes valid again, which was until then
    invalid, because the text was a replacement of this macro.
    ( Avoiding endless recursive macro replacement. )
*/
class Tok_UnblockMacro : public ::TextToken
{
  public:
                        Tok_UnblockMacro(
                            const char *        i_sMacroName ) : sMacroName(i_sMacroName) {}
    virtual const char* Text() const;

    virtual void        DealOut(
                            ::TokenDealer &     o_rDealer );
  private:
    String              sMacroName;
};



/* just for viewing:
class Tok_TypeKey : public cpp::Token                  // file-><type-PE>
class Tok_Template : public cpp::Token                 // file
class Tok_Namespace : public cpp::Token                // file
class Tok_Bracket : public cpp::Token                  // ueberall
class Tok_Separator : public cpp::Token                // ueberall

class Tok_Identifier : public cpp::Token               // ueberall
class Tok_NameSeparator : public cpp::Token            // Type, Name
class Tok_BuiltInType : public cpp::Token              // ueberall
class Tok_ConVol : public cpp::Token                   // class-><FuVa>
class Tok_StorageClass : public cpp::Token             // file-><type>,<FuVa>
class Tok_OperatorFunctionName : public cpp::Token     // class

class Tok_Protection : public cpp::Token               // class
class Tok_Virtual : public cpp::Token                  // class
class Tok_Friend : public cpp::Token                   // class
class Tok_Tilde : public cpp::Token                    // class, expression

class Tok_Ellipse : public cpp::Token                  // function-ParaList
class Tok_Assignment : public cpp::Token               // VariableDeclaraton, Function, Parameter
class Tok_Throw : public cpp::Token                    // function
class Tok_LessMore : public cpp::Token
class Tok_Operator : public cpp::Token                 // expression

class Tok_Ignore : public cpp::Token
class Tok_ContextChanger : public cpp::Token
*/

}   // namespace cpp

#endif
