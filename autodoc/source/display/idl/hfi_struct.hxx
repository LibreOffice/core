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



#ifndef ADC_DISPLAY_HFI_STRUCT_HXX
#define ADC_DISPLAY_HFI_STRUCT_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS

/** Is used to display ->ary::idl::Exception s as well as ->ary::idl::Struct s.
*/
class HF_IdlStruct : public HtmlFactory_Idl
{
  public:

                        HF_IdlStruct(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut,
                            bool                i_bIsException );
    virtual             ~HF_IdlStruct();

    void                Produce_byData(
                            const client &      ce ) const;
  private:
    // Interface HtmlFactory_Idl:
    virtual type_id     inq_BaseOf(
                            const client &      i_ce ) const;
    // Locals
    HF_NaviSubRow &     make_Navibar(
                            const client &      ce ) const;
    virtual void        produce_MemberDetails(
                            HF_SubTitleTable &  o_table,
                            const client &      ce ) const;
    // DATA
    bool                bIsException;
};



// IMPLEMENTATION


extern const String
    C_sCePrefix_Struct;
extern const String
    C_sCePrefix_Exception;


#endif
