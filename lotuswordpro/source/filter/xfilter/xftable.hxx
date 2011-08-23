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
#ifndef		_XFTABLE_HXX
#define		_XFTABLE_HXX

#include	"xfcontent.hxx"
#include	"xfrow.hxx"
#include	"xfcell.hxx"
#include	"xfcontentcontainer.hxx"
#include	<map>
#include	<vector>

class XFTable : public XFContent
{
public:
    XFTable();

    XFTable(const XFTable& other);

    XFTable& operator=(const XFTable& other);

    virtual ~XFTable();

public:
    void	SetTableName(rtl::OUString name);

    void	SetColumnStyle(sal_Int32 col, rtl::OUString style);

    void	AddRow(XFRow *pRow);

    void	AddHeaderRow(XFRow *pRow);

    void	SetDefaultCellStyle(rtl::OUString style);

    void	SetDefaultRowStyle(rtl::OUString style);

    void	SetDefaultColStyle(rtl::OUString style);

public:
    void	SetOwnerCell(XFCell *pCell);

    rtl::OUString	GetTableName();

    sal_Int32	GetRowCount();

    XFRow*		GetRow(sal_Int32 row);

    sal_Int32	GetColumnCount();

    rtl::OUString	GetColumnStyle(sal_Int32 col);

    sal_Bool	IsSubTable();

    void		Normalize();

    enumXFContent	GetContentType();

    virtual void	ToXml(IXFStream *pStrm);

    void		RemoveRow(sal_Int32 row);

private:
    rtl::OUString	m_strName;
    sal_Bool	m_bSubTable;
    XFCell		*m_pOwnerCell;
    XFContentContainer	m_aHeaderRows;
    std::map<sal_Int32,XFRow*>	m_aRows;
    std::map<sal_Int32,rtl::OUString>	m_aColumns;
    rtl::OUString	m_strDefCellStyle;
    rtl::OUString	m_strDefRowStyle;
    rtl::OUString	m_strDefColStyle;
};

inline void	XFTable::SetTableName(rtl::OUString name)
{
    m_strName = name;
}

inline void	XFTable::SetOwnerCell(XFCell *pCell)
{
    m_pOwnerCell = pCell;
    m_bSubTable = (pCell!=NULL);
}

inline sal_Bool XFTable::IsSubTable()
{
    return m_bSubTable;
}


inline void XFTable::SetDefaultCellStyle(rtl::OUString style)
{
    m_strDefCellStyle = style;
}

inline void XFTable::SetDefaultRowStyle(rtl::OUString style)
{
    m_strDefRowStyle = style;
}

inline void XFTable::SetDefaultColStyle(rtl::OUString style)
{
    m_strDefColStyle = style;
}

inline void XFTable::RemoveRow(sal_Int32 row)
{
    m_aRows.erase(row);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
