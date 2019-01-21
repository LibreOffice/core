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


// C and C++ includes
#include <stdlib.h>

// program-sensitive includes
#include <hash.hxx>

#include <rtl/character.hxx>

SvStringHashEntry * SvStringHashTable::Insert( const OString& rElement, sal_uInt32 * pInsertPos )
{
    auto it = maString2IntMap.find(rElement);
    if (it != maString2IntMap.end()) {
        *pInsertPos = it->second;
        return maInt2EntryMap[*pInsertPos].get();
    }
    maString2IntMap[rElement] = mnNextId;
    maInt2EntryMap[mnNextId] = std::make_unique<SvStringHashEntry>(rElement);
    *pInsertPos = mnNextId;
    mnNextId++;
    return maInt2EntryMap[*pInsertPos].get();
}

bool SvStringHashTable::Test( const OString& rElement, sal_uInt32 * pInsertPos )
{
    auto it = maString2IntMap.find(rElement);
    if (it != maString2IntMap.end()) {
        *pInsertPos = it->second;
        return true;
    }
    return false;
}

SvStringHashEntry * SvStringHashTable::Get( sal_uInt32 nInsertPos ) const
{
    auto it = maInt2EntryMap.find(nInsertPos);
    return it->second.get();
}

OString SvStringHashTable::GetNearString( const OString& rName ) const
{
    for( auto const & rPair : maInt2EntryMap )
    {
        SvStringHashEntry * pE = rPair.second.get();
        if( pE->GetName().equalsIgnoreAsciiCase( rName ) && pE->GetName() != rName  )
            return pE->GetName();
    }
    return OString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
