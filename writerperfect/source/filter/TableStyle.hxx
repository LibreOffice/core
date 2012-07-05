/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* TableStyle: Stores (and writes) table-based information that is
 * needed at the head of an OO document.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef _TABLESTYLE_H
#define _TABLESTYLE_H
#include <libwpd/libwpd.h>
#include <vector>

#include "Style.hxx"

class OdfDocumentHandler;

class TableCellStyle : public Style
{
public:
    virtual ~TableCellStyle() {};
    TableCellStyle(const WPXPropertyList &xPropList, const char *psName);
    virtual void write(OdfDocumentHandler *pHandler) const;
private:
    WPXPropertyList mPropList;
};

class TableRowStyle : public Style
{
public:
    virtual ~TableRowStyle() {};
    TableRowStyle(const WPXPropertyList &propList, const char *psName);
    virtual void write(OdfDocumentHandler *pHandler) const;
private:
    WPXPropertyList mPropList;
};

class TableStyle : public Style, public TopLevelElementStyle
{
public:
    TableStyle(const WPXPropertyList &xPropList, const WPXPropertyListVector &columns, const char *psName);
    virtual ~TableStyle();
    virtual void write(OdfDocumentHandler *pHandler) const;
    int getNumColumns() const
    {
        return mColumns.count();
    }
    void addTableCellStyle(TableCellStyle *pTableCellStyle)
    {
        mTableCellStyles.push_back(pTableCellStyle);
    }
    int getNumTableCellStyles()
    {
        return mTableCellStyles.size();
    }
    void addTableRowStyle(TableRowStyle *pTableRowStyle)
    {
        mTableRowStyles.push_back(pTableRowStyle);
    }
    int getNumTableRowStyles()
    {
        return mTableRowStyles.size();
    }
private:
    WPXPropertyList mPropList;
    WPXPropertyListVector mColumns;
    std::vector<TableCellStyle *> mTableCellStyles;
    std::vector<TableRowStyle *> mTableRowStyles;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
