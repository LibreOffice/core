/*************************************************************************
 *
 *  $RCSfile: pe_class.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-15 18:46:06 $
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

#include <precomp.h>
#include "pe_class.hxx"


// NOT FULLY DECLARED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/cpp/c_rwgate.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <all_toks.hxx>
#include "pe_base.hxx"
#include "pe_enum.hxx"
#include "pe_tydef.hxx"
#include "pe_vafu.hxx"
#include "pe_ignor.hxx"


namespace cpp {

// using ary::Cid;

PE_Class::PE_Class(Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Class> ),
        // pSpBase,
        // pSpTypedef,
        // pSpVarFunc,
        // pSpIgnore,
        // pSpuBase,
        // pSpuTypedef,
        // pSpuVarFunc,
        // pSpuUsing,
        // pSpuIgnoreFailure,
        // sLocalName,
        eClassKey(ary::cpp::CK_class),
        pCurObject(0),
        // aBases,
        eResult_KindOf(is_declaration)
{
    Setup_StatusFunctions();

    pSpBase     = new SP_Base(*this);
    pSpTypedef  = new SP_Typedef(*this);
    pSpVarFunc  = new SP_VarFunc(*this);
    pSpIgnore   = new SP_Ignore(*this);

    pSpuBase    = new SPU_Base(*pSpBase, 0, &PE_Class::SpReturn_Base);
    pSpuTypedef = new SPU_Typedef(*pSpTypedef, 0, 0);
    pSpuVarFunc = new SPU_VarFunc(*pSpVarFunc, 0, 0);

    pSpuTemplate= new SPU_Ignore(*pSpIgnore, 0, 0);
    pSpuUsing   = new SPU_Ignore(*pSpIgnore, 0, 0);
    pSpuIgnoreFailure
                = new SPU_Ignore(*pSpIgnore, 0, 0);
}


PE_Class::~PE_Class()
{
}

void
PE_Class::Call_Handler( const cpp::Token &  i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

Cpp_PE *
PE_Class::Handle_ChildFailure()
{
    SetCurSPU(pSpuIgnoreFailure.Ptr());
    return &pSpuIgnoreFailure->Child();
}

void
PE_Class::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Class>::F_Tok   F_Tok;

    static F_Tok stateF_start[] =           { &PE_Class::On_start_class,
                                              &PE_Class::On_start_struct,
                                              &PE_Class::On_start_union };
    static INT16 stateT_start[] =           { Tid_class,
                                              Tid_struct,
                                              Tid_union };

    static F_Tok stateF_expectName[] =      { &PE_Class::On_expectName_Identifier,
                                              &PE_Class::On_expectName_SwBracket_Left,
                                              &PE_Class::On_expectName_Colon
                                            };
    static INT16 stateT_expectName[] =      { Tid_Identifier,
                                              Tid_SwBracket_Left,
                                              Tid_Colon
                                            };

    static F_Tok stateF_gotName[] =         { &PE_Class::On_gotName_SwBracket_Left,
                                              &PE_Class::On_gotName_Semicolon,
                                              &PE_Class::On_gotName_Colon };
    static INT16 stateT_gotName[] =         { Tid_SwBracket_Left,
                                              Tid_Semicolon,
                                              Tid_Colon };

    static F_Tok stateF_bodyStd[] =         { &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_ClassKey,
                                              &PE_Class::On_bodyStd_ClassKey,
                                              &PE_Class::On_bodyStd_ClassKey,

                                              &PE_Class::On_bodyStd_enum,
                                              &PE_Class::On_bodyStd_typedef,
                                              &PE_Class::On_bodyStd_public,
                                              &PE_Class::On_bodyStd_protected,
                                              &PE_Class::On_bodyStd_private,

                                              &PE_Class::On_bodyStd_template,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_friend,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc,

                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc,

                                              &PE_Class::On_bodyStd_using,
                                              &PE_Class::On_bodyStd_SwBracket_Right,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc,
                                              &PE_Class::On_bodyStd_VarFunc };
    static INT16 stateT_bodyStd[] =         { Tid_Identifier,
                                              Tid_operator,
                                              Tid_class,
                                              Tid_struct,
                                              Tid_union,

                                              Tid_enum,
                                              Tid_typedef,
                                              Tid_public,
                                              Tid_protected,
                                              Tid_private,

                                              Tid_template,
                                              Tid_virtual,
                                              Tid_friend,
                                              Tid_Tilde,
                                              Tid_const,

                                              Tid_volatile,
                                              Tid_static,
                                              Tid_mutable,
                                                Tid_inline,
                                              Tid_explicit,

                                              Tid_using,
                                              Tid_SwBracket_Right,
                                              Tid_DoubleColon,
                                              Tid_BuiltInType,
                                              Tid_TypeSpecializer };

    static F_Tok stateF_inProtection[] =    { &PE_Class::On_inProtection_Colon };
    static INT16 stateT_inProtection[] =    { Tid_Colon };

    static F_Tok stateF_afterDecl[] =       { &PE_Class::On_afterDecl_Semicolon };
    static INT16 stateT_afterDecl[] =       { Tid_Semicolon };

    SEMPARSE_CREATE_STATUS(PE_Class, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Class, expectName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Class, gotName, On_gotName_Return2Type);
    SEMPARSE_CREATE_STATUS(PE_Class, bodyStd, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Class, inProtection, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Class, afterDecl, On_afterDecl_Return2Type);

#if 0
    static F_Tok stateF_inFriend[] =        { On_inFriend_class,
                                              On_inFriend_struct,
                                              On_inFriend_union };
                                              // Default: On_inFriend_Function
    static INT16 stateT_inFriend[] =        { Tid_class,
                                              Tid_struct,
                                              Tid_union };
#endif // 0
}

void
PE_Class::InitData()
{
    pStati->SetCur(start);
    sLocalName.clear();
    eClassKey = ary::cpp::CK_class;
    pCurObject = 0;
    csv::erase_container(aBases);
    eResult_KindOf = is_declaration;
}

void
PE_Class::TransferData()
{
    pStati->SetCur(size_of_states);
}

void
PE_Class::Hdl_SyntaxError( const char * i_sText)
{
    if ( *i_sText == ';' )
    {
         Cerr() << Env().CurFileName() << ", line "
                  << Env().LineCount()
                  << ": Sourcecode warning: ';' as a toplevel declaration is deprecated."
                  << Endl();
        SetTokenResult(done,stay);
        return;
    }

    StdHandlingOfSyntaxError(i_sText);
}

void
PE_Class::Init_CurObject()
{
    // KORR_FUTURE
    //   This will have to be done before parsing base classes, because of
    //   possible inline documentation for base classes.
    pCurObject = & Env().AryGate().Store_Class( Env().Context(), sLocalName, eClassKey );

      for ( PE_Base::BaseList::const_iterator it = aBases.begin();
          it !=  aBases.end();
          ++it )
    {
        pCurObject->Add_BaseClass( *it );
    }  // end for

    Dyn< StringVector >
            pTplParams( Env().Get_CurTemplateParameters() );
    if ( pTplParams )
    {
          for ( StringVector::const_iterator it = pTplParams->begin();
              it !=  pTplParams->end();
              ++it )
        {
            pCurObject->Add_TemplateParameterType( *it, 0 );
        }  // end for
    }
}

void
PE_Class::SpReturn_Base()
{
    aBases = pSpuBase->Child().Result_BaseIds();
    pStati->SetCur(gotName);
}

void
PE_Class::On_start_class( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(expectName);
    eClassKey = ary::cpp::CK_class;
}

void
PE_Class::On_start_struct( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(expectName);
    eClassKey = ary::cpp::CK_struct;
}

void
PE_Class::On_start_union( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(expectName);
    eClassKey = ary::cpp::CK_union;
}

void
PE_Class::On_expectName_Identifier( const char * i_sText )
{
    SetTokenResult(done, stay);
    pStati->SetCur(gotName);
    sLocalName = i_sText;
}

void
PE_Class::On_expectName_SwBracket_Left( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(bodyStd);

    sLocalName = "";
    Init_CurObject();
    sLocalName = pCurObject->LocalName();

    Env().OpenClass(*pCurObject);
}

void
PE_Class::On_expectName_Colon( const char * )
{
    pStati->SetCur(gotName);
    sLocalName = "";

    pSpuBase->Push(done);
}

void
PE_Class::On_gotName_SwBracket_Left( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(bodyStd);

    Init_CurObject();
    if ( sLocalName.empty() )
        sLocalName = pCurObject->LocalName();

    Env().OpenClass(*pCurObject);
}

void
PE_Class::On_gotName_Semicolon( const char * )
{
    SetTokenResult(not_done, pop_success);

    eResult_KindOf = is_predeclaration;
}

void
PE_Class::On_gotName_Colon( const char * )
{
    pSpuBase->Push(done);
}

void
PE_Class::On_gotName_Return2Type( const char * )
{
    SetTokenResult(not_done, pop_success);

    eResult_KindOf = is_qualified_typename;
}

void
PE_Class::On_bodyStd_VarFunc( const char * )
{
    pSpuVarFunc->Push(not_done);
}

void
PE_Class::On_bodyStd_ClassKey( const char * )
{
    pSpuVarFunc->Push(not_done);        // This is correct,
                                        //   classes are parsed via PE_Type.
}

void
PE_Class::On_bodyStd_enum( const char * )
{
    pSpuVarFunc->Push(not_done);        // This is correct,
                                        //   enums are parsed via PE_Type.
}

void
PE_Class::On_bodyStd_typedef( const char * )
{
    pSpuTypedef->Push(not_done);
}

void
PE_Class::On_bodyStd_public( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(inProtection);

    Env().SetCurProtection(ary::cpp::PROTECT_public);
}

void
PE_Class::On_bodyStd_protected( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(inProtection);

    Env().SetCurProtection(ary::cpp::PROTECT_protected);
}

void
PE_Class::On_bodyStd_private( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(inProtection);

    Env().SetCurProtection(ary::cpp::PROTECT_private);
}

void
PE_Class::On_bodyStd_template( const char * )
{
    pSpuTemplate->Push(done);
}

void
PE_Class::On_bodyStd_friend( const char * )
{
    // KORR_FUTURE
    pSpuUsing->Push(done);
}

void
PE_Class::On_bodyStd_using( const char * )
{
    pSpuUsing->Push(done);
}

void
PE_Class::On_bodyStd_SwBracket_Right( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(afterDecl);

    Env().CloseClass();
}

void
PE_Class::On_inProtection_Colon( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(bodyStd);
}

void
PE_Class::On_afterDecl_Semicolon( const char * )
{
    SetTokenResult(not_done, pop_success);
    eResult_KindOf = is_declaration;
}

void
PE_Class::On_afterDecl_Return2Type( const char * )
{
    SetTokenResult(not_done, pop_success);
    eResult_KindOf = is_implicit_declaration;
}


}   // namespace cpp




