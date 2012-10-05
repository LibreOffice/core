/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SW_TABFRM_HXX
#define SW_TABFRM_HXX

#include <tools/mempool.hxx>
#include "layfrm.hxx"
#include "flowfrm.hxx"

class SwTable;
class SwBorderAttrs;
class SwAttrSetChg;

class SwTabFrm: public SwLayoutFrm, public SwFlowFrm
{
    // OD 14.03.2003 #i11760# - adjustment, because of method signature change
    //darf mit den Flags spielen.
    friend void CalcCntnt( SwLayoutFrm *pLay, bool bNoColl, bool bNoCalcFollow );

    //Fuert Spezialbehandlung fuer _Get[Next|Prev]Leaf() durch.
    using SwFrm::GetLeaf;
    SwLayoutFrm *GetLeaf( MakePageType eMakePage, sal_Bool bFwd );

    SwTable* pTable;

    sal_Bool bComplete          :1; //Eintrage als Repaint ohne das CompletePaint
                                //der Basisklasse gesetzt werden muss. Damit
                                //sollen unertraegliche Tabellen-Repaints
                                //vermieden werden.
    sal_Bool bCalcLowers        :1; //Im MakeAll auf jedenfall auch fuer Stabilitaet
                                //des Inhaltes sorgen.
    sal_Bool bLowersFormatted   :1;//Kommunikation zwischen MakeAll und Layact
    sal_Bool bLockBackMove      :1; //BackMove-Test hat der Master erledigt.
    sal_Bool bResizeHTMLTable   :1; //Resize des HTMLTableLayout rufen im MakeAll
                                //Zur Optimierung, damit dies nicht im
                                //CntntFrm::Grow gerufen werden muss, denn dann
                                //wird es ggf. fuer jede Zelle gerufen #47483#
    sal_Bool bONECalcLowers     :1; //Primaer fuer die StarONE-SS. Beim MakeAll werden
                                //die Cntnts auf jedenfall per Calc() formatiert.
                                //es finden keine zusaetzlichen Invalidierungen
                                //statt und dieser Weg kann auch kaum garantien
                                //geben.

    sal_Bool bHasFollowFlowLine :1; // Means that the first line in the follow
                                // is indented to contain content from a broken
                                // cell
    sal_Bool bIsRebuildLastLine :1; // Means that currently the last line of the
                                // TabFrame is rebuilded. In this case we
                                // do not want any notification to the master
                                // table
    sal_Bool bRestrictTableGrowth :1;       // Usually, the table may grow infinite,
                                        // because the table can be split in
                                        // SwTabFrm::MakeAll. In MakeAll, this
                                        // flag is set to indicate that the table
                                        // may only grow inside its upper. This
                                        // is necessary, in order to let the text
                                        // flow into the FollowFlowLine
    sal_Bool bRemoveFollowFlowLinePending :1;
    // #i26945#
    sal_Bool bConsiderObjsForMinCellHeight :1; // Usually, the floating screen objects
                                           // are considered on the calculation
                                           // for the minimal cell height.
                                           // For splitting table rows algorithm
                                           // it's needed not to consider floating
                                           // screen object for the preparation
                                           // of the re-calculation of the
                                           // last table row.
    // #i26945#
    sal_Bool bObjsDoesFit :1; // For splitting table rows algorithm, this boolean
                          // indicates, if the floating screen objects fits

    //Split() spaltet den Frm an der angegebenen Stelle, es wird ein
    //Follow erzeugt und aufgebaut und direkt hinter this gepastet.
    //Join() Holt sich den Inhalt aus dem Follow und vernichtet diesen.
    bool Split( const SwTwips nCutPos, bool bTryToSplit, bool bTableRowKeep );
    bool Join();

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    virtual sal_Bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, sal_Bool bHead, sal_Bool &rReformat );

protected:
    virtual void MakeAll();
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );
        //Aendert nur die Framesize, nicht die PrtArea-SSize
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );

public:
    SwTabFrm( SwTable &, SwFrm* );  //Immer nach dem erzeugen _und_ pasten das
                            //Regist Flys rufen!
    SwTabFrm( SwTabFrm & ); //_Nur_ zum erzeugen von Follows
    ~SwTabFrm();

    void JoinAndDelFollows();   //Fuer DelFrms des TableNodes!

    //Ruft das RegistFlys der Zeilen.
    void RegistFlys();

    inline const SwTabFrm *GetFollow() const;
    inline       SwTabFrm *GetFollow();
    SwTabFrm* FindMaster( bool bFirstMaster = false ) const;

    virtual sal_Bool GetInfo( SfxPoolItem &rHnt ) const;
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual void  CheckDirection( sal_Bool bVert );

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, sal_Bool bNotify = sal_True );

                 SwCntntFrm *FindLastCntnt();
    inline const SwCntntFrm *FindLastCntnt() const;

    const SwTable *GetTable() const { return pTable; }
          SwTable *GetTable()       { return pTable; }

    sal_Bool IsComplete()  { return bComplete; }
    void SetComplete() { bComplete = sal_True; }
    void ResetComplete() { bComplete = sal_False; }

    sal_Bool IsLowersFormatted() const      { return bLowersFormatted; }
    void SetLowersFormatted( sal_Bool b )   { bLowersFormatted = b;    }

    void SetCalcLowers()        { bCalcLowers = sal_True;      } //Sparsam einsetzen!
    void SetResizeHTMLTable()   { bResizeHTMLTable = sal_True; } //dito
    void SetONECalcLowers()     { bONECalcLowers = sal_True;   }

    //
    // Start: New stuff for breaking table rows
    //
    sal_Bool HasFollowFlowLine() const { return bHasFollowFlowLine; }
    void SetFollowFlowLine( sal_Bool bNew ) { bHasFollowFlowLine = bNew; }

    sal_Bool IsRebuildLastLine() const { return bIsRebuildLastLine; }
    void SetRebuildLastLine( sal_Bool bNew ) { bIsRebuildLastLine = bNew; }

    sal_Bool IsRestrictTableGrowth() const { return bRestrictTableGrowth; }
    void SetRestrictTableGrowth( sal_Bool bNew ) { bRestrictTableGrowth = bNew; }

    sal_Bool IsRemoveFollowFlowLinePending() const { return bRemoveFollowFlowLinePending; }
    void SetRemoveFollowFlowLinePending( sal_Bool bNew ) { bRemoveFollowFlowLinePending = bNew; }

    // #i26945#
    sal_Bool IsConsiderObjsForMinCellHeight() const
    {
        return bConsiderObjsForMinCellHeight;
    }
    void SetConsiderObjsForMinCellHeight( sal_Bool _bNewConsiderObjsForMinCellHeight )
    {
        bConsiderObjsForMinCellHeight = _bNewConsiderObjsForMinCellHeight;
    }
    // #i26945#
    sal_Bool DoesObjsFit() const
    {
        return bObjsDoesFit;
    }
    void SetDoesObjsFit( sal_Bool _bNewObjsDoesFit )
    {
        bObjsDoesFit = _bNewObjsDoesFit;
    }

    bool RemoveFollowFlowLine();
    //
    // End: New stuff for breaking table rows
    //

    sal_Bool CalcFlyOffsets( SwTwips& rUpper, long& rLeftOffset,
                         long& rRightOffset ) const;

    SwTwips CalcHeightOfFirstContentLine() const;

    bool IsInHeadline( const SwFrm& rFrm ) const;
    SwRowFrm* GetFirstNonHeadlineRow() const;

    bool IsLayoutSplitAllowed() const;

    // #i29550#
    bool IsCollapsingBorders() const;

    // used for collapsing border lines:
    sal_uInt16 GetBottomLineSize() const;
    // <-- collapsing

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer);

    DECL_FIXEDMEMPOOL_NEWDEL(SwTabFrm)
};

inline const SwCntntFrm *SwTabFrm::FindLastCntnt() const
{
    return ((SwTabFrm*)this)->FindLastCntnt();
}

inline const SwTabFrm *SwTabFrm::GetFollow() const
{
    return (const SwTabFrm*)SwFlowFrm::GetFollow();
}
inline SwTabFrm *SwTabFrm::GetFollow()
{
    return (SwTabFrm*)SwFlowFrm::GetFollow();
}

#endif  // SW_TABFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
