/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtfly.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:43:57 $
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
#ifndef _TXTFLY_HXX
#define _TXTFLY_HXX
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

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
// --> OD 2004-10-06 #i26945#
class SwAnchoredObject;
// <--

// eine kleine Schweinerei, weil enums nicht forward-deklariert werden koennen.
typedef MSHORT _FlyCntnt;

SV_DECL_PTRARR( SwFlyList, SdrObject*, 10, 10 )

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
    const SdrObject* GetObject( MSHORT nPos ){ return pSdrObj[ nPos ]; }
    MSHORT GetCount() const { return nObjCnt; }
    void ClrObject( MSHORT nPos );
    static const SwRect CalcBoundRect( const SdrObject* pObj,
        const SwRect &rLine, const SwTxtFrm* pFrm, const long nXPos,
        const sal_Bool bRight );
#ifndef PRODUCT
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
    const SdrObject     *pCurrFly;

    const SwTxtFrm      *pCurrFrm;

    const SwCntntFrm    *pMaster;
    SwFlyList           *pFlyList;

    long nMinBottom;
    long nNextTop; // Hier wird die Oberkante des "naechsten" Rahmens gespeichert
    ULONG nIndex;
    sal_Bool bOn : 1;
    sal_Bool bLeftSide : 1;
    sal_Bool bTopRule: 1;
    sal_Bool mbIgnoreCurrentFrame: 1;
    sal_Bool mbIgnoreContour: 1;
    // --> OD 2004-12-17 #118809# - boolean, indicating if objects in page
    // header|footer are considered for text frames not in page header|footer.
    sal_Bool mbIgnoreObjsInHeaderFooter: 1;
    // <--
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
    // --> OD 2004-10-06 #i26945# - change first parameter:
    // Now it's the <SwAnchoredObject> instance of the floating screen object
    sal_Bool GetTop( const SwAnchoredObject* _pAnchoredObj,
                     const sal_Bool bInFtn,
                     const sal_Bool bInFooterOrHeader );
    // <--
    SwTwips CalcMinBottom() const;
    const SwCntntFrm* _GetMaster();

public:
    inline SwTxtFly()
    {
        mbIgnoreCurrentFrame = sal_False;
        mbIgnoreCurrentFrame = sal_False;
        // --> OD 2004-12-17 #118809#
        mbIgnoreObjsInHeaderFooter = sal_False;
        // <--
        pFlyList = 0; pMaster = 0;
    }
    inline SwTxtFly( const SwTxtFrm *pFrm )
        { CtorInit( pFrm ); }

    SwTxtFly( const SwTxtFly& rTxtFly );
    inline ~SwTxtFly() { delete pFlyList; }
    void CtorInit( const SwTxtFrm *pFrm );
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
        { ((SwTxtFly*)this)->nNextTop = nNew;   }

    // Liefert zu einem SdrObject das von ihm _beanspruchte_ Rect
    // (unter Beruecksichtigung der Order) zurueck.
    SwRect FlyToRect( const SdrObject *pObj, const SwRect &rRect ) const;

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
    // --> OD 2004-12-17 #118809#
    inline void SetIgnoreObjsInHeaderFooter( const sal_Bool _bNew )
    {
        mbIgnoreObjsInHeaderFooter = _bNew;
    }
    // <--

#ifndef PRODUCT
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
