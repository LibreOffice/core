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



#ifndef ADC_CPP_PE_TPLTP_HXX
#define ADC_CPP_PE_TPLTP_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS


namespace cpp {



class PE_TemplateTop : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        start,
        expect_qualifier,
        expect_name,
        expect_separator,
        size_of_states
    };
                        PE_TemplateTop(
                            Cpp_PE *            i_pParent );
                        ~PE_TemplateTop();

    const StringVector &
                        Result_Parameters() const;

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );
  private:
    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError(const char *);


    void                On_start_Less(const char *);

    void                On_expect_qualifier_ClassOrTypename(const char *);
    void                On_expect_qualifier_Greater(const char *);
    void                On_expect_qualifier_Other(const char *);

    void                On_expect_name_Identifier(const char *);

    void                On_expect_separator_Comma(const char *);
    void                On_expect_separator_Greater(const char *);

    // DATA
    Dyn< PeStatusArray<PE_TemplateTop> >
                        pStati;

    StringVector
                        aResult_Parameters;
    bool                bCurIsConstant;
};



// IMPLEMENTATION

inline const StringVector &
PE_TemplateTop::Result_Parameters() const
    { return aResult_Parameters; }


}   // namespace cpp


#endif

