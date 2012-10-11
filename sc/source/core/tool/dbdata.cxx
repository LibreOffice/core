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


#include <unotools/transliterationwrapper.hxx>

#include "dbdata.hxx"
#include "globalnames.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
#include "document.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "globstr.hrc"
#include "subtotalparam.hxx"
#include "sortparam.hxx"

#include <memory>

using ::std::auto_ptr;
using ::std::unary_function;
using ::std::for_each;
using ::std::find_if;
using ::std::remove_if;
using ::std::pair;

//---------------------------------------------------------------------------------------

bool ScDBData::less::operator() (const ScDBData& left, const ScDBData& right) const
{
    return ScGlobal::GetpTransliteration()->compareString(left.GetName(), right.GetName()) < 0;
}

ScDBData::ScDBData( const ::rtl::OUString& rName,
                    SCTAB nTab,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    bool bByR, bool bHasH) :
    mpSortParam(new ScSortParam),
    mpQueryParam(new ScQueryParam),
    mpSubTotal(new ScSubTotalParam),
    mpImportParam(new ScImportParam),
    aName       (rName),
    aUpper      (rName),
    nTable      (nTab),
    nStartCol   (nCol1),
    nStartRow   (nRow1),
    nEndCol     (nCol2),
    nEndRow     (nRow2),
    bByRow      (bByR),
    bHasHeader  (bHasH),
    bDoSize     (false),
    bKeepFmt    (false),
    bStripData  (false),
    bIsAdvanced (false),
    bDBSelection(false),
    nIndex      (0),
    bAutoFilter (false),
    bModified   (false)
{
    aUpper = ScGlobal::pCharClass->uppercase(aUpper);
}

ScDBData::ScDBData( const ScDBData& rData ) :
    ScRefreshTimer      ( rData ),
    mpSortParam(new ScSortParam(*rData.mpSortParam)),
    mpQueryParam(new ScQueryParam(*rData.mpQueryParam)),
    mpSubTotal(new ScSubTotalParam(*rData.mpSubTotal)),
    mpImportParam(new ScImportParam(*rData.mpImportParam)),
    aName               (rData.aName),
    aUpper              (rData.aUpper),
    nTable              (rData.nTable),
    nStartCol           (rData.nStartCol),
    nStartRow           (rData.nStartRow),
    nEndCol             (rData.nEndCol),
    nEndRow             (rData.nEndRow),
    bByRow              (rData.bByRow),
    bHasHeader          (rData.bHasHeader),
    bDoSize             (rData.bDoSize),
    bKeepFmt            (rData.bKeepFmt),
    bStripData          (rData.bStripData),
    bIsAdvanced         (rData.bIsAdvanced),
    aAdvSource          (rData.aAdvSource),
    bDBSelection        (rData.bDBSelection),
    nIndex              (rData.nIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified)
{
}

ScDBData::ScDBData( const ::rtl::OUString& rName, const ScDBData& rData ) :
    ScRefreshTimer      ( rData ),
    mpSortParam(new ScSortParam(*rData.mpSortParam)),
    mpQueryParam(new ScQueryParam(*rData.mpQueryParam)),
    mpSubTotal(new ScSubTotalParam(*rData.mpSubTotal)),
    mpImportParam(new ScImportParam(*rData.mpImportParam)),
    aName               (rName),
    aUpper              (rName),
    nTable              (rData.nTable),
    nStartCol           (rData.nStartCol),
    nStartRow           (rData.nStartRow),
    nEndCol             (rData.nEndCol),
    nEndRow             (rData.nEndRow),
    bByRow              (rData.bByRow),
    bHasHeader          (rData.bHasHeader),
    bDoSize             (rData.bDoSize),
    bKeepFmt            (rData.bKeepFmt),
    bStripData          (rData.bStripData),
    bIsAdvanced         (rData.bIsAdvanced),
    aAdvSource          (rData.aAdvSource),
    bDBSelection        (rData.bDBSelection),
    nIndex              (rData.nIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified)
{
    aUpper = ScGlobal::pCharClass->uppercase(aUpper);
}

ScDBData& ScDBData::operator= (const ScDBData& rData)
{
    // Don't modify the name.  The name is not mutable as it is used as a key
    // in the container to keep the db ranges sorted by the name.
    ScRefreshTimer::operator=( rData );
    mpSortParam.reset(new ScSortParam(*rData.mpSortParam));
    mpQueryParam.reset(new ScQueryParam(*rData.mpQueryParam));
    mpSubTotal.reset(new ScSubTotalParam(*rData.mpSubTotal));
    mpImportParam.reset(new ScImportParam(*rData.mpImportParam));
    nTable              = rData.nTable;
    nStartCol           = rData.nStartCol;
    nStartRow           = rData.nStartRow;
    nEndCol             = rData.nEndCol;
    nEndRow             = rData.nEndRow;
    bByRow              = rData.bByRow;
    bHasHeader          = rData.bHasHeader;
    bDoSize             = rData.bDoSize;
    bKeepFmt            = rData.bKeepFmt;
    bStripData          = rData.bStripData;
    bIsAdvanced         = rData.bIsAdvanced;
    aAdvSource          = rData.aAdvSource;
    bDBSelection        = rData.bDBSelection;
    nIndex              = rData.nIndex;
    bAutoFilter         = rData.bAutoFilter;

    return *this;
}

bool ScDBData::operator== (const ScDBData& rData) const
{
    //  Daten, die nicht in den Params sind

    if ( nTable     != rData.nTable     ||
         bDoSize    != rData.bDoSize    ||
         bKeepFmt   != rData.bKeepFmt   ||
         bIsAdvanced!= rData.bIsAdvanced||
         bStripData != rData.bStripData ||
//       SAB: I think this should be here, but I don't want to break something
//         bAutoFilter!= rData.bAutoFilter||
         ScRefreshTimer::operator!=( rData )
        )
        return false;

    if ( bIsAdvanced && aAdvSource != rData.aAdvSource )
        return false;

    ScSortParam aSort1, aSort2;
    GetSortParam(aSort1);
    rData.GetSortParam(aSort2);
    if (!(aSort1 == aSort2))
        return false;

    ScQueryParam aQuery1, aQuery2;
    GetQueryParam(aQuery1);
    rData.GetQueryParam(aQuery2);
    if (!(aQuery1 == aQuery2))
        return false;

    ScSubTotalParam aSubTotal1, aSubTotal2;
    GetSubTotalParam(aSubTotal1);
    rData.GetSubTotalParam(aSubTotal2);
    if (!(aSubTotal1 == aSubTotal2))
        return false;

    ScImportParam aImport1, aImport2;
    GetImportParam(aImport1);
    rData.GetImportParam(aImport2);
    if (!(aImport1 == aImport2))
        return false;

    return true;
}

ScDBData::~ScDBData()
{
    StopRefreshTimer();
}


::rtl::OUString ScDBData::GetSourceString() const
{
    ::rtl::OUStringBuffer aBuf;
    if (mpImportParam->bImport)
    {
        aBuf.append(mpImportParam->aDBName);
        aBuf.append(sal_Unicode('/'));
        aBuf.append(mpImportParam->aStatement);
    }
    return aBuf.makeStringAndClear();
}

::rtl::OUString ScDBData::GetOperations() const
{
    ::rtl::OUStringBuffer aBuf;
    if (mpQueryParam->GetEntryCount())
    {
        const ScQueryEntry& rEntry = mpQueryParam->GetEntry(0);
        if (rEntry.bDoQuery)
            aBuf.append(ScGlobal::GetRscString(STR_OPERATION_FILTER));
    }

    if (mpSortParam->maKeyState[0].bDoSort)
    {
        if (aBuf.getLength())
            aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        aBuf.append(ScGlobal::GetRscString(STR_OPERATION_SORT));
    }

    if (mpSubTotal->bGroupActive[0] && !mpSubTotal->bRemoveOnly)
    {
        if (aBuf.getLength())
            aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        aBuf.append(ScGlobal::GetRscString(STR_OPERATION_SUBTOTAL));
    }

    if (!aBuf.getLength())
        aBuf.append(ScGlobal::GetRscString(STR_OPERATION_NONE));

    return aBuf.makeStringAndClear();
}

void ScDBData::GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2) const
{
    rTab  = nTable;
    rCol1 = nStartCol;
    rRow1 = nStartRow;
    rCol2 = nEndCol;
    rRow2 = nEndRow;
}

void ScDBData::GetArea(ScRange& rRange) const
{
    SCROW nNewEndRow = nEndRow;
    rRange = ScRange( nStartCol, nStartRow, nTable, nEndCol, nNewEndRow, nTable );
}

void ScDBData::SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    nTable  = nTab;
    nStartCol = nCol1;
    nStartRow = nRow1;
    nEndCol   = nCol2;
    nEndRow   = nRow2;
}

void ScDBData::MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    sal_uInt16 i;
    long nDifX = ((long) nCol1) - ((long) nStartCol);
    long nDifY = ((long) nRow1) - ((long) nStartRow);

    long nSortDif = bByRow ? nDifX : nDifY;
    long nSortEnd = bByRow ? static_cast<long>(nCol2) : static_cast<long>(nRow2);

    for (i=0; i<mpSortParam->GetSortKeyCount(); i++)
    {
        mpSortParam->maKeyState[i].nField += nSortDif;
        if (mpSortParam->maKeyState[i].nField > nSortEnd)
        {
            mpSortParam->maKeyState[i].nField = 0;
            mpSortParam->maKeyState[i].bDoSort = false;
        }
    }

    SCSIZE nCount = mpQueryParam->GetEntryCount();
    for (i = 0; i < nCount; ++i)
    {
        ScQueryEntry& rEntry = mpQueryParam->GetEntry(i);
        rEntry.nField += nDifX;
        if (rEntry.nField > nCol2)
        {
            rEntry.nField = 0;
            rEntry.bDoQuery = false;
        }
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        mpSubTotal->nField[i] = sal::static_int_cast<SCCOL>( mpSubTotal->nField[i] + nDifX );
        if (mpSubTotal->nField[i] > nCol2)
        {
            mpSubTotal->nField[i] = 0;
            mpSubTotal->bGroupActive[i] = false;
        }
    }

    SetArea( nTab, nCol1, nRow1, nCol2, nRow2 );
}

void ScDBData::GetSortParam( ScSortParam& rSortParam ) const
{
    rSortParam = *mpSortParam;
    rSortParam.nCol1 = nStartCol;
    rSortParam.nRow1 = nStartRow;
    rSortParam.nCol2 = nEndCol;
    rSortParam.nRow2 = nEndRow;
    rSortParam.bByRow = bByRow;
    rSortParam.bHasHeader = bHasHeader;
}

void ScDBData::SetSortParam( const ScSortParam& rSortParam )
{
    mpSortParam.reset(new ScSortParam(rSortParam));
    bByRow = rSortParam.bByRow;
}

void ScDBData::GetQueryParam( ScQueryParam& rQueryParam ) const
{
    rQueryParam = *mpQueryParam;
    rQueryParam.nCol1 = nStartCol;
    rQueryParam.nRow1 = nStartRow;
    rQueryParam.nCol2 = nEndCol;
    rQueryParam.nRow2 = nEndRow;
    rQueryParam.nTab  = nTable;
    rQueryParam.bByRow = bByRow;
    rQueryParam.bHasHeader = bHasHeader;
}

void ScDBData::SetQueryParam(const ScQueryParam& rQueryParam)
{
    mpQueryParam.reset(new ScQueryParam(rQueryParam));

    //  set bIsAdvanced to false for everything that is not from the
    //  advanced filter dialog
    bIsAdvanced = false;
}

void ScDBData::SetAdvancedQuerySource(const ScRange* pSource)
{
    if (pSource)
    {
        aAdvSource = *pSource;
        bIsAdvanced = true;
    }
    else
        bIsAdvanced = false;
}

bool ScDBData::GetAdvancedQuerySource(ScRange& rSource) const
{
    rSource = aAdvSource;
    return bIsAdvanced;
}

void ScDBData::GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const
{
    rSubTotalParam = *mpSubTotal;

    // Share the data range with the parent db data.  The range in the subtotal
    // param struct is not used.
    rSubTotalParam.nCol1 = nStartCol;
    rSubTotalParam.nRow1 = nStartRow;
    rSubTotalParam.nCol2 = nEndCol;
    rSubTotalParam.nRow2 = nEndRow;
}

void ScDBData::SetSubTotalParam(const ScSubTotalParam& rSubTotalParam)
{
    mpSubTotal.reset(new ScSubTotalParam(rSubTotalParam));
}

void ScDBData::GetImportParam(ScImportParam& rImportParam) const
{
    rImportParam = *mpImportParam;
    // set the range.
    rImportParam.nCol1 = nStartCol;
    rImportParam.nRow1 = nStartRow;
    rImportParam.nCol2 = nEndCol;
    rImportParam.nRow2 = nEndRow;
}

void ScDBData::SetImportParam(const ScImportParam& rImportParam)
{
    // the range is ignored.
    mpImportParam.reset(new ScImportParam(rImportParam));
}

bool ScDBData::IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly) const
{
    if (nTab == nTable)
    {
        if ( bStartOnly )
            return ( nCol == nStartCol && nRow == nStartRow );
        else
            return ( nCol >= nStartCol && nCol <= nEndCol &&
                     nRow >= nStartRow && nRow <= nEndRow );
    }

    return false;
}

bool ScDBData::IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    return (bool)((nTab == nTable)
                    && (nCol1 == nStartCol) && (nRow1 == nStartRow)
                    && (nCol2 == nEndCol) && (nRow2 == nEndRow));
}

bool ScDBData::HasImportParam() const
{
    return mpImportParam && mpImportParam->bImport;
}

bool ScDBData::HasQueryParam() const
{
    if (!mpQueryParam)
        return false;

    if (!mpQueryParam->GetEntryCount())
        return false;

    return mpQueryParam->GetEntry(0).bDoQuery;
}

bool ScDBData::HasSortParam() const
{
    return mpSortParam &&
        !mpSortParam->maKeyState.empty() &&
        mpSortParam->maKeyState[0].bDoSort;
}

bool ScDBData::HasSubTotalParam() const
{
    return mpSubTotal && mpSubTotal->bGroupActive[0];
}

void ScDBData::UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos)
{
        ScRange aRange;
        GetArea( aRange );
        SCTAB nTab = aRange.aStart.Tab();               // hat nur eine Tabelle

        //  anpassen wie die aktuelle Tabelle bei ScTablesHint (tabvwsh5.cxx)

        if ( nTab == nOldPos )                          // verschobene Tabelle
            nTab = nNewPos;
        else if ( nOldPos < nNewPos )                   // nach hinten verschoben
        {
            if ( nTab > nOldPos && nTab <= nNewPos )    // nachrueckender Bereich
                --nTab;
        }
        else                                            // nach vorne verschoben
        {
            if ( nTab >= nNewPos && nTab < nOldPos )    // nachrueckender Bereich
                ++nTab;
        }

        bool bChanged = ( nTab != aRange.aStart.Tab() );
        if (bChanged)
            SetArea( nTab, aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(),aRange.aEnd .Row() );

        //  MoveTo ist nicht noetig, wenn nur die Tabelle geaendert ist

        SetModified(bChanged);

}

void ScDBData::UpdateReference(ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz)
{
    SCCOL theCol1;
    SCROW theRow1;
    SCTAB theTab1;
    SCCOL theCol2;
    SCROW theRow2;
    SCTAB theTab2;
    GetArea( theTab1, theCol1, theRow1, theCol2, theRow2 );
    theTab2 = theTab1;

    bool bDoUpdate = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                            nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                            theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) != UR_NOTHING;
    if (bDoUpdate)
        MoveTo( theTab1, theCol1, theRow1, theCol2, theRow2 );

    ScRange aRangeAdvSource;
    if ( GetAdvancedQuerySource(aRangeAdvSource) )
    {
        aRangeAdvSource.GetVars( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                    nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                    theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
        {
            aRangeAdvSource.aStart.Set( theCol1,theRow1,theTab1 );
            aRangeAdvSource.aEnd.Set( theCol2,theRow2,theTab2 );
            SetAdvancedQuerySource( &aRangeAdvSource );

            bDoUpdate = true;       // DBData is modified
        }
    }

    SetModified(bDoUpdate);

    //!     Testen, ob mitten aus dem Bereich geloescht/eingefuegt wurde !!!
}

void ScDBData::ExtendDataArea(ScDocument* pDoc)
{
    // Extend the DB area to include data rows immediately below.
    // or shrink it if all cells are empty
    pDoc->GetDataArea(nTable, nStartCol, nStartRow, nEndCol, nEndRow, false, true);
}

namespace {

class FindByTable : public unary_function<ScDBData, bool>
{
    SCTAB mnTab;
public:
    FindByTable(SCTAB nTab) : mnTab(nTab) {}

    bool operator() (const ScDBData& r) const
    {
        ScRange aRange;
        r.GetArea(aRange);
        return aRange.aStart.Tab() == mnTab;
    }
};

class UpdateRefFunc : public unary_function<ScDBData, void>
{
    ScDocument* mpDoc;
    UpdateRefMode meMode;
    SCCOL mnCol1;
    SCROW mnRow1;
    SCTAB mnTab1;
    SCCOL mnCol2;
    SCROW mnRow2;
    SCTAB mnTab2;
    SCsCOL mnDx;
    SCsROW mnDy;
    SCsTAB mnDz;

public:
    UpdateRefFunc(ScDocument* pDoc, UpdateRefMode eMode,
                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz) :
        mpDoc(pDoc), meMode(eMode),
        mnCol1(nCol1), mnRow1(nRow1), mnTab1(nTab1),
        mnCol2(nCol2), mnRow2(nRow2), mnTab2(nTab2),
        mnDx(nDx), mnDy(nDy), mnDz(nDz) {}

    void operator() (ScDBData& r)
    {
        r.UpdateReference(mpDoc, meMode, mnCol1, mnRow1, mnTab1, mnCol2, mnRow2, mnTab2, mnDx, mnDy, mnDz);
    }
};

class UpdateMoveTabFunc : public unary_function<ScDBData, void>
{
    SCTAB mnOldTab;
    SCTAB mnNewTab;
public:
    UpdateMoveTabFunc(SCTAB nOld, SCTAB nNew) : mnOldTab(nOld), mnNewTab(nNew) {}
    void operator() (ScDBData& r)
    {
        r.UpdateMoveTab(mnOldTab, mnNewTab);
    }
};

class FindByCursor : public unary_function<ScDBData, bool>
{
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;
    bool mbStartOnly;
public:
    FindByCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly) :
        mnCol(nCol), mnRow(nRow), mnTab(nTab), mbStartOnly(bStartOnly) {}

    bool operator() (const ScDBData& r)
    {
        return r.IsDBAtCursor(mnCol, mnRow, mnTab, mbStartOnly);
    }
};

class FindByRange : public unary_function<ScDBData, bool>
{
    const ScRange& mrRange;
public:
    FindByRange(const ScRange& rRange) : mrRange(rRange) {}

    bool operator() (const ScDBData& r)
    {
        return r.IsDBAtArea(
            mrRange.aStart.Tab(), mrRange.aStart.Col(), mrRange.aStart.Row(), mrRange.aEnd.Col(), mrRange.aEnd.Row());
    }
};

class FindByIndex : public unary_function<ScDBData, bool>
{
    sal_uInt16 mnIndex;
public:
    FindByIndex(sal_uInt16 nIndex) : mnIndex(nIndex) {}
    bool operator() (const ScDBData& r) const
    {
        return r.GetIndex() == mnIndex;
    }
};

class FindByName : public unary_function<ScDBData, bool>
{
    const ::rtl::OUString& mrName;
public:
    FindByName(const ::rtl::OUString& rName) : mrName(rName) {}
    bool operator() (const ScDBData& r) const
    {
        return r.GetName() == mrName;
    }
};

class FindByUpperName : public unary_function<ScDBData, bool>
{
    const ::rtl::OUString& mrName;
public:
    FindByUpperName(const ::rtl::OUString& rName) : mrName(rName) {}
    bool operator() (const ScDBData& r) const
    {
        return r.GetUpperName() == mrName;
    }
};

}

ScDBCollection::NamedDBs::NamedDBs(ScDBCollection& rParent, ScDocument& rDoc) :
    mrParent(rParent), mrDoc(rDoc) {}

ScDBCollection::NamedDBs::NamedDBs(const NamedDBs& r) :
    maDBs(r.maDBs), mrParent(r.mrParent), mrDoc(r.mrDoc) {}

ScDBCollection::NamedDBs::iterator ScDBCollection::NamedDBs::begin()
{
    return maDBs.begin();
}

ScDBCollection::NamedDBs::iterator ScDBCollection::NamedDBs::end()
{
    return maDBs.end();
}

ScDBCollection::NamedDBs::const_iterator ScDBCollection::NamedDBs::begin() const
{
    return maDBs.begin();
}

ScDBCollection::NamedDBs::const_iterator ScDBCollection::NamedDBs::end() const
{
    return maDBs.end();
}

ScDBData* ScDBCollection::NamedDBs::findByIndex(sal_uInt16 nIndex)
{
    DBsType::iterator itr = find_if(
        maDBs.begin(), maDBs.end(), FindByIndex(nIndex));
    return itr == maDBs.end() ? NULL : &(*itr);
}

ScDBData* ScDBCollection::NamedDBs::findByName(const ::rtl::OUString& rName)
{
    DBsType::iterator itr = find_if(
        maDBs.begin(), maDBs.end(), FindByName(rName));
    return itr == maDBs.end() ? NULL : &(*itr);
}

ScDBData* ScDBCollection::NamedDBs::findByUpperName(const ::rtl::OUString& rName)
{
    DBsType::iterator itr = find_if(
        maDBs.begin(), maDBs.end(), FindByUpperName(rName));
    return itr == maDBs.end() ? NULL : &(*itr);
}

bool ScDBCollection::NamedDBs::insert(ScDBData* p)
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr<ScDBData> pData(p);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (!pData->GetIndex())
        pData->SetIndex(mrParent.nEntryIndex++);

    pair<DBsType::iterator, bool> r = maDBs.insert(pData);

    if (r.second && p->HasImportParam() && !p->HasImportSelection())
    {
        p->SetRefreshHandler(mrParent.GetRefreshHandler());
        p->SetRefreshControl(mrDoc.GetRefreshTimerControlAddress());
    }
    return r.second;
}

void ScDBCollection::NamedDBs::erase(iterator itr)
{
    maDBs.erase(itr);
}

void ScDBCollection::NamedDBs::erase(const ScDBData& r)
{
    maDBs.erase(r);
}

bool ScDBCollection::NamedDBs::empty() const
{
    return maDBs.empty();
}

size_t ScDBCollection::NamedDBs::size() const
{
    return maDBs.size();
}

bool ScDBCollection::NamedDBs::operator== (const NamedDBs& r) const
{
    return maDBs == r.maDBs;
}

ScDBCollection::AnonDBs::iterator ScDBCollection::AnonDBs::begin()
{
    return maDBs.begin();
}

ScDBCollection::AnonDBs::iterator ScDBCollection::AnonDBs::end()
{
    return maDBs.end();
}

ScDBCollection::AnonDBs::const_iterator ScDBCollection::AnonDBs::begin() const
{
    return maDBs.begin();
}

ScDBCollection::AnonDBs::const_iterator ScDBCollection::AnonDBs::end() const
{
    return maDBs.end();
}

const ScDBData* ScDBCollection::AnonDBs::findAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly) const
{
    DBsType::const_iterator itr = find_if(
        maDBs.begin(), maDBs.end(), FindByCursor(nCol, nRow, nTab, bStartOnly));
    return itr == maDBs.end() ? NULL : &(*itr);
}

const ScDBData* ScDBCollection::AnonDBs::findByRange(const ScRange& rRange) const
{
    DBsType::const_iterator itr = find_if(
        maDBs.begin(), maDBs.end(), FindByRange(rRange));
    return itr == maDBs.end() ? NULL : &(*itr);
}

ScDBData* ScDBCollection::AnonDBs::getByRange(const ScRange& rRange)
{
    const ScDBData* pData = findByRange(rRange);
    if (!pData)
    {
        // Insert a new db data.  They all have identical names.
        rtl::OUString aName(RTL_CONSTASCII_USTRINGPARAM(STR_DB_GLOBAL_NONAME));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScDBData> pNew(new ScDBData(
            aName, rRange.aStart.Tab(), rRange.aStart.Col(), rRange.aStart.Row(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), true, false));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        pData = pNew.get();
        maDBs.push_back(pNew);
    }
    return const_cast<ScDBData*>(pData);
}

void ScDBCollection::AnonDBs::insert(ScDBData* p)
{
    rtl::OUString aName(RTL_CONSTASCII_USTRINGPARAM(STR_DB_GLOBAL_NONAME));
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDBData> pNew(p);
        SAL_WNODEPRECATED_DECLARATIONS_POP
    maDBs.push_back(pNew);
}

bool ScDBCollection::AnonDBs::empty() const
{
    return maDBs.empty();
}

bool ScDBCollection::AnonDBs::operator== (const AnonDBs& r) const
{
    return maDBs == r.maDBs;
}

ScDBCollection::ScDBCollection(ScDocument* pDocument) :
    pDoc(pDocument), nEntryIndex(SC_START_INDEX_DB_COLL), maNamedDBs(*this, *pDocument) {}

ScDBCollection::ScDBCollection(const ScDBCollection& r) :
    pDoc(r.pDoc), nEntryIndex(r.nEntryIndex), maNamedDBs(r.maNamedDBs), maAnonDBs(r.maAnonDBs) {}

ScDBCollection::NamedDBs& ScDBCollection::getNamedDBs()
{
    return maNamedDBs;
}

const ScDBCollection::NamedDBs& ScDBCollection::getNamedDBs() const
{
    return maNamedDBs;
}

ScDBCollection::AnonDBs& ScDBCollection::getAnonDBs()
{
    return maAnonDBs;
}

const ScDBCollection::AnonDBs& ScDBCollection::getAnonDBs() const
{
    return maAnonDBs;
}

const ScDBData* ScDBCollection::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bStartOnly) const
{
    // First, search the global named db ranges.
    NamedDBs::DBsType::const_iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByCursor(nCol, nRow, nTab, bStartOnly));
    if (itr != maNamedDBs.end())
        return &(*itr);

    // Check for the sheet-local anonymous db range.
    const ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtCursor(nCol,nRow,nTab,bStartOnly))
            return pNoNameData;

    // Check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findAtCursor(nCol, nRow, nTab, bStartOnly);
    if (pData)
        return pData;

    return NULL;
}

ScDBData* ScDBCollection::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bStartOnly)
{
    // First, search the global named db ranges.
    NamedDBs::DBsType::iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByCursor(nCol, nRow, nTab, bStartOnly));
    if (itr != maNamedDBs.end())
        return &(*itr);

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtCursor(nCol,nRow,nTab,bStartOnly))
            return pNoNameData;

    // Check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findAtCursor(nCol, nRow, nTab, bStartOnly);
    if (pData)
        return const_cast<ScDBData*>(pData);

    return NULL;
}

const ScDBData* ScDBCollection::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    // First, search the global named db ranges.
    ScRange aRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    NamedDBs::DBsType::const_iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByRange(aRange));
    if (itr != maNamedDBs.end())
        return &(*itr);

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    // Lastly, check the global anonymous db ranges.
    return maAnonDBs.findByRange(aRange);
}

ScDBData* ScDBCollection::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    // First, search the global named db ranges.
    ScRange aRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    NamedDBs::DBsType::iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByRange(aRange));
    if (itr != maNamedDBs.end())
        return &(*itr);

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    // Lastly, check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findByRange(aRange);
    if (pData)
        return const_cast<ScDBData*>(pData);

    return NULL;
}

void ScDBCollection::DeleteOnTab( SCTAB nTab )
{
    FindByTable func(nTab);
    // First, collect the positions of all items that need to be deleted.
    ::std::vector<NamedDBs::DBsType::iterator> v;
    {
        NamedDBs::DBsType::iterator itr = maNamedDBs.begin(), itrEnd = maNamedDBs.end();
        for (; itr != itrEnd; ++itr)
        {
            if (func(*itr))
                v.push_back(itr);
        }
    }

    // Delete them all.
    ::std::vector<NamedDBs::DBsType::iterator>::iterator itr = v.begin(), itrEnd = v.end();
    for (; itr != itrEnd; ++itr)
        maNamedDBs.erase(*itr);

    remove_if(maAnonDBs.begin(), maAnonDBs.end(), func);
}

void ScDBCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    ScDBData* pData = pDoc->GetAnonymousDBData(nTab1);
    if (pData)
    {
        if (nTab1 == nTab2 && nDz == 0)
        {
            pData->UpdateReference(
                pDoc, eUpdateRefMode,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz);
        }
        else
        {
            //this will perhabs break undo
        }
    }

    UpdateRefFunc func(pDoc, eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz);
    for_each(maNamedDBs.begin(), maNamedDBs.end(), func);
    for_each(maAnonDBs.begin(), maAnonDBs.end(), func);
}


void ScDBCollection::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    UpdateMoveTabFunc func(nOldPos, nNewPos);
    for_each(maNamedDBs.begin(), maNamedDBs.end(), func);
    for_each(maAnonDBs.begin(), maAnonDBs.end(), func);
}

ScDBData* ScDBCollection::GetDBNearCursor(SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScDBData* pNearData = NULL;
    NamedDBs::DBsType::iterator itr = maNamedDBs.begin(), itrEnd = maNamedDBs.end();
    for (; itr != itrEnd; ++itr)
    {
        SCTAB nAreaTab;
        SCCOL nStartCol, nEndCol;
        SCROW nStartRow, nEndRow;
        itr->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
        if ( nTab == nAreaTab && nCol+1 >= nStartCol && nCol <= nEndCol+1 &&
                                 nRow+1 >= nStartRow && nRow <= nEndRow+1 )
        {
            if ( nCol < nStartCol || nCol > nEndCol || nRow < nStartRow || nRow > nEndRow )
            {
                if (!pNearData)
                    pNearData = &(*itr);    // ersten angrenzenden Bereich merken
            }
            else
                return &(*itr);             // nicht "unbenannt" und Cursor steht wirklich drin
        }
    }
    if (pNearData)
        return pNearData;               // angrenzender, wenn nichts direkt getroffen
    return pDoc->GetAnonymousDBData(nTab);                  // "unbenannt" nur zurueck, wenn sonst nichts gefunden
}

bool ScDBCollection::empty() const
{
    return maNamedDBs.empty() && maAnonDBs.empty();
}

bool ScDBCollection::operator== (const ScDBCollection& r) const
{
    return maNamedDBs == r.maNamedDBs && maAnonDBs == r.maAnonDBs &&
        nEntryIndex == r.nEntryIndex && pDoc == r.pDoc && aRefreshHandler == r.aRefreshHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
