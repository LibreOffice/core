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
/*************************************************************************
 * Change History
 * 2005-01-21 create this file.
 ************************************************************************/
#include	"xfcell.hxx"
#include	"xfparagraph.hxx"
#include	"xftable.hxx"
#include	"xfrow.hxx"

XFCell::XFCell()
{
    m_pSubTable = NULL;
    m_nCol = 0;
    m_nColSpaned = 1;
    m_nRepeated = 0;
    m_eValueType = enumXFValueTypeNone;
    m_bProtect = sal_False;
}

XFCell::XFCell(const XFCell& other) : XFContentContainer(other)
{
    if( other.m_pSubTable )
        m_pSubTable = new XFTable(*other.m_pSubTable);
    else
        m_pSubTable = NULL;
    m_pOwnerRow = NULL;
    m_nCol = other.m_nCol;
    m_nColSpaned = other.m_nColSpaned;
    m_nRepeated = other.m_nRepeated;
    m_eValueType = other.m_eValueType;
    m_bProtect = other.m_bProtect;
    m_strValue = other.m_strValue;
    m_strDisplay = other.m_strDisplay;
    m_strFormula = other.m_strFormula;
}

XFCell&	XFCell::operator=(const XFCell& other)
{
    if( m_pSubTable )
        delete m_pSubTable;

    if( other.m_pSubTable )
        m_pSubTable = new XFTable(*other.m_pSubTable);
    else
        m_pSubTable = NULL;
    m_pOwnerRow = NULL;
    m_nCol = other.m_nCol;
    m_nColSpaned = other.m_nColSpaned;
    m_nRepeated = other.m_nRepeated;
    m_eValueType = other.m_eValueType;
    m_bProtect = other.m_bProtect;
    m_strValue = other.m_strValue;
    m_strDisplay = other.m_strDisplay;
    m_strFormula = other.m_strFormula;

    return *this;
}

XFCell::~XFCell()
{
    if( m_pSubTable )
        delete m_pSubTable;
}

void	XFCell::Add(IXFContent *pContent)
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
        XFTable	*pTable = static_cast<XFTable*>(pContent);
        if( !pTable )
            return;
        //the sub table will fill all the cell, there can't be other contents.
        pTable->SetOwnerCell(this);
        m_pSubTable = pTable;
    }
    else if( pContent->GetContentType() == enumXFContentText )
    {
        XFParagraph	*pPara = new XFParagraph();
        pPara->Add(pContent);
        XFContentContainer::Add(pPara);
    }
    else if( pContent->GetContentType() == enumXFContentFrame )
    {
        XFParagraph	*pPara = new XFParagraph();
        pPara->Add(pContent);
        XFContentContainer::Add(pPara);
    }
    else
    {
        XFContentContainer::Add(pContent);
    }
}


void	XFCell::SetValue(double value)
{
    SetValue(DoubleToOUString(value,18));
}

void	XFCell::SetValue(rtl::OUString value)
{
    m_eValueType = enumXFValueTypeFloat;
    m_strValue = value;

    //Reset();
    //XFParagraph	*pPara = new XFParagraph();
    //pPara->Add(m_strValue);
    //XFContentContainer::Add(pPara);
}

void XFCell::SetString(rtl::OUString str)
{
    m_eValueType = enumXFValueTypeString;
    m_strValue = str;

    Reset();
    XFParagraph	*pPara = new XFParagraph();
    pPara->Add(m_strValue);
    XFContentContainer::Add(pPara);
}

void	XFCell::SetPercent(double percent)
{
    SetPercent(DoubleToOUString(percent));
}

void	XFCell::SetPercent(rtl::OUString percent)
{
    m_eValueType = enumXFValueTypePercentage;
    m_strValue = percent;

    Reset();
    XFParagraph	*pPara = new XFParagraph();
    pPara->Add(m_strValue);
    XFContentContainer::Add(pPara);
}

void	XFCell::SetDate(rtl::OUString date)
{
    m_eValueType = enumXFValueTypeDate;
    m_strValue = date;

    Reset();
    XFParagraph	*pPara = new XFParagraph();
    pPara->Add(m_strValue);
    XFContentContainer::Add(pPara);
}

void	XFCell::SetTime(rtl::OUString time)
{
    m_eValueType = enumXFValueTypeTime;
    m_strValue = time;

    Reset();
    XFParagraph	*pPara = new XFParagraph();
    pPara->Add(m_strValue);
    XFContentContainer::Add(pPara);
}

void	XFCell::SetCurrency(rtl::OUString currency)
{
    m_eValueType = enumXFValueTypeCurrency;
    m_strValue = currency;

    Reset();
    XFParagraph	*pPara = new XFParagraph();
    pPara->Add(m_strValue);
    XFContentContainer::Add(pPara);
}

void	XFCell::SetDisplay(rtl::OUString display)
{
    m_strDisplay = display;

    Reset();
    XFParagraph	*pPara = new XFParagraph();
    pPara->Add(m_strDisplay);
    XFContentContainer::Add(pPara);
}

rtl::OUString	XFCell::GetCellName()
{
    rtl::OUString	name;
    XFRow	*pRow = m_pOwnerRow;
    XFTable	*pTable = pRow->GetOwnerTable();

    if( !pRow || !pTable )
        return A2OUSTR("");
    if( pTable->IsSubTable() )
    {
        name = pTable->GetTableName() + A2OUSTR(".") + Int32ToOUString(m_nCol) + A2OUSTR(".") + Int32ToOUString(pRow->GetRow());
    }
    else
    {
        name = GetTableColName(m_nCol) + Int32ToOUString(pRow->GetRow());
    }
    return name;
}

void	XFCell::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    if( GetStyleName().getLength() )
        pAttrList->AddAttribute( A2OUSTR("table:style-name"), GetStyleName() );
    if( m_nColSpaned>1 )
        pAttrList->AddAttribute( A2OUSTR("table:number-columns-spanned"), Int32ToOUString(m_nColSpaned) );
    if( m_nRepeated )
        pAttrList->AddAttribute( A2OUSTR("table:number-columns-repeated"), Int32ToOUString(m_nRepeated) );
    if( m_eValueType != enumXFValueTypeNone )
    {
        pAttrList->AddAttribute( A2OUSTR("table:value-type"), GetValueType(m_eValueType) );
        pAttrList->AddAttribute( A2OUSTR("table:value"), m_strValue );
    }
    if( m_strFormula.getLength() > 0 )
        pAttrList->AddAttribute( A2OUSTR("table:formula"), m_strFormula );

    if( m_bProtect )
        pAttrList->AddAttribute( A2OUSTR("table:protected"), A2OUSTR("true") );

    //for test only.
//	pAttrList->AddAttribute( A2OUSTR("table:cell-name"), GetCellName() );

    pStrm->StartElement( A2OUSTR("table:table-cell") );

    if( m_pSubTable )
        m_pSubTable->ToXml(pStrm);
    else
    {
        XFContentContainer::ToXml(pStrm);
    }

    pStrm->EndElement( A2OUSTR("table:table-cell") );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
