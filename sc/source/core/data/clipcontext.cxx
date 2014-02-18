/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "clipcontext.hxx"
#include "document.hxx"
#include "mtvelements.hxx"
#include <column.hxx>
#include <scitems.hxx>

#include <svl/intitem.hxx>

namespace sc {

ClipContextBase::ClipContextBase(ScDocument& rDoc) :
    mpSet(new ColumnBlockPositionSet(rDoc)) {}

ClipContextBase::~ClipContextBase() {}

ColumnBlockPosition* ClipContextBase::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    return mpSet->getBlockPosition(nTab, nCol);
}

CopyFromClipContext::CopyFromClipContext(ScDocument& rDoc,
    ScDocument* pRefUndoDoc, ScDocument* pClipDoc, sal_uInt16 nInsertFlag,
    bool bAsLink, bool bSkipAttrForEmptyCells) :
    ClipContextBase(rDoc),
    mnDestCol1(-1), mnDestCol2(-1),
    mnDestRow1(-1), mnDestRow2(-1),
    mnTabStart(-1), mnTabEnd(-1),
    mpRefUndoDoc(pRefUndoDoc), mpClipDoc(pClipDoc),
    mnInsertFlag(nInsertFlag), mnDeleteFlag(IDF_NONE),
    mpCondFormatList(NULL), mpSinglePattern(NULL), mpSingleNote(NULL),
    mbAsLink(bAsLink), mbSkipAttrForEmptyCells(bSkipAttrForEmptyCells),
    mbCloneNotes (mnInsertFlag & (IDF_NOTE|IDF_ADDNOTES)),
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

sal_uInt16 CopyFromClipContext::getInsertFlag() const
{
    return mnInsertFlag;
}

void CopyFromClipContext::setDeleteFlag( sal_uInt16 nFlag )
{
    mnDeleteFlag = nFlag;
}

sal_uInt16 CopyFromClipContext::getDeleteFlag() const
{
    return mnDeleteFlag;
}

ScCellValue& CopyFromClipContext::getSingleCell()
{
    return maSingleCell;
}

void CopyFromClipContext::setCondFormatList( ScConditionalFormatList* pCondFormatList )
{
    mpCondFormatList = pCondFormatList;
}

ScConditionalFormatList* CopyFromClipContext::getCondFormatList()
{
    return mpCondFormatList;
}

const ScPatternAttr* CopyFromClipContext::getSingleCellPattern() const
{
    return mpSinglePattern;
}

void CopyFromClipContext::setSingleCellPattern( const ScPatternAttr* pAttr )
{
    mpSinglePattern = pAttr;
}

const ScPostIt* CopyFromClipContext::getSingleCellNote() const
{
    return mpSingleNote;
}

void CopyFromClipContext::setSingleCellNote( const ScPostIt* pNote )
{
    mpSingleNote = pNote;
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
    sal_uLong nNumIndex = static_cast<const SfxUInt32Item*>(rCol.GetAttr(nRow, ATTR_VALUE_FORMAT))->GetValue();
    short nType = mpClipDoc->GetFormatTable()->GetType(nNumIndex);
    return (nType == NUMBERFORMAT_DATE) || (nType == NUMBERFORMAT_TIME) || (nType == NUMBERFORMAT_DATETIME);
}

CopyToClipContext::CopyToClipContext(
    ScDocument& rDoc, bool bKeepScenarioFlags, bool bCloneNotes) :
    ClipContextBase(rDoc), mbKeepScenarioFlags(bKeepScenarioFlags), mbCloneNotes(bCloneNotes) {}

CopyToClipContext::~CopyToClipContext() {}

bool CopyToClipContext::isKeepScenarioFlags() const
{
    return mbKeepScenarioFlags;
}

bool CopyToClipContext::isCloneNotes() const
{
    return mbCloneNotes;
}

CopyToDocContext::CopyToDocContext(ScDocument& rDoc) : ClipContextBase(rDoc) {}
CopyToDocContext::~CopyToDocContext() {}

MixDocContext::MixDocContext(ScDocument& rDoc) : ClipContextBase(rDoc) {}
MixDocContext::~MixDocContext() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
