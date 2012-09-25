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

#include <i18nutil/oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

oneToOneMapping::oneToOneMapping( OneToOneMappingTable_t *rpTable, const size_t rnBytes, const size_t rnUnitSize )
    : mpTable( rpTable ),
      mnSize( rnBytes / rnUnitSize )
{
}

oneToOneMapping::~oneToOneMapping()
{
}

sal_Unicode oneToOneMapping::find(const sal_Unicode nKey) const
{
    if( mpTable )
    {
        // binary search
        int bottom = 0;
        int top = mnSize - 1;
        int current;

        for (;;) {
            current = (top + bottom) / 2;
            if( nKey < mpTable[current].first )
                top = current - 1;
            else if( nKey > mpTable[current].first )
                bottom = current + 1;
            else
                return mpTable[current].second;

            if( bottom > top )
                return sal_Unicode( nKey );
        }
    }
    else
        return sal_Unicode( nKey );
}

oneToOneMappingWithFlag::oneToOneMappingWithFlag( UnicodePairWithFlag *rpTableWF, const size_t rnSize, const UnicodePairFlag rnFlag )
    : oneToOneMapping( NULL, rnSize, sizeof(UnicodePairWithFlag) ),
      mpTableWF ( rpTableWF ),
      mnFlag    ( rnFlag ),
      mbHasIndex( false )
{
}

oneToOneMappingWithFlag::~oneToOneMappingWithFlag()
{
    if( mbHasIndex )
        for( int i = 0; i < 256; i++ )
            if( mpIndex[i] )
                delete [] mpIndex[i];
}


void oneToOneMappingWithFlag::makeIndex()
{
    if( !mbHasIndex && mpTableWF )
    {
        int i, j, high, low, current = -1;

        for( i = 0; i < 256; i++ )
            mpIndex[i] = NULL;

        for( size_t k = 0; k < mnSize; k++ )
        {
            high = (mpTableWF[k].first >> 8) & 0xFF;
            low  = (mpTableWF[k].first)      & 0xFF;
            if( high != current )
            {
                current = high;
                mpIndex[high] = new UnicodePairWithFlag*[256];

                for( j = 0; j < 256; j++ )
                    mpIndex[high][j] = NULL;
            }
            mpIndex[high][low] = &mpTableWF[k];
        }

        mbHasIndex = true;
    }
}

sal_Unicode oneToOneMappingWithFlag::find( const sal_Unicode nKey ) const
{
    if( mpTableWF )
    {
        if( mbHasIndex )
        {
            // index search
            int high, low;
            high = (nKey >> 8) & 0xFF;
            low = nKey & 0xFF;
            if( mpIndex[high] != NULL &&
                mpIndex[high][low] != NULL &&
                mpIndex[high][low]->flag & mnFlag )
                return mpIndex[high][low]->second;
            else
                return sal_Unicode( nKey );
        }
        else
        {
            // binary search
            int bottom = 0;
            int top = mnSize - 1;
            int current;

            for (;;) {
                current = (top + bottom) / 2;
                if( nKey < mpTableWF[current].first )
                    top = current - 1;
                else if( nKey > mpTableWF[current].first )
                    bottom = current + 1;
                else
                {
                    if( mpTableWF[current].flag & mnFlag )
                        return mpTableWF[current].second;
                    else
                        return sal_Unicode( nKey );
                }

                if( bottom > top )
                    return sal_Unicode( nKey );
            }
        }
    }
    else
        return sal_Unicode( nKey );
}


} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
