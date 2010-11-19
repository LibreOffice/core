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

class ScDocShell;
class ScPreviewShell;
class FmFormView;

class ScPreview : public Window
{
private:
                                        // eingestellt:
    long            nPageNo;            // Seite im Dokument
    USHORT          nZoom;              // eingestellter Zoom
    Point           aOffset;            // positiv

                                        // berechnet:
    BOOL            bValid;             // folgende Werte gueltig
    SCTAB           nTabCount;
    SCTAB           nTabsTested;        // fuer wieviele Tabellen ist nPages gueltig?
    long            nPages[MAXTABCOUNT];
    long            nFirstAttr[MAXTABCOUNT];
    SCTAB           nTab;               // Tabelle
    long            nTabPage;           // Seite von Tabelle
    long            nTabStart;          // erste Seite der Tabelle (wirklich)
    long            nDisplayStart;      // dito, relativ zum Anfang der Zaehlung
    Date            aDate;
    Time            aTime;
    long            nTotalPages;
    Size            aPageSize;          // fuer GetOptimalZoom
    BOOL            bStateValid;
    BOOL            bLocationValid;
    ScPrintState    aState;
    ScPreviewLocationData* pLocationData;   // stores table layout for accessibility API
    FmFormView*     pDrawView;

                                        // intern:
    bool            bInPaint;
    bool            bInSetZoom;
    BOOL            bInGetState;
    ScDocShell*     pDocShell;
    ScPreviewShell* pViewShell;

    BOOL            bLeftRulerMove;
    BOOL            bRightRulerMove;
    BOOL            bTopRulerMove;
    BOOL            bBottomRulerMove;
    BOOL            bHeaderRulerMove;
    BOOL            bFooterRulerMove;

    BOOL            bLeftRulerChange;
    BOOL            bRightRulerChange;
    BOOL            bTopRulerChange;
    BOOL            bBottomRulerChange;
    BOOL            bHeaderRulerChange;
    BOOL            bFooterRulerChange;
    BOOL            bPageMargin;
    BOOL            bColRulerMove;
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
    void    CalcPages( SCTAB nToWhichTab );
    void    RecalcPages();
    void    UpdateDrawView();
    void    DoPrint( ScPreviewLocationData* pFillLocation );

    void    InvalidateLocationData( ULONG nId );

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

    void    DataChanged(BOOL bNewTime = FALSE);             // statt Invalidate rufen
    void    DoInvalidate();

    void    SetXOffset( long nX );
    void    SetYOffset( long nY );
    void    SetZoom(USHORT nNewZoom);
    void    SetPageNo( long nPage );

    BOOL    GetPageMargins()const { return bPageMargin; }
    void    SetPageMargins( BOOL bVal )  { bPageMargin = bVal; }
    void    DrawInvert( long nDragPos, USHORT nFlags );
    void    DragMove( long nDragMovePos, USHORT nFlags );


    const ScPreviewLocationData& GetLocationData();

    String  GetPosString();

    long    GetPageNo() const   { return nPageNo; }
    USHORT  GetZoom() const     { return nZoom; }
    Point   GetOffset() const   { return aOffset; }

    SCTAB   GetTab()            { if (!bValid) { CalcPages(0); RecalcPages(); } return nTab; }
    long    GetTotalPages()     { if (!bValid) { CalcPages(0); RecalcPages(); } return nTotalPages; }

    BOOL    AllTested() const   { return bValid && nTabsTested >= nTabCount; }

    USHORT  GetOptimalZoom(BOOL bWidthOnly);
    long    GetFirstPage(SCTAB nTab);

    void    CalcAll()           { CalcPages(MAXTAB); }
    void    SetInGetState(BOOL bSet) { bInGetState = bSet; }

    DECL_STATIC_LINK( ScPreview, InvalidateHdl, void* );
    static void StaticInvalidate();

    FmFormView* GetDrawView() { return pDrawView; }
};



#endif
