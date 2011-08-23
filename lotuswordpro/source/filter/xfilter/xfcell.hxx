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
#ifndef		_XFCell_HXX
#define		_XFCell_HXX

#include	"xfcontent.hxx"
#include	"xfcontentcontainer.hxx"
#include	<vector>

class XFTable;
class XFRow;

/**
 * @descr	Table cell object.
 */
class XFCell : public XFContentContainer
{
public:
    XFCell();

    XFCell(const XFCell& other);

    XFCell&	operator=(const XFCell& other);

    virtual ~XFCell();

public:
    /**
     * @descr	Add content for table cell.
     */
    void	Add(IXFContent *pContent);

    /**
     * @descr	If cell spans more the one column, then set column span.
     */
    void	SetColumnSpaned(sal_Int32 num);

    /**
     * @descr	Set whether the following cells use the same style and content.
     */
    void	SetRepeated(sal_Int32 num);

    /**
     * @descr	Set cell number value.
     */
    void	SetValue(double value);

    /**
     * @descr	Set cell number value.
     */
    void	SetValue(rtl::OUString value);

    /**
     * @descr	Set cell string value.
     */
    void	SetString(rtl::OUString str);

    /**
     * @descr	Set cell as a percent value with value percent.
     */
    void	SetPercent(double percent);

    /**
     * @descr	Set cell as a percent value with value percent.
     */
    void	SetPercent(rtl::OUString percent);

    /**
     * @descr	Set cell as a date value.
     */
    void	SetDate(rtl::OUString date);

    /**
     * @descr	Set cell as a time value.
     */
    void	SetTime(rtl::OUString time);

    /**
     * @descr	Set cell as a currency value.
     */
    void	SetCurrency(rtl::OUString currency);

    /**
     * @descr	Set cell display string.
     */
    void	SetDisplay(rtl::OUString display);

    /**
     * @descr	Set cell formula.
     */
    void	SetFormula(rtl::OUString formula);

    /**
     * @descr	Set cell protected.
     */
    void	SetProtect(sal_Bool protect=sal_True);

    /**
     * @descr	Set cell column id.
     */
    void	SetCol(sal_Int32 col);

    /**
     * @descr	Set cell owner row.
     */
    void	SetOwnerRow(XFRow *pRow);

    /**
     * @descr	Return cell name. It's a tool function for formula.
     */
    rtl::OUString	GetCellName();

    /**
     * @descr	Return cell column id.
     */
    sal_Int32	GetCol();

    /**
     * @descr	return cell'owner row.
     */
    XFRow*		GetOwnerRow();

    /**
     * @descr	If cell has a sub-table, return it, else return NULL.
     */
    XFTable*	GetSubTable();

    /**
     * @descr	return cell column span property.
     */
    sal_Int32	GetColSpaned();

    /**
     * @descr	Output cell as xml element.
     */
    virtual void	ToXml(IXFStream *pStrm);

private:
    XFRow		*m_pOwnerRow;
    XFTable		*m_pSubTable;
    sal_Int32	m_nCol;
    sal_Int32	m_nColSpaned;
    sal_Int32	m_nRepeated;
    enumXFValueType	m_eValueType;
    rtl::OUString	m_strValue;
    rtl::OUString	m_strDisplay;
    rtl::OUString	m_strFormula;
    sal_Bool	m_bProtect;
};

inline void	XFCell::SetColumnSpaned(sal_Int32 num)
{
    m_nColSpaned = num;
}

inline void	XFCell::SetRepeated(sal_Int32 repeated)
{
    m_nRepeated = repeated;
}

inline void XFCell::SetFormula(rtl::OUString formula)
{
    m_strFormula = formula;
}

inline void XFCell::SetProtect(sal_Bool protect/* =sal_True */)
{
    m_bProtect = protect;
}

inline void XFCell::SetCol(sal_Int32 col)
{
    m_nCol = col;
}

inline void	XFCell::SetOwnerRow(XFRow *pRow)
{
    m_pOwnerRow = pRow;
}

inline sal_Int32	XFCell::GetCol()
{
    return m_nCol;
}

inline XFRow*	XFCell::GetOwnerRow()
{
    return m_pOwnerRow;
}

inline XFTable*	XFCell::GetSubTable()
{
    return m_pSubTable;
}

inline sal_Int32 XFCell::GetColSpaned()
{
    return m_nColSpaned;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
