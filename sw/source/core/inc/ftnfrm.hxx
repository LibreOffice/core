/*************************************************************************
 *
 *  $RCSfile: ftnfrm.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ama $ $Date: 2001-10-19 10:12:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
    SwFtnContFrm( SwFrmFmt* );

    const SwFtnFrm* FindFootNote() const;
    const SwFtnFrm* FindEndNote() const;

    virtual SwTwips ShrinkFrm( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm( SwTwips, SZPTR
                             BOOL bTst = FALSE, BOOL bInfo = FALSE );
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

    BOOL bBackMoveLocked;       //Absaetze in dieser Fussnote duerfen derzeit
                                //nicht rueckwaerts fliessen.
#ifndef PRODUCT
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

    BOOL operator<( const SwTxtFtn* pTxtFtn ) const;

#ifdef PRODUCT
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

    void LockBackMove()     { bBackMoveLocked = TRUE; }
    void UnlockBackMove()   { bBackMoveLocked = FALSE;}
    BOOL IsBackMoveLocked() { return bBackMoveLocked; }

    // Verhindert, dass der letzte Inhalt den SwFtnFrm mitloescht (Cut())
    inline void ColLock()       { bColLocked = TRUE; }
    inline void ColUnlock()     { bColLocked = FALSE; }
};

#endif  //_FTNFRM_HXX
