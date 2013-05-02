/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Style: A base class from which all other styles are inherited, includes
 * a name.
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

#ifndef _STYLE_H
#define _STYLE_H
#include <libwpd/libwpd.h>

#include "DocumentElement.hxx"

class TopLevelElementStyle
{
public:
    TopLevelElementStyle() : mpsMasterPageName(0) {}
    virtual ~TopLevelElementStyle()
    {
        if (mpsMasterPageName) delete mpsMasterPageName;
    }
    void setMasterPageName(WPXString &sMasterPageName)
    {
        mpsMasterPageName = new WPXString(sMasterPageName);
    }
    const WPXString *getMasterPageName() const
    {
        return mpsMasterPageName;
    }

private:
    TopLevelElementStyle(const TopLevelElementStyle &);
    TopLevelElementStyle &operator=(const TopLevelElementStyle &);
    WPXString *mpsMasterPageName;
};

class Style
{
public:
    Style(const WPXString &psName) : msName(psName) {}
    virtual ~Style() {}

    virtual void write(OdfDocumentHandler *) const {};
    const WPXString &getName() const
    {
        return msName;
    }

private:
    WPXString msName;
};

class StyleManager
{
public:
    StyleManager() {}
    virtual ~StyleManager() {}

    virtual void clean() {};
    virtual void write(OdfDocumentHandler *) const = 0;

private:
    // forbide copy constructor/operator
    StyleManager(const StyleManager &);
    StyleManager &operator=(const StyleManager &);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
