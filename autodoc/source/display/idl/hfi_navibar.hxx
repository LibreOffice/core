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



#ifndef ADC_DISPLAY_HFI_NAVIBAR_HXX
#define ADC_DISPLAY_HFI_NAVIBAR_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "hi_factory.hxx"
    // PARAMETERS

namespace ary
{
namespace idl
{
class CodeEntity;
}
}


class HF_NaviSubRow;

/** @resp
    Creates a navigation bar for an IDL HTML documentation page.
*/
class HF_IdlNavigationBar : public HtmlFactory_Idl
{
  public:
                        HF_IdlNavigationBar(
                            HtmlEnvironment_Idl &
                                                io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlNavigationBar();

    void                Produce_CeMainRow(
                            const client &      i_ce,
                            bool                i_bNoUsePage = false );
    void                Produce_CeXrefsMainRow(
                            const client &      i_ce );
    void                Produce_ModuleMainRow(
                            const client &      i_ce );
    void                Produce_IndexMainRow();

    /** Adds the subrow to the o_rOut argument of the constructor.
    */
    DYN HF_NaviSubRow & Add_SubRow();

  private:
    const ary::idl::CodeEntity *
                        pCe;
};

extern const String
    C_sLocalManualLinks;

#endif
