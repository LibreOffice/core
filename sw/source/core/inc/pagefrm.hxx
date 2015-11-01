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
class SwFlyFrameFormat;
class SwPageDesc;
class SwContentFrm;
struct SwPosition;
struct SwCrsrMoveState;
class SwAttrSetChg;
namespace vcl { class Font; }
class SwSortedObjs;
class SwAnchoredObject;

enum SwPageChg
{
    CHG_NEWPAGE,
    CHG_CUTPAGE,
    CHG_CHGPAGE
};

/// A page of the document layout.
class SwPageFrm: public SwFootnoteBossFrm
{
    friend class SwFrm;

    SwSortedObjs *m_pSortedObjs;

    SwPageDesc *m_pDesc; //PageDesc that describes the Page

    sal_uInt16  m_nPhyPageNum; // Physical page number

    bool m_bInvalidContent      :1;
    bool m_bInvalidLayout       :1;
    bool m_bInvalidFlyContent   :1;
    bool m_bInvalidFlyLayout    :1;
    bool m_bInvalidFlyInCnt     :1;
    bool m_bFootnotePage        :1; // This Page is for document end footnotes
    bool m_bEmptyPage           :1; // This Page is an explicitly empty page
    bool m_bEndNotePage         :1; // 'Footnote page' for end notes
    bool m_bInvalidSpelling     :1; // We need online spelling
    bool m_bInvalidSmartTags    :1; // We need checking for smarttags
    bool m_bInvalidAutoCmplWrds :1; // Update auto complete word list
    bool m_bInvalidWordCount    :1;
    bool m_bHasGrid             :1; // Grid for Asian layout

    static const sal_Int8 mnShadowPxWidth;

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    /// Adapt the max. footnote height in each single column
    void SetColMaxFootnoteHeight();

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
                                     OutputDevice* pRenderContext,
                                     SwRect&       _orBottomShadowRect,
                                     bool bPaintLeftShadow,
                                     bool bPaintRightShadow,
                                     bool bRightSidebar );

    virtual void DestroyImpl() override;
    virtual ~SwPageFrm();

protected:
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwPageFrm)

    SwPageFrm( SwFrameFormat*, SwFrm*, SwPageDesc* );

    /// Make this public, so that the SwViewShell can access it when switching from browse mode
    /// Add/remove header/footer
    void PrepareHeader();
    void PrepareFooter();

    const SwSortedObjs  *GetSortedObjs() const  { return m_pSortedObjs; }
          SwSortedObjs  *GetSortedObjs()          { return m_pSortedObjs; }

    void AppendDrawObjToPage( SwAnchoredObject& _rNewObj );
    void RemoveDrawObjFromPage( SwAnchoredObject& _rToRemoveObj );

    void AppendFlyToPage( SwFlyFrm *pNew );
    void RemoveFlyFromPage( SwFlyFrm *pToRemove );
    void MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest ); // Optimized Remove/Append

    void  SetPageDesc( SwPageDesc *, SwFrameFormat * );
          SwPageDesc *GetPageDesc() { return m_pDesc; }
    const SwPageDesc *GetPageDesc() const { return m_pDesc; }
          SwPageDesc *FindPageDesc();

                 SwContentFrm  *FindLastBodyContent();
    inline       SwContentFrm  *FindFirstBodyContent();
    inline const SwContentFrm  *FindFirstBodyContent() const;
    inline const SwContentFrm  *FindLastBodyContent() const;

    SwRect GetBoundRect(OutputDevice* pOutputDevice) const;

    // Specialized GetContentPos() for Field in Frames
    void GetContentPosition( const Point &rPt, SwPosition &rPos ) const;

    bool IsEmptyPage() const { return m_bEmptyPage; } // Explicitly empty page

    void    UpdateFootnoteNum();

    /// Always call after Paste
    /// Creates the page-bound frames and formats the generic content
    void PreparePage( bool bFootnote );

    // Sends a Prepare() to all ContentFrames caused by a changed register template
    void PrepareRegisterChg();

    // Appends a fly frame - the given one or a new one - at the page frame.
    // Needed for <Modify> and <MakeFrms>
    // - return value not needed any more
    // - second parameter is of type <SwFlyFrameFormat*>
    // - third parameter only needed for assertion, but calling method assures
    //   this assertion. Thus, delete it.
    void PlaceFly( SwFlyFrm* pFly, SwFlyFrameFormat* pFormat );

    virtual bool GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0, bool bTestBackground = false ) const override;
    /// Get info from Client
    virtual bool GetInfo( SfxPoolItem& ) const override;

    virtual void Cut() override;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) override;
    virtual void CheckDirection( bool bVert ) override;
    void CheckGrid( bool bInvalidate );
    void PaintGrid( OutputDevice* pOut, SwRect &rRect ) const;
    bool HasGrid() const { return m_bHasGrid; }

    void PaintDecorators( ) const;
    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const override;
    virtual void PaintBreak() const override;

    /// Paint line number etc.
    void RefreshExtraData( const SwRect & ) const;

    /// Paint helper lines
    void RefreshSubsidiary( const SwRect& ) const;

    /// Foot note interface
    bool IsFootnotePage() const                          { return m_bFootnotePage; }
    bool IsEndNotePage() const                      { return m_bEndNotePage; }
    void SetFootnotePage( bool b )                       { m_bFootnotePage = b; }
    void SetEndNotePage( bool b )                   { m_bEndNotePage = b; }

    inline  sal_uInt16 GetPhyPageNum() const        { return m_nPhyPageNum;}
    inline  void SetPhyPageNum( sal_uInt16 nNum )   { m_nPhyPageNum = nNum;}
    inline  void DecrPhyPageNum()               { --m_nPhyPageNum;     }
    inline  void IncrPhyPageNum()               { ++m_nPhyPageNum;     }

    /// Validate, invalidate and query the Page status
    /// Layout/Content and Fly/non-Fly respectively are inspected separately
    inline void InvalidateFlyLayout() const;
    inline void InvalidateFlyContent() const;
    inline void InvalidateFlyInCnt() const;
    inline void InvalidateLayout() const;
    inline void InvalidateContent() const;
    inline void InvalidateSpelling() const;
    inline void InvalidateSmartTags() const;
    inline void InvalidateAutoCompleteWords() const;
    inline void InvalidateWordCount() const;
    inline void ValidateFlyLayout() const;
    inline void ValidateFlyContent() const;
    inline void ValidateFlyInCnt() const;
    inline void ValidateLayout() const;
    inline void ValidateContent() const;
    inline void ValidateSpelling()  const;
    inline void ValidateSmartTags() const;
    inline void ValidateAutoCompleteWords() const;
    inline void ValidateWordCount() const;
    inline bool IsInvalid() const;
    inline bool IsInvalidFly() const;
    bool IsRightShadowNeeded() const;
    bool IsLeftShadowNeeded() const;
    bool IsInvalidFlyLayout() const { return m_bInvalidFlyLayout; }
    bool IsInvalidFlyContent() const { return m_bInvalidFlyContent; }
    bool IsInvalidFlyInCnt() const { return m_bInvalidFlyInCnt; }
    bool IsInvalidLayout() const { return m_bInvalidLayout; }
    bool IsInvalidContent() const { return (m_bInvalidContent || m_bInvalidFlyInCnt); }
    bool IsInvalidSpelling() const { return m_bInvalidSpelling; }
    bool IsInvalidSmartTags() const { return m_bInvalidSmartTags; }
    bool IsInvalidAutoCompleteWords() const { return m_bInvalidAutoCmplWrds; }
    bool IsInvalidWordCount() const { return m_bInvalidWordCount; }

    /** SwPageFrm::GetDrawBackgrdColor

        determine the color, that is respectively will be drawn as background
        for the page frame.

        @return reference to an instance of class Color
    */
    const Color GetDrawBackgrdColor() const;

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
                                             OutputDevice* pRenderContext,
                                             SwRect& _orBorderAndShadowBoundRect,
                                             const bool bLeftShadow,
                                             const bool bRightShadow,
                                             const bool bRightSidebar
                                            );

    static void PaintNotesSidebar(const SwRect& _rPageRect, SwViewShell* _pViewShell, sal_uInt16 nPageNum, bool bRight);
    static void PaintNotesSidebarArrows(const Point &rMiddleFirst, const Point &rMiddleSecond, SwViewShell* _pViewShell, const Color& rColorUp, const Color& rColorDown);
    /**
        asks the page on which side a margin should be shown, e.g for notes
        returns true for left side, false for right side
    */
    sw::sidebarwindows::SidebarPosition SidebarPosition() const;

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const override;

    const SwRect PrtWithoutHeaderAndFooter() const;

    // in case this is am empty page, this function returns the 'reference' page
    const SwPageFrm& GetFormatPage() const;

    /// If in header or footer area, it also indicates the exact area in rControl.
    /// Header or footer must be active, otherwise returns false.
    bool IsOverHeaderFooterArea( const Point& rPt, FrameControlType &rControl ) const;

    // return font used to paint the "empty page" string
    static const vcl::Font& GetEmptyPageFont();

    static SwTwips GetSidebarBorderWidth( const SwViewShell* );
};

inline SwContentFrm *SwPageFrm::FindFirstBodyContent()
{
    SwLayoutFrm *pBody = FindBodyCont();
    return pBody ? pBody->ContainsContent() : 0;
}
inline const SwContentFrm *SwPageFrm::FindFirstBodyContent() const
{
    const SwLayoutFrm *pBody = FindBodyCont();
    return pBody ? pBody->ContainsContent() : 0;
}
inline const SwContentFrm *SwPageFrm::FindLastBodyContent() const
{
    return const_cast<SwPageFrm*>(this)->FindLastBodyContent();
}
inline void SwPageFrm::InvalidateFlyLayout() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidFlyLayout = true;
}
inline void SwPageFrm::InvalidateFlyContent() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidFlyContent = true;
}
inline void SwPageFrm::InvalidateFlyInCnt() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidFlyInCnt = true;
}
inline void SwPageFrm::InvalidateLayout() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidLayout = true;
}
inline void SwPageFrm::InvalidateContent() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidContent = true;
}
inline void SwPageFrm::InvalidateSpelling() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidSpelling = true;
}

inline void SwPageFrm::InvalidateSmartTags() const
{
   const_cast<SwPageFrm*>(this)->m_bInvalidSmartTags = true;
}
inline void SwPageFrm::InvalidateAutoCompleteWords() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidAutoCmplWrds = true;
}
inline void SwPageFrm::InvalidateWordCount() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidWordCount = true;
}
inline void SwPageFrm::ValidateFlyLayout() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidFlyLayout = false;
}
inline void SwPageFrm::ValidateFlyContent() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidFlyContent = false;
}
inline void SwPageFrm::ValidateFlyInCnt() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidFlyInCnt = false;
}
inline void SwPageFrm::ValidateLayout() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidLayout = false;
}
inline void SwPageFrm::ValidateContent() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidContent = false;
}
inline void SwPageFrm::ValidateSpelling() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidSpelling = false;
}

inline void SwPageFrm::ValidateSmartTags() const
{
   const_cast<SwPageFrm*>(this)->m_bInvalidSmartTags = false;
}
inline void SwPageFrm::ValidateAutoCompleteWords() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidAutoCmplWrds = false;
}
inline void SwPageFrm::ValidateWordCount() const
{
    const_cast<SwPageFrm*>(this)->m_bInvalidWordCount = false;
}

inline bool SwPageFrm::IsInvalid() const
{
    return (m_bInvalidContent || m_bInvalidLayout || m_bInvalidFlyInCnt);
}
inline bool SwPageFrm::IsInvalidFly() const
{
    return m_bInvalidFlyLayout || m_bInvalidFlyContent;
}


class SwTextGridItem;

SwTextGridItem const* GetGridItem(SwPageFrm const*const);

sal_uInt16 GetGridWidth(SwTextGridItem const&, SwDoc const&);

#endif // INCLUDED_SW_SOURCE_CORE_INC_PAGEFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
