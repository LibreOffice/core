/* libwriterperfect_filter.hxx: define basic functions for libwriterperfect/fileter
 *
 * Copyright (C) 2002-2003 William Lachance (wrlach@gmail.com)
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

#ifndef _LIBWRITERPERFECT_FILTER_H
#define _LIBWRITERPERFECT_FILTER_H
#include <string.h> // for strcmp

#include <libwpd/libwpd.h>
#include <libwpd/WPXString.h>

class WPXPropertyList;

WPXString propListToStyleKey(const WPXPropertyList & xPropList);

struct ltstr
{
    bool operator()(const WPXString & s1, const WPXString & s2) const
    {
        return strcmp(s1.cstr(), s2.cstr()) < 0;
    }
};


#endif
