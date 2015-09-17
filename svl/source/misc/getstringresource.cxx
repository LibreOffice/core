/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <map>
#include <memory>
#include <boost/noncopyable.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/instance.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/resmgr.hxx>
#include <tools/simplerm.hxx>

#include "getstringresource.hxx"

namespace {

class ResMgrMap: private boost::noncopyable {
public:
    ~ResMgrMap();

    SimpleResMgr * get(LanguageTag const & locale);

private:
    typedef std::map< OUString, SimpleResMgr * > Map;

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

SimpleResMgr * ResMgrMap::get(LanguageTag const & locale) {
    OUString code( locale.getBcp47());
    Map::iterator i(map_.find(code));
    if (i == map_.end()) {
        std::unique_ptr< SimpleResMgr > mgr(
            new SimpleResMgr("svl", locale));
        i = map_.insert(Map::value_type(code, mgr.get())).first;
        mgr.reset();
    }
    return i->second;
}

struct theResMgrMap: public rtl::Static< ResMgrMap, theResMgrMap > {};

}

namespace svl {

OUString getStringResource(sal_uInt16 id, LanguageTag const & locale)
{
    return theResMgrMap::get().get(locale)->ReadString(id);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
