/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_CORE_INC_PAGEFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_PAGEFRM_HXX

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
class SwSortedObjs;
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
    sal_Bool bInvalidSmartTags :1;  //checking for smarttags is needed
    sal_Bool bInvalidAutoCmplWrds :1; //Auto-Complete Wordliste aktualisieren
    sal_Bool bInvalidWordCount  :1;
    sal_Bool bHasGrid           :1; // Grid for Asian layout

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
                                     const SwViewShell*    _pViewShell,
                                     SwRect&       _orBottomShadowRect,
                                     bool bPaintLeftShadow,
                                     bool bPaintRightShadow,
                                     bool bRightSidebar );

protected:
    virtual void MakeAll() SAL_OVERRIDE;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwPageFrm)

    SwPageFrm( SwFrmFmt*, SwFrm*, SwPageDesc* );
    ~SwPageFrm();

    //public, damit die SwViewShell beim Umschalten vom BrowseMode darauf
    //zugreifen kann.
    void PrepareHeader();   //Kopf-/Fusszeilen anlegen/entfernen.
    void PrepareFooter();

    const SwSortedObjs  *GetSortedObjs() const  { return pSortedObjs; }
          SwSortedObjs  *GetSortedObjs()          { return pSortedObjs; }

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

    // Appends a fly frame - the given one or a new one - at the page frame.
    // Needed for <Modify> and <MakeFrms>
    // - return value not needed any more
    // - second parameter is of type <SwFlyFrmFmt*>
    // - third parameter only needed for assertion, but calling method assures
    //   this assertion. Thus, delete it.
    void PlaceFly( SwFlyFrm* pFly, SwFlyFrmFmt* pFmt );

    virtual bool GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0, bool bTestBackground = false ) const SAL_OVERRIDE;
        // erfrage vom Client Informationen
    virtual bool GetInfo( SfxPoolItem& ) const SAL_OVERRIDE;

    virtual void Cut() SAL_OVERRIDE;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) SAL_OVERRIDE;
    virtual void CheckDirection( bool bVert ) SAL_OVERRIDE;
    void CheckGrid( sal_Bool bInvalidate );
    void PaintGrid( OutputDevice* pOut, SwRect &rRect ) const;
    sal_Bool HasGrid() const { return bHasGrid; }

    void PaintDecorators( ) const;
    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const SAL_OVERRIDE;
    virtual void PaintBreak() const SAL_OVERRIDE;

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
    inline void ValidateSmartTags() const;
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
    sal_Bool IsInvalidSmartTags() const { return bInvalidSmartTags; }
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
                          SwViewShell* _pViewShell ) const;

    /** paint page border and shadow

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
                                      const SwViewShell*    _pViewShell,
                                      bool bPaintLeftShadow,
                                      bool bPaintRightShadow,
                                      bool bRightSidebar );

    /** get bound rectangle of border and shadow for repaints

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
                                             const SwViewShell*    _pViewShell,
                                             SwRect& _orBorderAndShadowBoundRect,
                                             const bool bLeftShadow,
                                             const bool bRightShadow,
                                             const bool bRightSidebar
                                            );

    static void PaintNotesSidebar(const SwRect& _rPageRect, SwViewShell* _pViewShell, sal_uInt16 nPageNum, bool bRight);
    static void PaintNotesSidebarArrows(const Point &aMiddleFirst, const Point &aMiddleSecond, SwViewShell* _pViewShell, const Color aColorUp, const Color aColorDown);
    /**
        asks the page on which side a margin should be shown, e.g for notes
        returns true for left side, false for right side
    */
    sw::sidebarwindows::SidebarPosition SidebarPosition() const;

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const SAL_OVERRIDE;

    inline sal_Int8 ShadowPxWidth() const
    {
        return mnShadowPxWidth;
    }

    const SwRect PrtWithoutHeaderAndFooter() const;

    // in case this is am empty page, this function returns the 'reference' page
    const SwPageFrm& GetFormatPage() const;

    /// If in header or footer area, it also indicates the exact area in rControl.
    bool IsOverHeaderFooterArea( const Point& rPt, FrameControlType &rControl ) const;

    // return font used to paint the "empty page" string
    static const Font& GetEmptyPageFont();

    static SwTwips GetSidebarBorderWidth( const SwViewShell* );
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

inline void SwPageFrm::InvalidateSmartTags() const
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

inline void SwPageFrm::ValidateSmartTags() const
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


class SwTextGridItem;

SwTextGridItem const* GetGridItem(SwPageFrm const*const);

sal_uInt16 GetGridWidth(SwTextGridItem const&, SwDoc const&);

#endif // INCLUDED_SW_SOURCE_CORE_INC_PAGEFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
