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



#ifndef ADC_CPP_PE_VARI_HXX
#define ADC_CPP_PE_VARI_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_vfflag.hxx>
    // PARAMETERS


namespace cpp {

class PE_Expression;


class PE_Variable : public Cpp_PE
{
  public:
    enum E_State
    {
        afterName,                  //
        afterSize,                  // after ]
        expectFinish,               // after InitExpression
        size_of_states
    };
                        PE_Variable(
                            Cpp_PE *            i_pParent );
                        ~PE_Variable();

    /** @return
        Bit 0x0001 != 0, if there is a size and
        bit 0x0002 != 0, if there is an initialisation.
    */
    UINT16              Result_Pattern() const;
    const String  &     Result_SizeExpression() const;
    const String  &     Result_InitExpression() const;

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    typedef SubPe< PE_Variable, PE_Expression >         SP_Expression;
    typedef SubPeUse< PE_Variable, PE_Expression>       SPU_Expression;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError(const char *);

    void                SpReturn_ArraySizeExpression();
    void                SpReturn_InitExpression();

    void                On_afterName_ArrayBracket_Left(const char * i_sText);
    void                On_afterName_Semicolon(const char * i_sText);
    void                On_afterName_Comma(const char * i_sText);
    void                On_afterName_Assign(const char * i_sText);

    void                On_afterSize_ArrayBracket_Right(const char * i_sText);

    void                On_expectFinish_Semicolon(const char * i_sText);
    void                On_expectFinish_Comma(const char * i_sText);
    void                On_expectFinish_Bracket_Right(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Variable> >
                        pStati;

    Dyn<SP_Expression>  pSpExpression;
    Dyn<SPU_Expression> pSpuArraySizeExpression;
    Dyn<SPU_Expression> pSpuInitExpression;

    String              sResultSizeExpression;
    String              sResultInitExpression;
};



// IMPLEMENTATION


inline UINT16
PE_Variable::Result_Pattern() const
    { return ( sResultSizeExpression.length() > 0 ? 1 : 0 )
             + ( sResultInitExpression.length() > 0 ? 2 : 0 ); }
inline const String  &
PE_Variable::Result_SizeExpression() const
    { return sResultSizeExpression; }
inline const String  &
PE_Variable::Result_InitExpression() const
    { return sResultInitExpression; }


}   // namespace cpp


#endif

