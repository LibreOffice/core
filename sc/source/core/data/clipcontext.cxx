/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <clipcontext.hxx>
#include <document.hxx>
#include <mtvelements.hxx>
#include <column.hxx>
#include <scitems.hxx>
#include <tokenarray.hxx>
#include <editutil.hxx>
#include <clipparam.hxx>

#include <svl/intitem.hxx>
#include <formula/errorcodes.hxx>
#include <refdata.hxx>

namespace sc {

ClipContextBase::ClipContextBase(ScDocument& rDoc) :
    mpSet(new ColumnBlockPositionSet(rDoc)) {}

ClipContextBase::~ClipContextBase() {}

ColumnBlockPosition* ClipContextBase::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    return mpSet->getBlockPosition(nTab, nCol);
}

CopyFromClipContext::CopyFromClipContext(ScDocument& rDoc,
    ScDocument* pRefUndoDoc, ScDocument* pClipDoc, InsertDeleteFlags nInsertFlag,
    bool bAsLink, bool bSkipAttrForEmptyCells) :
    ClipContextBase(rDoc),
    mnDestCol1(-1), mnDestCol2(-1),
    mnDestRow1(-1), mnDestRow2(-1),
    mnTabStart(-1), mnTabEnd(-1),
    mrDestDoc(rDoc),
    mpRefUndoDoc(pRefUndoDoc), mpClipDoc(pClipDoc),
    mnInsertFlag(nInsertFlag), mnDeleteFlag(InsertDeleteFlags::NONE),
    mpCondFormatList(nullptr),
    mbAsLink(bAsLink), mbSkipAttrForEmptyCells(bSkipAttrForEmptyCells),
    mbCloneNotes (mnInsertFlag & (InsertDeleteFlags::NOTE|InsertDeleteFlags::ADDNOTES)),
    mbTableProtected(false)
{
}

CopyFromClipContext::~CopyFromClipContext()
{
}

void CopyFromClipContext::setTabRange(SCTAB nStart, SCTAB nEnd)
{
    mnTabStart = nStart;
    mnTabEnd = nEnd;
}

SCTAB CopyFromClipContext::getTabStart() const
{
    return mnTabStart;
}

SCTAB CopyFromClipContext::getTabEnd() const
{
    return mnTabEnd;
}

void CopyFromClipContext::setDestRange( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    mnDestCol1 = nCol1;
    mnDestRow1 = nRow1;
    mnDestCol2 = nCol2;
    mnDestRow2 = nRow2;
}

CopyFromClipContext::Range CopyFromClipContext::getDestRange() const
{
    Range aRet;
    aRet.mnCol1 = mnDestCol1;
    aRet.mnCol2 = mnDestCol2;
    aRet.mnRow1 = mnDestRow1;
    aRet.mnRow2 = mnDestRow2;
    return aRet;
}

ScDocument* CopyFromClipContext::getUndoDoc()
{
    return mpRefUndoDoc;
}

ScDocument* CopyFromClipContext::getClipDoc()
{
    return mpClipDoc;
}

InsertDeleteFlags CopyFromClipContext::getInsertFlag() const
{
    return mnInsertFlag;
}

void CopyFromClipContext::setDeleteFlag( InsertDeleteFlags nFlag )
{
    mnDeleteFlag = nFlag;
}

InsertDeleteFlags CopyFromClipContext::getDeleteFlag() const
{
    return mnDeleteFlag;
}

void CopyFromClipContext::setSingleCellColumnSize( size_t nSize )
{
    maSingleCells.resize(nSize);
    maSingleCellAttrs.resize(nSize);
    maSinglePatterns.resize(nSize, nullptr);
    maSingleNotes.resize(nSize, nullptr);
}

ScCellValue& CopyFromClipContext::getSingleCell( size_t nColOffset )
{
    assert(nColOffset < maSingleCells.size());
    return maSingleCells[nColOffset];
}

sc::CellTextAttr& CopyFromClipContext::getSingleCellAttr( size_t nColOffset )
{
    assert(nColOffset < maSingleCellAttrs.size());
    return maSingleCellAttrs[nColOffset];
}

void CopyFromClipContext::setSingleCell( const ScAddress& rSrcPos, const ScColumn& rSrcCol )
{
    SCCOL nColOffset = rSrcPos.Col() - mpClipDoc->GetClipParam().getWholeRange().aStart.Col();
    ScCellValue& rSrcCell = getSingleCell(nColOffset);

    const sc::CellTextAttr* pAttr = rSrcCol.GetCellTextAttr(rSrcPos.Row());

    if (pAttr)
    {
        sc::CellTextAttr& rAttr = getSingleCellAttr(nColOffset);
        rAttr = *pAttr;
    }

    if (mbAsLink)
    {
        ScSingleRefData aRef;
        aRef.InitAddress(rSrcPos);
        aRef.SetFlag3D(true);

        ScTokenArray aArr;
        aArr.AddSingleReference(aRef);
        rSrcCell.set(new ScFormulaCell(mpClipDoc, rSrcPos, aArr));
        return;
    }

    rSrcCell.assign(*mpClipDoc, rSrcPos);

    // Check the paste flag to see whether we want to paste this cell.  If the
    // flag says we don't want to paste this cell, we'll return with true.
    InsertDeleteFlags nFlags = getInsertFlag();
    bool bNumeric  = (nFlags & InsertDeleteFlags::VALUE) != InsertDeleteFlags::NONE;
    bool bDateTime = (nFlags & InsertDeleteFlags::DATETIME) != InsertDeleteFlags::NONE;
    bool bString   = (nFlags & InsertDeleteFlags::STRING) != InsertDeleteFlags::NONE;
    bool bBoolean  = (nFlags & InsertDeleteFlags::SPECIAL_BOOLEAN) != InsertDeleteFlags::NONE;
    bool bFormula  = (nFlags & InsertDeleteFlags::FORMULA) != InsertDeleteFlags::NONE;

    switch (rSrcCell.meType)
    {
        case CELLTYPE_VALUE:
        {
            bool bPaste = isDateCell(rSrcCol, rSrcPos.Row()) ? bDateTime : bNumeric;
            if (!bPaste)
                // Don't paste this.
                rSrcCell.clear();
        }
        break;
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
        {
            if (!bString)
                // Skip pasting.
                rSrcCell.clear();
        }
        break;
        case CELLTYPE_FORMULA:
        {
            if (bBoolean)
            {
                // Check if this formula cell is a boolean cell, and if so, go ahead and paste it.
                const ScTokenArray* pCode = rSrcCell.mpFormula->GetCode();
                if (pCode && pCode->GetLen() == 1)
                {
                    const formula::FormulaToken* p = pCode->FirstToken();
                    if (p->GetOpCode() == ocTrue || p->GetOpCode() == ocFalse)
                        // This is a boolean formula. Good.
                        break;
                }
            }

            if (bFormula)
                // Good.
                break;

            FormulaError nErr = rSrcCell.mpFormula->GetErrCode();
            if (nErr != FormulaError::NONE)
            {
                // error codes are cloned with values
                if (!bNumeric)
                    // Error code is treated as numeric value. Don't paste it.
                    rSrcCell.clear();
                else
                {
                    // Turn this into a formula cell with just the error code.
                    ScFormulaCell* pErrCell = new ScFormulaCell(mpClipDoc, rSrcPos);
                    pErrCell->SetErrCode(nErr);
                    rSrcCell.set(pErrCell);
                }
            }
            else if (rSrcCell.mpFormula->IsEmptyDisplayedAsString())
            {
                // Empty stays empty and doesn't become 0.
                rSrcCell.clear();
            }
            else if (rSrcCell.mpFormula->IsValue())
            {
                bool bPaste = isDateCell(rSrcCol, rSrcPos.Row()) ? bDateTime : bNumeric;
                if (!bPaste)
                {
                    // Don't paste this.
                    rSrcCell.clear();
                    break;
                }

                // Turn this into a numeric cell.
                rSrcCell.set(rSrcCell.mpFormula->GetValue());
            }
            else if (bString)
            {
                svl::SharedString aStr = rSrcCell.mpFormula->GetString();
                if (aStr.isEmpty())
                {
                    // do not clone empty string
                    rSrcCell.clear();
                    break;
                }

                // Turn this into a string or edit cell.
                if (rSrcCell.mpFormula->IsMultilineResult())
                {
                    // TODO : Add shared string support to the edit engine to
                    // make this process simpler.
                    ScFieldEditEngine& rEngine = mrDestDoc.GetEditEngine();
                    rEngine.SetText(rSrcCell.mpFormula->GetString().getString());
                    std::unique_ptr<EditTextObject> pObj(rEngine.CreateTextObject());
                    pObj->NormalizeString(mrDestDoc.GetSharedStringPool());
                    rSrcCell.set(*pObj);
                }
                else
                    rSrcCell.set(rSrcCell.mpFormula->GetString());
            }
            else
                // We don't want to paste this.
                rSrcCell.clear();
        }
        break;
        case CELLTYPE_NONE:
        default:
            // There is nothing to paste.
            rSrcCell.clear();
    }
}

const ScPatternAttr* CopyFromClipContext::getSingleCellPattern( size_t nColOffset ) const
{
    assert(nColOffset < maSinglePatterns.size());
    return maSinglePatterns[nColOffset];
}

void CopyFromClipContext::setSingleCellPattern( size_t nColOffset, const ScPatternAttr* pAttr )
{
    assert(nColOffset < maSinglePatterns.size());
    maSinglePatterns[nColOffset] = pAttr;
}

const ScPostIt* CopyFromClipContext::getSingleCellNote( size_t nColOffset ) const
{
    assert(nColOffset < maSingleNotes.size());
    return maSingleNotes[nColOffset];
}

void CopyFromClipContext::setSingleCellNote( size_t nColOffset, const ScPostIt* pNote )
{
    assert(nColOffset < maSingleNotes.size());
    maSingleNotes[nColOffset] = pNote;
}

void CopyFromClipContext::setCondFormatList( ScConditionalFormatList* pCondFormatList )
{
    mpCondFormatList = pCondFormatList;
}

ScConditionalFormatList* CopyFromClipContext::getCondFormatList()
{
    return mpCondFormatList;
}

void CopyFromClipContext::setTableProtected( bool b )
{
    mbTableProtected = b;
}

bool CopyFromClipContext::isTableProtected() const
{
    return mbTableProtected;
}

bool CopyFromClipContext::isAsLink() const
{
    return mbAsLink;
}

bool CopyFromClipContext::isSkipAttrForEmptyCells() const
{
    return mbSkipAttrForEmptyCells;
}

bool CopyFromClipContext::isCloneNotes() const
{
    return mbCloneNotes;
}

bool CopyFromClipContext::isDateCell( const ScColumn& rCol, SCROW nRow ) const
{
    sal_uLong nNumIndex = rCol.GetAttr(nRow, ATTR_VALUE_FORMAT).GetValue();
    SvNumFormatType nType = mpClipDoc->GetFormatTable()->GetType(nNumIndex);
    return (nType == SvNumFormatType::DATE) || (nType == SvNumFormatType::TIME) || (nType == SvNumFormatType::DATETIME);
}

CopyToClipContext::CopyToClipContext(
    ScDocument& rDoc, bool bKeepScenarioFlags) :
    ClipContextBase(rDoc), mbKeepScenarioFlags(bKeepScenarioFlags) {}

CopyToClipContext::~CopyToClipContext() {}

bool CopyToClipContext::isKeepScenarioFlags() const
{
    return mbKeepScenarioFlags;
}

CopyToDocContext::CopyToDocContext(ScDocument& rDoc) :
    ClipContextBase(rDoc), mbStartListening(true) {}

CopyToDocContext::~CopyToDocContext() {}

void CopyToDocContext::setStartListening( bool b )
{
    mbStartListening = b;
}

bool CopyToDocContext::isStartListening() const
{
    return mbStartListening;
}

MixDocContext::MixDocContext(ScDocument& rDoc) : ClipContextBase(rDoc) {}
MixDocContext::~MixDocContext() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
