/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Style: A base class from which all other styles are inherited, includes
 * a name.
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

#ifndef _STYLE_H
#define _STYLE_H
#if defined _MSC_VER
#pragma warning( push, 1 )
#endif
#include <libwpd/libwpd.h>
#if defined _MSC_VER
#pragma warning( pop )
#endif
#include "DocumentElement.hxx"

class TopLevelElementStyle
{
public:
    TopLevelElementStyle() : mpsMasterPageName(NULL) { }
    virtual ~TopLevelElementStyle() { if (mpsMasterPageName) delete mpsMasterPageName; }
    void setMasterPageName(WPXString &sMasterPageName) { mpsMasterPageName = new WPXString(sMasterPageName); }
    const WPXString * getMasterPageName() const { return mpsMasterPageName; }

private:
    WPXString *mpsMasterPageName;
};

class Style
{
 public:
    Style(const WPXString &psName) : msName(psName) {}
    virtual ~Style() {}

    virtual void write(DocumentHandler * /* pHandler */) const {};
    const WPXString &getName() const { return msName; }

 private:
    WPXString msName;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
