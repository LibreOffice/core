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
