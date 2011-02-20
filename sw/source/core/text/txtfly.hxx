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
#ifndef _TXTFLY_HXX
#define _TXTFLY_HXX
#include <svl/svarray.hxx>

#include "swtypes.hxx"
#include "swrect.hxx"

class OutputDevice;
class SwCntntFrm;
class SwPageFrm;
class SwTxtFly;
class SdrObject;
class SwTxtPaintInfo;
class SwFmt;
class TextRanger;
class Color;
class SwAnchoredObject;

#include <fmtsrndenum.hxx>

#include <vector>
typedef std::vector< SwAnchoredObject* > SwAnchoredObjList;

/*************************************************************************
 *                      class SwFlyIter
 *************************************************************************/
enum PAGESIDE { LEFT_SIDE, RIGHT_SIDE, DONTKNOW_SIDE };

/*************************************************************************
 *                      class SwContourCache
 *************************************************************************/

class SwDrawTextInfo;
// Contour-Cache, globale Variable, in txtinit.cxx initialisiert/zerstoert
// und in txtfly.cxx benutzt bei Konturumfluss
class SwContourCache;
extern SwContourCache *pContourCache;
class SwTxtFrm;

#define POLY_CNT 20
#define POLY_MIN 5
#define POLY_MAX 4000

class SwContourCache
{
    friend void ClrContourCache();
    const SdrObject *pSdrObj[ POLY_CNT ];
    TextRanger *pTextRanger[ POLY_CNT ];
    long nPntCnt;
    MSHORT nObjCnt;
    const SwRect ContourRect( const SwFmt* pFmt, const SdrObject* pObj,
        const SwTxtFrm* pFrm, const SwRect &rLine, const long nXPos,
        const sal_Bool bRight );

public:
    SwContourCache();
    ~SwContourCache();
    const SdrObject* GetObject( MSHORT nPos ) const{ return pSdrObj[ nPos ]; }
    MSHORT GetCount() const { return nObjCnt; }
    void ClrObject( MSHORT nPos );
    static const SwRect CalcBoundRect( const SwAnchoredObject* pAnchoredObj,
                                       const SwRect &rLine,
                                       const SwTxtFrm* pFrm,
                                       const long nXPos,
                                       const sal_Bool bRight );
#if OSL_DEBUG_LEVEL > 1
    void ShowContour( OutputDevice* pOut, const SdrObject* pObj,
                      const Color& rClosedColor, const Color& rOpenColor );
#endif
};

/*************************************************************************
 *                      class SwTxtFly
 *************************************************************************/

class SwTxtFly
{
    const SwPageFrm     *pPage;
    const SwAnchoredObject* mpCurrAnchoredObj;
    const SwTxtFrm      *pCurrFrm;
    const SwCntntFrm    *pMaster;
    SwAnchoredObjList* mpAnchoredObjList;
    long nMinBottom;
    long nNextTop; // Hier wird die Oberkante des "naechsten" Rahmens gespeichert
    ULONG nIndex;
    sal_Bool bOn : 1;
    sal_Bool bLeftSide : 1;
    sal_Bool bTopRule: 1;
    sal_Bool mbIgnoreCurrentFrame: 1;
    sal_Bool mbIgnoreContour: 1;
    // boolean, indicating if objects in page header|footer are considered for
    // text frames not in page header|footer.
    sal_Bool mbIgnoreObjsInHeaderFooter: 1;
    SwRect _GetFrm( const SwRect &rPortion, sal_Bool bTop ) const;
    SwAnchoredObjList* InitAnchoredObjList();
    inline SwAnchoredObjList* GetAnchoredObjList() const
    {
        return mpAnchoredObjList
               ? mpAnchoredObjList
               : const_cast<SwTxtFly*>(this)->InitAnchoredObjList();
    }
    // iterates over the anchored object list <mpAnchoredObjList>
    sal_Bool ForEach( const SwRect &rRect, SwRect* pRect, sal_Bool bAvoid ) const;
    SwSurround _GetSurroundForTextWrap( const SwAnchoredObject* pAnchoredObj ) const;
    void CalcRightMargin( SwRect &rFly,
                          SwAnchoredObjList::size_type nPos,
                          const SwRect &rLine ) const;
    void CalcLeftMargin( SwRect &rFly,
                         SwAnchoredObjList::size_type nPos,
                         const SwRect &rLine ) const;
    SwAnchoredObjList::size_type GetPos( const SwAnchoredObject* pAnchoredObj ) const;
    sal_Bool GetTop( const SwAnchoredObject* _pAnchoredObj,
                     const sal_Bool bInFtn,
                     const sal_Bool bInFooterOrHeader );
    SwTwips CalcMinBottom() const;
    const SwCntntFrm* _GetMaster();

public:
    inline SwTxtFly() : pPage(0), mpCurrAnchoredObj(0), pCurrFrm(0), pMaster(0), mpAnchoredObjList(0), nMinBottom(0), nNextTop(0),
    nIndex(0) {}
    inline SwTxtFly( const SwTxtFrm *pFrm )
        { CtorInitTxtFly( pFrm ); }

    SwTxtFly( const SwTxtFly& rTxtFly );
    inline ~SwTxtFly() { delete mpAnchoredObjList; }
    void CtorInitTxtFly( const SwTxtFrm *pFrm );
    void SetTopRule(){ bTopRule = sal_False; }

    inline SwRect GetFrm( const SwRect &rPortion, sal_Bool bTop = sal_True ) const;
    inline sal_Bool IsOn() const { return bOn; }
    inline sal_Bool Relax( const SwRect &rRect );
    inline sal_Bool Relax();
    inline SwTwips GetMinBottom() const
        { return mpAnchoredObjList ? nMinBottom : CalcMinBottom(); }
    inline const SwCntntFrm* GetMaster() const
        { return pMaster ? pMaster : ((SwTxtFly*)this)->_GetMaster(); }
    inline long GetNextTop() const { return nNextTop; }
    // Diese temporaere Variable darf auch in const-Methoden manipuliert werden
    inline void SetNextTop( long nNew ) const
        { ((SwTxtFly*)this)->nNextTop = nNew;   }

    // determines the demanded rectangle for an anchored object,
    // considering its surround for text wrapping.
    SwRect AnchoredObjToRect( const SwAnchoredObject* pAnchoredObj,
                              const SwRect& rRect ) const;

    // Die Drawmethoden stellen sicher, dass ueberlappende Frames
    // (ausser bei transparenten Frames) nicht uebergepinselt werden.
    sal_Bool DrawTextOpaque( SwDrawTextInfo &rInf );

    void DrawFlyRect( OutputDevice* pOut, const SwRect &rRect,
                      const SwTxtPaintInfo &rInf, sal_Bool bNoGraphic = sal_False );

    // Liefert zurueck, ob die Zeile von einem Frame ueberlappt wird.
    sal_Bool IsAnyFrm( const SwRect &rLine ) const;
    sal_Bool IsAnyFrm() const;
    //Das Rechteck kann leer sein, es gilt dann der Frm.
    sal_Bool IsAnyObj( const SwRect& ) const;

    void SetIgnoreCurrentFrame( sal_Bool bNew ) { mbIgnoreCurrentFrame = bNew; }
    void SetIgnoreContour( sal_Bool bNew ) { mbIgnoreContour = bNew; }
    inline void SetIgnoreObjsInHeaderFooter( const sal_Bool _bNew )
    {
        mbIgnoreObjsInHeaderFooter = _bNew;
    }

#if OSL_DEBUG_LEVEL > 1
    void ShowContour( OutputDevice* pOut );
#endif
};

// Wenn in das rRect (meist die aktuelle Zeile) kein freifliegender
// Frame ragt, dann schalten wir uns ab.
// rRect ist dokumentglobal !
inline sal_Bool SwTxtFly::Relax( const SwRect &rRect )
{
    return 0 != (bOn = bOn && IsAnyFrm( rRect ));
}

inline sal_Bool SwTxtFly::Relax()
{
    return 0 != (bOn = bOn && IsAnyFrm());
}

inline SwRect SwTxtFly::GetFrm( const SwRect &rRect, sal_Bool bTop ) const
{
    return bOn ? _GetFrm( rRect, bTop ) : SwRect();
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
