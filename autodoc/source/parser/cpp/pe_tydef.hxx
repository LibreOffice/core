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



#ifndef ADC_CPP_PE_TYDEF_HXX
#define ADC_CPP_PE_TYDEF_HXX

// BASE CLASSES
#include "cpp_pe.hxx"
// USED SERVICES
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>


namespace cpp
{
    class PE_Type;




class PE_Typedef : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        start,
        expectName,
        afterName,
        size_of_states
    };
                        PE_Typedef(
                            Cpp_PE *        i_pParent );
                        ~PE_Typedef();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );
  private:
    typedef SubPe< PE_Typedef, PE_Type >        SP_Type;
    typedef SubPeUse< PE_Typedef, PE_Type>      SPU_Type;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                SpReturn_Type();

    void                On_start_typedef( const char * );
    void                On_expectName_Identifier( const char * );
    void                On_afterName_Semicolon( const char * );

    // DATA
    Dyn< PeStatusArray<PE_Typedef> >
                        pStati;
    Dyn<SP_Type>        pSpType;
    Dyn<SPU_Type>       pSpuType;

    String              sName;
    ary::cpp::Type_id   nType;
};




}   // namespace cpp
#endif
