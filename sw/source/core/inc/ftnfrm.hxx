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


#ifndef _FTNFRM_HXX
#define _FTNFRM_HXX

#include "layfrm.hxx"

class SwCntntFrm;
class SwTxtFtn;
class SwBorderAttrs;
class SwFtnFrm;

//Fuer Fussnoten gibt es einen Speziellen Bereich auf der Seite. Dieser
//Bereich ist ein SwFtnContFrm.
//Jede Fussnote ist durch einen SwFtnFrm abgegrenzt, dieser nimmt die
//Fussnotenabsaetze auf. SwFtnFrm koennen aufgespalten werden, sie gehen
//dann auf einer anderen Seite weiter.

class SwFtnContFrm: public SwLayoutFrm
{
public:
    SwFtnContFrm( SwFrmFmt*, SwFrm* );

    const SwFtnFrm* FindFootNote() const;

    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual void    Format( const SwBorderAttrs *pAttrs = 0 );
    virtual void    PaintBorder( const SwRect &, const SwPageFrm *pPage,
                                 const SwBorderAttrs & ) const;
            void    PaintLine( const SwRect &, const SwPageFrm * ) const;
};

class SwFtnFrm: public SwLayoutFrm
{
    //Zeiger auf den FtnFrm in dem die Fussnote weitergefuehrt wird:
    // 0     wenn kein Follow vorhanden,
    // this  beim letzten
    // der Follow sonst.
    SwFtnFrm     *pFollow;
    SwFtnFrm     *pMaster;      //Der FtnFrm dessen Follow ich bin.
    SwCntntFrm   *pRef;         //In diesem CntntFrm steht die Fussnotenref.
    SwTxtFtn     *pAttr;        //Fussnotenattribut (zum wiedererkennen)

    sal_Bool bBackMoveLocked : 1;   //Absaetze in dieser Fussnote duerfen derzeit
                                //nicht rueckwaerts fliessen.
    // --> OD 2005-05-18 #i49383# - control unlock of position of lower anchored objects.
    bool mbUnlockPosOfLowerObjs : 1;
    // <--
#ifdef DBG_UTIL
protected:
    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
#endif


public:
    SwFtnFrm( SwFrmFmt*, SwFrm*, SwCntntFrm*, SwTxtFtn* );

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

    sal_Bool operator<( const SwTxtFtn* pTxtFtn ) const;

#ifndef DBG_UTIL
    const SwCntntFrm *GetRef() const    { return pRef; }
         SwCntntFrm  *GetRef()          { return pRef; }
#else
    //JP 15.10.2001: in a non pro version test if the attribute has the same
    //              meaning which his reference is
    const SwCntntFrm *GetRef() const;
         SwCntntFrm  *GetRef();
#endif
    const SwCntntFrm *GetRefFromAttr()  const;
          SwCntntFrm *GetRefFromAttr();

    const SwFtnFrm *GetFollow() const   { return pFollow; }
          SwFtnFrm *GetFollow()         { return pFollow; }

    const SwFtnFrm *GetMaster() const   { return pMaster; }
          SwFtnFrm *GetMaster()         { return pMaster; }

    const SwTxtFtn   *GetAttr() const   { return pAttr; }
          SwTxtFtn   *GetAttr()         { return pAttr; }

    void SetFollow( SwFtnFrm *pNew ) { pFollow = pNew; }
    void SetMaster( SwFtnFrm *pNew ) { pMaster = pNew; }
    void SetRef   ( SwCntntFrm *pNew ) { pRef = pNew; }

    void InvalidateNxtFtnCnts( SwPageFrm* pPage );

    void LockBackMove()     { bBackMoveLocked = sal_True; }
    void UnlockBackMove()   { bBackMoveLocked = sal_False;}
    sal_Bool IsBackMoveLocked() { return bBackMoveLocked; }

    // Verhindert, dass der letzte Inhalt den SwFtnFrm mitloescht (Cut())
    inline void ColLock()       { bColLocked = sal_True; }
    inline void ColUnlock()     { bColLocked = sal_False; }

    // --> OD 2005-05-18 #i49383#
    inline void UnlockPosOfLowerObjs()
    {
        mbUnlockPosOfLowerObjs = true;
    }
    inline void KeepLockPosOfLowerObjs()
    {
        mbUnlockPosOfLowerObjs = false;
    }
    inline bool IsUnlockPosOfLowerObjs()
    {
        return mbUnlockPosOfLowerObjs;
    }
    // <--
    /** search for last content in the current footnote frame

        OD 2005-12-02 #i27138#

        @author OD

        @return SwCntntFrm*
        pointer to found last content frame. NULL, if none is found.
    */
    SwCntntFrm* FindLastCntnt();
};

#endif  //_FTNFRM_HXX
