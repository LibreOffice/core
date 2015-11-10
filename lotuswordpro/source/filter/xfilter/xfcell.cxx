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
#include "xfcell.hxx"
#include "xfparagraph.hxx"
#include "xftable.hxx"
#include "xfrow.hxx"

XFCell::XFCell()
{
    m_pSubTable = nullptr;
    m_pOwnerRow = nullptr;
    m_nCol = 0;
    m_nColSpaned = 1;
    m_nRepeated = 0;
    m_eValueType = enumXFValueTypeNone;
    m_bProtect = false;
}

XFCell::~XFCell()
{
    delete m_pSubTable;
}

void    XFCell::Add(XFContent *pContent)
{
    if( m_eValueType != enumXFValueTypeNone )
    {
        Reset();
        m_eValueType = enumXFValueTypeNone;
    }
    if( m_pSubTable )
    {
        assert(false);
        return;
    }
    if( pContent->GetContentType() == enumXFContentTable )
    {
        XFTable *pTable = static_cast<XFTable*>(pContent);
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
        return OUString("");

    XFTable *pTable = pRow->GetOwnerTable();

    if( !pTable )
        return OUString("");

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

void    XFCell::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if( !GetStyleName().isEmpty() )
        pAttrList->AddAttribute( "table:style-name", GetStyleName() );
    if( m_nColSpaned>1 )
        pAttrList->AddAttribute( "table:number-columns-spanned", OUString::number(m_nColSpaned) );
    if( m_nRepeated )
        pAttrList->AddAttribute( "table:number-columns-repeated", OUString::number(m_nRepeated) );
    if( m_eValueType != enumXFValueTypeNone )
    {
        pAttrList->AddAttribute( "table:value-type", GetValueType(m_eValueType) );
        pAttrList->AddAttribute( "table:value", m_strValue );
    }
    if( !m_strFormula.isEmpty() )
        pAttrList->AddAttribute( "table:formula", m_strFormula );

    if( m_bProtect )
        pAttrList->AddAttribute( "table:protected", "true" );

    //for test only.
//  pAttrList->AddAttribute( "table:cell-name", GetCellName() );

    pStrm->StartElement( "table:table-cell" );

    if( m_pSubTable )
        m_pSubTable->ToXml(pStrm);
    else
    {
        XFContentContainer::ToXml(pStrm);
    }

    pStrm->EndElement( "table:table-cell" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
