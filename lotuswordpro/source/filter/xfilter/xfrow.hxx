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
 * Table row object. A table is consist by rows.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-28 create and implements.
 ************************************************************************/
#ifndef		_XFROW_HXX
#define		_XFROW_HXX

#include	"xfcontent.hxx"
#include	<map>

class XFCell;
class XFTable;
class XFRow : public XFContent
{
public:
    XFRow();

    XFRow(const XFRow& other);

    XFRow& operator=(const XFRow& other);

    virtual ~XFRow();

public:
    void	AddCell(XFCell *pCell);

    void	SetRepeated(sal_Int32 repeat);

    void	SetRow(sal_Int32 row);

    void	SetOwnerTable(XFTable *pTable);

    sal_Int32	GetRow();

    sal_Int32	GetCellCount() const;

    XFCell*		GetCell(sal_Int32 col) const;

    XFTable*	GetOwnerTable();

    rtl::OUString	GetRowName();

    virtual void	ToXml(IXFStream *pStrm);

    void		RemoveCell(sal_Int32 cell);

private:
    XFTable		*m_pOwnerTable;
    std::map<sal_Int32,XFCell*>	m_aCells;
    sal_Int32	m_nRepeat;
    sal_Int32	m_nRow;
};

inline void XFRow::SetRepeated(sal_Int32 repeat)
{
    m_nRepeat = repeat;
}

inline void XFRow::SetRow(sal_Int32 row)
{
    m_nRow = row;
}

inline void XFRow::SetOwnerTable(XFTable *pTable)
{
    m_pOwnerTable = pTable;
}

inline sal_Int32 XFRow::GetRow()
{
    return m_nRow;
}

inline XFTable*	XFRow::GetOwnerTable()
{
    return m_pOwnerTable;
}

inline void XFRow::RemoveCell(sal_Int32 cell)
{
    m_aCells.erase(cell);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
