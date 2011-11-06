/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <tools/debug.hxx>
#include <i18npool/lang.h>
#include <sortopt.hxx>


SV_IMPL_PTRARR(SwSortKeys, SwSortKey*)

/*--------------------------------------------------------------------
    Beschreibung:   Sortier-Schluessel
 --------------------------------------------------------------------*/

SwSortKey::SwSortKey() :
    eSortOrder( SRT_ASCENDING ),
    nColumnId( 0 ),
    bIsNumeric( sal_True )
{
}

SwSortKey::SwSortKey(sal_uInt16 nId, const String& rSrtType, SwSortOrder eOrder) :
    sSortType( rSrtType ),
    eSortOrder( eOrder ),
    nColumnId( nId ),
    bIsNumeric( 0 == rSrtType.Len() )
{
}


SwSortKey::SwSortKey(const SwSortKey& rOld) :
    sSortType( rOld.sSortType ),
    eSortOrder( rOld.eSortOrder ),
    nColumnId( rOld.nColumnId ),
    bIsNumeric( rOld.bIsNumeric )
{
}

/*--------------------------------------------------------------------
    Beschreibung: Sortieroptionen fuers Sortieren
 --------------------------------------------------------------------*/


SwSortOptions::SwSortOptions()
    : eDirection( SRT_ROWS ),
    cDeli( 9 ),
    nLanguage( LANGUAGE_SYSTEM ),
    bTable( sal_False ),
    bIgnoreCase( sal_False )
{
}


SwSortOptions::SwSortOptions(const SwSortOptions& rOpt) :
    eDirection( rOpt.eDirection ),
    cDeli( rOpt.cDeli ),
    nLanguage( rOpt.nLanguage ),
    bTable( rOpt.bTable ),
    bIgnoreCase( rOpt.bIgnoreCase )
{
    for( sal_uInt16 i=0; i < rOpt.aKeys.Count(); ++i )
    {
        SwSortKey* pNew = new SwSortKey(*rOpt.aKeys[i]);
        aKeys.C40_INSERT( SwSortKey, pNew, aKeys.Count());
    }
}


SwSortOptions::~SwSortOptions()
{
    aKeys.DeleteAndDestroy(0, aKeys.Count());
}



