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
#ifndef _FTNFRM_HXX
#define _FTNFRM_HXX

#include "layfrm.hxx"
namespace binfilter {

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
    SwFtnContFrm( SwFrmFmt* );


    virtual SwTwips ShrinkFrm( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm( SwTwips, SZPTR
                             BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual void    Format( const SwBorderAttrs *pAttrs = 0 );
};

class SwFtnFrm: public SwLayoutFrm
{
    //Zeiger auf den FtnFrm in dem die Fussnote weitergefuehrt wird:
    // 0     wenn kein Follow vorhanden,
    // this  beim letzten
    // der Follow sonst.
    SwFtnFrm	 *pFollow;
    SwFtnFrm	 *pMaster;		//Der FtnFrm dessen Follow ich bin.
    SwCntntFrm	 *pRef;			//In diesem CntntFrm steht die Fussnotenref.
    SwTxtFtn	 *pAttr;		//Fussnotenattribut (zum wiedererkennen)

    BOOL bBackMoveLocked;		//Absaetze in dieser Fussnote duerfen derzeit
                                //nicht rueckwaerts fliessen.
#ifdef DBG_UTIL
protected:
    virtual SwTwips ShrinkFrm( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
#endif


public:
    SwFtnFrm( SwFrmFmt*, SwCntntFrm*, SwTxtFtn* );

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );


#ifndef DBG_UTIL
    const SwCntntFrm *GetRef() const	{ return pRef; }
         SwCntntFrm  *GetRef()  		{ return pRef; }
#else
    //JP 15.10.2001: in a non pro version test if the attribute has the same
    //				meaning which his reference is
    const SwCntntFrm *GetRef() const;
         SwCntntFrm  *GetRef();
#endif
    const SwCntntFrm *GetRefFromAttr()  const;
          SwCntntFrm *GetRefFromAttr();

    const SwFtnFrm *GetFollow() const 	{ return pFollow; }
          SwFtnFrm *GetFollow() 		{ return pFollow; }

    const SwFtnFrm *GetMaster() const	{ return pMaster; }
          SwFtnFrm *GetMaster() 		{ return pMaster; }

    const SwTxtFtn   *GetAttr() const 	{ return pAttr; }
          SwTxtFtn	 *GetAttr() 	  	{ return pAttr; }

    void SetFollow( SwFtnFrm *pNew ) { pFollow = pNew; }
    void SetMaster( SwFtnFrm *pNew ) { pMaster = pNew; }
    void SetRef   ( SwCntntFrm *pNew ) { pRef = pNew; }

    void InvalidateNxtFtnCnts( SwPageFrm* pPage );

    void LockBackMove()		{ bBackMoveLocked = TRUE; }
    void UnlockBackMove()   { bBackMoveLocked = FALSE;}
    BOOL IsBackMoveLocked() { return bBackMoveLocked; }

    // Verhindert, dass der letzte Inhalt den SwFtnFrm mitloescht (Cut())
    inline void ColLock()		{ bColLocked = TRUE; }
    inline void ColUnlock()   	{ bColLocked = FALSE; }
};

} //namespace binfilter
#endif	//_FTNFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
