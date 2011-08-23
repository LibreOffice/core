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
#include	"xfrow.hxx"
#include	"xfcell.hxx"
#include	"xftable.hxx"

XFRow::XFRow()
{
    m_nRepeat = 0;
    m_nRow = 0;
}

XFRow::XFRow(const XFRow& other)
{
    m_pOwnerTable = NULL;
    m_nRepeat = other.m_nRepeat;
    m_nRow = other.m_nRow;

    for( int i=1; i<=other.GetCellCount(); i++ )
    {
        XFCell *pCell = new XFCell;
        *pCell = *other.GetCell(i);
        AddCell(pCell);
    }
}

XFRow& XFRow::operator=(const XFRow& other)
{
    m_pOwnerTable = NULL;
    m_nRepeat = other.m_nRepeat;
    m_nRow = other.m_nRow;

    for( int i=1; i<=other.GetCellCount(); i++ )
    {
        XFCell *pCell = new XFCell;
        *pCell = *other.GetCell(i);
        AddCell(pCell);
    }
    return *this;
}


XFRow::~XFRow()
{
    std::map<sal_Int32,XFCell*>::iterator it;
    for( it=m_aCells.begin(); it!=m_aCells.end(); it++ )
    {
        XFCell *pCell = (*it).second;
        if( pCell )
            delete pCell;
    }
}

void	XFRow::AddCell(XFCell *pCell)
{
    if( !pCell )
        return;
/*	sal_Int32	col = pCell->GetCol();
    if( col<1 )
    {
        pCell->SetCol(m_aCells.size()+1);
    }

    col = pCell->GetCol();
    if( m_aCells.find(col) != m_aCells.end() )
        delete m_aCells[col];
*/
    sal_Int32 col = m_aCells.size()+1;
    pCell->SetCol(col);
    pCell->SetOwnerRow(this);
    m_aCells[col]=pCell;
}

sal_Int32	XFRow::GetCellCount() const
{
    return m_aCells.size();
}

XFCell*	XFRow::GetCell(sal_Int32 col) const
{
    if( m_aCells.find(col) == m_aCells.end() )
        return NULL;
    else
        return m_aCells.find(col)->second;
}

rtl::OUString	XFRow::GetRowName()
{
    if( m_pOwnerTable && m_pOwnerTable->IsSubTable() )
        return m_pOwnerTable->GetTableName();
    return A2OUSTR("");
}

void	XFRow::ToXml(IXFStream *pStrm)
{
    sal_Int32	lastCol = 0;
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if( GetStyleName().getLength() )
        pAttrList->AddAttribute( A2OUSTR("table:style-name"), GetStyleName() );
    if( m_nRepeat )
        pAttrList->AddAttribute( A2OUSTR("table:number-rows-repeated"), Int32ToOUString(m_nRepeat) );
    pStrm->StartElement( A2OUSTR("table:table-row") );

    std::map<sal_Int32,XFCell*>::iterator	it = m_aCells.begin();
    for( ; it!=m_aCells.end(); it++ )
    {
        int	col = (*it).first;
        XFCell	*pCell = (*it).second;
        if( !pCell )
            continue;
        if( col>lastCol+1 )
        {
            XFCell *pNULLCell = new XFCell();
            if( col>lastCol+2 )
                pNULLCell->SetRepeated(col-lastCol-1);
            pNULLCell->ToXml(pStrm);
        }
        pCell->ToXml(pStrm);
        lastCol = col;
    }

    pStrm->EndElement( A2OUSTR("table:table-row") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
