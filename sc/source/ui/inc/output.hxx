/*************************************************************************
 *
 *  $RCSfile: output.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:00 $
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

#ifndef SC_OUTPUT_HXX
#define SC_OUTPUT_HXX

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _FRACT_HXX //autogen
#include <tools/fract.hxx>
#endif

class Rectangle;
class Font;
class OutputDevice;
class Window;
class EditEngine;
class ScDocument;
class ScBaseCell;
class ScPatternAttr;
class SvxMarginItem;
class SdrObject;
class SdrOle2Obj;
struct RowInfo;
class ScTabViewShell;
class SvInPlaceObjectRef;
class ScPageBreakData;

// ---------------------------------------------------------------------------

#define SC_SCENARIO_HSPACE      60
#define SC_SCENARIO_VSPACE      50

//  Twips (Hoehe wird passend in Pixeln berechnet)
#define SC_CLIPMARK_SIZE    64

// ---------------------------------------------------------------------------

#define SC_OBJECTS_NONE         0
#define SC_OBJECTS_DRAWING      1
#define SC_OBJECTS_OLE          2
#define SC_OBJECTS_CHARTS       4
#define SC_OBJECTS_ALL          ( SC_OBJECTS_DRAWING | SC_OBJECTS_OLE | SC_OBJECTS_CHARTS )

enum ScOutputType { OUTTYPE_WINDOW, OUTTYPE_PRINTER };

class ScOutputData
{
friend class ScDrawStringsVars;
private:
    OutputDevice* pDev;         // Device
    OutputDevice* pRefDevice;   // Drucker bei Preview
    RowInfo* pRowInfo;          // Info-Block
    USHORT nArrCount;           // belegte Zeilen im Info-Block
    ScDocument* pDoc;           // Dokument
    USHORT nTab;                // Tabelle
    long nScrX;                 // Ausgabe Startpos. (Pixel)
    long nScrY;
    long nScrW;                 // Ausgabe Groesse (Pixel)
    long nScrH;
    USHORT nX1;                 // Start-/Endkoordinaten
    USHORT nY1;                 //  ( incl. versteckte )
    USHORT nX2;
    USHORT nY2;
    USHORT nVisX1;              // Start-/Endkoordinaten
    USHORT nVisY1;              //  ( sichtbarer Bereich )
    USHORT nVisX2;
    USHORT nVisY2;
    ScOutputType eType;         // Bildschirm/Drucker ...
    double nPPTX;               // Pixel per Twips
    double nPPTY;
//  USHORT nZoom;               // Zoom-Faktor (Prozent) - fuer GetFont
    Fraction aZoomX;
    Fraction aZoomY;

    SdrObject* pEditObj;        // beim Painten auslassen

    ScTabViewShell* pViewShell; // zum Connecten von sichtbaren Plug-Ins

    BOOL bEditMode;             // InPlace editierte Zelle - nicht ausgeben
    USHORT nEditCol;
    USHORT nEditRow;

    BOOL bMetaFile;             // Ausgabe auf Metafile (nicht in Pixeln!)
    BOOL bSingleGrid;           // beim Gitter bChanged auswerten

    BOOL bPagebreakMode;        // Seitenumbruch-Vorschau
    BOOL bSolidBackground;      // weiss statt transparent

    BOOL bSyntaxMode;           // Syntax-Highlighting
    Color* pValueColor;
    Color* pTextColor;
    Color* pFormulaColor;

    Color   aGridColor;

    BOOL    bShowNullValues;
    BOOL    bShowFormulas;
    BOOL    bShowSpellErrors;   // Spell-Errors in EditObjekten anzeigen
    BOOL    bMarkClipped;

    BOOL    bSnapPixel;

    BOOL    bAnyRotated;        // intern
    BOOL    bAnyClipped;        // intern

                            // private Methoden

    BOOL            GetMergeOrigin( USHORT nX, USHORT nY, USHORT nArrY,
                                    USHORT& rOverX, USHORT& rOverY,
                                    long& rVirtPosX, long& rVirtPosY,
                                    BOOL bClipVirt, BOOL bVisRowChanged );

    void            SetSyntaxColor( Font* pFont, ScBaseCell* pCell );
    void            SetEditSyntaxColor( EditEngine& rEngine, ScBaseCell* pCell );

    void            ConnectObject( const SvInPlaceObjectRef& rRef, SdrOle2Obj* pOleObj );

    double          GetStretch();

public:
                    ScOutputData( OutputDevice* pNewDev, ScOutputType eNewType,
                                    RowInfo* pNewRowInfo, USHORT nNewCount,
                                    ScDocument* pNewDoc,
                                    USHORT nNewTab, long nNewScrX, long nNewScrY,
                                    USHORT nNewX1, USHORT nNewY1, USHORT nNewX2, USHORT nNewY2,
                                    double nPixelPerTwipsX, double nPixelPerTwipsY,
                                    const Fraction* pZoomX = NULL,
                                    const Fraction* pZoomY = NULL );

                    ~ScOutputData();

    void    SetRefDevice( OutputDevice* pRDev ) { pRefDevice = pRDev; }
    void    SetEditObject( SdrObject* pObj )    { pEditObj = pObj; }
    void    SetViewShell( ScTabViewShell* pSh ) { pViewShell = pSh; }

    void    SetSolidBackground( BOOL bSet )     { bSolidBackground = bSet; }

    void    SetEditCell( USHORT nCol, USHORT nRow );
    void    SetSyntaxMode( BOOL bNewMode );
    void    SetMetaFileMode( BOOL bNewMode );
    void    SetSingleGrid( BOOL bNewMode );
    void    SetGridColor( const Color& rColor );
    void    SetMarkClipped( BOOL bSet );
    void    SetShowNullValues ( BOOL bSet = TRUE );
    void    SetShowFormulas   ( BOOL bSet = TRUE );
    void    SetShowSpellErrors( BOOL bSet = TRUE );
    long    GetScrW() const     { return nScrW; }
    long    GetScrH() const     { return nScrH; }

    void    SetSnapPixel( BOOL bSet = TRUE );

    void    DrawGrid( BOOL bGrid, BOOL bPage );
    void    DrawStrings();
    void    DrawBackground();
    void    DrawShadow();
    void    DrawExtraShadow(BOOL bLeft, BOOL bTop, BOOL bRight, BOOL bBottom);
    void    DrawFrame();

                    // im logischen MapMode !
                    //! Scale-Parameter koennen raus (immer 1.0) !!!!
    void    DrawEdit(BOOL bPixelToLogic, double nScaleX=1.0, double nScaleY=1.0);

    void    FindRotated();
    void    DrawRotatedFrame();                     // Pixel
    void    DrawRotated(BOOL bPixelToLogic);        // logisch

    void    DrawClear();
    void    DrawPageBorder( USHORT nStartX, USHORT nStartY, USHORT nEndX, USHORT nEndY );
    void    DrawingLayer( USHORT nLayer, USHORT nObjectFlags, long nLogStX, long nLogStY );

                                                    // nur Bildschirm:

    void    DrawingSingle( USHORT nLayer, USHORT nObjectFlags, USHORT nDummyFlags );

    void    DrawSelectiveObjects( USHORT nLayer, const Rectangle& rRect,
                                    USHORT nObjectFlags, USHORT nDummyFlags = 0 );

    BOOL    SetChangedClip();       // FALSE = nix

    void    FindChanged();
    void    SetPagebreakMode( ScPageBreakData* pPageData );
    void    DrawMark( Window* pWin );
    void    DrawRefMark( USHORT nRefStartX, USHORT nRefStartY,
                         USHORT nRefEndX, USHORT nRefEndY,
                         const Color& rColor, BOOL bHandle );
    void    DrawOneChange( USHORT nRefStartX, USHORT nRefStartY,
                            USHORT nRefEndX, USHORT nRefEndY,
                            const Color& rColor, USHORT nType );
    void    DrawChangeTrack();
    void    DrawClipMarks();

    void    DrawNoteMarks();
    void    PrintNoteMarks( const List& rPosList );     // Liste von ScTripeln
};



#endif

