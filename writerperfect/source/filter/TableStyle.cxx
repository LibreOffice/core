/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* TableStyle: Stores (and writes) table-based information that is 
 * needed at the head of an OO document.
 *
 * Copyright (C) 2002-2004 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2004 Net Integration Technologies, Inc. (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 *
 */

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */
#include <math.h>
#include "FilterInternal.hxx"
#include "TableStyle.hxx"
#include "DocumentElement.hxx"

#ifdef _MSC_VER
#include <minmax.h>
#endif

#include <string.h>

TableCellStyle::TableCellStyle(const WPXPropertyList &xPropList, const char *psName) :
    Style(psName),
        mPropList(xPropList)
{
}

void TableCellStyle::write(DocumentHandlerInterface *pHandler) const
{
    TagOpenElement styleOpen("style:style");
    styleOpen.addAttribute("style:name", getName());
    styleOpen.addAttribute("style:family", "table-cell");
    styleOpen.write(pHandler);

        // WLACH_REFACTORING: Only temporary.. a much better solution is to
        // generalize this sort of thing into the "Style" superclass
        WPXPropertyList stylePropList;
        WPXPropertyList::Iter i(mPropList);
        for (i.rewind(); i.next();)
        {
                if (strlen(i.key()) > 2 && strncmp(i.key(), "fo", 2) == 0)
                        stylePropList.insert(i.key(), i()->clone());
        }
        stylePropList.insert("fo:padding", "0.0382in");
        pHandler->startElement("style:table-cell-properties", stylePropList);
    pHandler->endElement("style:table-cell-properties");

    pHandler->endElement("style:style");	
}

TableRowStyle::TableRowStyle(const WPXPropertyList &propList, const char *psName) :
    Style(psName),
        mPropList(propList)
{
}

void TableRowStyle::write(DocumentHandlerInterface *pHandler) const
{
    TagOpenElement styleOpen("style:style");
    styleOpen.addAttribute("style:name", getName());
    styleOpen.addAttribute("style:family", "table-row");
    styleOpen.write(pHandler);
    
        TagOpenElement stylePropertiesOpen("style:table-row-properties");
        if (mPropList["style:min-row-height"])
                stylePropertiesOpen.addAttribute("style:min-row-height", mPropList["style:min-row-height"]->getStr());
        else if (mPropList["style:row-height"])
                stylePropertiesOpen.addAttribute("style:row-height", mPropList["style:row-height"]->getStr());
    stylePropertiesOpen.addAttribute("fo:keep-together", "auto");
        stylePropertiesOpen.write(pHandler);
        pHandler->endElement("style:table-row-properties");
    
    pHandler->endElement("style:style");		
}
    

TableStyle::TableStyle(const WPXPropertyList &xPropList, const WPXPropertyListVector &columns, const char *psName) : 
    Style(psName),
        mPropList(xPropList),
        mColumns(columns)
{
}

TableStyle::~TableStyle()
{
    typedef std::vector<TableCellStyle *>::iterator TCSVIter;
    typedef std::vector<TableRowStyle *>::iterator TRSVIter;
    for (TCSVIter iterTableCellStyles = mTableCellStyles.begin() ; iterTableCellStyles != mTableCellStyles.end(); iterTableCellStyles++)
        delete(*iterTableCellStyles);
    for (TRSVIter iterTableRowStyles = mTableRowStyles.begin() ; iterTableRowStyles != mTableRowStyles.end(); iterTableRowStyles++)
        delete(*iterTableRowStyles);
}

void TableStyle::write(DocumentHandlerInterface *pHandler) const
{
    TagOpenElement styleOpen("style:style");
    styleOpen.addAttribute("style:name", getName());
    styleOpen.addAttribute("style:family", "table");
    if (getMasterPageName())
        styleOpen.addAttribute("style:master-page-name", getMasterPageName()->cstr());
    styleOpen.write(pHandler);

    TagOpenElement stylePropertiesOpen("style:table-properties");
        if (mPropList["table:align"])
                stylePropertiesOpen.addAttribute("table:align", mPropList["table:align"]->getStr());
    if (mPropList["fo:margin-left"])
        stylePropertiesOpen.addAttribute("fo:margin-left", mPropList["fo:margin-left"]->getStr());
    if (mPropList["fo:margin-right"])
        stylePropertiesOpen.addAttribute("fo:margin-right", mPropList["fo:margin-right"]->getStr());
    if (mPropList["style:width"])
        stylePropertiesOpen.addAttribute("style:width", mPropList["style:width"]->getStr());
    if (mPropList["fo:break-before"])
        stylePropertiesOpen.addAttribute("fo:break-before", mPropList["fo:break-before"]->getStr());
    stylePropertiesOpen.write(pHandler);

    pHandler->endElement("style:table-properties");

    pHandler->endElement("style:style");
        
    int i=1;
        WPXPropertyListVector::Iter j(mColumns);
    for (j.rewind(); j.next();)
    {
        TagOpenElement styleOpen2("style:style");
        WPXString sColumnName;
        sColumnName.sprintf("%s.Column%i", getName().cstr(), i);
        styleOpen2.addAttribute("style:name", sColumnName);
        styleOpen2.addAttribute("style:family", "table-column");
        styleOpen2.write(pHandler);

        pHandler->startElement("style:table-column-properties", j());
        pHandler->endElement("style:table-column-properties");

        pHandler->endElement("style:style");

        i++;
    }

    typedef std::vector<TableRowStyle *>::const_iterator TRSVIter;
    for (TRSVIter iterTableRow = mTableRowStyles.begin() ; iterTableRow != mTableRowStyles.end(); iterTableRow++)
        (*iterTableRow)->write(pHandler);

    typedef std::vector<TableCellStyle *>::const_iterator TCSVIter;
    for (TCSVIter iterTableCell = mTableCellStyles.begin() ; iterTableCell != mTableCellStyles.end(); iterTableCell++)
        (*iterTableCell)->write(pHandler);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
