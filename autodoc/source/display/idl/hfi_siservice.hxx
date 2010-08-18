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

#ifndef ADC_DISPLAY_HFI_SISERVICE_HXX
#define ADC_DISPLAY_HFI_SISERVICE_HXX


// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_comrela.hxx>

class HF_NaviSubRow;
class HF_SubTitleTable;

class HF_IdlSglIfcService : public HtmlFactory_Idl
{
  public:
                        HF_IdlSglIfcService(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlSglIfcService();

    void                Produce_byData(
                           const client &       i_ce ) const;
   private:
    DYN HF_NaviSubRow & make_Navibar(
                            const client &      i_ce ) const;

    void                produce_MemberDetails(
                            HF_SubTitleTable &  o_table,
                            const client &      i_ce ) const;
};



// IMPLEMENTATION

extern const String
    C_sCePrefix_Service;



#endif


