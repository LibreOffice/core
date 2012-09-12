/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef SW_FLYFRM_HXX
#define SW_FLYFRM_HXX

#include "layfrm.hxx"
#include <list>
#include "frmfmt.hxx"

class SwPageFrm;
class SwFmtFrmSize;
struct SwCrsrMoveState;
class SwBorderAttrs;
class SwVirtFlyDrawObj;
class SwSpzFrmFmts;
class SwAttrSetChg;
class PolyPolygon;
class SwFlyDrawContact;
class SwDrawContact;
class SwFmt;

#include <anchoredobject.hxx>

//Sucht ausgehend von pOldAnch einen Anker fuer Absatzgebundene Rahmen.
//Wird beim Draggen von Absatzgebundenen Objekten zur Ankeranzeige sowie
//fuer Ankerwechsel benoetigt.
//implementiert in layout/flycnt.cxx
const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const sal_Bool bBody = sal_False );

// berechnet das Rechteck, in dem das Objekt bewegt bzw. resized werden darf
sal_Bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, sal_Bool bMove = sal_True );

//allg. Basisklasse fuer alle Freifliegenden Rahmen
// OD 2004-03-22 #i26791# - inherit also from <SwAnchoredFlyFrm>
class SwFlyFrm : public SwLayoutFrm, public SwAnchoredObject
{
    //darf Locken. Definiert in frmtool.cxx
    friend void AppendObjs   ( const SwSpzFrmFmts *, sal_uLong, SwFrm *, SwPageFrm * );
    friend void Notify( SwFlyFrm *, SwPageFrm *pOld, const SwRect &rOld,
                        const SwRect* pOldPrt );

    void InitDrawObj( sal_Bool bNotify );   //Wird von den CToren gerufen.
    void FinitDrawObj();                //Wird vom CTor gerufen.

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    using SwLayoutFrm::CalcRel;

    sal_uInt32 _GetOrdNumForNewRef( const SwFlyDrawContact* );
    SwVirtFlyDrawObj* CreateNewRef( SwFlyDrawContact* );

protected:

    SwFlyFrm *pPrevLink,        // Vorgaenger/Nachfolger fuer Verkettung mit
             *pNextLink;        // Textfluss

    // OD 2004-05-27 #i26791# - moved to <SwAnchoredObject>
//    Point aRelPos;   //Die Relative Position zum Master

private:
    sal_Bool bLocked    :1; //Cntnt-gebundene Flys muessen derart blockiert werden
                        //koennen, dass sie nicht Formatiert werden; :MakeAll
                        //returnt dann sofort. Dies ist bei Seitenwechseln
                        //waehrend der Formatierung notwendig.
                        //Auch wahrend des RootCTors ist dies notwendig da
                        //sonst der Anker formatiert wird obwohl die Root noch
                        //nicht korrekt an der Shell haengt und weil sonst
                        //initial zuviel Formatiert wuerde.
    sal_Bool bNotifyBack:1; //sal_True wenn am Ende eines MakeAll() der Background
                        //vom NotifyDTor benachrichtigt werden muss.
protected:

    sal_Bool bInvalid :1;   //Pos, PrtArea od. SSize wurden Invalidiert, sie werden
                        //gleich wieder Validiert, denn sie muessen _immer_
                        //gueltig sein. Damit in LayAction korrekt gearbeitet
                        //werden kann muss hier festgehalten werden, dass sie
                        //invalidiert wurden. Ausnahmen bestaetigen die Regelt!
    sal_Bool bMinHeight:1;  //sal_True wenn die vom Attribut vorgegebene Hoehe eine
                        //eine Minimalhoehe ist (der Frm also bei Bedarf
                        //darueberhinaus wachsen kann).
    sal_Bool bHeightClipped :1; //sal_True wenn der Fly nicht die Pos/Size anhand der Attrs
    sal_Bool bWidthClipped  :1; //formatieren konnte, weil z.B. nicht genug Raum vorh.
                            //war.
    sal_Bool bFormatHeightOnly  :1; //Damit nach einer Anpassung der Breite
                                //(CheckClip) nur das Format aufgerufen wird;
                                //nicht aber die Breite anhand der Attribute
                                //wieder bestimmt wird.
    sal_Bool bInCnt         :1; // FLY_AS_CHAR, anchored as character
    sal_Bool bAtCnt         :1; // FLY_AT_PARA, anchored at paragraph
    sal_Bool bLayout        :1; // FLY_AT_PAGE, FLY_AT_FLY, at page or at frame
    sal_Bool bAutoPosition  :1; // FLY_AT_CHAR, anchored at character
    sal_Bool bNoShrink      :1; // temporary forbud of shrinking to avoid loops
    sal_Bool bLockDeleteContent :1;           // If the flag is set, the content of the
                                        // fly frame is not deleted if moved to
                                        // invisible layer.

    friend class SwNoTxtFrm; // Darf NotifyBackground rufen
//    virtual void NotifyBackground( SwPageFrm *pPage,
//                                   const SwRect& rRect, PrepareHint eHint) = 0;

    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    void MakePrtArea( const SwBorderAttrs &rAttrs );

    void Lock()         { bLocked = sal_True; }
    void Unlock()       { bLocked = sal_False; }

    void SetMinHeight()  { bMinHeight = sal_True; }
    void ResetMinHeight(){ bMinHeight = sal_False; }

    Size CalcRel( const SwFmtFrmSize &rSz ) const;
    SwTwips CalcAutoWidth() const;

    SwFlyFrm( SwFlyFrmFmt*, SwFrm*, SwFrm *pAnchor );

    /** method to assure that anchored object is registered at the correct
        page frame

        OD 2004-07-02 #i28701#

        @author OD
    */
    virtual void RegisterAtCorrectPage();

    virtual bool _SetObjTop( const SwTwips _nTop );
    virtual bool _SetObjLeft( const SwTwips _nLeft );

    virtual const SwRect GetObjBoundRect() const;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
//Bug 120881:Modify here for Directly Page Numbering
    sal_Bool IsPageNumberingFrm();
//Bug 120881(End)
    // OD 2004-03-23 #i26791#
    TYPEINFO();

    virtual ~SwFlyFrm();
        // erfrage vom Client Informationen
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual Size ChgSize( const Size& aNewSize );
    virtual sal_Bool GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0 ) const;

    virtual void  CheckDirection( sal_Bool bVert );
    virtual void Cut();
#ifdef DBG_UTIL
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

    SwTwips _Shrink( SwTwips, sal_Bool bTst );
    SwTwips _Grow  ( SwTwips, sal_Bool bTst );
    void    _Invalidate( SwPageFrm *pPage = 0 );

    sal_Bool FrmSizeChg( const SwFmtFrmSize & );

    SwFlyFrm *GetPrevLink() const { return pPrevLink; }
    SwFlyFrm *GetNextLink() const { return pNextLink; }

    static void ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );
    static void UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );

    SwFlyFrm *FindChainNeighbour( SwFrmFmt &rFmt, SwFrm *pAnch = 0 );

    // OD 2004-03-24 #i26791#
    const SwVirtFlyDrawObj* GetVirtDrawObj() const;
    SwVirtFlyDrawObj *GetVirtDrawObj();
    void NotifyDrawObj();

    void ChgRelPos( const Point &rAbsPos );
    sal_Bool IsInvalid() const { return bInvalid; }
    void Invalidate() const { ((SwFlyFrm*)this)->bInvalid = sal_True; }
    void Validate() const { ((SwFlyFrm*)this)->bInvalid = sal_False; }

    sal_Bool IsMinHeight()  const { return bMinHeight; }
    sal_Bool IsLocked()     const { return bLocked; }
    sal_Bool IsAutoPos()    const { return bAutoPosition; }
    sal_Bool IsFlyInCntFrm() const { return bInCnt; }
    sal_Bool IsFlyFreeFrm() const { return bAtCnt || bLayout; }
    sal_Bool IsFlyLayFrm() const { return bLayout; }
    sal_Bool IsFlyAtCntFrm() const { return bAtCnt; }

    sal_Bool IsNotifyBack() const { return bNotifyBack; }
    void SetNotifyBack()      { bNotifyBack = sal_True; }
    void ResetNotifyBack()    { bNotifyBack = sal_False; }
    sal_Bool IsNoShrink()   const { return bNoShrink; }
    void SetNoShrink( sal_Bool bNew ) { bNoShrink = bNew; }
    sal_Bool IsLockDeleteContent()  const { return bLockDeleteContent; }
    void SetLockDeleteContent( sal_Bool bNew ) { bLockDeleteContent = bNew; }


    sal_Bool IsClipped()        const   { return bHeightClipped || bWidthClipped; }
    sal_Bool IsHeightClipped()  const   { return bHeightClipped; }
    sal_Bool IsWidthClipped()   const   { return bWidthClipped;  }

    sal_Bool IsLowerOf( const SwLayoutFrm* pUpper ) const;
    inline sal_Bool IsUpperOf( const SwFlyFrm& _rLower ) const
    {
        return _rLower.IsLowerOf( this );
    }

    SwFrm *FindLastLower();

    // OD 16.04.2003 #i13147# - add parameter <_bForPaint> to avoid load of
    // the graphic during paint. Default value: sal_False
    sal_Bool GetContour( PolyPolygon&   rContour,
                     const sal_Bool _bForPaint = sal_False ) const;


    //Auf dieser Shell painten (PreView, Print-Flag usw. rekursiv beachten)?.
    static sal_Bool IsPaint( SdrObject *pObj, const ViewShell *pSh );

    /** SwFlyFrm::IsBackgroundTransparent - for feature #99657#

        OD 12.08.2002
        determines, if background of fly frame has to be drawn transparent
        definition found in /core/layout/paintfrm.cxx

        @author OD

        @return true, if background color is transparent or a existing background
        graphic is transparent.
    */
    sal_Bool IsBackgroundTransparent() const;

    /** SwFlyFrm::IsShadowTransparent - for feature #99657#

        OD 05.08.2002
        determine, if shadow color of fly frame has to be drawn transparent
        definition found in /core/layout/paintfrm.cxx

        @author OD

        @return true, if shadow color is transparent.
    */
    sal_Bool IsShadowTransparent() const;

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

    /** method to determine, if a format on the Writer fly frame is possible

        OD 2004-05-11 #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwAnchoredObject::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame is locked resp. col-locked.

        @author OD
    */
    virtual bool IsFormatPossible() const;
    static void GetAnchoredObjects( std::list<SwAnchoredObject*>&, const SwFmt& rFmt );

    // overwriting "SwFrmFmt *SwLayoutFrm::GetFmt" to provide the correct derived return type.
    // (This is in order to skip on the otherwise necessary casting of the result to
    // 'SwFlyFrmFmt *' after calls to this function. The casting is now done in this function.)
    virtual const SwFlyFrmFmt *GetFmt() const;
    virtual       SwFlyFrmFmt *GetFmt();
};
#endif
