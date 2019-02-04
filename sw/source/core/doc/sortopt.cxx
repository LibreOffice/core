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

#include <i18nlangtag/lang.h>
#include <sortopt.hxx>

SwSortKey::SwSortKey() :
    eSortOrder( SRT_ASCENDING ),
    nColumnId( 0 ),
    bIsNumeric( true )
{
}

SwSortKey::SwSortKey(sal_uInt16 nId, const OUString& rSrtType, SwSortOrder eOrder) :
    sSortType( rSrtType ),
    eSortOrder( eOrder ),
    nColumnId( nId ),
    bIsNumeric( rSrtType.isEmpty() )
{
}

SwSortOptions::SwSortOptions()
    : eDirection( SRT_ROWS ),
    cDeli( 9 ),
    nLanguage( LANGUAGE_SYSTEM ),
    bTable( false ),
    bIgnoreCase( false )
{
}

SwSortOptions::SwSortOptions(const SwSortOptions& rOpt) :
    eDirection( rOpt.eDirection ),
    cDeli( rOpt.cDeli ),
    nLanguage( rOpt.nLanguage ),
    bTable( rOpt.bTable ),
    bIgnoreCase( rOpt.bIgnoreCase )
{
    for(auto const & pKey : rOpt.aKeys)
    {
        aKeys.push_back( std::make_unique<SwSortKey>(*pKey) );
    }
}

SwSortOptions::~SwSortOptions()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
