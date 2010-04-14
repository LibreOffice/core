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

#include <precomp.h>
#include <toolkit/hf_title.hxx>


// NOT FULLY DEFINED SERVICES
#include <stdlib.h>


const String C_sTitleBorder("0");
const String C_sTitleWidth("100%");
const String C_sTitlePadding("5");
const String C_sTitleSpacing("3");

const String C_sSubTitleBorder("1");
const String C_sSubTitleWidth("100%");
const String C_sSubTitlePadding("5");
const String C_sSubTitleSpacing("0");
const String C_sColSpan("colspan");


HF_TitleTable::HF_TitleTable( Xml::Element & o_rOut )
    :   HtmlMaker(o_rOut >> *new Html::Table( C_sTitleBorder,
                                              C_sTitleWidth,
                                              C_sTitlePadding,
                                              C_sTitleSpacing )
                            << new Html::ClassAttr("title-table")
                            << new Html::StyleAttr("margin-bottom:6pt;") )
{
}

HF_TitleTable::~HF_TitleTable()
{
}

void
HF_TitleTable::Produce_Title( const char * i_title )
{
    Add_Row()
        << new Html::ClassAttr("title")
        << i_title;
}

void
HF_TitleTable::Produce_Title( const char *        i_annotations,
                              const char *        i_title )
{
    if (csv::no_str(i_annotations))
    {
        Produce_Title(i_title);
        return;
    }

    Xml::Element &
        rRow = Add_Row();
    rRow
        << new Html::ClassAttr("title");

    Xml::Element &
        rTable = rRow
                    >> *new Html::Table()
                        << new Html::ClassAttr("title-table")
                        << new Html::WidthAttr("99%");
    Xml::Element &
        rInnerRow = rTable
                        >> *new Html::TableRow;
    rInnerRow
        >> *new Html::TableCell
            << new Html::WidthAttr("25%")
            << new Html::ClassAttr("title2")
            << i_annotations;
    rInnerRow
        >> *new Html::TableCell
            << new Html::WidthAttr("50%")
            << new Html::ClassAttr("title")
            << i_title;
    rInnerRow
        >> *new Html::TableCell
            << new Html::WidthAttr("*");
}

Xml::Element &
HF_TitleTable::Add_Row()
{
    return CurOut()
            >> *new Html::TableRow
                >> *new Html::TableCell;
}


inline const char *
get_SubTitleCssClass(HF_SubTitleTable::E_SubLevel i_eSubTitleLevel)
{
    return i_eSubTitleLevel == HF_SubTitleTable::sublevel_1
            ?   "subtitle"
            :   "crosstitle";
}


HF_SubTitleTable::HF_SubTitleTable( Xml::Element &      o_rOut,
                                    const String &      i_label,
                                    const String &      i_title,
                                    int                 i_nColumns,
                                    E_SubLevel          i_eSubTitleLevel )
    :   HtmlMaker( o_rOut
                    << new Html::Label(i_label)
                    >> *new Html::Table( C_sSubTitleBorder,
                                         C_sSubTitleWidth,
                                         C_sSubTitlePadding,
                                         C_sSubTitleSpacing )
                        << new Html::ClassAttr(get_SubTitleCssClass(i_eSubTitleLevel)) )
{
    csv_assert(i_nColumns > 0);

    if (i_eSubTitleLevel == sublevel_3)
        return;

    Xml::Element &
        rCell = CurOut()
                    >> *new Html::TableRow
                        >> *new Html::TableCell
                        << new Html::ClassAttr(get_SubTitleCssClass(i_eSubTitleLevel)) ;

    if (i_nColumns > 1)
    {
        StreamLock sl(20);
        String sColumns = sl() << i_nColumns << c_str;
        rCell
            << new Xml::AnAttribute(C_sColSpan, sColumns);
    }
    rCell
        << i_title;
}

HF_SubTitleTable::~HF_SubTitleTable()
{
}

Xml::Element &
HF_SubTitleTable::Add_Row()
{
    return CurOut() >> *new Html::TableRow;
}
