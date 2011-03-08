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


#include <svl/svarray.hxx>

#include "ftnboss.hxx"
#include <tools/mempool.hxx>

#include <SidebarWindowsTypes.hxx>

class SwFlyFrm;
class SwFlyFrmFmt;
class SwPageDesc;
class SwCntntFrm;
struct SwPosition;
struct SwCrsrMoveState;
class SdrObject;
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

class SwPageFrm: public SwFtnBossFrm
{
    friend class SwFrm;

    // #i28701# - use <SwSortedObjs>
    SwSortedObjs *pSortedObjs;

    SwPageDesc *pDesc;      //PageDesc der die Seite beschreibt.

    USHORT  nPhyPageNum;        //Physikalische Seitennummer.

    BOOL bInvalidCntnt      :1;
    BOOL bInvalidLayout     :1;
    BOOL bInvalidFlyCntnt   :1;
    BOOL bInvalidFlyLayout  :1;
    BOOL bInvalidFlyInCnt   :1;
    BOOL bFtnPage           :1; //Diese Seite ist fuer Dokumentende-Fussnoten.
    BOOL bEmptyPage         :1; //Dies ist eine explizite Leerseite
    BOOL bEndNotePage       :1; //'Fussnotenseite' fuer Endnoten
    BOOL bInvalidSpelling   :1; //Das Online-Spelling ist gefordert
    BOOL bInvalidSmartTags :1;  //checking for smarttags is needed  // SMARTTAGS
    BOOL bInvalidAutoCmplWrds :1; //Auto-Complete Wordliste aktualisieren
    BOOL bInvalidWordCount  :1;
    BOOL bHasGrid           :1; // Grid for Asian layout

    // #i28701# - boolean, indicating that layout of page frame
    // is in progress.
    bool mbLayoutInProgress;

    static const sal_Int8 mnShadowPxWidth;

    void _UpdateAttr( SfxPoolItem*, SfxPoolItem*, BYTE &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    // Anpassen der max. Fussnotenhoehen in den einzelnen Spalten
    void SetColMaxFtnHeight();

    /** determine rectangle for right page shadow

        #i9719#

        @param _rPageRect
        input parameter - constant instance reference of the page rectangle.
        Generally, it's the frame area of the page, but for empty pages in print
        preview, this parameter is useful.

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param _orRightShadowRect
        output parameter - instance reference of the right shadow rectangle for
        the given page rectangle
    */
    static void GetRightShadowRect( const SwRect& _rPageRect,
                                    ViewShell*    _pViewShell,
                                    SwRect&       _orRightShadowRect,
                                    bool bRightSidebar );

    /** determine rectangle for bottom page shadow

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

    static void GetBottomShadowRect( const SwRect& _rPageRect,
                                     ViewShell*    _pViewShell,
                                     SwRect&       _orBottomShadowRect,
                                     bool bFullBottomShadow,
                                     bool bRightSidebar );

    /** adds the sidebar used for notes to right and left border
        #i6193#

        @param aRect
        input parameter - current rect, we change borders if we want a sidebar

        @param _pViewShell
        input parameter - instance of the view shell, for which the rectangle
        has to be generated.

        @param bPx
        input parameter - if set to true, we add in pixel
    */
    static void AddSidebarBorders( Rectangle& aRect, ViewShell* _pViewShell, bool bRight, bool bPx = false);
    static void AddSidebarBorders(       SwRect& aRect, ViewShell* _pViewShell, bool bRight, bool bPx = false);

protected:
    virtual void MakeAll();

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwPageFrm)

    SwPageFrm( SwFrmFmt*, SwPageDesc* );
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

    //Spezialisiertes GetCntntPos() fuer Felder in Rahmen.
    void GetCntntPosition( const Point &rPt, SwPosition &rPos ) const;

    BOOL IsEmptyPage() const { return bEmptyPage; } //explizite Leerseite.

    void    UpdateFtnNum();

    //Immer nach dem Paste rufen. Erzeugt die Seitengeb. Rahmen und Formatiert
    //generischen Inhalt.
    void PreparePage( BOOL bFtn );

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

    virtual BOOL GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0 ) const;
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
    virtual void  CheckDirection( BOOL bVert );
    void CheckGrid( BOOL bInvalidate );
    void PaintGrid( OutputDevice* pOut, SwRect &rRect ) const;
    BOOL HasGrid() const { return bHasGrid; }

    //Zeilennummern usw malen
    void RefreshExtraData( const SwRect & ) const;

    //Hilfslinien malen.
    void RefreshSubsidiary( const SwRect& ) const;

    //Fussnotenschnittstelle
    BOOL IsFtnPage() const                                  { return bFtnPage; }
    BOOL IsEndNotePage() const                              { return bEndNotePage; }
    void SetFtnPage( BOOL b )                               { bFtnPage = b; }
    void SetEndNotePage( BOOL b )                           { bEndNotePage = b; }

    inline  USHORT GetPhyPageNum() const        { return nPhyPageNum;}
    inline  void SetPhyPageNum( USHORT nNum )   { nPhyPageNum = nNum;}
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
    inline BOOL IsInvalid() const;
    inline BOOL IsInvalidFly() const;
    BOOL IsInvalidFlyLayout() const { return bInvalidFlyLayout; }
    BOOL IsInvalidFlyCntnt() const { return bInvalidFlyCntnt; }
    BOOL IsInvalidFlyInCnt() const { return bInvalidFlyInCnt; }
    BOOL IsInvalidLayout() const { return bInvalidLayout; }
    BOOL IsInvalidCntnt() const { return (bInvalidCntnt || bInvalidFlyInCnt); }
    BOOL IsInvalidSpelling() const { return bInvalidSpelling; }
    BOOL IsInvalidSmartTags() const { return bInvalidSmartTags; }   // SMARTTAGS
    BOOL IsInvalidAutoCompleteWords() const { return bInvalidAutoCmplWrds; }
    BOOL IsInvalidWordCount() const { return bInvalidWordCount; }

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
                                      ViewShell*    _pViewShell,
                                      bool bPaintRightShadow,
                                      bool bFullBottomShadow,
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
                                             ViewShell*    _pViewShell,
                                             SwRect& _orBorderAndShadowBoundRect,
                                             const bool bRightSidebar );

    static void PaintNotesSidebar(const SwRect& _rPageRect, ViewShell* _pViewShell, USHORT nPageNum, bool bRight);
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
    ((SwPageFrm*)this)->bInvalidFlyLayout = TRUE;
}
inline void SwPageFrm::InvalidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = TRUE;
}
inline void SwPageFrm::InvalidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = TRUE;
}
inline void SwPageFrm::InvalidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = TRUE;
}
inline void SwPageFrm::InvalidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = TRUE;
}
inline void SwPageFrm::InvalidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = TRUE;
}
// SMARTTAGS
inline void    SwPageFrm::InvalidateSmartTags() const
{
   ((SwPageFrm*)this)->bInvalidSmartTags = TRUE;
}
inline void SwPageFrm::InvalidateAutoCompleteWords() const
{
    ((SwPageFrm*)this)->bInvalidAutoCmplWrds = TRUE;
}
inline void SwPageFrm::InvalidateWordCount() const
{
    ((SwPageFrm*)this)->bInvalidWordCount = TRUE;
}
inline void SwPageFrm::ValidateFlyLayout() const
{
    ((SwPageFrm*)this)->bInvalidFlyLayout = FALSE;
}
inline void SwPageFrm::ValidateFlyCntnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyCntnt = FALSE;
}
inline void SwPageFrm::ValidateFlyInCnt() const
{
    ((SwPageFrm*)this)->bInvalidFlyInCnt = FALSE;
}
inline void SwPageFrm::ValidateLayout() const
{
    ((SwPageFrm*)this)->bInvalidLayout = FALSE;
}
inline void SwPageFrm::ValidateCntnt() const
{
    ((SwPageFrm*)this)->bInvalidCntnt = FALSE;
}
inline void SwPageFrm::ValidateSpelling() const
{
    ((SwPageFrm*)this)->bInvalidSpelling = FALSE;
}
// SMARTTAGS
inline void    SwPageFrm::ValidateSmartTags() const
{
   ((SwPageFrm*)this)->bInvalidSmartTags = FALSE;
}
inline void SwPageFrm::ValidateAutoCompleteWords() const
{
    ((SwPageFrm*)this)->bInvalidAutoCmplWrds = FALSE;
}
inline void SwPageFrm::ValidateWordCount() const
{
    ((SwPageFrm*)this)->bInvalidWordCount = FALSE;
}

inline BOOL SwPageFrm::IsInvalid() const
{
    return (bInvalidCntnt || bInvalidLayout || bInvalidFlyInCnt);
}
inline BOOL SwPageFrm::IsInvalidFly() const
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
