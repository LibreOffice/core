/*************************************************************************
 *
 *  $RCSfile: hf_title.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:33:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
