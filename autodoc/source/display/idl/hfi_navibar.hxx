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

#ifndef ADC_DISPLAY_HFI_NAVIBAR_HXX
#define ADC_DISPLAY_HFI_NAVIBAR_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "hi_factory.hxx"
    // PARAMETERS

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
};

extern const String
    C_sLocalManualLinks;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
