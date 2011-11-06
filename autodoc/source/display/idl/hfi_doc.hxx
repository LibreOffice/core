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



#ifndef ADC_DISPLAY_HFI_DOC_HXX
#define ADC_DISPLAY_HFI_DOC_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS


class HF_DocEntryList;


class HF_IdlDocu : public HtmlFactory_Idl
{
  public:
                        HF_IdlDocu(
                            Environment &       io_rEnv,
                            HF_DocEntryList &   o_rOut );
    virtual             ~HF_IdlDocu();


    /** Produces documentation by the CodeInfo accompanying
        ->i_ce.
    */
    void                Produce_fromCodeEntity(
                            const client &      i_ce ) const;

    /** Produces documentation by the CodeInfo accompanying
        a link or reference to a CodeEntity.

        @param i_rScopeGivingCe
        Gives the scope from which links are to be calculated.
    */
    void                Produce_fromReference(
                            const ce_info &     i_rDocuForReference,
                            const client &      i_rScopeGivingCe ) const;

  private:
    // Locals
    /** Produces documentation.

        @param i_rScopeGivingCe
        Gives the scope from which links are to be calculated.
    */
    void                Produce_byDocuAndScope(
                            const ce_info &     i_rDocu,
                            const client *      i_pClient,      /// May be 0.
                            const client &      i_rScopeGivingCe ) const;

    // DATA
    HF_DocEntryList &   rOut;
};


#endif
