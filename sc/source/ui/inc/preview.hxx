/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    sal_uInt16          nZoom;              // eingestellter Zoom
    Point           aOffset;            // positiv

                                        // berechnet:
    sal_Bool            bValid;             // folgende Werte gueltig
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
    sal_Bool            bStateValid;
    sal_Bool            bLocationValid;
    ScPrintState    aState;
    ScPreviewLocationData* pLocationData;   // stores table layout for accessibility API
    FmFormView*     pDrawView;

                                        // intern:
    bool            bInPaint;
    bool            bInSetZoom;
    sal_Bool        bInGetState;
    ScDocShell*     pDocShell;
    ScPreviewShell* pViewShell;

    sal_Bool            bLeftRulerMove;
    sal_Bool            bRightRulerMove;
    sal_Bool            bTopRulerMove;
    sal_Bool            bBottomRulerMove;
    sal_Bool            bHeaderRulerMove;
    sal_Bool            bFooterRulerMove;

    sal_Bool            bLeftRulerChange;
    sal_Bool            bRightRulerChange;
    sal_Bool            bTopRulerChange;
    sal_Bool            bBottomRulerChange;
    sal_Bool            bHeaderRulerChange;
    sal_Bool            bFooterRulerChange;
    sal_Bool            bPageMargin;
    sal_Bool            bColRulerMove;
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

    void    DataChanged(sal_Bool bNewTime = sal_False);             // statt Invalidate rufen
    void    DoInvalidate();

    void    SetXOffset( long nX );
    void    SetYOffset( long nY );
    void    SetZoom(sal_uInt16 nNewZoom);
    void    SetPageNo( long nPage );

    sal_Bool    GetPageMargins()const { return bPageMargin; }
    void    SetPageMargins( sal_Bool bVal )  { bPageMargin = bVal; }
    void    DrawInvert( long nDragPos, sal_uInt16 nFlags );
    void    DragMove( long nDragMovePos, sal_uInt16 nFlags );


    const ScPreviewLocationData& GetLocationData();

    String  GetPosString();

    long    GetPageNo() const   { return nPageNo; }
    sal_uInt16  GetZoom() const     { return nZoom; }
    Point   GetOffset() const   { return aOffset; }

    SCTAB   GetTab()            { if (!bValid) { CalcPages(0); RecalcPages(); } return nTab; }
    long    GetTotalPages()     { if (!bValid) { CalcPages(0); RecalcPages(); } return nTotalPages; }

    sal_Bool    AllTested() const   { return bValid && nTabsTested >= nTabCount; }

    sal_uInt16  GetOptimalZoom(sal_Bool bWidthOnly);
    long    GetFirstPage(SCTAB nTab);

    void    CalcAll()           { CalcPages(MAXTAB); }
    void    SetInGetState(sal_Bool bSet) { bInGetState = bSet; }

    DECL_STATIC_LINK( ScPreview, InvalidateHdl, void* );
    static void StaticInvalidate();

    FmFormView* GetDrawView() { return pDrawView; }
    virtual void SwitchView();
};



#endif
