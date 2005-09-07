/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_struct.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:47:38 $
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
