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
#ifndef _SWPVIEW_HXX
#define _SWPVIEW_HXX

#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/zoomitem.hxx>
#include "swdllapi.h"
#include "shellid.hxx"

class SwViewOption;
class SwDocShell;
class SwScrollbar;
class ViewShell;
class SwPagePreView;
class ImageButton;
class Button;
class SwRect;
class DataChangedEvent;
class CommandEvent;
class SvtAccessibilityOptions;
class SwPagePreviewLayout;

// Delete member <mnVirtPage> and its accessor
class SwPagePreViewWin : public Window
{
    ViewShell*          mpViewShell;
    sal_uInt16              mnSttPage;
    sal_uInt8                mnRow, mnCol;
    Size                maPxWinSize;
    Fraction            maScale;
    SwPagePreView&      mrView;
    bool                mbCalcScaleForPreviewLayout;
    Rectangle           maPaintedPreviewDocRect;
    SwPagePreviewLayout* mpPgPrevwLayout;

    void SetPagePreview( sal_uInt8 nRow, sal_uInt8 nCol );

    using Window::Scroll;

public:
    SwPagePreViewWin( Window* pParent, SwPagePreView& rView );
    ~SwPagePreViewWin();

    // calls ViewShell::Paint
    virtual void Paint( const Rectangle& rRect );
    virtual void KeyInput( const KeyEvent & );
    virtual void Command( const CommandEvent& rCEvt );
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void DataChanged( const DataChangedEvent& );

    void SetViewShell( ViewShell* pShell );

    ViewShell* GetViewShell() const { return mpViewShell; }

    sal_uInt8    GetRow() const      { return mnRow; }
    void    SetRow( sal_uInt8 n )    { if( n ) mnRow = n; }

    sal_uInt8    GetCol() const      { return mnCol; }
    void    SetCol( sal_uInt8 n )    { if( n ) mnCol = n; }

    sal_uInt16  GetSttPage() const      { return mnSttPage; }
    void    SetSttPage( sal_uInt16 n )  { mnSttPage = n; }

    /** get selected page number of document preview

        @return selected page number
    */
    sal_uInt16 SelectedPage() const;

    /** set selected page number in document preview

        @param _nSelectedPageNum
        input parameter - physical page number of page that will be the selected one.
    */
    void SetSelectedPage( sal_uInt16 _nSelectedPageNum );

    // If we only have one column we do not have a oth page
    sal_uInt16  GetDefSttPage() const   { return 1 == mnCol ? 1 : 0; }

    void CalcWish( sal_uInt8 nNewRow, sal_uInt8 nNewCol );

    const Size& GetWinSize() const  { return maPxWinSize; }
    void SetWinSize( const Size& rNewSize );

    // Add <MV_SELPAGE>, <MV_SCROLL>
    enum MoveMode{ MV_CALC, MV_PAGE_UP, MV_PAGE_DOWN, MV_DOC_STT, MV_DOC_END,
                   MV_SELPAGE, MV_SCROLL, MV_NEWWINSIZE };
    int MovePage( int eMoveMode );

    // Create the status bar's string
    OUString GetStatusStr( sal_uInt16 nPageCount ) const;

    void RepaintCoreRect( const SwRect& rRect );

    /** Method to adjust preview to a new zoom factor
        paint of preview is prepared for a new zoom factor
        Zoom type has also been considered.
        Thus, add new parameter <_eZoomType>
    */
    void AdjustPreviewToNewZoom( const sal_uInt16 _nZoomFactor,
                                 const SvxZoomType _eZoomType );

    const Rectangle& GetPaintedPreviewDocRect() const
    {
        return maPaintedPreviewDocRect;
    }

    void Scroll(long nXMove, long nYMove, sal_uInt16 nFlags = 0);

    /** Method to enable/disable book preview
        @param _bBookPreview
        input parameter - boolean indicating, if book preview mode has to
        switch on <sal_True> or of <sal_False>

        @return boolean indicating, if book preview mode has changed.
    */
    bool SetBookPreviewMode( const bool _bBookPreview );

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
                    CreateAccessible();
};

/**
 * View of a document
 */
class SW_DLLPUBLIC SwPagePreView: public SfxViewShell
{
    // ViewWindow and handle to core
    // current dispatcher shell
    SwPagePreViewWin        aViewWin;
    //viewdata of the previous SwView and the new crsrposition
    OUString                sSwViewData;
    //and the new cursor position if the user double click in the PagePreView
    String                  sNewCrsrPos;
    // to support keyboard the number of the page to go to can be set too
    sal_uInt16                  nNewPage;
    // visible range
    OUString                sPageStr;
    Size                    aDocSz;
    Rectangle               aVisArea;

    // MDI control elements
    SwScrollbar             *pHScrollbar;
    SwScrollbar             *pVScrollbar;
    bool                    mbHScrollbarEnabled;
    bool                    mbVScrollbarEnabled;
    ImageButton             *pPageUpBtn,
                            *pPageDownBtn;
    // dummy window for filling the lower right edge when both scrollbars are active
    Window                  *pScrollFill;

    sal_uInt16                  mnPageCount;
    sal_Bool                    bNormalPrint;

    // New members to reset design mode at draw view for form shell on switching
    // back from writer page preview to normal view.
    sal_Bool                mbResetFormDesignMode:1;
    sal_Bool                mbFormDesignModeToReset:1;

    SW_DLLPRIVATE void            Init(const SwViewOption* = 0);
    SW_DLLPRIVATE Point           AlignToPixel(const Point& rPt) const;

    SW_DLLPRIVATE int             _CreateScrollbar( sal_Bool bHori );
    SW_DLLPRIVATE DECL_LINK( ScrollHdl, SwScrollbar * );
    SW_DLLPRIVATE DECL_LINK( EndScrollHdl, SwScrollbar * );
    SW_DLLPRIVATE DECL_LINK( BtnPage, Button * );
    SW_DLLPRIVATE int             ChgPage( int eMvMode, int bUpdateScrollbar = sal_True );


    SW_DLLPRIVATE virtual SfxPrinter*     GetPrinter( sal_Bool bCreate = sal_False );
    SW_DLLPRIVATE virtual sal_uInt16      SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false );
    SW_DLLPRIVATE virtual bool            HasPrintOptionsPage() const;
    SW_DLLPRIVATE virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent,
                                                const SfxItemSet &rOptions );

    SW_DLLPRIVATE void CalcAndSetBorderPixel( SvBorder &rToFill, sal_Bool bInner );

    /** Helper method to execute SfxRequest FN_PAGE_UP and FN_PAGE_DOWN

        @param _bPgUp
        input parameter - boolean that indicates, if FN_PAGE_UP or FN_PAGE_DOWN
        has to be executed.

        @param _pReq
        optional input parameter - pointer to the <SfxRequest> instance, if existing.
    */
    SW_DLLPRIVATE void _ExecPgUpAndPgDown( const bool  _bPgUp,
                             SfxRequest* _pReq = 0 );

protected:
    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );

    void         SetZoom(SvxZoomType eSet, sal_uInt16 nFactor);

public:
    SFX_DECL_VIEWFACTORY(SwPagePreView);
    SFX_DECL_INTERFACE(SW_PAGEPREVIEW)
    TYPEINFO();

    inline Window*          GetFrameWindow() const { return &(GetViewFrame())->GetWindow(); }
    inline ViewShell*       GetViewShell() const { return aViewWin.GetViewShell(); }
    inline const Rectangle& GetVisArea() const { return aVisArea; }
    inline void             GrabFocusViewWin() { aViewWin.GrabFocus(); }
    inline void             RepaintCoreRect( const SwRect& rRect )
                                { aViewWin.RepaintCoreRect( rRect ); }

    void            DocSzChgd(const Size& rNewSize);
    const Size&     GetDocSz() const { return aDocSz; }

    virtual void    SetVisArea( const Rectangle&, sal_Bool bUpdateScrollbar = sal_True);

    inline void     AdjustEditWin();

    void            ScrollViewSzChg();
    void            ScrollDocSzChg();
    void            ShowHScrollbar(sal_Bool bShow);
    void            ShowVScrollbar(sal_Bool bShow);
    void            EnableHScrollbar(bool bEnable);
    void            EnableVScrollbar(bool bEnable);

    sal_uInt16          GetPageCount() const        { return mnPageCount; }
    sal_uInt16      GetSelectedPage() const {return aViewWin.SelectedPage();}

    sal_Bool            HandleWheelCommands( const CommandEvent& );

    OUString        GetPrevSwViewData() const       { return sSwViewData; }
    void            SetNewCrsrPos( const String& rStr ) { sNewCrsrPos = rStr; }
    const String&   GetNewCrsrPos() const           { return sNewCrsrPos; }

    sal_uInt16          GetNewPage() const {return nNewPage;}
    void            SetNewPage(sal_uInt16 nSet)  {nNewPage = nSet;}

    // Handler
    void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&);
    void            StateUndo(SfxItemSet&);

    SwDocShell*     GetDocShell();

    // apply Accessiblity options
    void ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions);

    // Inline method to request values of new members
    // <mbResetFormDesignMode> and <mbFormDesignModeToReset>
    inline sal_Bool ResetFormDesignMode() const
    {
        return mbResetFormDesignMode;
    }

    inline sal_Bool FormDesignModeToReset() const
    {
        return mbFormDesignModeToReset;
    }

    /** Adjust position of vertical scrollbar

        Currently used, if the complete preview layout rows fit into to the given
        window, if a new page is selected and this page is visible.

        @param _nNewThumbPos
        input parameter - new position, which will be assigned to the vertical
        scrollbar.
    */
    void SetVScrollbarThumbPos( const sal_uInt16 _nNewThumbPos );

    SwPagePreView( SfxViewFrame* pFrame, SfxViewShell* );
    ~SwPagePreView();
};

// ----------------- inline methods ----------------------
inline void SwPagePreView::AdjustEditWin()
{
    OuterResizePixel( Point(), GetFrameWindow()->GetOutputSizePixel() );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
