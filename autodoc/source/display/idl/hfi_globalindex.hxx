/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_globalindex.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:34:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

