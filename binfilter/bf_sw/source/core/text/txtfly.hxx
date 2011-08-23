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

#ifndef _SVARRAY_HXX //autogen
#include <bf_svtools/svarray.hxx>
#endif

#include "swtypes.hxx"
#include "swrect.hxx"
class OutputDevice; 
class VirtualDevice; 
class PolyPolygon; 
class Color; 
namespace binfilter {


class SwFont;
class SwCntntFrm;
class SwFlyFrm;
class SwPageFrm;
class SwTxtFly;
class SdrObject;
class SwWrongList;
class SwTxtPaintInfo;
class SwFmt;

class TextRanger;


// eine kleine Schweinerei, weil enums nicht forward-deklariert werden koennen.
typedef MSHORT _FlyCntnt;

SV_DECL_PTRARR( SwFlyList, SdrObject*, 10, 10 )

/*************************************************************************
 *						class SwFlyIter
 *************************************************************************/
enum PAGESIDE { LEFT_SIDE, RIGHT_SIDE, DONTKNOW_SIDE };

/*************************************************************************
 *						class SwContourCache
 *************************************************************************/

class SwDrawTextInfo;
// Contour-Cache, globale Variable, in txtinit.cxx initialisiert/zerstoert
// und in txtfly.cxx benutzt bei Konturumfluss
class SwContourCache;
extern SwContourCache *pContourCache;

#ifdef VERTICAL_LAYOUT
class SwTxtFrm;
#endif

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
#ifdef VERTICAL_LAYOUT
    const SwRect ContourRect( const SwFmt* pFmt, const SdrObject* pObj,
        const SwTxtFrm* pFrm, const SwRect &rLine, const long nXPos,
        const sal_Bool bRight );
#else
    const SwRect ContourRect( const SwFmt* pFmt, const SdrObject* pObj,
        const SwRect &rLine, const long nXPos, const sal_Bool bRight );
#endif

public:
    SwContourCache();
    ~SwContourCache();
    const SdrObject* GetObject( MSHORT nPos ){ return pSdrObj[ nPos ]; }
    MSHORT GetCount() const { return nObjCnt; }
    void ClrObject( MSHORT nPos );
#ifdef VERTICAL_LAYOUT
    static const SwRect CalcBoundRect( const SdrObject* pObj,
        const SwRect &rLine, const SwTxtFrm* pFrm, const long nXPos,
        const sal_Bool bRight );
#else
    static const SwRect CalcBoundRect( const SdrObject* pObj,
        const SwRect &rLine, const long nXPos, const sal_Bool bRight );
#endif
#ifdef DBG_UTIL
#endif
};

/*************************************************************************
 *						class SwTxtFly
 *************************************************************************/

class SwTxtFly
{
    const SwPageFrm		*pPage;
    const SdrObject		*pCurrFly;

#ifdef VERTICAL_LAYOUT
    const SwTxtFrm      *pCurrFrm;
#else
    const SwCntntFrm    *pCurrFrm;
#endif

    const SwCntntFrm	*pMaster;
    SwFlyList			*pFlyList;

    long nMinBottom;
    long nNextTop; // Hier wird die Oberkante des "naechsten" Rahmens gespeichert
    ULONG nIndex;
    sal_Bool bOn : 1;
    sal_Bool bLeftSide : 1;
    sal_Bool bTopRule: 1;
    sal_Bool mbIgnoreCurrentFrame: 1;
    sal_Bool mbIgnoreContour: 1;
    SwRect _GetFrm( const SwRect &rPortion, sal_Bool bTop ) const;
    SwFlyList* InitFlyList();
    // iteriert ueber die Fly-Liste
    sal_Bool ForEach( const SwRect &rRect, SwRect* pRect, sal_Bool bAvoid ) const;
    _FlyCntnt CalcSmart( const SdrObject *pObj ) const;
    // liefert die Order eines FlyFrms
    _FlyCntnt GetOrder( const SdrObject *pObj ) const;
    void CalcRightMargin( SwRect &rFly, MSHORT nPos, const SwRect &rLine ) const;
    void CalcLeftMargin( SwRect &rFly, MSHORT nPos, const SwRect &rLine ) const;
    MSHORT GetPos( const SdrObject *pObj ) const;
    sal_Bool GetTop( const SdrObject *pNew, const sal_Bool bInFtn,
                     const sal_Bool bInFooterOrHeader );
    SwTwips CalcMinBottom() const;
    const SwCntntFrm* _GetMaster();

public:
    inline SwTxtFly() { mbIgnoreCurrentFrame = sal_False;
                        mbIgnoreCurrentFrame = sal_False;
                        pFlyList = 0; pMaster = 0; }
    inline SwTxtFly( const SwTxtFrm *pFrm )
        { CtorInit( pFrm ); }

    SwTxtFly( const SwTxtFly& rTxtFly );
    inline ~SwTxtFly() { delete pFlyList; }
#ifdef VERTICAL_LAYOUT
    void CtorInit( const SwTxtFrm *pFrm );
#else
    void CtorInit( const SwCntntFrm *pFrm );
#endif
    void SetTopRule(){ bTopRule = sal_False; }

    SwFlyList* GetFlyList() const
        { return pFlyList ? pFlyList : ((SwTxtFly*)this)->InitFlyList(); }
    inline SwRect GetFrm( const SwRect &rPortion, sal_Bool bTop = sal_True ) const;
    inline sal_Bool IsOn() const { return bOn; }
    inline sal_Bool Relax( const SwRect &rRect );
    inline sal_Bool Relax();
    inline SwTwips GetMinBottom() const
        { return pFlyList ? nMinBottom : CalcMinBottom(); }
    inline const SwCntntFrm* GetMaster() const
        { return pMaster ? pMaster : ((SwTxtFly*)this)->_GetMaster(); }
    inline long GetNextTop() const { return nNextTop; }
    // Diese temporaere Variable darf auch in const-Methoden manipuliert werden
    inline void SetNextTop( long nNew ) const
        { ((SwTxtFly*)this)->nNextTop = nNew;	}

    // Liefert zu einem SdrObject das von ihm _beanspruchte_ Rect
    // (unter Beruecksichtigung der Order) zurueck.
    SwRect FlyToRect( const SdrObject *pObj, const SwRect &rRect ) const;

    // Die Drawmethoden stellen sicher, dass ueberlappende Frames
    // (ausser bei transparenten Frames) nicht uebergepinselt werden.


    // Liefert zurueck, ob die Zeile von einem Frame ueberlappt wird.
    sal_Bool IsAnyFrm( const SwRect &rLine ) const;
    sal_Bool IsAnyFrm() const;
    //Das Rechteck kann leer sein, es gilt dann der Frm.
    sal_Bool IsAnyObj( const SwRect& ) const;

    void SetIgnoreCurrentFrame( sal_Bool bNew ) { mbIgnoreCurrentFrame = bNew; }
    void SetIgnoreContour( sal_Bool bNew ) { mbIgnoreContour = bNew; }

#ifdef DBG_UTIL
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

} //namespace binfilter
#endif
