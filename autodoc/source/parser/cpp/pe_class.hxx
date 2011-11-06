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



#ifndef ADC_CPP_PE_CLASS_HXX
#define ADC_CPP_PE_CLASS_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // OTHER
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>
#include "all_toks.hxx"

namespace ary
{
namespace cpp
{
    class Class;
}
}


namespace cpp
{


using ary::cpp::E_Protection;
using ary::cpp::E_Virtuality;


class PE_Base;
class PE_Enum;
class PE_Typedef;
class PE_VarFunc;
class PE_Ignore;
class PE_Defines;


class PE_Class : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        start,              /// before class, struct or union
        expectName,         /// after class, struct or union
        gotName,            /// after name, before : or {
        bodyStd,            /// after {
        inProtection,       /// after public, protected or private and before ":"
        afterDecl,          /// after ending }
        size_of_states
    };

    enum E_KindOfResult
    {
        is_declaration,             // normal
        is_implicit_declaration,    // like in: class Abc { public int n; } aAbc;
        is_predeclaration,          // like:    class Abc;
        is_qualified_typename       // like in: class Abc * fx();

    };

                        PE_Class(
                            Cpp_PE *            i_pParent );
                        ~PE_Class();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );
    virtual Cpp_PE *    Handle_ChildFailure();

    E_KindOfResult      Result_KindOf() const;
    const String  &     Result_LocalName() const;
    const String  &     Result_FirstNameSegment() const;

  private:
    typedef SubPe< PE_Class, PE_Base >          SP_Base;
//  typedef SubPe< PE_Class, PE_Enum>           SP_Enum;
    typedef SubPe< PE_Class, PE_Typedef>        SP_Typedef;
    typedef SubPe< PE_Class, PE_VarFunc>        SP_VarFunc;
    typedef SubPe< PE_Class, PE_Ignore >        SP_Ignore;
    typedef SubPe< PE_Class, PE_Defines>        SP_Defines;

    typedef SubPeUse< PE_Class, PE_Base>        SPU_Base;
//  typedef SubPeUse< PE_Class, PE_Enum>            SPU_Enum;
    typedef SubPeUse< PE_Class, PE_Typedef>     SPU_Typedef;
    typedef SubPeUse< PE_Class, PE_VarFunc>     SPU_VarFunc;
    typedef SubPeUse< PE_Class, PE_Ignore>      SPU_Ignore;
    typedef SubPeUse< PE_Class, PE_Defines>     SPU_Defines;

    typedef ary::cpp::List_Bases                BaseList;
    typedef ary::cpp::S_Classes_Base            Base;
    typedef ary::cpp::E_Protection              E_Protection;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);
    void                Init_CurObject();

    void                SpReturn_Base();

    void                On_start_class( const char * );
    void                On_start_struct( const char * );
    void                On_start_union( const char * );

    void                On_expectName_Identifier( const char * );
    void                On_expectName_SwBracket_Left( const char * );
    void                On_expectName_Colon( const char * );

    void                On_gotName_SwBracket_Left( const char * );
    void                On_gotName_Semicolon( const char * );
    void                On_gotName_Colon( const char * );
    void                On_gotName_Return2Type( const char * );

    void                On_bodyStd_VarFunc( const char * );
    void                On_bodyStd_ClassKey( const char * );
    void                On_bodyStd_enum( const char * );
    void                On_bodyStd_typedef( const char * );
    void                On_bodyStd_public( const char * );
    void                On_bodyStd_protected( const char * );
    void                On_bodyStd_private( const char * );
    void                On_bodyStd_template( const char * );
    void                On_bodyStd_friend( const char * );
    void                On_bodyStd_using( const char * );
    void                On_bodyStd_SwBracket_Right( const char * );
    void                On_bodyStd_DefineName(const char * );
    void                On_bodyStd_MacroName(const char * );

    void                On_inProtection_Colon( const char * );

    void                On_afterDecl_Semicolon( const char * );
    void                On_afterDecl_Return2Type( const char * );

    // DATA
    Dyn< PeStatusArray<PE_Class> >
                        pStati;

    Dyn<SP_Base>        pSpBase;
//  Dyn<SP_Enum>        pSpEnum;
    Dyn<SP_Typedef>     pSpTypedef;
    Dyn<SP_VarFunc>     pSpVarFunc;
    Dyn<SP_Ignore>      pSpIgnore;
    Dyn<SP_Defines>     pSpDefs;

    Dyn<SPU_Base>       pSpuBase;
//  Dyn<SPU_Enum>       pSpuEnum;
    Dyn<SPU_Typedef>    pSpuTypedef;
    Dyn<SPU_VarFunc>    pSpuVarFunc;

    Dyn<SPU_Ignore>     pSpuTemplate;
    Dyn<SPU_Ignore>     pSpuUsing;
    Dyn<SPU_Ignore>     pSpuIgnoreFailure;
    Dyn<SPU_Defines>    pSpuDefs;



    String              sLocalName;
    ary::cpp::E_ClassKey
                        eClassKey;
    ary::cpp::Class *   pCurObject;
    BaseList            aBases;

    E_KindOfResult      eResult_KindOf;
};



// IMPLEMENTATION

inline PE_Class::E_KindOfResult
PE_Class::Result_KindOf() const
{
    return eResult_KindOf;
}

inline const String  &
PE_Class::Result_LocalName() const
{
    return sLocalName;
}

inline const String  &
PE_Class::Result_FirstNameSegment() const
{
    return sLocalName;
}




}   // namespace cpp


#if 0   // Branches

class struct union
    -> Class
    -> Predeclaration

typedef
    -> Typedef

enum
    -> Enum

TypeDeclaration
    -> Function In Class
    -> Variable

public, protected, private
    -> Protection declaration

friend
    ->  Friend Class
    ->  Friend Function

virtual
    -> Function In Class

using
    -> Using Declaration


#endif // 0


#endif

