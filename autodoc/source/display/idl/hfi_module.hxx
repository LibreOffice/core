/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_module.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:43:54 $
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

#ifndef ADC_DISPLAY_HFI_MODULE_HXX
#define ADC_DISPLAY_HFI_MODULE_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS


class HF_NaviSubRow;

class HF_IdlModule : public HtmlFactory_Idl
{
  public:
                        HF_IdlModule(
                            Environment &       io_rEnv,    // The CurDirectory() is the one of the here displayed Module.
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlModule();

    void                Produce_byData(
                           const client &       i_ce ) const;
  private:
    typedef std::vector< const ary::idl::CodeEntity* >  ce_ptr_list;

    DYN HF_NaviSubRow & make_Navibar(
                            const client &      i_ce ) const;
    bool                produce_ChildList(
                            const String &      i_sName,
                            const String &      i_sLabel,
                            const ce_ptr_list & i_list ) const;
    void                produce_Link(
                            Xml::Element &      o_row,
                            const client *      i_ce ) const;
    void                produce_LinkDoc(
                            Xml::Element &      o_row,
                            const client *      i_ce ) const;
};



// IMPLEMENTATION


extern const String
    C_sCePrefix_Module;





#endif


