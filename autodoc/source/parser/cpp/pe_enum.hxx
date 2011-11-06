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



#ifndef ADC_CPP_PE_ENUM_HXX
#define ADC_CPP_PE_ENUM_HXX




// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS
// #include "all_toks.hxx"


namespace cpp {


class PE_EnumValue;

class PE_Enum : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        expectName,         /// after "enum"
        gotName,            /// after name, before : or {
        bodyStd,            /// after {
        afterBlock,         /// after ending }
        size_of_states
    };

    enum E_KindOfResult
    {
        is_declaration,             // normal
        is_implicit_declaration,    // like in: enum Abc { rot, gelb, blau } aAbc;
        is_qualified_typename       // like in: enum Abc * fx();

    };
                        PE_Enum(
                            Cpp_PE *            i_pParent );
                        ~PE_Enum();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    E_KindOfResult      Result_KindOf() const;
    const String  &     Result_LocalName() const;
    const String  &     Result_FirstNameSegment() const;

  private:
    typedef SubPe< PE_Enum, PE_EnumValue >      SP_EnumValue;
    typedef SubPeUse< PE_Enum, PE_EnumValue>    SPU_EnumValue;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                On_expectName_Identifier( const char * );
    void                On_expectName_SwBracket_Left( const char * );

    void                On_gotName_SwBracket_Left( const char * );
    void                On_gotName_Return2Type( const char * );

    void                On_bodyStd_Identifier( const char * );
    void                On_bodyStd_SwBracket_Right( const char * );

    void                On_afterBlock_Semicolon( const char * );
    void                On_afterBlock_Return2Type( const char * );

    // DATA
    Dyn< PeStatusArray<PE_Enum> >
                        pStati;
    Dyn<SP_EnumValue>   pSpValue;
    Dyn<SPU_EnumValue>  pSpuValue;

    String              sLocalName;
    ary::cpp::Enum *    pCurObject;

    E_KindOfResult      eResult_KindOf;
};



// IMPLEMENTATION
inline PE_Enum::E_KindOfResult
PE_Enum::Result_KindOf() const
{
    return eResult_KindOf;
}

inline const String  &
PE_Enum::Result_LocalName() const
{
    return sLocalName;
}

inline const String  &
PE_Enum::Result_FirstNameSegment() const
{
    return sLocalName;
}


}   // namespace cpp


#endif

