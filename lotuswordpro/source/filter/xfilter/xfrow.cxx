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

#include <sal/config.h>

#include <xfilter/ixfattrlist.hxx>
#include <xfilter/ixfstream.hxx>
#include <xfilter/xfrow.hxx>
#include <xfilter/xfcell.hxx>

XFRow::XFRow()
    : m_pOwnerTable(nullptr)
    , m_nRepeat(0)
    , m_nRow(0)
{
}

XFRow::~XFRow()
{
}

void XFRow::AddCell(rtl::Reference<XFCell> const & rCell)
{
    if (!rCell)
        return;
    sal_Int32 col = m_aCells.size() + 1;
    rCell->SetCol(col);
    rCell->SetOwnerRow(this);
    m_aCells.push_back(rCell);
}

sal_Int32 XFRow::GetCellCount() const
{
    return m_aCells.size();
}

// 1 based
XFCell* XFRow::GetCell(sal_Int32 col) const
{
    assert(col > 0);
    size_t nIndex = col - 1;
    if (nIndex < m_aCells.size())
        return m_aCells[nIndex].get();
    return nullptr;
}

void    XFRow::ToXml(IXFStream *pStrm)
{
    sal_Int32   lastCol = 0;
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if( !GetStyleName().isEmpty() )
        pAttrList->AddAttribute( u"table:style-name"_ustr, GetStyleName() );
    if( m_nRepeat )
        pAttrList->AddAttribute( u"table:number-rows-repeated"_ustr, OUString::number(m_nRepeat) );
    pStrm->StartElement( u"table:table-row"_ustr );

    for (size_t nIndex = 0, nCount = m_aCells.size(); nIndex < nCount; ++nIndex)
    {
        XFCell *pCell = m_aCells[nIndex].get();
        if (!pCell)
            continue;
        int col = nIndex + 1;
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

    pStrm->EndElement( u"table:table-row"_ustr );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
