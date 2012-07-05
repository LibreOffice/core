/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FilterInternal: Debugging information
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */
#ifndef __FILTERINTERNAL_HXX__
#define __FILTERINTERNAL_HXX__

#include <string.h> // for strcmp

#include <libwpd/libwpd.h>
#include <libwpd/WPXString.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// used by FontStyle.cxx
#define IMP_DEFAULT_FONT_PITCH "variable"

#if 0
const double fDefaultSideMargin = 1.0; // inches
const double fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const double fDefaultPageHeight = 11.0; // inches
#endif

#ifdef DEBUG
#include <stdio.h>
#define WRITER_DEBUG_MSG(M) printf M
#else
#define WRITER_DEBUG_MSG(M)
#endif

#if defined(SHAREDPTR_TR1)
#include <tr1/memory>
using std::tr1::shared_ptr;
#elif defined(SHAREDPTR_STD)
#include <memory>
using std::shared_ptr;
#else
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;
#endif


class WPXPropertyList;
WPXString propListToStyleKey(const WPXPropertyList &xPropList);

struct ltstr
{
    bool operator()(const WPXString &s1, const WPXString &s2) const
    {
        return strcmp(s1.cstr(), s2.cstr()) < 0;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
