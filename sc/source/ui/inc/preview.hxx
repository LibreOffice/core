/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: preview.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:11:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_PREVIEW_HXX
#define SC_PREVIEW_HXX


#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
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
    BOOL            bInPaint;
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
