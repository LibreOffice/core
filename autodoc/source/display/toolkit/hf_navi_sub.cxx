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

#include <precomp.h>
#include <toolkit/hf_navi_sub.hxx>


// NOT FULLY DEFINED SERVICES


HF_NaviSubRow::HF_NaviSubRow( Xml::Element & o_rOut )
    :   HtmlMaker(o_rOut),
        aRow(),
        pMyRow(0)
{
    Setup_Row();
}

HF_NaviSubRow::~HF_NaviSubRow()
{
}

void
HF_NaviSubRow::AddItem( const String &      i_sText,
                        const String &      i_sLink,
                        bool                i_bSwitchOn )
{
    aRow.push_back( SubRow_Item( SubRow_Data(i_sText,i_sLink),
                                 i_bSwitchOn ));
}

void
HF_NaviSubRow::SwitchOn( int i_nIndex )
{
    if ( i_nIndex < int(aRow.size()) )
        aRow[i_nIndex].second = true;
}

void
HF_NaviSubRow::Setup_Row()
{
    Html::Table *
                pTable = new Html::Table;
    CurOut()
        >> *pTable
           << new Html::ClassAttr("navisub") 
           << new Xml::AnAttribute( "border", "0" )
           << new Xml::AnAttribute( "cellpadding", "0" );
    pMyRow = &pTable->AddRow();
}

void
HF_NaviSubRow::Produce_Row()
{
    for ( SubRow::const_iterator it = aRow.begin();
          it != aRow.end();
          ++it )
    {
        Xml::Element &
            rCell = *pMyRow
                     >> *new Html::TableCell
                         << new Html::ClassAttr("navisub");
        StreamLock sl(100);
        Xml::Element &
            rGoon = (*it).second
                            ?   ( rCell
                                  >> *new Html::Link( sl()
                                                      << "#"
                                                      << (*it).first.second
                                                      << c_str )
                                     << new Html::ClassAttr("navisub")
                                )
                            :   rCell;
        rGoon
            << (*it).first.first;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
