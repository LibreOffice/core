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

#include "ScChart2DataSequence.hxx"

#include "formula/token.hxx"
#include "osl/diagnose.h"
#include "vcl/svapp.hxx"

#include "brdcst.hxx"
#include "cellvalue.hxx"
#include "compiler.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "globstr.hrc"
#include "hints.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"
#include "reftokenhelper.hxx"
#include "scmatrix.hxx"
#include "simplehintids.hxx"
#include "tokenarray.hxx"
#include "unoreflist.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace formula;
using namespace std;
using std::shared_ptr;
using rtl::OUString;

SC_SIMPLE_SERVICE_INFO( ScChart2DataSequence, "ScChart2DataSequence",
        "com.sun.star.chart2.data.DataSequence")

namespace {

/**
 * This function object is used to accumulatively count the numbers of
 * columns and rows in all reference tokens.
 */
class AccumulateRangeSize : public unary_function<ScTokenRef, void>
{
public:
    AccumulateRangeSize() :
        mnCols(0), mnRows(0) {}

    AccumulateRangeSize(const AccumulateRangeSize& r) :
        mnCols(r.mnCols), mnRows(r.mnRows) {}

    void operator() (const ScTokenRef& pToken)
    {
        ScRange r;
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        ScRefTokenHelper::getRangeFromToken(r, pToken, ScAddress(), bExternal);
        r.Justify();
        mnCols += r.aEnd.Col() - r.aStart.Col() + 1;
        mnRows += r.aEnd.Row() - r.aStart.Row() + 1;
    }

    SCCOL getCols() const { return mnCols; }
    SCROW getRows() const { return mnRows; }
private:
    SCCOL mnCols;
    SCROW mnRows;
};


/**
 * Function object to create a range string from a token list.
 */
class Tokens2RangeString : public unary_function<ScTokenRef, void>
{
public:
    Tokens2RangeString(ScDocument* pDoc, FormulaGrammar::Grammar eGram, sal_Unicode cRangeSep) :
        mpRangeStr(new OUStringBuffer),
        mpDoc(pDoc),
        meGrammar(eGram),
        mcRangeSep(cRangeSep),
        mbFirst(true)
    {
    }

    Tokens2RangeString(const Tokens2RangeString& r) :
        mpRangeStr(r.mpRangeStr),
        mpDoc(r.mpDoc),
        meGrammar(r.meGrammar),
        mcRangeSep(r.mcRangeSep),
        mbFirst(r.mbFirst)
    {
    }

    void operator() (const ScTokenRef& rToken)
    {
        ScCompiler aCompiler(mpDoc, ScAddress(0,0,0));
        aCompiler.SetGrammar(meGrammar);
        OUString aStr;
        aCompiler.CreateStringFromToken(aStr, rToken.get());
        if (mbFirst)
            mbFirst = false;
        else
            mpRangeStr->append(mcRangeSep);
        mpRangeStr->append(aStr);
    }

    void getString(OUString& rStr)
    {
        rStr = mpRangeStr->makeStringAndClear();
    }

private:
    shared_ptr<OUStringBuffer>  mpRangeStr;
    ScDocument*         mpDoc;
    FormulaGrammar::Grammar  meGrammar;
    sal_Unicode         mcRangeSep;
    bool                mbFirst;
};


/**
 * This function object is used to generate label strings from a list of
 * reference tokens.
 */
class GenerateLabelStrings : public unary_function<ScTokenRef, void>
{
public:
    GenerateLabelStrings(sal_Int32 nSize, chart2::data::LabelOrigin eOrigin, bool bColumn) :
        mpLabels(new Sequence<OUString>(nSize)),
        meOrigin(eOrigin),
        mnCount(0),
        mbColumn(bColumn) {}

    GenerateLabelStrings(const GenerateLabelStrings& r) :
        mpLabels(r.mpLabels),
        meOrigin(r.meOrigin),
        mnCount(r.mnCount),
        mbColumn(r.mbColumn) {}

    void operator() (const ScTokenRef& pToken)
    {
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        ScRange aRange;
        ScRefTokenHelper::getRangeFromToken(aRange, pToken, ScAddress(), bExternal);
        OUString* pArr = mpLabels->getArray();
        if (mbColumn)
        {
            for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
            {
                if ( meOrigin != chart2::data::LabelOrigin_LONG_SIDE)
                {
                    OUString aString = ScGlobal::GetRscString(STR_COLUMN);
                    aString += " ";
                    ScAddress aPos( nCol, 0, 0 );
                    OUString aColStr(aPos.Format(SCA_VALID_COL, NULL));
                    aString += aColStr;
                    pArr[mnCount] = aString;
                }
                else //only indices for categories
                    pArr[mnCount] = OUString::number( mnCount+1 );
                ++mnCount;
            }
        }
        else
        {
            for (sal_Int32 nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
            {
                if (meOrigin != chart2::data::LabelOrigin_LONG_SIDE)
                {
                    OUString aString = ScGlobal::GetRscString(STR_ROW) +
                                       " " + OUString::number( nRow+1 );
                    pArr[mnCount] = aString;
                }
                else //only indices for categories
                    pArr[mnCount] = OUString::number( mnCount+1 );
                ++mnCount;
            }
        }
    }

    Sequence<OUString> getLabels() const { return *mpLabels; }

private:
    shared_ptr< Sequence<OUString> >    mpLabels;
    chart2::data::LabelOrigin           meOrigin;
    sal_Int32                           mnCount;
    bool                                mbColumn;
};


const SfxItemPropertyMapEntry* lcl_GetDataSequencePropertyMap()
{
    static const SfxItemPropertyMapEntry aDataSequencePropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_HIDDENVALUES), 0, cppu::UnoType<uno::Sequence<sal_Int32>>::get(),                 0, 0 },
        {OUString(SC_UNONAME_ROLE), 0, cppu::UnoType<com::sun::star::chart2::data::DataSequenceRole>::get(),                  0, 0 },
        {OUString(SC_UNONAME_INCLUDEHIDDENCELLS), 0,        cppu::UnoType<bool>::get(),                  0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataSequencePropertyMap_Impl;
}

sal_uLong getDisplayNumberFormat(ScDocument* pDoc, const ScAddress& rPos)
{
    sal_uLong nFormat = pDoc->GetNumberFormat(rPos); // original format from cell.
    return nFormat;
}

}

/*static*/
void ScChart2DataSequence::convertTokensToString(OUString& rStr, const vector<ScTokenRef>& rTokens, ScDocument* pDoc)
{
    const sal_Unicode cRangeSep = ScCompiler::GetNativeSymbolChar(ocSep);
    FormulaGrammar::Grammar eGrammar = pDoc->GetGrammar();
    Tokens2RangeString func(pDoc, eGrammar, cRangeSep);
    func = ::std::for_each(rTokens.begin(), rTokens.end(), func);
    func.getString(rStr);
}


ScChart2DataSequence::Item::Item() :
    mfValue(0.0), mbIsValue(false)
{
    ::rtl::math::setNan(&mfValue);
}

ScChart2DataSequence::HiddenRangeListener::HiddenRangeListener(ScChart2DataSequence& rParent) :
    mrParent(rParent)
{
}

ScChart2DataSequence::HiddenRangeListener::~HiddenRangeListener()
{
}

void ScChart2DataSequence::HiddenRangeListener::notify()
{
    mrParent.setDataChangedHint(true);
}

ScChart2DataSequence::ScChart2DataSequence( ScDocument* pDoc,
        const uno::Reference < chart2::data::XDataProvider >& xDP,
        vector<ScTokenRef>* pTokens,
        bool bIncludeHiddenCells )
    : m_bIncludeHiddenCells( bIncludeHiddenCells)
    , m_nObjectId( 0 )
    , m_pDocument( pDoc)
    , m_pTokens(pTokens)
    , m_pRangeIndices(NULL)
    , m_pExtRefListener(NULL)
    , m_xDataProvider( xDP)
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
    , m_pHiddenListener(NULL)
    , m_pValueListener( NULL )
    , m_bGotDataChangedHint(false)
    , m_bExtDataRebuildQueued(false)
    , mbTimeBased(false)
    , mnTimeBasedStart(0)
    , mnTimeBasedEnd(0)
    , mnCurrentTab(0)
{
    OSL_ENSURE(pTokens, "reference token list is null");

    if ( m_pDocument )
    {
        m_pDocument->AddUnoObject( *this);
        m_nObjectId = m_pDocument->GetNewUnoId();
    }
    // FIXME: real implementation of identifier and it's mapping to ranges.
    // Reuse ScChartListener?

    // BM: don't use names of named ranges but the UI range strings
//  String  aStr;
//  rRangeList->Format( aStr, SCR_ABS_3D, m_pDocument );
//    m_aIdentifier = aStr;

//      m_aIdentifier = "ID_";
//      static sal_Int32 nID = 0;
//      m_aIdentifier += OUString::valueOf( ++nID);
}

ScChart2DataSequence::~ScChart2DataSequence()
{
    SolarMutexGuard g;

    if ( m_pDocument )
    {
        m_pDocument->RemoveUnoObject( *this);
        if (m_pHiddenListener.get())
        {
            ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
            if (pCLC)
                pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
        }
        StopListeningToAllExternalRefs();
    }

    delete m_pValueListener;
}

void ScChart2DataSequence::RefChanged()
{
    if( m_pValueListener && !m_aValueListeners.empty() )
    {
        m_pValueListener->EndListeningAll();

        if( m_pDocument )
        {
            ScChartListenerCollection* pCLC = NULL;
            if (m_pHiddenListener.get())
            {
                pCLC = m_pDocument->GetChartListenerCollection();
                if (pCLC)
                    pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
            }

            vector<ScTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
            for (; itr != itrEnd; ++itr)
            {
                ScRange aRange;
                if (!ScRefTokenHelper::getRangeFromToken(aRange, *itr, ScAddress()))
                    continue;

                m_pDocument->StartListeningArea(aRange, false, m_pValueListener);
                if (pCLC)
                    pCLC->StartListeningHiddenRange(aRange, m_pHiddenListener.get());
            }
        }
    }
}

void ScChart2DataSequence::BuildDataCache()
{
    m_bExtDataRebuildQueued = false;

    if (!m_aDataArray.empty())
        return;

    if (!m_pTokens.get())
    {
        OSL_FAIL("m_pTokens == NULL!  Something is wrong.");
        return;
    }

    StopListeningToAllExternalRefs();

    ::std::list<sal_Int32> aHiddenValues;
    sal_Int32 nDataCount = 0;
    sal_Int32 nHiddenValueCount = 0;

    for (vector<ScTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
          itr != itrEnd; ++itr)
    {
        if (ScRefTokenHelper::isExternalRef(*itr))
        {
            nDataCount += FillCacheFromExternalRef(*itr);
        }
        else
        {
            ScRange aRange;
            if (!ScRefTokenHelper::getRangeFromToken(aRange, *itr, ScAddress()))
                continue;

            SCCOL nLastCol = -1;
            SCROW nLastRow = -1;
            for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); ++nTab)
            {
                for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
                {
                    for (SCROW nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
                    {
                        bool bColHidden = m_pDocument->ColHidden(nCol, nTab, NULL, &nLastCol);
                        bool bRowHidden = m_pDocument->RowHidden(nRow, nTab, NULL, &nLastRow);

                        if (bColHidden || bRowHidden)
                        {
                            // hidden cell
                            ++nHiddenValueCount;
                            aHiddenValues.push_back(nDataCount-1);

                            if( !m_bIncludeHiddenCells )
                                continue;
                        }

                        Item aItem;

                        ScAddress aAdr(nCol, nRow, nTab);
                        aItem.maString = m_pDocument->GetString(aAdr);

                        switch (m_pDocument->GetCellType(aAdr))
                        {
                            case CELLTYPE_VALUE:
                                aItem.mfValue = m_pDocument->GetValue(aAdr);
                                aItem.mbIsValue = true;
                            break;
                            case CELLTYPE_FORMULA:
                            {
                                ScFormulaCell* pFCell = m_pDocument->GetFormulaCell(aAdr);
                                if (!pFCell)
                                    break;
                                sal_uInt16 nErr = pFCell->GetErrCode();
                                if (nErr)
                                    break;

                                if (pFCell->IsValue())
                                {
                                    aItem.mfValue = pFCell->GetValue();
                                    aItem.mbIsValue = true;
                                }
                            }
                            break;
                            case CELLTYPE_EDIT:
                            case CELLTYPE_NONE:
                            case CELLTYPE_STRING:
                            default:
                                ; // do nothing
                        }

                        m_aDataArray.push_back(aItem);
                        ++nDataCount;
                    }
                }
            }
        }
    }

    // convert the hidden cell list to sequence.
    m_aHiddenValues.realloc(nHiddenValueCount);
    sal_Int32* pArr = m_aHiddenValues.getArray();
    ::std::list<sal_Int32>::const_iterator itr = aHiddenValues.begin(), itrEnd = aHiddenValues.end();
    for (;itr != itrEnd; ++itr, ++pArr)
        *pArr = *itr;

    // Clear the data series cache when the array is re-built.
    m_aMixedDataCache.realloc(0);
}

void ScChart2DataSequence::RebuildDataCache()
{
    if (!m_bExtDataRebuildQueued)
    {
        m_aDataArray.clear();
        m_pDocument->BroadcastUno(ScHint(SC_HINT_DATACHANGED, ScAddress()));
        m_bExtDataRebuildQueued = true;
        m_bGotDataChangedHint = true;
    }
}

sal_Int32 ScChart2DataSequence::FillCacheFromExternalRef(const ScTokenRef& pToken)
{
    ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
    ScRange aRange;
    if (!ScRefTokenHelper::getRangeFromToken(aRange, pToken, ScAddress(), true))
        return 0;

    sal_uInt16 nFileId = pToken->GetIndex();
    OUString aTabName = pToken->GetString().getString();
    ScExternalRefCache::TokenArrayRef pArray = pRefMgr->getDoubleRefTokens(nFileId, aTabName, aRange, NULL);
    if (!pArray)
        // no external data exists for this range.
        return 0;

    // Start listening for this external document.
    ExternalRefListener* pExtRefListener = GetExtRefListener();
    pRefMgr->addLinkListener(nFileId, pExtRefListener);
    pExtRefListener->addFileId(nFileId);

    ScExternalRefCache::TableTypeRef pTable = pRefMgr->getCacheTable(nFileId, aTabName, false, NULL);
    sal_Int32 nDataCount = 0;
    for (FormulaToken* p = pArray->First(); p; p = pArray->Next())
    {
        // Cached external range is always represented as a single
        // matrix token, although that might change in the future when
        // we introduce a new token type to store multi-table range
        // data.

        if (p->GetType() != svMatrix)
        {
            OSL_FAIL("Cached array is not a matrix token.");
            continue;
        }

        const ScMatrix* pMat = p->GetMatrix();
        SCSIZE nCSize, nRSize;
        pMat->GetDimensions(nCSize, nRSize);
        for (SCSIZE nC = 0; nC < nCSize; ++nC)
        {
            for (SCSIZE nR = 0; nR < nRSize; ++nR)
            {
                if (pMat->IsValue(nC, nR) || pMat->IsBoolean(nC, nR))
                {
                    Item aItem;

                    aItem.mbIsValue = true;
                    aItem.mfValue = pMat->GetDouble(nC, nR);

                    SvNumberFormatter* pFormatter = m_pDocument->GetFormatTable();
                    if (pFormatter)
                    {
                        const double fVal = aItem.mfValue;
                        Color* pColor = NULL;
                        sal_uInt32 nFmt = 0;
                        if (pTable)
                        {
                            // Get the correct format index from the cache.
                            SCCOL nCol = aRange.aStart.Col() + static_cast<SCCOL>(nC);
                            SCROW nRow = aRange.aStart.Row() + static_cast<SCROW>(nR);
                            pTable->getCell(nCol, nRow, &nFmt);
                        }
                        pFormatter->GetOutputString(fVal, nFmt, aItem.maString, &pColor);
                    }

                    m_aDataArray.push_back(aItem);
                    ++nDataCount;
                }
                else if (pMat->IsString(nC, nR))
                {
                    Item aItem;

                    aItem.mbIsValue = false;
                    aItem.maString = pMat->GetString(nC, nR).getString();

                    m_aDataArray.push_back(Item());
                    ++nDataCount;
                }
            }
        }
    }
    return nDataCount;
}

void ScChart2DataSequence::UpdateTokensFromRanges(const ScRangeList& rRanges)
{
    if (!m_pRangeIndices.get())
        return;

    for ( size_t i = 0, nCount = rRanges.size(); i < nCount; ++i )
    {
        ScTokenRef pToken;
        const ScRange* pRange = rRanges[i];
        OSL_ENSURE(pRange, "range object is NULL.");

        ScRefTokenHelper::getTokenFromRange(pToken, *pRange);
        sal_uInt32 nOrigPos = (*m_pRangeIndices)[i];
        (*m_pTokens)[nOrigPos] = pToken;
    }

    RefChanged();

    // any change of the range address is broadcast to value (modify) listeners
    if ( !m_aValueListeners.empty() )
        m_bGotDataChangedHint = true;
}

ScChart2DataSequence::ExternalRefListener* ScChart2DataSequence::GetExtRefListener()
{
    if (!m_pExtRefListener.get())
        m_pExtRefListener.reset(new ExternalRefListener(*this, m_pDocument));

    return m_pExtRefListener.get();
}

void ScChart2DataSequence::StopListeningToAllExternalRefs()
{
    if (!m_pExtRefListener.get())
        return;

    const std::unordered_set<sal_uInt16>& rFileIds = m_pExtRefListener->getAllFileIds();
    std::unordered_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
    ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
    for (; itr != itrEnd; ++itr)
        pRefMgr->removeLinkListener(*itr, m_pExtRefListener.get());

    m_pExtRefListener.reset();
}

void ScChart2DataSequence::CopyData(const ScChart2DataSequence& r)
{
    if (!m_pDocument)
    {
        OSL_FAIL("document instance is NULL!?");
        return;
    }

    list<Item> aDataArray(r.m_aDataArray);
    m_aDataArray.swap(aDataArray);

    m_aHiddenValues = r.m_aHiddenValues;
    m_aRole = r.m_aRole;

    if (r.m_pRangeIndices.get())
        m_pRangeIndices.reset(new vector<sal_uInt32>(*r.m_pRangeIndices));

    if (r.m_pExtRefListener.get())
    {
        // Re-register all external files that the old instance was
        // listening to.

        ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
        m_pExtRefListener.reset(new ExternalRefListener(*this, m_pDocument));
        const std::unordered_set<sal_uInt16>& rFileIds = r.m_pExtRefListener->getAllFileIds();
        std::unordered_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
        for (; itr != itrEnd; ++itr)
        {
            pRefMgr->addLinkListener(*itr, m_pExtRefListener.get());
            m_pExtRefListener->addFileId(*itr);
        }
    }
}

void ScChart2DataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint )
    {
        sal_uLong nId = pSimpleHint->GetId();
        if ( nId ==SFX_HINT_DYING )
        {
            m_pDocument = NULL;
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            // delayed broadcast as in ScCellRangesBase

            if ( m_bGotDataChangedHint && m_pDocument )
            {
                m_aDataArray.clear();
                lang::EventObject aEvent;
                aEvent.Source.set((cppu::OWeakObject*)this);

                if( m_pDocument )
                {
                    for ( size_t n=0; n<m_aValueListeners.size(); n++ )
                        m_pDocument->AddUnoListenerCall( m_aValueListeners[n], aEvent );
                }

                m_bGotDataChangedHint = false;
            }
        }
        else if ( nId == SC_HINT_CALCALL )
        {
            // broadcast from DoHardRecalc - set m_bGotDataChangedHint
            // (SFX_HINT_DATACHANGED follows separately)

            if ( !m_aValueListeners.empty() )
                m_bGotDataChangedHint = true;
        }
    }
    else if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        // Create a range list from the token list, have the range list
        // updated, and bring the change back to the token list.

        ScRangeList aRanges;
        m_pRangeIndices.reset(new vector<sal_uInt32>());
        vector<ScTokenRef>::const_iterator itrBeg = m_pTokens->begin(), itrEnd = m_pTokens->end();
        for (vector<ScTokenRef>::const_iterator itr = itrBeg ;itr != itrEnd; ++itr)
        {
            if (!ScRefTokenHelper::isExternalRef(*itr))
            {
                ScRange aRange;
                ScRefTokenHelper::getRangeFromToken(aRange, *itr, ScAddress());
                aRanges.Append(aRange);
                sal_uInt32 nPos = distance(itrBeg, itr);
                m_pRangeIndices->push_back(nPos);
            }
        }

        OSL_ENSURE(m_pRangeIndices->size() == static_cast<size_t>(aRanges.size()),
                   "range list and range index list have different sizes.");

        unique_ptr<ScRangeList> pUndoRanges;
        if ( m_pDocument->HasUnoRefUndo() )
            pUndoRanges.reset(new ScRangeList(aRanges));

        const ScUpdateRefHint& rRef = static_cast<const ScUpdateRefHint&>(rHint);
        bool bChanged = aRanges.UpdateReference(
            rRef.GetMode(), m_pDocument, rRef.GetRange(), rRef.GetDx(), rRef.GetDy(), rRef.GetDz());

        if (bChanged)
        {
            OSL_ENSURE(m_pRangeIndices->size() == aRanges.size(),
                       "range list and range index list have different sizes after the reference update.");

            // Bring the change back from the range list to the token list.
            UpdateTokensFromRanges(aRanges);

            if (pUndoRanges.get())
                m_pDocument->AddUnoRefChange(m_nObjectId, *pUndoRanges);
        }
    }
    else if ( dynamic_cast<const ScUnoRefUndoHint*>(&rHint) )
    {
        const ScUnoRefUndoHint& rUndoHint = static_cast<const ScUnoRefUndoHint&>(rHint);

        do
        {
            if (rUndoHint.GetObjectId() != m_nObjectId)
                break;

            // The hint object provides the old ranges.  Restore the old state
            // from these ranges.

            if (!m_pRangeIndices.get() || m_pRangeIndices->empty())
            {
                OSL_FAIL(" faulty range indices");
                break;
            }

            const ScRangeList& rRanges = rUndoHint.GetRanges();

            size_t nCount = rRanges.size();
            if (nCount != m_pRangeIndices->size())
            {
                OSL_FAIL("range count and range index count differ.");
                break;
            }

            UpdateTokensFromRanges(rRanges);
        }
        while (false);
    }
}

IMPL_LINK( ScChart2DataSequence, ValueListenerHdl, SfxHint*, pHint )
{
    if ( m_pDocument && pHint && dynamic_cast<const SfxSimpleHint*>(pHint) &&
            static_cast<const SfxSimpleHint*>(pHint)->GetId() & SC_HINT_DATACHANGED)
    {
        //  This may be called several times for a single change, if several formulas
        //  in the range are notified. So only a flag is set that is checked when
        //  SFX_HINT_DATACHANGED is received.

        setDataChangedHint(true);
    }
    return 0;
}

ScChart2DataSequence::ExternalRefListener::ExternalRefListener(
    ScChart2DataSequence& rParent, ScDocument* pDoc) :
    ScExternalRefManager::LinkListener(),
    mrParent(rParent),
    mpDoc(pDoc)
{
}

ScChart2DataSequence::ExternalRefListener::~ExternalRefListener()
{
    if (!mpDoc || mpDoc->IsInDtorClear())
        // The document is being destroyed.  Do nothing.
        return;

    // Make sure to remove all pointers to this object.
    mpDoc->GetExternalRefManager()->removeLinkListener(this);
}

void ScChart2DataSequence::ExternalRefListener::notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType)
{
    switch (eType)
    {
        case ScExternalRefManager::LINK_MODIFIED:
        {
            if (maFileIds.count(nFileId))
                // We are listening to this external document.
                mrParent.RebuildDataCache();
        }
        break;
        case ScExternalRefManager::LINK_BROKEN:
            removeFileId(nFileId);
        break;
    }
}

void ScChart2DataSequence::ExternalRefListener::addFileId(sal_uInt16 nFileId)
{
    maFileIds.insert(nFileId);
}

void ScChart2DataSequence::ExternalRefListener::removeFileId(sal_uInt16 nFileId)
{
    maFileIds.erase(nFileId);
}

uno::Sequence< uno::Any> SAL_CALL ScChart2DataSequence::getData()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    BuildDataCache();

    if (!m_aMixedDataCache.getLength())
    {
        // Build a cache for the 1st time...

        sal_Int32 nCount = m_aDataArray.size();
        m_aMixedDataCache.realloc(nCount);
        uno::Any* pArr = m_aMixedDataCache.getArray();
        ::std::list<Item>::const_iterator itr = m_aDataArray.begin(), itrEnd = m_aDataArray.end();
        for (; itr != itrEnd; ++itr, ++pArr)
        {
            if (itr->mbIsValue)
                *pArr <<= itr->mfValue;
            else
                *pArr <<= itr->maString;
        }
    }
    return m_aMixedDataCache;
}

// XNumericalDataSequence --------------------------------------------------

uno::Sequence< double > SAL_CALL ScChart2DataSequence::getNumericalData()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    BuildDataCache();

    double fNAN;
    ::rtl::math::setNan(&fNAN);

    sal_Int32 nCount = m_aDataArray.size();
    uno::Sequence<double> aSeq(nCount);
    double* pArr = aSeq.getArray();
    ::std::list<Item>::const_iterator itr = m_aDataArray.begin(), itrEnd = m_aDataArray.end();
    for (; itr != itrEnd; ++itr, ++pArr)
        *pArr = itr->mbIsValue ? itr->mfValue : fNAN;

    return aSeq;
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence< OUString > SAL_CALL ScChart2DataSequence::getTextualData()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aSeq;
    if ( !m_pDocument )
        throw uno::RuntimeException();

    BuildDataCache();

    sal_Int32 nCount = m_aDataArray.size();
    if ( nCount > 0 )
    {
        aSeq =  uno::Sequence<OUString>(nCount);
        OUString* pArr = aSeq.getArray();
        ::std::list<Item>::const_iterator itr = m_aDataArray.begin(), itrEnd = m_aDataArray.end();
        for(; itr != itrEnd; ++itr, ++pArr)
            *pArr = itr->maString;
    }
    else if ( m_pTokens.get() && m_pTokens->front() )
    {
        if( m_pTokens->front()->GetType() == svString )
        {
            aSeq = uno::Sequence<OUString>(1);
            aSeq[0] = m_pTokens->front()->GetString().getString();
        }
    }

    return aSeq;
}

OUString SAL_CALL ScChart2DataSequence::getSourceRangeRepresentation()
            throw ( uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aStr;
    OSL_ENSURE( m_pDocument, "No Document -> no SourceRangeRepresentation" );
    if (m_pDocument && m_pTokens.get())
        convertTokensToString(aStr, *m_pTokens, m_pDocument);

    return aStr;
}


uno::Sequence< OUString > SAL_CALL ScChart2DataSequence::generateLabel(chart2::data::LabelOrigin eOrigin)
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    if (!m_pTokens.get())
        return Sequence<OUString>();

    // Determine the total size of all ranges.
    AccumulateRangeSize func;
    func = ::std::for_each(m_pTokens->begin(), m_pTokens->end(), func);
    SCCOL nCols = func.getCols();
    SCROW nRows = func.getRows();

    // Detemine whether this is column-major or row-major.
    bool bColumn = true;
    if ((eOrigin == chart2::data::LabelOrigin_SHORT_SIDE) ||
        (eOrigin == chart2::data::LabelOrigin_LONG_SIDE))
    {
        if (nRows > nCols)
        {
            if (eOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
                bColumn = true;
            else
                bColumn = false;
        }
        else if (nCols > nRows)
        {
            if (eOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
                bColumn = false;
            else
                bColumn = true;
        }
        else
            return Sequence<OUString>();
    }

    // Generate label strings based on the info so far.
    sal_Int32 nCount = bColumn ? nCols : nRows;
    GenerateLabelStrings genLabels(nCount, eOrigin, bColumn);
    genLabels = ::std::for_each(m_pTokens->begin(), m_pTokens->end(), genLabels);
    Sequence<OUString> aSeq = genLabels.getLabels();

    return aSeq;
}


::sal_Int32 SAL_CALL ScChart2DataSequence::getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    // index -1 means a heuristic value for the entire sequence
    bool bGetSeriesFormat = (nIndex == -1);

    SolarMutexGuard aGuard;
    if ( !m_pDocument || !m_pTokens.get())
        return 0;

    // TODO: Handle external references too.

    sal_Int32 nCount = 0;

    ScRangeList aRanges;
    ScRefTokenHelper::getRangeListFromTokens(aRanges, *m_pTokens, ScAddress());
    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        ScRange* p = aRanges[i];
        for (SCTAB nTab = p->aStart.Tab(); nTab <= p->aEnd.Tab(); ++nTab)
        {
            for (SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
            {
                if (!m_bIncludeHiddenCells)
                {
                    // Skip hidden columns.
                    SCCOL nLastCol = -1;
                    bool bColHidden = m_pDocument->ColHidden(nCol, nTab, NULL, &nLastCol);
                    if (bColHidden)
                    {
                        nCol = nLastCol;
                        continue;
                    }
                }

                for (SCROW nRow = p->aStart.Row(); nRow <= p->aEnd.Row(); ++nRow)
                {
                    if (!m_bIncludeHiddenCells)
                    {
                        // Skip hidden rows.
                        SCROW nLastRow = -1;
                        bool bRowHidden = m_pDocument->RowHidden(nRow, nTab, NULL, &nLastRow);
                        if (bRowHidden)
                        {
                            nRow = nLastRow;
                            continue;
                        }
                    }

                    ScAddress aPos(nCol, nRow, nTab);

                    if( bGetSeriesFormat )
                    {
                        // TODO: use nicer heuristic
                        // return format of first non-empty cell
                        ScRefCellValue aCell;
                        aCell.assign(*m_pDocument, aPos);
                        if (!aCell.isEmpty())
                            return static_cast<sal_Int32>(getDisplayNumberFormat(m_pDocument, aPos));
                    }
                    else if( nCount == nIndex )
                    {
                        return static_cast<sal_Int32>(getDisplayNumberFormat(m_pDocument, aPos));
                    }
                    ++nCount;
                }
            }
        }
    }
    return 0;
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2DataSequence::createClone()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    unique_ptr< vector<ScTokenRef> > pTokensNew;
    if (m_pTokens.get())
    {
        // Clone tokens.
        pTokensNew.reset(new vector<ScTokenRef>);
        pTokensNew->reserve(m_pTokens->size());
        vector<ScTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
        for (; itr != itrEnd; ++itr)
        {
            ScTokenRef p((*itr)->Clone());
            pTokensNew->push_back(p);
        }
    }

    unique_ptr<ScChart2DataSequence> p(new ScChart2DataSequence(m_pDocument, m_xDataProvider, pTokensNew.release(), m_bIncludeHiddenCells));
    p->CopyData(*this);
    Reference< util::XCloneable > xClone(p.release());

    return xClone;
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2DataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException, std::exception)
{
    // like ScCellRangesBase::addModifyListener
    SolarMutexGuard aGuard;
    if (!m_pTokens.get() || m_pTokens->empty())
        return;

    ScRangeList aRanges;
    ScRefTokenHelper::getRangeListFromTokens(aRanges, *m_pTokens, ScAddress());
    uno::Reference<util::XModifyListener> *pObj =
            new uno::Reference<util::XModifyListener>( aListener );
    m_aValueListeners.push_back( pObj );

    if ( m_aValueListeners.size() == 1 )
    {
        if (!m_pValueListener)
            m_pValueListener = new ScLinkListener( LINK( this, ScChart2DataSequence, ValueListenerHdl ) );

        if (!m_pHiddenListener.get())
            m_pHiddenListener.reset(new HiddenRangeListener(*this));

        if( m_pDocument )
        {
            ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
            vector<ScTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
            for (; itr != itrEnd; ++itr)
            {
                ScRange aRange;
                if (!ScRefTokenHelper::getRangeFromToken(aRange, *itr, ScAddress()))
                    continue;

                m_pDocument->StartListeningArea( aRange, false, m_pValueListener );
                if (pCLC)
                    pCLC->StartListeningHiddenRange(aRange, m_pHiddenListener.get());
            }
        }

        acquire();  // don't lose this object (one ref for all listeners)
    }
}

void SAL_CALL ScChart2DataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException, std::exception)
{
    // like ScCellRangesBase::removeModifyListener

    SolarMutexGuard aGuard;
    if (!m_pTokens.get() || m_pTokens->empty())
        return;

    acquire();      // in case the listeners have the last ref - released below

    sal_uInt16 nCount = m_aValueListeners.size();
    for ( sal_uInt16 n=nCount; n--; )
    {
        uno::Reference<util::XModifyListener>& rObj = m_aValueListeners[n];
        if ( rObj == aListener )
        {
            m_aValueListeners.erase( m_aValueListeners.begin() + n );

            if ( m_aValueListeners.empty() )
            {
                if (m_pValueListener)
                    m_pValueListener->EndListeningAll();

                if (m_pHiddenListener.get() && m_pDocument)
                {
                    ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
                    if (pCLC)
                        pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
                }

                release();      // release the ref for the listeners
            }

            break;
        }
    }

    release();      // might delete this object
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2DataSequence::getPropertySetInfo() throw( uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScChart2DataSequence::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if ( rPropertyName == SC_UNONAME_ROLE )
    {
        if ( !(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
    {
        bool bOldValue = m_bIncludeHiddenCells;
        if ( !(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
        if( bOldValue != m_bIncludeHiddenCells )
            m_aDataArray.clear();//data array is dirty now
    }
    else if( rPropertyName == "TimeBased" )
    {
        bool bTimeBased = mbTimeBased;
        rValue>>= bTimeBased;
        mbTimeBased = bTimeBased;
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
}

uno::Any SAL_CALL ScChart2DataSequence::getPropertyValue(const OUString& rPropertyName)
    throw(beans::UnknownPropertyException,
          lang::WrappedTargetException,
          uno::RuntimeException,
          std::exception)
{
    uno::Any aRet;
    if ( rPropertyName == SC_UNONAME_ROLE )
        aRet <<= m_aRole;
    else if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
        aRet <<= m_bIncludeHiddenCells;
    else if ( rPropertyName == SC_UNONAME_HIDDENVALUES )
    {
        // This property is read-only thus cannot be set externally via
        // setPropertyValue(...).
        BuildDataCache();
        aRet <<= m_aHiddenValues;
    }
    else if (rPropertyName == SC_UNONAME_TIME_BASED)
    {
        aRet <<= mbTimeBased;
    }
    else if (rPropertyName == SC_UNONAME_HAS_STRING_LABEL)
    {
        // Read-only property.  It returns whether or not the label value is a
        // direct user input, rather than an indirect reference.
        bool bHasStringLabel = false;
        if (m_pTokens->size() == 1)
        {
            const formula::FormulaToken& rToken = *(*m_pTokens)[0];
            bHasStringLabel = rToken.GetType() == formula::svString;
        }
        aRet <<= bHasStringLabel;
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
    return aRet;
}

void SAL_CALL ScChart2DataSequence::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataSequence::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataSequence::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataSequence::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void ScChart2DataSequence::setDataChangedHint(bool b)
{
    m_bGotDataChangedHint = b;
}

sal_Bool ScChart2DataSequence::switchToNext(sal_Bool bWrap)
    throw (uno::RuntimeException, std::exception)
{
    if(!m_pTokens || !mbTimeBased)
        return sal_True;

    if(mnCurrentTab >= mnTimeBasedEnd)
    {
        if(bWrap)
            setToPointInTime(0);
        return false;
    }

    for(vector<ScTokenRef>::iterator itr = m_pTokens->begin(),
            itrEnd = m_pTokens->end(); itr != itrEnd; ++itr)
    {
        if ((*itr)->GetType() != svDoubleRef)
            continue;

        ScComplexRefData& rData = *(*itr)->GetDoubleRef();
        ScSingleRefData& s = rData.Ref1;
        ScSingleRefData& e = rData.Ref2;

        s.IncTab(1);
        e.IncTab(1);
    }

    ++mnCurrentTab;

    RebuildDataCache();

    return sal_True;
}

void ScChart2DataSequence::setRange(sal_Int32 nStart, sal_Int32 nEnd)
    throw (uno::RuntimeException, std::exception)
{
    mnTimeBasedStart = nStart;
    mnTimeBasedEnd = nEnd;
    mnCurrentTab = mnTimeBasedStart;
}

sal_Bool ScChart2DataSequence::setToPointInTime(sal_Int32 nPoint)
    throw (uno::RuntimeException, std::exception)
{
    if(!m_pTokens)
        return sal_True;

    if(nPoint > mnTimeBasedEnd - mnTimeBasedStart)
        return false;

    SCTAB nTab = mnTimeBasedStart + nPoint;
    for(vector<ScTokenRef>::iterator itr = m_pTokens->begin(),
            itrEnd = m_pTokens->end(); itr != itrEnd; ++itr)
    {
        if ((*itr)->GetType() != svDoubleRef)
            continue;

        ScComplexRefData& rData = *(*itr)->GetDoubleRef();
        ScSingleRefData& s = rData.Ref1;
        ScSingleRefData& e = rData.Ref2;

        s.SetAbsTab(nTab);
        e.SetAbsTab(nTab);
    }

    mnCurrentTab = nTab;

    RebuildDataCache();

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
