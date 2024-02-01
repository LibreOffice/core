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

#include <memory>
#include <vector>

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>

#include <tools/stream.hxx>
#include <tools/UnitConversion.hxx>

#include <svx/svdetc.hxx>
#include <editeng/outlobj.hxx>

#include <cell.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/editids.hrc>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svtools/htmltokn.h>
#include <svtools/parhtml.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

namespace sdr::table
{
namespace
{
struct RowColSpan
{
    sal_Int32 mnRowSpan;
    sal_Int32 mnColSpan;
    explicit RowColSpan()
        : mnRowSpan(1)
        , mnColSpan(1)
    {
    }
};

struct HTMLCellDefault
{
    sal_Int32 mnRowSpan;
    sal_Int32 mnColSpan; // MergeCell if >1, merged cells if 0
    sal_Int32 mnCellX;

    explicit HTMLCellDefault()
        : mnRowSpan(1)
        , mnColSpan(1)
        , mnCellX(0)
    {
    }
};
}

typedef std::vector<std::shared_ptr<HTMLCellDefault>> HTMLCellDefaultVector;

namespace
{
struct HTMLCellInfo
{
    SfxItemSet maItemSet;
    sal_Int32 mnStartPara;
    sal_Int32 mnParaCount;
    sal_Int32 mnCellX;
    sal_Int32 mnRowSpan;
    std::shared_ptr<HTMLCellInfo> mxVMergeCell;

    explicit HTMLCellInfo(SfxItemPool& rPool)
        : maItemSet(rPool)
        , mnStartPara(0)
        , mnParaCount(0)
        , mnCellX(0)
        , mnRowSpan(1)
    {
    }
};
}

typedef std::shared_ptr<HTMLCellInfo> HTMLCellInfoPtr;
typedef std::vector<HTMLCellInfoPtr> HTMLColumnVector;

typedef std::shared_ptr<HTMLColumnVector> HTMLColumnVectorPtr;

class SdrTableHTMLParser
{
public:
    explicit SdrTableHTMLParser(SdrTableObj& rTableObj);

    void Read(SvStream& rStream);

    void ProcToken(HtmlImportInfo* pInfo);

    void NextRow();
    void NextColumn();
    void NewCellRow();

    void InsertCell(sal_Int32 nStartPara, sal_Int32 nEndPara);
    void InsertColumnEdge(sal_Int32 nEdge);

    void FillTable();

    DECL_LINK(HTMLImportHdl, HtmlImportInfo&, void);

private:
    SdrTableObj& mrTableObj;
    std::unique_ptr<SdrOutliner> mpOutliner;
    SfxItemPool& mrItemPool;

    HTMLCellDefaultVector maDefaultList;
    HTMLCellDefaultVector::iterator maDefaultIterator;

    HtmlTokenId mnLastToken;
    bool mbNewDef;

    sal_Int32 mnCellStartPara;

    sal_Int32 mnRowCnt;
    sal_Int32 mnLastEdge;
    sal_Int32 mnVMergeIdx;

    std::vector<sal_Int32> maColumnEdges;
    std::vector<sal_Int32>::iterator maLastEdge;
    std::vector<HTMLColumnVectorPtr> maRows;

    std::unique_ptr<HTMLCellDefault> mpInsDefault;
    HTMLCellDefault* mpActDefault;
    sal_Int32 mnCellInRow;

    Reference<XTable> mxTable;

    HTMLColumnVectorPtr mxLastRow;
    // Copy assignment is forbidden and not implemented.
    SdrTableHTMLParser(const SdrTableHTMLParser&) = delete;
    SdrTableHTMLParser& operator=(const SdrTableHTMLParser&) = delete;
};

SdrTableHTMLParser::SdrTableHTMLParser(SdrTableObj& rTableObj)
    : mrTableObj(rTableObj)
    , mpOutliner(SdrMakeOutliner(OutlinerMode::TextObject, rTableObj.getSdrModelFromSdrObject()))
    , mrItemPool(rTableObj.getSdrModelFromSdrObject().GetItemPool())
    , mnLastToken(HtmlTokenId::NONE)
    , mbNewDef(false)
    , mnCellStartPara(0)
    , mnRowCnt(0)
    , mnLastEdge(0)
    , mnVMergeIdx(0)
    , mpActDefault(nullptr)
    , mnCellInRow(-1)
    , mxTable(rTableObj.getTable())
{
    mpOutliner->SetUpdateLayout(true);
    mpOutliner->SetStyleSheet(0, mrTableObj.GetStyleSheet());
    mpInsDefault.reset(new HTMLCellDefault());
}

void SdrTableHTMLParser::Read(SvStream& rStream)
{
    EditEngine& rEdit = const_cast<EditEngine&>(mpOutliner->GetEditEngine());

    Link<HtmlImportInfo&, void> aOldLink(rEdit.GetHtmlImportHdl());
    rEdit.SetHtmlImportHdl(LINK(this, SdrTableHTMLParser, HTMLImportHdl));
    mpOutliner->Read(rStream, OUString(), EETextFormat::Html);
    rEdit.SetHtmlImportHdl(aOldLink);

    FillTable();
}

IMPL_LINK(SdrTableHTMLParser, HTMLImportHdl, HtmlImportInfo&, rInfo, void)
{
    switch (rInfo.eState)
    {
        case HtmlImportState::NextToken:
            ProcToken(&rInfo);
            break;
        case HtmlImportState::End:
            if (rInfo.aSelection.nEndPos)
            {
                mpActDefault = nullptr;
                //TODO: ??
                //                rInfo.nToken = RTF_PAR;
                rInfo.aSelection.nEndPara++;
                ProcToken(&rInfo);
            }
            break;
        case HtmlImportState::SetAttr:
        case HtmlImportState::InsertText:
        case HtmlImportState::InsertPara:
            break;
        default:
            SAL_WARN("svx.table", "unknown ImportInfo.eState");
    }
}

void SdrTableHTMLParser::NextRow()
{
    mxLastRow = maRows.back();
    mnVMergeIdx = 0;
    ++mnRowCnt;
}

void SdrTableHTMLParser::InsertCell(sal_Int32 nStartPara, sal_Int32 nEndPara)
{
    HTMLCellInfoPtr xCellInfo = std::make_shared<HTMLCellInfo>(mrItemPool);
    xCellInfo->mnStartPara = nStartPara;
    xCellInfo->mnParaCount = nEndPara - nStartPara;
    xCellInfo->mnCellX = mpActDefault->mnCellX;
    xCellInfo->mnRowSpan = mpActDefault->mnRowSpan;

    if (mxLastRow != nullptr)
    {
        sal_Int32 nSize = mxLastRow->size();
        while (mnVMergeIdx < nSize && (*mxLastRow)[mnVMergeIdx]->mnCellX < xCellInfo->mnCellX)
            ++mnVMergeIdx;

        if (xCellInfo->mnRowSpan == 0 && mnVMergeIdx < nSize)
        {
            HTMLCellInfoPtr xLastCell((*mxLastRow)[mnVMergeIdx]);
            if (xLastCell->mnRowSpan)
                xCellInfo->mxVMergeCell = xLastCell;
            else
                xCellInfo->mxVMergeCell = xLastCell->mxVMergeCell;
        }
    }

    if (!maRows.empty())
    {
        HTMLColumnVectorPtr xColumn(maRows.back());
        if (xCellInfo->mxVMergeCell)
        {
            if (xColumn->empty() || xColumn->back()->mxVMergeCell != xCellInfo->mxVMergeCell)
                xCellInfo->mxVMergeCell->mnRowSpan++;
        }

        xColumn->push_back(xCellInfo);
    }
}

void SdrTableHTMLParser::InsertColumnEdge(sal_Int32 nEdge)
{
    auto aNextEdge = std::lower_bound(maLastEdge, maColumnEdges.end(), nEdge);

    if (aNextEdge == maColumnEdges.end() || nEdge != *aNextEdge)
    {
        maLastEdge = maColumnEdges.insert(aNextEdge, nEdge);
        mnLastEdge = nEdge;
    }
}

void SdrTableHTMLParser::FillTable()
{
    try
    {
        sal_Int32 nColCount = mxTable->getColumnCount();
        Reference<XTableColumns> xCols(mxTable->getColumns(), UNO_SET_THROW);
        sal_Int32 nColMax = maColumnEdges.size();
        if (nColCount < nColMax)
        {
            xCols->insertByIndex(nColCount, nColMax - nColCount);
            nColCount = mxTable->getColumnCount();
        }

        static constexpr OUStringLiteral sWidth(u"Width");
        sal_Int32 nCol, nLastEdge = 0;
        for (nCol = 0; nCol < nColCount; nCol++)
        {
            Reference<XPropertySet> xSet(xCols->getByIndex(nCol), UNO_QUERY_THROW);
            sal_Int32 nWidth = maColumnEdges[nCol] - nLastEdge;

            xSet->setPropertyValue(sWidth, Any(nWidth));
            nLastEdge += nWidth;
        }

        const sal_Int32 nRowCount = mxTable->getRowCount();
        if (nRowCount < mnRowCnt)
        {
            Reference<XTableRows> xRows(mxTable->getRows(), UNO_SET_THROW);
            xRows->insertByIndex(nRowCount, mnRowCnt - nRowCount);
        }

        for (sal_Int32 nRow = 0; nRow < static_cast<sal_Int32>(maRows.size()); nRow++)
        {
            HTMLColumnVectorPtr xColumn(maRows[nRow]);
            nCol = 0;
            auto aEdge = maColumnEdges.begin();
            for (sal_Int32 nIdx = 0;
                 nCol < nColMax && nIdx < static_cast<sal_Int32>(xColumn->size()); nIdx++)
            {
                HTMLCellInfoPtr xCellInfo((*xColumn)[nIdx]);

                CellRef xCell(dynamic_cast<Cell*>(mxTable->getCellByPosition(nCol, nRow).get()));
                if (xCell.is() && xCellInfo)
                {
                    const SfxPoolItem* pPoolItem = nullptr;
                    if (xCellInfo->maItemSet.GetItemState(SDRATTR_TABLE_BORDER, false, &pPoolItem)
                        == SfxItemState::SET)
                        xCell->SetMergedItem(*pPoolItem);

                    std::optional<OutlinerParaObject> pTextObject(mpOutliner->CreateParaObject(
                        xCellInfo->mnStartPara, xCellInfo->mnParaCount));
                    if (pTextObject)
                    {
                        SdrOutliner& rOutliner = mrTableObj.ImpGetDrawOutliner();
                        rOutliner.SetUpdateLayout(true);
                        rOutliner.SetText(*pTextObject);
                        mrTableObj.NbcSetOutlinerParaObjectForText(rOutliner.CreateParaObject(),
                                                                   xCell.get());
                    }

                    sal_Int32 nLastRow = nRow;
                    if (xCellInfo->mnRowSpan)
                        nLastRow += xCellInfo->mnRowSpan - 1;

                    aEdge = std::lower_bound(aEdge, maColumnEdges.end(), xCellInfo->mnCellX);
                    sal_Int32 nLastCol = nCol;
                    if (aEdge != maColumnEdges.end())
                    {
                        nLastCol = std::distance(maColumnEdges.begin(), aEdge);
                        ++aEdge;
                    }

                    if (nLastCol > nCol || nLastRow > nRow)
                    {
                        Reference<XMergeableCellRange> xRange(
                            mxTable->createCursorByRange(
                                mxTable->getCellRangeByPosition(nCol, nRow, nLastCol, nLastRow)),
                            UNO_QUERY_THROW);
                        if (xRange->isMergeable())
                            xRange->merge();
                    }
                    nCol = nLastCol + 1;
                }
            }
        }

        tools::Rectangle aRect(mrTableObj.GetSnapRect());
        aRect.SetRight(aRect.Left() + nLastEdge);
        mrTableObj.NbcSetSnapRect(aRect);
    }
    catch (Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx", "");
    }
}

void SdrTableHTMLParser::NewCellRow()
{
    if (mbNewDef)
    {
        mbNewDef = false;

        maRows.push_back(std::make_shared<std::vector<std::shared_ptr<HTMLCellInfo>>>());
    }
    maDefaultIterator = maDefaultList.begin();

    NextColumn();

    DBG_ASSERT(mpActDefault, "NewCellRow: pActDefault==0");
}

void SdrTableHTMLParser::NextColumn()
{
    if (maDefaultIterator != maDefaultList.end())
        mpActDefault = (*maDefaultIterator++).get();
    else
        mpActDefault = nullptr;
}

static RowColSpan lcl_GetRowColSpan(const HTMLOptions& options)
{
    RowColSpan aRowColSpan;
    for (HTMLOptions::const_iterator optionIt = options.begin(); optionIt != options.end();
         ++optionIt)
    {
        if (optionIt->GetToken() == HtmlOptionId::COLSPAN)
        {
            aRowColSpan.mnColSpan = optionIt->GetNumber();
        }
        else if (optionIt->GetToken() == HtmlOptionId::ROWSPAN)
        {
            aRowColSpan.mnRowSpan = optionIt->GetNumber();
        }
    }
    return aRowColSpan;
}

//TODO: width is pixel - detect document pixel with to determine real width
static sal_Int32 lcl_GetWidth(const HTMLOptions& options)
{
    for (HTMLOptions::const_iterator optionIt = options.begin(); optionIt != options.end();
         ++optionIt)
    {
        if (optionIt->GetToken() == HtmlOptionId::WIDTH)
        {
            //const OUString& value = optionIt->GetString();
            //TODO: Which conversion is required?
            return 1000;
        }
    }
    return 1000;
}
void SdrTableHTMLParser::ProcToken(HtmlImportInfo* pInfo)
{
    HTMLParser* pHtmlParser = static_cast<HTMLParser*>(pInfo->pParser);
    const HTMLOptions& options = pHtmlParser->GetOptions();
    switch (pInfo->nToken)
    {
        case HtmlTokenId::TABLE_ON:
            maDefaultList.clear();
            mnLastToken = pInfo->nToken;
            maLastEdge = maColumnEdges.begin();
            mnLastEdge = 0;
            break;
        case HtmlTokenId::TABLE_OFF:
            break;
        case HtmlTokenId::TABLEHEADER_ON:
        case HtmlTokenId::TABLEDATA_ON:
        {
            ++mnCellInRow;
            DBG_ASSERT(mpActDefault, "TABLEDATA_OFF: pActDefault==0");
            RowColSpan aRowColSpan = lcl_GetRowColSpan(options);
            mpActDefault->mnColSpan = aRowColSpan.mnColSpan;
            mpActDefault->mnRowSpan = aRowColSpan.mnRowSpan;
            mnCellStartPara = pInfo->aSelection.nStartPara;
        }
        break;
        case HtmlTokenId::TABLEDATA_OFF:
        case HtmlTokenId::TABLEHEADER_OFF:
        {
            DBG_ASSERT(mpActDefault, "TABLEDATA_OFF: pActDefault==0");
            if (mbNewDef || !mpActDefault)
                NewCellRow();
            if (!mpActDefault)
                mpActDefault = mpInsDefault.get();
            if (mpActDefault->mnColSpan > 0)
            {
                mpActDefault->mnCellX = maColumnEdges[mnCellInRow + mpActDefault->mnColSpan - 1];
                InsertCell(mnCellStartPara, pInfo->aSelection.nEndPara);
            }
            NextColumn();
            mnLastToken = pInfo->nToken;
        }
        break;
        case HtmlTokenId::TABLEROW_ON:
            mbNewDef = true;
            NewCellRow();
            mnLastToken = pInfo->nToken;
            break;
        case HtmlTokenId::TABLEROW_OFF:
        {
            NextRow();
            mnCellInRow = -1;
            mnLastToken = pInfo->nToken;
        }
        break;
        case HtmlTokenId::COL_ON:
        {
            std::shared_ptr<HTMLCellDefault> pDefault(mpInsDefault.release());
            maDefaultList.push_back(pDefault);

            const sal_Int32 nSize = lcl_GetWidth(options) + mnLastEdge;
            if (nSize > mnLastEdge)
                InsertColumnEdge(nSize);

            mpInsDefault.reset(new HTMLCellDefault());
            mnLastEdge = nSize;
            mnLastToken = pInfo->nToken;
        }
        break;
        case HtmlTokenId::COL_OFF:
            break;

        default:
            break;
    }
}

void ImportAsHTML(SvStream& rStream, SdrTableObj& rObj)
{
    SdrTableHTMLParser aParser(rObj);
    aParser.Read(rStream);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
