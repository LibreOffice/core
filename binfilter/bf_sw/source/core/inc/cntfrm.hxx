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
#ifndef _CNTFRM_HXX
#define _CNTFRM_HXX

#include "frame.hxx"
#include "flowfrm.hxx"
#include "cshtyp.hxx"
namespace binfilter {

class SwLayoutFrm;
class SwPageFrm;
class SwCntntNode;
struct SwCrsrMoveState;
class SwBorderAttrs;
class SwAttrSetChg;

//Implementiert in cntfrm.cxx, wird von cntfrm.cxx und crsrsh.cxx angezogen
extern BOOL GetFrmInPage( const SwCntntFrm*, SwWhichPage, SwPosPage, SwPaM* );

class SwCntntFrm: public SwFrm, public SwFlowFrm
{
    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );	//ruft MakePrtArea

    BOOL _WouldFit( SwTwips nSpace, SwLayoutFrm *pNewUpper, BOOL bTstMove );
    virtual void MakeAll();

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    virtual BOOL ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL, BOOL& );

protected:

    BOOL MakePrtArea( const SwBorderAttrs & );

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
    virtual SwTwips ShrinkFrm( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );

    SwCntntFrm( SwCntntNode * const );

public:
    virtual ~SwCntntFrm();
    TYPEINFO();	//bereits in Basisklassen drin

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

    inline 		 SwCntntNode *GetNode();
    inline const SwCntntNode *GetNode() const;
                 USHORT 	  GetSectionLevel();

    inline const SwCntntFrm *GetFollow() const;
    inline		 SwCntntFrm *GetFollow();

        //Layoutabhaengiges Cursortravelling
    inline	BOOL	StartNextPage( SwPaM * ) const;
    inline	BOOL	StartPrevPage( SwPaM * ) const;
    inline	BOOL	StartCurrPage( SwPaM * ) const;
    inline	BOOL	EndCurrPage( SwPaM * ) const;
    inline	BOOL	EndNextPage( SwPaM * ) const;
    inline	BOOL	EndPrevPage( SwPaM * ) const;

    //nMaxHeight liefert die benoetigte Hoehe,
    //bSplit sagt, obj der Absatz gesplittet werden muss.
    virtual BOOL WouldFit( SwTwips &nMaxHeight, BOOL &bSplit );

};

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

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
