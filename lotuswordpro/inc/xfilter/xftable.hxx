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
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFTABLE_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFTABLE_HXX

#include <xfilter/xfcontent.hxx>
#include <xfilter/xfrow.hxx>
#include <xfilter/xfcell.hxx>
#include <xfilter/xfcontentcontainer.hxx>
#include <map>
#include <vector>

class XFTable : public XFContent
{
public:
    XFTable();

    virtual ~XFTable() override;

public:
    void    SetTableName(const OUString& name);

    void    SetColumnStyle(sal_Int32 col, const OUString& style);

    void    AddRow(rtl::Reference<XFRow> const & rRow);

    void    AddHeaderRow(XFRow *pRow);

public:
    void    SetOwnerCell(XFCell *pCell);

    OUString   GetTableName();

    sal_uInt16  GetRowCount();

    XFRow*      GetRow(sal_Int32 row);

    sal_Int32   GetColumnCount();

    bool    IsSubTable();

    enumXFContent   GetContentType() override;

    virtual void    ToXml(IXFStream *pStrm) override;

    void RemoveRow(sal_uInt16 row)
    {
        m_aRows.erase(row);
    }

private:
    OUString   m_strName;
    bool    m_bSubTable;
    XFCell      *m_pOwnerCell;
    rtl::Reference<XFContentContainer>  m_aHeaderRows;
    std::map<sal_uInt16, rtl::Reference<XFRow>>  m_aRows;
    std::map<sal_Int32,OUString>   m_aColumns;
    OUString   m_strDefCellStyle;
    OUString   m_strDefRowStyle;
    OUString   m_strDefColStyle;
};

inline void XFTable::SetTableName(const OUString& name)
{
    m_strName = name;
}

inline void XFTable::SetOwnerCell(XFCell *pCell)
{
    m_pOwnerCell = pCell;
    m_bSubTable = (pCell!=nullptr);
}

inline bool XFTable::IsSubTable()
{
    return m_bSubTable;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
