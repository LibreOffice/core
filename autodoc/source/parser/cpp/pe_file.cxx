/*************************************************************************
 *
 *  $RCSfile: pe_file.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-15 18:46:16 $
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
#include "pe_file.hxx"

// NOT FULLY DECLARED SERVICES
#include "pe_defs.hxx"
#include "pe_enum.hxx"
#include "pe_namsp.hxx"
#include "pe_tpltp.hxx"
#include "pe_tydef.hxx"
#include "pe_vafu.hxx"
#include "pe_ignor.hxx"


// NOT FULLY DECLARED SERVICES


namespace cpp
{

PE_File::PE_File( cpp::PeEnvironment & io_rEnv)
    :   Cpp_PE(io_rEnv),
        pEnv(&io_rEnv),
        pStati( new PeStatusArray<PE_File> ),
        // pSpNamespace,
        // pSpTypedef,
        // pSpVarFunc,
        // pSpIgnore,
        // pSpuNamespace,
        // pSpuClass,
        // pSpuTypedef,
        // pSpuVarFunc,
        // pSpuTemplate,
        // pSpuUsing,
        // pSpuIgnoreFailure,
        bWithinSingleExternC(false)
{
    Setup_StatusFunctions();

    pSpNamespace = new SP_Namespace(*this);
    pSpTypedef   = new SP_Typedef(*this);
    pSpVarFunc   = new SP_VarFunc(*this);
    pSpTemplate  = new SP_Template(*this);
    pSpDefs      = new SP_Defines(*this);
    pSpIgnore    = new SP_Ignore(*this);

    pSpuNamespace   = new SPU_Namespace(*pSpNamespace, 0, 0);
    pSpuTypedef     = new SPU_Typedef(*pSpTypedef, 0, 0);
    pSpuVarFunc     = new SPU_VarFunc(*pSpVarFunc, 0, &PE_File::SpReturn_VarFunc);
    pSpuTemplate    = new SPU_Template(*pSpTemplate, 0, &PE_File::SpReturn_Template);
    pSpuDefs        = new SPU_Defines(*pSpDefs, 0, 0);
    pSpuUsing       = new SPU_Ignore(*pSpIgnore, 0, 0);
    pSpuIgnoreFailure
                    = new SPU_Ignore(*pSpIgnore, 0, 0);
}

PE_File::~PE_File()
{
}

void
PE_File::Call_Handler( const cpp::Token &   i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

Cpp_PE *
PE_File::Handle_ChildFailure()
{
    SetCurSPU(pSpuIgnoreFailure.Ptr());
    return &pSpuIgnoreFailure->Child();
}

ary::cpp::RwGate &
PE_File::AryGate() const
{
     return const_cast< PE_File& >(*this).Env().AryGate();
}

void
PE_File::Setup_StatusFunctions()
{
    typedef CallFunction<PE_File>::F_Tok    F_Tok;
    static F_Tok stateF_std[] =             { &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_ClassKey,
                                              &PE_File::On_std_ClassKey,
                                              &PE_File::On_std_ClassKey,
                                              &PE_File::On_std_enum,
                                              &PE_File::On_std_typedef,
                                              &PE_File::On_std_template,
                                              &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_extern,
                                              &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_namespace,
                                              &PE_File::On_std_using,
                                              &PE_File::On_std_SwBracketRight,
                                              &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_DefineName,
                                              &PE_File::On_std_MacroName,
                                              &PE_File::On_std_VarFunc,
                                              &PE_File::On_std_VarFunc };
    static INT16 stateT_std[] =             { Tid_Identifier,
                                              Tid_class,
                                              Tid_struct,
                                              Tid_union,
                                              Tid_enum,
                                              Tid_typedef,
                                              Tid_template,
                                              Tid_const,
                                              Tid_volatile,
                                              Tid_extern,
                                              Tid_static,
                                              Tid_register,
                                              Tid_inline,
                                              Tid_namespace,
                                              Tid_using,
                                              Tid_SwBracket_Right,
                                              Tid_DoubleColon,
                                              Tid_DefineName,
                                              Tid_MacroName,
                                              Tid_BuiltInType,
                                              Tid_TypeSpecializer };

    static F_Tok stateF_in_extern[] =       { &PE_File::On_in_extern_Constant };
    static INT16 stateT_in_extern[] =       { Tid_Constant };

    static F_Tok stateF_in_externC[] =      { &PE_File::On_in_externC_SwBracket_Left };
    static INT16 stateT_in_externC[] =      { Tid_SwBracket_Left };


    SEMPARSE_CREATE_STATUS(PE_File, std, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_File, in_extern, On_in_extern_Ignore);
    SEMPARSE_CREATE_STATUS(PE_File, in_externC, On_in_externC_NoBlock);
}

void
PE_File::InitData()
{
    pStati->SetCur(std);
}

void
PE_File::TransferData()
{
    pStati->SetCur(size_of_states);
}

void
PE_File::Hdl_SyntaxError( const char * i_sText)
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
PE_File::SpReturn_VarFunc()
{
     if (bWithinSingleExternC)
    {
        Env().CloseBlock();
         bWithinSingleExternC = false;
    }
}

void
PE_File::SpReturn_Template()
{
    Env().OpenTemplate( pSpuTemplate->Child().Result_Parameters() );
}

void
PE_File::On_std_namespace(const char * i_sText)
{
    pSpuNamespace->Push(done);
}

void
PE_File::On_std_ClassKey(const char * i_sText)
{
    pSpuVarFunc->Push(not_done);        // This is correct,
                                        //   classes are parsed via PE_Type.
}

void
PE_File::On_std_typedef(const char * i_sText)
{
    pSpuTypedef->Push(not_done);
}

void
PE_File::On_std_enum(const char * i_sText)
{
    pSpuVarFunc->Push(not_done);        // This is correct,
                                        //   enums are parsed via PE_Type.
}

void
PE_File::On_std_VarFunc(const char * i_sText)
{
    pSpuVarFunc->Push(not_done);
}

void
PE_File::On_std_template(const char * i_sText)
{
    pSpuTemplate->Push(done);
}

void
PE_File::On_std_extern(const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(in_extern);
}

void
PE_File::On_std_using(const char * i_sText)
{
    pSpuUsing->Push(done);
}

void
PE_File::On_std_SwBracketRight(const char * i_sText)
{
    SetTokenResult(done,stay);
    Env().CloseBlock();
}

void
PE_File::On_std_DefineName(const char * i_sText)
{
    pSpuDefs->Push(not_done);
}

void
PE_File::On_std_MacroName(const char * i_sText)
{
    pSpuDefs->Push(not_done);
}

void
PE_File::On_in_extern_Constant(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(in_externC);

    Env().OpenExternC();
}

void
PE_File::On_in_extern_Ignore(const char * i_sText)
{
    SetTokenResult(not_done, stay);
    pStati->SetCur(std);
}

void
PE_File::On_in_externC_SwBracket_Left(const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(std);
}

void
PE_File::On_in_externC_NoBlock(const char * i_sText)
{
    SetTokenResult(not_done, stay);
    pStati->SetCur(std);

    bWithinSingleExternC = true;
}


}   // namespace cpp


