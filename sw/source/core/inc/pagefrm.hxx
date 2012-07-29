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
#ifndef _PAGEFRM_HXX
#define _PAGEFRM_HXX

#include <viewsh.hxx>
#include "ftnboss.hxx"
#include <tools/mempool.hxx>

#include <SidebarWindowsTypes.hxx>

class SwFlyFrm;
class SwFlyFrmFmt;
class SwPageDesc;
class SwCntntFrm;
struct SwPosition;
struct SwCrsrMoveState;
class SwAttrSetChg;
class Font;

// #i28701# - replaced by class <SwSortedObjs>
class SwSortedObjs;
// #i28701#
class SwAnchoredObject;

enum SwPageChg
{
    CHG_NEWPAGE,
    CHG_CUTPAGE,
    CHG_CHGPAGE
};

/// A page of the document layout.
class SwPageFrm: public SwFtnBossFrm
{
    friend class SwFrm;

    // #i28701# - use <SwSortedObjs>
    SwSortedObjs *pSortedObjs;

    SwPageDesc *pDesc;      //PageDesc der die Seite beschreibt.

    sal_uInt16  nPhyPageNum;        //Physikalische Seitennummer.

    sal_Bool bInvalidCntnt      :1;
    sal_Bool bInvalidLayout     :1;
    sal_Bool bInvalidFlyCntnt   :1;
    sal_Bool bInvalidFlyLayout  :1;
    sal_Bool bInvalidFlyInCnt   :1;
    sal_Bool bFtnPage           :1; //Diese Seite ist fuer Dokumentende-Fussnoten.
    sal_Bool bEmptyPage         :1; //Dies ist eine explizite Leerseite
    sal_Bool bEndNotePage       :1; //'Fussnotenseite' fuer Endnoten
    sal_Bool bInvalidSpelling   :1; //Das Online-Spelling ist gefordert
    sal_Bool bInvalidSmartTags :1;  //checking for smarttags is needed  // SMARTTAGS
    sal_Bool bInvalidAutoCmplWrds :1; //Auto-Complete Wordliste aktualisieren
    sal_Bool bInvalidWordCount  :1;
    sal_Bool bHasGrid           :1; // Grid for Asian layout

    // #i28701# - boolean, indicating that layout of page frame
    // is in progress.
    bool mbLayoutInProgress;

    static const sal_Int8 mnShadowPxWidth;

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    // Anpassen der max. Fussnotenhoehen in den einzelnen Spalten
    void SetColMaxFtnHeight();

    /** determine rectangle for horizontal page shadow

        #i9719#

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param _orBottomShadowRect
        output parameter - instance reference of the bottom shadow rectangle for
        the given page rectangle
    */

    static void GetHorizontalShadowRect( const SwRect& _rPageRect,
                                     const ViewShell*    _pViewShell,
                                     SwRect&       _orBottomShadowRect,
                                     bool bPaintLeftShadow,
                                     bool bPaintRightShadow,
                                     bool bRightSidebar );

protected:
    virtual void MakeAll();
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwPageFrm)

    SwPageFrm( SwFrmFmt*, SwFrm*, SwPageDesc* );
    ~SwPageFrm();

    //public, damit die ViewShell beim Umschalten vom BrowseMode darauf
    //zugreifen kann.
    void PrepareHeader();   //Kopf-/Fusszeilen anlegen/entfernen.
    void PrepareFooter();

    const SwSortedObjs  *GetSortedObjs() const  { return pSortedObjs; }
          SwSortedObjs  *GetSortedObjs()          { return pSortedObjs; }

    // #i28701# - new methods to append/remove drawing objects
    void AppendDrawObjToPage( SwAnchoredObject& _rNewObj );
    void RemoveDrawObjFromPage( SwAnchoredObject& _rToRemoveObj );

    void AppendFlyToPage( SwFlyFrm *pNew );
    void RemoveFlyFromPage( SwFlyFrm *pToRemove );
    void MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest );//optimiertes Remove/Append

    void  SetPageDesc( SwPageDesc *, SwFrmFmt * );
          SwPageDesc *GetPageDesc() { return pDesc; }
    const SwPageDesc *GetPageDesc() const { return pDesc; }
          SwPageDesc *FindPageDesc();

                 SwCntntFrm  *FindLastBodyCntnt();
    inline       SwCntntFrm  *FindFirstBodyCntnt();
    inline const SwCntntFrm  *FindFirstBodyCntnt() const;
    inline const SwCntntFrm  *FindLastBodyCntnt() const;

    SwRect GetBoundRect() const;

    //Spezialisiertes GetCntntPos() fuer Felder in Rahmen.
    void GetCntntPosition( const Point &rPt, SwPosition &rPos ) const;

    sal_Bool IsEmptyPage() const { return bEmptyPage; } //explizite Leerseite.

    void    UpdateFtnNum();

    //Immer nach dem Paste rufen. Erzeugt die Seitengeb. Rahmen und Formatiert
    //generischen Inhalt.
    void PreparePage( sal_Bool bFtn );

    //Schickt an alle ContentFrames ein Prepare wg. geaenderter Registervorlage
    void PrepareRegisterChg();

    // #i50432# - adjust method description and synopsis.
    // Appends a fly frame - the given one or a new one - at the page frame.
    // Needed for <Modify> and <MakeFrms>
    // - return value not needed any more
    // - second parameter is of type <SwFlyFrmFmt*>
    // - third parameter only needed for assertion, but calling method assures
    //   this assertion. Thus, delete it.
    void PlaceFly( SwFlyFrm* pFly, SwFlyFrmFmt* pFmt );

    virtual sal_Bool GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0 ) const;
        // erfrage vom Client Informationen
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
    virtual void  CheckDirection( sal_Bool bVert );
    void CheckGrid( sal_Bool bInvalidate );
    void PaintGrid( OutputDevice* pOut, SwRect &rRect ) const;
    sal_Bool HasGrid() const { return bHasGrid; }

    void PaintDecorators( ) const;
    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const;
    virtual void PaintBreak() const;

    //Zeilennummern usw malen
    void RefreshExtraData( const SwRect & ) const;

    //Hilfslinien malen.
    void RefreshSubsidiary( const SwRect& ) const;

    //Fussnotenschnittstelle
    sal_Bool IsFtnPage() const                                  { return bFtnPage; }
    sal_Bool IsEndNotePage() const                              { return bEndNotePage; }
    void SetFtnPage( sal_Bool b )                               { bFtnPage = b; }
    void SetEndNotePage( sal_Bool b )                           { bEndNotePage = b; }

    inline  sal_uInt16 GetPhyPageNum() const        { return nPhyPageNum;}
    inline  void SetPhyPageNum( sal_uInt16 nNum )   { nPhyPageNum = nNum;}
    inline  void DecrPhyPageNum()               { --nPhyPageNum;     }
    inline  void IncrPhyPageNum()               { ++nPhyPageNum;     }

    //Validieren, invalidieren und abfragen des Status der Seite.
    //Layout/Cntnt und jeweils Fly/nicht Fly werden getrennt betrachtet.
    inline void InvalidateFlyLayout() const;
    inline void InvalidateFlyCntnt() const;
    inline void InvalidateFlyInCnt() const;
    inline void InvalidateLayout() const;
    inline void InvalidateCntnt() const;
    inline void InvalidateSpelling() const;
    inline void InvalidateSmartTags() const;
    inline void InvalidateAutoCompleteWords() const;
    inline void InvalidateWordCount() const;
    inline void ValidateFlyLayout() const;
    inline void ValidateFlyCntnt() const;
    inline void ValidateFlyInCnt() const;
    inline void ValidateLayout() const;
    inline void ValidateCntnt() const;
    inline void ValidateSpelling()  const;
    inline void ValidateSmartTags() const;        // SMARTTAGS
    inline void ValidateAutoCompleteWords() const;
    inline void ValidateWordCount() const;
    inline sal_Bool IsInvalid() const;
    inline sal_Bool IsInvalidFly() const;
    sal_Bool IsRightShadowNeeded() const;
    sal_Bool IsLeftShadowNeeded() const;
    sal_Bool IsInvalidFlyLayout() const { return bInvalidFlyLayout; }
    sal_Bool IsInvalidFlyCntnt() const { return bInvalidFlyCntnt; }
    sal_Bool IsInvalidFlyInCnt() const { return bInvalidFlyInCnt; }
    sal_Bool IsInvalidLayout() const { return bInvalidLayout; }
    sal_Bool IsInvalidCntnt() const { return (bInvalidCntnt || bInvalidFlyInCnt); }
    sal_Bool IsInvalidSpelling() const { return bInvalidSpelling; }
    sal_Bool IsInvalidSmartTags() const { return bInvalidSmartTags; }   // SMARTTAGS
    sal_Bool IsInvalidAutoCompleteWords() const { return bInvalidAutoCmplWrds; }
    sal_Bool IsInvalidWordCount() const { return bInvalidWordCount; }

    /** SwPageFrm::GetDrawBackgrdColor

        determine the color, that is respectively will be drawn as background
        for the page frame.

        @return reference to an instance of class Color
    */
    const Color& GetDrawBackgrdColor() const;

    /** paint margin area of a page

        implement paint of margin area; margin area will be painted for a
        view shell with a window and if the document is not in online layout.

        @param _rOutputRect
        input parameter - constant instance reference of the rectangle, for
        which an output has to be generated.

        @param _pViewShell
        input parameter - instance of the view shell, on which the output
        has to be generated.
    */
    void PaintMarginArea( const SwRect& _rOutputRect,
                          ViewShell* _pViewShell ) const;

    /** paint page border and shadow

        #i9719#
        implement paint of page border and shadow

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, on which the output
        has to be generated.

        @param bPaintRightShadow
        Should we paint shadow on the right or not (used in book mode)

        @param bFullBottomShadow
        Should we have a bottom shadow of the same size as the pages or
        not (for right pages in book mode in a LTR environment).

        @param bRightSidebar
        Is the note sidebar on the right or not (used to adjust the
        shadow with & position).
    */
    static void PaintBorderAndShadow( const SwRect& _rPageRect,
                                      const ViewShell*    _pViewShell,
                                      bool bPaintLeftShadow,
                                      bool bPaintRightShadow,
                                      bool bRightSidebar );

    /** get bound rectangle of border and shadow for repaints

        #i9719#

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param _orBorderAndShadowBoundRect
        output parameter - instance reference of the bounded border and shadow
        rectangle for the given page rectangle
    */
    static void GetBorderAndShadowBoundRect( const SwRect& _rPageRect,
                                             const ViewShell*    _pViewShell,
                                             SwRect& _orBorderAndShadowBoundRect,
                                             const bool bLeftShadow,
                                             const bool bRightShadow,
                                             const bool bRightSidebar
                                            );

    static void PaintNotesSidebar(const SwRect& _rPageRect, ViewShell* _pViewShell, sal_uInt16 nPageNum, bool bRight);
    static void PaintNotesSidebarArrows(const Point &aMiddleFirst, const Point &aMiddleSecond, ViewShell* _pViewShell, const Color aColorUp, const Color aColorDown);
    /**
        asks the page on which side a margin should be shown, e.g for notes
        returns true for left side, false for right side
    */
    sw::sidebarwindows::SidebarPosition SidebarPosition() const;

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    inline sal_Int8 ShadowPxWidth() const
    {
        return mnShadowPxWidth;
    }

    const SwRect PrtWithoutHeaderAndFooter() const;

    // #i28701#
    inline bool IsLayoutInProgress() const
    {
        return mbLayoutInProgress;
    }
    inline void SetLayoutInProgress( const bool _bLayoutInProgress )
    {
        mbLayoutInProgress = _bLayoutInProgress;
    }

    // in case this is am empty page, this function returns the 'reference' page
    const SwPageFrm& GetFormatPage() const;

    /// If in header or footer area, it also indicates the exact area in rControl.
    bool IsOverHeaderFooterArea( const Point& rPt, FrameControlType &rControl ) const;

    // return font used to paint the "empty page" string
    static const Font& GetEmptyPageFont();

    static SwTwips GetSidebarBorderWidth( const ViewShell* );
};

inline SwCntntFrm *SwPageFrm::FindFirstBodyCntnt()
{
    SwLayoutFrm *pBody = FindBodyCont();
    return pBody ? pBody->ContainsCntnt() : 0;
}
inline const SwCntntFrm *SwPageFrm::FindFirstBodyCntnt() const
{
    const SwLayoutFrm *pBody = FindBodyCont();
    return pBody ? pBody->ContainsCntnt() : 0;
}
inline const SwCntntFrm *SwPageFrm::FindLastBodyCntnt() const
{
    return ((SwPageFrm*)this)->FindLastBodyCntnt();
}
inline void SwPageFrm::InvalidateFlyLayout() const
{
    ((SwPageFrm*)this)->bInvalidFlyLayout = sal_True;
}
inline void SwPageFrm::InvalidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = sal_True;
}
inline void SwPageFrm::InvalidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = sal_True;
}
inline void SwPageFrm::InvalidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = sal_True;
}
inline void SwPageFrm::InvalidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = sal_True;
}
inline void SwPageFrm::InvalidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = sal_True;
}
// SMARTTAGS
inline void    SwPageFrm::InvalidateSmartTags() const
{
   ((SwPageFrm*)this)->bInvalidSmartTags = sal_True;
}
inline void SwPageFrm::InvalidateAutoCompleteWords() const
{
    ((SwPageFrm*)this)->bInvalidAutoCmplWrds = sal_True;
}
inline void SwPageFrm::InvalidateWordCount() const
{
    ((SwPageFrm*)this)->bInvalidWordCount = sal_True;
}
inline void SwPageFrm::ValidateFlyLayout() const
{
    ((SwPageFrm*)this)->bInvalidFlyLayout = sal_False;
}
inline void SwPageFrm::ValidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = sal_False;
}
inline void SwPageFrm::ValidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = sal_False;
}
inline void SwPageFrm::ValidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = sal_False;
}
inline void SwPageFrm::ValidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = sal_False;
}
inline void SwPageFrm::ValidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = sal_False;
}
// SMARTTAGS
inline void    SwPageFrm::ValidateSmartTags() const
{
   ((SwPageFrm*)this)->bInvalidSmartTags = sal_False;
}
inline void SwPageFrm::ValidateAutoCompleteWords() const
{
    ((SwPageFrm*)this)->bInvalidAutoCmplWrds = sal_False;
}
inline void SwPageFrm::ValidateWordCount() const
{
    ((SwPageFrm*)this)->bInvalidWordCount = sal_False;
}

inline sal_Bool SwPageFrm::IsInvalid() const
{
    return (bInvalidCntnt || bInvalidLayout || bInvalidFlyInCnt);
}
inline sal_Bool SwPageFrm::IsInvalidFly() const
{
    return bInvalidFlyLayout || bInvalidFlyCntnt;
}

#define GETGRID( pPage ) const SwTextGridItem *pGrid = NULL; \
 {if( pPage && pPage->HasGrid() && GRID_NONE==(pGrid=(SwTextGridItem*)&pPage->\
     GetPageDesc()->GetMaster().GetFmtAttr(RES_TEXTGRID))->GetGridType() ) \
    pGrid = NULL;}

#define GETGRIDWIDTH( pGrid , pDoc )  pDoc->IsSquaredPageMode() ? \
    pGrid->GetBaseHeight(): pGrid->GetBaseWidth()



#endif  //_PAGEFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
