/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
