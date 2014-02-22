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


#include <svl/whiter.hxx>
#include <svl/itemset.hxx>

DBG_NAME(SfxWhichIter)



SfxWhichIter::SfxWhichIter( const SfxItemSet& rSet, sal_uInt16 nFromWh, sal_uInt16 nToWh ):
    pRanges(rSet.GetRanges()),
    pStart(rSet.GetRanges()),
    nOfst(0), nFrom(nFromWh), nTo(nToWh)
{
    DBG_CTOR(SfxWhichIter, 0);
    if ( nFrom > 0 )
        FirstWhich();
}



SfxWhichIter::~SfxWhichIter()
{
    DBG_DTOR(SfxWhichIter, 0);
}



sal_uInt16 SfxWhichIter::NextWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    while ( 0 != *pRanges )
    {
        const sal_uInt16 nLastWhich = *pRanges + nOfst;
        ++nOfst;
        if (*(pRanges+1) == nLastWhich)
        {
            pRanges += 2;
            nOfst = 0;
        }
        sal_uInt16 nWhich = *pRanges + nOfst;
        if ( 0 == nWhich || ( nWhich >= nFrom && nWhich <= nTo ) )
            return nWhich;
    }
    return 0;
}



sal_uInt16 SfxWhichIter::FirstWhich()
{
    DBG_CHKTHIS(SfxWhichIter, 0);
    pRanges = pStart;
    nOfst = 0;
    if ( *pRanges >= nFrom && *pRanges <= nTo )
        return *pRanges;
    return NextWhich();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
