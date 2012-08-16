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
#ifndef SC_PREVIEW_HXX
#define SC_PREVIEW_HXX


#include <vcl/window.hxx>
#include "printfun.hxx"     // ScPrintState
#include "markdata.hxx"

#include <vector>

class ScDocShell;
class ScPreviewShell;
class FmFormView;

class ScPreview : public Window
{
private:
    ScMarkData::MarkedTabsType maSelectedTabs;
                                        // set:
    long            nPageNo;            // Pages in document
    sal_uInt16          nZoom;              // set Zoom
    Point           aOffset;            // positive

                                        // calculated:
    SCTAB           nTabCount;
    SCTAB           nTabsTested;        // for how many sheets is nPages valid?
    std::vector<long>       nPages;
    std::vector<long>       nFirstAttr;
    SCTAB           nTab;               // Sheet
    long            nTabPage;           // Page of sheet
    long            nTabStart;          // First (real) page of the sheet
    long            nDisplayStart;      // same as above, relative to the start of counting
    Date            aDate;
    Time            aTime;
    long            nTotalPages;
    Size            aPageSize;          // for GetOptimalZoom
    ScPrintState    aState;
    ScPreviewLocationData* pLocationData;   // stores table layout for accessibility API
    FmFormView*     pDrawView;

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

    ScRange         aPageArea;
    long            nRight[ MAXCOL+1 ];
    long            nLeftPosition;
    long            mnScale;
    SCCOL           nColNumberButttonDown;
    Point           aButtonDownChangePoint;
    Point           aButtonDownPt;
    Point           aButtonUpPt;
    long            nHeaderHeight;
    long            nFooterHeight;

    void    TestLastPage();
    void    CalcPages();
    void    RecalcPages();
    void    UpdateDrawView();
    void    DoPrint( ScPreviewLocationData* pFillLocation );

    void    InvalidateLocationData( sal_uLong nId );

    using Window::SetZoom;

protected:
    virtual void   Paint( const Rectangle& rRect );
    virtual void   Command( const CommandEvent& rCEvt );
    virtual void   KeyInput( const KeyEvent& rKEvt );
    virtual void   MouseMove( const MouseEvent& rMEvt );
    virtual void   MouseButtonDown( const MouseEvent& rMEvt );
    virtual void   MouseButtonUp( const MouseEvent& rMEvt );

    virtual void   GetFocus();
    virtual void   LoseFocus();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

public:
            ScPreview( Window* pParent, ScDocShell* pDocSh, ScPreviewShell* pViewSh );
            ~ScPreview();

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

    SC_DLLPUBLIC void    DataChanged(bool bNewTime = false);             //  Instead of calling Invalidate
    void    DoInvalidate();

    void    SetXOffset( long nX );
    void    SetYOffset( long nY );
    void    SetZoom(sal_uInt16 nNewZoom);
    SC_DLLPUBLIC void    SetPageNo( long nPage );

    bool    GetPageMargins() const { return bPageMargin; }
    void    SetPageMargins( bool bVal )  { bPageMargin = bVal; }
    void    DrawInvert( long nDragPos, sal_uInt16 nFlags );
    void    DragMove( long nDragMovePos, sal_uInt16 nFlags );


    const ScPreviewLocationData& GetLocationData();

    String  GetPosString();

    long    GetPageNo() const   { return nPageNo; }
    sal_uInt16  GetZoom() const     { return nZoom; }
    Point   GetOffset() const   { return aOffset; }

    SCTAB   GetTab()            { if (!bValid) { CalcPages(); RecalcPages(); } return nTab; }
    long    GetTotalPages()     { if (!bValid) { CalcPages(); RecalcPages(); } return nTotalPages; }

    bool    AllTested() const   { return bValid && nTabsTested >= nTabCount; }

    sal_uInt16  GetOptimalZoom(bool bWidthOnly);
    SC_DLLPUBLIC long    GetFirstPage(SCTAB nTab);

    void    CalcAll()           { CalcPages(); }
    void    SetInGetState(bool bSet) { bInGetState = bSet; }

    DECL_STATIC_LINK( ScPreview, InvalidateHdl, void* );
    static void StaticInvalidate();

    FmFormView* GetDrawView() { return pDrawView; }

    SC_DLLPUBLIC void SetSelectedTabs(const ScMarkData& rMark);
    SC_DLLPUBLIC const ScMarkData::MarkedTabsType& GetSelectedTabs() const;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
