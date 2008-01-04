/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cntfrm.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2008-01-04 13:21:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
extern BOOL GetFrmInPage( const SwCntntFrm*, SwWhichPage, SwPosPage, SwPaM* );

class SwCntntFrm: public SwFrm, public SwFlowFrm
{
    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );    //ruft MakePrtArea

    // --> OD 2007-11-26 #b6614158#
    // parameter <bObjsInNewUpper>, indicating that objects are existing in
    // remaining area of new upper
    BOOL _WouldFit( SwTwips nSpace,
                    SwLayoutFrm *pNewUpper,
                    BOOL bTstMove,
                    const bool bObjsInNewUpper );
    // <--
    virtual void MakeAll();

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    virtual BOOL ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL, BOOL& );

    const SwCntntFrm* ImplGetNextCntntFrm( bool bFwd ) const;

protected:

    BOOL MakePrtArea( const SwBorderAttrs & );

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
    virtual SwTwips ShrinkFrm( SwTwips, BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, BOOL bTst = FALSE, BOOL bInfo = FALSE );

    SwCntntFrm( SwCntntNode * const );

public:
    virtual ~SwCntntFrm();
    TYPEINFO(); //bereits in Basisklassen drin

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

    inline       SwCntntNode *GetNode();
    inline const SwCntntNode *GetNode() const;
                 USHORT       GetSectionLevel();

    inline const SwCntntFrm *GetFollow() const;
    inline       SwCntntFrm *GetFollow();
    SwTxtFrm* FindMaster() const;

        //Layoutabhaengiges Cursortravelling
    virtual BOOL    LeftMargin( SwPaM * ) const;
    virtual BOOL    RightMargin( SwPaM *, BOOL bAPI = FALSE ) const;
    virtual BOOL    UnitUp( SwPaM *, const SwTwips nOffset = 0,
                            BOOL bSetInReadOnly = FALSE ) const;
    virtual BOOL    UnitDown( SwPaM *, const SwTwips nOffset = 0,
                            BOOL bSetInReadOnly = FALSE ) const;
    inline  BOOL    StartNextPage( SwPaM * ) const;
    inline  BOOL    StartPrevPage( SwPaM * ) const;
    inline  BOOL    StartCurrPage( SwPaM * ) const;
    inline  BOOL    EndCurrPage( SwPaM * ) const;
    inline  BOOL    EndNextPage( SwPaM * ) const;
    inline  BOOL    EndPrevPage( SwPaM * ) const;

    // nMaxHeight is the required height
    // bSplit indicates, that the paragraph has to be split
    // bTst indicates, that we are currently doing a test formatting
    virtual sal_Bool WouldFit( SwTwips &nMaxHeight, sal_Bool &bSplit, sal_Bool bTst );

    BOOL MoveFtnCntFwd( BOOL, SwFtnBossFrm* );//von MoveFwd gerufen bei Ftn-Inhalt

    inline  SwCntntFrm* GetNextCntntFrm() const;
    inline  SwCntntFrm* GetPrevCntntFrm() const;
};

inline SwCntntFrm* SwCntntFrm::GetNextCntntFrm() const
{
    if ( GetNext() && GetNext()->IsCntntFrm() )
        return (SwCntntFrm*)GetNext();
    else
        return (SwCntntFrm*)ImplGetNextCntntFrm( true );
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
