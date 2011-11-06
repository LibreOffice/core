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



#ifndef ADC_CPP_PE_ENVAL_HXX
#define ADC_CPP_PE_ENVAL_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS


namespace cpp {

class PE_Expression;


class PE_EnumValue : public Cpp_PE
{
  public:
    enum E_State
    {
        start,                      // before name
        afterName,
        expectFinish,               // after init-expression
        size_of_states
    };
                        PE_EnumValue(
                            Cpp_PE *            i_pParent );
                        ~PE_EnumValue();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    typedef SubPe< PE_EnumValue, PE_Expression >        SP_Expression;
    typedef SubPeUse< PE_EnumValue, PE_Expression>      SPU_Expression;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError(const char *);

    void                SpReturn_InitExpression();

    void                On_start_Identifier(const char * );

    void                On_afterName_SwBracket_Right(const char * );
    void                On_afterName_Comma(const char * );
    void                On_afterName_Assign(const char * );

    void                On_expectFinish_SwBracket_Right(const char * );
    void                On_expectFinish_Comma(const char * );

    // DATA
    Dyn< PeStatusArray<PE_EnumValue> >
                        pStati;
    Dyn<SP_Expression>  pSpExpression;
    Dyn<SPU_Expression> pSpuInitExpression;

    String              sName;
    String              sInitExpression;
};




}   // namespace cpp
#endif

