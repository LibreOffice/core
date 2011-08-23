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
 * Text table object. It's the table used in writer.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-03-31 create and implements.
 ************************************************************************/
#include	"xftexttable.hxx"

XFTextTable::XFTextTable()
{

}

void	XFTextTable::SetColCount(sal_Int32 col)
{
    m_nColCount = col;
}

void	XFTextTable::SetRowCount(sal_Int32 row)
{
    m_nRowCount = row;
}

void	XFTextTable::SetCell(sal_Int32 row, sal_Int32 col, XFCell *pCell)
{
    if( row<0 || col<0 || !pCell )
        return;

    XFRow *pRow = GetRow(row);
    if( !pRow )
    {
        pRow = new XFRow();
        pRow->SetRow(row);
        AddRow(pRow);

        pRow = GetRow(row);
    }
    if( !pRow )
        return;
    pCell->SetCol(col);

    pRow->AddCell(pCell);
}

virtual void	XFTextTable::ToXml(IXFStream *pStrm)
{
    for( int i=0; i<m_nColCount; i++ )
    {
        rtl::OUString strColStyle = GetColumnStyle(i);
        if( strColStyle.getLength()>0 )
            SetColumnStyle(col, m_strDefColStyle);
    }

    for( int i=0; i<m_nRowCount; i++ )
    {
        XFRow *pRow = GetRow(i);
        if( !pRow )
        {
            pRow = new XFRow();
            pRow->SetRow(i);
            pRow->SetStyleName(m_strDefRowStyle);

            XFCell *pCell = new XFCell();
            pCell->SetColumnSpaned(m_nColCount);
            pRow->AddCell(pCell);

            SetRow(i, pRow);
        }
        else
        {
            for( int j=0; j<m_nColCount; j++ )
            {
                XFCell *pCell = pRow->GetCell(j);
                if( !pCell )
                {
                    pCell = new XFCell();
                    pCell->SetCol(j);
                    pCell->SetStyleName(m_strDefCellStyle);
                    pRow->AddCell(pCell);
                }
            }
        }
    }

    XFTable::ToXml(pStrm);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
