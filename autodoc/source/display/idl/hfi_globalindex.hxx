/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

