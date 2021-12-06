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

#include <iterator>
#include <memory>
#include <string_view>

#include <inputhdl.hxx>
#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/acorrcfg.hxx>
#include <formula/errorcodes.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/misspellrange.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/charclass.hxx>
#include <vcl/help.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/cursor.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <formula/formulahelper.hxx>
#include <formula/funcvarargs.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <osl/diagnose.h>

#include <attrib.hxx>
#include <inputwin.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <scmod.hxx>
#include <uiitems.hxx>
#include <global.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <patattr.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <editutil.hxx>
#include <appoptio.hxx>
#include <docoptio.hxx>
#include <validat.hxx>
#include <rfindlst.hxx>
#include <inputopt.hxx>
#include <simpleformulacalc.hxx>
#include <compiler.hxx>
#include <editable.hxx>
#include <funcdesc.hxx>
#include <markdata.hxx>
#include <tokenarray.hxx>
#include <gridwin.hxx>
#include <output.hxx>
#include <fillinfo.hxx>

// Maximum Ranges in RangeFinder
#define RANGEFIND_MAX   128

using namespace formula;

namespace {

// Formula data replacement character for a pair of parentheses at end of
// function name, to force sorting parentheses before all other characters.
// Collation may treat parentheses differently.
const sal_Unicode cParenthesesReplacement = 0x0001;

ScTypedCaseStrSet::const_iterator findText(
    const ScTypedCaseStrSet& rDataSet, ScTypedCaseStrSet::const_iterator const & itPos,
    const OUString& rStart, OUString& rResult, bool bBack)
{
    auto lIsMatch = [&rStart](const ScTypedStrData& rData) {
        return (rData.GetStringType() != ScTypedStrData::Value) && ScGlobal::GetTransliteration().isMatch(rStart, rData.GetString()); };

    if (bBack) // Backwards
    {
        ScTypedCaseStrSet::const_reverse_iterator it = rDataSet.rbegin(), itEnd = rDataSet.rend();
        if (itPos != rDataSet.end())
        {
            size_t nPos = std::distance(rDataSet.begin(), itPos);
            size_t nRPos = rDataSet.size() - 1 - nPos;
            std::advance(it, nRPos);
            ++it;
        }

        it = std::find_if(it, itEnd, lIsMatch);
        if (it != itEnd)
        {
            rResult = it->GetString();
            return (++it).base(); // convert the reverse iterator back to iterator.
        }
    }
    else // Forwards
    {
        ScTypedCaseStrSet::const_iterator it = rDataSet.begin(), itEnd = rDataSet.end();
        if (itPos != itEnd)
        {
            it = std::next(itPos);
        }

        it = std::find_if(it, itEnd, lIsMatch);
        if (it != itEnd)
        {
            rResult = it->GetString();
            return it;
        }
    }

    return rDataSet.end(); // no matching text found
}

OUString getExactMatch(const ScTypedCaseStrSet& rDataSet, const OUString& rString)
{
    auto it = std::find_if(rDataSet.begin(), rDataSet.end(),
        [&rString](const ScTypedStrData& rData) {
            return (rData.GetStringType() != ScTypedStrData::Value)
                && ScGlobal::GetTransliteration().isEqual(rData.GetString(), rString);
        });
    if (it != rDataSet.end())
        return it->GetString();
    return rString;
}

// This assumes that rResults is a sorted ring w.r.t ScTypedStrData::LessCaseInsensitive() or
// in the reverse direction, whose origin is specified by nRingOrigin.
sal_Int32 getLongestCommonPrefixLength(const std::vector<OUString>& rResults, const OUString& rUserEntry, sal_Int32 nRingOrigin)
{
    sal_Int32 nResults = rResults.size();
    if (!nResults)
        return 0;

    if (nResults == 1)
        return rResults[0].getLength();

    sal_Int32 nMinLen = rUserEntry.getLength();
    sal_Int32 nLastIdx = nRingOrigin ? nRingOrigin - 1 : nResults - 1;
    const OUString& rFirst = rResults[nRingOrigin];
    const OUString& rLast = rResults[nLastIdx];
    const sal_Int32 nMaxLen = std::min(rFirst.getLength(), rLast.getLength());

    for (sal_Int32 nLen = nMaxLen; nLen > nMinLen; --nLen)
    {
        if (ScGlobal::GetTransliteration().isMatch(rFirst.copy(0, nLen), rLast))
            return nLen;
    }

    return nMinLen;
}

ScTypedCaseStrSet::const_iterator findTextAll(
    const ScTypedCaseStrSet& rDataSet, ScTypedCaseStrSet::const_iterator const & itPos,
    const OUString& rStart, ::std::vector< OUString > &rResultVec, bool bBack, sal_Int32* pLongestPrefixLen = nullptr)
{
    rResultVec.clear(); // clear contents

    if (!rDataSet.size())
        return rDataSet.end();

    sal_Int32 nRingOrigin = 0;
    size_t nCount = 0;
    ScTypedCaseStrSet::const_iterator retit;
    if ( bBack ) // Backwards
    {
        ScTypedCaseStrSet::const_reverse_iterator it, itEnd;
        if ( itPos == rDataSet.end() )
        {
            it = rDataSet.rend();
            --it;
            itEnd = it;
        }
        else
        {
            it = rDataSet.rbegin();
            size_t nPos = std::distance(rDataSet.begin(), itPos);
            size_t nRPos = rDataSet.size() - 1 - nPos; // if itPos == rDataSet.end(), then nRPos = -1
            std::advance(it, nRPos);
            if ( it == rDataSet.rend() )
                it = rDataSet.rbegin();
            itEnd = it;
        }
        bool bFirstTime = true;

        while ( it != itEnd || bFirstTime )
        {
            ++it;
            if ( it == rDataSet.rend() ) // go to the first if reach the end
            {
                it = rDataSet.rbegin();
                nRingOrigin = nCount;
            }

            if ( bFirstTime )
                bFirstTime = false;
            const ScTypedStrData& rData = *it;
            if ( rData.GetStringType() == ScTypedStrData::Value )
                // skip values
                continue;

            if ( !ScGlobal::GetTransliteration().isMatch(rStart, rData.GetString()) )
                // not a match
                continue;

            rResultVec.push_back(rData.GetString()); // set the match data
            if ( nCount == 0 ) // convert the reverse iterator back to iterator.
            {
                // actually we want to do "retit = it;".
                retit = rDataSet.begin();
                size_t nRPos = std::distance(rDataSet.rbegin(), it);
                size_t nPos = rDataSet.size() - 1 - nRPos;
                std::advance(retit, nPos);
            }
            ++nCount;
        }
    }
    else // Forwards
    {
        ScTypedCaseStrSet::const_iterator it, itEnd;
        it = itPos;
        if ( it == rDataSet.end() )
            it = rDataSet.begin();
        itEnd = it;
        bool bFirstTime = true;

        while ( it != itEnd || bFirstTime )
        {
            ++it;
            if ( it == rDataSet.end() ) // go to the first if reach the end
            {
                it = rDataSet.begin();
                nRingOrigin = nCount;
            }

            if ( bFirstTime )
                bFirstTime = false;
            const ScTypedStrData& rData = *it;
            if ( rData.GetStringType() == ScTypedStrData::Value )
                // skip values
                continue;

            if ( !ScGlobal::GetTransliteration().isMatch(rStart, rData.GetString()) )
                // not a match
                continue;

            rResultVec.push_back(rData.GetString()); // set the match data
            if ( nCount == 0 )
                retit = it; // remember first match iterator
            ++nCount;
        }
    }

    if (pLongestPrefixLen)
    {
        if (nRingOrigin >= static_cast<sal_Int32>(nCount))
        {
            // All matches were picked when rDataSet was read in one direction.
            nRingOrigin = 0;
        }
        // rResultsVec is a sorted ring with nRingOrigin "origin".
        // The direction of sorting is not important for getLongestCommonPrefixLength.
        *pLongestPrefixLen = getLongestCommonPrefixLength(rResultVec, rStart, nRingOrigin);
    }

    if ( nCount > 0 ) // at least one function has matched
        return retit;
    return rDataSet.end(); // no matching text found
}

}

void ScInputHandler::SendReferenceMarks( const SfxViewShell* pViewShell,
                            const std::vector<ReferenceMark>& rReferenceMarks )
{
    if ( !pViewShell )
        return;

    bool bSend = false;

    std::stringstream ss;

    ss << "{ \"marks\": [ ";

    for ( size_t i = 0; i < rReferenceMarks.size(); i++ )
    {
        if ( rReferenceMarks[i].Is() )
        {
            if ( bSend )
                ss << ", ";

            ss << "{ \"rectangle\": \""
               << rReferenceMarks[i].nX << ", "
               << rReferenceMarks[i].nY << ", "
               << rReferenceMarks[i].nWidth << ", "
               << rReferenceMarks[i].nHeight << "\", "
                  "\"color\": \"" << rReferenceMarks[i].aColor.AsRGBHexString() << "\", "
                  "\"part\": \"" << rReferenceMarks[i].nTab << "\" } ";

            bSend = true;
        }
    }

    ss <<  " ] }";

    OString aPayload = ss.str().c_str();
    pViewShell->libreOfficeKitViewCallback(
                LOK_CALLBACK_REFERENCE_MARKS, aPayload.getStr() );
}

void ScInputHandler::InitRangeFinder( const OUString& rFormula )
{
    DeleteRangeFinder();
    if ( !pActiveViewSh || !SC_MOD()->GetInputOptions().GetRangeFinder() )
        return;
    ScDocShell* pDocSh = pActiveViewSh->GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    const sal_Unicode cSheetSep = rDoc.GetSheetSeparator();

    OUString aDelimiters = ScEditUtil::ModifyDelimiters(" !~\"");
        // delimiters (in addition to ScEditUtil): only characters that are
        // allowed in formulas next to references and the quotation mark (so
        // string constants can be skipped)

    sal_Int32 nColon = aDelimiters.indexOf( ':' );
    if ( nColon != -1 )
        aDelimiters = aDelimiters.replaceAt( nColon, 1, u""); // Delimiter without colon
    sal_Int32 nDot = aDelimiters.indexOf(cSheetSep);
    if ( nDot != -1 )
        aDelimiters = aDelimiters.replaceAt( nDot, 1 , u""); // Delimiter without dot

    const sal_Unicode* pChar = rFormula.getStr();
    sal_Int32 nLen = rFormula.getLength();
    sal_Int32 nPos = 0;
    sal_Int32 nStart = 0;
    sal_uInt16 nCount = 0;
    ScRange aRange;
    while ( nPos < nLen && nCount < RANGEFIND_MAX )
    {
        // Skip separator
        while ( nPos<nLen && ScGlobal::UnicodeStrChr( aDelimiters.getStr(), pChar[nPos] ) )
        {
            if ( pChar[nPos] == '"' )                       // String
            {
                ++nPos;
                while (nPos<nLen && pChar[nPos] != '"')     // Skip until end
                    ++nPos;
            }
            ++nPos; // Separator or closing quote
        }

        //  text between separators
        nStart = nPos;
handle_r1c1:
        {
            bool bSingleQuoted = false;
            while (nPos < nLen)
            {
                // tdf#114113: handle addresses with quoted sheet names like "'Sheet 1'.A1"
                // Literal single quotes in sheet names are masked by another single quote
                if (pChar[nPos] == '\'')
                {
                    bSingleQuoted = !bSingleQuoted;
                }
                else if (!bSingleQuoted) // Get everything in single quotes, including separators
                {
                    if (ScGlobal::UnicodeStrChr(aDelimiters.getStr(), pChar[nPos]))
                        break;
                }
                ++nPos;
            }
        }

        // for R1C1 '-' in R[-]... or C[-]... are not delimiters
        // Nothing heroic here to ensure that there are '[]' around a negative
        // integer.  we need to clean up this code.
        if( nPos < nLen && nPos > 0 &&
            '-' == pChar[nPos] && '[' == pChar[nPos-1] &&
            formula::FormulaGrammar::CONV_XL_R1C1 == rDoc.GetAddressConvention() )
        {
            nPos++;
            goto handle_r1c1;
        }

        if ( nPos > nStart )
        {
            OUString aTest = rFormula.copy( nStart, nPos-nStart );
            const ScAddress::Details aAddrDetails( rDoc, aCursorPos );
            ScRefFlags nFlags = aRange.ParseAny( aTest, rDoc, aAddrDetails );
            if ( nFlags & ScRefFlags::VALID )
            {
                //  Set tables if not specified
                if ( (nFlags & ScRefFlags::TAB_3D) == ScRefFlags::ZERO)
                    aRange.aStart.SetTab( pActiveViewSh->GetViewData().GetTabNo() );
                if ( (nFlags & ScRefFlags::TAB2_3D) == ScRefFlags::ZERO)
                    aRange.aEnd.SetTab( aRange.aStart.Tab() );

                if ( ( nFlags & (ScRefFlags::COL2_VALID|ScRefFlags::ROW2_VALID|ScRefFlags::TAB2_VALID) ) ==
                                 ScRefFlags::ZERO )
                {
                    // #i73766# if a single ref was parsed, set the same "abs" flags for ref2,
                    // so Format doesn't output a double ref because of different flags.
                    ScRefFlags nAbsFlags = nFlags & (ScRefFlags::COL_ABS|ScRefFlags::ROW_ABS|ScRefFlags::TAB_ABS);
                    applyStartToEndFlags(nFlags, nAbsFlags);
                }

                if (!nCount)
                {
                    mpEditEngine->SetUpdateLayout( false );
                    pRangeFindList.reset(new ScRangeFindList( pDocSh->GetTitle() ));
                }

                Color nColor = pRangeFindList->Insert( ScRangeFindData( aRange, nFlags, nStart, nPos ) );

                ESelection aSel( 0, nStart, 0, nPos );
                SfxItemSet aSet( mpEditEngine->GetEmptyItemSet() );
                aSet.Put( SvxColorItem( nColor, EE_CHAR_COLOR ) );
                mpEditEngine->QuickSetAttribs( aSet, aSel );
                ++nCount;
            }
        }

        // Do not skip last separator; could be a quote (?)
    }

    UpdateLokReferenceMarks();

    if (nCount)
    {
        mpEditEngine->SetUpdateLayout( true );

        pDocSh->Broadcast( SfxHint( SfxHintId::ScShowRangeFinder ) );
    }
}

ReferenceMark ScInputHandler::GetReferenceMark( const ScViewData& rViewData, ScDocShell* pDocSh,
                                    tools::Long nX1, tools::Long nX2, tools::Long nY1, tools::Long nY2,
                                    tools::Long nTab, const Color& rColor )
{
    ScSplitPos eWhich = rViewData.GetActivePart();

    // This method is LOK specific.
    if (comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
    {
        SCCOL nCol1 = nX1, nCol2 = nX2;
        SCROW nRow1 = nY1, nRow2 = nY2;
        PutInOrder(nCol1, nCol2);
        PutInOrder(nRow1, nRow2);
        if (nCol1 == nCol2 && nRow1 == nRow2)
            pDocSh->GetDocument().ExtendMerge(nCol1, nRow1, nCol2, nRow2, nTab);

        Point aTopLeft = rViewData.GetPrintTwipsPos(nCol1, nRow1);
        Point aBottomRight = rViewData.GetPrintTwipsPos(nCol2 + 1, nRow2 + 1);
        tools::Long nSizeX = aBottomRight.X() - aTopLeft.X() - 1;
        tools::Long nSizeY = aBottomRight.Y() - aTopLeft.Y() - 1;

        return ReferenceMark(aTopLeft.X(), aTopLeft.Y(), nSizeX, nSizeY, nTab, rColor);
    }

    Point aScrPos = rViewData.GetScrPos( nX1, nY1, eWhich );
    tools::Long nScrX = aScrPos.X();
    tools::Long nScrY = aScrPos.Y();

    double nPPTX = rViewData.GetPPTX();
    double nPPTY = rViewData.GetPPTY();

    Fraction aZoomX = rViewData.GetZoomX();
    Fraction aZoomY = rViewData.GetZoomY();

    ScTableInfo aTabInfo;
    pDocSh->GetDocument().FillInfo( aTabInfo, nX1, nY1, nX2, nY2,
                                    nTab, nPPTX, nPPTY, false, false );

    ScOutputData aOutputData( nullptr, OUTTYPE_WINDOW, aTabInfo,
                              &( pDocSh->GetDocument() ), nTab,
                              nScrX, nScrY,
                              nX1, nY1, nX2, nY2,
                              nPPTX, nPPTY,
                              &aZoomX, &aZoomY );

    return aOutputData.FillReferenceMark( nX1, nY1, nX2, nY2,
                                          rColor );
}

void ScInputHandler::UpdateLokReferenceMarks()
{
    if ( !comphelper::LibreOfficeKit::isActive())
        return;

    ScTabViewShell* pShell = pActiveViewSh ? pActiveViewSh
                                : dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());

    if (!pShell)
        return;

    ScViewData& rViewData = pShell->GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScRangeFindList* pRangeFinder = GetRangeFindList();

    if ( !pRangeFinder && !rViewData.IsRefMode() )
        return;

    sal_uInt16 nAdditionalMarks = 0;
    std::vector<ReferenceMark> aReferenceMarks( 1 );

    if ( rViewData.IsRefMode() )
    {
        nAdditionalMarks = 1;

        const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
        Color aRefColor( rColorCfg.GetColorValue( svtools::CALCREFERENCE ).nColor );
        tools::Long nX1 = rViewData.GetRefStartX();
        tools::Long nX2 = rViewData.GetRefEndX();
        tools::Long nY1 = rViewData.GetRefStartY();
        tools::Long nY2 = rViewData.GetRefEndY();
        tools::Long nTab = rViewData.GetRefStartZ();

        if (rViewData.GetRefEndZ() == rViewData.GetTabNo())
            nTab = rViewData.GetRefEndZ();

        PutInOrder(nX1, nX2);
        PutInOrder(nY1, nY2);

        aReferenceMarks[0] = ScInputHandler::GetReferenceMark( rViewData, pDocSh,
                                                   nX1, nX2, nY1, nY2,
                                                   nTab, aRefColor );
    }

    sal_uInt16 nCount = pRangeFinder ?
        ( static_cast<sal_uInt16>( pRangeFinder->Count() ) + nAdditionalMarks ) : nAdditionalMarks;
    aReferenceMarks.resize( nCount );

    if ( nCount && pRangeFinder && !pRangeFinder->IsHidden() &&
         pRangeFinder->GetDocName() == pDocSh->GetTitle() )
    {
        for (sal_uInt16 i = 0; i < nCount - nAdditionalMarks; i++)
        {
            ScRangeFindData& rData = pRangeFinder->GetObject( i );
            ScRange aRef = rData.aRef;
            aRef.PutInOrder();

            tools::Long nX1 = aRef.aStart.Col();
            tools::Long nX2 = aRef.aEnd.Col();
            tools::Long nY1 = aRef.aStart.Row();
            tools::Long nY2 = aRef.aEnd.Row();
            tools::Long nTab = aRef.aStart.Tab();

            aReferenceMarks[i + nAdditionalMarks] = ScInputHandler::GetReferenceMark( rViewData, pDocSh,
                                                                          nX1, nX2, nY1, nY2,
                                                                          nTab, rData.nColor );

            ScInputHandler::SendReferenceMarks( pShell, aReferenceMarks );
        }
    }
    else if ( nCount )
    {
        ScInputHandler::SendReferenceMarks( pShell, aReferenceMarks );
    }
    else
    {
        // Clear
        aReferenceMarks.clear();
        ScInputHandler::SendReferenceMarks( pShell, aReferenceMarks );
    }
}

void ScInputHandler::SetDocumentDisposing( bool b )
{
    mbDocumentDisposing = b;
}

static void lcl_Replace( EditView* pView, const OUString& rNewStr, const ESelection& rOldSel )
{
    if ( !pView )
        return;

    ESelection aOldSel = pView->GetSelection();
    if (aOldSel.HasRange())
        pView->SetSelection( ESelection( aOldSel.nEndPara, aOldSel.nEndPos,
                                         aOldSel.nEndPara, aOldSel.nEndPos ) );

    EditEngine* pEngine = pView->GetEditEngine();
    pEngine->QuickInsertText( rNewStr, rOldSel );

    // Dummy InsertText for Update and Paint
    // To do that we need to cancel the selection from above (before QuickInsertText)
    pView->InsertText( OUString() );

    sal_Int32 nLen = pEngine->GetTextLen(0);
    ESelection aSel( 0, nLen, 0, nLen );
    pView->SetSelection( aSel ); // Set cursor to the end
}

void ScInputHandler::UpdateRange( sal_uInt16 nIndex, const ScRange& rNew )
{
    ScTabViewShell* pDocView = pRefViewSh ? pRefViewSh : pActiveViewSh;
    if ( pDocView && pRangeFindList && nIndex < pRangeFindList->Count() )
    {
        ScRangeFindData& rData = pRangeFindList->GetObject( nIndex );
        sal_Int32 nOldStart = rData.nSelStart;
        sal_Int32 nOldEnd = rData.nSelEnd;
        Color nNewColor = pRangeFindList->FindColor( rNew, nIndex );

        ScRange aJustified = rNew;
        aJustified.PutInOrder(); // Always display Ref in the Formula the right way
        ScDocument& rDoc = pDocView->GetViewData().GetDocument();
        const ScAddress::Details aAddrDetails( rDoc, aCursorPos );
        OUString aNewStr(aJustified.Format(rDoc, rData.nFlags, aAddrDetails));
        ESelection aOldSel( 0, nOldStart, 0, nOldEnd );
        SfxItemSet aSet( mpEditEngine->GetEmptyItemSet() );

        DataChanging();

        lcl_Replace( pTopView, aNewStr, aOldSel );
        lcl_Replace( pTableView, aNewStr, aOldSel );
        aSet.Put( SvxColorItem( nNewColor, EE_CHAR_COLOR ) );
        mpEditEngine->QuickSetAttribs( aSet, aOldSel );

        bInRangeUpdate = true;
        DataChanged();
        bInRangeUpdate = false;

        tools::Long nDiff = aNewStr.getLength() - static_cast<tools::Long>(nOldEnd-nOldStart);

        rData.aRef = rNew;
        rData.nSelEnd = rData.nSelEnd + nDiff;
        rData.nColor = nNewColor;

        sal_uInt16 nCount = static_cast<sal_uInt16>(pRangeFindList->Count());
        for (sal_uInt16 i=nIndex+1; i<nCount; i++)
        {
            ScRangeFindData& rNext = pRangeFindList->GetObject( i );
            rNext.nSelStart = rNext.nSelStart + nDiff;
            rNext.nSelEnd   = rNext.nSelEnd   + nDiff;
        }

        EditView* pActiveView = pTopView ? pTopView : pTableView;
        pActiveView->ShowCursor( false );
    }
    else
    {
        OSL_FAIL("UpdateRange: we're missing something");
    }
}

void ScInputHandler::DeleteRangeFinder()
{
    ScTabViewShell* pPaintView = pRefViewSh ? pRefViewSh : pActiveViewSh;
    if ( pRangeFindList && pPaintView )
    {
        ScDocShell* pDocSh = pActiveViewSh->GetViewData().GetDocShell();
        pRangeFindList->SetHidden(true);
        pDocSh->Broadcast( SfxHint( SfxHintId::ScShowRangeFinder ) );  // Steal
        pRangeFindList.reset();
    }
}

static OUString GetEditText(const EditEngine* pEng)
{
    return ScEditUtil::GetMultilineString(*pEng);
}

static void lcl_RemoveTabs(OUString& rStr)
{
    rStr = rStr.replace('\t', ' ');
}

static void lcl_RemoveLineEnd(OUString& rStr)
{
    rStr = convertLineEnd(rStr, LINEEND_LF);
    rStr = rStr.replace('\n', ' ');
}

static sal_Int32 lcl_MatchParenthesis( const OUString& rStr, sal_Int32 nPos )
{
    int nDir;
    sal_Unicode c1, c2 = 0;
    c1 = rStr[nPos];
    switch ( c1 )
    {
    case '(' :
        c2 = ')';
        nDir = 1;
        break;
    case ')' :
        c2 = '(';
        nDir = -1;
        break;
    case '<' :
        c2 = '>';
        nDir = 1;
        break;
    case '>' :
        c2 = '<';
        nDir = -1;
        break;
    case '{' :
        c2 = '}';
        nDir = 1;
        break;
    case '}' :
        c2 = '{';
        nDir = -1;
        break;
    case '[' :
        c2 = ']';
        nDir = 1;
        break;
    case ']' :
        c2 = '[';
        nDir = -1;
        break;
    default:
        nDir = 0;
    }
    if ( !nDir )
        return -1;
    sal_Int32 nLen = rStr.getLength();
    const sal_Unicode* p0 = rStr.getStr();
    const sal_Unicode* p;
    const sal_Unicode* p1;
    sal_uInt16 nQuotes = 0;
    if ( nPos < nLen / 2 )
    {
        p = p0;
        p1 = p0 + nPos;
    }
    else
    {
        p = p0 + nPos;
        p1 = p0 + nLen;
    }
    while ( p < p1 )
    {
        if ( *p++ == '\"' )
            nQuotes++;
    }
    // Odd number of quotes that we find ourselves in a string
    bool bLookInString = ((nQuotes % 2) != 0);
    bool bInString = bLookInString;
    p = p0 + nPos;
    p1 = (nDir < 0 ? p0 : p0 + nLen) ;
    sal_uInt16 nLevel = 1;
    while ( p != p1 && nLevel )
    {
        p += nDir;
        if ( *p == '\"' )
        {
            bInString = !bInString;
            if ( bLookInString && !bInString )
                p = p1; // That's it then
        }
        else if ( bInString == bLookInString )
        {
            if ( *p == c1 )
                nLevel++;
            else if ( *p == c2 )
                nLevel--;
        }
    }
    if ( nLevel )
        return -1;
    return static_cast<sal_Int32>(p - p0);
}

ScInputHandler::ScInputHandler()
    :   pInputWin( nullptr ),
        pTableView( nullptr ),
        pTopView( nullptr ),
        pTipVisibleParent( nullptr ),
        nTipVisible( nullptr ),
        pTipVisibleSecParent( nullptr ),
        nTipVisibleSec( nullptr ),
        nFormSelStart( 0 ),
        nFormSelEnd( 0 ),
        nCellPercentFormatDecSep( 0 ),
        nAutoPar( 0 ),
        eMode( SC_INPUT_NONE ),
        bUseTab( false ),
        bTextValid( true ),
        bModified( false ),
        bSelIsRef( false ),
        bFormulaMode( false ),
        bInRangeUpdate( false ),
        bParenthesisShown( false ),
        bCreatingFuncView( false ),
        bInEnterHandler( false ),
        bCommandErrorShown( false ),
        bInOwnChange( false ),
        bProtected( false ),
        bLastIsSymbol( false ),
        mbDocumentDisposing(false),
        mbPartialPrefix(false),
        nValidation( 0 ),
        eAttrAdjust( SvxCellHorJustify::Standard ),
        aScaleX( 1,1 ),
        aScaleY( 1,1 ),
        pRefViewSh( nullptr ),
        pLastPattern( nullptr )
{
    //  The InputHandler is constructed with the view, so SfxViewShell::Current
    //  doesn't have the right view yet. pActiveViewSh is updated in NotifyChange.
    pActiveViewSh = nullptr;

    //  Bindings (only still used for Invalidate) are retrieved if needed on demand

    pDelayTimer.reset( new Timer( "ScInputHandlerDelay timer" ) );
    pDelayTimer->SetTimeout( 500 ); // 500 ms delay
    pDelayTimer->SetInvokeHandler( LINK( this, ScInputHandler, DelayTimer ) );
}

ScInputHandler::~ScInputHandler()
{
    //  If this is the application InputHandler, the dtor is called after SfxApplication::Main,
    //  thus we can't rely on any Sfx functions
    if (!mbDocumentDisposing) // inplace
        EnterHandler(); // Finish input

    if (SC_MOD()->GetRefInputHdl() == this)
        SC_MOD()->SetRefInputHdl(nullptr);

    if ( pInputWin && pInputWin->GetInputHandler() == this )
        pInputWin->SetInputHandler( nullptr );
}

void ScInputHandler::SetRefScale( const Fraction& rX, const Fraction& rY )
{
    if ( rX != aScaleX || rY != aScaleY )
    {
        aScaleX = rX;
        aScaleY = rY;
        if (mpEditEngine)
        {
            MapMode aMode( MapUnit::Map100thMM, Point(), aScaleX, aScaleY );
            mpEditEngine->SetRefMapMode( aMode );
        }
    }
}

void ScInputHandler::UpdateRefDevice()
{
    if (!mpEditEngine)
        return;

    bool bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
    bool bInPlace = pActiveViewSh && pActiveViewSh->GetViewFrame()->GetFrame().IsInPlace();
    EEControlBits nCtrl = mpEditEngine->GetControlWord();
    if ( bTextWysiwyg || bInPlace )
        nCtrl |= EEControlBits::FORMAT100;    // EditEngine default: always format for 100%
    else
        nCtrl &= ~EEControlBits::FORMAT100;   // when formatting for screen, use the actual MapMode
    mpEditEngine->SetControlWord( nCtrl );
    if ( bTextWysiwyg && pActiveViewSh )
        mpEditEngine->SetRefDevice( pActiveViewSh->GetViewData().GetDocument().GetPrinter() );
    else
        mpEditEngine->SetRefDevice( nullptr );

    MapMode aMode( MapUnit::Map100thMM, Point(), aScaleX, aScaleY );
    mpEditEngine->SetRefMapMode( aMode );

    //  SetRefDevice(NULL) uses VirtualDevice, SetRefMapMode forces creation of a local VDev,
    //  so the DigitLanguage can be safely modified (might use an own VDev instead of NULL).
    if ( !( bTextWysiwyg && pActiveViewSh ) )
    {
        mpEditEngine->GetRefDevice()->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
    }
}

void ScInputHandler::ImplCreateEditEngine()
{
    if ( mpEditEngine )
        return;

    if ( pActiveViewSh )
    {
        ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocShell()->GetDocument();
        mpEditEngine = std::make_unique<ScFieldEditEngine>(&rDoc, rDoc.GetEnginePool(), rDoc.GetEditPool());
    }
    else
        mpEditEngine = std::make_unique<ScFieldEditEngine>(nullptr, EditEngine::CreatePool().get(), nullptr, true);

    mpEditEngine->SetWordDelimiters( ScEditUtil::ModifyDelimiters( mpEditEngine->GetWordDelimiters() ) );
    UpdateRefDevice();      // also sets MapMode
    mpEditEngine->SetPaperSize( Size( 1000000, 1000000 ) );
    pEditDefaults.reset( new SfxItemSet( mpEditEngine->GetEmptyItemSet() ) );

    mpEditEngine->SetControlWord( mpEditEngine->GetControlWord() | EEControlBits::AUTOCORRECT );
    mpEditEngine->SetReplaceLeadingSingleQuotationMark( false );
    mpEditEngine->SetModifyHdl( LINK( this, ScInputHandler, ModifyHdl ) );
}

void ScInputHandler::UpdateAutoCorrFlag()
{
    EEControlBits nCntrl = mpEditEngine->GetControlWord();
    EEControlBits nOld = nCntrl;

    // Don't use pLastPattern here (may be invalid because of AutoStyle)
    bool bDisable = bLastIsSymbol || bFormulaMode;
    if ( bDisable )
        nCntrl &= ~EEControlBits::AUTOCORRECT;
    else
        nCntrl |= EEControlBits::AUTOCORRECT;

    if ( nCntrl != nOld )
        mpEditEngine->SetControlWord(nCntrl);
}

void ScInputHandler::UpdateSpellSettings( bool bFromStartTab )
{
    if ( !pActiveViewSh )
        return;

    ScViewData& rViewData = pActiveViewSh->GetViewData();
    bool bOnlineSpell = rViewData.GetDocument().GetDocOptions().IsAutoSpell();

    //  SetDefaultLanguage is independent of the language attributes,
    //  ScGlobal::GetEditDefaultLanguage is always used.
    //  It must be set every time in case the office language was changed.

    mpEditEngine->SetDefaultLanguage( ScGlobal::GetEditDefaultLanguage() );

    //  if called for changed options, update flags only if already editing
    //  if called from StartTable, always update flags

    if ( bFromStartTab || eMode != SC_INPUT_NONE )
    {
        EEControlBits nCntrl = mpEditEngine->GetControlWord();
        EEControlBits nOld = nCntrl;
        if( bOnlineSpell )
            nCntrl |= EEControlBits::ONLINESPELLING;
        else
            nCntrl &= ~EEControlBits::ONLINESPELLING;
        // No AutoCorrect for Symbol Font (EditEngine does no evaluate Default)
        if ( pLastPattern && pLastPattern->IsSymbolFont() )
            nCntrl &= ~EEControlBits::AUTOCORRECT;
        else
            nCntrl |= EEControlBits::AUTOCORRECT;
        if ( nCntrl != nOld )
            mpEditEngine->SetControlWord(nCntrl);

        ScDocument& rDoc = rViewData.GetDocument();
        rDoc.ApplyAsianEditSettings( *mpEditEngine );
        mpEditEngine->SetDefaultHorizontalTextDirection(
            rDoc.GetEditTextDirection( rViewData.GetTabNo() ) );
        mpEditEngine->SetFirstWordCapitalization( false );
    }

    //  Language is set separately, so the speller is needed only if online spelling is active
    if ( bOnlineSpell ) {
        css::uno::Reference<css::linguistic2::XSpellChecker1> xXSpellChecker1( LinguMgr::GetSpellChecker() );
        mpEditEngine->SetSpeller( xXSpellChecker1 );
    }

    bool bHyphen = pLastPattern && pLastPattern->GetItem(ATTR_HYPHENATE).GetValue();
    if ( bHyphen ) {
        css::uno::Reference<css::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
        mpEditEngine->SetHyphenator( xXHyphenator );
    }
}

// Function/Range names etc. as Tip help

//  The other types are defined in ScDocument::GetFormulaEntries
void ScInputHandler::GetFormulaData()
{
    if ( !pActiveViewSh )
        return;

    ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocShell()->GetDocument();

    if ( pFormulaData )
        pFormulaData->clear();
    else
    {
        pFormulaData.reset( new ScTypedCaseStrSet );
    }

    if( pFormulaDataPara )
        pFormulaDataPara->clear();
    else
        pFormulaDataPara.reset( new ScTypedCaseStrSet );

    const OUString aParenthesesReplacement( cParenthesesReplacement);
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    sal_uInt32 nListCount = pFuncList->GetCount();
    for(sal_uInt32 i=0;i<nListCount;i++)
    {
        const ScFuncDesc* pDesc = pFuncList->GetFunction( i );
        if ( pDesc->mxFuncName )
        {
            const sal_Unicode* pName = pDesc->mxFuncName->getStr();
            const sal_Int32 nLen = pDesc->mxFuncName->getLength();
            // fdo#75264 fill maFormulaChar with all characters used in formula names
            for ( sal_Int32 j = 0; j < nLen; j++ )
            {
                sal_Unicode c = pName[ j ];
                maFormulaChar.insert( c );
            }
            OUString aFuncName = *pDesc->mxFuncName + aParenthesesReplacement;
            pFormulaData->insert(ScTypedStrData(aFuncName, 0.0, 0.0, ScTypedStrData::Standard));
            pDesc->initArgumentInfo();
            OUString aEntry = pDesc->getSignature();
            pFormulaDataPara->insert(ScTypedStrData(aEntry, 0.0, 0.0, ScTypedStrData::Standard));
        }
    }
    miAutoPosFormula = pFormulaData->end();
    rDoc.GetFormulaEntries( *pFormulaData );
    rDoc.GetFormulaEntries( *pFormulaDataPara );
}

IMPL_LINK( ScInputHandler, ShowHideTipVisibleParentListener, VclWindowEvent&, rEvent, void )
{
    if (rEvent.GetId() == VclEventId::ObjectDying || rEvent.GetId() == VclEventId::WindowHide
        || rEvent.GetId() == VclEventId::WindowLoseFocus)
        HideTip();
}

IMPL_LINK( ScInputHandler, ShowHideTipVisibleSecParentListener, VclWindowEvent&, rEvent, void )
{
    if (rEvent.GetId() == VclEventId::ObjectDying || rEvent.GetId() == VclEventId::WindowHide
        || rEvent.GetId() == VclEventId::WindowLoseFocus)
        HideTipBelow();
}

void ScInputHandler::HideTip()
{
    if ( nTipVisible )
    {
        pTipVisibleParent->RemoveEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleParentListener ) );
        Help::HidePopover(pTipVisibleParent, nTipVisible );
        nTipVisible = nullptr;
        pTipVisibleParent = nullptr;
    }
    aManualTip.clear();
}
void ScInputHandler::HideTipBelow()
{
    if ( nTipVisibleSec )
    {
        pTipVisibleSecParent->RemoveEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleSecParentListener ) );
        Help::HidePopover(pTipVisibleSecParent, nTipVisibleSec);
        nTipVisibleSec = nullptr;
        pTipVisibleSecParent = nullptr;
    }
    aManualTip.clear();
}

namespace
{

bool lcl_hasSingleToken(const OUString& s, sal_Unicode c)
{
    return !s.isEmpty() && s.indexOf(c)<0;
}

}

void ScInputHandler::ShowArgumentsTip( OUString& rSelText )
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        return;
    }

    ScDocShell* pDocSh = pActiveViewSh->GetViewData().GetDocShell();
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    const sal_Unicode cSheetSep = pDocSh->GetDocument().GetSheetSeparator();
    FormulaHelper aHelper(ScGlobal::GetStarCalcFunctionMgr());
    bool bFound = false;
    while( !bFound )
    {
        rSelText += ")";
        sal_Int32 nLeftParentPos = lcl_MatchParenthesis( rSelText, rSelText.getLength()-1 );
        if( nLeftParentPos != -1 )
        {
            sal_Int32 nNextFStart = aHelper.GetFunctionStart( rSelText, nLeftParentPos, true);
            const IFunctionDescription* ppFDesc;
            ::std::vector< OUString> aArgs;
            if( aHelper.GetNextFunc( rSelText, false, nNextFStart, nullptr, &ppFDesc, &aArgs ) )
            {
                if( !ppFDesc->getFunctionName().isEmpty() )
                {
                    sal_Int32 nArgPos = aHelper.GetArgStart( rSelText, nNextFStart, 0 );
                    sal_uInt16 nArgs = static_cast<sal_uInt16>(ppFDesc->getParameterCount());
                    OUString aFuncName( ppFDesc->getFunctionName() + "(");
                    OUString aNew;
                    ScTypedCaseStrSet::const_iterator it =
                        findText(*pFormulaDataPara, pFormulaDataPara->end(), aFuncName, aNew, false);
                    if (it != pFormulaDataPara->end())
                    {
                        bool bFlag = false;
                        sal_uInt16 nActive = 0;
                        for( sal_uInt16 i=0; i < nArgs; i++ )
                        {
                            sal_Int32 nLength = aArgs[i].getLength();
                            if( nArgPos <= rSelText.getLength()-1 )
                            {
                                nActive = i+1;
                                bFlag = true;
                            }
                            nArgPos+=nLength+1;
                        }
                        if( bFlag )
                        {
                            sal_Int32 nStartPosition = 0;
                            sal_Int32 nEndPosition = 0;

                            if( lcl_hasSingleToken(aNew, cSep) )
                            {
                                for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                {
                                    sal_Unicode cNext = aNew[i];
                                    if( cNext == '(' )
                                    {
                                        nStartPosition = i+1;
                                    }
                                }
                            }
                            else if( lcl_hasSingleToken(aNew, cSheetSep) )
                            {
                                sal_uInt16 nCount = 0;
                                for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                {
                                    sal_Unicode cNext = aNew[i];
                                    if( cNext == '(' )
                                    {
                                        nStartPosition = i+1;
                                    }
                                    else if( cNext == cSep )
                                    {
                                        nCount ++;
                                        nEndPosition = i;
                                        if( nCount == nActive )
                                        {
                                            break;
                                        }
                                        nStartPosition = nEndPosition+1;
                                    }
                                }
                            }
                            else
                            {
                                sal_uInt16 nCount = 0;
                                for (sal_Int32 i = 0; i < aNew.getLength(); ++i)
                                {
                                    sal_Unicode cNext = aNew[i];
                                    if( cNext == '(' )
                                    {
                                        nStartPosition = i+1;
                                    }
                                    else if( cNext == cSep )
                                    {
                                        nCount ++;
                                        nEndPosition = i;
                                        if( nCount == nActive )
                                        {
                                            break;
                                        }
                                        nStartPosition = nEndPosition+1;
                                    }
                                    else if( cNext == cSheetSep )
                                    {
                                        continue;
                                    }
                                }
                            }

                            if (nStartPosition > 0)
                            {
                                nArgs = ppFDesc->getParameterCount();
                                sal_Int16 nVarArgsSet = 0;
                                if ( nArgs >= PAIRED_VAR_ARGS )
                                {
                                    nVarArgsSet = 2;
                                    nArgs -= PAIRED_VAR_ARGS - nVarArgsSet;
                                }
                                else if ( nArgs >= VAR_ARGS )
                                {
                                    nVarArgsSet = 1;
                                    nArgs -= VAR_ARGS - nVarArgsSet;
                                }
                                if ( nVarArgsSet > 0 && nActive > nArgs )
                                    nActive = nArgs - (nActive - nArgs) % nVarArgsSet;
                                aNew = OUString::Concat(aNew.subView(0, nStartPosition)) +
                                        u"\x25BA" +
                                        aNew.subView(nStartPosition) +
                                        " : " +
                                        ppFDesc->getParameterDescription(nActive-1);
                                if (eMode != SC_INPUT_TOP)
                                {
                                    ShowTipBelow( aNew );
                                }
                                else
                                {
                                    ShowTip(aNew);
                                }
                                bFound = true;
                            }
                        }
                        else
                        {
                            ShowTipBelow( aNew );
                            bFound = true;
                        }
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
}

void ScInputHandler::ShowTipCursor()
{
    HideTip();
    HideTipBelow();
    EditView* pActiveView = pTopView ? pTopView : pTableView;

    if ( !(bFormulaMode && pActiveView && pFormulaDataPara && mpEditEngine->GetParagraphCount() == 1) )
        return;

    OUString aParagraph = mpEditEngine->GetText( 0 );
    ESelection aSel = pActiveView->GetSelection();
    aSel.Adjust();

    if ( aParagraph.getLength() < aSel.nEndPos )
        return;

    if ( aSel.nEndPos > 0 )
    {
        OUString aSelText( aParagraph.copy( 0, aSel.nEndPos ));

        ShowArgumentsTip( aSelText );
    }
}

void ScInputHandler::ShowTip( const OUString& rText )
{
    // aManualTip needs to be set afterwards from outside

    HideTip();
    HideTipBelow();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (!pActiveView)
        return;

    Point aPos;
    if (pInputWin && pInputWin->GetEditView() == pActiveView)
    {
        pTipVisibleParent = pInputWin->GetEditWindow();
        aPos = pInputWin->GetCursorScreenPixelPos();
    }
    else
    {
        pTipVisibleParent = pActiveView->GetWindow();
        if (vcl::Cursor* pCur = pActiveView->GetCursor())
            aPos = pTipVisibleParent->LogicToPixel( pCur->GetPos() );
        aPos = pTipVisibleParent->OutputToScreenPixel( aPos );
    }

    tools::Rectangle aRect( aPos, aPos );
    QuickHelpFlags const nAlign = QuickHelpFlags::Left|QuickHelpFlags::Bottom;
    nTipVisible = Help::ShowPopover(pTipVisibleParent, aRect, rText, nAlign);
    pTipVisibleParent->AddEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleParentListener ) );
}

void ScInputHandler::ShowTipBelow( const OUString& rText )
{
    HideTipBelow();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( !pActiveView )
        return;

    Point aPos;
    if (pInputWin && pInputWin->GetEditView() == pActiveView)
    {
        pTipVisibleSecParent = pInputWin->GetEditWindow();
        aPos = pInputWin->GetCursorScreenPixelPos(true);
    }
    else
    {
        pTipVisibleSecParent = pActiveView->GetWindow();
        if (vcl::Cursor* pCur = pActiveView->GetCursor())
        {
            Point aLogicPos = pCur->GetPos();
            aLogicPos.AdjustY(pCur->GetHeight() );
            aPos = pTipVisibleSecParent->LogicToPixel( aLogicPos );
        }
        aPos = pTipVisibleSecParent->OutputToScreenPixel( aPos );
    }

    tools::Rectangle aRect( aPos, aPos );
    QuickHelpFlags const nAlign = QuickHelpFlags::Left | QuickHelpFlags::Top | QuickHelpFlags::NoEvadePointer;
    nTipVisibleSec = Help::ShowPopover(pTipVisibleSecParent, aRect, rText, nAlign);
    pTipVisibleSecParent->AddEventListener( LINK( this, ScInputHandler, ShowHideTipVisibleSecParentListener ) );
}

bool ScInputHandler::GetFuncName( OUString& aStart, OUString& aResult )
{
    if ( aStart.isEmpty() )
        return false;

    aStart = ScGlobal::getCharClass().uppercase( aStart );
    sal_Int32 nPos = aStart.getLength() - 1;
    sal_Unicode c = aStart[ nPos ];
    // fdo#75264 use maFormulaChar to check if characters are used in function names
    ::std::set< sal_Unicode >::const_iterator p = maFormulaChar.find( c );
    if ( p == maFormulaChar.end() )
        return false; // last character is not part of any function name, quit

    ::std::vector<sal_Unicode> aTemp { c };
    for(sal_Int32 i = nPos - 1; i >= 0; --i)
    {
        c = aStart[ i ];
        p = maFormulaChar.find( c );

        if (p == maFormulaChar.end())
            break;

        aTemp.push_back( c );
    }

    ::std::vector<sal_Unicode>::reverse_iterator rIt = aTemp.rbegin();
    aResult = OUString( *rIt++ );
    while ( rIt != aTemp.rend() )
        aResult += OUStringChar( *rIt++ );

    return true;
}

namespace {
    /// Rid ourselves of unwanted " quoted json characters.
    OString escapeJSON(const OUString &aStr)
    {
        OUString aEscaped = aStr;
        aEscaped = aEscaped.replaceAll("\n", " ");
        aEscaped = aEscaped.replaceAll("\"", "'");
        return OUStringToOString(aEscaped, RTL_TEXTENCODING_UTF8);
    }
}

void ScInputHandler::ShowFuncList( const ::std::vector< OUString > & rFuncStrVec )
{
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (rFuncStrVec.size() && pViewShell && pViewShell->isLOKMobilePhone())
        {
            auto aPos = pFormulaData->begin();
            sal_uInt32 nCurIndex = std::distance(aPos, miAutoPosFormula);
            const sal_uInt32 nSize = pFormulaData->size();

            OUString aFuncNameStr;
            OUString aDescFuncNameStr;
            OStringBuffer aPayload;
            aPayload.append("[ ");
            for (const OUString& rFunc : rFuncStrVec)
            {
                if ( rFunc[rFunc.getLength()-1] == cParenthesesReplacement )
                {
                    aFuncNameStr = rFunc.copy(0, rFunc.getLength()-1);
                }
                else
                {
                    aFuncNameStr = rFunc;
                }

                FormulaHelper aHelper(ScGlobal::GetStarCalcFunctionMgr());
                aDescFuncNameStr = aFuncNameStr + "()";
                sal_Int32 nNextFStart = 0;
                const IFunctionDescription* ppFDesc;
                ::std::vector< OUString > aArgs;
                OUString eqPlusFuncName = "=" + aDescFuncNameStr;
                if ( aHelper.GetNextFunc( eqPlusFuncName, false, nNextFStart, nullptr, &ppFDesc, &aArgs ) )
                {
                    if ( !ppFDesc->getFunctionName().isEmpty() )
                    {
                        aPayload.append("{");
                        aPayload.append("\"index\": ");
                        aPayload.append(static_cast<sal_Int64>(nCurIndex));
                        aPayload.append(", ");
                        aPayload.append("\"signature\": \"");
                        aPayload.append(escapeJSON(ppFDesc->getSignature()));
                        aPayload.append("\", ");
                        aPayload.append("\"description\": \"");
                        aPayload.append(escapeJSON(ppFDesc->getDescription()));
                        aPayload.append("\"}, ");
                    }
                }
                ++nCurIndex;
                if (nCurIndex == nSize)
                    nCurIndex = 0;
            }
            sal_Int32 nLen = aPayload.getLength();
            aPayload[nLen - 2] = ' ';
            aPayload[nLen - 1] = ']';

            OString s = aPayload.makeStringAndClear();
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CALC_FUNCTION_LIST, s.getStr());
        }
        // not tunnel tooltips in the lok case
        return;
    }

    OUStringBuffer aTipStr;
    OUString aFuncNameStr;
    OUString aDescFuncNameStr;
    ::std::vector<OUString>::const_iterator itStr = rFuncStrVec.begin();
    sal_Int32 nMaxFindNumber = 3;
    sal_Int32 nRemainFindNumber = nMaxFindNumber;
    for ( ; itStr != rFuncStrVec.end(); ++itStr )
    {
        const OUString& rFunc = *itStr;
        if ( rFunc[rFunc.getLength()-1] == cParenthesesReplacement )
        {
            aFuncNameStr = rFunc.copy(0, rFunc.getLength()-1);
        }
        else
        {
            aFuncNameStr = rFunc;
        }
        if ( itStr == rFuncStrVec.begin() )
        {
            aTipStr = "[";
            aDescFuncNameStr = aFuncNameStr + "()";
        }
        else
        {
            aTipStr.append(", ");
        }
        aTipStr.append(aFuncNameStr);
        if ( itStr == rFuncStrVec.begin() )
            aTipStr.append("]");
        if ( --nRemainFindNumber <= 0 )
            break;
    }
    sal_Int32 nRemainNumber = rFuncStrVec.size() - nMaxFindNumber;
    if ( nRemainFindNumber == 0 && nRemainNumber > 0 )
    {
        OUString aMessage( ScResId( STR_FUNCTIONS_FOUND ) );
        aMessage = aMessage.replaceFirst("%2", OUString::number(nRemainNumber));
        aMessage = aMessage.replaceFirst("%1", aTipStr.makeStringAndClear());
        aTipStr = aMessage;
    }
    FormulaHelper aHelper(ScGlobal::GetStarCalcFunctionMgr());
    sal_Int32 nNextFStart = 0;
    const IFunctionDescription* ppFDesc;
    ::std::vector< OUString > aArgs;
    OUString eqPlusFuncName = "=" + aDescFuncNameStr;
    if ( aHelper.GetNextFunc( eqPlusFuncName, false, nNextFStart, nullptr, &ppFDesc, &aArgs ) )
    {
        if ( !ppFDesc->getFunctionName().isEmpty() )
        {
            aTipStr.append(" : " + ppFDesc->getDescription());
        }
    }
    ShowTip( aTipStr.makeStringAndClear() );
}

void ScInputHandler::UseFormulaData()
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;

    // Formulas may only have 1 paragraph
    if ( !(pActiveView && pFormulaData && mpEditEngine->GetParagraphCount() == 1) )
        return;

    OUString aParagraph = mpEditEngine->GetText( 0 );
    ESelection aSel = pActiveView->GetSelection();
    aSel.Adjust();

    // Due to differences between table and input cell (e.g clipboard with line breaks),
    // the selection may not be in line with the EditEngine anymore.
    // Just return without any indication as to why.
    if ( aSel.nEndPos > aParagraph.getLength() )
        return;

    if ( aParagraph.getLength() > aSel.nEndPos &&
         ( ScGlobal::getCharClass().isLetterNumeric( aParagraph, aSel.nEndPos ) ||
           aParagraph[ aSel.nEndPos ] == '_' ||
           aParagraph[ aSel.nEndPos ] == '.' ||
           aParagraph[ aSel.nEndPos ] == '$'   ) )
        return;

    //  Is the cursor at the end of a word?
    if ( aSel.nEndPos <= 0 )
        return;

    OUString aSelText( aParagraph.copy( 0, aSel.nEndPos ));

    OUString aText;
    if ( GetFuncName( aSelText, aText ) )
    {
        // function name is incomplete:
        // show matching functions name as tip above cell
        ::std::vector<OUString> aNewVec;
        miAutoPosFormula = pFormulaData->end();
        miAutoPosFormula = findTextAll(*pFormulaData, miAutoPosFormula, aText, aNewVec, false);
        if (miAutoPosFormula != pFormulaData->end())
        {
            // check if partial function name is not between quotes
            sal_Unicode cBetweenQuotes = 0;
            for ( int n = 0; n < aSelText.getLength(); n++ )
            {
                if (cBetweenQuotes)
                {
                    if (aSelText[n] == cBetweenQuotes)
                        cBetweenQuotes = 0;
                }
                else if ( aSelText[ n ] == '"' )
                    cBetweenQuotes = '"';
                else if ( aSelText[ n ] == '\'' )
                    cBetweenQuotes = '\'';
            }
            if ( cBetweenQuotes )
                return;  // we're between quotes

            ShowFuncList(aNewVec);
            aAutoSearch = aText;
        }
        return;
    }

    // function name is complete:
    // show tip below the cell with function name and arguments of function
    ShowArgumentsTip( aSelText );
}

void ScInputHandler::NextFormulaEntry( bool bBack )
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pFormulaData )
    {
        ::std::vector<OUString> aNewVec;
        ScTypedCaseStrSet::const_iterator itNew = findTextAll(*pFormulaData, miAutoPosFormula, aAutoSearch, aNewVec, bBack);
        if (itNew != pFormulaData->end())
        {
            miAutoPosFormula = itNew;
            ShowFuncList( aNewVec );
        }
    }

    // For Tab we always call HideCursor first
    if (pActiveView)
        pActiveView->ShowCursor();
}

namespace {

void completeFunction( EditView* pView, const OUString& rInsert, bool& rParInserted )
{
    if (!pView)
        return;

    ESelection aSel = pView->GetSelection();

    bool bNoInitialLetter = false;
    OUString aOld = pView->GetEditEngine()->GetText(0);
    // in case we want just insert a function and not completing
    if ( comphelper::LibreOfficeKit::isActive() )
    {
        ESelection aSelRange = aSel;
        --aSelRange.nStartPos;
        --aSelRange.nEndPos;
        pView->SetSelection(aSelRange);
        pView->SelectCurrentWord();

        if ( aOld == "=" )
        {
            bNoInitialLetter = true;
            aSelRange.nStartPos = 1;
            aSelRange.nEndPos = 1;
            pView->SetSelection(aSelRange);
        }
        else if ( pView->GetSelected().startsWith("()") )
        {
            bNoInitialLetter = true;
            ++aSelRange.nStartPos;
            ++aSelRange.nEndPos;
            pView->SetSelection(aSelRange);
        }
    }

    if(!bNoInitialLetter)
    {
        const sal_Int32 nMinLen = std::max(aSel.nEndPos - aSel.nStartPos, sal_Int32(1));
        // Since transliteration service is used to test for match, the replaced string could be
        // longer than rInsert, so in order to find longest match before the cursor, test whole
        // string from start to current cursor position (don't limit to length of rInsert)
        // Disclaimer: I really don't know if a match longer than rInsert is actually possible,
        // so the above is based on assumptions how "transliteration" might possibly work. If
        // it's in fact impossible, an optimization would be useful to limit aSel.nStartPos to
        // std::max(sal_Int32(0), aSel.nEndPos - rInsert.getLength()).
        aSel.nStartPos = 0;
        pView->SetSelection(aSel);
        const OUString aAll = pView->GetSelected();
        OUString aMatch;
        for (sal_Int32 n = aAll.getLength(); n >= nMinLen && aMatch.isEmpty(); --n)
        {
            const OUString aTest = aAll.copy(aAll.getLength() - n); // n trailing chars
            if (ScGlobal::GetTransliteration().isMatch(aTest, rInsert))
                aMatch = aTest; // Found => break the loop
        }

        aSel.nStartPos = aSel.nEndPos - aMatch.getLength();
        pView->SetSelection(aSel);
    }

    OUString aInsStr = rInsert;
    sal_Int32 nInsLen = aInsStr.getLength();
    bool bDoParen = ( nInsLen > 1 && aInsStr[nInsLen-2] == '('
                                  && aInsStr[nInsLen-1] == ')' );
    if ( bDoParen )
    {
        // Do not insert parentheses after function names if there already are some
        // (e.g. if the function name was edited).
        ESelection aWordSel = pView->GetSelection();

        // aWordSel.EndPos points one behind string if word at end
        if (aWordSel.nEndPos < aOld.getLength())
        {
            sal_Unicode cNext = aOld[aWordSel.nEndPos];
            if ( cNext == '(' )
            {
                bDoParen = false;
                aInsStr = aInsStr.copy( 0, nInsLen - 2 ); // Skip parentheses
            }
        }
    }

    pView->InsertText( aInsStr );

    if ( bDoParen ) // Put cursor between parentheses
    {
        aSel = pView->GetSelection();
        --aSel.nStartPos;
        --aSel.nEndPos;
        pView->SetSelection(aSel);

        rParInserted = true;
    }
}

}

void ScInputHandler::PasteFunctionData()
{
    if (pFormulaData && miAutoPosFormula != pFormulaData->end())
    {
        const ScTypedStrData& rData = *miAutoPosFormula;
        OUString aInsert = rData.GetString();
        if (aInsert[aInsert.getLength()-1] == cParenthesesReplacement)
            aInsert = OUString::Concat(aInsert.subView( 0, aInsert.getLength()-1)) + "()";
        bool bParInserted = false;

        DataChanging(); // Cannot be new
        completeFunction( pTopView, aInsert, bParInserted );
        completeFunction( pTableView, aInsert, bParInserted );
        DataChanged();
        ShowTipCursor();

        if (bParInserted)
            AutoParAdded();
    }

    HideTip();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (comphelper::LibreOfficeKit::isActive() && pTopView && pInputWin)
        pInputWin->TextGrabFocus();
    if (pActiveView)
        pActiveView->ShowCursor();
}

void ScInputHandler::LOKPasteFunctionData(const OUString& rFunctionName)
{
    // in case we have no top view try to create it
    if (!pTopView && pInputWin)
    {
        ScInputMode eCurMode = eMode;
        SetMode(SC_INPUT_TOP);
        if (!pTopView)
            SetMode(eCurMode);
    }

    EditView* pEditView = pTopView ? pTopView : pTableView;

    if (!pActiveViewSh || !pEditView)
        return;

    bool bEdit = false;
    OUString aFormula;
    const EditEngine* pEditEngine = pEditView->GetEditEngine();
    if (pEditEngine)
    {
        aFormula = pEditEngine->GetText(0);
        bEdit = aFormula.getLength() > 1 && (aFormula[0] == '=' || aFormula[0] == '+' || aFormula[0] == '-');
    }

    if ( !bEdit )
    {
        OUString aNewFormula('=');
        if ( aFormula.startsWith("=") )
            aNewFormula = aFormula;

        InputReplaceSelection( aNewFormula );
    }

    if (pFormulaData)
    {
        OUString aNew;
        ScTypedCaseStrSet::const_iterator aPos = findText(*pFormulaData, pFormulaData->begin(), rFunctionName, aNew, /* backward = */false);

        if (aPos != pFormulaData->end())
        {
            miAutoPosFormula = aPos;
            PasteFunctionData();
        }
    }
}

// Calculate selection and display as tip help
static OUString lcl_Calculate( const OUString& rFormula, ScDocument& rDoc, const ScAddress &rPos )
{
    //TODO: Merge with ScFormulaDlg::CalcValue and move into Document!
    // Quotation marks for Strings are only inserted here.

    if(rFormula.isEmpty())
        return OUString();

    std::optional<ScSimpleFormulaCalculator> pCalc( std::in_place, rDoc, rPos, rFormula, false );

    // FIXME: HACK! In order to not get a #REF! for ColRowNames, if a name is actually inserted as a Range
    // into the whole Formula, but is interpreted as a single cell reference when displaying it on its own
    bool bColRowName = pCalc->HasColRowName();
    if ( bColRowName )
    {
        // ColRowName in RPN code?
        if ( pCalc->GetCode()->GetCodeLen() <= 1 )
        {   // ==1: Single one is as a Parameter always a Range
            // ==0: It might be one, if ...
            OUString aBraced = "(" + rFormula + ")";
            pCalc.emplace( rDoc, rPos, aBraced, false );
        }
        else
            bColRowName = false;
    }

    FormulaError nErrCode = pCalc->GetErrCode();
    if ( nErrCode != FormulaError::NONE )
        return ScGlobal::GetErrorString(nErrCode);

    SvNumberFormatter& aFormatter = *rDoc.GetFormatTable();
    OUString aValue;
    if ( pCalc->IsValue() )
    {
        double n = pCalc->GetValue();
        sal_uInt32 nFormat = aFormatter.GetStandardFormat( n, 0,
                pCalc->GetFormatType(), ScGlobal::eLnge );
        aFormatter.GetInputLineString( n, nFormat, aValue );
        //! display OutputString but insert InputLineString
    }
    else
    {
        OUString aStr = pCalc->GetString().getString();
        sal_uInt32 nFormat = aFormatter.GetStandardFormat(
                pCalc->GetFormatType(), ScGlobal::eLnge);
        {
            const Color* pColor;
            aFormatter.GetOutputString( aStr, nFormat,
                    aValue, &pColor );
        }

        aValue = "\"" + aValue + "\"";
        //! Escape quotation marks in String??
    }

    ScRange aTestRange;
    if ( bColRowName || (aTestRange.Parse(rFormula, rDoc) & ScRefFlags::VALID) )
        aValue += " ...";

    return aValue;
}

void ScInputHandler::FormulaPreview()
{
    OUString aValue;
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pActiveViewSh )
    {
        OUString aPart = pActiveView->GetSelected();
        if (aPart.isEmpty())
            aPart = mpEditEngine->GetText(0);
        ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocShell()->GetDocument();
        aValue = lcl_Calculate( aPart, rDoc, aCursorPos );
    }

    if (!aValue.isEmpty())
    {
        ShowTip( aValue );          //  Display as QuickHelp
        aManualTip = aValue;        //  Set after ShowTip
        if (pFormulaData)
            miAutoPosFormula = pFormulaData->end();
        if (pColumnData)
            miAutoPosColumn = pColumnData->end();
    }
}

void ScInputHandler::PasteManualTip()
{
    //  Three dots at the end -> Range reference -> do not insert
    //  FIXME: Once we have matrix constants, we can change this
    sal_Int32 nTipLen = aManualTip.getLength();
    sal_uInt32 const nTipLen2(sal::static_int_cast<sal_uInt32>(nTipLen));
    if ( nTipLen && ( nTipLen < 3 || aManualTip.subView( nTipLen2-3 ) != u"..." ) )
    {
        DataChanging(); // Cannot be new

        OUString aInsert = aManualTip;
        EditView* pActiveView = pTopView ? pTopView : pTableView;
        if (!pActiveView->HasSelection())
        {
            // Nothing selected -> select everything
            sal_Int32 nOldLen = mpEditEngine->GetTextLen(0);
            ESelection aAllSel( 0, 0, 0, nOldLen );
            if ( pTopView )
                pTopView->SetSelection( aAllSel );
            if ( pTableView )
                pTableView->SetSelection( aAllSel );
        }

        ESelection aSel = pActiveView->GetSelection();
        aSel.Adjust();
        OSL_ENSURE( !aSel.nStartPara && !aSel.nEndPara, "Too many paragraphs in Formula" );
        if ( !aSel.nStartPos )  // Selection from the start?
        {
            if ( aSel.nEndPos == mpEditEngine->GetTextLen(0) )
            {
                //  Everything selected -> skip quotation marks
                if ( aInsert[0] == '"' )
                    aInsert = aInsert.copy(1);
                sal_Int32 nInsLen = aInsert.getLength();
                if ( aInsert.endsWith("\"") )
                    aInsert = aInsert.copy( 0, nInsLen-1 );
            }
            else if ( aSel.nEndPos )
            {
                //  Not everything selected -> do not overwrite equality sign
                //FIXME: Even double equality signs??
                aSel.nStartPos = 1;
                if ( pTopView )
                    pTopView->SetSelection( aSel );
                if ( pTableView )
                    pTableView->SetSelection( aSel );
            }
        }
        if ( pTopView )
            pTopView->InsertText( aInsert, true );
        if ( pTableView )
            pTableView->InsertText( aInsert, true );

        DataChanged();
    }

    HideTip();
}

void ScInputHandler::ResetAutoPar()
{
    nAutoPar = 0;
}

void ScInputHandler::AutoParAdded()
{
    ++nAutoPar; // Closing parenthesis can be overwritten
}

bool ScInputHandler::CursorAtClosingPar()
{
    // Test if the cursor is before a closing parenthesis
    // Selection from SetReference has been removed before
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && !pActiveView->HasSelection() && bFormulaMode )
    {
        ESelection aSel = pActiveView->GetSelection();
        sal_Int32 nPos = aSel.nStartPos;
        OUString aFormula = mpEditEngine->GetText(0);
        if ( nPos < aFormula.getLength() && aFormula[nPos] == ')' )
            return true;
    }
    return false;
}

void ScInputHandler::SkipClosingPar()
{
    //  this is called when a ')' is typed and the cursor is before a ')'
    //  that can be overwritten -> just set the cursor behind the ')'

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView)
    {
        ESelection aSel = pActiveView->GetSelection();
        ++aSel.nStartPos;
        ++aSel.nEndPos;

        //  this is in a formula (only one paragraph), so the selection
        //  can be used directly for the TopView

        if ( pTopView )
            pTopView->SetSelection( aSel );
        if ( pTableView )
            pTableView->SetSelection( aSel );
    }

    OSL_ENSURE(nAutoPar, "SkipClosingPar: count is wrong");
    --nAutoPar;
}

// Auto input

void ScInputHandler::GetColData()
{
    if ( !pActiveViewSh )
        return;

    ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocShell()->GetDocument();

    if ( pColumnData )
        pColumnData->clear();
    else
        pColumnData.reset( new ScTypedCaseStrSet );

    std::vector<ScTypedStrData> aEntries;
    rDoc.GetDataEntries(
        aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), aEntries);
    if (!aEntries.empty())
        pColumnData->insert(aEntries.begin(), aEntries.end());

    miAutoPosColumn = pColumnData->end();
}

void ScInputHandler::UseColData() // When typing
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( !(pActiveView && pColumnData) )
        return;

    //  Only change when cursor is at the end
    ESelection aSel = pActiveView->GetSelection();
    aSel.Adjust();

    sal_Int32 nParCnt = mpEditEngine->GetParagraphCount();
    if ( aSel.nEndPara+1 != nParCnt )
        return;

    sal_Int32 nParLen = mpEditEngine->GetTextLen( aSel.nEndPara );
    if ( aSel.nEndPos != nParLen )
        return;

    OUString aText = GetEditText(mpEditEngine.get());
    if (aText.isEmpty())
        return;

    std::vector< OUString > aResultVec;
    OUString aNew;
    sal_Int32 nLongestPrefixLen = 0;
    miAutoPosColumn = pColumnData->end();
    mbPartialPrefix = false;
    miAutoPosColumn = findTextAll(*pColumnData, miAutoPosColumn, aText, aResultVec, false, &nLongestPrefixLen);

    if (nLongestPrefixLen <= 0 || aResultVec.empty())
        return;

    if (aResultVec.size() > 1)
    {
        mbPartialPrefix = true;
        bUseTab = true; // Allow Ctrl (+ Shift + ) + TAB cycling.
        miAutoPosColumn = pColumnData->end();

        // Display the rest of longest common prefix as suggestion.
        aNew = aResultVec[0].copy(0, nLongestPrefixLen);
    }
    else
    {
        aNew = aResultVec[0];
    }

    // Strings can contain line endings (e.g. due to dBase import),
    // which would result in multiple paragraphs here, which is not desirable.
    //! Then GetExactMatch doesn't work either
    lcl_RemoveLineEnd( aNew );

    // Keep paragraph, just append the rest
    //! Exact replacement in EnterHandler !!!
    // One Space between paragraphs:
    sal_Int32 nEdLen = mpEditEngine->GetTextLen() + nParCnt - 1;
    OUString aIns = aNew.copy(nEdLen);

    // Selection must be "backwards", so the cursor stays behind the last
    // typed character
    ESelection aSelection( aSel.nEndPara, aSel.nEndPos + aIns.getLength(),
                           aSel.nEndPara, aSel.nEndPos );

    // When editing in input line, apply to both edit views
    if ( pTableView )
    {
        pTableView->InsertText( aIns );
        pTableView->SetSelection( aSelection );
    }
    if ( pTopView )
    {
        pTopView->InsertText( aIns );
        pTopView->SetSelection( aSelection );
    }

    aAutoSearch = aText; // To keep searching - nAutoPos is set
}

void ScInputHandler::NextAutoEntry( bool bBack )
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if ( pActiveView && pColumnData )
    {
        if (!aAutoSearch.isEmpty())
        {
            // Is the selection still valid (could be changed via the mouse)?
            ESelection aSel = pActiveView->GetSelection();
            aSel.Adjust();
            sal_Int32 nParCnt = mpEditEngine->GetParagraphCount();
            if ( aSel.nEndPara+1 == nParCnt && aSel.nStartPara == aSel.nEndPara )
            {
                OUString aText = GetEditText(mpEditEngine.get());
                sal_Int32 nSelLen = aSel.nEndPos - aSel.nStartPos;
                sal_Int32 nParLen = mpEditEngine->GetTextLen( aSel.nEndPara );
                if ( aSel.nEndPos == nParLen && aText.getLength() == aAutoSearch.getLength() + nSelLen )
                {
                    OUString aNew;
                    ScTypedCaseStrSet::const_iterator itNew =
                        findText(*pColumnData, miAutoPosColumn, aAutoSearch, aNew, bBack);

                    if (itNew != pColumnData->end())
                    {
                        // match found!
                        miAutoPosColumn = itNew;
                        bInOwnChange = true;        // disable ModifyHdl (reset below)
                        mbPartialPrefix = false;

                        lcl_RemoveLineEnd( aNew );
                        OUString aIns = aNew.copy(aAutoSearch.getLength());

                        //  when editing in input line, apply to both edit views
                        if ( pTableView )
                        {
                            pTableView->DeleteSelected();
                            pTableView->InsertText( aIns );
                            pTableView->SetSelection( ESelection(
                                                        aSel.nEndPara, aSel.nStartPos + aIns.getLength(),
                                                        aSel.nEndPara, aSel.nStartPos ) );
                        }
                        if ( pTopView )
                        {
                            pTopView->DeleteSelected();
                            pTopView->InsertText( aIns );
                            pTopView->SetSelection( ESelection(
                                                        aSel.nEndPara, aSel.nStartPos + aIns.getLength(),
                                                        aSel.nEndPara, aSel.nStartPos ) );
                        }

                        bInOwnChange = false;
                    }
                }
            }
        }
    }

    // For Tab, HideCursor was always called first
    if (pActiveView)
        pActiveView->ShowCursor();
}

// Highlight parentheses
void ScInputHandler::UpdateParenthesis()
{
    // Find parentheses
    //TODO: Can we disable parentheses highlighting per parentheses?
    bool bFound = false;
    if ( bFormulaMode && eMode != SC_INPUT_TOP )
    {
        if ( pTableView && !pTableView->HasSelection() ) // Selection is always at the bottom
        {
            ESelection aSel = pTableView->GetSelection();
            if (aSel.nStartPos)
            {
                // Examine character left to the cursor
                sal_Int32 nPos = aSel.nStartPos - 1;
                OUString aFormula = mpEditEngine->GetText(0);
                sal_Unicode c = aFormula[nPos];
                if ( c == '(' || c == ')' )
                {
                    sal_Int32 nOther = lcl_MatchParenthesis( aFormula, nPos );
                    if ( nOther != -1 )
                    {
                        SfxItemSet aSet( mpEditEngine->GetEmptyItemSet() );
                        aSet.Put( SvxWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT ) );

                        //! Distinguish if cell is already highlighted!!!!
                        if (bParenthesisShown)
                        {
                            // Remove old highlighting
                            sal_Int32 nCount = mpEditEngine->GetParagraphCount();
                            for (sal_Int32 i=0; i<nCount; i++)
                                mpEditEngine->RemoveCharAttribs( i, EE_CHAR_WEIGHT );
                        }

                        ESelection aSelThis( 0,nPos, 0,nPos+1 );
                        mpEditEngine->QuickSetAttribs( aSet, aSelThis );
                        ESelection aSelOther( 0,nOther, 0,nOther+1 );
                        mpEditEngine->QuickSetAttribs( aSet, aSelOther );

                        // Dummy InsertText for Update and Paint (selection is empty)
                        pTableView->InsertText( OUString() );

                        bFound = true;
                    }
                }
            }

            //  mark parenthesis right of cursor if it will be overwritten (nAutoPar)
            //  with different color (COL_LIGHTBLUE) ??
        }
    }

    // Remove old highlighting, if no new one is set
    if ( bParenthesisShown && !bFound && pTableView )
    {
        sal_Int32 nCount = mpEditEngine->GetParagraphCount();
        for (sal_Int32 i=0; i<nCount; i++)
            pTableView->RemoveCharAttribs( i, EE_CHAR_WEIGHT );
    }

    bParenthesisShown = bFound;
}

void ScInputHandler::ViewShellGone(const ScTabViewShell* pViewSh) // Executed synchronously!
{
    if ( pViewSh == pActiveViewSh )
    {
        pLastState.reset();
        pLastPattern = nullptr;
    }

    if ( pViewSh == pRefViewSh )
    {
        //! The input from the EnterHandler does not arrive anymore
        // We end the EditMode anyways
        EnterHandler();
        bFormulaMode = false;
        pRefViewSh = nullptr;
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScRefModeChanged ) );
        SC_MOD()->SetRefInputHdl(nullptr);
        if (pInputWin)
            pInputWin->SetFormulaMode(false);
        UpdateAutoCorrFlag();
    }

    pActiveViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );

    if ( pActiveViewSh && pActiveViewSh == pViewSh )
    {
        OSL_FAIL("pActiveViewSh is gone");
        pActiveViewSh = nullptr;
    }

    if ( SC_MOD()->GetInputOptions().GetTextWysiwyg() )
        UpdateRefDevice(); // Don't keep old document's printer as RefDevice
}

void ScInputHandler::UpdateActiveView()
{
    ImplCreateEditEngine();

    // #i20588# Don't rely on focus to find the active edit view. Instead, the
    // active pane at the start of editing is now stored (GetEditActivePart).
    // GetActiveWin (the currently active pane) fails for ref input across the
    // panes of a split view.

    vcl::Window* pShellWin = pActiveViewSh ?
                pActiveViewSh->GetWindowByPos( pActiveViewSh->GetViewData().GetEditActivePart() ) :
                nullptr;

    sal_uInt16 nCount = mpEditEngine->GetViewCount();
    if (nCount > 0)
    {
        pTableView = mpEditEngine->GetView();
        for (sal_uInt16 i=1; i<nCount; i++)
        {
            EditView* pThis = mpEditEngine->GetView(i);
            vcl::Window* pWin = pThis->GetWindow();
            if ( pWin==pShellWin )
                pTableView = pThis;
        }
    }
    else
        pTableView = nullptr;

    // setup the pTableView editeng for tiled rendering to get cursor and selections
    if (pTableView && pActiveViewSh)
    {
        if (comphelper::LibreOfficeKit::isActive())
        {
            pTableView->RegisterViewShell(pActiveViewSh);
        }
    }

    if (pInputWin && (eMode == SC_INPUT_TOP || eMode == SC_INPUT_TABLE))
    {
        // tdf#71409: Always create the edit engine instance for the input
        // window, in order to properly manage accessibility events.
        pTopView = pInputWin->GetEditView();
        if (eMode != SC_INPUT_TOP)
            pTopView = nullptr;
    }
    else
        pTopView = nullptr;
}

void ScInputHandler::SetInputWindow(  ScInputWindow* pNew )
{
    pInputWin = pNew;
}

void ScInputHandler::StopInputWinEngine( bool bAll )
{
    if (pInputWin && !pInputWin->isDisposed())
        pInputWin->StopEditEngine( bAll );

    pTopView = nullptr; // invalid now
}

EditView* ScInputHandler::GetActiveView()
{
    UpdateActiveView();
    return pTopView ? pTopView : pTableView;
}

void ScInputHandler::ForgetLastPattern()
{
    pLastPattern = nullptr;
    if ( !pLastState && pActiveViewSh )
        pActiveViewSh->UpdateInputHandler( true ); // Get status again
    else
        NotifyChange( pLastState.get(), true );
}

void ScInputHandler::UpdateAdjust( sal_Unicode cTyped )
{
    SvxAdjust eSvxAdjust;
    switch (eAttrAdjust)
    {
        case SvxCellHorJustify::Standard:
            {
                bool bNumber = false;
                if (cTyped)                                     // Restarted
                    bNumber = (cTyped>='0' && cTyped<='9');     // Only ciphers are numbers
                else if ( pActiveViewSh )
                {
                    ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocShell()->GetDocument();
                    bNumber = ( rDoc.GetCellType( aCursorPos ) == CELLTYPE_VALUE );
                }
                eSvxAdjust = bNumber ? SvxAdjust::Right : SvxAdjust::Left;
            }
            break;
        case SvxCellHorJustify::Block:
            eSvxAdjust = SvxAdjust::Block;
            break;
        case SvxCellHorJustify::Center:
            eSvxAdjust = SvxAdjust::Center;
            break;
        case SvxCellHorJustify::Right:
            eSvxAdjust = SvxAdjust::Right;
            break;
        default:    // SvxCellHorJustify::Left
            eSvxAdjust = SvxAdjust::Left;
            break;
    }

    bool bAsianVertical = pLastPattern &&
        pLastPattern->GetItem( ATTR_STACKED ).GetValue() &&
        pLastPattern->GetItem( ATTR_VERTICAL_ASIAN ).GetValue();
    if ( bAsianVertical )
    {
        // Always edit at top of cell -> LEFT when editing vertically
        eSvxAdjust = SvxAdjust::Left;
    }

    pEditDefaults->Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );
    mpEditEngine->SetDefaults( *pEditDefaults );

    if ( pActiveViewSh )
    {
        pActiveViewSh->GetViewData().SetEditAdjust( eSvxAdjust );
    }
    mpEditEngine->SetVertical( bAsianVertical );
}

void ScInputHandler::RemoveAdjust()
{
    // Delete hard alignment attributes
    bool bUndo = mpEditEngine->IsUndoEnabled();
    if ( bUndo )
        mpEditEngine->EnableUndo( false );

    // Non-default paragraph attributes (e.g. from clipboard)
    // must be turned into character attributes
    mpEditEngine->RemoveParaAttribs();

    if ( bUndo )
        mpEditEngine->EnableUndo( true );

}

void ScInputHandler::RemoveRangeFinder()
{
    // Delete pRangeFindList and colors
    mpEditEngine->SetUpdateLayout(false);
    sal_Int32 nCount = mpEditEngine->GetParagraphCount(); // Could just have been inserted
    for (sal_Int32 i=0; i<nCount; i++)
        mpEditEngine->RemoveCharAttribs( i, EE_CHAR_COLOR );
    mpEditEngine->SetUpdateLayout(true);

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    pActiveView->ShowCursor( false );

    DeleteRangeFinder(); // Deletes the list and the labels on the table
}

bool ScInputHandler::StartTable( sal_Unicode cTyped, bool bFromCommand, bool bInputActivated,
        ScEditEngineDefaulter* pTopEngine )
{
    bool bNewTable = false;

    if (bModified)
        return false;

    if (pActiveViewSh)
    {
        ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocShell()->GetDocument();

        if (!rDoc.ValidCol(aCursorPos.Col()))
            return false;

        ImplCreateEditEngine();
        UpdateActiveView();
        SyncViews();


        const ScMarkData& rMark = pActiveViewSh->GetViewData().GetMarkData();
        ScEditableTester aTester;
        if ( rMark.IsMarked() || rMark.IsMultiMarked() )
            aTester.TestSelection( rDoc, rMark );
        else
            aTester.TestSelectedBlock(
                rDoc, aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Col(), aCursorPos.Row(), rMark );

        bool bStartInputMode = true;

        if (!aTester.IsEditable())
        {
            bProtected = true;
            // We allow read-only input mode activation regardless
            // whether it's part of an array or not or whether explicit cell
            // activation is requested (double-click or F2) or a click in input
            // line.
            bool bShowError = (!bInputActivated || !aTester.GetMessageId() || aTester.GetMessageId() != STR_PROTECTIONERR) &&
                !pActiveViewSh->GetViewData().GetDocShell()->IsReadOnly();
            if (bShowError)
            {
                eMode = SC_INPUT_NONE;
                StopInputWinEngine( true );
                UpdateFormulaMode();
                if ( pActiveViewSh && ( !bFromCommand || !bCommandErrorShown ) )
                {
                    //  Prevent repeated error messages for the same cell from command events
                    //  (for keyboard events, multiple messages are wanted).
                    //  Set the flag before showing the error message because the command handler
                    //  for the next IME command may be called when showing the dialog.
                    if ( bFromCommand )
                        bCommandErrorShown = true;

                    pActiveViewSh->GetActiveWin()->GrabFocus();
                    pActiveViewSh->ErrorMessage(aTester.GetMessageId());
                }
                bStartInputMode = false;
            }
        }

        if (bStartInputMode)
        {
            // UpdateMode is enabled again in ScViewData::SetEditEngine (and not needed otherwise)
            mpEditEngine->SetUpdateLayout( false );

            // Take over attributes in EditEngine
            const ScPatternAttr* pPattern = rDoc.GetPattern( aCursorPos.Col(),
                                                              aCursorPos.Row(),
                                                              aCursorPos.Tab() );
            if (pPattern != pLastPattern)
            {
                // Percent format?
                const SfxItemSet& rAttrSet = pPattern->GetItemSet();
                const SfxPoolItem* pItem;

                if ( SfxItemState::SET == rAttrSet.GetItemState( ATTR_VALUE_FORMAT, true, &pItem ) )
                {
                    sal_uInt32 nFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                    if (SvNumFormatType::PERCENT == rDoc.GetFormatTable()->GetType( nFormat ))
                        nCellPercentFormatDecSep = rDoc.GetFormatTable()->GetFormatDecimalSep( nFormat).toChar();
                    else
                        nCellPercentFormatDecSep = 0;
                }
                else
                    nCellPercentFormatDecSep = 0; // Default: no percent

                // Validity specified?
                if ( SfxItemState::SET == rAttrSet.GetItemState( ATTR_VALIDDATA, true, &pItem ) )
                    nValidation = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                else
                    nValidation = 0;

                //  EditEngine Defaults
                //  In no case SetParaAttribs, because the EditEngine might already
                //  be filled (for Edit cells).
                //  SetParaAttribs would change the content.

                //! The SetDefaults is now (since MUST/src602
                //! EditEngine changes) implemented as a SetParaAttribs.
                //! Any problems?

                pPattern->FillEditItemSet( pEditDefaults.get() );
                mpEditEngine->SetDefaults( *pEditDefaults );
                pLastPattern = pPattern;
                bLastIsSymbol = pPattern->IsSymbolFont();

                //  Background color must be known for automatic font color.
                //  For transparent cell background, the document background color must be used.

                Color aBackCol = pPattern->GetItem( ATTR_BACKGROUND ).GetColor();
                ScModule* pScMod = SC_MOD();
                if ( aBackCol.IsTransparent() ||
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                    aBackCol = pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
                mpEditEngine->SetBackgroundColor( aBackCol );

                // Adjustment
                eAttrAdjust = pPattern->GetItem(ATTR_HOR_JUSTIFY).GetValue();
                if ( eAttrAdjust == SvxCellHorJustify::Repeat &&
                     pPattern->GetItem(ATTR_LINEBREAK).GetValue() )
                {
                    // #i31843# "repeat" with "line breaks" is treated as default alignment
                    eAttrAdjust = SvxCellHorJustify::Standard;
                }
            }

            if (pTopEngine)
            {
                // Necessary to sync SvxAutoCorrect behavior. This has to be
                // done before InitRangeFinder() below.
                MergeLanguageAttributes( *pTopEngine);
            }

            //  UpdateSpellSettings enables online spelling if needed
            //  -> also call if attributes are unchanged
            UpdateSpellSettings( true ); // uses pLastPattern

            // Fill EditEngine
            OUString aStr;
            if (bTextValid)
            {
                mpEditEngine->SetTextCurrentDefaults(aCurrentText);
                aStr = aCurrentText;
                bTextValid = false;
                aCurrentText.clear();
            }
            else
                aStr = GetEditText(mpEditEngine.get());

            if (aStr.startsWith("{=") && aStr.endsWith("}") )  // Matrix formula?
            {
                aStr = aStr.copy(1, aStr.getLength() -2);
                mpEditEngine->SetTextCurrentDefaults(aStr);
                if ( pInputWin )
                    pInputWin->SetTextString(aStr);
            }

            UpdateAdjust( cTyped );

            if ( SC_MOD()->GetAppOptions().GetAutoComplete() )
                GetColData();

            if ( !aStr.isEmpty() && ( aStr[0] == '=' || aStr[0] == '+' || aStr[0] == '-' ) &&
                 !cTyped && !bCreatingFuncView )
                InitRangeFinder(aStr); // Formula is being edited -> RangeFinder

            bNewTable = true; // -> PostEditView Call
        }
    }

    if (!bProtected && pInputWin)
        pInputWin->SetOkCancelMode();

    return bNewTable;
}

void ScInputHandler::MergeLanguageAttributes( ScEditEngineDefaulter& rDestEngine ) const
{
    const SfxItemSet& rSrcSet = mpEditEngine->GetDefaults();
    rDestEngine.SetDefaultItem( rSrcSet.Get( EE_CHAR_LANGUAGE ));
    rDestEngine.SetDefaultItem( rSrcSet.Get( EE_CHAR_LANGUAGE_CJK ));
    rDestEngine.SetDefaultItem( rSrcSet.Get( EE_CHAR_LANGUAGE_CTL ));
}

static void lcl_SetTopSelection( EditView* pEditView, ESelection& rSel )
{
    OSL_ENSURE( rSel.nStartPara==0 && rSel.nEndPara==0, "SetTopSelection: Para != 0" );

    EditEngine* pEngine = pEditView->GetEditEngine();
    sal_Int32 nCount = pEngine->GetParagraphCount();
    if (nCount > 1)
    {
        sal_Int32 nParLen = pEngine->GetTextLen(rSel.nStartPara);
        while (rSel.nStartPos > nParLen && rSel.nStartPara+1 < nCount)
        {
            rSel.nStartPos -= nParLen + 1; // Including space from line break
            nParLen = pEngine->GetTextLen(++rSel.nStartPara);
        }

        nParLen = pEngine->GetTextLen(rSel.nEndPara);
        while (rSel.nEndPos > nParLen && rSel.nEndPara+1 < nCount)
        {
            rSel.nEndPos -= nParLen + 1; // Including space from line break
            nParLen = pEngine->GetTextLen(++rSel.nEndPara);
        }
    }

    ESelection aSel = pEditView->GetSelection();

    if (   rSel.nStartPara != aSel.nStartPara || rSel.nEndPara != aSel.nEndPara
        || rSel.nStartPos  != aSel.nStartPos  || rSel.nEndPos  != aSel.nEndPos )
        pEditView->SetSelection( rSel );
}

void ScInputHandler::SyncViews( const EditView* pSourceView )
{
    if (pSourceView)
    {
        bool bSelectionForTopView = false;
        if (pTopView && pTopView != pSourceView)
            bSelectionForTopView = true;
        bool bSelectionForTableView = false;
        if (pTableView && pTableView != pSourceView)
            bSelectionForTableView = true;
        if (bSelectionForTopView || bSelectionForTableView)
        {
            ESelection aSel(pSourceView->GetSelection());
            if (bSelectionForTopView)
                pTopView->SetSelection(aSel);
            if (bSelectionForTableView)
                lcl_SetTopSelection(pTableView, aSel);
        }
    }
    // Only sync selection from topView if we are actually editing there
    else if (pTopView && pTableView)
    {
        ESelection aSel(pTopView->GetSelection());
        lcl_SetTopSelection( pTableView, aSel );
    }
}

IMPL_LINK_NOARG(ScInputHandler, ModifyHdl, LinkParamNone*, void)
{
    if ( !bInOwnChange && ( eMode==SC_INPUT_TYPE || eMode==SC_INPUT_TABLE ) &&
         mpEditEngine && mpEditEngine->IsUpdateLayout() && pInputWin )
    {
        // Update input line from ModifyHdl for changes that are not
        // wrapped by DataChanging/DataChanged calls (like Drag&Drop)
        OUString aText(ScEditUtil::GetMultilineString(*mpEditEngine));
        lcl_RemoveTabs(aText);
        pInputWin->SetTextString(aText);
    }
}

/**
 * @return true means new view created
 */
bool ScInputHandler::DataChanging( sal_Unicode cTyped, bool bFromCommand )
{
    if (pActiveViewSh)
        pActiveViewSh->GetViewData().SetPasteMode( ScPasteFlags::NONE );
    bInOwnChange = true; // disable ModifyHdl (reset in DataChanged)

    if ( eMode == SC_INPUT_NONE )
        return StartTable( cTyped, bFromCommand, false, nullptr );
    else
        return false;
}

void ScInputHandler::DataChanged( bool bFromTopNotify, bool bSetModified )
{
    ImplCreateEditEngine();

    if (eMode==SC_INPUT_NONE)
        eMode = SC_INPUT_TYPE;

    if ( eMode == SC_INPUT_TOP && pTopView && !bFromTopNotify )
    {
        //  table EditEngine is formatted below, input line needs formatting after paste
        //  #i20282# not when called from the input line's modify handler
        pTopView->GetEditEngine()->QuickFormatDoc( true );

        //  #i23720# QuickFormatDoc hides the cursor, but can't show it again because it
        //  can't safely access the EditEngine's current view, so the cursor has to be
        //  shown again here.
        pTopView->ShowCursor();
    }

    if (bSetModified)
        bModified = true;
    bSelIsRef = false;

    if ( pRangeFindList && !bInRangeUpdate )
        RemoveRangeFinder(); // Delete attributes and labels

    UpdateParenthesis(); // Highlight parentheses anew

    if (eMode==SC_INPUT_TYPE || eMode==SC_INPUT_TABLE)
    {
        OUString aText;
        if (pInputWin)
            aText = ScEditUtil::GetMultilineString(*mpEditEngine);
        else
            aText = GetEditText(mpEditEngine.get());
        lcl_RemoveTabs(aText);

        if ( pInputWin )
            pInputWin->SetTextString( aText );

        if (comphelper::LibreOfficeKit::isActive())
        {
            if (pActiveViewSh)
                pActiveViewSh->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_FORMULA, aText.toUtf8().getStr());
        }
    }

    // If the cursor is before the end of a paragraph, parts are being pushed to
    // the right (independently from the eMode) -> Adapt View!
    // If the cursor is at the end, the StatusHandler of the ViewData is sufficient.
    //
    // First make sure the status handler is called now if the cursor
    // is outside the visible area
    mpEditEngine->QuickFormatDoc();

    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (pActiveView && pActiveViewSh)
    {
        ScViewData& rViewData = pActiveViewSh->GetViewData();

        bool bNeedGrow = ( rViewData.GetEditAdjust() != SvxAdjust::Left ); // Always right-aligned
        if (!bNeedGrow)
        {
            // Cursor before the end?
            ESelection aSel = pActiveView->GetSelection();
            aSel.Adjust();
            bNeedGrow = ( aSel.nEndPos != mpEditEngine->GetTextLen(aSel.nEndPara) );
        }
        if (!bNeedGrow)
        {
            bNeedGrow = rViewData.GetDocument().IsLayoutRTL( rViewData.GetTabNo() );
        }
        if (bNeedGrow)
        {
            // Adjust inplace view
            rViewData.EditGrowY();
            rViewData.EditGrowX();
        }
    }

    UpdateFormulaMode();
    bTextValid = false; // Changes only in the EditEngine
    bInOwnChange = false;
}

void ScInputHandler::UpdateFormulaMode()
{
    SfxApplication* pSfxApp = SfxGetpApp();

    bool bIsFormula = !bProtected && mpEditEngine->GetParagraphCount() == 1;
    if (bIsFormula)
    {
        const OUString& rText = mpEditEngine->GetText(0);
        bIsFormula = !rText.isEmpty() &&
            (rText[0] == '=' || rText[0] == '+' || rText[0] == '-');
    }

    if ( bIsFormula )
    {
        if (!bFormulaMode)
        {
            bFormulaMode = true;
            pRefViewSh = pActiveViewSh;
            pSfxApp->Broadcast( SfxHint( SfxHintId::ScRefModeChanged ) );
            ScModule* pMod = SC_MOD();
            pMod->SetRefInputHdl(this);
            if (pInputWin)
                pInputWin->SetFormulaMode(true);

            // in LOK, we always need to perform the GetFormulaData() call so
            // that the formula insertion works
            if (comphelper::LibreOfficeKit::isActive() || pMod->GetAppOptions().GetAutoComplete())
                GetFormulaData();

            UpdateParenthesis();
            UpdateAutoCorrFlag();
        }
    }
    else // Deactivate
    {
        if (bFormulaMode)
        {
            ShowRefFrame();
            bFormulaMode = false;
            pRefViewSh = nullptr;
            pSfxApp->Broadcast( SfxHint( SfxHintId::ScRefModeChanged ) );
            SC_MOD()->SetRefInputHdl(nullptr);
            if (pInputWin)
                pInputWin->SetFormulaMode(false);
            UpdateAutoCorrFlag();
        }
    }
}

void ScInputHandler::ShowRefFrame()
{
    // Modifying pActiveViewSh here would interfere with the bInEnterHandler / bRepeat
    // checks in NotifyChange, and lead to keeping the wrong value in pActiveViewSh.
    // A local variable is used instead.
    ScTabViewShell* pVisibleSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
    if ( !(pRefViewSh && pRefViewSh != pVisibleSh) )
        return;

    bool bFound = false;
    SfxViewFrame* pRefFrame = pRefViewSh->GetViewFrame();
    SfxViewFrame* pOneFrame = SfxViewFrame::GetFirst();
    while ( pOneFrame && !bFound )
    {
        if ( pOneFrame == pRefFrame )
            bFound = true;
        pOneFrame = SfxViewFrame::GetNext( *pOneFrame );
    }

    if (bFound)
    {
        // We count on Activate working synchronously here
        // (pActiveViewSh is set while doing so)
        pRefViewSh->SetActive(); // Appear and SetViewFrame

        //  pLastState is set correctly in the NotifyChange from the Activate
    }
    else
    {
        OSL_FAIL("ViewFrame for reference input is not here anymore");
    }
}

void ScInputHandler::RemoveSelection()
{
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (!pActiveView)
        return;

    ESelection aSel = pActiveView->GetSelection();
    aSel.nStartPara = aSel.nEndPara;
    aSel.nStartPos  = aSel.nEndPos;
    if (pTableView)
        pTableView->SetSelection( aSel );
    if (pTopView)
        pTopView->SetSelection( aSel );
}

void ScInputHandler::InvalidateAttribs()
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    SfxBindings& rBindings = pViewFrm->GetBindings();

    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );

    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ATTR_CHAR_OVERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );

    rBindings.Invalidate( SID_HYPERLINK_GETLINK );

    rBindings.Invalidate( SID_ATTR_CHAR_KERNING );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_ATTR_CHAR_STRIKEOUT );
    rBindings.Invalidate( SID_ATTR_CHAR_SHADOWED );

    rBindings.Invalidate( SID_SAVEDOC );
    rBindings.Invalidate( SID_DOC_MODIFIED );
}

// --------------- public methods --------------------------------------------

void ScInputHandler::SetMode( ScInputMode eNewMode, const OUString* pInitText, ScEditEngineDefaulter* pTopEngine )
{
    if ( eMode == eNewMode )
        return;

    ImplCreateEditEngine();

    if (bProtected)
    {
        eMode = SC_INPUT_NONE;
        StopInputWinEngine( true );
        if (pActiveViewSh)
            pActiveViewSh->GetActiveWin()->GrabFocus();
        return;
    }

    if (eNewMode != SC_INPUT_NONE && pActiveViewSh)
        // Disable paste mode when edit mode starts.
        pActiveViewSh->GetViewData().SetPasteMode( ScPasteFlags::NONE );

    bInOwnChange = true; // disable ModifyHdl (reset below)

    ScInputMode eOldMode = eMode;
    eMode = eNewMode;
    if (eOldMode == SC_INPUT_TOP && eNewMode != eOldMode)
        StopInputWinEngine( false );

    if (eMode==SC_INPUT_TOP || eMode==SC_INPUT_TABLE)
    {
        if (eOldMode == SC_INPUT_NONE) // not if switching between modes
        {
            if (StartTable(0, false, eMode == SC_INPUT_TABLE, pTopEngine))
            {
                if (pActiveViewSh)
                    pActiveViewSh->GetViewData().GetDocShell()->PostEditView( mpEditEngine.get(), aCursorPos );
            }
        }

        if (pInitText)
        {
            mpEditEngine->SetTextCurrentDefaults(*pInitText);
            bModified = true;
        }

        sal_Int32 nPara = mpEditEngine->GetParagraphCount()-1;
        sal_Int32 nLen = mpEditEngine->GetText(nPara).getLength();
        sal_uInt16 nCount = mpEditEngine->GetViewCount();

        for (sal_uInt16 i=0; i<nCount; i++)
        {
            if ( eMode == SC_INPUT_TABLE && eOldMode == SC_INPUT_TOP )
            {
                // Keep Selection
            }
            else
            {
                mpEditEngine->GetView(i)->
                    SetSelection( ESelection( nPara, nLen, nPara, nLen ) );
            }
            mpEditEngine->GetView(i)->ShowCursor(false);
        }
    }

    UpdateActiveView();
    if (eMode==SC_INPUT_TABLE || eMode==SC_INPUT_TYPE)
    {
        if (pTableView)
            pTableView->SetEditEngineUpdateLayout(true);
    }
    else
    {
        if (pTopView)
            pTopView->SetEditEngineUpdateLayout(true);
    }

    if (eNewMode != eOldMode)
        UpdateFormulaMode();

    bInOwnChange = false;
}

/**
 * @return true if rString only contains digits (no autocorrect then)
 */
static bool lcl_IsNumber(const OUString& rString)
{
    sal_Int32 nLen = rString.getLength();
    for (sal_Int32 i=0; i<nLen; i++)
    {
        sal_Unicode c = rString[i];
        if ( c < '0' || c > '9' )
            return false;
    }
    return true;
}

static void lcl_SelectionToEnd( EditView* pView )
{
    if ( pView )
    {
        EditEngine* pEngine = pView->GetEditEngine();
        sal_Int32 nParCnt = pEngine->GetParagraphCount();
        if ( nParCnt == 0 )
            nParCnt = 1;
        ESelection aSel( nParCnt-1, pEngine->GetTextLen(nParCnt-1) ); // empty selection, cursor at the end
        pView->SetSelection( aSel );
    }
}

void ScInputHandler::EnterHandler( ScEnterMode nBlockMode )
{
    if (!mbDocumentDisposing && comphelper::LibreOfficeKit::isActive()
        && pActiveViewSh != SfxViewShell::Current())
        return;

    // Macro calls for validity can cause a lot of problems, so inhibit
    // nested calls of EnterHandler().
    if (bInEnterHandler) return;
    bInEnterHandler = true;
    bInOwnChange = true; // disable ModifyHdl (reset below)
    mbPartialPrefix = false;

    ImplCreateEditEngine();

    bool bMatrix = ( nBlockMode == ScEnterMode::MATRIX );

    SfxApplication* pSfxApp     = SfxGetpApp();
    std::unique_ptr<EditTextObject> pObject;
    std::unique_ptr<ScPatternAttr> pCellAttrs;
    bool            bForget     = false; // Remove due to validity?

    OUString aString = GetEditText(mpEditEngine.get());
    OUString aPreAutoCorrectString(aString);
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    if (bModified && pActiveView && !aString.isEmpty() && !lcl_IsNumber(aString))
    {
        if (pColumnData && miAutoPosColumn != pColumnData->end())
        {
            // #i47125# If AutoInput appended something, do the final AutoCorrect
            // with the cursor at the end of the input.
            lcl_SelectionToEnd(pTopView);
            lcl_SelectionToEnd(pTableView);
        }

        vcl::Window* pFrameWin = pActiveViewSh ? pActiveViewSh->GetFrameWin() : nullptr;

        if (pTopView)
            pTopView->CompleteAutoCorrect(); // CompleteAutoCorrect for both Views
        if (pTableView)
            pTableView->CompleteAutoCorrect(pFrameWin);
        aString = GetEditText(mpEditEngine.get());
    }
    lcl_RemoveTabs(aString);
    lcl_RemoveTabs(aPreAutoCorrectString);

    // Test if valid (always with simple string)
    if (bModified && nValidation && pActiveViewSh)
    {
        ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocument();
        const ScValidationData* pData = rDoc.GetValidationEntry( nValidation );
        if (pData && pData->HasErrMsg())
        {
            // #i67990# don't use pLastPattern in EnterHandler
            const ScPatternAttr* pPattern = rDoc.GetPattern( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab() );

            bool bOk;

            if (pData->GetDataMode() == SC_VALID_CUSTOM)
            {
                bOk = pData->IsDataValidCustom( aString, *pPattern, aCursorPos,  ScValidationData::CustomValidationPrivateAccess() );
            }
            else
            {
                bOk = pData->IsDataValid( aString, *pPattern, aCursorPos );
            }

            if (!bOk)
            {
                pActiveViewSh->StopMarking();   // (the InfoBox consumes the MouseButtonUp)

                // tdf#125917 Release the grab that a current mouse-down event being handled
                // by ScTabView has put on the mouse via its SelectionEngine.
                // Otherwise the warning box cannot interact with the mouse
                if (ScTabView* pView = pActiveViewSh->GetViewData().GetView())
                {
                    if (ScViewSelectionEngine* pSelEngine = pView->GetSelEngine())
                        pSelEngine->ReleaseMouse();
                }

                if (pData->DoError(pActiveViewSh->GetFrameWeld(), aString, aCursorPos))
                    bForget = true;                 // Do not take over input
            }
        }
    }

    // Check for input into DataPilot table
    if ( bModified && pActiveViewSh && !bForget )
    {
        ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocument();
        ScDPObject* pDPObj = rDoc.GetDPAtCursor( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab() );
        if ( pDPObj )
        {
            // Any input within the DataPilot table is either a valid renaming
            // or an invalid action - normal cell input is always aborted
            pActiveViewSh->DataPilotInput( aCursorPos, aString );
            bForget = true;
        }
    }

    std::vector<editeng::MisspellRanges> aMisspellRanges;
    mpEditEngine->CompleteOnlineSpelling();
    bool bSpellErrors = !bFormulaMode && mpEditEngine->HasOnlineSpellErrors();
    if ( bSpellErrors )
    {
        //  #i3820# If the spell checker flags numerical input as error,
        //  it still has to be treated as number, not EditEngine object.
        if ( pActiveViewSh )
        {
            ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocument();
            // #i67990# don't use pLastPattern in EnterHandler
            const ScPatternAttr* pPattern = rDoc.GetPattern( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab() );
            if (pPattern)
            {
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                // without conditional format, as in ScColumn::SetString
                sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter );
                double nVal;
                if ( pFormatter->IsNumberFormat( aString, nFormat, nVal ) )
                {
                    bSpellErrors = false;       // ignore the spelling errors
                }
            }
        }
    }

    //  After RemoveAdjust, the EditView must not be repainted (has wrong font size etc).
    //  SetUpdateLayout must come after CompleteOnlineSpelling.
    //  The view is hidden in any case below (Broadcast).
    mpEditEngine->SetUpdateLayout( false );

    if ( bModified && !bForget ) // What is being entered (text/object)?
    {
        sal_Int32 nParCnt = mpEditEngine->GetParagraphCount();
        if ( nParCnt == 0 )
            nParCnt = 1;

        bool bUniformAttribs = true;
        SfxItemSet aPara1Attribs = mpEditEngine->GetAttribs(0, 0, mpEditEngine->GetTextLen(0));
        for (sal_Int32 nPara = 1; nPara < nParCnt; ++nPara)
        {
            SfxItemSet aPara2Attribs = mpEditEngine->GetAttribs(nPara, 0, mpEditEngine->GetTextLen(nPara));
            if (!(aPara1Attribs == aPara2Attribs))
            {
                // Paragraph format different from that of the 1st paragraph.
                bUniformAttribs = false;
                break;
            }
        }

        ESelection aSel( 0, 0, nParCnt-1, mpEditEngine->GetTextLen(nParCnt-1) );
        SfxItemSet aOldAttribs = mpEditEngine->GetAttribs( aSel );
        const SfxPoolItem* pItem = nullptr;

        // Find common (cell) attributes before RemoveAdjust
        if ( pActiveViewSh && bUniformAttribs )
        {
            std::optional<SfxItemSet> pCommonAttrs;
            for (sal_uInt16 nId = EE_CHAR_START; nId <= EE_CHAR_END; nId++)
            {
                SfxItemState eState = aOldAttribs.GetItemState( nId, false, &pItem );
                if ( eState == SfxItemState::SET &&
                        nId != EE_CHAR_ESCAPEMENT && nId != EE_CHAR_PAIRKERNING &&
                        nId != EE_CHAR_KERNING && nId != EE_CHAR_XMLATTRIBS &&
                            *pItem != pEditDefaults->Get(nId) )
                {
                    if ( !pCommonAttrs )
                        pCommonAttrs.emplace( mpEditEngine->GetEmptyItemSet() );
                    pCommonAttrs->Put( *pItem );
                }
            }

            if ( pCommonAttrs )
            {
                ScDocument& rDoc = pActiveViewSh->GetViewData().GetDocument();
                pCellAttrs = std::make_unique<ScPatternAttr>(rDoc.GetPool());
                pCellAttrs->GetFromEditItemSet( &*pCommonAttrs );
            }
        }

        // Clear ParaAttribs (including adjustment)
        RemoveAdjust();

        bool bAttrib = false; // Formatting present?

        //  check if EditObject is needed
        if (nParCnt > 1)
            bAttrib = true;
        else
        {
            for (sal_uInt16 nId = EE_CHAR_START; nId <= EE_CHAR_END && !bAttrib; nId++)
            {
                SfxItemState eState = aOldAttribs.GetItemState( nId, false, &pItem );
                if (eState == SfxItemState::DONTCARE)
                    bAttrib = true;
                else if (eState == SfxItemState::SET)
                {
                    // Keep same items in EditEngine as in ScEditAttrTester
                    if ( nId == EE_CHAR_ESCAPEMENT || nId == EE_CHAR_PAIRKERNING ||
                         nId == EE_CHAR_KERNING || nId == EE_CHAR_XMLATTRIBS )
                    {
                        if ( *pItem != pEditDefaults->Get(nId) )
                            bAttrib = true;
                    }
                }
            }

            // Contains fields?
            SfxItemState eFieldState = aOldAttribs.GetItemState( EE_FEATURE_FIELD, false );
            if ( eFieldState == SfxItemState::DONTCARE || eFieldState == SfxItemState::SET )
                bAttrib = true;

            // Not converted characters?
            SfxItemState eConvState = aOldAttribs.GetItemState( EE_FEATURE_NOTCONV, false );
            if ( eConvState == SfxItemState::DONTCARE || eConvState == SfxItemState::SET )
                bAttrib = true;

            // Always recognize formulas as formulas
            // We still need the preceding test due to cell attributes
        }

        if (bSpellErrors)
            mpEditEngine->GetAllMisspellRanges(aMisspellRanges);

        if (bMatrix)
            bAttrib = false;

        if (bAttrib)
        {
            mpEditEngine->ClearSpellErrors();
            pObject = mpEditEngine->CreateTextObject();
        }
        else if (SC_MOD()->GetAppOptions().GetAutoComplete()) // Adjust Upper/Lower case
        {
            // Perform case-matching only when the typed text is partial.
            if (pColumnData && aAutoSearch.getLength() < aString.getLength())
                aString = getExactMatch(*pColumnData, aString);
        }
    }

    // Don't rely on ShowRefFrame switching the active view synchronously
    // execute the function directly on the correct view's bindings instead
    // pRefViewSh is reset in ShowRefFrame - get pointer before ShowRefFrame call
    ScTabViewShell* pExecuteSh = pRefViewSh ? pRefViewSh : pActiveViewSh;

    if (bFormulaMode)
    {
        ShowRefFrame();

        if (pExecuteSh)
        {
            pExecuteSh->SetTabNo(aCursorPos.Tab());
            pExecuteSh->ActiveGrabFocus();
        }

        bFormulaMode = false;
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScRefModeChanged ) );
        SC_MOD()->SetRefInputHdl(nullptr);
        if (pInputWin)
            pInputWin->SetFormulaMode(false);
        UpdateAutoCorrFlag();
    }
    pRefViewSh = nullptr; // Also without FormulaMode due to FunctionsAutoPilot
    DeleteRangeFinder();
    ResetAutoPar();

    bool bOldMod = bModified;

    bModified = false;
    bSelIsRef = false;
    eMode     = SC_INPUT_NONE;
    StopInputWinEngine(true);

    // Text input (through number formats) or ApplySelectionPattern modify
    // the cell's attributes, so pLastPattern is no longer valid
    pLastPattern = nullptr;

    if (bOldMod && !bProtected && !bForget)
    {
        bool bInsertPreCorrectedString = true;
        // No typographic quotes in formulas
        if (aString.startsWith("="))
        {
            SvxAutoCorrect* pAuto = SvxAutoCorrCfg::Get().GetAutoCorrect();
            if ( pAuto )
            {
                bInsertPreCorrectedString = false;
                OUString aReplace(pAuto->GetStartDoubleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::getLocaleData().getDoubleQuotationMarkStart();
                if( aReplace != "\"" )
                    aString = aString.replaceAll( aReplace, "\"" );

                aReplace = OUString(pAuto->GetEndDoubleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::getLocaleData().getDoubleQuotationMarkEnd();
                if( aReplace != "\"" )
                    aString = aString.replaceAll( aReplace, "\"" );

                aReplace = OUString(pAuto->GetStartSingleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::getLocaleData().getQuotationMarkStart();
                if( aReplace != "'" )
                    aString = aString.replaceAll( aReplace, "'" );

                aReplace = OUString(pAuto->GetEndSingleQuote());
                if( aReplace.isEmpty() )
                    aReplace = ScGlobal::getLocaleData().getQuotationMarkEnd();
                if( aReplace != "'" )
                    aString = aString.replaceAll( aReplace, "'");
            }
        }

        pSfxApp->Broadcast( SfxHint( SfxHintId::ScKillEditViewNoPaint ) );

        if ( pExecuteSh )
        {
            SfxBindings& rBindings = pExecuteSh->GetViewFrame()->GetBindings();

            sal_uInt16 nId = FID_INPUTLINE_ENTER;
            if ( nBlockMode == ScEnterMode::BLOCK )
                nId = FID_INPUTLINE_BLOCK;
            else if ( nBlockMode == ScEnterMode::MATRIX )
                nId = FID_INPUTLINE_MATRIX;

            const SfxPoolItem* aArgs[2];
            aArgs[1] = nullptr;

            if ( bInsertPreCorrectedString && aString != aPreAutoCorrectString )
            {
                ScInputStatusItem aItem(FID_INPUTLINE_STATUS,
                                       aCursorPos, aCursorPos, aCursorPos,
                                       aPreAutoCorrectString, pObject.get());
                aArgs[0] = &aItem;
                rBindings.Execute(nId, aArgs);
            }

            ScInputStatusItem aItemCorrected(FID_INPUTLINE_STATUS,
                                             aCursorPos, aCursorPos, aCursorPos,
                                             aString, pObject.get());
            if ( !aMisspellRanges.empty() )
                aItemCorrected.SetMisspellRanges(&aMisspellRanges);

            aArgs[0] = &aItemCorrected;
            rBindings.Execute(nId, aArgs);
        }

        pLastState.reset(); // pLastState still contains the old text
    }
    else
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScKillEditView ) );

    if ( bOldMod && pExecuteSh && pCellAttrs && !bForget )
    {
        // Combine with input?
        pExecuteSh->ApplySelectionPattern( *pCellAttrs, true );
        pExecuteSh->AdjustBlockHeight();
    }

    HideTip();
    HideTipBelow();

    nFormSelStart = nFormSelEnd = 0;
    aFormText.clear();

    bInOwnChange = false;
    bInEnterHandler = false;
}

void ScInputHandler::CancelHandler()
{
    bInOwnChange = true; // Also without FormulaMode due to FunctionsAutoPilot

    ImplCreateEditEngine();

    bModified = false;
    mbPartialPrefix = false;

    // Don't rely on ShowRefFrame switching the active view synchronously
    // execute the function directly on the correct view's bindings instead
    // pRefViewSh is reset in ShowRefFrame - get pointer before ShowRefFrame call
    ScTabViewShell* pExecuteSh = pRefViewSh ? pRefViewSh : pActiveViewSh;

    if (bFormulaMode)
    {
        ShowRefFrame();
        if (pExecuteSh)
        {
            pExecuteSh->SetTabNo(aCursorPos.Tab());
            pExecuteSh->ActiveGrabFocus();
        }
        bFormulaMode = false;
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScRefModeChanged ) );
        SC_MOD()->SetRefInputHdl(nullptr);
        if (pInputWin)
            pInputWin->SetFormulaMode(false);
        UpdateAutoCorrFlag();
    }
    pRefViewSh = nullptr; // Also without FormulaMode due to FunctionsAutoPilot
    DeleteRangeFinder();
    ResetAutoPar();

    eMode = SC_INPUT_NONE;
    StopInputWinEngine( true );
    if (pExecuteSh)
        pExecuteSh->StopEditShell();

    aCursorPos.Set(pExecuteSh->GetViewData().GetDocument().MaxCol()+1,0,0); // Invalid flag
    mpEditEngine->SetTextCurrentDefaults(OUString());

    if ( !pLastState && pExecuteSh )
        pExecuteSh->UpdateInputHandler( true );  // Update status again
    else
        NotifyChange( pLastState.get(), true );

    nFormSelStart = nFormSelEnd = 0;
    aFormText.clear();

    bInOwnChange = false;

    if ( comphelper::LibreOfficeKit::isActive() && pExecuteSh )
    {
        // Clear
        std::vector<ReferenceMark> aReferenceMarks;
        ScInputHandler::SendReferenceMarks( pActiveViewSh, aReferenceMarks );
    }
}

bool ScInputHandler::IsModalMode( const SfxObjectShell* pDocSh )
{
    // References to unnamed document; that doesn't work
    return bFormulaMode && pRefViewSh
            && pRefViewSh->GetViewData().GetDocument().GetDocumentShell() != pDocSh
            && !pDocSh->HasName();
}

void ScInputHandler::AddRefEntry()
{
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // E.g. FillMode

    DataChanging();                         // Cannot be new

    RemoveSelection();
    OUString aText = GetEditText(mpEditEngine.get());
    sal_Unicode cLastChar = 0;
    sal_Int32 nPos = aText.getLength() - 1;
    while (nPos >= 0) //checking space
    {
        cLastChar = aText[nPos];
        if (cLastChar != ' ')
            break;
        --nPos;
    }

    bool bAppendSeparator = (cLastChar != '(' && cLastChar != cSep && cLastChar != '=');
    if (bAppendSeparator)
    {
        if (pTableView)
            pTableView->InsertText( OUString(cSep) );
        if (pTopView)
            pTopView->InsertText( OUString(cSep) );
    }

    DataChanged();
}

void ScInputHandler::SetReference( const ScRange& rRef, const ScDocument& rDoc )
{
    HideTip();

    const ScDocument* pThisDoc = nullptr;
    if (pRefViewSh)
        pThisDoc = &pRefViewSh->GetViewData().GetDocument();
    bool bOtherDoc = (pThisDoc != &rDoc);
    if (bOtherDoc && !rDoc.GetDocumentShell()->HasName())
    {
        // References to unnamed document; that doesn't work
        // SetReference should not be called, then
        return;
    }
    if (!pThisDoc)
        pThisDoc = &rDoc;

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // E.g. FillMode

    // Never overwrite the "="!
    EditView* pActiveView = pTopView ? pTopView : pTableView;
    ESelection aSel = pActiveView->GetSelection();
    aSel.Adjust();
    if ( aSel.nStartPara == 0 && aSel.nStartPos == 0 )
        return;

    DataChanging();                         // Cannot be new

    // Turn around selection if backwards.
    if (pTableView)
    {
        ESelection aTabSel = pTableView->GetSelection();
        if (aTabSel.nStartPos > aTabSel.nEndPos && aTabSel.nStartPara == aTabSel.nEndPara)
        {
            aTabSel.Adjust();
            pTableView->SetSelection(aTabSel);
        }
    }
    if (pTopView)
    {
        ESelection aTopSel = pTopView->GetSelection();
        if (aTopSel.nStartPos > aTopSel.nEndPos && aTopSel.nStartPara == aTopSel.nEndPara)
        {
            aTopSel.Adjust();
            pTopView->SetSelection(aTopSel);
        }
    }

    // Create string from reference, in the syntax of the document being edited.
    OUString aRefStr;
    const ScAddress::Details aAddrDetails( *pThisDoc, aCursorPos );
    if (bOtherDoc)
    {
        // Reference to other document
        OSL_ENSURE(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

        // Always 3D and absolute.
        OUString aTmp(rRef.Format(rDoc, ScRefFlags::VALID | ScRefFlags::TAB_ABS_3D, aAddrDetails));

        SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
        // #i75893# convert escaped URL of the document to something user friendly
        OUString aFileName = pObjSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );

        switch(aAddrDetails.eConv)
        {
             case formula::FormulaGrammar::CONV_XL_A1 :
             case formula::FormulaGrammar::CONV_XL_OOX :
             case formula::FormulaGrammar::CONV_XL_R1C1 :
                         aRefStr = "[\'" + aFileName + "']";
                         break;
             case formula::FormulaGrammar::CONV_OOO :
             default:
                         aRefStr = "\'" + aFileName + "'#";
                         break;
        }
        aRefStr += aTmp;
    }
    else
    {
        if ( rRef.aStart.Tab() != aCursorPos.Tab() ||
             rRef.aStart.Tab() != rRef.aEnd.Tab() )
            // pointer-selected => absolute sheet reference
            aRefStr = rRef.Format(rDoc, ScRefFlags::VALID | ScRefFlags::TAB_ABS_3D, aAddrDetails);
        else
            aRefStr = rRef.Format(rDoc, ScRefFlags::VALID, aAddrDetails);
    }
    bool bLOKShowSelect = true;
    if(comphelper::LibreOfficeKit::isActive() && pRefViewSh->GetViewData().GetRefTabNo() != pRefViewSh->GetViewData().GetTabNo())
        bLOKShowSelect = false;

    if (pTableView || pTopView)
    {
        if (pTableView)
            pTableView->InsertText( aRefStr, true, bLOKShowSelect );
        if (pTopView)
            pTopView->InsertText( aRefStr, true, bLOKShowSelect );

        DataChanged();
    }

    bSelIsRef = true;
}

void ScInputHandler::InsertFunction( const OUString& rFuncName, bool bAddPar )
{
    if ( eMode == SC_INPUT_NONE )
    {
        OSL_FAIL("InsertFunction, not during input mode");
        return;
    }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // E.g. FillMode

    DataChanging();                         // Cannot be new

    OUString aText = rFuncName;
    if (bAddPar)
        aText += "()";

    if (pTableView)
    {
        pTableView->InsertText( aText );
        if (bAddPar)
        {
            ESelection aSel = pTableView->GetSelection();
            --aSel.nStartPos;
            --aSel.nEndPos;
            pTableView->SetSelection(aSel);
        }
    }
    if (pTopView)
    {
        pTopView->InsertText( aText );
        if (bAddPar)
        {
            ESelection aSel = pTopView->GetSelection();
            --aSel.nStartPos;
            --aSel.nEndPos;
            pTopView->SetSelection(aSel);
        }
    }

    DataChanged();

    if (bAddPar)
        AutoParAdded();
}

void ScInputHandler::ClearText()
{
    if ( eMode == SC_INPUT_NONE )
    {
        OSL_FAIL("ClearText, not during input mode");
        return;
    }

    UpdateActiveView();
    if (!pTableView && !pTopView)
        return;                             // E.g. FillMode

    DataChanging();                         // Cannot be new

    if (pTableView)
    {
        pTableView->GetEditEngine()->SetText( "" );
        pTableView->SetSelection( ESelection(0,0, 0,0) );
    }
    if (pTopView)
    {
        pTopView->GetEditEngine()->SetText( "" );
        pTopView->SetSelection( ESelection(0,0, 0,0) );
    }

    DataChanged();
}

bool ScInputHandler::KeyInput( const KeyEvent& rKEvt, bool bStartEdit /* = false */ )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nModi  = aCode.GetModifier();
    bool bShift   = aCode.IsShift();
    bool bControl = aCode.IsMod1();
    bool bAlt     = aCode.IsMod2();
    sal_uInt16 nCode  = aCode.GetCode();
    sal_Unicode nChar = rKEvt.GetCharCode();

    if (bAlt && !bControl && nCode != KEY_RETURN)
        // Alt-Return and Alt-Ctrl-* are accepted. Everything else with ALT are not.
        return false;

    // There is a partial autocomplete suggestion.
    // Allow its completion with right arrow key (without modifiers).
    if (mbPartialPrefix && nCode == KEY_RIGHT && !bControl && !bShift && !bAlt &&
        (pTopView || pTableView))
    {
        if (pTopView)
            pTopView->PostKeyEvent(KeyEvent(0, css::awt::Key::MOVE_TO_END_OF_PARAGRAPH));
        if (pTableView)
            pTableView->PostKeyEvent(KeyEvent(0, css::awt::Key::MOVE_TO_END_OF_PARAGRAPH));

        mbPartialPrefix = false;

        // Indicate that this event has been consumed and ScTabViewShell should not act on this.
        return true;
    }

    if (!bControl && nCode == KEY_TAB)
    {
        // Normal TAB moves the cursor right.
        EnterHandler();

        if (pActiveViewSh)
            pActiveViewSh->FindNextUnprot( bShift, true );
        return true;
    }

    bool bInputLine = ( eMode==SC_INPUT_TOP );

    bool bUsed = false;
    bool bSkip = false;
    bool bDoEnter = false;

    switch ( nCode )
    {
        case KEY_RETURN:
            // New line when in the input line and Shift/Ctrl-Enter is pressed,
            // or when in a cell and Ctrl-Enter is pressed.
            if ((pInputWin && bInputLine && bControl != bShift) || (!bInputLine && bControl && !bShift))
            {
                bDoEnter = true;
            }
            else if (nModi == 0 && nTipVisible && pFormulaData && miAutoPosFormula != pFormulaData->end())
            {
                PasteFunctionData();
                bUsed = true;
            }
            else if ( nModi == 0 && nTipVisible && !aManualTip.isEmpty() )
            {
                PasteManualTip();
                bUsed = true;
            }
            else
            {
                ScEnterMode nMode = ScEnterMode::NORMAL;
                if ( bShift && bControl )
                    nMode = ScEnterMode::MATRIX;
                else if ( bAlt )
                    nMode = ScEnterMode::BLOCK;
                EnterHandler( nMode );

                if (pActiveViewSh)
                    pActiveViewSh->MoveCursorEnter( bShift && !bControl );

                bUsed = true;
            }
            break;
        case KEY_TAB:
            if (bControl && !bAlt)
            {
                if (pFormulaData && nTipVisible && miAutoPosFormula != pFormulaData->end())
                {
                    // Iterate
                    NextFormulaEntry( bShift );
                    bUsed = true;
                }
                else if (pColumnData && bUseTab)
                {
                    // Iterate through AutoInput entries
                    NextAutoEntry( bShift );
                    bUsed = true;
                }
            }
            break;
        case KEY_ESCAPE:
            if ( nTipVisible )
            {
                HideTip();
                bUsed = true;
            }
            else if( nTipVisibleSec )
            {
                HideTipBelow();
                bUsed = true;
            }
            else if (eMode != SC_INPUT_NONE)
            {
                CancelHandler();
                bUsed = true;
            }
            else
                bSkip = true;
            break;
        case KEY_F2:
            if ( !bShift && !bControl && !bAlt && eMode == SC_INPUT_TABLE )
            {
                eMode = SC_INPUT_TYPE;
                bUsed = true;
            }
            break;
    }

    // Only execute cursor keys if already in EditMode
    // E.g. due to Shift-Ctrl-PageDn (not defined as an accelerator)
    bool bCursorKey = EditEngine::DoesKeyMoveCursor(rKEvt);
    bool bInsKey = ( nCode == KEY_INSERT && !nModi ); // Treat Insert like Cursorkeys
    if ( !bUsed && !bSkip && ( bDoEnter || EditEngine::DoesKeyChangeText(rKEvt) ||
                    ( eMode != SC_INPUT_NONE && ( bCursorKey || bInsKey ) ) ) )
    {
        HideTip();
        HideTipBelow();

        if (bSelIsRef)
        {
            RemoveSelection();
            bSelIsRef = false;
        }

        UpdateActiveView();
        bool bNewView = DataChanging( nChar );

        if (bProtected)                             // Protected cell?
            bUsed = true;                           // Don't forward KeyEvent
        else                                        // Changes allowed
        {
            if (bNewView )                          // Create anew
            {
                if (pActiveViewSh)
                    pActiveViewSh->GetViewData().GetDocShell()->PostEditView( mpEditEngine.get(), aCursorPos );
                UpdateActiveView();
                if (eMode==SC_INPUT_NONE)
                    if (pTableView || pTopView)
                    {
                        OUString aStrLoP;

                        if (bStartEdit && nCellPercentFormatDecSep != 0 &&
                                ((nChar >= '0' && nChar <= '9') || nChar == '-' || nChar == nCellPercentFormatDecSep))
                        {
                            aStrLoP = "%";
                        }

                        if (pTableView)
                        {
                            pTableView->GetEditEngine()->SetText( aStrLoP );
                            if ( !aStrLoP.isEmpty() )
                                pTableView->SetSelection( ESelection(0,0, 0,0) );   // before the '%'

                            // Don't call SetSelection if the string is empty anyway,
                            // to avoid breaking the bInitial handling in ScViewData::EditGrowY
                        }
                        if (pTopView)
                        {
                            pTopView->GetEditEngine()->SetText( aStrLoP );
                            if ( !aStrLoP.isEmpty() )
                                pTopView->SetSelection( ESelection(0,0, 0,0) );     // before the '%'
                        }
                    }
                SyncViews();
            }

            if (pTableView || pTopView)
            {
                if (bDoEnter)
                {
                    if (pTableView)
                        if( pTableView->PostKeyEvent( KeyEvent( '\r', vcl::KeyCode(KEY_RETURN) ) ) )
                            bUsed = true;
                    if (pTopView)
                        if( pTopView->PostKeyEvent( KeyEvent( '\r', vcl::KeyCode(KEY_RETURN) ) ) )
                            bUsed = true;
                }
                else if ( nAutoPar && nChar == ')' && CursorAtClosingPar() )
                {
                    SkipClosingPar();
                    bUsed = true;
                }
                else
                {
                    if (pTableView)
                    {
                        if (pTopView)
                            pTableView->SetControlWord(pTableView->GetControlWord() | EVControlBits::SINGLELINEPASTE);

                        vcl::Window* pFrameWin = pActiveViewSh ? pActiveViewSh->GetFrameWin() : nullptr;
                        if ( pTableView->PostKeyEvent( rKEvt, pFrameWin ) )
                            bUsed = true;

                        pTableView->SetControlWord(pTableView->GetControlWord() & ~EVControlBits::SINGLELINEPASTE);
                    }
                    if (pTopView)
                    {
                        if ( bUsed && rKEvt.GetKeyCode().GetFunction() == KeyFuncType::CUT )
                            pTopView->DeleteSelected();
                        else if ( pTopView->PostKeyEvent( rKEvt ) )
                            bUsed = true;
                    }
                }

                // AutoInput:
                if ( bUsed && SC_MOD()->GetAppOptions().GetAutoComplete() )
                {
                    bUseTab = false;
                    if (pFormulaData)
                        miAutoPosFormula = pFormulaData->end();     // do not search further
                    if (pColumnData)
                        miAutoPosColumn = pColumnData->end();

                    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
                    if ( nChar && nChar != 8 && nChar != 127 &&     // no 'backspace', no 'delete'
                         KeyFuncType::CUT != eFunc)                      // and no 'CTRL-X'
                    {
                        if (bFormulaMode)
                            UseFormulaData();
                        else
                            UseColData();
                    }
                }

                // When the selection is changed manually or an opening parenthesis
                // is typed, stop overwriting parentheses
                if ( bUsed && nChar == '(' )
                    ResetAutoPar();

                if ( KEY_INSERT == nCode )
                {
                    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                    if (pViewFrm)
                        pViewFrm->GetBindings().Invalidate( SID_ATTR_INSERT );
                }
                if( bUsed && bFormulaMode && ( bCursorKey || bInsKey || nCode == KEY_DELETE || nCode == KEY_BACKSPACE ) )
                {
                    ShowTipCursor();
                }
                if( bUsed && bFormulaMode && nCode == KEY_BACKSPACE )
                {
                    UseFormulaData();
                }

            }

            // #i114511# don't count cursor keys as modification
            bool bSetModified = !bCursorKey;
            DataChanged(false, bSetModified); // also calls UpdateParenthesis()

            // In the LOK case, we want to set the document modified state
            // right away at the start of the edit, so that the content is
            // saved even when the user leaves the document before hitting
            // Enter
            if (comphelper::LibreOfficeKit::isActive() && bSetModified && pActiveViewSh && !pActiveViewSh->GetViewData().GetDocShell()->IsModified())
                pActiveViewSh->GetViewData().GetDocShell()->SetModified();

            InvalidateAttribs();        //! in DataChanged?
        }
    }

    if (pTopView && eMode != SC_INPUT_NONE)
        SyncViews();

    return bUsed;
}

OUString ScInputHandler::GetSurroundingText()
{
    if (eMode != SC_INPUT_NONE)
    {
        UpdateActiveView();
        if (pTableView || pTopView)
        {
            if (pTableView)
                return pTableView->GetSurroundingText();
            else if (pTopView)                      // call only once
                return pTopView->GetSurroundingText();
        }
    }
    return OUString();
}

Selection ScInputHandler::GetSurroundingTextSelection()
{
    if (eMode != SC_INPUT_NONE)
    {
        UpdateActiveView();
        if (pTableView || pTopView)
        {
            if (pTableView)
                return pTableView->GetSurroundingTextSelection();
            else if (pTopView)                      // call only once
                return pTopView->GetSurroundingTextSelection();
        }
    }
    return Selection(0, 0);
}

bool ScInputHandler::DeleteSurroundingText(const Selection& rSelection)
{
    if (eMode != SC_INPUT_NONE)
    {
        UpdateActiveView();
        if (pTableView || pTopView)
        {
            if (pTableView)
                return pTableView->DeleteSurroundingText(rSelection);
            else if (pTopView)                      // call only once
                return pTopView->DeleteSurroundingText(rSelection);
        }
    }
    return false;
}

void ScInputHandler::InputCommand( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::CursorPos )
    {
        // For CommandEventId::CursorPos, do as little as possible, because
        // with remote VCL, even a ShowCursor will generate another event.
        if ( eMode != SC_INPUT_NONE )
        {
            UpdateActiveView();
            if (pTableView || pTopView)
            {
                if (pTableView)
                    pTableView->Command( rCEvt );
                else if (pTopView)                      // call only once
                    pTopView->Command( rCEvt );
            }
        }
    }
    else if ( rCEvt.GetCommand() == CommandEventId::QueryCharPosition )
    {
        if ( eMode != SC_INPUT_NONE )
        {
            UpdateActiveView();
            if (pTableView || pTopView)
            {
                if (pTableView)
                    pTableView->Command( rCEvt );
                else if (pTopView)                      // call only once
                    pTopView->Command( rCEvt );
            }
        }
    }
    else
    {
        HideTip();
        HideTipBelow();

        if ( bSelIsRef )
        {
            RemoveSelection();
            bSelIsRef = false;
        }

        UpdateActiveView();
        bool bNewView = DataChanging( 0, true );

        if (!bProtected)                            // changes allowed
        {
            if (bNewView)                           // create new edit view
            {
                if (pActiveViewSh)
                    pActiveViewSh->GetViewData().GetDocShell()->PostEditView( mpEditEngine.get(), aCursorPos );
                UpdateActiveView();
                if (eMode==SC_INPUT_NONE)
                    if (pTableView || pTopView)
                    {
                        if (pTableView)
                        {
                            pTableView->GetEditEngine()->SetText( "" );
                            pTableView->SetSelection( ESelection(0,0, 0,0) );
                        }
                        if (pTopView)
                        {
                            pTopView->GetEditEngine()->SetText( "" );
                            pTopView->SetSelection( ESelection(0,0, 0,0) );
                        }
                    }
                SyncViews();
            }

            if (pTableView || pTopView)
            {
                if (pTableView)
                    pTableView->Command( rCEvt );
                if (pTopView && !comphelper::LibreOfficeKit::isActive())
                    pTopView->Command( rCEvt );

                if ( rCEvt.GetCommand() == CommandEventId::EndExtTextInput )
                {
                    //  AutoInput after ext text input

                    if (pFormulaData)
                        miAutoPosFormula = pFormulaData->end();
                    if (pColumnData)
                        miAutoPosColumn = pColumnData->end();

                    if (bFormulaMode)
                        UseFormulaData();
                    else
                        UseColData();
                }
            }

            DataChanged();              //  calls UpdateParenthesis()
            InvalidateAttribs();        //! in DataChanged ?
        }

        if (pTopView && eMode != SC_INPUT_NONE)
            SyncViews();
    }
}

void ScInputHandler::NotifyChange( const ScInputHdlState* pState,
                                   bool bForce, ScTabViewShell* pSourceSh,
                                   bool bStopEditing)
{
    // If the call originates from a macro call in the EnterHandler,
    // return immediately and don't mess up the status
    if (bInEnterHandler)
        return;

    bool bRepeat = (pState == pLastState.get());
    if (!bRepeat && pState && pLastState)
        bRepeat = (*pState == *pLastState);
    if (bRepeat && !bForce)
        return;

    bInOwnChange = true;                // disable ModifyHdl (reset below)

    if ( pState && !pLastState )        // Enable again
        bForce = true;

    bool bHadObject = pLastState && pLastState->GetEditData();

    //! Before EditEngine gets eventually created (so it gets the right pools)
    if ( pSourceSh )
        pActiveViewSh = pSourceSh;
    else
        pActiveViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );

    ImplCreateEditEngine();

    if ( pState != pLastState.get() )
    {
        pLastState.reset( pState ? new ScInputHdlState( *pState ) : nullptr);
    }

    if ( pState && pActiveViewSh )
    {
        ScModule* pScMod = SC_MOD();

        ScTabViewShell* pScTabViewShell = dynamic_cast<ScTabViewShell*>(pScMod->GetViewShell());

        // Also take foreign reference input into account here (e.g. FunctionsAutoPilot),
        // FormEditData, if we're switching from Help to Calc:
        if ( !bFormulaMode && !pScMod->IsFormulaMode() &&
             ( !pScTabViewShell || !pScTabViewShell->GetFormEditData() ) )
        {
            bool bIgnore = false;
            if ( bModified )
            {
                if (pState->GetPos() != aCursorPos)
                {
                    if (!bProtected)
                        EnterHandler();
                }
                else
                    bIgnore = true;
            }

            if ( !bIgnore )
            {
                const ScAddress&        rSPos   = pState->GetStartPos();
                const ScAddress&        rEPos   = pState->GetEndPos();
                const EditTextObject*   pData   = pState->GetEditData();
                OUString aString = pState->GetString();
                bool bTxtMod = false;
                ScDocShell* pDocSh = pActiveViewSh->GetViewData().GetDocShell();
                ScDocument& rDoc = pDocSh->GetDocument();

                aCursorPos  = pState->GetPos();

                if ( pData )
                    bTxtMod = true;
                else if ( bHadObject )
                    bTxtMod = true;
                else if ( bTextValid )
                    bTxtMod = ( aString != aCurrentText );
                else
                    bTxtMod = ( aString != GetEditText(mpEditEngine.get()) );

                if ( bTxtMod || bForce )
                {
                    if (pData)
                    {
                        mpEditEngine->SetTextCurrentDefaults( *pData );
                        if (pInputWin)
                            aString = ScEditUtil::GetMultilineString(*mpEditEngine);
                        else
                            aString = GetEditText(mpEditEngine.get());
                        lcl_RemoveTabs(aString);
                        bTextValid = false;
                        aCurrentText.clear();
                    }
                    else
                    {
                        aCurrentText = aString;
                        bTextValid = true;              //! To begin with remember as a string
                    }

                    if ( pInputWin )
                        pInputWin->SetTextString(aString);

                    if (comphelper::LibreOfficeKit::isActive() && pActiveViewSh)
                        pActiveViewSh->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_FORMULA, aString.toUtf8().getStr());
                }

                if ( pInputWin || comphelper::LibreOfficeKit::isActive())                        // Named range input
                {
                    OUString aPosStr;
                    bool bSheetLocal = false;
                    const ScAddress::Details aAddrDetails( rDoc, aCursorPos );

                    // Is the range a name?
                    //! Find by Timer?
                    if ( pActiveViewSh )
                        pActiveViewSh->GetViewData().GetDocument().
                            GetRangeAtBlock( ScRange( rSPos, rEPos ), aPosStr, &bSheetLocal);

                    if ( aPosStr.isEmpty() )           // Not a name -> format
                    {
                        ScRefFlags nFlags = ScRefFlags::ZERO;
                        if( aAddrDetails.eConv == formula::FormulaGrammar::CONV_XL_R1C1 )
                            nFlags |= ScRefFlags::COL_ABS | ScRefFlags::ROW_ABS;
                        if ( rSPos != rEPos )
                        {
                            ScRange r(rSPos, rEPos);
                            applyStartToEndFlags(nFlags);
                            aPosStr = r.Format(rDoc, ScRefFlags::VALID | nFlags, aAddrDetails);
                        }
                        else
                            aPosStr = aCursorPos.Format(ScRefFlags::VALID | nFlags, &rDoc, aAddrDetails);
                    }
                    else if (bSheetLocal)
                    {
                        OUString aName;
                        if (rDoc.GetName( rSPos.Tab(), aName))
                            aPosStr = ScPosWnd::createLocalRangeName( aPosStr, aName);
                    }

                    if (pInputWin)
                    {
                        // Disable the accessible VALUE_CHANGE event
                        bool bIsSuppressed = pInputWin->IsAccessibilityEventsSuppressed(false);
                        pInputWin->SetAccessibilityEventsSuppressed(true);
                        pInputWin->SetPosString(aPosStr);
                        pInputWin->SetAccessibilityEventsSuppressed(bIsSuppressed);
                        pInputWin->SetSumAssignMode();
                    }

                    if (comphelper::LibreOfficeKit::isActive() && pActiveViewSh)
                        pActiveViewSh->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_ADDRESS, aPosStr.toUtf8().getStr());
                }

                if (bStopEditing) {
                    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScKillEditView ) );

                    //  As long as the content is not edited, turn off online spelling.
                    //  Online spelling is turned back on in StartTable, after setting
                    //  the right language from cell attributes.

                    EEControlBits nCntrl = mpEditEngine->GetControlWord();
                    if ( nCntrl & EEControlBits::ONLINESPELLING )
                        mpEditEngine->SetControlWord( nCntrl & ~EEControlBits::ONLINESPELLING );
                }

                bModified = false;
                bSelIsRef = false;
                bProtected = false;
                bCommandErrorShown = false;
            }
        }

        if ( pInputWin)
        {
            // Do not enable if RefDialog is open
            if(!pScMod->IsFormulaMode()&& !pScMod->IsRefDialogOpen())
            {
                if ( !pInputWin->IsEnabled())
                {
                    pDelayTimer->Stop();
                    pInputWin->Enable();
                }
            }
            else if(pScMod->IsRefDialogOpen())
            {   // Because every document has its own InputWin,
                // we should start Timer again, because the input line may
                // still be active
                if ( !pDelayTimer->IsActive() )
                    pDelayTimer->Start();
            }
        }
    }
    else // !pState || !pActiveViewSh
    {
        if ( !pDelayTimer->IsActive() )
            pDelayTimer->Start();
    }

    HideTip();
    HideTipBelow();
    bInOwnChange = false;
}

void ScInputHandler::UpdateCellAdjust( SvxCellHorJustify eJust )
{
    eAttrAdjust = eJust;
    UpdateAdjust( 0 );
}

void ScInputHandler::ResetDelayTimer()
{
    if( pDelayTimer->IsActive() )
    {
        pDelayTimer->Stop();
        if ( pInputWin )
            pInputWin->Enable();
    }
}

IMPL_LINK_NOARG( ScInputHandler, DelayTimer, Timer*, void )
{
    if ( !(nullptr == pLastState || SC_MOD()->IsFormulaMode() || SC_MOD()->IsRefDialogOpen()))
        return;

    //! New method at ScModule to query if function autopilot is open
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if ( pViewFrm && pViewFrm->GetChildWindow( SID_OPENDLG_FUNCTION ) )
    {
        if ( pInputWin)
        {
            pInputWin->EnableButtons( false );
            pInputWin->Disable();
        }
    }
    else if ( !bFormulaMode ) // Keep formula e.g. for help
    {
        bInOwnChange = true; // disable ModifyHdl (reset below)

        pActiveViewSh = nullptr;
        mpEditEngine->SetTextCurrentDefaults( OUString() );
        if ( pInputWin )
        {
            pInputWin->SetPosString( OUString() );
            pInputWin->SetTextString( OUString() );
            pInputWin->Disable();
        }

        bInOwnChange = false;
    }
}

void ScInputHandler::InputSelection( const EditView* pView )
{
    SyncViews( pView );
    ShowTipCursor();
    UpdateParenthesis(); // Selection changed -> update parentheses highlighting

    // When the selection is changed manually, stop overwriting parentheses
    ResetAutoPar();
}

void ScInputHandler::InputChanged( const EditView* pView, bool bFromNotify )
{
    UpdateActiveView();

    // #i20282# DataChanged needs to know if this is from the input line's modify handler
    bool bFromTopNotify = ( bFromNotify && pView == pTopView );

    bool bNewView = DataChanging();                     //FIXME: Is this at all possible?
    aCurrentText = pView->GetEditEngine()->GetText();   // Also remember the string
    mpEditEngine->SetTextCurrentDefaults( aCurrentText );
    DataChanged( bFromTopNotify );
    bTextValid = true; // Is set to false in DataChanged

    if ( pActiveViewSh )
    {
        ScViewData& rViewData = pActiveViewSh->GetViewData();
        if ( bNewView )
            rViewData.GetDocShell()->PostEditView( mpEditEngine.get(), aCursorPos );

        rViewData.EditGrowY();
        rViewData.EditGrowX();
    }

    SyncViews( pView );
}

const OUString& ScInputHandler::GetEditString()
{
    if (mpEditEngine)
    {
        aCurrentText = mpEditEngine->GetText(); // Always new from Engine
        bTextValid = true;
    }

    return aCurrentText;
}

Size ScInputHandler::GetTextSize()
{
    Size aSize;
    if ( mpEditEngine )
        aSize = Size( mpEditEngine->CalcTextWidth(), mpEditEngine->GetTextHeight() );

    return aSize;
}

bool ScInputHandler::GetTextAndFields( ScEditEngineDefaulter& rDestEngine )
{
    bool bRet = false;
    if (mpEditEngine)
    {
        // Contains field?
        sal_Int32 nParCnt = mpEditEngine->GetParagraphCount();
        SfxItemSet aSet = mpEditEngine->GetAttribs( ESelection(0,0,nParCnt,0) );
        SfxItemState eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, false );
        if ( eFieldState == SfxItemState::DONTCARE || eFieldState == SfxItemState::SET )
        {
            // Copy content
            std::unique_ptr<EditTextObject> pObj = mpEditEngine->CreateTextObject();
            rDestEngine.SetTextCurrentDefaults(*pObj);
            pObj.reset();

            // Delete attributes
            for (sal_Int32 i=0; i<nParCnt; i++)
                rDestEngine.RemoveCharAttribs( i );

            // Combine paragraphs
            while ( nParCnt > 1 )
            {
                sal_Int32 nLen = rDestEngine.GetTextLen( 0 );
                ESelection aSel( 0,nLen, 1,0 );
                rDestEngine.QuickInsertText( OUString(' '), aSel ); // Replace line break with space
                --nParCnt;
            }

            bRet = true;
        }
    }
    return bRet;
}

/**
 * Methods for FunctionAutoPilot:
 * InputGetSelection, InputSetSelection, InputReplaceSelection, InputGetFormulaStr
 */
void ScInputHandler::InputGetSelection( sal_Int32& rStart, sal_Int32& rEnd )
{
    rStart = nFormSelStart;
    rEnd = nFormSelEnd;
}

EditView* ScInputHandler::GetFuncEditView()
{
    UpdateActiveView(); // Due to pTableView

    EditView* pView = nullptr;
    if ( pInputWin )
    {
        pInputWin->MakeDialogEditView();
        pView = pInputWin->GetEditView();
    }
    else
    {
        if ( eMode != SC_INPUT_TABLE )
        {
            bCreatingFuncView = true; // Don't display RangeFinder
            SetMode( SC_INPUT_TABLE );
            bCreatingFuncView = false;
            if ( pTableView )
                pTableView->GetEditEngine()->SetText( OUString() );
        }
        pView = pTableView;
    }

    return pView;
}

void ScInputHandler::InputSetSelection( sal_Int32 nStart, sal_Int32 nEnd )
{
    if ( nStart <= nEnd )
    {
        nFormSelStart = nStart;
        nFormSelEnd = nEnd;
    }
    else
    {
        nFormSelEnd = nStart;
        nFormSelStart = nEnd;
    }

    EditView* pView = GetFuncEditView();
    if (pView)
        pView->SetSelection( ESelection(0,nStart, 0,nEnd) );

    bModified = true;
}

void ScInputHandler::InputReplaceSelection( const OUString& rStr )
{
    if (!pRefViewSh)
        pRefViewSh = pActiveViewSh;

    OSL_ENSURE(nFormSelEnd>=nFormSelStart,"Selection broken...");

    sal_Int32 nOldLen = nFormSelEnd - nFormSelStart;
    sal_Int32 nNewLen = rStr.getLength();

    OUStringBuffer aBuf(aFormText);
    if (nOldLen)
        aBuf.remove(nFormSelStart, nOldLen);
    if (nNewLen)
        aBuf.insert(nFormSelStart, rStr);

    aFormText = aBuf.makeStringAndClear();

    nFormSelEnd = nFormSelStart + nNewLen;

    EditView* pView = GetFuncEditView();
    if (pView)
    {
        pView->SetEditEngineUpdateLayout( false );
        pView->GetEditEngine()->SetText( aFormText );
        pView->SetSelection( ESelection(0,nFormSelStart, 0,nFormSelEnd) );
        pView->SetEditEngineUpdateLayout( true );
    }
    bModified = true;
}

void ScInputHandler::InputTurnOffWinEngine()
{
    bInOwnChange = true;                // disable ModifyHdl (reset below)

    eMode = SC_INPUT_NONE;
    /* TODO: it would be better if there was some way to reset the input bar
     * engine instead of deleting and having it recreate through
     * GetFuncEditView(), but first least invasively let this fix fdo#71667 and
     * fdo#72278 without reintroducing fdo#69971. */
    StopInputWinEngine(true);

    bInOwnChange = false;
}

/**
 * ScInputHdlState
 */
ScInputHdlState::ScInputHdlState( const ScAddress& rCurPos,
                                  const ScAddress& rStartPos,
                                  const ScAddress& rEndPos,
                                  const OUString& rString,
                                  const EditTextObject* pData )
    :   aCursorPos  ( rCurPos ),
        aStartPos   ( rStartPos ),
        aEndPos     ( rEndPos ),
        aString     ( rString ),
        pEditData   ( pData ? pData->Clone() : nullptr )
{
}

ScInputHdlState::ScInputHdlState( const ScInputHdlState& rCpy )
{
    *this = rCpy;
}

ScInputHdlState::~ScInputHdlState()
{
}

bool ScInputHdlState::operator==( const ScInputHdlState& r ) const
{
    return (    (aStartPos  == r.aStartPos)
             && (aEndPos    == r.aEndPos)
             && (aCursorPos == r.aCursorPos)
             && (aString    == r.aString)
             && ScGlobal::EETextObjEqual( pEditData.get(), r.pEditData.get() ) );
}

ScInputHdlState& ScInputHdlState::operator=( const ScInputHdlState& r )
{
    if (this != &r)
    {
        aCursorPos  = r.aCursorPos;
        aStartPos   = r.aStartPos;
        aEndPos     = r.aEndPos;
        aString     = r.aString;
        pEditData.reset();
        if (r.pEditData)
            pEditData = r.pEditData->Clone();
    }
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
