/*************************************************************************
 *
 *  $RCSfile: all_toks.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    udmstri             sText;
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
    udmstri             sText;
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
    udmstri             sText; \
}; \
const INT16 Tid_##name = tid

DECL_TOKEN_CLASS_WITHTEXT(DefineName,43);
DECL_TOKEN_CLASS_WITHTEXT(MacroName,44);
DECL_TOKEN_CLASS_WITHTEXT(MacroParameter,45);
DECL_TOKEN_CLASS_WITHTEXT(PreProDefinition,46);



/** char short int long float double wchar_t size_t
*/
DECL_TOKEN_CLASS_WITHTEXT(BuiltInType, 47);
/** signed unsigned
*/
DECL_TOKEN_CLASS_WITHTEXT(TypeSpecializer, 48);
DECL_TOKEN_CLASS_WITHTEXT(Constant, 49);



#if 0
/** char short int long float double wchar_t size_t
*/
class Tok_BuiltInType : public cpp::Token
{
  public:
                        Tok_BuiltInType(
                            const char *        i_sText ) : sText(i_sText) {}
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual INT16       TypeId() const;
    virtual const char *
                        Text() const;
  private:
    udmstri             sText;
};
const INT16 Tid_BuiltInType = 47;

/** signed unsigned
*/
class Tok_TypeSpecializer : public cpp::Token
{
  public:
                        Tok_TypeSpecializer(
                            const char *        i_sText ) : sText(i_sText) {}
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual INT16       TypeId() const;
    virtual const char *
                        Text() const;
  private:
    udmstri             sText;
};
const INT16 Tid_TypeSpecializer = 48;


class Tok_Constant : public cpp::Token
{
  public:
                        Tok_Constant(
                            const char *        i_sText ) : sText(i_sText) {}
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual INT16       TypeId() const;
    virtual const char *
                        Text() const;
  private:
    udmstri             sText;
};
const INT16 Tid_Constant = 49;

#endif // 0


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
    udmstri             sMacroName;
};




#if 0   // These tokens will not be created, but just ignored.
/// auto typename __cdecl __stdcall __fastcall
class Tok_Ignore : public cpp::Token
{
  public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};
#endif // 0






#if 0   // These tokens will not be created, but just ignored.
///  /* // /** /// #
class Tok_ContextChanger : public cpp::Token
{
  public:
    enum E_Id { id_ml_comment,  id_sl_comment,  id_ml_docu, id_sl_docu, id_preprocessor };
    typedef udm::Enum<E_Id> Ec_Id;

    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
    Ec_Id               Id() const { return eId; }

  private:
    Ec_Id               eId;
};
#endif // 0


#if 0 // just for viewing:
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
#endif // 0


}   // namespace cpp

#endif

