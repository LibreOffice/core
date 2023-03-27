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

#ifndef INCLUDED_XMLOFF_SINGLEPROPERTYSETINFOCACHE_HXX
#define INCLUDED_XMLOFF_SINGLEPROPERTYSETINFOCACHE_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/uno/Reference.hxx>
#include <utility>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::beans { class XPropertySetInfo; }

class SinglePropertySetInfoCache
{
    typedef std::map<css::uno::Reference<css::beans::XPropertySetInfo>, bool>
        Map;

    OUString m_sName;
    Map m_Map;

public:

    inline SinglePropertySetInfoCache( OUString sName );

    bool hasProperty(
            const css::uno::Reference< css::beans::XPropertySet >& rPropSet,
            css::uno::Reference< css::beans::XPropertySetInfo >& rPropSetInfo );

    inline bool hasProperty(
            const css::uno::Reference< css::beans::XPropertySet>& rPropSet );
};

inline SinglePropertySetInfoCache::SinglePropertySetInfoCache(
        OUString _sName ) :
    m_sName(std::move( _sName ))
{
}

inline bool SinglePropertySetInfoCache::hasProperty(
        const css::uno::Reference< css::beans::XPropertySet>& rPropSet )
{
    css::uno::Reference< css::beans::XPropertySetInfo > xInfo;
    return hasProperty( rPropSet, xInfo );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
