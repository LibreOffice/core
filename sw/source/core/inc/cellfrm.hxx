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


#ifndef SW_CELLFRM_HXX
#define SW_CELLFRM_HXX

#include <tools/mempool.hxx>

#include "layfrm.hxx"

class SwTableBox;
struct SwCrsrMoveState;
class SwBorderAttrs;

class SwCellFrm: public SwLayoutFrm
{
    const SwTableBox* pTabBox;

protected:
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    SwCellFrm( const SwTableBox &, SwFrm*, bool bInsertContent = true );
    ~SwCellFrm();

    virtual sal_Bool GetCrsrOfst( SwPosition *, Point&, SwCrsrMoveState* = 0 ) const;
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual void CheckDirection( sal_Bool bVert );

    // --> OD 2010-02-17 #i103961#
    virtual void Cut();
    // <--

    const SwTableBox *GetTabBox() const { return pTabBox; }

    // used for breaking table rows:
    SwCellFrm* GetFollowCell() const;
    SwCellFrm* GetPreviousCell() const;

    // used for rowspan stuff:
    const SwCellFrm& FindStartEndOfRowSpanCell( bool bStart, bool bCurrentTab ) const;
    long GetLayoutRowSpan() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwCellFrm)
};

#endif
