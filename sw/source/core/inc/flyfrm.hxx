/*************************************************************************
 *
 *  $RCSfile: flyfrm.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:04:36 $
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

#include <orntenum.hxx>

// OD 2004-03-22 #i26791#
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif

//Sucht ausgehend von pOldAnch einen Anker fuer Absatzgebundene Rahmen.
//Wird beim Draggen von Absatzgebundenen Objekten zur Ankeranzeige sowie
//fuer Ankerwechsel benoetigt.
//implementiert in layout/flycnt.cxx
const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const BOOL bBody = FALSE );

// berechnet das Rechteck, in dem das Objekt bewegt bzw. resized werden darf
BOOL CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, BOOL bMove = TRUE );

//allg. Basisklasse fuer alle Freifliegenden Rahmen
// OD 2004-03-22 #i26791# - inherit also from <SwAnchoredFlyFrm>
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

protected:

    SwFlyFrm *pPrevLink,        // Vorgaenger/Nachfolger fuer Verkettung mit
             *pNextLink;        // Textfluss

    // OD 2004-05-27 #i26791# - moved to <SwAnchoredObject>
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
    BOOL bInCnt         :1; // FLY_IN_CNTNT, als Zeichen verankert
    BOOL bAtCnt         :1; // FLY_AT_CNTNT, am Absatz verankert
    BOOL bLayout        :1; // FLY_PAGE, FLY_AT_FLY, an Seite oder Rahmen
    BOOL bAutoPosition  :1; // FLY_AUTO_CNTNT, im Text verankerter Rahmen
    BOOL bNoShrink      :1; // temporary forbud of shrinking to avoid loops
    BOOL bLockDeleteContent :1;           // If the flag is set, the content of the
                                        // fly frame is not deleted if moved to
                                        // invisible layer.

    friend class SwNoTxtFrm; // Darf NotifyBackground rufen
//    virtual void NotifyBackground( SwPageFrm *pPage,
//                                   const SwRect& rRect, PrepareHint eHint) = 0;

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

        OD 2004-07-02 #i28701#

        @author OD
    */
    virtual void RegisterAtCorrectPage();

public:
    // OD 2004-03-23 #i26791#
    TYPEINFO();

    virtual ~SwFlyFrm();
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;
    virtual void Paint( const SwRect& ) const;
    virtual void ChgSize( const Size& aNewSize );
    virtual BOOL GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0 ) const;

    virtual void  CheckDirection( BOOL bVert );
    virtual void Cut();
#ifndef PRODUCT
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

    SwTwips _Shrink( SwTwips, SZPTR BOOL bTst );
    SwTwips _Grow  ( SwTwips, SZPTR BOOL bTst );
    void    _Invalidate( SwPageFrm *pPage = 0 );

    BOOL FrmSizeChg( const SwFmtFrmSize & );

    SwFlyFrm *GetPrevLink() { return pPrevLink; }
    SwFlyFrm *GetNextLink() { return pNextLink; }

    static void ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );
    static void UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );

    SwFlyFrm *FindChainNeighbour( SwFrmFmt &rFmt, SwFrm *pAnch = 0 );

    // OD 2004-03-24 #i26791#
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

    // OD 16.04.2003 #i13147# - add parameter <_bForPaint> to avoid load of
    // the graphic during paint. Default value: sal_False
    BOOL GetContour( PolyPolygon&   rContour,
                     const sal_Bool _bForPaint = sal_False ) const;

    BOOL ConvertHoriTo40( SwHoriOrient &rHori, SwRelationOrient &rRel, SwTwips &rPos ) const;

    //Auf dieser Shell painten (PreView, Print-Flag usw. rekursiv beachten)?.
    static BOOL IsPaint( SdrObject *pObj, const ViewShell *pSh );

    /** SwFlyFrm::IsBackgroundTransparent - for feature #99657#

        OD 12.08.2002
        determines, if background of fly frame has to be drawn transparent
        definition found in /core/layout/paintfrm.cxx

        @author OD

        @return true, if background color is transparent or a existing background
        graphic is transparent.
    */
    const sal_Bool IsBackgroundTransparent() const;

    /** SwFlyFrm::IsShadowTransparent - for feature #99657#

        OD 05.08.2002
        determine, if shadow color of fly frame has to be drawn transparent
        definition found in /core/layout/paintfrm.cxx

        @author OD

        @return true, if shadow color is transparent.
    */
    const sal_Bool IsShadowTransparent() const;

    // OD 2004-01-19 #110582#
    void Chain( SwFrm* _pAnchor );
    void Unchain();
    void InsertCnt();
    void DeleteCnt();
    // OD 2004-02-12 #110582#-2
    void InsertColumns();

    // OD 2004-03-23 #i26791# - pure virtual methods of base class <SwAnchoredObject>
    virtual void MakeObjPos();
    virtual void InvalidateObjPos();

    virtual SwFrmFmt& GetFrmFmt();
    virtual const SwFrmFmt& GetFrmFmt() const;

    virtual const SwRect GetObjRect() const;
    virtual void SetObjTop( const SwTwips _nTop );
    virtual void SetObjLeft( const SwTwips _nLeft );

    /** method to determine, if a format on the Writer fly frame is possible

        OD 2004-05-11 #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwAnchoredObject::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame is locked resp. col-locked.

        @author OD
    */
    virtual bool IsFormatPossible() const;
};
#endif
