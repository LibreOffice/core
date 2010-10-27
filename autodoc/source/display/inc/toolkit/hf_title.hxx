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

#ifndef ADC_DISPLAY_HF_TITLE_HXX
#define ADC_DISPLAY_HF_TITLE_HXX



// USED SERVICES
    // BASE CLASSES
#include <toolkit/htmlfactory.hxx>
    // COMPONENTS
    // PARAMETERS


class HF_TitleTable : public HtmlMaker
{
  public:
                        HF_TitleTable(
                            Xml::Element &      o_rOut );
    virtual             ~HF_TitleTable();

    void                Produce_Title(
                            const char *        i_title );
    void                Produce_Title(
                            const char *        i_annotations,
//                            const char *        i_label,
                            const char *        i_title );

    /// @return a Html::TableCell reference.
    Xml::Element &      Add_Row();
};


class HF_SubTitleTable : public HtmlMaker
{
  public:
    enum E_SubLevel
    {
        sublevel_1,     /// Big title.
        sublevel_2,     /// Small title.
        sublevel_3      /// No title.
    };

    /** @param i_nColumns [1 .. n]
        @param i_nSubTitleLevel [1 .. 2]
        1 is a bit bigger than 2.
    */

                        HF_SubTitleTable(
                            Xml::Element &      o_rOut,
                            const String &      i_label,
                            const String &      i_title,
                            int                 i_nColumns,
                            E_SubLevel          i_eSubTitleLevel = sublevel_1 );
    virtual             ~HF_SubTitleTable();

    /// @return an Html::TableRow reference.
    Xml::Element &      Add_Row();
};


// IMPLEMENTATION



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
