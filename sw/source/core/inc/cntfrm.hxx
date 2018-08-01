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


#ifndef _CNTFRM_HXX
#define _CNTFRM_HXX
#include "frame.hxx"
#include "flowfrm.hxx"
#include "cshtyp.hxx"

class SwLayoutFrm;
class SwCntntNode;
class SwBorderAttrs;
class SwAttrSetChg;
class SwTxtFrm;

//Implementiert in cntfrm.cxx, wird von cntfrm.cxx und crsrsh.cxx angezogen
extern sal_Bool GetFrmInPage( const SwCntntFrm*, SwWhichPage, SwPosPage, SwPaM* );

class SwCntntFrm: public SwFrm, public SwFlowFrm
{
    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );    //ruft MakePrtArea

    // --> OD 2007-11-26 #b6614158#
    // parameter <bObjsInNewUpper>, indicating that objects are existing in
    // remaining area of new upper
    sal_Bool _WouldFit( SwTwips nSpace,
                    SwLayoutFrm *pNewUpper,
                    sal_Bool bTstMove,
                    const bool bObjsInNewUpper );
    // <--
    virtual void MakeAll();

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    virtual sal_Bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, sal_Bool, sal_Bool& );

    const SwCntntFrm* ImplGetNextCntntFrm( bool bFwd ) const;

protected:

    sal_Bool MakePrtArea( const SwBorderAttrs & );

    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );
    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );

    SwCntntFrm( SwCntntNode * const, SwFrm* );

public:
    virtual ~SwCntntFrm();
    TYPEINFO(); //bereits in Basisklassen drin

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

    inline       SwCntntNode *GetNode();
    inline const SwCntntNode *GetNode() const;
                 sal_uInt16       GetSectionLevel();

    inline const SwCntntFrm *GetFollow() const;
    inline       SwCntntFrm *GetFollow();
    SwTxtFrm* FindMaster() const;

        //Layoutabhaengiges Cursortravelling
    virtual sal_Bool    LeftMargin( SwPaM * ) const;
    virtual sal_Bool    RightMargin( SwPaM *, sal_Bool bAPI = sal_False ) const;
    virtual sal_Bool    UnitUp( SwPaM *, const SwTwips nOffset = 0,
                            sal_Bool bSetInReadOnly = sal_False ) const;
    virtual sal_Bool    UnitDown( SwPaM *, const SwTwips nOffset = 0,
                            sal_Bool bSetInReadOnly = sal_False ) const;
    inline  sal_Bool    StartNextPage( SwPaM * ) const;
    inline  sal_Bool    StartPrevPage( SwPaM * ) const;
    inline  sal_Bool    StartCurrPage( SwPaM * ) const;
    inline  sal_Bool    EndCurrPage( SwPaM * ) const;
    inline  sal_Bool    EndNextPage( SwPaM * ) const;
    inline  sal_Bool    EndPrevPage( SwPaM * ) const;

    // nMaxHeight is the required height
    // bSplit indicates, that the paragraph has to be split
    // bTst indicates, that we are currently doing a test formatting
    virtual sal_Bool WouldFit( SwTwips &nMaxHeight, sal_Bool &bSplit, sal_Bool bTst );

    sal_Bool MoveFtnCntFwd( sal_Bool, SwFtnBossFrm* );//von MoveFwd gerufen bei Ftn-Inhalt

    inline  SwCntntFrm* GetNextCntntFrm() const;
    inline  SwCntntFrm* GetNextCntntFrm( bool ) const;
    inline  SwCntntFrm* GetPrevCntntFrm() const;
    static bool CalcLowers( SwLayoutFrm* pLay, const SwLayoutFrm* pDontLeave, long nBottom, bool bSkipRowSpanCells );
    void RegisterToNode( SwCntntNode& );
    static void DelFrms( const SwCntntNode& );
};

inline SwCntntFrm* SwCntntFrm::GetNextCntntFrm() const
{
    if ( GetNext() && GetNext()->IsCntntFrm() )
        return (SwCntntFrm*)GetNext();
    else
        return (SwCntntFrm*)ImplGetNextCntntFrm( true );
}

inline SwCntntFrm* SwCntntFrm::GetNextCntntFrm( bool bPrune ) const
{
    if ( bPrune && !GetPrev() )
        return NULL;
    else if ( bPrune || !(GetNext() && GetNext()->IsCntntFrm()))
        return (SwCntntFrm*)ImplGetNextCntntFrm( true );
    else
        return (SwCntntFrm*)GetNext();
}

inline SwCntntFrm* SwCntntFrm::GetPrevCntntFrm() const
{
    if ( GetPrev() && GetPrev()->IsCntntFrm() )
        return (SwCntntFrm*)GetPrev();
    else
        return (SwCntntFrm*)ImplGetNextCntntFrm( false );
}

inline SwCntntNode *SwCntntFrm::GetNode()
{
    return (SwCntntNode*)GetDep();
}
inline const SwCntntNode *SwCntntFrm::GetNode() const
{
    return (SwCntntNode*)GetDep();
}

inline const SwCntntFrm *SwCntntFrm::GetFollow() const
{
    return (const SwCntntFrm*)SwFlowFrm::GetFollow();
}
inline SwCntntFrm *SwCntntFrm::GetFollow()
{
    return (SwCntntFrm*)SwFlowFrm::GetFollow();
}

#endif
