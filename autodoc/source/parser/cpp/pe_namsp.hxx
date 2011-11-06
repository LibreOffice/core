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



#ifndef ADC_CPP_PE_NAMSP_HXX
#define ADC_CPP_PE_NAMSP_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_pe.hxx>
    // PARAMETERS

namespace ary
{
namespace cpp
{
class Namespace;
}
}


namespace cpp
{


class PE_Namespace : public Cpp_PE
{
  public:
    enum E_State
    {
        start,
        gotName,
        expectSemicolon,        /// after namespace assignment
        size_of_states
    };
                        PE_Namespace(
                            Cpp_PE *            i_pParent );
                        ~PE_Namespace();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    ary::cpp::Namespace *
                        Result_OpenedNamespace() const;
  private:
    void                Setup_StatusFunctions();

    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                On_start_Identifier(const char * i_sText);
    void                On_start_SwBracket_Left(const char * i_sText);
    void                On_gotName_SwBracket_Left(const char * i_sText);
    void                On_gotName_Assign(const char * i_sText);
    void                On_expectSemicolon_Semicolon(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Namespace> >
                        pStati;

    String              sLocalName;
    bool                bPush;
};




}   // namespace cpp
#endif

