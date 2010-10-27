/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ListStyle: Stores (and writes) list-based information that is
 * needed at the head of an OO document.
 *
 * Copyright (C) 2002-2003 William Lachance (william.lachance@sympatico.ca)
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
#ifndef _LISTSTYLE_H
#define _LISTSTYLE_H
#if defined _MSC_VER
#pragma warning( push, 1 )
#endif
#include <libwpd/libwpd.h>
#if defined _MSC_VER
#pragma warning( pop )
#endif

#define WP6_NUM_LIST_LEVELS 8 // see WP6FileStructure.h (we shouldn't need to reference this)

#include "Style.hxx"
#include "WriterProperties.hxx"

class DocumentElement;

class ListLevelStyle
{
public:
    virtual ~ListLevelStyle() {};
    virtual void write(DocumentHandler *pHandler, int iLevel) const = 0;
};

class OrderedListLevelStyle : public ListLevelStyle
{
public:
    OrderedListLevelStyle(const WPXPropertyList &xPropList);
    void write(DocumentHandler *pHandler, int iLevel) const;
private:
        WPXPropertyList mPropList;
};

class UnorderedListLevelStyle : public ListLevelStyle
{
public:
    UnorderedListLevelStyle(const WPXPropertyList &xPropList);
    void write(DocumentHandler *pHandler, int iLevel) const;
private:
        WPXPropertyList mPropList;
};

class ListStyle : public Style
{
public:
    ListStyle(const char *psName, const int iListID);
    virtual ~ListStyle();
    virtual void updateListLevel(const int iLevel, const WPXPropertyList &xPropList) = 0;
    virtual void write(DocumentHandler *pHandler) const;
    int getListID() const { return miListID; }
    bool isListLevelDefined(int iLevel) const;

protected:
    void setListLevel(int iLevel, ListLevelStyle *iListLevelStyle);

private:
    ListLevelStyle *mppListLevels[WP6_NUM_LIST_LEVELS];
    int miNumListLevels;
    const int miListID;
};

class OrderedListStyle : public ListStyle
{
public:
    OrderedListStyle(const char *psName, const int iListID) : ListStyle(psName, iListID) {}
    void updateListLevel(const int iLevel, const WPXPropertyList &xPropList);
};

class UnorderedListStyle : public ListStyle
{
public:
    UnorderedListStyle(const char *psName, const int iListID) : ListStyle(psName, iListID) {}
    void updateListLevel(const int iLevel, const WPXPropertyList &xPropList);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
