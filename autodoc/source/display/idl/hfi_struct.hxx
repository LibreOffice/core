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
