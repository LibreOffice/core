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




#ifndef ADC_CPP_PE_EXPR_HXX
#define ADC_CPP_PE_EXPR_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS


namespace cpp {


class PE_Expression : public Cpp_PE
{
  public:
    enum E_State
    {
        std,
        size_of_states
    };
                        PE_Expression(
                            Cpp_PE *            i_pParent );
                        ~PE_Expression();

    const char *        Result_Text() const;

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                On_std_Default( const char *);

    void                On_std_SwBracket_Left( const char *);
    void                On_std_SwBracket_Right( const char *);
    void                On_std_ArrayBracket_Left( const char *);
    void                On_std_ArrayBracket_Right( const char *);
    void                On_std_Bracket_Left( const char *);
    void                On_std_Bracket_Right( const char *);
    void                On_std_Semicolon( const char *);
    void                On_std_Comma( const char *);

    // DATA
    Dyn< PeStatusArray<PE_Expression> >
                        pStati;

    csv::StreamStr      aResult_Text;

    intt                nBracketCounter;
};



// IMPLEMENTATION

inline const char *
PE_Expression::Result_Text() const
{
    return aResult_Text.c_str();
}


}   // namespace cpp




#endif


