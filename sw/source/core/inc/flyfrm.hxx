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
#ifndef _FLYFRM_HXX
#define _FLYFRM_HXX

#include "layfrm.hxx"

class SwPageFrm;
class SwFlyFrmFmt;
class SwFmtFrmSize;
struct SwCrsrMoveState;
class SwBorderAttrs;
class SwVirtFlyDrawObj;
class SwSpzFrmFmts;
class SwAttrSetChg;
class PolyPolygon;

// #i26791#
#include <anchoredobject.hxx>

//Sucht ausgehend von pOldAnch einen Anker fuer Absatzgebundene Rahmen.
//Wird beim Draggen von Absatzgebundenen Objekten zur Ankeranzeige sowie
//fuer Ankerwechsel benoetigt.
//implementiert in layout/flycnt.cxx
const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const BOOL bBody = FALSE );

// berechnet das Rechteck, in dem das Objekt bewegt bzw. resized werden darf
BOOL CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, BOOL bMove = TRUE );

//allg. Basisklasse fuer alle Freifliegenden Rahmen
// #i26791# - inherit also from <SwAnchoredFlyFrm>
class SwFlyFrm : public SwLayoutFrm, public SwAnchoredObject
{
    //darf Locken. Definiert in frmtool.cxx
    friend void AppendObjs   ( const SwSpzFrmFmts *, ULONG, SwFrm *, SwPageFrm * );
    friend void AppendAllObjs( const SwSpzFrmFmts * );
    friend void Notify( SwFlyFrm *, SwPageFrm *pOld, const SwRect &rOld,
                        const SwRect* pOldPrt );

    void InitDrawObj( BOOL bNotify );   //Wird von den CToren gerufen.
    void FinitDrawObj();                //Wird vom CTor gerufen.

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    using SwLayoutFrm::CalcRel;

protected:

    SwFlyFrm *pPrevLink,        // Vorgaenger/Nachfolger fuer Verkettung mit
             *pNextLink;        // Textfluss

    // #i26791# - moved to <SwAnchoredObject>
//    Point aRelPos;   //Die Relative Position zum Master

private:
    BOOL bLocked    :1; //Cntnt-gebundene Flys muessen derart blockiert werden
                        //koennen, dass sie nicht Formatiert werden; :MakeAll
                        //returnt dann sofort. Dies ist bei Seitenwechseln
                        //waehrend der Formatierung notwendig.
                        //Auch wahrend des RootCTors ist dies notwendig da
                        //sonst der Anker formatiert wird obwohl die Root noch
                        //nicht korrekt an der Shell haengt und weil sonst
                        //initial zuviel Formatiert wuerde.
    BOOL bNotifyBack:1; //TRUE wenn am Ende eines MakeAll() der Background
                        //vom NotifyDTor benachrichtigt werden muss.
protected:

    BOOL bInvalid :1;   //Pos, PrtArea od. SSize wurden Invalidiert, sie werden
                        //gleich wieder Validiert, denn sie muessen _immer_
                        //gueltig sein. Damit in LayAction korrekt gearbeitet
                        //werden kann muss hier festgehalten werden, dass sie
                        //invalidiert wurden. Ausnahmen bestaetigen die Regelt!
    BOOL bMinHeight:1;  //TRUE wenn die vom Attribut vorgegebene Hoehe eine
                        //eine Minimalhoehe ist (der Frm also bei Bedarf
                        //darueberhinaus wachsen kann).
    BOOL bHeightClipped :1; //TRUE wenn der Fly nicht die Pos/Size anhand der Attrs
    BOOL bWidthClipped  :1; //formatieren konnte, weil z.B. nicht genug Raum vorh.
                            //war.
    BOOL bFormatHeightOnly  :1; //Damit nach einer Anpassung der Breite
                                //(CheckClip) nur das Format aufgerufen wird;
                                //nicht aber die Breite anhand der Attribute
                                //wieder bestimmt wird.
    BOOL bInCnt         :1; // FLY_AS_CHAR, anchored as character
    BOOL bAtCnt         :1; // FLY_AT_PARA, anchored at paragraph
    BOOL bLayout        :1; // FLY_AT_PAGE, FLY_AT_FLY, at page or at frame
    BOOL bAutoPosition  :1; // FLY_AT_CHAR, anchored at character
    BOOL bNoShrink      :1; // temporary forbud of shrinking to avoid loops
    BOOL bLockDeleteContent :1;           // If the flag is set, the content of the
                                        // fly frame is not deleted if moved to
                                        // invisible layer.

    friend class SwNoTxtFrm; // Darf NotifyBackground rufen

    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    void MakePrtArea( const SwBorderAttrs &rAttrs );

    void Lock()         { bLocked = TRUE; }
    void Unlock()       { bLocked = FALSE; }

    void SetMinHeight()  { bMinHeight = TRUE; }
    void ResetMinHeight(){ bMinHeight = FALSE; }

    Size CalcRel( const SwFmtFrmSize &rSz ) const;
    SwTwips CalcAutoWidth() const;

    SwFlyFrm( SwFlyFrmFmt*, SwFrm *pAnchor );

    /** method to assure that anchored object is registered at the correct
        page frame

        #i28701#
    */
    virtual void RegisterAtCorrectPage();

    // #i68520#
    virtual bool _SetObjTop( const SwTwips _nTop );
    virtual bool _SetObjLeft( const SwTwips _nLeft );

    // #i70122#
    virtual const SwRect GetObjBoundRect() const;

public:
    // #i26791#
    TYPEINFO();

    virtual ~SwFlyFrm();
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;
    virtual void Paint( const SwRect&, const SwPrtOptions *pPrintData = NULL ) const;
    virtual Size ChgSize( const Size& aNewSize );
    virtual BOOL GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0 ) const;

    virtual void  CheckDirection( BOOL bVert );
    virtual void Cut();
#if OSL_DEBUG_LEVEL > 1
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

    SwTwips _Shrink( SwTwips, BOOL bTst );
    SwTwips _Grow  ( SwTwips, BOOL bTst );
    void    _Invalidate( SwPageFrm *pPage = 0 );

    BOOL FrmSizeChg( const SwFmtFrmSize & );

    SwFlyFrm *GetPrevLink() const { return pPrevLink; }
    SwFlyFrm *GetNextLink() const { return pNextLink; }

    static void ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );
    static void UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );

    SwFlyFrm *FindChainNeighbour( SwFrmFmt &rFmt, SwFrm *pAnch = 0 );

    // #i26791#
    const SwVirtFlyDrawObj* GetVirtDrawObj() const;
    SwVirtFlyDrawObj *GetVirtDrawObj();
    void NotifyDrawObj();

    void ChgRelPos( const Point &rAbsPos );
    BOOL IsInvalid() const { return bInvalid; }
    void Invalidate() const { ((SwFlyFrm*)this)->bInvalid = TRUE; }
    void Validate() const { ((SwFlyFrm*)this)->bInvalid = FALSE; }

    BOOL IsMinHeight()  const { return bMinHeight; }
    BOOL IsLocked()     const { return bLocked; }
    BOOL IsAutoPos()    const { return bAutoPosition; }
    BOOL IsFlyInCntFrm() const { return bInCnt; }
    BOOL IsFlyFreeFrm() const { return bAtCnt || bLayout; }
    BOOL IsFlyLayFrm() const { return bLayout; }
    BOOL IsFlyAtCntFrm() const { return bAtCnt; }

    BOOL IsNotifyBack() const { return bNotifyBack; }
    void SetNotifyBack()      { bNotifyBack = TRUE; }
    void ResetNotifyBack()    { bNotifyBack = FALSE; }
    BOOL IsNoShrink()   const { return bNoShrink; }
    void SetNoShrink( BOOL bNew ) { bNoShrink = bNew; }
    BOOL IsLockDeleteContent()  const { return bLockDeleteContent; }
    void SetLockDeleteContent( BOOL bNew ) { bLockDeleteContent = bNew; }


    BOOL IsClipped()        const   { return bHeightClipped || bWidthClipped; }
    BOOL IsHeightClipped()  const   { return bHeightClipped; }
    BOOL IsWidthClipped()   const   { return bWidthClipped;  }

    BOOL IsLowerOf( const SwLayoutFrm* pUpper ) const;
    inline BOOL IsUpperOf( const SwFlyFrm& _rLower ) const
    {
        return _rLower.IsLowerOf( this );
    }

    SwFrm *FindLastLower();

    // #i13147# - add parameter <_bForPaint> to avoid load of
    // the graphic during paint. Default value: sal_False
    BOOL GetContour( PolyPolygon&   rContour,
                     const sal_Bool _bForPaint = sal_False ) const;


    //Auf dieser Shell painten (PreView, Print-Flag usw. rekursiv beachten)?.
    static BOOL IsPaint( SdrObject *pObj, const ViewShell *pSh );

    /** SwFlyFrm::IsBackgroundTransparent

        determines, if background of fly frame has to be drawn transparent
        definition found in /core/layout/paintfrm.cxx

        @return true, if background color is transparent or a existing background
        graphic is transparent.
    */
    sal_Bool IsBackgroundTransparent() const;

    /** SwFlyFrm::IsShadowTransparent

        determine, if shadow color of fly frame has to be drawn transparent
        definition found in /core/layout/paintfrm.cxx

        @return true, if shadow color is transparent.
    */
    sal_Bool IsShadowTransparent() const;

    void Chain( SwFrm* _pAnchor );
    void Unchain();
    void InsertCnt();
    void DeleteCnt();
    void InsertColumns();

    // #i26791# - pure virtual methods of base class <SwAnchoredObject>
    virtual void MakeObjPos();
    virtual void InvalidateObjPos();

    virtual SwFrmFmt& GetFrmFmt();
    virtual const SwFrmFmt& GetFrmFmt() const;

    virtual const SwRect GetObjRect() const;

    /** method to determine, if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwAnchoredObject::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame is locked resp. col-locked.
    */
    virtual bool IsFormatPossible() const;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
