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

#ifndef SC_PRINTFUN_HXX
#define SC_PRINTFUN_HXX


#include "pagepar.hxx"
#include "editutil.hxx"

#include <vcl/print.hxx>

class SfxPrinter;
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


                                            // Settings for headers/footers
struct ScPrintHFParam
{
    sal_Bool                bEnable;
    sal_Bool                bDynamic;
    sal_Bool                bShared;
    long                nHeight;            // in total (height + distance + frames)
    long                nManHeight;         // set size (min when dynamic)
  sal_uInt16              nDistance;
    sal_uInt16              nLeft;              // edges
    sal_uInt16              nRight;
    const ScPageHFItem* pLeft;
    const ScPageHFItem* pRight;
    const SvxBoxItem*   pBorder;
    const SvxBrushItem* pBack;
    const SvxShadowItem* pShadow;
};


struct ScPrintState                         //  Save Variables from ScPrintFunc
{
    SCTAB   nPrintTab;
    SCCOL   nStartCol;
    SCROW   nStartRow;
    SCCOL   nEndCol;
    SCROW   nEndRow;
    sal_uInt16  nZoom;
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
    sal_Bool*   pHidden;
    //!     Cache Number of really visible?

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
    sal_Bool    IsHidden(size_t nX) const;

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

    MapMode             aOldPrinterMode;    //  MapMode befor the call

    Point               aSrcOffset;         //  Paper-1/100 mm
    Point               aOffset;            //  scaled by a factor of page size
    sal_uInt16              nManualZoom;        //  Zoom in Preview (percent)
    sal_Bool                bClearWin;          //  Clear output before
    sal_Bool                bUseStyleColor;
    sal_Bool                bIsRender;

    SCTAB               nPrintTab;
    long                nPageStart;         //  Offset for the first page
    long                nDocPages;          //  Number of Pages in Document

    const ScRange*      pUserArea;          //  Selection, if set in dialog

    const SfxItemSet*   pParamSet;          //  Selected template
    sal_Bool                bState;             // created from State-struct

                                            //  Parameter from template:
    sal_uInt16              nLeftMargin;
    sal_uInt16              nTopMargin;
    sal_uInt16              nRightMargin;
    sal_uInt16              nBottomMargin;
    sal_Bool                bCenterHor;
    sal_Bool                bCenterVer;
    sal_Bool                bLandscape;
    sal_Bool                bSourceRangeValid;

    sal_uInt16              nPageUsage;
    Size                aPageSize;          // Printer Twips
    const SvxBoxItem*   pBorderItem;
    const SvxBrushItem* pBackgroundItem;
    const SvxShadowItem* pShadowItem;

    ScRange             aLastSourceRange;
    ScPrintHFParam      aHdr;
    ScPrintHFParam      aFtr;
    ScPageTableParam    aTableParam;
    ScPageAreaParam     aAreaParam;

                                            // Calculated values:
    sal_uInt16              nZoom;
    sal_Bool                bPrintCurrentTable;
    sal_Bool                bMultiArea;
    long                nTabPages;
    long                nTotalPages;

    Rectangle           aPageRect;          // Document Twips

    MapMode             aLogicMode;         // Set in DoPrint
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

    SCCOL*              pPageEndX;          // page layout
    SCROW*              pPageEndY;
    ScPageRowEntry*     pPageRows;
    size_t              nPagesX;
    size_t              nPagesY;
    size_t              nTotalY;

    ScHeaderEditEngine* pEditEngine;
    SfxItemSet*         pEditDefaults;

    ScHeaderFieldData   aFieldData;

    std::vector<ScAddress> aNotePosList;        // The order of notes

    ScPageBreakData*    pPageData;          // for recording the breaks etc.

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
                                const Rectangle& rBound, ScViewData* pViewData, sal_Bool bMetaFile );

    void            SetDrawView( FmFormView* pNew );

    void            SetOffset( const Point& rOfs );
    void            SetManualZoom( sal_uInt16 nNewZoom );
    void            SetDateTime( const Date& rDate, const Time& rTime );

    void            SetClearFlag( sal_Bool bFlag );
    void            SetUseStyleColor( sal_Bool bFlag );
    void            SetRenderFlag( sal_Bool bFlag );

    void            SetExclusivelyDrawOleAndDrawObjects();//for printing selected objects without surrounding cell contents

    sal_Bool            UpdatePages();

    void            ApplyPrintSettings();       // Already called from DoPrint()
    long            DoPrint( const MultiSelection& rPageRanges,
                                long nStartPage, long nDisplayStart, bool bDoPrint,
                                ScPreviewLocationData* pLocationData );

                    // Query values - immediately

    Size            GetPageSize() const { return aPageSize; }
    Size            GetDataSize() const;
    void            GetScaleData( Size& rPhysSize, long& rDocHdr, long& rDocFtr );
    long            GetFirstPageNo() const  { return aTableParam.nFirstPageNo; }

                    // Query last value - after DoPrint!!

    double          GetScaleX() const { return nScaleX; }
    double          GetScaleY() const { return nScaleY; }
    long            GetTotalPages() const { return nTotalPages; }
    sal_uInt16          GetZoom() const { return nZoom; }

    void            ResetBreaks( SCTAB nTab );

    void            GetPrintState( ScPrintState& rState );
    sal_Bool            GetLastSourceRange( ScRange& rRange ) const;
    sal_uInt16          GetLeftMargin() const{return nLeftMargin;}
    sal_uInt16          GetRightMargin() const{return nRightMargin;}
    sal_uInt16          GetTopMargin() const{return nTopMargin;}
    sal_uInt16          GetBottomMargin() const{return nBottomMargin;}
    void            SetLeftMargin(sal_uInt16 nRulerLeftDistance){ nLeftMargin = nRulerLeftDistance; }
    void            SetRightMargin(sal_uInt16 nRulerRightDistance){ nRightMargin = nRulerRightDistance; }
    void            SetTopMargin(sal_uInt16 nRulerTopDistance){ nTopMargin = nRulerTopDistance; }
    void            SetBottomMargin(sal_uInt16 nRulerBottomDistance){ nBottomMargin = nRulerBottomDistance; }
    ScPrintHFParam  GetHeader(){return aHdr;}
    ScPrintHFParam  GetFooter(){return aFtr;}

private:
    void            Construct( const ScPrintOptions* pOptions );
    void            InitParam( const ScPrintOptions* pOptions );
    void            CalcZoom( sal_uInt16 nRangeNo );
    void            CalcPages();
    long            CountPages();
    long            CountNotePages();

    sal_Bool            AdjustPrintArea( sal_Bool bNew );

    Size            GetDocPageSize();

    long            TextHeight( const EditTextObject* pObject );
    void            MakeEditEngine();
    void            UpdateHFHeight( ScPrintHFParam& rParam );

    void            InitModes();

    sal_Bool            IsLeft( long nPageNo );
    sal_Bool            IsMirror( long nPageNo );
    void            ReplaceFields( long nPageNo );      // changes Text in pEditEngine
    void            MakeTableString();                  // sets aTableStr

    void            PrintPage( long nPageNo,
                                    SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );
    void            PrintArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    long nScrX, long nScrY,
                                    sal_Bool bShLeft, sal_Bool bShTop, sal_Bool bShRight, sal_Bool bShBottom );
    void            LocateArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    long nScrX, long nScrY, sal_Bool bRepCol, sal_Bool bRepRow,
                                    ScPreviewLocationData& rLocationData );
    void            PrintColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY );
    void            PrintRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY );
    void            LocateColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY,
                                sal_Bool bRepCol, ScPreviewLocationData& rLocationData );
    void            LocateRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY,
                                sal_Bool bRepRow, ScPreviewLocationData& rLocationData );
    void            PrintHF( long nPageNo, sal_Bool bHeader, long nStartY,
                                    sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );

    long            PrintNotes( long nPageNo, long nNoteStart, sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );
    long            DoNotes( long nNoteStart, sal_Bool bDoPrint, ScPreviewLocationData* pLocationData );

    void            DrawBorder( long nScrX, long nScrY, long nScrW, long nScrH,
                                    const SvxBoxItem* pBorderData,
                                    const SvxBrushItem* pBackground,
                                    const SvxShadowItem* pShadow );

    void            FillPageData();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
