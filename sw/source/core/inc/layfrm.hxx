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


#ifndef SW_LAYFRM_HXX
#define SW_LAYFRM_HXX

#include "frame.hxx"

class SwCntntFrm;
class SwFlowFrm;
class SwFmtCol;
struct SwCrsrMoveState;
class SwFrmFmt;
class SwBorderAttrs;
class SwFmtFrmSize;
class SwCellFrm;

class SwLayoutFrm: public SwFrm
{
    //Der verkappte SwFrm
    friend class SwFlowFrm;
    friend class SwFrm;

        //Hebt die Lower waehrend eines Spaltenumbaus auf.
    friend SwFrm* SaveCntnt( SwLayoutFrm *, SwFrm * );
    friend void   RestoreCntnt( SwFrm *, SwLayoutFrm *, SwFrm *pSibling, bool bGrow );

#if OSL_DEBUG_LEVEL > 1
    //entfernt leere SwSectionFrms aus einer Kette
    friend SwFrm* SwClearDummies( SwFrm* pFrm );
#endif

    void CopySubtree( const SwLayoutFrm *pDest );
protected:
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual void MakeAll();

    SwFrm           *pLower;

    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );

    long CalcRel( const SwFmtFrmSize &rSz, sal_Bool bWidth ) const;

public:
    // --> OD 2004-06-29 #i28701#
    void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const;
    void RefreshLaySubsidiary( const SwPageFrm*, const SwRect& ) const;
    void RefreshExtraData( const SwRect & ) const;

        //Proportionale Groessenanpassung der untergeordneten.
    void ChgLowersProp( const Size& rOldSize );

    void AdjustColumns( const SwFmtCol *pCol, sal_Bool bAdjustAttributes );

    void ChgColumns( const SwFmtCol &rOld, const SwFmtCol &rNew,
        const sal_Bool bChgFtn = sal_False );


        //Painted die Column-Trennlinien fuer die innenliegenden Columns.
    void PaintColLines( const SwRect &, const SwFmtCol &,
                        const SwPageFrm * ) const;

    virtual bool    FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    virtual sal_Bool  GetCrsrOfst( SwPosition *, Point&,
                               SwCrsrMoveState* = 0 ) const;

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

        //sucht den dichtesten Cntnt zum SPoint, wird bei Seiten, Flys und Cells
        //benutzt wenn GetCrsrOfst versagt hat.
    const SwCntntFrm* GetCntntPos( Point &rPoint, const sal_Bool bDontLeave,
                                   const sal_Bool bBodyOnly = sal_False,
                                   const sal_Bool bCalc = sal_False,
                                   const SwCrsrMoveState *pCMS = 0,
                                   const sal_Bool bDefaultExpand = sal_True ) const;

    SwLayoutFrm( SwFrmFmt*, SwFrm* );
    ~SwLayoutFrm();

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    const SwFrm *Lower() const { return pLower; }
          SwFrm *Lower()       { return pLower; }
    const SwCntntFrm *ContainsCntnt() const;
    inline SwCntntFrm *ContainsCntnt();
    const SwCellFrm *FirstCell() const;
    inline SwCellFrm *FirstCell();
    // --> OD 2006-02-01 #130797#
    // Method <ContainsAny()> doesn't investigate content of footnotes by default.
    // But under certain circumstances this investigation is intended.
    // Thus, introduce new optional parameter <_bInvestigateFtnForSections>.
    // It's default is <false>, still indicating that content of footnotes isn't
    // investigated for sections.
    const SwFrm *ContainsAny( const bool _bInvestigateFtnForSections = false ) const;
    inline SwFrm *ContainsAny( const bool _bInvestigateFtnForSections = false );
    // <--
    sal_Bool IsAnLower( const SwFrm * ) const;

    virtual const SwFrmFmt *GetFmt() const;
    virtual       SwFrmFmt *GetFmt();
    void        SetFrmFmt( SwFrmFmt* );

    //Verschieben der Ftns aller Lower - ab dem StartCntnt.
    //sal_True wenn mindestens eine Ftn verschoben wurde.
    //Ruft das Update der Seitennummer wenn bFtnNums gesetzt ist.
    sal_Bool MoveLowerFtns( SwCntntFrm *pStart, SwFtnBossFrm *pOldBoss,
                        SwFtnBossFrm *pNewBoss, const sal_Bool bFtnNums );

    // --> OD 2004-07-01 #i28701# - change purpose of method and its name
    // --> OD 2005-03-11 #i44016# - add parameter <_bUnlockPosOfObjs> to
    // force an unlockposition call for the lower objects.
    void NotifyLowerObjs( const bool _bUnlockPosOfObjs = false );
    // <--

    //Invalidiert diejenigen innenliegenden Frames, deren Breite und/oder
    //Hoehe Prozentual berechnet werden. Auch Rahmen, die an this oder an
    //innenliegenden verankert sind werden ggf. invalidiert.
    void InvaPercentLowers( SwTwips nDiff = 0 );

    //Gerufen von Format fuer Rahmen und Bereichen mit Spalten.
    void FormatWidthCols( const SwBorderAttrs &, const SwTwips nBorder,
                          const SwTwips nMinHeight );

    // InnerHeight returns the height of the content and may be bigger or
    // less than the PrtArea-Height of the layoutframe himself
    SwTwips InnerHeight() const;

    /** method to check relative position of layout frame to
        a given layout frame.

        OD 08.11.2002 - refactoring of pseudo-local method <lcl_Apres(..)> in
        <txtftn.cxx> for #104840#.

        @param _aCheckRefLayFrm
        constant reference of an instance of class <SwLayoutFrm> which
        is used as the reference for the relative position check.

        @author OD

        @return true, if <this> is positioned before the layout frame <p>
    */
    bool IsBefore( const SwLayoutFrm* _pCheckRefLayFrm ) const;

    const SwFrm* GetLastLower() const;
    inline SwFrm* GetLastLower();
};

//Um doppelte Implementierung zu sparen wird hier ein bischen gecasted
inline SwCntntFrm* SwLayoutFrm::ContainsCntnt()
{
    return (SwCntntFrm*)(((const SwLayoutFrm*)this)->ContainsCntnt());
}

inline SwCellFrm* SwLayoutFrm::FirstCell()
{
    return (SwCellFrm*)(((const SwLayoutFrm*)this)->FirstCell());
}

// --> OD 2006-02-01 #130797#
inline SwFrm* SwLayoutFrm::ContainsAny( const bool _bInvestigateFtnForSections )
{
    return (SwFrm*)(((const SwLayoutFrm*)this)->ContainsAny( _bInvestigateFtnForSections ));
}
// <--

// Diese SwFrm-inlines sind hier, damit frame.hxx nicht layfrm.hxx includen muss
inline sal_Bool SwFrm::IsColBodyFrm() const
{
    return nType == FRMC_BODY && GetUpper()->IsColumnFrm();
}

inline sal_Bool SwFrm::IsPageBodyFrm() const
{
    return nType == FRMC_BODY && GetUpper()->IsPageFrm();
}

inline SwFrm* SwLayoutFrm::GetLastLower()
{
    return const_cast<SwFrm*>(static_cast<const SwLayoutFrm*>(this)->GetLastLower());
}

#endif  // SW_LAYFRM_HXX
