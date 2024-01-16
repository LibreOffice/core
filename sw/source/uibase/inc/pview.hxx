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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_PVIEW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_PVIEW_HXX

#include <tools/link.hxx>
#include <tools/fract.hxx>
#include <vcl/window.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/zoomitem.hxx>
#include <swdllapi.h>
#include <shellid.hxx>

class SwViewOption;
class SwDocShell;
class SwScrollbar;
class SwViewShell;
class SwPagePreview;
class ImageButton;
class Button;
class SwRect;
class DataChangedEvent;
class CommandEvent;
class SwPagePreviewLayout;

/// Provides the VCL widget that is used for the main area of the File -> Print Preview window.
class SAL_DLLPUBLIC_RTTI SwPagePreviewWin final : public vcl::Window
{
    SwViewShell* mpViewShell;
    sal_uInt16 mnSttPage;
    sal_Int16 mnRow;
    sal_Int16 mnCol;
    Size maPxWinSize;
    Fraction maScale;
    SwPagePreview& mrView;
    bool mbCalcScaleForPreviewLayout;
    tools::Rectangle maPaintedPreviewDocRect;
    SwPagePreviewLayout* mpPgPreviewLayout;

    void SetPagePreview( sal_Int16 nRow, sal_Int16 nCol );

    using Window::Scroll;

public:
    SwPagePreviewWin( vcl::Window* pParent, SwPagePreview& rView );
    virtual ~SwPagePreviewWin() override;

    // calls SwViewShell::Paint
    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void KeyInput( const KeyEvent & ) override;
    virtual void Command( const CommandEvent& rCEvt ) override;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void DataChanged( const DataChangedEvent& ) override;

    void SetViewShell( SwViewShell* pShell );

    SwViewShell* GetViewShell() const
    {
        return mpViewShell;
    }

    sal_Int16 GetRow() const
    {
        return mnRow;
    }

    sal_Int16 GetCol() const
    {
        return mnCol;
    }

    sal_uInt16 GetSttPage() const
    {
        return mnSttPage;
    }

    void SetSttPage(sal_uInt16 n)
    {
        mnSttPage = n;
    }

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
    sal_uInt16 GetDefSttPage() const   { return 1 == mnCol ? 1 : 0; }

    void CalcWish( sal_Int16 nNewRow, sal_Int16 nNewCol );

    void SetWinSize( const Size& rNewSize );

    // Add <MV_SELPAGE>, <MV_SCROLL>
    enum MoveMode{ MV_CALC, MV_PAGE_UP, MV_PAGE_DOWN, MV_DOC_STT, MV_DOC_END,
                   MV_SELPAGE, MV_SCROLL, MV_NEWWINSIZE, MV_SPECIFIC_PAGE };
    bool MovePage( int eMoveMode );

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

    const tools::Rectangle& GetPaintedPreviewDocRect() const
    {
        return maPaintedPreviewDocRect;
    }

    void Scroll(tools::Long nXMove, tools::Long nYMove, ScrollFlags nFlags = ScrollFlags::NONE) override;

    /** Method to enable/disable book preview
        @param _bBookPreview
        input parameter - boolean indicating, if book preview mode has to
        switch on <true> or of <false>

        @return boolean indicating, if book preview mode has changed.
    */
    bool SetBookPreviewMode( const bool _bBookPreview );

    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;
};

/**
 * View of a document
 */
class SW_DLLPUBLIC SwPagePreview final : public SfxViewShell
{
    // ViewWindow and handle to core
    // current dispatcher shell
    VclPtr<SwPagePreviewWin> m_pViewWin;
    //viewdata of the previous SwView and the new cursor position
    OUString m_sSwViewData;
    //and the new cursor position if the user double click in the PagePreview
    OUString m_sNewCursorPosition;
    // to support keyboard the number of the page to go to can be set too
    sal_uInt16 m_nNewPage;
    // visible range
    OUString m_sPageStr;
    Size m_aDocSize;
    tools::Rectangle               m_aVisArea;

    // MDI control elements
    VclPtr<SwScrollbar> m_pHScrollbar;
    VclPtr<SwScrollbar> m_pVScrollbar;
    bool mbHScrollbarEnabled : 1;
    bool mbVScrollbarEnabled : 1;

    sal_uInt16 mnPageCount;
    bool m_bNormalPrint;

    // New members to reset design mode at draw view for form shell on switching
    // back from writer page preview to normal view.
    bool mbResetFormDesignMode:1;
    bool mbFormDesignModeToReset:1;

    SAL_DLLPRIVATE void Init();
    SAL_DLLPRIVATE Point AlignToPixel(const Point& rPt) const;

    SAL_DLLPRIVATE void CreateScrollbar( bool bHori);
    DECL_DLLPRIVATE_LINK(HoriScrollHdl, weld::Scrollbar&, void);
    DECL_DLLPRIVATE_LINK(VertScrollHdl, weld::Scrollbar&, void);
    SAL_DLLPRIVATE void ScrollHdl(weld::Scrollbar&, bool bHorizontal);
    SAL_DLLPRIVATE void EndScrollHdl(weld::Scrollbar&, bool bHorizontal);
    SAL_DLLPRIVATE bool ChgPage( int eMvMode, bool bUpdateScrollbar = true );

    SAL_DLLPRIVATE virtual SfxPrinter*     GetPrinter( bool bCreate = false ) override;
    SAL_DLLPRIVATE virtual sal_uInt16      SetPrinter( SfxPrinter *pNewPrinter, SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL ) override;
    SAL_DLLPRIVATE virtual bool            HasPrintOptionsPage() const override;
    SAL_DLLPRIVATE virtual std::unique_ptr<SfxTabPage> CreatePrintOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rOptions) override;

    SAL_DLLPRIVATE void CalcAndSetBorderPixel( SvBorder &rToFill );

    /** Helper method to execute SfxRequest FN_PAGE_UP and FN_PAGE_DOWN

        @param _bPgUp
        input parameter - boolean that indicates, if FN_PAGE_UP or FN_PAGE_DOWN
        has to be executed.

        @param _pReq
        optional input parameter - pointer to the <SfxRequest> instance, if existing.
    */
    SAL_DLLPRIVATE void ExecPgUpAndPgDown( const bool  _bPgUp,
                             SfxRequest* _pReq );

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize, bool inplaceEditModeChange ) override;
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize ) override;

    void Activate(bool bMDI) override;

    void         SetZoom(SvxZoomType eSet, sal_uInt16 nFactor);

public:
    SFX_DECL_VIEWFACTORY(SwPagePreview);
    SFX_DECL_INTERFACE(SW_PAGEPREVIEW)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SwViewShell* GetViewShell() const
    { return m_pViewWin->GetViewShell(); }
    void RepaintCoreRect( const SwRect& rRect )
    { m_pViewWin->RepaintCoreRect( rRect ); }

    void DocSzChgd(const Size& rNewSize);

    void SetVisArea( const tools::Rectangle& );

    void ScrollViewSzChg();
    void ScrollDocSzChg();
    void ShowHScrollbar(bool bShow);
    void ShowVScrollbar(bool bShow);
    void EnableHScrollbar(bool bEnable);
    void EnableVScrollbar(bool bEnable);

    sal_uInt16 GetPageCount() const        { return mnPageCount; }
    sal_uInt16 GetSelectedPage() const {return m_pViewWin->SelectedPage();}

    bool HandleWheelCommands( const CommandEvent& );

    const OUString& GetPrevSwViewData() const       { return m_sSwViewData; }
    void SetNewCursorPos( const OUString& rStr ) { m_sNewCursorPosition = rStr; }
    const OUString& GetNewCursorPos() const           { return m_sNewCursorPosition; }

    sal_uInt16 GetNewPage() const {return m_nNewPage;}

    // Handler
    void Execute(SfxRequest&);
    void GetState(SfxItemSet&);
    static void StateUndo(SfxItemSet&);

    SwDocShell* GetDocShell();

    // Inline method to request values of new members
    // <mbResetFormDesignMode> and <mbFormDesignModeToReset>
    bool ResetFormDesignMode() const
    {
        return mbResetFormDesignMode;
    }

    bool FormDesignModeToReset() const
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

    SwPagePreview(SfxViewFrame& rFrame, SfxViewShell*);
    virtual ~SwPagePreview() override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
