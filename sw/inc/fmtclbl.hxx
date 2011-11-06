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


#ifndef _FMTCLBL_HXX
#define _FMTCLBL_HXX


#include <svl/eitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include "swdllapi.h"

class SW_DLLPUBLIC SwFmtNoBalancedColumns : public SfxBoolItem
{
public:
    SwFmtNoBalancedColumns( sal_Bool bFlag = sal_False )
        : SfxBoolItem( RES_COLUMNBALANCE, bFlag ) {}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
};


inline const SwFmtNoBalancedColumns &SwAttrSet::GetBalancedColumns(sal_Bool bInP) const
    { return (const SwFmtNoBalancedColumns&)Get( RES_COLUMNBALANCE, bInP ); }

inline const SwFmtNoBalancedColumns &SwFmt::GetBalancedColumns(sal_Bool bInP) const
    { return aSet.GetBalancedColumns( bInP ); }

#endif

