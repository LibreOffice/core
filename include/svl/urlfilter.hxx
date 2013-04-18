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

#ifndef SVTOOLS_URL_FILTER_HXX
#define SVTOOLS_URL_FILTER_HXX

#include <tools/wldcrd.hxx>
#include <functional>
#include <vector>
/** filters allowed URLs
*/
class IUrlFilter
{
public:
    virtual bool isUrlAllowed( const OUString& _rURL ) const = 0;

protected:
    virtual inline ~IUrlFilter() = 0;
};

inline IUrlFilter::~IUrlFilter() {}

struct FilterMatch : public ::std::unary_function< bool, WildCard >
{
private:
    const OUString&   m_rCompareString;
public:
    FilterMatch( const OUString& _rCompareString ) : m_rCompareString( _rCompareString ) { }

    bool operator()( const WildCard& _rMatcher )
    {
        return _rMatcher.Matches( m_rCompareString ) ? true : false;
    }

    static void createWildCardFilterList(const OUString& _rFilterList,::std::vector< WildCard >& _rFilters);
};

#endif // SVTOOLS_URL_FILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
