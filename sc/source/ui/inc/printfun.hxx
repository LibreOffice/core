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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PRINTFUN_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PRINTFUN_HXX

#include <memory>
#include <pagepar.hxx>
#include <editutil.hxx>

class SfxPrinter;
class ScDocShell;
class ScDocument;
class ScViewData;
class SfxItemSet;
class ScPageHFItem;
class EditTextObject;
class MultiSelection;
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

                                            // Settings for headers/footers
struct ScPrintHFParam
{
    bool                bEnable;
    bool                bDynamic;
    bool                bShared;
    long                nHeight;            // in total (height + distance + frames)
    long                nManHeight;         // set size (min when dynamic)
    sal_uInt16          nDistance;
    sal_uInt16          nLeft;              // edges
    sal_uInt16          nRight;
    const ScPageHFItem* pLeft;
    const ScPageHFItem* pRight;
    const SvxBoxItem*   pBorder;
    const SvxBrushItem* pBack;
    const SvxShadowItem* pShadow;
};

class ScPageRowEntry
{
private:
    SCROW                    nStartRow;
    SCROW                    nEndRow;
    size_t                   nPagesX;
    std::vector<bool>        aHidden;
    //!     Cache Number of really visible?

public:
            ScPageRowEntry()    { nStartRow = nEndRow = 0; nPagesX = 0; }

            ScPageRowEntry(const ScPageRowEntry& r);
    ScPageRowEntry& operator=(const ScPageRowEntry& r);

    SCROW   GetStartRow() const     { return nStartRow; }
    SCROW   GetEndRow() const       { return nEndRow; }
    size_t  GetPagesX() const       { return nPagesX; }
    void    SetStartRow(SCROW n)    { nStartRow = n; }
    void    SetEndRow(SCROW n)      { nEndRow = n; }

    void    SetPagesX(size_t nNew);
    void    SetHidden(size_t nX);
    bool    IsHidden(size_t nX) const;

    size_t  CountVisible() const;
};

namespace sc
{

struct PrintPageRangesInput
{
    bool m_bSkipEmpty;
    bool m_bPrintArea;
    SCROW m_nStartRow;
    SCROW m_nEndRow;
    SCCOL m_nStartCol;
    SCCOL m_nEndCol;
    SCTAB m_nPrintTab;
    Size  m_aDocSize;

    PrintPageRangesInput()
        : m_bSkipEmpty(false)
        , m_bPrintArea(false)
        , m_nStartRow(0)
        , m_nEndRow(0)
        , m_nStartCol(0)
        , m_nEndCol(0)
        , m_nPrintTab(0)
    {}
};

class PrintPageRanges
{
public:
    PrintPageRanges();

    std::vector<SCCOL> m_aPageEndX;
    std::vector<SCROW> m_aPageEndY;
    std::vector<ScPageRowEntry> m_aPageRows;

    size_t m_nPagesX;
    size_t m_nPagesY;
    size_t m_nTotalY;

    PrintPageRangesInput m_aInput;

    bool checkIfAlreadyCalculatedAndSet(bool bSkipEmpty, bool bPrintArea,
                                        SCROW nStartRow, SCROW nEndRow,
                                        SCCOL nStartCol, SCCOL nEndCol,
                                        SCTAB nPrintTab, Size const & aDocSize);

    void calculate(ScDocument* pDoc, bool bSkipEmpty, bool bPrintArea,
                   SCROW nStartRow, SCROW nEndRow, SCCOL nStartCol, SCCOL nEndCol,
                   SCTAB nPrintTab, Size const & aDocSize);
};

}

struct ScPrintState                         //  Save Variables from ScPrintFunc
{
    SCTAB   nPrintTab;
    SCCOL   nStartCol;
    SCROW   nStartRow;
    SCCOL   nEndCol;
    SCROW   nEndRow;
    bool    bPrintAreaValid; // the 4 variables above are set
    sal_uInt16  nZoom;
    size_t  nPagesX;
    size_t  nPagesY;
    long    nTabPages;
    long    nTotalPages;
    long    nPageStart;
    long    nDocPages;

    // Additional state of page ranges
    bool bSavedStateRanges;
    sc::PrintPageRangesInput aPrintPageRangesInput;
    size_t nTotalY;
    std::vector<SCCOL> aPageEndX;
    std::vector<SCROW> aPageEndY;
    std::vector<ScPageRowEntry> aPageRows;

    ScPrintState()
        : nPrintTab(0)
        , nStartCol(0)
        , nStartRow(0)
        , nEndCol(0)
        , nEndRow(0)
        , bPrintAreaValid(false)
        , nZoom(0)
        , nPagesX(0)
        , nPagesY(0)
        , nTabPages(0)
        , nTotalPages(0)
        , nPageStart(0)
        , nDocPages(0)
        , bSavedStateRanges(false)
        , nTotalY(0)
    {}
};

class ScPrintFunc
{
private:
    ScDocShell*         pDocShell;
    ScDocument*         pDoc;
    VclPtr<SfxPrinter>   pPrinter;
    VclPtr<OutputDevice> pDev;
    FmFormView*         pDrawView;

    MapMode             aOldPrinterMode;    //  MapMode before the call

    Point               aSrcOffset;         //  Paper-1/100 mm
    Point               aOffset;            //  scaled by a factor of page size
    sal_uInt16          nManualZoom;        //  Zoom in Preview (percent)
    bool                bClearWin;          //  Clear output before
    bool                bUseStyleColor;
    bool                bIsRender;

    SCTAB               nPrintTab;
    long                nPageStart;         //  Offset for the first page
    long                nDocPages;          //  Number of Pages in Document

    const ScRange*      pUserArea;          //  Selection, if set in dialog

    const SfxItemSet*   pParamSet;          //  Selected template
    bool                bFromPrintState;    // created from State-struct

                                            //  Parameter from template:
    sal_uInt16          nLeftMargin;
    sal_uInt16          nTopMargin;
    sal_uInt16          nRightMargin;
    sal_uInt16          nBottomMargin;
    bool                bCenterHor;
    bool                bCenterVer;
    bool                bLandscape;
    bool                bSourceRangeValid;

    SvxPageUsage        nPageUsage;
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
    sal_uInt16          nZoom;
    bool                bPrintCurrentTable;
    bool                bMultiArea;
    bool                mbHasPrintRange;
    long                nTabPages;
    long                nTotalPages;

    tools::Rectangle           aPageRect;          // Document Twips

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
    bool                bPrintAreaValid; // the 4 variables above are set

    sc::PrintPageRanges m_aRanges;

    std::unique_ptr<ScHeaderEditEngine> pEditEngine;
    std::unique_ptr<SfxItemSet>         pEditDefaults;

    ScHeaderFieldData   aFieldData;

    std::vector<ScAddress> aNotePosList;        // The order of notes

    ScPageBreakData*    pPageData;          // for recording the breaks etc.

public:
                    ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, SCTAB nTab,
                                 long nPage = 0, long nDocP = 0,
                                 const ScRange* pArea = nullptr,
                                 const ScPrintOptions* pOptions = nullptr,
                                 ScPageBreakData* pData = nullptr );

                    ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter,
                                const ScPrintState& rState, const ScPrintOptions* pOptions );

                    // ctors for device other than printer - for preview and pdf:

                    ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell, SCTAB nTab,
                                 long nPage = 0, long nDocP = 0,
                                 const ScRange* pArea = nullptr,
                                 const ScPrintOptions* pOptions = nullptr );

                    ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell,
                                 const ScPrintState& rState,
                                 const ScPrintOptions* pOptions );

                    ~ScPrintFunc();

    static void     DrawToDev( ScDocument* pDoc, OutputDevice* pDev, double nPrintFactor,
                                const tools::Rectangle& rBound, ScViewData* pViewData, bool bMetaFile );

    void            SetDrawView( FmFormView* pNew );

    void            SetOffset( const Point& rOfs );
    void            SetManualZoom( sal_uInt16 nNewZoom );
    void            SetDateTime( const DateTime& );

    void            SetClearFlag( bool bFlag );
    void            SetUseStyleColor( bool bFlag );
    void            SetRenderFlag( bool bFlag );

    void            SetExclusivelyDrawOleAndDrawObjects();//for printing selected objects without surrounding cell contents

    bool            UpdatePages();

    void            ApplyPrintSettings();       // Already called from DoPrint()
    long            DoPrint( const MultiSelection& rPageRanges,
                                long nStartPage, long nDisplayStart, bool bDoPrint,
                                ScPreviewLocationData* pLocationData );

                    // Query values - immediately

    const Size&     GetPageSize() const { return aPageSize; }
    Size            GetDataSize() const;
    void            GetScaleData( Size& rPhysSize, long& rDocHdr, long& rDocFtr );
    long            GetFirstPageNo() const  { return aTableParam.nFirstPageNo; }

    long            GetTotalPages() const { return nTotalPages; }
    sal_uInt16      GetZoom() const { return nZoom; }

    void            ResetBreaks( SCTAB nTab );

    void            GetPrintState(ScPrintState& rState, bool bSavePageRanges = false);
    bool            GetLastSourceRange( ScRange& rRange ) const;
    sal_uInt16      GetLeftMargin() const{return nLeftMargin;}
    sal_uInt16      GetRightMargin() const{return nRightMargin;}
    sal_uInt16      GetTopMargin() const{return nTopMargin;}
    sal_uInt16      GetBottomMargin() const{return nBottomMargin;}
    const ScPrintHFParam& GetHeader(){return aHdr;}
    const ScPrintHFParam& GetFooter(){return aFtr;}

    bool HasPrintRange() const { return mbHasPrintRange;}

private:
    void            Construct( const ScPrintOptions* pOptions );
    void            InitParam( const ScPrintOptions* pOptions );
    void            CalcZoom( sal_uInt16 nRangeNo );
    void            CalcPages();
    long            CountPages();
    long            CountNotePages();

    bool            AdjustPrintArea( bool bNew );

    Size            GetDocPageSize();

    long            TextHeight( const EditTextObject* pObject );
    void            MakeEditEngine();
    void            UpdateHFHeight( ScPrintHFParam& rParam );

    void            InitModes();

    bool            IsLeft( long nPageNo );
    bool            IsMirror( long nPageNo );
    void            MakeTableString();                  // sets aTableStr

    void            PrintPage( long nPageNo,
                                    SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    bool bDoPrint, ScPreviewLocationData* pLocationData );
    void            PrintArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    long nScrX, long nScrY,
                                    bool bShLeft, bool bShTop, bool bShRight, bool bShBottom );
    void            LocateArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    long nScrX, long nScrY, bool bRepCol, bool bRepRow,
                                    ScPreviewLocationData& rLocationData );
    void            PrintColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY );
    void            PrintRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY );
    void            LocateColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY,
                                bool bRepCol, ScPreviewLocationData& rLocationData );
    void            LocateRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY,
                                bool bRepRow, ScPreviewLocationData& rLocationData );
    void            PrintHF( long nPageNo, bool bHeader, long nStartY,
                                    bool bDoPrint, ScPreviewLocationData* pLocationData );

    long            PrintNotes( long nPageNo, long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData );
    long            DoNotes( long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData );

    void            DrawBorder( long nScrX, long nScrY, long nScrW, long nScrH,
                                    const SvxBoxItem* pBorderData,
                                    const SvxBrushItem* pBackground,
                                    const SvxShadowItem* pShadow );

    void            FillPageData();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
