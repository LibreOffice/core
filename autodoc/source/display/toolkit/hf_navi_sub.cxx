/*************************************************************************
 *
 *  $RCSfile: hf_navi_sub.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:15:25 $
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


