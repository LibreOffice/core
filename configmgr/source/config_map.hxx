/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef CONFIG_MAP_HXX
#define CONFIG_MAP_HXX

#include <map>
#include <rtl/ustring.hxx>

// The realisation here is that while a map is a reasonably compact
// representation, there is often no need to have it completely
// sorted, so we can use a fast in-line length comparison as the
// initial compare, rather than sorting of sub string contents.

struct LengthContentsCompare
{
    bool operator()(const OUString& a, const OUString& b) const
    {
        if (a.getLength() == b.getLength())
            return a < b;
        else
            return a.getLength() < b.getLength();
    }
};

template <class T> struct config_map : public std::map<OUString, T, LengthContentsCompare>
{
};

#endif // CONFIG_MAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
