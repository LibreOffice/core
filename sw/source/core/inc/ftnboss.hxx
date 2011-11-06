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


#ifndef _FTNBOSS_HXX
#define _FTNBOSS_HXX

#include "layfrm.hxx"

class SwFtnBossFrm;
class SwFtnContFrm;
class SwFtnFrm;
class SwTxtFtn;


//Setzen des maximalen Fussnotenbereiches. Restaurieren des alten Wertes im DTor.
//Implementierung im ftnfrm.cxx
class SwSaveFtnHeight
{
    SwFtnBossFrm *pBoss;
    const SwTwips nOldHeight;
    SwTwips nNewHeight;
public:
    SwSaveFtnHeight( SwFtnBossFrm *pBs, const SwTwips nDeadLine );
    ~SwSaveFtnHeight();
};

#define NA_ONLY_ADJUST 0
#define NA_GROW_SHRINK 1
#define NA_GROW_ADJUST 2
#define NA_ADJUST_GROW 3

class SwFtnBossFrm: public SwLayoutFrm
{
    //Fuer die privaten Fussnotenoperationen
    friend class SwFrm;
    friend class SwSaveFtnHeight;
    friend class SwPageFrm; // fuer das Setzen der MaxFtnHeight

    //Maximale Hoehe des Fussnotencontainers fuer diese Seite.
    SwTwips nMaxFtnHeight;

    SwFtnContFrm *MakeFtnCont();
    SwFtnFrm     *FindFirstFtn();
    sal_uInt8 _NeighbourhoodAdjustment( const SwFrm* pFrm ) const;
protected:

    void          InsertFtn( SwFtnFrm * );
    static void   ResetFtn( const SwFtnFrm *pAssumed );
public:
    inline SwFtnBossFrm( SwFrmFmt* pFmt, SwFrm* pSib ) : SwLayoutFrm( pFmt, pSib ) {}

                 SwLayoutFrm *FindBodyCont();
    inline const SwLayoutFrm *FindBodyCont() const;
    inline void SetMaxFtnHeight( const SwTwips nNewMax ) { nMaxFtnHeight = nNewMax; }

    //Fussnotenschnittstelle
    void AppendFtn( SwCntntFrm *, SwTxtFtn * );
    void RemoveFtn( const SwCntntFrm *, const SwTxtFtn *, sal_Bool bPrep = sal_True );
    static       SwFtnFrm     *FindFtn( const SwCntntFrm *, const SwTxtFtn * );
                 SwFtnContFrm *FindFtnCont();
    inline const SwFtnContFrm *FindFtnCont() const;
           const SwFtnFrm     *FindFirstFtn( SwCntntFrm* ) const;
                 SwFtnContFrm *FindNearestFtnCont( sal_Bool bDontLeave = sal_False );

    void ChangeFtnRef( const SwCntntFrm *pOld, const SwTxtFtn *,
                       SwCntntFrm *pNew );
    void RearrangeFtns( const SwTwips nDeadLine, const sal_Bool bLock = sal_False,
                        const SwTxtFtn *pAttr = 0 );

    //SS damit der Textformatierer Temporaer die Fussnotenhoehe begrenzen
    //kann. DeadLine in Dokumentkoordinaten.
    void    SetFtnDeadLine( const SwTwips nDeadLine );
    SwTwips GetMaxFtnHeight() const { return nMaxFtnHeight; }

    //Liefert den Wert, der noch uebrig ist, bis der Body seine minimale
    //Hoehe erreicht hat.
    SwTwips GetVarSpace() const;

        //Layoutseitig benoetigte Methoden
    /// OD 03.04.2003 #108446# - add parameters <_bCollectOnlyPreviousFtns> and
    /// <_pRefFtnBossFrm> in order to control, if only footnotes, which are positioned
    /// before the given reference footnote boss frame have to be collected.
    /// Note: if parameter <_bCollectOnlyPreviousFtns> is true, then parameter
    /// <_pRefFtnBossFrm> have to be referenced to an object.
    static void _CollectFtns( const SwCntntFrm*   _pRef,
                              SwFtnFrm*           _pFtn,
                              SvPtrarr&           _rFtnArr,
                              const sal_Bool      _bCollectOnlyPreviousFtns = sal_False,
                              const SwFtnBossFrm* _pRefFtnBossFrm = NULL);
    /// OD 03.04.2003 #108446# - add parameter <_bCollectOnlyPreviousFtns> in
    /// order to control, if only footnotes, which are positioned before the
    /// footnote boss frame <this> have to be collected.
    void    CollectFtns( const SwCntntFrm* _pRef,
                         SwFtnBossFrm*     _pOld,
                         SvPtrarr&         _rFtnArr,
                         const sal_Bool    _bCollectOnlyPreviousFtns = sal_False );
    void    _MoveFtns( SvPtrarr &rFtnArr, sal_Bool bCalc = sal_False );
    void    MoveFtns( const SwCntntFrm *pSrc, SwCntntFrm *pDest,
                      SwTxtFtn *pAttr );
    // Sollte AdjustNeighbourhood gerufen werden (oder Grow/Shrink)?
    sal_uInt8 NeighbourhoodAdjustment( const SwFrm* pFrm ) const
        { return IsPageFrm() ? NA_ONLY_ADJUST : _NeighbourhoodAdjustment( pFrm ); }
};

inline const SwLayoutFrm *SwFtnBossFrm::FindBodyCont() const
{
    return ((SwFtnBossFrm*)this)->FindBodyCont();
}
inline const SwFtnContFrm *SwFtnBossFrm::FindFtnCont() const
{
    return ((SwFtnBossFrm*)this)->FindFtnCont();
}

#endif  //_FTNBOSS_HXX
