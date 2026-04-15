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
#include <xfilter/xftable.hxx>
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfrow.hxx>
#include <xfilter/xfglobal.hxx>
#include <cassert>

XFTable::XFTable()
{
    m_strName = XFGlobal::GenTableName();
    m_bSubTable = false;
    m_pOwnerCell = nullptr;
}

XFTable::~XFTable()
{
    m_aRows.clear();
    m_aColumns.clear();
}

void    XFTable::SetColumnStyle(sal_Int32 col, const OUString& style)
{
    m_aColumns[col] = style;
}

bool XFTable::ContainsTable(const XFTable* pTable) const
{
    for (auto const& elem : m_aRows)
    {
        const XFRow *pRow = elem.second.get();

        for (sal_Int32 i = 0; i < pRow->GetCellCount(); ++i)
        {
            const XFCell* pCell = pRow->GetCell(i + 1); //starts at 1, not 0
            if (const XFTable* pSubTable = pCell->GetSubTable())
            {
                if (pSubTable == pTable)
                    return true;
                if (pSubTable->ContainsTable(pTable))
                    return true;
            }
            if (pCell->HierarchyContains(pTable))
                return true;
        }
    }

    return false;
}

void XFTable::AddRow(rtl::Reference<XFRow> const & rRow)
{
    assert(rRow);

    for (sal_Int32 i = 0; i < rRow->GetCellCount(); ++i)
    {
        XFCell* pFirstCell = rRow->GetCell(i + 1); //starts at 1, not 0
        if (const XFTable* pSubTable = pFirstCell->GetSubTable())
        {
            if (pSubTable == this || pSubTable->ContainsTable(this))
                throw std::runtime_error("table is a subtable of itself");
        }
        if (pFirstCell->HierarchyContains(this))
            throw std::runtime_error("table is a subtable of itself");

    }

    int row = rRow->GetRow();

    if( row<1 )
        rRow->SetRow(m_aRows.size()+1);

    row = rRow->GetRow();

    rRow->SetOwnerTable(this);
    m_aRows[row] = rRow;
}

void XFTable::AddHeaderRow(XFRow *pRow)
{
    if( !pRow)
        return;
    if (!m_aHeaderRows.is())
        return;
    m_aHeaderRows->Add(pRow);
}

OUString XFTable::GetTableName()
{
    if( m_bSubTable )
    {
        return m_pOwnerCell->GetCellName();
    }
    else
        return m_strName;
}

sal_uInt16 XFTable::GetRowCount()
{
    sal_uInt16 rowMax = 0;
    for (auto const& row : m_aRows)
    {
        if (row.first > rowMax)
            rowMax = row.first;
    }

    return rowMax;
}

XFRow*  XFTable::GetRow(sal_Int32 row)
{
    return m_aRows[row].get();
}

sal_Int32   XFTable::GetColumnCount()
{
    int     colMax = -1;
    for (auto const& column : m_aColumns)
    {
        if( column.first>colMax )
            colMax = column.first;
    }
    return colMax;
}

enumXFContent XFTable::GetContentType()
{
    return enumXFContentTable;
}

void    XFTable::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    //sub table shouldn't use table name.
    if( !m_bSubTable )
        pAttrList->AddAttribute( u"table:name"_ustr, m_strName);

    if( !GetStyleName().isEmpty() )
        pAttrList->AddAttribute( u"table:style-name"_ustr, GetStyleName() );

    if( m_bSubTable )
        pStrm->StartElement( u"table:sub-table"_ustr );
    else
        pStrm->StartElement( u"table:table"_ustr );

    //output columns:
    {
        int lastCol = 0;
        for (auto const& column : m_aColumns)
        {
            sal_Int32   col = column.first;
            OUString   style = m_aColumns[col];

            //default col repeated:
            if( col >lastCol+1 )
            {
                if( col > lastCol + 2 )
                {
                    if( !m_strDefColStyle.isEmpty() )
                    {
                        pAttrList->AddAttribute( u"table:style-name"_ustr, m_strDefColStyle );
                    }
                    pAttrList->AddAttribute( u"table:number-columns-repeated"_ustr, OUString::number(col-lastCol-1) );
                }
                pStrm->StartElement( u"table:table-column"_ustr );
                pStrm->EndElement( u"table:table-column"_ustr );
            }

            if( !style.isEmpty() )
            {
                pAttrList->AddAttribute( u"table:style-name"_ustr, style );
            }
            pStrm->StartElement( u"table:table-column"_ustr );
            pStrm->EndElement( u"table:table-column"_ustr );
            lastCol = col;
        }
    }

    if (m_aHeaderRows.is() && m_aHeaderRows->GetCount()>0)
    {
        pStrm->StartElement( u"table:table-header-rows"_ustr );
        m_aHeaderRows->ToXml(pStrm);
        pStrm->EndElement( u"table:table-header-rows"_ustr );
    }
    //output rows:
    {
        int     lastRow = 0;

        for (auto const& elem : m_aRows)
        {
            int row = elem.first;
            XFRow *pRow = elem.second.get();

            //null row repeated:
            if( row>lastRow+1 )
            {
                XFRow *pNullRow = new XFRow();
                pNullRow->SetStyleName(m_strDefRowStyle);
                if( row>lastRow+2)
                    pNullRow->SetRepeated(row-lastRow-1);
                rtl::Reference<XFCell> xCell(new XFCell);
                xCell->SetStyleName(m_strDefCellStyle);
                pNullRow->AddCell(xCell);
                pNullRow->ToXml(pStrm);
            }
            pRow->ToXml(pStrm);
            lastRow = row;
        }
    }

    if( m_bSubTable )
        pStrm->EndElement( u"table:sub-table"_ustr );
    else
        pStrm->EndElement( u"table:table"_ustr );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
