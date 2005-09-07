/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hf_navi_sub.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:01:22 $
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
        Xml::Element &
            rGoon = (*it).second
                            ?   ( rCell
                                  >> *new Html::Link( StreamLock(100)()
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


