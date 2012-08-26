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
// OD 12.12.2002 #103492#
class SwPagePreviewLayout;

// OD 24.03.2003 #108282# - delete member <mnVirtPage> and its accessor.
class SwPagePreViewWin : public Window
{
    ViewShell*          mpViewShell;
    sal_uInt16              mnSttPage;
    sal_uInt8                mnRow, mnCol;
    Size                maPxWinSize;
    Fraction            maScale;
    SwPagePreView&      mrView;
    // OD 02.12.2002 #103492#
    bool                mbCalcScaleForPreviewLayout;
    Rectangle           maPaintedPreviewDocRect;
    // OD 12.12.2002 #103492#
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

        OD 13.12.2002 #103492#

        @author OD

        @return selected page number
    */
    sal_uInt16 SelectedPage() const;

    /** set selected page number in document preview

        OD 13.12.2002 #103492#

        @author OD

        @param _nSelectedPageNum
        input parameter - physical page number of page that will be the selected one.
    */
    void SetSelectedPage( sal_uInt16 _nSelectedPageNum );

    //JP 19.08.98: bei Einspaltigkeit gibt es keine 0. Seite!
    sal_uInt16  GetDefSttPage() const   { return 1 == mnCol ? 1 : 0; }

    void CalcWish( sal_uInt8 nNewRow, sal_uInt8 nNewCol );

    const Size& GetWinSize() const  { return maPxWinSize; }
    void SetWinSize( const Size& rNewSize );

    // OD 18.12.2002 #103492# - add <MV_SELPAGE>, <MV_SCROLL>
    enum MoveMode{ MV_CALC, MV_PAGE_UP, MV_PAGE_DOWN, MV_DOC_STT, MV_DOC_END,
                   MV_SELPAGE, MV_SCROLL, MV_NEWWINSIZE };
    int MovePage( int eMoveMode );

    // erzeuge den String fuer die StatusLeiste
    OUString GetStatusStr( sal_uInt16 nPageCount ) const;

    void RepaintCoreRect( const SwRect& rRect );

    /** method to adjust preview to a new zoom factor

        OD 02.12.2002 #103492#
        paint of preview is prepared for a new zoom factor
        OD 24.09.2003 #i19975# - zoom type has also been considered.
        Thus, add new parameter <_eZoomType>

        @author OD
    */
    void AdjustPreviewToNewZoom( const sal_uInt16 _nZoomFactor,
                                 const SvxZoomType _eZoomType );

    const Rectangle& GetPaintedPreviewDocRect() const
    {
        return maPaintedPreviewDocRect;
    }

    void Scroll(long nXMove, long nYMove, sal_uInt16 nFlags = 0);

    /** method to enable/disable book preview

        OD 2004-03-05 #i18143#

        @author OD

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

/*--------------------------------------------------------------------
    Description:   view of a document
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwPagePreView: public SfxViewShell
{
    // ViewWindow and handle to core
    // current dispatcher shell
    SwPagePreViewWin        aViewWin;
    //viewdata of the previous SwView and the new crsrposition
    String                  sSwViewData,
    //and the new cursor position if the user double click in the PagePreView
                            sNewCrsrPos;
    // to support keyboard the number of the page to go to can be set too
    sal_uInt16                  nNewPage;
    // visible range
    OUString                sPageStr;
    Size                    aDocSz;
    Rectangle               aVisArea;

    // MDI control elements
    SwScrollbar             *pHScrollbar;
    SwScrollbar             *pVScrollbar;
    ImageButton             *pPageUpBtn,
                            *pPageDownBtn;
    // dummy window for filling the lower right edge when both scrollbars are active
    Window                  *pScrollFill;

    sal_uInt16                  mnPageCount;
    sal_Bool                    bNormalPrint;

    // OD 09.01.2003 #106334#
    // new members to reset design mode at draw view for form shell on switching
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
    SW_DLLPRIVATE virtual sal_uInt16          SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false );
    SW_DLLPRIVATE virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent,
                                                const SfxItemSet &rOptions );

    SW_DLLPRIVATE void CalcAndSetBorderPixel( SvBorder &rToFill, sal_Bool bInner );

    /** help method to execute SfxRequest FN_PAGE_UP and FN_PAGE_DOWN

        OD 04.03.2003 #107369#

        @param _bPgUp
        input parameter - boolean that indicates, if FN_PAGE_UP or FN_PAGE_DOWN
        has to be executed.

        @param _pReq
        optional input parameter - pointer to the <SfxRequest> instance, if existing.

        @author OD
    */
    SW_DLLPRIVATE void _ExecPgUpAndPgDown( const bool  _bPgUp,
                             SfxRequest* _pReq = 0 );

protected:
    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual Size    GetOptimalSizePixel() const;

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

    sal_uInt16          GetPageCount() const        { return mnPageCount; }
    sal_uInt16      GetSelectedPage() const {return aViewWin.SelectedPage();}

    sal_Bool            HandleWheelCommands( const CommandEvent& );

    const String&   GetPrevSwViewData() const       { return sSwViewData; }
    void            SetNewCrsrPos( const String& rStr ) { sNewCrsrPos = rStr; }
    const String&   GetNewCrsrPos() const           { return sNewCrsrPos; }

    sal_uInt16          GetNewPage() const {return nNewPage;}
    void            SetNewPage(sal_uInt16 nSet)  {nNewPage = nSet;}

        // handler
    void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&);
    void            StateUndo(SfxItemSet&);

    SwDocShell*     GetDocShell();

    //apply Accessiblity options
    void ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions);

    // OD 09.01.2003 #106334# - inline method to request values of new members
    // <mbResetFormDesignMode> and <mbFormDesignModeToReset>
    inline sal_Bool ResetFormDesignMode() const
    {
        return mbResetFormDesignMode;
    }

    inline sal_Bool FormDesignModeToReset() const
    {
        return mbFormDesignModeToReset;
    }

    /** adjust position of vertical scrollbar

        OD 19.02.2003 #107369
        Currently used, if the complete preview layout rows fit into to the given
        window, if a new page is selected and this page is visible.

        @author OD

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
