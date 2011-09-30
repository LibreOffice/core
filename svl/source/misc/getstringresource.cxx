/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_svl.hxx"
#include "sal/config.h"

#include <map>

#include "boost/noncopyable.hpp"
#include "boost/scoped_ptr.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "rtl/instance.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "tools/resmgr.hxx"
#include "tools/simplerm.hxx"

#include "getstringresource.hxx"

namespace {

namespace css = com::sun::star;

class ResMgrMap: private boost::noncopyable {
public:
    ~ResMgrMap();

    SimpleResMgr * get(css::lang::Locale const & locale);

private:
    typedef std::map< rtl::OUString, SimpleResMgr * > Map;

    Map map_;
        // one SimpleResMgr for each language for which a resource was requested
        // (when using the "non-simple" resmgr, the first request for any
        // language wins, any further requests for any other languages supply
        // the resmgr of the first call; for the simple resmgr we have a mgr
        // for each language ever requested)
};

ResMgrMap::~ResMgrMap() {
    for (Map::iterator i(map_.begin()); i != map_.end(); ++i) {
        delete i->second;
    }
}

SimpleResMgr * ResMgrMap::get(css::lang::Locale const & locale) {
    rtl::OUStringBuffer buf(locale.Language);
    buf.append(sal_Unicode('-'));
    buf.append(locale.Country);
    rtl::OUString code(buf.makeStringAndClear());
    Map::iterator i(map_.find(code));
    if (i == map_.end()) {
        boost::scoped_ptr< SimpleResMgr > mgr(
            new SimpleResMgr(CREATEVERSIONRESMGR_NAME(svl), locale));
        i = map_.insert(Map::value_type(code, mgr.get())).first;
        mgr.reset();
    }
    return i->second;
}

struct theResMgrMap: public rtl::Static< ResMgrMap, theResMgrMap > {};

}

namespace svl {

rtl::OUString getStringResource(sal_uInt16 id, css::lang::Locale const & locale)
{
    return theResMgrMap::get().get(locale)->ReadString(id);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
