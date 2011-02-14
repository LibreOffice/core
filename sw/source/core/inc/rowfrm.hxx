/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

public:
    SwRowFrm( const SwTableLine &, bool bInsertContent = true );
    ~SwRowFrm();

    virtual void Cut();
    virtual void  Modify( SfxPoolItem*, SfxPoolItem* );

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
