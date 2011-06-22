/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
