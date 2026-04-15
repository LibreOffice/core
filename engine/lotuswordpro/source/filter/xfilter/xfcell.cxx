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
 * Table cell.
 ************************************************************************/
#include <xfilter/xfcell.hxx>

#include <stdexcept>

#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfparagraph.hxx>
#include <xfilter/xftable.hxx>
#include <xfilter/xfrow.hxx>
#include <xfilter/xfutil.hxx>

XFCell::XFCell()
    : m_pOwnerRow(nullptr)
    , m_nCol(0)
    , m_nColSpaned(1)
    , m_nRepeated(0)
    , m_eValueType(enumXFValueTypeNone)
    , m_bProtect(false)
{}

XFCell::~XFCell()
{
}

void    XFCell::Add(XFContent *pContent)
{
    if( m_eValueType != enumXFValueTypeNone )
    {
        Reset();
        m_eValueType = enumXFValueTypeNone;
    }
    if (m_pSubTable.is())
    {
        throw std::runtime_error("subtable already set");
    }
    if (!pContent)
    {
        throw std::runtime_error("no content");
    }
    if( pContent->GetContentType() == enumXFContentTable )
    {
        XFTable *pTable = dynamic_cast<XFTable*>(pContent);
        if( !pTable )
            return;
        //the sub table will fill all the cell, there can't be other contents.
        pTable->SetOwnerCell(this);
        m_pSubTable = pTable;
    }
    else if( pContent->GetContentType() == enumXFContentText )
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->Add(pContent);
        XFContentContainer::Add(pPara);
    }
    else if( pContent->GetContentType() == enumXFContentFrame )
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->Add(pContent);
        XFContentContainer::Add(pPara);
    }
    else
    {
        XFContentContainer::Add(pContent);
    }
}

void    XFCell::SetValue(double value)
{
    SetValue(OUString::number(value));
}

void    XFCell::SetValue(const OUString& value)
{
    m_eValueType = enumXFValueTypeFloat;
    m_strValue = value;
}

OUString   XFCell::GetCellName()
{
    XFRow *pRow = m_pOwnerRow;

    if( !pRow )
        return OUString();

    XFTable *pTable = pRow->GetOwnerTable();

    if( !pTable )
        return OUString();

    OUString name;
    if( pTable->IsSubTable() )
    {
        name = pTable->GetTableName() + "." + OUString::number(m_nCol) + "." + OUString::number(pRow->GetRow());
    }
    else
    {
        name = GetTableColName(m_nCol) + OUString::number(pRow->GetRow());
    }
    return name;
}

void XFCell::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if( !GetStyleName().isEmpty() )
        pAttrList->AddAttribute( u"table:style-name"_ustr, GetStyleName() );
    if( m_nColSpaned>1 )
        pAttrList->AddAttribute( u"table:number-columns-spanned"_ustr, OUString::number(m_nColSpaned) );
    if( m_nRepeated )
        pAttrList->AddAttribute( u"table:number-columns-repeated"_ustr, OUString::number(m_nRepeated) );
    if( m_eValueType != enumXFValueTypeNone )
    {
        pAttrList->AddAttribute( u"table:value-type"_ustr, GetValueType(m_eValueType) );
        pAttrList->AddAttribute( u"table:value"_ustr, m_strValue );
    }
    if( !m_strFormula.isEmpty() )
        pAttrList->AddAttribute( u"table:formula"_ustr, m_strFormula );

    if( m_bProtect )
        pAttrList->AddAttribute( u"table:protected"_ustr, u"true"_ustr );

    //for test only.
//  pAttrList->AddAttribute( "table:cell-name", GetCellName() );

    pStrm->StartElement( u"table:table-cell"_ustr );

    if( m_pSubTable.is() )
        m_pSubTable->ToXml(pStrm);
    else
    {
        XFContentContainer::ToXml(pStrm);
    }

    pStrm->EndElement( u"table:table-cell"_ustr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
