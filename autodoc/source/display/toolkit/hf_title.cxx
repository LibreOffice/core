/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hf_title.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-03 16:58:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        String sColumns = StreamLock(20)() << i_nColumns << c_str;
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
