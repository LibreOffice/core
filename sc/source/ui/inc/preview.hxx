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
#pragma once

#include <vcl/window.hxx>
#include "printfun.hxx"
#include <markdata.hxx>

#include <vector>

class ScDocShell;
class ScPreviewShell;
class FmFormView;

class SAL_DLLPUBLIC_RTTI ScPreview : public vcl::Window
{
private:
    ScMarkData::MarkedTabsType maSelectedTabs;
                                        // set:
    tools::Long            nPageNo;            // Pages in document
    sal_uInt16          nZoom;              // set Zoom
    Point           aOffset;            // positive

                                        // calculated:
    SCTAB           nTabCount;
    SCTAB           nTabsTested;        // for how many sheets is nPages valid?
    std::vector<tools::Long>       nPages;
    std::vector<tools::Long>       nFirstAttr;
    SCTAB           nTab;               // Sheet
    tools::Long            nTabPage;           // Page of sheet
    tools::Long            nTabStart;          // First (real) page of the sheet
    tools::Long            nDisplayStart;      // same as above, relative to the start of counting
    DateTime        aDateTime;
    tools::Long            nTotalPages;
    ScPrintState    aState;
    std::unique_ptr<ScPreviewLocationData> pLocationData;   // stores table layout for accessibility API
    std::unique_ptr<FmFormView> pDrawView;

                                        // internal:
    ScDocShell*     pDocShell;
    ScPreviewShell* pViewShell;

    bool            bInGetState:1;
    bool            bValid:1;             // the following values true
    bool            bStateValid:1;
    bool            bLocationValid:1;
    bool            bInPaint:1;
    bool            bInSetZoom:1;
    bool            bLeftRulerMove:1;
    bool            bRightRulerMove:1;
    bool            bTopRulerMove:1;
    bool            bBottomRulerMove:1;
    bool            bHeaderRulerMove:1;
    bool            bFooterRulerMove:1;
    bool            bLeftRulerChange:1;
    bool            bRightRulerChange:1;
    bool            bTopRulerChange:1;
    bool            bBottomRulerChange:1;
    bool            bHeaderRulerChange:1;
    bool            bFooterRulerChange:1;
    bool            bPageMargin:1;
    bool            bColRulerMove:1;
    bool            mbHasEmptyRangeTable:1; /// we have at least one sheet with empty print range (print range set to '- none -').

    ScRange         aPageArea;
    std::vector<tools::Long> mvRight;
    tools::Long            nLeftPosition;
    tools::Long            mnScale;
    SCCOL           nColNumberButtonDown;
    Point           aButtonDownChangePoint;
    Point           aButtonDownPt;
    Point           aButtonUpPt;
    tools::Long            nHeaderHeight;
    tools::Long            nFooterHeight;

    void    TestLastPage();
    void    CalcPages();
    void    RecalcPages();
    void    UpdateDrawView();
    void    DoPrint( ScPreviewLocationData* pFillLocation );

    void    InvalidateLocationData( SfxHintId nId );

    using Window::SetZoom;

protected:
    virtual void   Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void   Command( const CommandEvent& rCEvt ) override;
    virtual void   KeyInput( const KeyEvent& rKEvt ) override;
    virtual void   MouseMove( const MouseEvent& rMEvt ) override;
    virtual void   MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void   MouseButtonUp( const MouseEvent& rMEvt ) override;

    virtual void   GetFocus() override;
    virtual void   LoseFocus() override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

public:
            ScPreview( vcl::Window* pParent, ScDocShell* pDocSh, ScPreviewShell* pViewSh );
            virtual ~ScPreview() override;
    virtual void dispose() override;

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

    SC_DLLPUBLIC void    DataChanged(bool bNewTime);             //  Instead of calling Invalidate
    void    DoInvalidate();

    void    SetXOffset( tools::Long nX );
    void    SetYOffset( tools::Long nY );
    void    SetZoom(sal_uInt16 nNewZoom);
    SC_DLLPUBLIC void    SetPageNo( tools::Long nPage );

    bool    GetPageMargins() const { return bPageMargin; }
    void    SetPageMargins( bool bVal )  { bPageMargin = bVal; }
    void    DrawInvert( tools::Long nDragPos, PointerStyle nFlags );
    void    DragMove( tools::Long nDragMovePos, PointerStyle nFlags );

    const ScPreviewLocationData& GetLocationData();

    OUString  GetPosString();

    tools::Long    GetPageNo() const   { return nPageNo; }
    sal_uInt16  GetZoom() const     { return nZoom; }
    const Point& GetOffset() const   { return aOffset; }

    SCTAB   GetTab()            { if (!bValid) { CalcPages(); RecalcPages(); } return nTab; }
    tools::Long    GetTotalPages()     { if (!bValid) { CalcPages(); RecalcPages(); } return nTotalPages; }

    bool    AllTested() const   { return bValid && nTabsTested >= nTabCount; }

    sal_uInt16  GetOptimalZoom(bool bWidthOnly);
    SC_DLLPUBLIC tools::Long    GetFirstPage(SCTAB nTab);

    void    CalcAll()           { CalcPages(); }
    void    SetInGetState(bool bSet) { bInGetState = bSet; }

    DECL_STATIC_LINK( ScPreview, InvalidateHdl, void*, void );
    static void StaticInvalidate();

    FmFormView* GetDrawView() { return pDrawView.get(); }

    SC_DLLPUBLIC void SetSelectedTabs(const ScMarkData& rMark);
    const ScMarkData::MarkedTabsType& GetSelectedTabs() const { return maSelectedTabs; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
