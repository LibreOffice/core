/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regmap.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _REGMAP_HXX_
#define _REGMAP_HXX_

#include <hash_map>
#include <list>
#include <set>
#include <rtl/ustring.hxx>

using namespace rtl;

class ORegKey;

struct EqualString
{
    sal_Bool operator()(const OUString& str1, const OUString& str2) const
    {
        return (str1 == str2);
    }
};

struct HashString
{
    size_t operator()(const OUString& str) const
    {
        return str.hashCode();
    }
};

struct LessString
{
    sal_Bool operator()(const OUString& str1, const OUString& str2) const
    {
        return (str1 < str2);
    }
};

typedef ::std::hash_map< OUString, ORegKey*, HashString, EqualString > KeyMap;

typedef ::std::list< ORegKey* > LinkList;

typedef ::std::set< OUString, LessString >  StringSet;

#endif
