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


#ifndef _ROWFRM_HXX
#define _ROWFRM_HXX

#include <tools/mempool.hxx>

#include "layfrm.hxx"

class SwTableLine;
class SwBorderAttrs;

class SwRowFrm: public SwLayoutFrm
{
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
        //Aendern nur die Framesize, nicht die PrtArea-SSize
    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );

    const SwTableLine *pTabLine;
    SwRowFrm* pFollowRow;
    // --> collapsing borders FME 2005-05-27 #i29550#
    sal_uInt16 mnTopMarginForLowers;
    sal_uInt16 mnBottomMarginForLowers;
    sal_uInt16 mnBottomLineSize;
    // <-- collapsing
    bool bIsFollowFlowRow;
    bool bIsRepeatedHeadline;
    bool mbIsRowSpanLine;

protected:
    virtual void MakeAll();
    virtual void  Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    SwRowFrm( const SwTableLine &, SwFrm*, bool bInsertContent = true );
    ~SwRowFrm();

    virtual void Cut();

    //Zum Anmelden der Flys nachdem eine Zeile erzeugt _und_ eingefuegt wurde.
    //Muss vom Erzeuger gerufen werden, denn erst nach dem Konstruieren wird
    //Das Teil gepastet; mithin ist auch erst dann die Seite zum Anmelden der
    //Flys erreichbar.
    void RegistFlys( SwPageFrm *pPage = 0 );

    const SwTableLine *GetTabLine() const { return pTabLine; }

    //Passt die Zellen auf die aktuelle Hoehe an, invalidiert die Zellen
    //wenn die Direction nicht der Hoehe entspricht.
    void AdjustCells( const SwTwips nHeight, const sal_Bool bHeight );

    //
    //
    SwRowFrm* GetFollowRow() const { return pFollowRow; }
    void SetFollowRow( SwRowFrm* pNew ) { pFollowRow = pNew; }

    // --> collapsing borders FME 2005-05-27 #i29550#
    sal_uInt16 GetTopMarginForLowers() const { return mnTopMarginForLowers; }
    void   SetTopMarginForLowers( sal_uInt16 nNew ) { mnTopMarginForLowers = nNew; }
    sal_uInt16 GetBottomMarginForLowers() const { return mnBottomMarginForLowers; }
    void   SetBottomMarginForLowers( sal_uInt16 nNew ) { mnBottomMarginForLowers = nNew; }
    sal_uInt16 GetBottomLineSize() const { return mnBottomLineSize; }
    void   SetBottomLineSize( sal_uInt16 nNew ) { mnBottomLineSize = nNew; }
    // <-- collapsing

    bool IsRepeatedHeadline() const { return bIsRepeatedHeadline; }
    void SetRepeatedHeadline( bool bNew ) { bIsRepeatedHeadline = bNew; }

    // --> split table rows
    bool IsRowSplitAllowed() const;
    bool IsFollowFlowRow() const { return bIsFollowFlowRow; }
    void SetFollowFlowRow( bool bNew ) { bIsFollowFlowRow = bNew; }
    // <-- split table rows

    // --> FME 2006-02-10 #131283# Table row keep feature
    bool ShouldRowKeepWithNext() const;
    // <--

    // --> FME 2006-10-30 #i4032# NEW TABLES
    bool IsRowSpanLine() const { return mbIsRowSpanLine; }
    void SetRowSpanLine( bool bNew ) { mbIsRowSpanLine = bNew; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwRowFrm)
};

#endif
