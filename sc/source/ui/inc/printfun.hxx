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

#include <memory>
#include <map>
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

constexpr sal_Int64 PRINT_HEADER_WIDTH = o3tl::toTwips(1, o3tl::Length::cm);
constexpr sal_Int64 PRINT_HEADER_HEIGHT = o3tl::toTwips(12.8, o3tl::Length::pt);

                                            // Settings for headers/footers
struct ScPrintHFParam
{
    bool                bEnable;
    bool                bDynamic;
    bool                bShared;
    bool                bSharedFirst;
    tools::Long                nHeight;            // in total (height + distance + frames)
    tools::Long                nManHeight;         // set size (min when dynamic)
    sal_uInt16          nDistance;
    sal_uInt16          nLeft;              // edges
    sal_uInt16          nRight;
    const ScPageHFItem* pLeft;
    const ScPageHFItem* pRight;
    const ScPageHFItem* pFirst;
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
    ScRange m_aRange;
    Size  m_aDocSize;

    PrintPageRangesInput()
        : m_bSkipEmpty(false)
        , m_bPrintArea(false)
    {}

    PrintPageRangesInput(bool bSkipEmpty, bool bPrintArea, ScRange const& rRange, Size const& rDocSize)
        : m_bSkipEmpty(bSkipEmpty)
        , m_bPrintArea(bPrintArea)
        , m_aRange(rRange)
        , m_aDocSize(rDocSize)
    {}

    bool operator==(PrintPageRangesInput const& rInput) const
    {
        return
            m_bSkipEmpty == rInput.m_bSkipEmpty &&
            m_bPrintArea == rInput.m_bPrintArea &&
            m_aRange == rInput.m_aRange &&
            m_aDocSize   == rInput.m_aDocSize;
    }

    PrintPageRangesInput& operator=(PrintPageRangesInput const& rInput)
    {
        m_bSkipEmpty = rInput.m_bSkipEmpty;
        m_bPrintArea = rInput.m_bPrintArea;
        m_aRange = rInput.m_aRange;
        m_aDocSize = rInput.m_aDocSize;

        return *this;
    }

    SCROW getStartRow() const
    {
        return m_aRange.aStart.Row();
    }

    SCROW getEndRow() const
    {
        return m_aRange.aEnd.Row();
    }

    SCCOL getStartColumn() const
    {
        return m_aRange.aStart.Col();
    }

    SCCOL getEndColumn() const
    {
        return m_aRange.aEnd.Col();
    }

    SCTAB getPrintTab() const
    {
        return m_aRange.aStart.Tab();
    }
};

class PrintPageRanges
{
public:
    PrintPageRanges();

    // use shared_ptr to avoid copying this (potentially large) data back and forth
    std::shared_ptr<std::vector<SCCOL>> m_xPageEndX;
    std::shared_ptr<std::vector<SCROW>> m_xPageEndY;
    std::shared_ptr<std::map<size_t, ScPageRowEntry>> m_xPageRows;

    size_t m_nPagesX;
    size_t m_nPagesY;
    size_t m_nTotalY;

    PrintPageRangesInput m_aInput;

    void calculate(ScDocument& rDoc, PrintPageRangesInput const& rInput);
};

} // end sc namespace

// Used to save expensive-to-compute data from ScPrintFunc in between
// uses of ScPrintFunc
struct ScPrintState
{
    SCTAB   nPrintTab;
    SCCOL   nStartCol;
    SCROW   nStartRow;
    SCCOL   nEndCol;
    SCROW   nEndRow;
    bool    bPrintAreaValid; // the 4 variables above are set
    sal_uInt16  nZoom;
    tools::Long    nTabPages;
    tools::Long    nTotalPages;
    tools::Long    nPageStart;
    tools::Long    nDocPages;

    // Additional state of page ranges
    sc::PrintPageRanges m_aRanges;

    ScPrintState()
        : nPrintTab(0)
        , nStartCol(0)
        , nStartRow(0)
        , nEndCol(0)
        , nEndRow(0)
        , bPrintAreaValid(false)
        , nZoom(0)
        , nTabPages(0)
        , nTotalPages(0)
        , nPageStart(0)
        , nDocPages(0)
    {}
};

class ScPrintFunc
{
private:
    ScDocShell*         pDocShell;
    ScDocument&         rDoc;
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
    tools::Long                nPageStart;         //  Offset for the first page
    tools::Long                nDocPages;          //  Number of Pages in Document

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
    tools::Long                nTabPages;
    tools::Long                nTotalPages;

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

    Size                aPrintPageSize;         // print page size in Print dialog
    bool                bPrintPageLandscape;    // print page orientation in Print dialog
    bool                bUsePrintDialogSetting; // use Print dialog setting

public:
                    ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, SCTAB nTab,
                                 tools::Long nPage = 0, tools::Long nDocP = 0,
                                 const ScRange* pArea = nullptr,
                                 const ScPrintOptions* pOptions = nullptr,
                                 ScPageBreakData* pData = nullptr,
                                 Size aPrintPageSize = {},
                                 bool bPrintPageLandscape = false,
                                 bool bUsePrintDialogSetting = false );

                    ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter,
                                const ScPrintState& rState, const ScPrintOptions* pOptions,
                                Size aPrintPageSize = {},
                                bool bPrintPageLandscape = false,
                                bool bUsePrintDialogSetting = false );

                    // ctors for device other than printer - for preview and pdf:

                    ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell, SCTAB nTab,
                                 tools::Long nPage = 0, tools::Long nDocP = 0,
                                 const ScRange* pArea = nullptr,
                                 const ScPrintOptions* pOptions = nullptr );

                    ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell,
                                 const ScPrintState& rState,
                                 const ScPrintOptions* pOptions, Size aPrintPageSize = {},
                                 bool bPrintPageLandscape = false,
                                 bool bUsePrintDialogSetting = false);

                    ~ScPrintFunc();

    static void     DrawToDev( ScDocument& rDoc, OutputDevice* pDev, double nPrintFactor,
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
    tools::Long            DoPrint( const MultiSelection& rPageRanges,
                                tools::Long nStartPage, tools::Long nDisplayStart, bool bDoPrint,
                                ScPreviewLocationData* pLocationData );

                    // Query values - immediately

    const Size&     GetPageSize() const { return aPageSize; }
    Size            GetDataSize() const;
    void            GetScaleData( Size& rPhysSize, tools::Long& rDocHdr, tools::Long& rDocFtr );
    tools::Long            GetFirstPageNo() const  { return aTableParam.nFirstPageNo; }

    tools::Long            GetTotalPages() const { return nTotalPages; }
    sal_uInt16      GetZoom() const { return nZoom; }

    void            ResetBreaks( SCTAB nTab );

    void            GetPrintState(ScPrintState& rState);
    bool            GetLastSourceRange( ScRange& rRange ) const;
    sal_uInt16      GetLeftMargin() const{return nLeftMargin;}
    sal_uInt16      GetRightMargin() const{return nRightMargin;}
    sal_uInt16      GetTopMargin() const{return nTopMargin;}
    sal_uInt16      GetBottomMargin() const{return nBottomMargin;}
    const ScPrintHFParam& GetHeader() const {return aHdr;}
    const ScPrintHFParam& GetFooter() const {return aFtr;}

    bool HasPrintRange() const { return mbHasPrintRange;}

private:
    void            Construct( const ScPrintOptions* pOptions );
    void            InitParam( const ScPrintOptions* pOptions );
    void            CalcZoom( sal_uInt16 nRangeNo );
    void            CalcPages();
    tools::Long            CountPages();
    tools::Long            CountNotePages();

    bool            AdjustPrintArea( bool bNew );

    Size            GetDocPageSize();

    tools::Long            TextHeight( const EditTextObject* pObject );
    void            MakeEditEngine();
    void            UpdateHFHeight( ScPrintHFParam& rParam );

    void            InitModes();

    bool            IsLeft( tools::Long nPageNo );
    bool            IsMirror( tools::Long nPageNo );
    void            MakeTableString();                  // sets aTableStr

    void            PrintPage( tools::Long nPageNo,
                                    SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    bool bDoPrint, ScPreviewLocationData* pLocationData );
    void            PrintArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    tools::Long nScrX, tools::Long nScrY,
                                    bool bShLeft, bool bShTop, bool bShRight, bool bShBottom );
    void            LocateArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    tools::Long nScrX, tools::Long nScrY, bool bRepCol, bool bRepRow,
                                    ScPreviewLocationData& rLocationData );
    void            PrintColHdr( SCCOL nX1, SCCOL nX2, tools::Long nScrX, tools::Long nScrY );
    void            PrintRowHdr( SCROW nY1, SCROW nY2, tools::Long nScrX, tools::Long nScrY );
    void            LocateColHdr( SCCOL nX1, SCCOL nX2, tools::Long nScrX, tools::Long nScrY,
                                bool bRepCol, ScPreviewLocationData& rLocationData );
    void            LocateRowHdr( SCROW nY1, SCROW nY2, tools::Long nScrX, tools::Long nScrY,
                                bool bRepRow, ScPreviewLocationData& rLocationData );
    void            PrintHF( tools::Long nPageNo, bool bHeader, tools::Long nStartY,
                                    bool bDoPrint, ScPreviewLocationData* pLocationData );

    tools::Long            PrintNotes( tools::Long nPageNo, tools::Long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData );
    tools::Long            DoNotes( tools::Long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData );

    void            DrawBorder( tools::Long nScrX, tools::Long nScrY, tools::Long nScrW, tools::Long nScrH,
                                    const SvxBoxItem* pBorderData,
                                    const SvxBrushItem* pBackground,
                                    const SvxShadowItem* pShadow );

    void            FillPageData();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
