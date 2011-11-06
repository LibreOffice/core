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



#ifndef ADC_DISPLAY_HFI_GLOBALINDEX_HXX
#define ADC_DISPLAY_HFI_GLOBALINDEX_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_gate.hxx>


class HF_IdlTypeText;

class HF_IdlGlobalIndex : public HtmlFactory_Idl
{
  public:
    typedef std::vector<ary::idl::Ce_id> PageData;

                        HF_IdlGlobalIndex(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlGlobalIndex();

    void                Produce_Page(
                            ary::idl::alphabetical_index::E_Letter
                                                i_letter ) const;
  private:
    void                make_Navibar() const;   /// Called by @->Produce_Page()
    void                produce_Line(           /// Called by @->Produce_Page()
                            PageData::const_iterator
                                                i_entry,
                            const HF_IdlTypeText &
                                                i_typeLinkWriter ) const;

    void                write_EntryItself(      /// Called by @->produceLine()
                            Xml::Element &      o_destination,
                            const ary::idl::CodeEntity &
                                                i_entry,
                            const HF_IdlTypeText &
                                                i_typeLinkWriter ) const;

    void                write_OwnerOfEntry(     /// Called by @->produceLine()
                            Xml::Element &      o_destination,
                            const ary::idl::CodeEntity &
                                                i_entry,
                            const HF_IdlTypeText &
                                                i_typeLinkWriter ) const;

    void                write_EntrySecondTime(  /// Called by @->produceLine()
                            Xml::Element &      o_destination,
                            const ary::idl::CodeEntity &
                                                i_entry,
                            const HF_IdlTypeText &
                                                i_typeLinkWriter ) const;
};



#endif

