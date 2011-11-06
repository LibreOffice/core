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




#ifndef ADC_CPP_PE_PARAM_HXX
#define ADC_CPP_PE_PARAM_HXX

// BASE CLASSES
#include "cpp_pe.hxx"
// USED SERVICES
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_vfflag.hxx>




namespace cpp
{
    class PE_Type;
    class PE_Variable;




class PE_Parameter : public Cpp_PE
{
  public:
    enum E_State
    {
        start,
        expectName,
        afterName,
        finished,
        size_of_states
    };
    typedef ary::cpp::S_Parameter       S_ParamInfo;

    explicit            PE_Parameter(
                            Cpp_PE *            i_pParent );
                        ~PE_Parameter();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    ary::cpp::Type_id   Result_FrontType() const;
    const S_ParamInfo & Result_ParamInfo() const;

  private:
    typedef SubPe< PE_Parameter, PE_Type >          SP_Type;
    typedef SubPeUse< PE_Parameter, PE_Type >       SPU_Type;
    typedef SubPe< PE_Parameter, PE_Variable >      SP_Variable;
    typedef SubPeUse< PE_Parameter, PE_Variable >   SPU_Variable;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                SpInit_Type();       // Type and Ignore.
    void                SpInit_Variable();
    void                SpReturn_Type();
    void                SpReturn_Variable();

    void                On_start_Type(const char * i_sText);
    void                On_start_Bracket_Right(const char * i_sText);
    void                On_start_Ellipse(const char * i_sText);

    void                On_expectName_Identifier(const char * i_sText);
    void                On_expectName_ArrayBracket_Left(const char * i_sText);
    void                On_expectName_Bracket_Right(const char * i_sText);
    void                On_expectName_Comma(const char * i_sText);

    void                On_afterName_ArrayBracket_Left(const char * i_sText);
    void                On_afterName_Bracket_Right(const char * i_sText);
    void                On_afterName_Comma(const char * i_sText);
    void                On_afterName_Assign(const char * i_sText);

    void                On_finished_Bracket_Right(const char * i_sText);
    void                On_finished_Comma(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Parameter> >
                        pStati;

    Dyn<SP_Type>        pSpType;
    Dyn<SPU_Type>       pSpuType;
    Dyn<SP_Variable>    pSpVariable;
    Dyn<SPU_Variable>   pSpuVariable;

    S_ParamInfo         aResultParamInfo;
};




// IMPLEMENTATION
inline ary::cpp::Type_id
PE_Parameter::Result_FrontType() const
{
    return aResultParamInfo.nType;
}

inline const PE_Parameter::S_ParamInfo &
PE_Parameter::Result_ParamInfo() const
{
    return aResultParamInfo;
}




}   // namespace cpp
#endif
