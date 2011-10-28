/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FilterInternal.hxx: define basic functions for libwriterperfect/fileter
 *
 * Copyright (C) 2002-2004 William Lachance (wrlach@gmail.com)
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

#include <libwpd/WPXPropertyList.h>

#include "FilterInternal.hxx"

WPXString propListToStyleKey(const WPXPropertyList &xPropList)
{
    WPXString sKey;
    WPXPropertyList::Iter i(xPropList);
    for (i.rewind(); i.next(); )
    {
        WPXString sProp;
        sProp.sprintf("[%s:%s]", i.key(), i()->getStr().cstr());
        sKey.append(sProp);
    }

    return sKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
