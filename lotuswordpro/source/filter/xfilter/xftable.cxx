/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Table object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-24 create and implements.
 ************************************************************************/
#include	"xftable.hxx"
#include	"xfrow.hxx"
#include	"xfcolstyle.hxx"
#include	"xfstylemanager.hxx"
#include	<cassert>

XFTable::XFTable()
{
    m_strName = XFGlobal::GenTableName();
    m_bSubTable = sal_False;
    m_pOwnerCell = NULL;
}

XFTable::XFTable(const XFTable& other)
{
    m_strName = other.m_strName;
    m_bSubTable = other.m_bSubTable;
    m_pOwnerCell = NULL;
    for( int i=1; i<=other.m_aHeaderRows.GetCount(); i++ )
    {
        XFRow *pRow = (XFRow*)other.m_aHeaderRows.GetContent(i+1);
        if( pRow )
        {
            m_aHeaderRows.Add( new XFRow(*pRow) );
        }
    }

    m_aColumns = other.m_aColumns;
    m_strDefCellStyle = other.m_strDefCellStyle;
    m_strDefRowStyle = other.m_strDefRowStyle;
    m_strDefColStyle = other.m_strDefColStyle;
}

XFTable& XFTable::operator =(const XFTable& other)
{
    m_strName = other.m_strName;
    m_bSubTable = other.m_bSubTable;
    m_pOwnerCell = NULL;
    for( int i=1; i<=other.m_aHeaderRows.GetCount(); i++ )
    {
        XFRow *pRow = (XFRow*)other.m_aHeaderRows.GetContent(i);
        if( pRow )
        {
            m_aHeaderRows.Add( new XFRow(*pRow) );
        }
    }

    m_aColumns = other.m_aColumns;
    m_strDefCellStyle = other.m_strDefCellStyle;
    m_strDefRowStyle = other.m_strDefRowStyle;
    m_strDefColStyle = other.m_strDefColStyle;
    return *this;
}



XFTable::~XFTable()
{
    std::map<sal_Int32,XFRow*>::iterator it;
    for( it=m_aRows.begin(); it!=m_aRows.end(); it++ )
    {
        XFRow *pRow = (*it).second;
        if( pRow )
            delete pRow;
    }
    m_aRows.clear();
    m_aColumns.clear();
}

void	XFTable::SetColumnStyle(sal_Int32 col, rtl::OUString style)
{
    m_aColumns[col] = style;
}

void	XFTable::AddRow(XFRow *pRow)
{
    assert(pRow);
    if( !pRow )
        return;

    int	row = pRow->GetRow();

    if( row<1 )
        pRow->SetRow(m_aRows.size()+1);

    row = pRow->GetRow();
    if( m_aRows.find(row) != m_aRows.end() )
        delete m_aRows[row];

    pRow->SetOwnerTable(this);
    m_aRows[row] = pRow;
}

void	XFTable::AddHeaderRow(XFRow *pRow)
{
    if( !pRow )
        return;
    m_aHeaderRows.Add(pRow);
}

rtl::OUString XFTable::GetTableName()
{
    if( m_bSubTable )
    {
        return m_pOwnerCell->GetCellName();
    }
    else
        return m_strName;
}

sal_Int32	XFTable::GetRowCount()
{
    sal_Int32	rowMax = -1;
    std::map<sal_Int32,XFRow*>::iterator it;
    for( it=m_aRows.begin(); it!=m_aRows.end(); it++ )
    {
        if( it->first>rowMax )
            rowMax = it->first;
    }

    return rowMax;
/*
    if( m_aRows.size()<1 )
        return 0;
    std::map<sal_Int32,XFRow*>::iterator it = m_aRows.end();
    it--;
    return it->first;*/
}

XFRow*	XFTable::GetRow(sal_Int32 row)
{
    return m_aRows[row];
}

sal_Int32	XFTable::GetColumnCount()
{
    int		colMax = -1;
    std::map<sal_Int32,rtl::OUString>::iterator it;
    for( it=m_aColumns.begin(); it!=m_aColumns.end(); it++ )
    {
        if( it->first>colMax )
            colMax = it->first;
    }
    return colMax;
/*
    if( m_aColumns.size()<1 )
        return 0;
    std::map<sal_Int32,rtl::OUString>::iterator it = m_aColumns.end();
    it--;
    return it->first;*/
}

rtl::OUString XFTable::GetColumnStyle(sal_Int32 col)
{
    if( m_aColumns.find(col) != m_aColumns.end() )
        return m_aColumns[col];
    return A2OUSTR("");
}

enumXFContent XFTable::GetContentType()
{
    return enumXFContentTable;
}

void	XFTable::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    //sub table shouldn't use table name.
    if( !m_bSubTable )
        pAttrList->AddAttribute( A2OUSTR("table:name"), m_strName);

    if( GetStyleName().getLength() )
        pAttrList->AddAttribute( A2OUSTR("table:style-name"), GetStyleName() );

    if( m_bSubTable )
        pStrm->StartElement( A2OUSTR("table:sub-table") );
    else
        pStrm->StartElement( A2OUSTR("table:table") );

    //test code
//	sal_Int32 rowMax = GetRowCount();
//	sal_Int32 colMax = GetColumnCount();
    //output columns:
    {
        int	lastCol = 0;
        std::map<sal_Int32,rtl::OUString>::iterator it;
        for( it=m_aColumns.begin(); it!=m_aColumns.end(); it++ )
        {
            sal_Int32	col = (*it).first;
            rtl::OUString	style = m_aColumns[col];

            //default col repeated:
            if( col >lastCol+1 )
            {
                if( col > lastCol + 2 )
                {
                    if( m_strDefColStyle.getLength() > 0 )
                    {
                        pAttrList->AddAttribute( A2OUSTR("table:style-name"), m_strDefColStyle );
                    }
                    pAttrList->AddAttribute( A2OUSTR("table:number-columns-repeated"), Int32ToOUString(col-lastCol-1) );
                }
                pStrm->StartElement( A2OUSTR("table:table-column") );
                pStrm->EndElement( A2OUSTR("table:table-column") );
            }

            if( style.getLength() > 0 )
            {
                pAttrList->AddAttribute( A2OUSTR("table:style-name"), style );
            }
            pStrm->StartElement( A2OUSTR("table:table-column") );
            pStrm->EndElement( A2OUSTR("table:table-column") );
            lastCol = col;
        }
    }

    if( m_aHeaderRows.GetCount()>0 )
    {
        pStrm->StartElement( A2OUSTR("table:table-header-rows") );
        m_aHeaderRows.ToXml(pStrm);
        pStrm->EndElement( A2OUSTR("table:table-header-rows") );
    }
    //output rows:
    {
        int		lastRow = 0;
        pAttrList = pStrm->GetAttrList();

        std::map<sal_Int32,XFRow* >::iterator it = m_aRows.begin();
        for( ; it!=m_aRows.end(); it++ )
        {
            int	row = (*it).first;
            XFRow *pRow = (*it).second;

            //null row repeated:
            if( row>lastRow+1 )
            {
                XFRow *pNullRow = new XFRow();
                pNullRow->SetStyleName(m_strDefRowStyle);
                if( row>lastRow+2)
                    pNullRow->SetRepeated(row-lastRow-1);
                XFCell *pCell = new XFCell();
                pCell->SetStyleName(m_strDefCellStyle);
                pNullRow->AddCell(pCell);
                pNullRow->ToXml(pStrm);
            }
            pRow->ToXml(pStrm);
            lastRow = row;
        }
    }

    if( m_bSubTable )
        pStrm->EndElement( A2OUSTR("table:sub-table") );
    else
        pStrm->EndElement( A2OUSTR("table:table") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
