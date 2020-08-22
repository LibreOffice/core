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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR2_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR2_HXX

#include <fmtfsize.hxx>
#include <svl/itemset.hxx>
#include <svx/swframetypes.hxx>
#include <swtable.hxx>

#include "ww8scan.hxx"
#include "ww8par.hxx"

class WW8RStyle;

class WW8DupProperties
{
public:
    WW8DupProperties(SwDoc &rDoc, SwWW8FltControlStack *pStack);
    void Insert(const SwPosition &rPos);
private:
    WW8DupProperties(const WW8DupProperties&) = delete;
    WW8DupProperties& operator=(const WW8DupProperties&) = delete;
    SwWW8FltControlStack* pCtrlStck;
    SfxItemSet aChrSet,aParSet;
};

struct WW8SwFlyPara
{
    SwFlyFrameFormat* pFlyFormat;

                // part 1: directly derived Sw attributes
    sal_Int16 nXPos, nYPos;         // Position
    sal_Int16 nLeMgn, nRiMgn;       // borders
    sal_Int16 nUpMgn, nLoMgn;       // borders
    sal_Int16 nWidth, nHeight;      // size
    sal_Int16 nNetWidth;

    SwFrameSize eHeightFix;         // height fixed or min
    static constexpr RndStdIds eAnchor = RndStdIds::FLY_AT_PARA; // binding
    short       eHRel;              // page or page border
    short       eVRel;              // page or page border
    sal_Int16   eVAlign;            // up, down, centered
    sal_Int16   eHAlign;            // left, right, centered
    css::text::WrapTextMode
                eSurround;          // wrap mode

    sal_uInt8 nXBind, nYBind;        // bound relative to what

                // part 2: changes found during reading
    long nNewNetWidth;
    std::unique_ptr<SwPosition> xMainTextPos;   // to return to main text after apo
    sal_uInt16 nLineSpace;          // line space in tw for graf apos
    bool bAutoWidth;
    bool bTogglePos;

    // add parameter <nWWPgTop> - WW8's page top margin
    WW8SwFlyPara( SwPaM& rPaM,
                  SwWW8ImplReader& rIo,
                  WW8FlyPara& rWW,
                  const sal_uInt32 nWWPgTop,
                  const sal_uInt32 nPgWidth,
                  const sal_Int32 nIniFlyDx,
                  const sal_Int32 nIniFlyDy );

    void BoxUpWidth( long nWidth );
    std::unique_ptr<SwWW8FltAnchorStack> xOldAnchorStck;
};

class WW8RStyle: public WW8Style
{
friend class SwWW8ImplReader;
    wwSprmParser maSprmParser;
    SwWW8ImplReader* mpIo;   // parser class
    SvStream* mpStStrm;      // input file

    SwNumRule* mpStyRule;    // bullets and enumerations in styles

    sal_uInt8* mpParaSprms;           // all ParaSprms of the UPX if UPX.Papx
    sal_uInt16 mnSprmsLen;           // its length

    sal_uInt8 mnWwNumLevel;           // for bullets and enumerations in styles

    bool mbTextColChanged;
    bool mbFontChanged;      // For Simulating Default-Font
    bool mbCJKFontChanged;   // For Simulating Default-CJK Font
    bool mbCTLFontChanged;   // For Simulating Default-CTL Font
    bool mbFSizeChanged;     // For Simulating Default-FontSize
    bool mbFCTLSizeChanged;  // For Simulating Default-CTL FontSize
    bool mbWidowsChanged;    // For Simulating Default-Widows / Orphans
    bool mbBidiChanged;      // For Simulating Default-LTR

    void ImportSprms(std::size_t nPosFc, short nLen, bool bPap);
    void ImportSprms(sal_uInt8 *pSprms, short nLen, bool bPap);
    void ImportGrupx(short nLen, bool bPara, bool bOdd);
    short ImportUPX(short nLen, bool bPAP, bool bOdd);

    void Set1StyleDefaults();
    void Import1Style(sal_uInt16 nNr);
    void RecursiveReg(sal_uInt16 nNr);

    void ImportNewFormatStyles();
    void ScanStyles();
    void ImportOldFormatStyles();

    bool PrepareStyle(SwWW8StyInf &rSI, ww::sti eSti, sal_uInt16 nThisStyle, sal_uInt16 nNextStyle);
    void PostStyle(SwWW8StyInf const &rSI, bool bOldNoImp);

    WW8RStyle(const WW8RStyle&) = delete;
    WW8RStyle& operator=(const WW8RStyle&) = delete;
public:
    WW8RStyle( WW8Fib& rFib, SwWW8ImplReader* pI );
    void Import();
    void PostProcessStyles();
    SprmResult HasParaSprm(sal_uInt16 nId) const;
};

class WW8FlySet: public SfxItemSet
{
private:
    const WW8FlySet& operator=(const WW8FlySet&) = delete;
    void Init(const SwWW8ImplReader& rReader, const SwPaM* pPaM);
public:
    WW8FlySet(SwWW8ImplReader& rReader, const WW8FlyPara* pFW,
        const WW8SwFlyPara* pFS, bool bGraf);
    WW8FlySet(SwWW8ImplReader& rReader, const SwPaM* pPaM, const WW8_PIC& rPic,
        long nWidth, long nHeight);
};

// Gets filled in WW8TabDesc::MergeCells().
// Algorithm must ensure proper row and column order in WW8SelBoxInfo!
class WW8SelBoxInfo
{
private:
    std::vector<std::vector<SwTableBox*> > m_vRows;

    WW8SelBoxInfo(WW8SelBoxInfo const&) = delete;
    WW8SelBoxInfo& operator=(WW8SelBoxInfo const&) = delete;

public:
    short nGroupXStart;
    short nGroupWidth;
    bool bGroupLocked;

    WW8SelBoxInfo(short nXCenter, short nWidth)
        : nGroupXStart( nXCenter ), nGroupWidth( nWidth ), bGroupLocked(false)
    {}

    size_t size() const
    {
        size_t nResult = 0;
        for (auto& it : m_vRows)
            nResult += it.size();
        return nResult;
    }

    size_t rowsCount() const { return m_vRows.size(); }

    const std::vector<SwTableBox*>& row( size_t nIndex ) { return m_vRows[nIndex]; }

    void push_back( SwTableBox* pBox )
    {
        bool bDone = false;
        for (auto& iRow : m_vRows)
            if (iRow[0]->GetUpper() == pBox->GetUpper())
            {
                iRow.push_back(pBox);
                bDone = true;
                break;
            }
        if (!bDone)
        {
            const size_t sz = m_vRows.size();
            m_vRows.resize(sz+1);
            m_vRows[sz].push_back(pBox);
        }
    }
};

class WW8TabDesc
{
    std::vector<OUString> m_aNumRuleNames;
    std::unique_ptr<sw::util::RedlineStack> mxOldRedlineStack;

    SwWW8ImplReader* m_pIo;

    WW8TabBandDesc* m_pFirstBand;
    WW8TabBandDesc* m_pActBand;

    std::unique_ptr<SwPosition> m_xTmpPos;

    SwTableNode* m_pTableNd;          // table node
    const SwTableLines* m_pTabLines;  // row array of node
    SwTableLine* m_pTabLine;          // current row
    SwTableBoxes* m_pTabBoxes;        // boxes array in current row
    SwTableBox* m_pTabBox;            // current cell

    std::vector<std::unique_ptr<WW8SelBoxInfo>> m_MergeGroups;   // list of all cells to be merged

    WW8_TCell* m_pCurrentWWCell;

    short m_nRows;
    short m_nDefaultSwCols;
    short m_nBands;
    short m_nMinLeft;
    short m_nMaxRight;
    short m_nSwWidth;
    short m_nPreferredWidth;
    short m_nPercentWidth;

    bool m_bOk;
    bool m_bClaimLineFormat;
    sal_Int16 m_eOri;
    bool m_bIsBiDi;
                                // 2. common admin info
    short m_nCurrentRow;
    short m_nCurrentBandRow;    // SW: row of current band
                                // 3. admin info for writer
    short m_nCurrentCol;

    sal_uInt16 m_nRowsToRepeat;

    // 4. methods

    sal_uInt16 GetLogicalWWCol() const;
    void SetTabBorders( SwTableBox* pBox, short nIdx );
    void SetTabShades( SwTableBox* pBox, short nWwIdx );
    void SetTabVertAlign( SwTableBox* pBox, short nWwIdx );
    void SetTabDirection( SwTableBox* pBox, short nWwIdx );
    void CalcDefaults();
    void SetPamInCell(short nWwCol, bool bPam);
    void InsertCells( short nIns );
    void AdjustNewBand();

    WW8SelBoxInfo* FindMergeGroup(short nX1, short nWidth, bool bExact);

    // single box - maybe used in a merge group
    // (the merge groups are processed later at once)
    void UpdateTableMergeGroup(WW8_TCell const & rCell,
        WW8SelBoxInfo* pActGroup, SwTableBox* pActBox, sal_uInt16 nCol  );

    WW8TabDesc(WW8TabDesc const&) = delete;
    WW8TabDesc& operator=(WW8TabDesc const&) = delete;

public:
    const SwTable* m_pTable;          // table
    SwPosition* m_pParentPos;
    SwFlyFrameFormat* m_pFlyFormat;
    SfxItemSet m_aItemSet;
    bool IsValidCell(short nCol) const;
    bool InFirstParaInCell() const;

    WW8TabDesc( SwWW8ImplReader* pIoClass, WW8_CP nStartCp );
    bool Ok() const { return m_bOk; }
    void CreateSwTable();
    void UseSwTable();
    void SetSizePosition(SwFrameFormat* pFrameFormat);
    void TableCellEnd();
    void MoveOutsideTable();
    void ParkPaM();
    void FinishSwTable();
    void MergeCells();
    short GetMinLeft() const { return m_nMinLeft; }
    ~WW8TabDesc();

    const WW8_TCell* GetCurrentWWCell() const { return m_pCurrentWWCell; }
    short GetCurrentCol() const { return m_nCurrentCol; }
    // find name of numrule valid for current WW-COL
    OUString GetNumRuleName() const;
    void SetNumRuleName( const OUString& rName );

    sw::util::RedlineStack* getOldRedlineStack() { return mxOldRedlineStack.get(); }
};

enum WW8LvlType {WW8_None, WW8_Outline, WW8_Numbering, WW8_Sequence, WW8_Pause};

WW8LvlType GetNumType(sal_uInt8 nWwLevelNo);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
