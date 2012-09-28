/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ListStyle: Stores (and writes) list-based information that is
 * needed at the head of an OO document.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef _LISTSTYLE_H
#define _LISTSTYLE_H
#include <libwpd/libwpd.h>

#define WP6_NUM_LIST_LEVELS 8 // see WP6FileStructure.h (we shouldn't need to reference this)

#include "Style.hxx"

class ListLevelStyle
{
public:
    virtual ~ListLevelStyle() {};
    virtual void write(OdfDocumentHandler *pHandler, int iLevel) const = 0;
};

class OrderedListLevelStyle : public ListLevelStyle
{
public:
    OrderedListLevelStyle(const WPXPropertyList &xPropList);
    void write(OdfDocumentHandler *pHandler, int iLevel) const;
private:
    WPXPropertyList mPropList;
};

class UnorderedListLevelStyle : public ListLevelStyle
{
public:
    UnorderedListLevelStyle(const WPXPropertyList &xPropList);
    void write(OdfDocumentHandler *pHandler, int iLevel) const;
private:
    WPXPropertyList mPropList;
};

class ListStyle : public Style
{
public:
    ListStyle(const char *psName, const int iListID);
    virtual ~ListStyle();
    virtual void updateListLevel(const int iLevel, const WPXPropertyList &xPropList) = 0;
    virtual void write(OdfDocumentHandler *pHandler) const;
    int getListID()
    {
        return miListID;
    }
    bool isListLevelDefined(int iLevel) const;

protected:
    void setListLevel(int iLevel, ListLevelStyle *iListLevelStyle);

private:
    ListLevelStyle *mppListLevels[WP6_NUM_LIST_LEVELS];
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
