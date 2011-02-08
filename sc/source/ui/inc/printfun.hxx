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

#ifndef SC_PRINTFUN_HXX
#define SC_PRINTFUN_HXX


#include "pagepar.hxx"
#include "editutil.hxx"

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif

class SfxPrinter;
class SfxProgress;
class ScDocShell;
class ScDocument;
class ScViewData;
class SfxItemSet;
class ScPageHFItem;
class EditTextObject;
class MultiSelection;
class ScHeaderEditEngine;
class ScPageBreakData;
class ScPreviewLocationData;
class ScPrintOptions;
class SvxBoxItem;
class SvxBrushItem;
class SvxShadowItem;
class FmFormView;

#define RANGENO_NORANGE             USHRT_MAX

#define PRINT_HEADER_WIDTH          (1.0 * TWIPS_PER_CM)
#define PRINT_HEADER_HEIGHT         (12.8 * TWIPS_PER_POINT)
#define PRINT_HEADER_FONTHEIGHT     200


                                            //  Einstellungen fuer Kopf-/Fusszeilen
struct ScPrintHFParam
{
    BOOL                bEnable;
    BOOL                bDynamic;
    BOOL                bShared;
    long                nHeight;            //  insgesamt (Hoehe+Abstand+Rahmen)
    long                nManHeight;         //  eingestellte Groesse (Min. bei dynamisch)
    USHORT              nDistance;
    USHORT              nLeft;              //  Raender
    USHORT              nRight;
    const ScPageHFItem* pLeft;
    const ScPageHFItem* pRight;
    const SvxBoxItem*   pBorder;
    const SvxBrushItem* pBack;
    const SvxShadowItem* pShadow;
};


// "Ersatz" fuer SV-JobSetup:

class ScJobSetup
{
public:
    ScJobSetup( SfxPrinter* pPrinter );

    Size        aUserSize;
    MapMode     aUserMapMode;
    Paper   ePaper;
    Orientation eOrientation;
    USHORT      nPaperBin;
};

struct ScPrintState                         //  Variablen aus ScPrintFunc retten
{
    SCTAB   nPrintTab;
    SCCOL   nStartCol;
    SCROW   nStartRow;
    SCCOL   nEndCol;
    SCROW   nEndRow;
    USHORT  nZoom;
    size_t  nPagesX;
    size_t  nPagesY;
    long    nTabPages;
    long    nTotalPages;
    long    nPageStart;
    long    nDocPages;
};

class ScPageRowEntry
{
private:
    SCROW   nStartRow;
    SCROW   nEndRow;
    size_t  nPagesX;
    BOOL*   pHidden;
    //!     Anzahl wirklich sichtbarer cachen???

public:
            ScPageRowEntry()    { nStartRow = nEndRow = 0; nPagesX = 0; pHidden = NULL; }
            ~ScPageRowEntry()   { delete[] pHidden; }

            ScPageRowEntry(const ScPageRowEntry& r);
    const ScPageRowEntry& operator=(const ScPageRowEntry& r);

    SCROW   GetStartRow() const     { return nStartRow; }
    SCROW   GetEndRow() const       { return nEndRow; }
    size_t  GetPagesX() const       { return nPagesX; }
    void    SetStartRow(SCROW n)    { nStartRow = n; }
    void    SetEndRow(SCROW n)      { nEndRow = n; }

    void    SetPagesX(size_t nNew);
    void    SetHidden(size_t nX);
    BOOL    IsHidden(size_t nX) const;

    size_t  CountVisible() const;
};

class ScPrintFunc
{
private:
    ScDocShell*         pDocShell;
    ScDocument*         pDoc;
    SfxPrinter*         pPrinter;
    OutputDevice*       pDev;
    FmFormView*         pDrawView;

    MapMode             aOldPrinterMode;    //  MapMode vor dem Aufruf

    Point               aSrcOffset;         //  Papier-1/100 mm
    Point               aOffset;            //  mit Faktor aus Seitenformat skaliert
    USHORT              nManualZoom;        //  Zoom in Preview (Prozent)
    BOOL                bClearWin;          //  Ausgabe vorher loeschen
    BOOL                bUseStyleColor;
    BOOL                bIsRender;

    SCTAB               nPrintTab;
    long                nPageStart;         //  Offset fuer erste Seite
    long                nDocPages;          //  Seiten im Dokument

    const ScRange*      pUserArea;          //  Selektion, wenn im Dialog eingestellt

    const SfxItemSet*   pParamSet;          //  eingestellte Vorlage
    BOOL                bState;             //  aus State-struct erzeugt

                                            //  Parameter aus Vorlage:
    USHORT              nLeftMargin;
    USHORT              nTopMargin;
    USHORT              nRightMargin;
    USHORT              nBottomMargin;
    BOOL                bCenterHor;
    BOOL                bCenterVer;
    BOOL                bLandscape;
    BOOL                bSourceRangeValid;

    USHORT              nPageUsage;
    Size                aPageSize;          //  Drucker-Twips
    const SvxBoxItem*   pBorderItem;
    const SvxBrushItem* pBackgroundItem;
    const SvxShadowItem* pShadowItem;

    ScRange             aLastSourceRange;
    ScPrintHFParam      aHdr;
    ScPrintHFParam      aFtr;
    ScPageTableParam    aTableParam;
    ScPageAreaParam     aAreaParam;

                                            //  berechnete Werte:
    USHORT              nZoom;
    BOOL                bPrintCurrentTable;
    BOOL                bMultiArea;
    long                nTabPages;
    long                nTotalPages;

    Rectangle           aPageRect;          //  Dokument-Twips

    MapMode             aLogicMode;         //  in DoPrint gesetzt
    MapMode             aOffsetMode;
    MapMode             aTwipMode;
    double              nScaleX;
    double              nScaleY;

    SCCOL               nRepeatStartCol;
    SCCOL               nRepeatEndCol;
    SCROW               nRepeatStartRow;
    SCROW               nRepeatEndRow;

    SCCOL               nStartCol;
    SCROW               nStartRow;
    SCCOL               nEndCol;
    SCROW               nEndRow;

    SCCOL*              pPageEndX;          // Seitenaufteilung
    SCROW*              pPageEndY;
    ScPageRowEntry*     pPageRows;
    size_t              nPagesX;
    size_t              nPagesY;
    size_t              nTotalY;

    ScHeaderEditEngine* pEditEngine;
    SfxItemSet*         pEditDefaults;

    ScHeaderFieldData   aFieldData;

    List                aNotePosList;       //  Reihenfolge der Notizen

    ScPageBreakData*    pPageData;          // zum Eintragen der Umbrueche etc.

public:
                    ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, SCTAB nTab,
                                 long nPage = 0, long nDocP = 0,
                                 const ScRange* pArea = NULL,
                                 const ScPrintOptions* pOptions = NULL,
                                 ScPageBreakData* pData = NULL );

                    // ctors for device other than printer - for preview and pdf:

                    ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell, SCTAB nTab,
                                 long nPage = 0, long nDocP = 0,
                                 const ScRange* pArea = NULL,
                                 const ScPrintOptions* pOptions = NULL );

                    ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell,
                                 const ScPrintState& rState,
                                 const ScPrintOptions* pOptions );

                    ~ScPrintFunc();

    static void     DrawToDev( ScDocument* pDoc, OutputDevice* pDev, double nPrintFactor,
                                const Rectangle& rBound, ScViewData* pViewData, BOOL bMetaFile );

    void            SetDrawView( FmFormView* pNew );

    void            SetOffset( const Point& rOfs );
    void            SetManualZoom( USHORT nNewZoom );
    void            SetDateTime( const Date& rDate, const Time& rTime );

    void            SetClearFlag( BOOL bFlag );
    void            SetUseStyleColor( BOOL bFlag );
    void            SetRenderFlag( BOOL bFlag );

    void            SetExclusivelyDrawOleAndDrawObjects();//for printing selected objects without surrounding cell contents

    BOOL            UpdatePages();

    void            ApplyPrintSettings();       // aus DoPrint() schon gerufen
    long            DoPrint( const MultiSelection& rPageRanges,
                                long nStartPage, long nDisplayStart, BOOL bDoPrint,
                                ScPreviewLocationData* pLocationData );

                    //  Werte abfragen - sofort

    Size            GetPageSize() const { return aPageSize; }
    Size            GetDataSize() const;
    void            GetScaleData( Size& rPhysSize, long& rDocHdr, long& rDocFtr );
    long            GetFirstPageNo() const  { return aTableParam.nFirstPageNo; }

                    //  letzte Werte abfragen - nach DoPrint !!!

    double          GetScaleX() const { return nScaleX; }
    double          GetScaleY() const { return nScaleY; }
    long            GetTotalPages() const { return nTotalPages; }
    USHORT          GetZoom() const { return nZoom; }

    void            ResetBreaks( SCTAB nTab );

    void            GetPrintState( ScPrintState& rState );
    BOOL            GetLastSourceRange( ScRange& rRange ) const;
    USHORT          GetLeftMargin() const{return nLeftMargin;}
    USHORT          GetRightMargin() const{return nRightMargin;}
    USHORT          GetTopMargin() const{return nTopMargin;}
    USHORT          GetBottomMargin() const{return nBottomMargin;}
    void            SetLeftMargin(USHORT nRulerLeftDistance){ nLeftMargin = nRulerLeftDistance; }
    void            SetRightMargin(USHORT nRulerRightDistance){ nRightMargin = nRulerRightDistance; }
    void            SetTopMargin(USHORT nRulerTopDistance){ nTopMargin = nRulerTopDistance; }
    void            SetBottomMargin(USHORT nRulerBottomDistance){ nBottomMargin = nRulerBottomDistance; }
    ScPrintHFParam  GetHeader(){return aHdr;}
    ScPrintHFParam  GetFooter(){return aFtr;}

private:
    void            Construct( const ScPrintOptions* pOptions );
    void            InitParam( const ScPrintOptions* pOptions );
    void            CalcZoom( USHORT nRangeNo );
    void            CalcPages();
    long            CountPages();
    long            CountNotePages();

    BOOL            AdjustPrintArea( BOOL bNew );

    Size            GetDocPageSize();

    long            TextHeight( const EditTextObject* pObject );
    void            MakeEditEngine();
    void            UpdateHFHeight( ScPrintHFParam& rParam );

    void            InitModes();

    BOOL            IsLeft( long nPageNo );
    BOOL            IsMirror( long nPageNo );
    void            ReplaceFields( long nPageNo );      // aendert Text in pEditEngine
    void            MakeTableString();                  // setzt aTableStr

    void            PrintPage( long nPageNo,
                                    SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    BOOL bDoPrint, ScPreviewLocationData* pLocationData );
    void            PrintArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    long nScrX, long nScrY,
                                    BOOL bShLeft, BOOL bShTop, BOOL bShRight, BOOL bShBottom );
    void            LocateArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    long nScrX, long nScrY, BOOL bRepCol, BOOL bRepRow,
                                    ScPreviewLocationData& rLocationData );
    void            PrintColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY );
    void            PrintRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY );
    void            LocateColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY,
                                BOOL bRepCol, ScPreviewLocationData& rLocationData );
    void            LocateRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY,
                                BOOL bRepRow, ScPreviewLocationData& rLocationData );
    void            PrintHF( long nPageNo, BOOL bHeader, long nStartY,
                                    BOOL bDoPrint, ScPreviewLocationData* pLocationData );

    long            PrintNotes( long nPageNo, long nNoteStart, BOOL bDoPrint, ScPreviewLocationData* pLocationData );
    long            DoNotes( long nNoteStart, BOOL bDoPrint, ScPreviewLocationData* pLocationData );

    void            DrawBorder( long nScrX, long nScrY, long nScrW, long nScrH,
                                    const SvxBoxItem* pBorderData,
                                    const SvxBrushItem* pBackground,
                                    const SvxShadowItem* pShadow );

    void            FillPageData();
};



#endif

