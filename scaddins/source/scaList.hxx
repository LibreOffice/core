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

#ifndef INCLUDED_SCADDINS_SOURCE_SCALIST_HXX
#define INCLUDED_SCADDINS_SOURCE_SCALIST_HXX

#include <vector>


class ScaList
{
    private:
        std::vector<void*>                   pData;
        std::vector<void*>::const_iterator   nCurr;

    public:
                                    ScaList() = default;
        virtual                     ~ScaList() = default;

        inline sal_uInt32           Count() const       { return pData.size(); }

        inline const void*             GetObject( sal_uInt32 nIndex ) const
                                            { return (nIndex < pData.size()) ? pData[ nIndex ] : nullptr; }

        inline void*                   First();
        inline void*                   Next();

        inline void                   Append( void* pNew ){pData.push_back(pNew);}
    };

void* ScaList::First(){
    if (pData.empty()) return nullptr;
    nCurr = pData.begin();
    return *nCurr;
}

void* ScaList::Next(){
    if (*(nCurr) == *(pData.rbegin())) return nullptr;
    return *(++nCurr);
}

#endif // INCLUDED_SCADDINS_SOURCE_SCALIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
