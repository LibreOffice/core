/*************************************************************************
 *
 *  $RCSfile: layfrm.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _LAYFRM_HXX
#define _LAYFRM_HXX

#include "frame.hxx"

class SwCntntFrm;
class SwFlowFrm;
class Sw3FrameIo;
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
    friend void   RestoreCntnt( SwFrm *, SwLayoutFrm *, SwFrm *pSibling );

    //entfernt leere SwSectionFrms aus einer Kette
    friend SwFrm* SwClearDummies( SwFrm* pFrm );

    void CopySubtree( const SwLayoutFrm *pDest );
protected:
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual void MakeAll();

    SwFrm           *pLower;

    virtual SwTwips ShrinkFrm( SwTwips, const SzPtr,
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, const SzPtr,
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );

    long CalcRel( const SwFmtFrmSize &rSz, BOOL bWidth ) const;

    SwLayoutFrm( Sw3FrameIo&, SwLayoutFrm* );
    void LoadLower( Sw3FrameIo& );
    void StoreLower( Sw3FrameIo& ) const;
public:
    void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const;
    void RefreshLaySubsidiary( const SwPageFrm*, const SwRect& ) const;
    void RefreshExtraData( const SwRect & ) const;

        //Proportionale Groessenanpassung der untergeordneten.
    void ChgLowersProp( const Size& rOldSize );

    void AdjustColumns( const SwFmtCol *pCol, BOOL bAdjustAttributes );
    void ChgColumns( const SwFmtCol &rOld, const SwFmtCol &rNew,
        const BOOL bChgFtn = FALSE );


        //Painted die Column-Trennlinien fuer die innenliegenden Columns.
    void PaintColLines( const SwRect &, const SwFmtCol &,
                        const SwPageFrm * ) const;

    virtual void Store( Sw3FrameIo& ) const;

    virtual BOOL  GetCrsrOfst( SwPosition *, Point&,
                                const SwCrsrMoveState* = 0 ) const;

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );

        //sucht den dichtesten Cntnt zum SPoint, wird bei Seiten, Flys und Cells
        //benutzt wenn GetCrsrOfst versagt hat.
    const SwCntntFrm* GetCntntPos( Point &rPoint, const BOOL bDontLeave,
                                   const BOOL bBodyOnly = FALSE,
                                   const BOOL bCalc = FALSE,
                                   const SwCrsrMoveState *pCMS = 0,
                                   const BOOL bDefaultExpand = TRUE ) const;

    SwLayoutFrm( SwFrmFmt* );
    ~SwLayoutFrm();

    virtual void Paint( const SwRect& ) const;
    const SwFrm *Lower() const { return pLower; }
          SwFrm *Lower()       { return pLower; }
    const SwCntntFrm *ContainsCntnt() const;
    inline SwCntntFrm *ContainsCntnt();
    const SwCellFrm *FirstCell() const;
    inline SwCellFrm *FirstCell();
    const SwFrm *ContainsAny() const;
    inline SwFrm *ContainsAny();
    BOOL IsAnLower( const SwFrm * ) const;

    const SwFrmFmt *GetFmt() const { return (const SwFrmFmt*)GetDep(); }
          SwFrmFmt *GetFmt()       { return (SwFrmFmt*)GetDep(); }
    void            SetFrmFmt( SwFrmFmt* );

    //Verschieben der Ftns aller Lower - ab dem StartCntnt.
    //TRUE wenn mindestens eine Ftn verschoben wurde.
    //Ruft das Update der Seitennummer wenn bFtnNums gesetzt ist.
    BOOL MoveLowerFtns( SwCntntFrm *pStart, SwFtnBossFrm *pOldBoss,
                        SwFtnBossFrm *pNewBoss, const BOOL bFtnNums );

    //Sorgt dafuer, dass innenliegende Flys noetigenfalls zum clippen bzw.
    //reformatieren invalidiert werden.
    void NotifyFlys();

    //Invalidiert diejenigen innenliegenden Frames, deren Breite und/oder
    //Hoehe Prozentual berechnet werden. Auch Rahmen, die an this oder an
    //innenliegenden verankert sind werden ggf. invalidiert.
    void InvaPercentLowers();

    //Gerufen von Format fuer Rahmen und Bereichen mit Spalten.
    void FormatWidthCols( const SwBorderAttrs &, const SwTwips nBorder,
                          const SwTwips nMinHeight );

    // InnerHeight returns the height of the content and may be bigger or
    // less than the PrtArea-Height of the layoutframe himself
    SwTwips InnerHeight() const;
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

inline SwFrm* SwLayoutFrm::ContainsAny()
{
    return (SwFrm*)(((const SwLayoutFrm*)this)->ContainsAny());
}

// Diese SwFrm-inlines sind hier, damit frame.hxx nicht layfrm.hxx includen muss
inline BOOL SwFrm::IsColBodyFrm() const
{
    return nType == FRM_BODY && GetUpper()->IsColumnFrm();
}

inline BOOL SwFrm::IsPageBodyFrm() const
{
    return nType == FRM_BODY && GetUpper()->IsPageFrm();
}

#endif  //_LAYFRM_HXX
