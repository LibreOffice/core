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

#ifndef ADC_DISPLAY_HF_NAVI_SUB_HXX
#define ADC_DISPLAY_HFI_NAVI_SUB_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "htmlfactory.hxx"
    // PARAMETERS


class HF_NaviSubRow : public HtmlMaker
{
  public:
                        HF_NaviSubRow(
                            Xml::Element &      o_rOut );
    virtual             ~HF_NaviSubRow();

    void                AddItem(
                            const String &      i_sText,
                            const String &      i_sLink,
                            bool                i_bSwitchOn );
    void                SwitchOn(
                            int                 i_nIndex );
    void                Produce_Row();

  private:
    typedef std::pair<String,String>    SubRow_Data;
    typedef std::pair<SubRow_Data,bool> SubRow_Item;
    typedef std::vector<SubRow_Item>    SubRow;

    /** Puts the row's table into the parent XML-element, but
        doesn't write the items, because the actvity-status of
        the subitems isn't known yet.
    */
    void                Setup_Row();

    // DATA
    SubRow              aRow;
    Xml::Element *      pMyRow;
};




// IMPLEMENTATION




#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
