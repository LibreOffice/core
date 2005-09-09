/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regmap.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:17:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _REGMAP_HXX_
#define _REGMAP_HXX_

#include <hash_map>
#include <list>
#include <set>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
