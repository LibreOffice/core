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

#ifndef ADC_DISPLAY_HF_NAVI_MAIN_HXX
#define ADC_DISPLAY_HF_NAVI_MAIN_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "htmlfactory.hxx"
    // PARAMETERS


class HF_MainItem;


/** @task
    Create a HTML navigation bar with lightly coloured background. 

    @descr
    There are three kinds of items:
    Item with link:             Add_StdItem(),   
    Item without link:          Add_NoneItem(),
    Item that is current page:  Add_SelfItem().
*/
class HF_NaviMainRow : public HtmlMaker
{
  public:
    enum E_Style
    {
        eStd,
        eSelf,
        eNo
    };
                        HF_NaviMainRow(
                            Xml::Element &      o_out );
                        ~HF_NaviMainRow();

    void                Add_StdItem(
                            const char *        i_sText,
                            const char *        i_sLink );
    void                Add_SelfItem(
                            const char *        i_sText );
    void                Add_NoneItem(
                            const char *        i_sText );

    void                Produce_Row();

  private:
    // DATA
    typedef std::vector< DYN HF_MainItem* > ItemList;

    ItemList            aItems;
    Xml::Element *      pRow;
};



// IMPLEMENTATION




#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
