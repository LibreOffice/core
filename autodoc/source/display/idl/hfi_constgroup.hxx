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

#ifndef ADC_DISPLAY_HFI_CONSTGROUP_HXX
#define ADC_DISPLAY_HFI_CONSTGROUP_HXX


// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS

class HF_NaviSubRow;

class HF_IdlConstGroup : public HtmlFactory_Idl
{
  public:
                        HF_IdlConstGroup(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlConstGroup();

    void                Produce_byData(
                            const client &      ce ) const;
  private:
    HF_NaviSubRow &     make_Navibar(
                            const client &      ce ) const;
    virtual void        produce_MemberDetails(
                            HF_SubTitleTable &  o_table,
                            const client &      ce ) const;
};



// IMPLEMENTATION


extern const String
    C_sCePrefix_Constants;

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
