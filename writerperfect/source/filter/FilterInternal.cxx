/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FilterInternal.hxx: define basic functions for libwriterperfect/fileter
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
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
