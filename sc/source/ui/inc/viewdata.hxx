/*************************************************************************
 *
 *  $RCSfile: viewdata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-25 17:33:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_VIEWDATA_HXX
#define SC_VIEWDATA_HXX

#ifndef SC_VIEWOPTI_HXX
#include "viewopti.hxx"
#endif

#ifndef SC_MARKDATA_HXX
#include "markdata.hxx"
#endif


// ---------------------------------------------------------------------------

#define SC_SLIDER_SIZE      2
#define SC_SMALL3DSHADOW    COL_BLACK
#define SC_SIZE_NONE        65535
#define SC_TABSTART_NONE    65535

#define SC_FILL_NONE        0
#define SC_FILL_FILL        1
#define SC_FILL_EMBED_LT    2
#define SC_FILL_EMBED_RB    3
#define SC_FILL_MATRIX      4

enum ScSplitMode { SC_SPLIT_NONE = 0, SC_SPLIT_NORMAL, SC_SPLIT_FIX };

enum ScSplitPos { SC_SPLIT_TOPLEFT, SC_SPLIT_TOPRIGHT, SC_SPLIT_BOTTOMLEFT, SC_SPLIT_BOTTOMRIGHT };
enum ScHSplitPos { SC_SPLIT_LEFT, SC_SPLIT_RIGHT };
enum ScVSplitPos { SC_SPLIT_TOP, SC_SPLIT_BOTTOM };

inline ScHSplitPos WhichH( ScSplitPos ePos );
inline ScVSplitPos WhichV( ScSplitPos ePos );
inline ScSplitPos Which( ScHSplitPos eHPos );
inline ScSplitPos Which( ScVSplitPos eVPos );

//  Bildschirmverhalten bei Cursorbewegungen:
enum ScFollowMode { SC_FOLLOW_NONE, SC_FOLLOW_LINE, SC_FOLLOW_FIX, SC_FOLLOW_JUMP };

//  Mausmodi um Bereiche zu selektieren
enum ScRefType { SC_REFTYPE_NONE, SC_REFTYPE_REF, SC_REFTYPE_FILL,
                    SC_REFTYPE_EMBED_LT, SC_REFTYPE_EMBED_RB };

class ScDocShell;
class ScDocument;
class ScDBFunc;
class ScTabViewShell;
class ScDrawView;
class ScEditEngineDefaulter;
class EditView;
class EditStatus;
class Outliner;
class Window;
class SfxObjectShell;
class SfxBindings;
class SfxDispatcher;
class ScPatternAttr;
class ScRangeListRef;
class ScExtDocOptions;

//--------------------------------------------------------------------------


class ScViewDataTable                           // Daten pro Tabelle
{
friend class ScViewData;
private:
    long            nTPosX[2];                  // MapMode - Offset (Twips)
    long            nTPosY[2];
    long            nMPosX[2];                  // MapMode - Offset (1/100 mm)
    long            nMPosY[2];
    long            nPixPosX[2];                // Offset in Pixeln
    long            nPixPosY[2];
    long            nHSplitPos;
    long            nVSplitPos;

    ScSplitMode     eHSplitMode;
    ScSplitMode     eVSplitMode;
    ScSplitPos      eWhichActive;

    USHORT          nFixPosX;                   // Zellposition des Splitters beim Fixieren
    USHORT          nFixPosY;

    USHORT          nCurX;
    USHORT          nCurY;
    USHORT          nOldCurX;
    USHORT          nOldCurY;
    USHORT          nPosX[2];
    USHORT          nPosY[2];

    BOOL            bOldCurValid;               // "virtuelle" Cursorpos. bei zusammengefassten

                    ScViewDataTable();
                    ScViewDataTable( const ScViewDataTable& rDataTable );
                    ~ScViewDataTable();
};

// ---------------------------------------------------------------------------

class ScViewData
{
private:
    double              nPPTX, nPPTY;               // Scaling-Faktoren

    ScViewDataTable*    pTabData[MAXTAB+1];
    ScViewDataTable*    pThisTab;                   // Daten der angezeigten Tabelle
    ScDocShell*         pDocShell;
    ScDocument*         pDoc;
    ScDBFunc*           pView;
    ScTabViewShell*     pViewShell;
    EditView*           pEditView[4];               // gehoert dem Fenster
    ScViewOptions*      pOptions;
    EditView*           pSpellingView;

    ScMarkData          aMarkData;

    long                nEditMargin;

    Size                aScenButSize;               // Groesse eines Szenario-Buttons

    Size                aScrSize;
    MapMode             aLogicMode;                 // skalierter 1/100mm-MapMode

    Fraction            aZoomX;                     // eingestellter Zoom X
    Fraction            aZoomY;                     // eingestellter Zoom Y (angezeigt)
    Fraction            aPageZoomX;                 // Zoom im Seitenumbruch-Vorschaumodus
    Fraction            aPageZoomY;

    ScRefType           eRefType;

    USHORT              nTabNo;                     // angezeigte Tabelle
    USHORT              nRefTabNo;                  // Tabelle auf die sich RefInput bezieht
    USHORT              nRefStartX;
    USHORT              nRefStartY;
    USHORT              nRefStartZ;
    USHORT              nRefEndX;
    USHORT              nRefEndY;
    USHORT              nRefEndZ;
    USHORT              nFillStartX;                // Fill-Cursor
    USHORT              nFillStartY;
    USHORT              nFillEndX;
    USHORT              nFillEndY;
    USHORT              nEditCol;                   // Position dazu
    USHORT              nEditRow;
    USHORT              nEditEndCol;                // Ende der Edit-View
    USHORT              nEditEndRow;
    USHORT              nTabStartCol;               // fuer Enter nach Tab
    ScRange             aDelRange;                  // fuer AutoFill-Loeschen

    BOOL                bEditActive[4];             // aktiv?
    BOOL                bActive;                    // aktives Fenster ?
    BOOL                bIsRefMode;                 // Referenzeingabe
    BOOL                bDelMarkValid;              // nur gueltig bei SC_REFTYPE_FILL
    BYTE                nFillMode;                  // Modus
    BOOL                bPagebreak;                 // Seitenumbruch-Vorschaumodus

    DECL_LINK (EmptyEditHdl, EditStatus*);
    DECL_LINK (EditEngineHdl, EditStatus*);

    void            CalcPPT();

public:
                    ScViewData( ScDocShell* pDocSh, ScTabViewShell* pViewSh );
                    ScViewData( const ScViewData& rViewData );
                    ~ScViewData();

    void            InitData( ScDocument* pDocument );
    void            InitFrom( const ScViewData* pRef );

    void            SetDocShell( ScDocShell* pShell );

    ScDocShell*     GetDocShell() const     { return pDocShell; }
    ScDBFunc*       GetView() const         { return pView; }
    ScTabViewShell* GetViewShell() const    { return pViewShell; }

    SfxObjectShell* GetSfxDocShell() const;
    SfxBindings&    GetBindings();          // from ViewShell's ViewFrame
    SfxDispatcher&  GetDispatcher();        // from ViewShell's ViewFrame

    ScMarkData&     GetMarkData()           { return aMarkData; }

    Window*         GetDialogParent();          // von tabvwsh weitergeleitet
    Window*         GetActiveWin();             // von View
    ScDrawView*     GetScDrawView();            // von View
    BOOL            IsMinimized();              // von View

    void            UpdateInputHandler( BOOL bForce = FALSE );

    void            WriteUserData(String& rData);
    void            ReadUserData(const String& rData);
    void            ReadExtOptions(const ScExtDocOptions& rOpt);

    ScDocument*     GetDocument() const;

    void            SetViewShell( ScTabViewShell* pViewSh );

    BOOL            IsActive() const            { return bActive; }
    void            Activate(BOOL bActivate)    { bActive = bActivate; }

    void            UpdateThis();

    void            InsertTab( USHORT nTab );
    void            DeleteTab( USHORT nTab );
    void            CopyTab( USHORT nSrcTab, USHORT nDestTab );
    void            MoveTab( USHORT nSrcTab, USHORT nDestTab );

    USHORT          GetRefTabNo() const                     { return nRefTabNo; }
    void            SetRefTabNo( USHORT nNewTab )           { nRefTabNo = nNewTab; }

    USHORT          GetTabNo() const                        { return nTabNo; }
    ScSplitPos      GetActivePart() const                   { return pThisTab->eWhichActive; }
    USHORT          GetPosX( ScHSplitPos eWhich ) const     { return pThisTab->nPosX[eWhich]; }
    USHORT          GetPosY( ScVSplitPos eWhich ) const     { return pThisTab->nPosY[eWhich]; }
    USHORT          GetCurX() const                         { return pThisTab->nCurX; }
    USHORT          GetCurY() const                         { return pThisTab->nCurY; }
    BOOL            HasOldCursor() const                    { return pThisTab->bOldCurValid; }
    USHORT          GetOldCurX() const;
    USHORT          GetOldCurY() const;
    ScSplitMode     GetHSplitMode() const                   { return pThisTab->eHSplitMode; }
    ScSplitMode     GetVSplitMode() const                   { return pThisTab->eVSplitMode; }
    long            GetHSplitPos() const                    { return pThisTab->nHSplitPos; }
    long            GetVSplitPos() const                    { return pThisTab->nVSplitPos; }
    USHORT          GetFixPosX() const                      { return pThisTab->nFixPosX; }
    USHORT          GetFixPosY() const                      { return pThisTab->nFixPosY; }
    BOOL            IsPagebreakMode() const                 { return bPagebreak; }

    void            SetPosX( ScHSplitPos eWhich, USHORT nNewPosX );
    void            SetPosY( ScVSplitPos eWhich, USHORT nNewPosY );
    void            SetCurX( USHORT nNewCurX )                      { pThisTab->nCurX = nNewCurX; }
    void            SetCurY( USHORT nNewCurY )                      { pThisTab->nCurY = nNewCurY; }
    void            SetOldCursor( USHORT nNewX, USHORT nNewY );
    void            ResetOldCursor();
    void            SetHSplitMode( ScSplitMode eMode )              { pThisTab->eHSplitMode = eMode; }
    void            SetVSplitMode( ScSplitMode eMode )              { pThisTab->eVSplitMode = eMode; }
    void            SetHSplitPos( long nPos )                       { pThisTab->nHSplitPos = nPos; }
    void            SetVSplitPos( long nPos )                       { pThisTab->nVSplitPos = nPos; }
    void            SetFixPosX( USHORT nPos )                       { pThisTab->nFixPosX = nPos; }
    void            SetFixPosY( USHORT nPos )                       { pThisTab->nFixPosY = nPos; }
    void            SetPagebreakMode( BOOL bSet );

    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY );

    const Fraction& GetZoomX() const        { return bPagebreak ? aPageZoomX : aZoomX; }
    const Fraction& GetZoomY() const        { return bPagebreak ? aPageZoomY : aZoomY; }

    const MapMode&  GetLogicMode( ScSplitPos eWhich );
    const MapMode&  GetLogicMode();                     // Offset 0

    long            GetTPosX( ScHSplitPos eWhich ) const        { return pThisTab->nTPosX[eWhich]; }
    long            GetTPosY( ScVSplitPos eWhich ) const        { return pThisTab->nTPosY[eWhich]; }

    double          GetPPTX() const { return nPPTX; }
    double          GetPPTY() const { return nPPTY; }

    BOOL            GetSimpleArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rStartTab,
                                    USHORT& rEndCol, USHORT& rEndRow, USHORT& rEndTab,
                                    BOOL bMergeMark = TRUE );
    BOOL            GetSimpleArea( ScRange& rRange, BOOL bMergeMark = TRUE );
    void            GetMultiArea( ScRangeListRef& rRange, BOOL bMergeMark = TRUE );

    BOOL            SimpleColMarked();
    BOOL            SimpleRowMarked();

    void            SetFillMode( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow );
    void            SetDragMode( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    BYTE nMode );
    void            GetFillData( USHORT& rStartCol, USHORT& rStartRow,
                                 USHORT& rEndCol, USHORT& rEndRow );
    void            ResetFillMode();
    BOOL            IsAnyFillMode()             { return nFillMode != SC_FILL_NONE; }
    BOOL            IsFillMode()                { return nFillMode == SC_FILL_FILL; }
    BYTE            GetFillMode()               { return nFillMode; }

                    // TRUE: Zelle ist zusammengefasst
    BOOL            GetMergeSizePixel( USHORT nX, USHORT nY, long& rSizeXPix, long& rSizeYPix );
    BOOL            GetPosFromPixel( long nClickX, long nClickY, ScSplitPos eWhich,
                                        short& rPosX, short& rPosY,
                                        BOOL bTestMerge = TRUE, BOOL bRepair = FALSE,
                                        BOOL bNextIfLarge = TRUE );
    void            GetMouseQuadrant( const Point& rClickPos, ScSplitPos eWhich,
                                        short nPosX, short nPosY, BOOL& rLeft, BOOL& rTop );

    BOOL            IsRefMode() const                       { return bIsRefMode; }
    ScRefType       GetRefType() const                      { return eRefType; }
    USHORT          GetRefStartX() const                    { return nRefStartX; }
    USHORT          GetRefStartY() const                    { return nRefStartY; }
    USHORT          GetRefStartZ() const                    { return nRefStartZ; }
    USHORT          GetRefEndX() const                      { return nRefEndX; }
    USHORT          GetRefEndY() const                      { return nRefEndY; }
    USHORT          GetRefEndZ() const                      { return nRefEndZ; }

    void            SetRefMode( BOOL bNewMode, ScRefType eNewType )
                                    { bIsRefMode = bNewMode; eRefType = eNewType; }

    void            SetRefStart( USHORT nNewX, USHORT nNewY, USHORT nNewZ )
                            { nRefStartX = nNewX; nRefStartY = nNewY; nRefStartZ = nNewZ; }
    void            SetRefEnd( USHORT nNewX, USHORT nNewY, USHORT nNewZ )
                            { nRefEndX = nNewX; nRefEndY = nNewY; nRefEndZ = nNewZ; }

    void            ResetDelMark()                          { bDelMarkValid = FALSE; }
    void            SetDelMark( const ScRange& rRange )
                            { aDelRange = rRange; bDelMarkValid = TRUE; }

    BOOL            GetDelMark( ScRange& rRange ) const
                            { rRange = aDelRange; return bDelMarkValid; }

    inline void     GetMoveCursor( USHORT& rCurX, USHORT& rCurY );

    const ScViewOptions&    GetOptions() const { return *pOptions; }
    void                    SetOptions( const ScViewOptions& rOpt );

    BOOL    IsGridMode      () const            { return pOptions->GetOption( VOPT_GRID ); }
    void    SetGridMode     ( BOOL bNewMode )   { pOptions->SetOption( VOPT_GRID, bNewMode ); }
    BOOL    IsSyntaxMode    () const            { return pOptions->GetOption( VOPT_SYNTAX ); }
    void    SetSyntaxMode   ( BOOL bNewMode )   { pOptions->SetOption( VOPT_SYNTAX, bNewMode ); }
    BOOL    IsHeaderMode    () const            { return pOptions->GetOption( VOPT_HEADER ); }
    void    SetHeaderMode   ( BOOL bNewMode )   { pOptions->SetOption( VOPT_HEADER, bNewMode ); }
    BOOL    IsTabMode       () const            { return pOptions->GetOption( VOPT_TABCONTROLS ); }
    void    SetTabMode      ( BOOL bNewMode )   { pOptions->SetOption( VOPT_TABCONTROLS, bNewMode ); }
    BOOL    IsVScrollMode   () const            { return pOptions->GetOption( VOPT_VSCROLL ); }
    void    SetVScrollMode  ( BOOL bNewMode )   { pOptions->SetOption( VOPT_VSCROLL, bNewMode ); }
    BOOL    IsHScrollMode   () const            { return pOptions->GetOption( VOPT_HSCROLL ); }
    void    SetHScrollMode  ( BOOL bNewMode )   { pOptions->SetOption( VOPT_HSCROLL, bNewMode ); }
    BOOL    IsOutlineMode   () const            { return pOptions->GetOption( VOPT_OUTLINER ); }
    void    SetOutlineMode  ( BOOL bNewMode )   { pOptions->SetOption( VOPT_OUTLINER, bNewMode ); }

    void            KillEditView();
    void            ResetEditView();
    void            SetEditEngine( ScSplitPos eWhich,
                                    ScEditEngineDefaulter* pNewEngine,
                                    Window* pWin, USHORT nNewX, USHORT nNewY );
    void            GetEditView( ScSplitPos eWhich, EditView*& rViewPtr, USHORT& rCol, USHORT& rRow );
    BOOL            HasEditView( ScSplitPos eWhich ) const
                                        { return pEditView[eWhich] && bEditActive[eWhich]; }
    EditView*       GetEditView( ScSplitPos eWhich ) const
                                        { return pEditView[eWhich]; }

    void            EditGrowX();
    void            EditGrowY();

    USHORT          GetEditViewCol() const          { return nEditCol; }
    USHORT          GetEditViewRow() const          { return nEditRow; }
    USHORT          GetEditEndCol() const           { return nEditEndCol; }
    USHORT          GetEditEndRow() const           { return nEditEndRow; }

    Rectangle       GetEditArea( ScSplitPos eWhich, USHORT nPosX, USHORT nPosY, Window* pWin,
                                    const ScPatternAttr* pPattern, BOOL bForceToTop );

    void            SetTabNo( USHORT nNewTab );
    void            SetActivePart( ScSplitPos eNewActive );

    Point           GetScrPos( USHORT nWhereX, USHORT nWhereY, ScSplitPos eWhich,
                                BOOL bAllowNeg = FALSE ) const;
    Point           GetScrPos( USHORT nWhereX, USHORT nWhereY, ScHSplitPos eWhich ) const;
    Point           GetScrPos( USHORT nWhereX, USHORT nWhereY, ScVSplitPos eWhich ) const;

    USHORT          CellsAtX( short nPosX, short nDir, ScHSplitPos eWhichX, USHORT nScrSizeY = SC_SIZE_NONE ) const;
    USHORT          CellsAtY( short nPosY, short nDir, ScVSplitPos eWhichY, USHORT nScrSizeX = SC_SIZE_NONE ) const;

    USHORT          VisibleCellsX( ScHSplitPos eWhichX ) const;     // angezeigte komplette Zellen
    USHORT          VisibleCellsY( ScVSplitPos eWhichY ) const;
    USHORT          PrevCellsX( ScHSplitPos eWhichX ) const;        // Zellen auf der vorgehenden Seite
    USHORT          PrevCellsY( ScVSplitPos eWhichY ) const;
    USHORT          LastCellsX( ScHSplitPos eWhichX ) const;        // Zellen auf der letzten Seite
    USHORT          LastCellsY( ScVSplitPos eWhichY ) const;

    BOOL            IsOle();
    void            UpdateOle( ScSplitPos eWhich );
    void            SetScreen( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );
    void            SetScreen( const Rectangle& rVisArea );
    void            SetScreenPos( const Point& rVisAreaStart );

    void            UpdateScreenZoom( const Fraction& rNewX, const Fraction& rNewY );

    Size            GetScrSize() const              { return aScrSize; }

    void            RecalcPixPos();
    Point           GetPixPos( ScSplitPos eWhich ) const
                    { return Point( pThisTab->nPixPosX[WhichH(eWhich)],
                                    pThisTab->nPixPosY[WhichV(eWhich)] ); }
    void            SetSpellingView( EditView* pSpView) { pSpellingView = pSpView; }
    EditView*       GetSpellingView() const { return pSpellingView; }

    void            UpdateOutlinerFlags( Outliner& rOutl ) const;

    Point           GetMousePosPixel();

    BOOL            UpdateFixX(USHORT nTab = MAXTAB+1);
    BOOL            UpdateFixY(USHORT nTab = MAXTAB+1);

    USHORT          GetTabStartCol() const          { return nTabStartCol; }
    void            SetTabStartCol(USHORT nNew)     { nTabStartCol = nNew; }

    ScAddress       GetCurPos() const;

    const Size&     GetScenButSize() const              { return aScenButSize; }
    void            SetScenButSize(const Size& rNew)    { aScenButSize = rNew; }
};


// ---------------------------------------------------------------------------

inline void ScViewData::GetMoveCursor( USHORT& rCurX, USHORT& rCurY )
{
    if ( bIsRefMode )
    {
        rCurX = nRefEndX;
        rCurY = nRefEndY;
    }
    else
    {
        rCurX = GetCurX();
        rCurY = GetCurY();
    }
}

inline ScHSplitPos WhichH( ScSplitPos ePos )
{
    return (ePos==SC_SPLIT_TOPLEFT || ePos==SC_SPLIT_BOTTOMLEFT) ?
                SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
}

inline ScVSplitPos WhichV( ScSplitPos ePos )
{
    return (ePos==SC_SPLIT_TOPLEFT || ePos==SC_SPLIT_TOPRIGHT) ?
                SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
}

inline ScSplitPos Which( ScHSplitPos eHPos )
{
    return (eHPos==SC_SPLIT_LEFT) ?
                SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
}

inline ScSplitPos Which( ScVSplitPos eVPos )
{
    return (eVPos==SC_SPLIT_TOP) ?
                SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT;
}



#endif

