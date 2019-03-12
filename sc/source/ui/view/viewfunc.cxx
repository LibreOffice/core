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

#include <config_features.h>

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/justifyitem.hxx>
#include <sfx2/bindings.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/wrkwin.hxx>
#include <stdlib.h>
#include <unotools/charclass.hxx>

#include <viewfunc.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <uiitems.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <undocell.hxx>
#include <undoblk.hxx>
#include <undotab.hxx>
#include <refundo.hxx>
#include <dbdata.hxx>
#include <olinetab.hxx>
#include <rangeutl.hxx>
#include <rangenam.hxx>
#include <globstr.hrc>
#include <global.hxx>
#include <stlsheet.hxx>
#include <editutil.hxx>
#include <formulacell.hxx>
#include <scresid.hxx>
#include <inputhdl.hxx>
#include <scmod.hxx>
#include <inputopt.hxx>
#include <compiler.hxx>
#include <docfunc.hxx>
#include <appoptio.hxx>
#include <dociter.hxx>
#include <sizedev.hxx>
#include <editable.hxx>
#include <scui_def.hxx>
#include <funcdesc.hxx>
#include <docuno.hxx>
#include <cellsuno.hxx>
#include <tokenarray.hxx>
#include <rowheightcontext.hxx>
#include <docfuncutil.hxx>
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>
#include <conditio.hxx>
#include <columnspanset.hxx>

#include <memory>

static void lcl_PostRepaintCondFormat( const ScConditionalFormat *pCondFmt, ScDocShell *pDocSh )
{
    if( pCondFmt )
    {
        const ScRangeList& rRanges = pCondFmt->GetRange();

        pDocSh->PostPaint( rRanges, PaintPartFlags::All );
    }
}

ScViewFunc::ScViewFunc( vcl::Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    ScTabView( pParent, rDocSh, pViewShell ),
    bFormatValid( false )
{
}

ScViewFunc::~ScViewFunc()
{
}

void ScViewFunc::StartFormatArea()
{
    //  anything to do?
    if ( !SC_MOD()->GetInputOptions().GetExtendFormat() )
        return;

    //  start only with single cell (marked or cursor position)
    ScRange aMarkRange;
    bool bOk = (GetViewData().GetSimpleArea( aMarkRange ) == SC_MARK_SIMPLE);
    if ( bOk && aMarkRange.aStart != aMarkRange.aEnd )
        bOk = false;

    if (bOk)
    {
        bFormatValid = true;
        aFormatSource = aMarkRange.aStart;
        aFormatArea = ScRange( aFormatSource );
    }
    else
        bFormatValid = false;       // discard old range
}

bool ScViewFunc::TestFormatArea( SCCOL nCol, SCROW nRow, SCTAB nTab, bool bAttrChanged )
{
    //  anything to do?
    if ( !SC_MOD()->GetInputOptions().GetExtendFormat() )
        return false;

    //  Test: treat input with numberformat (bAttrChanged) always as new Attribute
    //  (discard old Area ). If not wanted, discard if-statement
    if ( bAttrChanged )
    {
        StartFormatArea();
        return false;
    }

    //! Test if cell empty ???

    bool bFound = false;
    ScRange aNewRange = aFormatArea;
    if ( bFormatValid && nTab == aFormatSource.Tab() )
    {
        if ( nRow >= aFormatArea.aStart.Row() && nRow <= aFormatArea.aEnd.Row() )
        {
            //  within range?
            if ( nCol >= aFormatArea.aStart.Col() && nCol <= aFormatArea.aEnd.Col() )
            {
                bFound = true;          // do not change range
            }
            //  left ?
            if ( nCol+1 == aFormatArea.aStart.Col() )
            {
                bFound = true;
                aNewRange.aStart.SetCol( nCol );
            }
            //  right ?
            if ( nCol == aFormatArea.aEnd.Col()+1 )
            {
                bFound = true;
                aNewRange.aEnd.SetCol( nCol );
            }
        }
        if ( nCol >= aFormatArea.aStart.Col() && nCol <= aFormatArea.aEnd.Col() )
        {
            //  top ?
            if ( nRow+1 == aFormatArea.aStart.Row() )
            {
                bFound = true;
                aNewRange.aStart.SetRow( nRow );
            }
            //  bottom ?
            if ( nRow == aFormatArea.aEnd.Row()+1 )
            {
                bFound = true;
                aNewRange.aEnd.SetRow( nRow );
            }
        }
    }

    if (bFound)
        aFormatArea = aNewRange;    // extend
    else
        bFormatValid = false;       // outside of range -> break

    return bFound;
}

void ScViewFunc::DoAutoAttributes( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                   bool bAttrChanged )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();

    const ScPatternAttr* pSource = rDoc.GetPattern(
                            aFormatSource.Col(), aFormatSource.Row(), nTab );
    if ( !pSource->GetItem(ATTR_MERGE).IsMerged() )
    {
        ScRange aRange( nCol, nRow, nTab, nCol, nRow, nTab );
        ScMarkData aMark;
        aMark.SetMarkArea( aRange );

        ScDocFunc &rFunc = GetViewData().GetDocFunc();

        // pOldPattern is only valid until call to ApplyAttributes!
        const ScPatternAttr* pOldPattern = rDoc.GetPattern( nCol, nRow, nTab );
        const ScStyleSheet* pSrcStyle = pSource->GetStyleSheet();
        if ( pSrcStyle && pSrcStyle != pOldPattern->GetStyleSheet() )
            rFunc.ApplyStyle( aMark, pSrcStyle->GetName(), false );

        rFunc.ApplyAttributes( aMark, *pSource, false );
    }

    if ( bAttrChanged )                             // value entered with number format?
        aFormatSource.Set( nCol, nRow, nTab );      // then set a new source
}

//      additional routines

sal_uInt16 ScViewFunc::GetOptimalColWidth( SCCOL nCol, SCTAB nTab, bool bFormula )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();

    double nPPTX = GetViewData().GetPPTX();
    double nPPTY = GetViewData().GetPPTY();
    Fraction aZoomX = GetViewData().GetZoomX();
    Fraction aZoomY = GetViewData().GetZoomY();

    ScSizeDeviceProvider aProv(pDocSh);
    if (aProv.IsPrinter())
    {
        nPPTX = aProv.GetPPTX();
        nPPTY = aProv.GetPPTY();
        aZoomX = aZoomY = Fraction( 1, 1 );
    }

    sal_uInt16 nTwips = rDoc.GetOptimalColWidth( nCol, nTab, aProv.GetDevice(),
                                nPPTX, nPPTY, aZoomX, aZoomY, bFormula, &rMark );
    return nTwips;
}

bool ScViewFunc::SelectionEditable( bool* pOnlyNotBecauseOfMatrix /* = NULL */ )
{
    bool bRet;
    ScDocument* pDoc = GetViewData().GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();
    if (rMark.IsMarked() || rMark.IsMultiMarked())
        bRet = pDoc->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix );
    else
    {
        SCCOL nCol = GetViewData().GetCurX();
        SCROW nRow = GetViewData().GetCurY();
        SCTAB nTab = GetViewData().GetTabNo();
        bRet = pDoc->IsBlockEditable( nTab, nCol, nRow, nCol, nRow,
            pOnlyNotBecauseOfMatrix );
    }
    return bRet;
}

#ifndef LRU_MAX
#define LRU_MAX 10
#endif

static bool lcl_FunctionKnown( sal_uInt16 nOpCode )
{
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uLong nCount = pFuncList->GetCount();
        for (sal_uLong i=0; i<nCount; i++)
            if ( pFuncList->GetFunction(i)->nFIndex == nOpCode )
                return true;
    }
    return false;
}

static bool lcl_AddFunction( ScAppOptions& rAppOpt, sal_uInt16 nOpCode )
{
    sal_uInt16 nOldCount = rAppOpt.GetLRUFuncListCount();
    sal_uInt16* pOldList = rAppOpt.GetLRUFuncList();
    sal_uInt16 nPos;
    for (nPos=0; nPos<nOldCount; nPos++)
        if (pOldList[nPos] == nOpCode)          // is the function already in the list?
        {
            if ( nPos == 0 )
                return false;                   // already at the top -> no change

            //  count doesn't change, so the original array is modified

            for (sal_uInt16 nCopy=nPos; nCopy>0; nCopy--)
                pOldList[nCopy] = pOldList[nCopy-1];
            pOldList[0] = nOpCode;

            return true;                        // list has changed
        }

    if ( !lcl_FunctionKnown( nOpCode ) )
        return false;                           // not in function list -> no change

    sal_uInt16 nNewCount = std::min( static_cast<sal_uInt16>(nOldCount + 1), sal_uInt16(LRU_MAX) );
    sal_uInt16 nNewList[LRU_MAX];
    nNewList[0] = nOpCode;
    for (nPos=1; nPos<nNewCount; nPos++)
        nNewList[nPos] = pOldList[nPos-1];
    rAppOpt.SetLRUFuncList( nNewList, nNewCount );

    return true;                                // list has changed
}

namespace HelperNotifyChanges
{
    static void NotifyIfChangesListeners(const ScDocShell &rDocShell, ScMarkData& rMark, SCCOL nCol, SCROW nRow)
    {
        if (ScModelObj *pModelObj = getMustPropagateChangesModel(rDocShell))
        {
            ScRangeList aChangeRanges;
            for (const auto& rTab : rMark)
                aChangeRanges.push_back( ScRange( nCol, nRow, rTab ) );

            HelperNotifyChanges::Notify(*pModelObj, aChangeRanges, "cell-change");
        }
    }
}

//      actual functions

//  input - undo OK
void ScViewFunc::EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab,
                            const OUString& rString,
                            const EditTextObject* pData )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScMarkData rMark(GetViewData().GetMarkData());
    bool bRecord = pDoc->IsUndoEnabled();
    SCTAB i;

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocFunc &rFunc = GetViewData().GetDocFunc();
    ScDocShellModificator aModificator( *pDocSh );

    ScEditableTester aTester( pDoc, nCol,nRow, nCol,nRow, rMark );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        PaintArea(nCol, nRow, nCol, nRow);        // possibly the edit-engine is still painted there
        return;
    }

    if ( bRecord )
        rFunc.EnterListAction( STR_UNDO_ENTERDATA );

    bool bFormula = false;

    // a single '=' character is handled as string (needed for special filters)
    if ( rString.getLength() > 1 )
    {
        if ( rString[0] == '=' )
        {
            // handle as formula
            bFormula = true;
        }
        else if ( rString[0] == '+' || rString[0] == '-' )
        {
            // if there is more than one leading '+' or '-' character, remove the additional ones
            sal_Int32 nIndex = 1;
            sal_Int32 nLen = rString.getLength();
            while ( nIndex < nLen && ( rString[ nIndex ] == '+' || rString[ nIndex ] == '-' ) )
            {
                ++nIndex;
            }
            OUString aString = rString.replaceAt( 1, nIndex - 1, "" );

            // if the remaining part without the leading '+' or '-' character
            // is non-empty and not a number, handle as formula
            if ( aString.getLength() > 1 )
            {
                sal_uInt32 nFormat = 0;
                pDoc->GetNumberFormat( nCol, nRow, nTab, nFormat );
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                double fNumber = 0;
                if ( !pFormatter->IsNumberFormat( aString, nFormat, fNumber ) )
                {
                    bFormula = true;
                }
            }
        }
    }

    bool bNumFmtChanged = false;
    if ( bFormula )
    {   // formula, compile with autoCorrection
        i = rMark.GetFirstSelected();
        ScAddress aPos( nCol, nRow, i );
        ScCompiler aComp( pDoc, aPos, pDoc->GetGrammar(), true, false );
//2do: enable/disable autoCorrection via calcoptions
        aComp.SetAutoCorrection( true );
        if ( rString[0] == '+' || rString[0] == '-' )
        {
            aComp.SetExtendedErrorDetection( ScCompiler::EXTENDED_ERROR_DETECTION_NAME_BREAK );
        }
        OUString aFormula( rString );
        std::unique_ptr< ScTokenArray > pArr;
        bool bAgain;
        do
        {
            bAgain = false;
            bool bAddEqual = false;
            pArr = aComp.CompileString( aFormula );
            bool bCorrected = aComp.IsCorrected();
            std::unique_ptr< ScTokenArray > pArrFirst;
            if ( bCorrected )
            {   // try to parse with first parser-correction
                pArrFirst = std::move( pArr );
                pArr = aComp.CompileString( aComp.GetCorrectedFormula() );
            }
            if ( pArr->GetCodeError() == FormulaError::NONE )
            {
                bAddEqual = true;
                aComp.CompileTokenArray();
                bCorrected |= aComp.IsCorrected();
            }
            if ( bCorrected )
            {
                OUString aCorrectedFormula;
                if ( bAddEqual )
                {
                    aCorrectedFormula = "=" + aComp.GetCorrectedFormula();
                }
                else
                    aCorrectedFormula = aComp.GetCorrectedFormula();
                short nResult;
                if ( aCorrectedFormula.getLength() == 1 )
                    nResult = RET_NO;   // empty formula, just '='
                else
                {
                    OUString aMessage( ScResId( SCSTR_FORMULA_AUTOCORRECTION ) );
                    aMessage += aCorrectedFormula;

                    vcl::Window* pWin = GetViewData().GetDialogParent();
                    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                                   aMessage));
                    xQueryBox->set_default_response(RET_YES);
                    nResult = xQueryBox->run();
                }
                if ( nResult == RET_YES )
                {
                    aFormula = aCorrectedFormula;
                    bAgain = true;
                }
                else
                {
                    if ( pArrFirst )
                        pArr = std::move( pArrFirst );
                }
            }
        } while ( bAgain );
        // to be used in multiple tabs, the formula must be compiled anew
        // via ScFormulaCell copy-ctor because of RangeNames,
        // the same code-array for all cells is not possible.
        // If the array has an error, (it) must be RPN-erased in the newly generated
        // cells and the error be set explicitly, so that
        // via FormulaCell copy-ctor and Interpreter it will be, when possible,
        // ironed out again, too intelligent.. e.g.: =1))
        FormulaError nError = pArr->GetCodeError();
        if ( nError == FormulaError::NONE )
        {
            //  update list of recent functions with all functions that
            //  are not within parentheses

            ScModule* pScMod = SC_MOD();
            ScAppOptions aAppOpt = pScMod->GetAppOptions();
            bool bOptChanged = false;

            formula::FormulaToken** ppToken = pArr->GetArray();
            sal_uInt16 nTokens = pArr->GetLen();
            sal_uInt16 nLevel = 0;
            for (sal_uInt16 nTP=0; nTP<nTokens; nTP++)
            {
                formula::FormulaToken* pTok = ppToken[nTP];
                OpCode eOp = pTok->GetOpCode();
                if ( eOp == ocOpen )
                    ++nLevel;
                else if ( eOp == ocClose && nLevel )
                    --nLevel;
                if ( nLevel == 0 && pTok->IsFunction() &&
                        lcl_AddFunction( aAppOpt, sal::static_int_cast<sal_uInt16>( eOp ) ) )
                    bOptChanged = true;
            }

            if ( bOptChanged )
            {
                pScMod->SetAppOptions(aAppOpt);
            }
        }

        ScFormulaCell aCell(pDoc, aPos, std::move( pArr ), formula::FormulaGrammar::GRAM_DEFAULT, ScMatrixMode::NONE);

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        for (const auto& rTab : rMark)
        {
            i = rTab;
            aPos.SetTab( i );
            const sal_uInt32 nIndex = pDoc->GetAttr(
                        nCol, nRow, i, ATTR_VALUE_FORMAT )->GetValue();
            const SvNumFormatType nType = pFormatter->GetType( nIndex);
            if (nType == SvNumFormatType::TEXT ||
                    ((rString[0] == '+' || rString[0] == '-') && nError != FormulaError::NONE && rString == aFormula))
            {
                if ( pData )
                {
                    // A clone of pData will be stored in the cell.
                    rFunc.SetEditCell(aPos, *pData, true);
                }
                else
                    rFunc.SetStringCell(aPos, aFormula, true);
            }
            else
            {
                ScFormulaCell* pCell = new ScFormulaCell( aCell, *pDoc, aPos );
                if ( nError != FormulaError::NONE )
                {
                    pCell->GetCode()->DelRPN();
                    pCell->SetErrCode( nError );
                    if(pCell->GetCode()->IsHyperLink())
                        pCell->GetCode()->SetHyperLink(false);
                }
                if (nType == SvNumFormatType::LOGICAL)
                {
                    // Reset to General so the actual format can be determined
                    // after the cell has been interpreted. A sticky boolean
                    // number format is highly likely unwanted.. see tdf#75650.
                    // General of same locale as current number format.
                    const SvNumberformat* pEntry = pFormatter->GetEntry( nIndex);
                    const LanguageType nLang = (pEntry ? pEntry->GetLanguage() : ScGlobal::eLnge);
                    const sal_uInt32 nFormat = pFormatter->GetStandardFormat( SvNumFormatType::NUMBER, nLang);
                    ScPatternAttr aPattern( pDoc->GetPool());
                    aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nFormat));
                    ScMarkData aMark;
                    aMark.SelectTable( i, true);
                    aMark.SetMarkArea( ScRange( aPos));
                    rFunc.ApplyAttributes( aMark, aPattern, false);
                    bNumFmtChanged = true;
                }
                rFunc.SetFormulaCell(aPos, pCell, true);
            }
        }
    }
    else
    {
        for (const auto& rTab : rMark)
        {
            bool bNumFmtSet = false;
            rFunc.SetNormalString( bNumFmtSet, ScAddress( nCol, nRow, rTab ), rString, false );
            if (bNumFmtSet)
            {
                /* FIXME: if set on any sheet results in changed only on
                 * sheet nTab for TestFormatArea() and DoAutoAttributes() */
                bNumFmtChanged = true;
            }
        }
    }

    bool bAutoFormat = TestFormatArea(nCol, nRow, nTab, bNumFmtChanged);

    if (bAutoFormat)
        DoAutoAttributes(nCol, nRow, nTab, bNumFmtChanged);

    pDocSh->UpdateOle(&GetViewData());

    HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, rMark, nCol, nRow);

    if ( bRecord )
        rFunc.EndListAction();

    aModificator.SetDocumentModified();
    lcl_PostRepaintCondFormat( pDoc->GetCondFormat( nCol, nRow, nTab ), pDocSh );
}

// enter value in single cell (on nTab only)

void ScViewFunc::EnterValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rValue )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScDocShell* pDocSh = GetViewData().GetDocShell();

    if ( pDoc && pDocSh )
    {
        bool bUndo(pDoc->IsUndoEnabled());
        ScDocShellModificator aModificator( *pDocSh );

        ScEditableTester aTester( pDoc, nTab, nCol,nRow, nCol,nRow );
        if (aTester.IsEditable())
        {
            ScAddress aPos( nCol, nRow, nTab );
            ScCellValue aUndoCell;
            if (bUndo)
                aUndoCell.assign(*pDoc, aPos);

            pDoc->SetValue( nCol, nRow, nTab, rValue );

            // because of ChangeTrack after change in document
            if (bUndo)
            {
                pDocSh->GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoEnterValue>(pDocSh, aPos, aUndoCell, rValue));
            }

            pDocSh->PostPaintCell( aPos );
            pDocSh->UpdateOle(&GetViewData());
            aModificator.SetDocumentModified();
        }
        else
            ErrorMessage(aTester.GetMessageId());
    }
}

void ScViewFunc::EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab,
                            const EditTextObject& rData, bool bTestSimple )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScMarkData& rMark = GetViewData().GetMarkData();
    ScDocument& rDoc = pDocSh->GetDocument();
    bool bRecord = rDoc.IsUndoEnabled();

    ScDocShellModificator aModificator( *pDocSh );

    ScEditableTester aTester( &rDoc, nTab, nCol,nRow, nCol,nRow );
    if (aTester.IsEditable())
    {

        //      test for attribute

        bool bSimple = false;
        bool bCommon = false;
        std::unique_ptr<ScPatternAttr> pCellAttrs;
        OUString aString;

        const ScPatternAttr* pOldPattern = rDoc.GetPattern( nCol, nRow, nTab );
        ScTabEditEngine aEngine( *pOldPattern, rDoc.GetEnginePool(), &rDoc );
        aEngine.SetText(rData);

        if (bTestSimple)                    // test, if simple string without attribute
        {
            ScEditAttrTester aAttrTester( &aEngine );
            bSimple = !aAttrTester.NeedsObject();
            bCommon = aAttrTester.NeedsCellAttr();

            // formulas have to be recognized even if they're formatted
            // (but common attributes are still collected)

            if ( !bSimple && aEngine.GetParagraphCount() == 1 )
            {
                OUString aParStr(aEngine.GetText( 0 ));
                if ( aParStr[0] == '=' )
                    bSimple = true;
            }

            if (bCommon)                // attribute for tab
            {
                pCellAttrs.reset(new ScPatternAttr( *pOldPattern ));
                pCellAttrs->GetFromEditItemSet( &aAttrTester.GetAttribs() );
                //! remove common attributes from EditEngine?
            }
        }

        // #i97726# always get text for "repeat" of undo action
        aString = ScEditUtil::GetSpaceDelimitedString(aEngine);

        //      undo

        std::unique_ptr<EditTextObject> pUndoData;
        ScUndoEnterData::ValuesType aOldValues;

        if (bRecord && !bSimple)
        {
            for (const auto& rTab : rMark)
            {
                ScUndoEnterData::Value aOldValue;
                aOldValue.mnTab = rTab;
                aOldValue.maCell.assign(rDoc, ScAddress(nCol, nRow, rTab));
                aOldValues.push_back(aOldValue);
            }

            pUndoData = rData.Clone();
        }

        //      enter data

        if (bCommon)
            rDoc.ApplyPattern(nCol,nRow,nTab,*pCellAttrs);         //! undo

        if (bSimple)
        {
            if (bCommon)
                AdjustRowHeight(nRow,nRow);

            EnterData(nCol,nRow,nTab,aString);
        }
        else
        {
            for (const auto& rTab : rMark)
            {
                ScAddress aPos(nCol, nRow, rTab);
                rDoc.SetEditText(aPos, rData, rDoc.GetEditPool());
            }

            if ( bRecord )
            {   //  because of ChangeTrack current first
                pDocSh->GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoEnterData>(pDocSh, ScAddress(nCol,nRow,nTab), aOldValues, aString, std::move(pUndoData)));
            }

            HideAllCursors();

            AdjustRowHeight(nRow,nRow);

            for (const auto& rTab : rMark)
                pDocSh->PostPaintCell( nCol, nRow, rTab );

            ShowAllCursors();

            pDocSh->UpdateOle(&GetViewData());

            HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, rMark, nCol, nRow);

            aModificator.SetDocumentModified();
        }
        lcl_PostRepaintCondFormat( rDoc.GetCondFormat( nCol, nRow, nTab ), pDocSh );
    }
    else
    {
        ErrorMessage(aTester.GetMessageId());
        PaintArea( nCol, nRow, nCol, nRow );        // possibly the edit-engine is still painted there
    }
}

void ScViewFunc::EnterDataAtCursor( const OUString& rString )
{
    SCCOL nPosX = GetViewData().GetCurX();
    SCROW nPosY = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();

    EnterData( nPosX, nPosY, nTab, rString );
}

void ScViewFunc::EnterMatrix( const OUString& rString, ::formula::FormulaGrammar::Grammar eGram )
{
    ScViewData& rData = GetViewData();
    const SCCOL nCol = rData.GetCurX();
    const SCROW nRow = rData.GetCurY();
    const ScMarkData& rMark = rData.GetMarkData();
    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        //  nothing marked -> temporarily calculate block
        //  with size of result formula to get the size

        ScDocument* pDoc = rData.GetDocument();
        SCTAB nTab = rData.GetTabNo();
        ScFormulaCell aFormCell( pDoc, ScAddress(nCol,nRow,nTab), rString, eGram, ScMatrixMode::Formula );

        SCSIZE nSizeX;
        SCSIZE nSizeY;
        aFormCell.GetResultDimensions( nSizeX, nSizeY );
        if ( nSizeX != 0 && nSizeY != 0 &&
             nCol+nSizeX-1 <= sal::static_int_cast<SCSIZE>(MAXCOL) &&
             nRow+nSizeY-1 <= sal::static_int_cast<SCSIZE>(MAXROW) )
        {
            ScRange aResult( nCol, nRow, nTab,
                             sal::static_int_cast<SCCOL>(nCol+nSizeX-1),
                             sal::static_int_cast<SCROW>(nRow+nSizeY-1), nTab );
            MarkRange( aResult, false );
        }
    }

    ScRange aRange;
    if (rData.GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = rData.GetDocShell();
        bool bSuccess = pDocSh->GetDocFunc().EnterMatrix(
            aRange, &rMark, nullptr, rString, false, false, EMPTY_OUSTRING, eGram );
        if (bSuccess)
            pDocSh->UpdateOle(&GetViewData());
        else
            PaintArea(nCol, nRow, nCol, nRow);        // possibly the edit-engine is still painted there
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

SvtScriptType ScViewFunc::GetSelectionScriptType()
{
    SvtScriptType nScript = SvtScriptType::NONE;

    ScDocument* pDoc = GetViewData().GetDocument();
    const ScMarkData& rMark = GetViewData().GetMarkData();
    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        // no selection -> cursor

        nScript = pDoc->GetScriptType( GetViewData().GetCurX(),
                            GetViewData().GetCurY(), GetViewData().GetTabNo());
    }
    else
    {
        ScRangeList aRanges;
        rMark.FillRangeListWithMarks( &aRanges, false );
        nScript = pDoc->GetRangeScriptType(aRanges);
    }

    if (nScript == SvtScriptType::NONE)
        nScript = ScGlobal::GetDefaultScriptType();

    return nScript;
}

const ScPatternAttr* ScViewFunc::GetSelectionPattern()
{
    // Don't use UnmarkFiltered in slot state functions, for performance reasons.
    // The displayed state is always that of the whole selection including filtered rows.

    const ScMarkData& rMark = GetViewData().GetMarkData();
    ScDocument* pDoc = GetViewData().GetDocument();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        //  MarkToMulti is no longer necessary for pDoc->GetSelectionPattern
        const ScPatternAttr* pAttr = pDoc->GetSelectionPattern( rMark );
        return pAttr;
    }
    else
    {
        SCCOL  nCol = GetViewData().GetCurX();
        SCROW  nRow = GetViewData().GetCurY();
        SCTAB  nTab = GetViewData().GetTabNo();

        ScMarkData aTempMark( rMark );      // copy sheet selection
        aTempMark.SetMarkArea( ScRange( nCol, nRow, nTab ) );
        const ScPatternAttr* pAttr = pDoc->GetSelectionPattern( aTempMark );
        return pAttr;
    }
}

void ScViewFunc::GetSelectionFrame( SvxBoxItem&     rLineOuter,
                                    SvxBoxInfoItem& rLineInner )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    const ScMarkData& rMark = GetViewData().GetMarkData();

    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
        pDoc->GetSelectionFrame( rMark, rLineOuter, rLineInner );
    else
    {
        const ScPatternAttr* pAttrs =
                    pDoc->GetPattern( GetViewData().GetCurX(),
                                      GetViewData().GetCurY(),
                                      GetViewData().GetTabNo() );

        rLineOuter = pAttrs->GetItem( ATTR_BORDER );
        rLineInner = pAttrs->GetItem( ATTR_BORDER_INNER );
        rLineInner.SetTable(false);
        rLineInner.SetDist(true);
        rLineInner.SetMinDist(false);
    }
}

//  apply attribute - undo OK
//
//  complete set ( ATTR_STARTINDEX, ATTR_ENDINDEX )

void ScViewFunc::ApplyAttributes( const SfxItemSet* pDialogSet,
                                  const SfxItemSet* pOldSet,
                                  bool bAdjustBlockHeight)
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aOldAttrs( std::make_unique<SfxItemSet>(*pOldSet) );
    ScPatternAttr aNewAttrs( std::make_unique<SfxItemSet>(*pDialogSet) );
    aNewAttrs.DeleteUnchanged( &aOldAttrs );

    if ( pDialogSet->GetItemState( ATTR_VALUE_FORMAT ) == SfxItemState::SET )
    {   // don't reset to default SYSTEM GENERAL if not intended
        sal_uInt32 nOldFormat =
            pOldSet->Get( ATTR_VALUE_FORMAT ).GetValue();
        sal_uInt32 nNewFormat =
            pDialogSet->Get( ATTR_VALUE_FORMAT ).GetValue();
        if ( nNewFormat != nOldFormat )
        {
            SvNumberFormatter* pFormatter =
                GetViewData().GetDocument()->GetFormatTable();
            const SvNumberformat* pOldEntry = pFormatter->GetEntry( nOldFormat );
            LanguageType eOldLang =
                pOldEntry ? pOldEntry->GetLanguage() : LANGUAGE_DONTKNOW;
            const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewFormat );
            LanguageType eNewLang =
                pNewEntry ? pNewEntry->GetLanguage() : LANGUAGE_DONTKNOW;
            if ( eNewLang != eOldLang )
            {
                aNewAttrs.GetItemSet().Put(
                    SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );

                //  only the language has changed -> do not touch numberformat-attribute
                sal_uInt32 nNewMod = nNewFormat % SV_COUNTRY_LANGUAGE_OFFSET;
                if ( nNewMod == ( nOldFormat % SV_COUNTRY_LANGUAGE_OFFSET ) &&
                     nNewMod <= SV_MAX_COUNT_STANDARD_FORMATS )
                    aNewAttrs.GetItemSet().ClearItem( ATTR_VALUE_FORMAT );
            }
        }
    }

    if (pDialogSet->HasItem(ATTR_FONT_LANGUAGE))
        // font language has changed.  Redo the online spelling.
        ResetAutoSpell();

    const SvxBoxItem&     rOldOuter = pOldSet->Get(ATTR_BORDER);
    const SvxBoxItem&     rNewOuter = pDialogSet->Get(ATTR_BORDER);
    const SvxBoxInfoItem& rOldInner = pOldSet->Get(ATTR_BORDER_INNER);
    const SvxBoxInfoItem& rNewInner = pDialogSet->Get(ATTR_BORDER_INNER);
    SfxItemSet&           rNewSet   = aNewAttrs.GetItemSet();
    SfxItemPool*          pNewPool  = rNewSet.GetPool();

    pNewPool->Put(rNewOuter);        // don't delete yet
    pNewPool->Put(rNewInner);
    rNewSet.ClearItem( ATTR_BORDER );
    rNewSet.ClearItem( ATTR_BORDER_INNER );

    /*
     * establish whether border attribute is to be set:
     * 1. new != old
     * 2. is one of the borders not-DontCare (since 238.f: IsxxValid())
     *
     */

    bool bFrame =    (pDialogSet->GetItemState( ATTR_BORDER ) != SfxItemState::DEFAULT)
                  || (pDialogSet->GetItemState( ATTR_BORDER_INNER ) != SfxItemState::DEFAULT);

    if (&rNewOuter == &rOldOuter && &rNewInner == &rOldInner)
        bFrame = false;

    //  this should be intercepted by the pool: ?!??!??

    if (bFrame && rNewOuter == rOldOuter && rNewInner == rOldInner)
        bFrame = false;

    bFrame =   bFrame
            && (   rNewInner.IsValid(SvxBoxInfoItemValidFlags::LEFT)
                || rNewInner.IsValid(SvxBoxInfoItemValidFlags::RIGHT)
                || rNewInner.IsValid(SvxBoxInfoItemValidFlags::TOP)
                || rNewInner.IsValid(SvxBoxInfoItemValidFlags::BOTTOM)
                || rNewInner.IsValid(SvxBoxInfoItemValidFlags::HORI)
                || rNewInner.IsValid(SvxBoxInfoItemValidFlags::VERT) );

    if (!bFrame)
        ApplySelectionPattern( aNewAttrs );            // standard only
    else
    {
        // if new items are default-items, overwrite the old items:

        bool bDefNewOuter = IsStaticDefaultItem(&rNewOuter);
        bool bDefNewInner = IsStaticDefaultItem(&rNewInner);

        ApplyPatternLines( aNewAttrs,
                           bDefNewOuter ? rOldOuter : rNewOuter,
                           bDefNewInner ? &rOldInner : &rNewInner );
    }

    pNewPool->Remove(rNewOuter);         // release
    pNewPool->Remove(rNewInner);

    //  adjust height only if needed
    if (bAdjustBlockHeight)
        AdjustBlockHeight();

    // CellContentChanged is called in ApplySelectionPattern / ApplyPatternLines
}

void ScViewFunc::ApplyAttr( const SfxPoolItem& rAttrItem, bool bAdjustBlockHeight )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aNewAttrs( std::make_unique<SfxItemSet>( *GetViewData().GetDocument()->GetPool(),
                                            svl::Items<ATTR_PATTERN_START, ATTR_PATTERN_END>{} ) );

    aNewAttrs.GetItemSet().Put( rAttrItem );
    //  if justify is set (with Buttons), always indentation 0
    if ( rAttrItem.Which() == ATTR_HOR_JUSTIFY )
        aNewAttrs.GetItemSet().Put( SfxUInt16Item( ATTR_INDENT, 0 ) );
    ApplySelectionPattern( aNewAttrs );

    // Prevent useless compute
    if (bAdjustBlockHeight)
        AdjustBlockHeight();

    // CellContentChanged is called in ApplySelectionPattern
}

//  patterns and borders

void ScViewFunc::ApplyPatternLines( const ScPatternAttr& rAttr, const SvxBoxItem& rNewOuter,
                                    const SvxBoxInfoItem* pNewInner )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScMarkData aFuncMark( GetViewData().GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
    bool bRecord = true;
    if (!pDoc->IsUndoEnabled())
        bRecord = false;

    bool bRemoveAdjCellBorder = rNewOuter.IsRemoveAdjacentCellBorder();
    ScRange aMarkRange, aMarkRangeWithEnvelope;
    aFuncMark.MarkToSimple();
    bool bMulti = aFuncMark.IsMultiMarked();
    if (bMulti)
        aFuncMark.GetMultiMarkArea( aMarkRange );
    else if (aFuncMark.IsMarked())
        aFuncMark.GetMarkArea( aMarkRange );
    else
    {
        aMarkRange = ScRange( GetViewData().GetCurX(),
                            GetViewData().GetCurY(), GetViewData().GetTabNo() );
        DoneBlockMode();
        InitOwnBlockMode();
        aFuncMark.SetMarkArea(aMarkRange);
        MarkDataChanged();
    }
    if( bRemoveAdjCellBorder )
        aFuncMark.GetSelectionCover( aMarkRangeWithEnvelope );
    else
        aMarkRangeWithEnvelope = aMarkRange;

    ScDocShell* pDocSh = GetViewData().GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    if (bRecord)
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = pDoc->GetTableCount();
        bool bCopyOnlyMarked = false;
        if( !bRemoveAdjCellBorder )
            bCopyOnlyMarked = bMulti;
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        for (const auto& rTab : aFuncMark)
            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab( rTab, rTab );

        ScRange aCopyRange = aMarkRangeWithEnvelope;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, bCopyOnlyMarked, *pUndoDoc, &aFuncMark );

        pDocSh->GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoSelectionAttr>(
                pDocSh, aFuncMark,
                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), aMarkRange.aStart.Tab(),
                aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), aMarkRange.aEnd.Tab(),
                std::move(pUndoDoc), bCopyOnlyMarked, &rAttr, &rNewOuter, pNewInner, &aMarkRangeWithEnvelope ) );
    }

    sal_uInt16 nExt = SC_PF_TESTMERGE;
    pDocSh->UpdatePaintExt( nExt, aMarkRangeWithEnvelope ); // content before the change

    pDoc->ApplySelectionFrame(aFuncMark, rNewOuter, pNewInner);

    pDocSh->UpdatePaintExt( nExt, aMarkRangeWithEnvelope ); // content after the change

    aFuncMark.MarkToMulti();
    pDoc->ApplySelectionPattern( rAttr, aFuncMark );

    pDocSh->PostPaint( aMarkRange, PaintPartFlags::Grid, nExt );
    pDocSh->UpdateOle(&GetViewData());
    aModificator.SetDocumentModified();
    CellContentChanged();

    StartFormatArea();
}

//  pattern only

void ScViewFunc::ApplySelectionPattern( const ScPatternAttr& rAttr, bool bCursorOnly )
{
    ScViewData& rViewData   = GetViewData();
    ScDocShell* pDocSh      = rViewData.GetDocShell();
    ScDocument& rDoc        = pDocSh->GetDocument();
    ScMarkData aFuncMark( rViewData.GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, &rDoc );

    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    //  State from old ItemSet doesn't matter for paint flags, as any change will be
    //  from SfxItemState::SET in the new ItemSet (default is ignored in ApplyPattern).
    //  New alignment is checked (check in PostPaint isn't enough) in case a right
    //  alignment is changed to left.
    const SfxItemSet& rNewSet = rAttr.GetItemSet();
    bool bSetLines = rNewSet.GetItemState( ATTR_BORDER ) == SfxItemState::SET ||
                     rNewSet.GetItemState( ATTR_SHADOW ) == SfxItemState::SET;
    bool bSetAlign = rNewSet.GetItemState( ATTR_HOR_JUSTIFY ) == SfxItemState::SET;

    sal_uInt16 nExtFlags = 0;
    if ( bSetLines )
        nExtFlags |= SC_PF_LINES;
    if ( bSetAlign )
        nExtFlags |= SC_PF_WHOLEROWS;

    ScDocShellModificator aModificator( *pDocSh );

    bool bMulti = aFuncMark.IsMultiMarked();
    aFuncMark.MarkToMulti();
    bool bOnlyTab = (!aFuncMark.IsMultiMarked() && !bCursorOnly && aFuncMark.GetSelectCount() > 1);
    if (bOnlyTab)
    {
        SCCOL nCol = rViewData.GetCurX();
        SCROW nRow = rViewData.GetCurY();
        SCTAB nTab = rViewData.GetTabNo();
        aFuncMark.SetMarkArea(ScRange(nCol,nRow,nTab));
        aFuncMark.MarkToMulti();
    }

    ScRangeList aChangeRanges;

    if (aFuncMark.IsMultiMarked() && !bCursorOnly)
    {
        ScRange aMarkRange;
        aFuncMark.GetMultiMarkArea( aMarkRange );
        SCTAB nTabCount = rDoc.GetTableCount();
        for (const auto& rTab : aFuncMark)
        {
            ScRange aChangeRange( aMarkRange );
            aChangeRange.aStart.SetTab( rTab );
            aChangeRange.aEnd.SetTab( rTab );
            aChangeRanges.push_back( aChangeRange );
        }

        SCCOL nStartCol = aMarkRange.aStart.Col();
        SCROW nStartRow = aMarkRange.aStart.Row();
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCCOL nEndCol = aMarkRange.aEnd.Col();
        SCROW nEndRow = aMarkRange.aEnd.Row();
        SCTAB nEndTab = aMarkRange.aEnd.Tab();

        ScUndoSelectionAttr* pUndoAttr = nullptr;
        ScEditDataArray* pEditDataArray = nullptr;
        if (bRecord)
        {
            ScRange aCopyRange = aMarkRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);

            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nStartTab, nStartTab );
            for (const auto& rTab : aFuncMark)
                if (rTab != nStartTab)
                    pUndoDoc->AddUndoTab( rTab, rTab );
            rDoc.CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, bMulti, *pUndoDoc, &aFuncMark );

            aFuncMark.MarkToMulti();

            pUndoAttr = new ScUndoSelectionAttr(
                pDocSh, aFuncMark, nStartCol, nStartRow, nStartTab,
                nEndCol, nEndRow, nEndTab, std::move(pUndoDoc), bMulti, &rAttr );
            pDocSh->GetUndoManager()->AddUndoAction(std::unique_ptr<ScUndoSelectionAttr>(pUndoAttr));
            pEditDataArray = pUndoAttr->GetDataArray();
        }

        rDoc.ApplySelectionPattern( rAttr, aFuncMark, pEditDataArray );

        pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                           nEndCol,   nEndRow,   nEndTab,
                           PaintPartFlags::Grid, nExtFlags | SC_PF_TESTMERGE );
        pDocSh->UpdateOle(&GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
    }
    else                            // single cell - simpler undo
    {
        SCCOL nCol = rViewData.GetCurX();
        SCROW nRow = rViewData.GetCurY();
        SCTAB nTab = rViewData.GetTabNo();

        std::unique_ptr<EditTextObject> pOldEditData;
        std::unique_ptr<EditTextObject> pNewEditData;
        ScAddress aPos(nCol, nRow, nTab);
        ScRefCellValue aCell(rDoc, aPos);
        if (aCell.meType == CELLTYPE_EDIT)
        {
            const EditTextObject* pEditObj = aCell.mpEditText;
            pOldEditData = pEditObj->Clone();
            rDoc.RemoveEditTextCharAttribs(aPos, rAttr);
            pEditObj = rDoc.GetEditText(aPos);
            pNewEditData = pEditObj->Clone();
        }

        aChangeRanges.push_back(aPos);
        std::unique_ptr<ScPatternAttr> pOldPat(new ScPatternAttr(*rDoc.GetPattern( nCol, nRow, nTab )));

        rDoc.ApplyPattern( nCol, nRow, nTab, rAttr );

        const ScPatternAttr* pNewPat = rDoc.GetPattern( nCol, nRow, nTab );

        if (bRecord)
        {
            std::unique_ptr<ScUndoCursorAttr> pUndo(new ScUndoCursorAttr(
                pDocSh, nCol, nRow, nTab, pOldPat.get(), pNewPat, &rAttr ));
            pUndo->SetEditData(std::move(pOldEditData), std::move(pNewEditData));
            pDocSh->GetUndoManager()->AddUndoAction(std::move(pUndo));
        }
        pOldPat.reset();     // is copied in undo (Pool)

        pDocSh->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PaintPartFlags::Grid, nExtFlags | SC_PF_TESTMERGE );
        pDocSh->UpdateOle(&GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
    }

    ScModelObj* pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh);
    if (pModelObj)
    {
        css::uno::Sequence< css::beans::PropertyValue > aProperties;
        sal_Int32 nCount = 0;
        const SfxItemPropertyMap& rMap = ScCellObj::GetCellPropertyMap();
        PropertyEntryVector_t aPropVector = rMap.getPropertyEntries();
        for ( sal_uInt16 nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; ++nWhich )
        {
            const SfxPoolItem* pItem = nullptr;
            if ( rNewSet.GetItemState( nWhich, true, &pItem ) == SfxItemState::SET && pItem )
            {
                for ( const auto& rProp : aPropVector)
                {
                    if ( rProp.nWID == nWhich )
                    {
                        css::uno::Any aVal;
                        pItem->QueryValue( aVal, rProp.nMemberId );
                        aProperties.realloc( nCount + 1 );
                        aProperties[ nCount ].Name = rProp.sName;
                        aProperties[ nCount ].Value = aVal;
                        ++nCount;
                    }
                }
            }
        }
        HelperNotifyChanges::Notify(*pModelObj, aChangeRanges, "attribute", aProperties);
    }

    StartFormatArea();
}

void ScViewFunc::ApplyUserItemSet( const SfxItemSet& rItemSet )
{
    //  ItemSet from UI, may have different pool

    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aNewAttrs( GetViewData().GetDocument()->GetPool() );
    SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
    rNewSet.Put( rItemSet, false );
    ApplySelectionPattern( aNewAttrs );

    AdjustBlockHeight();
}

const SfxStyleSheet* ScViewFunc::GetStyleSheetFromMarked()
{
    // Don't use UnmarkFiltered in slot state functions, for performance reasons.
    // The displayed state is always that of the whole selection including filtered rows.

    const ScStyleSheet* pSheet      = nullptr;
    ScViewData&         rViewData   = GetViewData();
    ScDocument*         pDoc        = rViewData.GetDocument();
    ScMarkData&         rMark       = rViewData.GetMarkData();

    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
        pSheet = pDoc->GetSelectionStyle( rMark );                  // MarkToMulti isn't necessary
    else
        pSheet = pDoc->GetStyle( rViewData.GetCurX(),
                                 rViewData.GetCurY(),
                                 rViewData.GetTabNo() );

    return pSheet;
}

void ScViewFunc::SetStyleSheetToMarked( const SfxStyleSheet* pStyleSheet )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    if ( !pStyleSheet) return;

    ScViewData& rViewData   = GetViewData();
    ScDocShell* pDocSh      = rViewData.GetDocShell();
    ScDocument& rDoc        = pDocSh->GetDocument();
    ScMarkData aFuncMark( rViewData.GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, &rDoc );
    SCTAB nTabCount     = rDoc.GetTableCount();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScDocShellModificator aModificator( *pDocSh );

    if ( aFuncMark.IsMarked() || aFuncMark.IsMultiMarked() )
    {
        ScRange aMarkRange;
        aFuncMark.MarkToMulti();
        aFuncMark.GetMultiMarkArea( aMarkRange );

        if ( bRecord )
        {
            SCTAB nTab = rViewData.GetTabNo();
            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nTab, nTab );
            for (const auto& rTab : aFuncMark)
                if (rTab != nTab)
                    pUndoDoc->AddUndoTab( rTab, rTab );

            ScRange aCopyRange = aMarkRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            rDoc.CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, true, *pUndoDoc, &aFuncMark );
            aFuncMark.MarkToMulti();

            OUString aName = pStyleSheet->GetName();
            pDocSh->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoSelectionStyle>( pDocSh, aFuncMark, aMarkRange, aName, std::move(pUndoDoc) ) );
        }

        rDoc.ApplySelectionStyle( static_cast<const ScStyleSheet&>(*pStyleSheet), aFuncMark );

        if (!AdjustBlockHeight())
            rViewData.GetDocShell()->PostPaint( aMarkRange, PaintPartFlags::Grid );

        aFuncMark.MarkToSimple();
    }
    else
    {
        SCCOL nCol = rViewData.GetCurX();
        SCROW nRow = rViewData.GetCurY();
        SCTAB nTab = rViewData.GetTabNo();

        if ( bRecord )
        {
            ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndo( &rDoc, nTab, nTab );
            for (const auto& rTab : aFuncMark)
                if (rTab != nTab)
                    pUndoDoc->AddUndoTab( rTab, rTab );

            ScRange aCopyRange( nCol, nRow, 0, nCol, nRow, nTabCount-1 );
            rDoc.CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, false, *pUndoDoc );

            ScRange aMarkRange ( nCol, nRow, nTab );
            ScMarkData aUndoMark = aFuncMark;
            aUndoMark.SetMultiMarkArea( aMarkRange );

            OUString aName = pStyleSheet->GetName();
            pDocSh->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoSelectionStyle>( pDocSh, aUndoMark, aMarkRange, aName, std::move(pUndoDoc) ) );
        }

        for (const auto& rTab : aFuncMark)
            rDoc.ApplyStyle( nCol, nRow, rTab, static_cast<const ScStyleSheet&>(*pStyleSheet) );

        if (!AdjustBlockHeight())
            rViewData.GetDocShell()->PostPaintCell( nCol, nRow, nTab );

    }

    aModificator.SetDocumentModified();

    StartFormatArea();
}

void ScViewFunc::RemoveStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet )
{
    if ( !pStyleSheet) return;

    ScViewData& rViewData   = GetViewData();
    ScDocument* pDoc        = rViewData.GetDocument();
    ScDocShell* pDocSh      = rViewData.GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    ScopedVclPtrInstance< VirtualDevice > pVirtDev;
    pVirtDev->SetMapMode(MapMode(MapUnit::MapPixel));
    pDoc->StyleSheetChanged( pStyleSheet, true, pVirtDev,
                                rViewData.GetPPTX(),
                                rViewData.GetPPTY(),
                                rViewData.GetZoomX(),
                                rViewData.GetZoomY() );

    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Grid|PaintPartFlags::Left );
    aModificator.SetDocumentModified();

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if (pHdl)
        pHdl->ForgetLastPattern();
}

void ScViewFunc::UpdateStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet )
{
    if ( !pStyleSheet) return;

    ScViewData& rViewData   = GetViewData();
    ScDocument* pDoc        = rViewData.GetDocument();
    ScDocShell* pDocSh      = rViewData.GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    ScopedVclPtrInstance< VirtualDevice > pVirtDev;
    pVirtDev->SetMapMode(MapMode(MapUnit::MapPixel));
    pDoc->StyleSheetChanged( pStyleSheet, false, pVirtDev,
                                rViewData.GetPPTX(),
                                rViewData.GetPPTY(),
                                rViewData.GetZoomX(),
                                rViewData.GetZoomY() );

    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Grid|PaintPartFlags::Left );
    aModificator.SetDocumentModified();

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if (pHdl)
        pHdl->ForgetLastPattern();
}


void ScViewFunc::OnLOKInsertDeleteColumn(SCCOL nStartCol, long nOffset)
{
    if (!comphelper::LibreOfficeKit::isActive() || nOffset == 0)
        return;

    SCTAB nCurrentTabIndex = GetViewData().GetTabNo();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
        if (pTabViewShell)
        {
            pTabViewShell->GetViewData().GetLOKWidthHelper(nCurrentTabIndex)->invalidateByIndex(nStartCol);

            // if we remove a column the cursor position  and the current selection
            // in other views could need to be moved on the left by one column.
            if (pTabViewShell != this)
            {
                if (pTabViewShell->getPart() == nCurrentTabIndex)
                {
                    SCCOL nX = pTabViewShell->GetViewData().GetCurX();
                    if (nX > nStartCol || (nX == nStartCol && nOffset > 0))
                    {
                        ScInputHandler* pInputHdl = pTabViewShell->GetInputHandler();
                        SCROW nY = pTabViewShell->GetViewData().GetCurY();
                        pTabViewShell->SetCursor(nX + nOffset, nY);
                        if (pInputHdl && pInputHdl->IsInputMode())
                        {
                            pInputHdl->SetModified();
                        }
                    }

                    ScMarkData aMultiMark( pTabViewShell->GetViewData().GetMarkData() );
                    aMultiMark.SetMarking( false );
                    aMultiMark.MarkToMulti();
                    if (aMultiMark.IsMultiMarked())
                    {
                        aMultiMark.ShiftCols(nStartCol, nOffset);
                        pTabViewShell->SetMarkData(aMultiMark);
                    }
                }
                else
                {
                    SCROW nX = pTabViewShell->GetViewData().GetCurXForTab(nCurrentTabIndex);
                    if (nX > nStartCol || (nX == nStartCol && nOffset > 0))
                    {
                        pTabViewShell->GetViewData().SetCurXForTab(nX + nOffset, nCurrentTabIndex);
                    }
                }
            }
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void ScViewFunc::OnLOKInsertDeleteRow(SCROW nStartRow, long nOffset)
{
    if (!comphelper::LibreOfficeKit::isActive() || nOffset == 0)
        return;

    SCTAB nCurrentTabIndex = GetViewData().GetTabNo();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
        if (pTabViewShell)
        {
            pTabViewShell->GetViewData().GetLOKHeightHelper(nCurrentTabIndex)->invalidateByIndex(nStartRow);

            // if we remove a row the cursor position and the current selection
            // in other views could need to be moved up by one row.
            if (pTabViewShell != this)
            {
                if (pTabViewShell->getPart() == nCurrentTabIndex)
                {
                    SCROW nY = pTabViewShell->GetViewData().GetCurY();
                    if (nY > nStartRow || (nY == nStartRow && nOffset > 0))
                    {
                        ScInputHandler* pInputHdl = pTabViewShell->GetInputHandler();
                        SCCOL nX = pTabViewShell->GetViewData().GetCurX();
                        pTabViewShell->SetCursor(nX, nY + nOffset);
                        if (pInputHdl && pInputHdl->IsInputMode())
                        {
                            pInputHdl->SetModified();
                        }
                    }

                    ScMarkData aMultiMark( pTabViewShell->GetViewData().GetMarkData() );
                    aMultiMark.SetMarking( false );
                    aMultiMark.MarkToMulti();
                    if (aMultiMark.IsMultiMarked())
                    {
                        aMultiMark.ShiftRows(nStartRow, nOffset);
                        pTabViewShell->SetMarkData(aMultiMark);
                    }
                }
                else
                {
                    SCROW nY = pTabViewShell->GetViewData().GetCurYForTab(nCurrentTabIndex);
                    if (nY > nStartRow || (nY == nStartRow && nOffset > 0))
                    {
                        pTabViewShell->GetViewData().SetCurYForTab(nY + nOffset, nCurrentTabIndex);
                    }
                }
            }
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void ScViewFunc::OnLOKSetWidthOrHeight(SCCOLROW nStart, bool bWidth)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    SCTAB nCurTab = GetViewData().GetTabNo();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
        if (pTabViewShell)
        {
            if (bWidth)
                pTabViewShell->GetViewData().GetLOKWidthHelper(nCurTab)->invalidateByIndex(nStart);
            else
                pTabViewShell->GetViewData().GetLOKHeightHelper(nCurTab)->invalidateByIndex(nStart);
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

//  insert cells - undo OK

bool ScViewFunc::InsertCells( InsCellCmd eCmd, bool bRecord, bool bPartOfPaste )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        const ScMarkData& rMark = GetViewData().GetMarkData();
        bool bSuccess = pDocSh->GetDocFunc().InsertCells( aRange, &rMark, eCmd, bRecord, false, bPartOfPaste );
        if (bSuccess)
        {
            bool bInsertCols = ( eCmd == INS_INSCOLS_BEFORE || eCmd == INS_INSCOLS_AFTER);
            bool bInsertRows = ( eCmd == INS_INSROWS_BEFORE || eCmd == INS_INSROWS_AFTER );

            pDocSh->UpdateOle(&GetViewData());
            CellContentChanged();
            ResetAutoSpell();

            if ( bInsertCols || bInsertRows )
            {
                OUString aOperation = bInsertRows ?
                    OUString("insert-rows"):
                    OUString("insert-columns");
                HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, aRange, aOperation);
            }

            if (comphelper::LibreOfficeKit::isActive())
            {
                if (bInsertCols)
                    ScTabViewShell::notifyAllViewsHeaderInvalidation(COLUMN_HEADER, GetViewData().GetTabNo());

                if (bInsertRows)
                    ScTabViewShell::notifyAllViewsHeaderInvalidation(ROW_HEADER, GetViewData().GetTabNo());
            }
        }
        return bSuccess;
    }
    else
    {
        ErrorMessage(STR_NOMULTISELECT);
        return false;
    }
}

//  delete cells - undo OK

void ScViewFunc::DeleteCells( DelCellCmd eCmd )
{
    ScRange aRange;
    if ( GetViewData().GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        const ScMarkData& rMark = GetViewData().GetMarkData();

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
        // #i94841# [Collaboration] if deleting rows is rejected, the content is sometimes wrong
        if ( pDocSh->IsDocShared() && ( eCmd == DelCellCmd::Rows || eCmd == DelCellCmd::Cols ) )
        {
            ScRange aDelRange( aRange.aStart );
            SCCOLROW nCount = 0;
            if ( eCmd == DelCellCmd::Rows )
            {
                nCount = sal::static_int_cast< SCCOLROW >( aRange.aEnd.Row() - aRange.aStart.Row() + 1 );
            }
            else
            {
                nCount = sal::static_int_cast< SCCOLROW >( aRange.aEnd.Col() - aRange.aStart.Col() + 1 );
            }
            while ( nCount > 0 )
            {
                pDocSh->GetDocFunc().DeleteCells( aDelRange, &rMark, eCmd, false );
                --nCount;
            }
        }
        else
#endif
        {
            pDocSh->GetDocFunc().DeleteCells( aRange, &rMark, eCmd, false );
        }

        pDocSh->UpdateOle(&GetViewData());
        CellContentChanged();
        ResetAutoSpell();

        if ( eCmd == DelCellCmd::Rows || eCmd == DelCellCmd::Cols )
        {
            OUString aOperation = ( eCmd == DelCellCmd::Rows) ?
              OUString("delete-rows"):
              OUString("delete-columns");
            HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, aRange, aOperation);
        }

        //  put cursor directly behind deleted range
        SCCOL nCurX = GetViewData().GetCurX();
        SCROW nCurY = GetViewData().GetCurY();
        if ( eCmd==DelCellCmd::CellsLeft || eCmd==DelCellCmd::Cols )
            nCurX = aRange.aStart.Col();
        else
            nCurY = aRange.aStart.Row();
        SetCursor( nCurX, nCurY );

        if (comphelper::LibreOfficeKit::isActive())
        {
            if (eCmd == DelCellCmd::Cols)
                ScTabViewShell::notifyAllViewsHeaderInvalidation(COLUMN_HEADER, GetViewData().GetTabNo());

            if (eCmd == DelCellCmd::Rows)
                ScTabViewShell::notifyAllViewsHeaderInvalidation(ROW_HEADER, GetViewData().GetTabNo());
        }
    }
    else
    {
        if (eCmd == DelCellCmd::Cols)
            DeleteMulti( false );
        else if (eCmd == DelCellCmd::Rows)
            DeleteMulti( true );
        else
            ErrorMessage(STR_NOMULTISELECT);
    }

    Unmark();
}

void ScViewFunc::DeleteMulti( bool bRows )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData aFuncMark( GetViewData().GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, &rDoc );

    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    std::vector<sc::ColRowSpan> aSpans;
    if (bRows)
        aSpans = aFuncMark.GetMarkedRowSpans();
    else
        aSpans = aFuncMark.GetMarkedColSpans();

    if (aSpans.empty())
    {
        SCCOLROW nCurPos = bRows ? GetViewData().GetCurY() : GetViewData().GetCurX();
        aSpans.emplace_back(nCurPos, nCurPos);
    }

    //  test if allowed

    const char* pErrorId = nullptr;
    bool bNeedRefresh = false;
    for (size_t i = 0, n = aSpans.size(); i < n && !pErrorId; ++i)
    {
        SCCOLROW nStart = aSpans[i].mnStart;
        SCCOLROW nEnd = aSpans[i].mnEnd;

        SCCOL nStartCol, nEndCol;
        SCROW nStartRow, nEndRow;
        if ( bRows )
        {
            nStartCol = 0;
            nEndCol   = MAXCOL;
            nStartRow = static_cast<SCROW>(nStart);
            nEndRow   = static_cast<SCROW>(nEnd);
        }
        else
        {
            nStartCol = static_cast<SCCOL>(nStart);
            nEndCol   = static_cast<SCCOL>(nEnd);
            nStartRow = 0;
            nEndRow   = MAXROW;
        }

        // cell protection (only needed for first range, as all following cells are moved)
        if (i == 0)
        {
            // test to the end of the sheet
            ScEditableTester aTester( &rDoc, nTab, nStartCol, nStartRow, MAXCOL, MAXROW );
            if (!aTester.IsEditable())
                pErrorId = aTester.GetMessageId();
        }

        // merged cells
        SCCOL nMergeStartX = nStartCol;
        SCROW nMergeStartY = nStartRow;
        SCCOL nMergeEndX   = nEndCol;
        SCROW nMergeEndY   = nEndRow;
        rDoc.ExtendMerge( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );
        rDoc.ExtendOverlapped( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );

        if ( nMergeStartX != nStartCol || nMergeStartY != nStartRow )
        {
            // Disallow deleting parts of a merged cell.
            // Deleting the start is allowed (merge is removed), so the end doesn't have to be checked.

            pErrorId = STR_MSSG_DELETECELLS_0;
        }
        if ( nMergeEndX != nEndCol || nMergeEndY != nEndRow )
        {
            // detect if the start of a merged cell is deleted, so the merge flags can be refreshed

            bNeedRefresh = true;
        }
    }

    if (pErrorId)
    {
        ErrorMessage(pErrorId);
        return;
    }

    //  proceed

    WaitObject aWait( GetFrameWin() );      // important for TrackFormulas in UpdateReference

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;
    if (bRecord)
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nTab, nTab, !bRows, bRows );      // row height

        for (sc::ColRowSpan & rSpan : aSpans)
        {
            SCCOLROW nStart = rSpan.mnStart;
            SCCOLROW nEnd = rSpan.mnEnd;
            if (bRows)
                rDoc.CopyToDocument( 0,nStart,nTab, MAXCOL,nEnd,nTab, InsertDeleteFlags::ALL,false,*pUndoDoc );
            else
                rDoc.CopyToDocument( static_cast<SCCOL>(nStart),0,nTab,
                        static_cast<SCCOL>(nEnd),MAXROW,nTab,
                        InsertDeleteFlags::ALL,false,*pUndoDoc );
        }

        //  all Formulas because of references
        SCTAB nTabCount = rDoc.GetTableCount();
        pUndoDoc->AddUndoTab( 0, nTabCount-1 );
        rDoc.CopyToDocument( 0,0,0, MAXCOL,MAXROW,MAXTAB, InsertDeleteFlags::FORMULA,false,*pUndoDoc );

        pUndoData.reset(new ScRefUndoData( &rDoc ));

        rDoc.BeginDrawUndo();
    }

    std::vector<sc::ColRowSpan>::const_reverse_iterator ri = aSpans.rbegin(), riEnd = aSpans.rend();
    for (; ri != riEnd; ++ri)
    {
        SCCOLROW nEnd = ri->mnEnd;
        SCCOLROW nStart = ri->mnStart;

        if (bRows)
            rDoc.DeleteRow( 0,nTab, MAXCOL,nTab, nStart, static_cast<SCSIZE>(nEnd-nStart+1) );
        else
            rDoc.DeleteCol( 0,nTab, MAXROW,nTab, static_cast<SCCOL>(nStart), static_cast<SCSIZE>(nEnd-nStart+1) );
    }

    if (bNeedRefresh)
    {
        SCCOLROW nFirstStart = aSpans[0].mnStart;
        SCCOL nStartCol = bRows ? 0 : static_cast<SCCOL>(nFirstStart);
        SCROW nStartRow = bRows ? static_cast<SCROW>(nFirstStart) : 0;
        SCCOL nEndCol = MAXCOL;
        SCROW nEndRow = MAXROW;

        rDoc.RemoveFlagsTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, ScMF::Hor | ScMF::Ver );
        rDoc.ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab, true );
    }

    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDeleteMulti>(
                pDocSh, bRows, bNeedRefresh, nTab, aSpans, std::move(pUndoDoc), std::move(pUndoData)));
    }

    if (!AdjustRowHeight(0, MAXROW))
    {
        if (bRows)
        {
            pDocSh->PostPaint(
                0, aSpans[0].mnStart, nTab,
                MAXCOL, MAXROW, nTab, (PaintPartFlags::Grid | PaintPartFlags::Left));
        }
        else
        {
            pDocSh->PostPaint(
                static_cast<SCCOL>(aSpans[0].mnStart), 0, nTab,
                MAXCOL, MAXROW, nTab, (PaintPartFlags::Grid | PaintPartFlags::Top));
        }
    }

    ResetAutoSpell();
    aModificator.SetDocumentModified();

    CellContentChanged();

    //  put cursor directly behind the first deleted range
    SCCOL nCurX = GetViewData().GetCurX();
    SCROW nCurY = GetViewData().GetCurY();
    if ( bRows )
        nCurY = aSpans[0].mnStart;
    else
        nCurX = static_cast<SCCOL>(aSpans[0].mnStart);
    SetCursor( nCurX, nCurY );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );
}

//  delete contents

void ScViewFunc::DeleteContents( InsertDeleteFlags nFlags )
{
    ScViewData& rViewData = GetViewData();
    rViewData.SetPasteMode( ScPasteFlags::NONE );
    rViewData.GetViewShell()->UpdateCopySourceOverlay();

    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    bool bEditable = SelectionEditable( &bOnlyNotBecauseOfMatrix );
    if ( !bEditable )
    {
        if ( !(bOnlyNotBecauseOfMatrix &&
                ((nFlags & (InsertDeleteFlags::ATTRIB | InsertDeleteFlags::EDITATTR)) == nFlags)) )
        {
            ErrorMessage(bOnlyNotBecauseOfMatrix ? STR_MATRIXFRAGMENTERR : STR_PROTECTIONERR);
            return;
        }
    }

    ScRange aMarkRange;
    bool bSimple = false;

    ScDocument* pDoc = GetViewData().GetDocument();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScMarkData aFuncMark( GetViewData().GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );

    bool bRecord =true;
    if (!pDoc->IsUndoEnabled())
        bRecord = false;

    if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
    {
        aMarkRange.aStart.SetCol(GetViewData().GetCurX());
        aMarkRange.aStart.SetRow(GetViewData().GetCurY());
        aMarkRange.aStart.SetTab(GetViewData().GetTabNo());
        aMarkRange.aEnd = aMarkRange.aStart;
        if ( pDoc->HasAttrib( aMarkRange, HasAttrFlags::Merged ) )
        {
            aFuncMark.SetMarkArea( aMarkRange );
        }
        else
            bSimple = true;
    }

    HideAllCursors();   // for if summary is cancelled

    ScDocFunc& rDocFunc = pDocSh->GetDocFunc();
    if (bSimple)
        rDocFunc.DeleteCell(aMarkRange.aStart, aFuncMark, nFlags, bRecord);
    else
        rDocFunc.DeleteContents(aFuncMark, nFlags, bRecord, false);

    pDocSh->UpdateOle(&GetViewData());

    if (ScModelObj *pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh))
    {
        ScRangeList aChangeRanges;
        if ( bSimple )
        {
            aChangeRanges.push_back( aMarkRange );
        }
        else
        {
            aFuncMark.FillRangeListWithMarks( &aChangeRanges, false );
        }
        HelperNotifyChanges::Notify(*pModelObj, aChangeRanges);
    }

    CellContentChanged();
    ShowAllCursors();

    if ( nFlags & InsertDeleteFlags::ATTRIB )
    {
        if ( nFlags & InsertDeleteFlags::CONTENTS )
            bFormatValid = false;
        else
            StartFormatArea();              // delete attribute is also attribute-change
    }
}

//  column width/row height (via header) - undo OK

void ScViewFunc::SetWidthOrHeight(
    bool bWidth, const std::vector<sc::ColRowSpan>& rRanges, ScSizeMode eMode,
    sal_uInt16 nSizeTwips, bool bRecord, const ScMarkData* pMarkData )
{
    if (rRanges.empty())
        return;

    // Use view's mark if none specified, but do not modify the original data,
    // i.e. no MarkToMulti() on that.
    ScMarkData aMarkData( pMarkData ? *pMarkData : GetViewData().GetMarkData());

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCCOL nCurX = GetViewData().GetCurX();
    SCROW nCurY = GetViewData().GetCurY();
    SCTAB nFirstTab = aMarkData.GetFirstSelected();
    SCTAB nCurTab = GetViewData().GetTabNo();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDocShellModificator aModificator( *pDocSh );

    bool bAllowed = true;
    for (const SCTAB& nTab : aMarkData)
    {
        bAllowed = std::all_of(rRanges.begin(), rRanges.end(),
            [&bWidth, &rDoc, &nTab](const sc::ColRowSpan& rRange) {
                bool bOnlyMatrix;
                bool bIsBlockEditable;
                if (bWidth)
                    bIsBlockEditable = rDoc.IsBlockEditable(nTab, rRange.mnStart, 0, rRange.mnEnd, MAXROW, &bOnlyMatrix);
                else
                    bIsBlockEditable = rDoc.IsBlockEditable(nTab, 0, rRange.mnStart, MAXCOL, rRange.mnEnd, &bOnlyMatrix);
                return bIsBlockEditable || bOnlyMatrix;
            });
        if (!bAllowed)
            break;
    }

    // Allow users to resize cols/rows in readonly docs despite the r/o state.
    // It is frustrating to be unable to see content in mis-sized cells.
    if( !bAllowed && !pDocSh->IsReadOnly() )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    SCCOLROW nStart = rRanges.front().mnStart;
    SCCOLROW nEnd = rRanges.back().mnEnd;

    OnLOKSetWidthOrHeight(nStart, bWidth);

    bool bFormula = false;
    if ( eMode == SC_SIZE_OPTIMAL )
    {
        const ScViewOptions& rOpts = GetViewData().GetOptions();
        bFormula = rOpts.GetOption( VOPT_FORMULAS );
    }

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScOutlineTable> pUndoTab;
    std::vector<sc::ColRowSpan> aUndoRanges;

    if ( bRecord )
    {
        rDoc.BeginDrawUndo();                          // Drawing Updates

        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        for (const SCTAB& nTab : aMarkData)
        {
            if (bWidth)
            {
                if ( nTab == nFirstTab )
                    pUndoDoc->InitUndo( &rDoc, nTab, nTab, true );
                else
                    pUndoDoc->AddUndoTab( nTab, nTab, true );
                rDoc.CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                        static_cast<SCCOL>(nEnd), MAXROW, nTab, InsertDeleteFlags::NONE,
                        false, *pUndoDoc );
            }
            else
            {
                if ( nTab == nFirstTab )
                    pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );
                else
                    pUndoDoc->AddUndoTab( nTab, nTab, false, true );
                rDoc.CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
            }
        }

        aUndoRanges = rRanges;

        //! outlines from all tab?
        ScOutlineTable* pTable = rDoc.GetOutlineTable( nCurTab );
        if (pTable)
            pUndoTab.reset(new ScOutlineTable( *pTable ));
    }

    if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
        aMarkData.MarkToMulti();

    bool bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    bool bOutline = false;

    for (const SCTAB& nTab : aMarkData)
    {
        for (const sc::ColRowSpan & rRange : rRanges)
        {
            SCCOLROW nStartNo = rRange.mnStart;
            SCCOLROW nEndNo = rRange.mnEnd;

            if ( !bWidth )                      // height always blockwise
            {
                if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                {
                    bool bAll = ( eMode==SC_SIZE_OPTIMAL );
                    if (!bAll)
                    {
                        //  delete CRFlags::ManualSize for all in range,
                        //  then SetOptimalHeight with bShrink = FALSE
                        for (SCROW nRow = nStartNo; nRow <= nEndNo; ++nRow)
                        {
                            SCROW nLastRow = nRow;
                            if (rDoc.RowHidden(nRow, nTab, nullptr, &nLastRow))
                            {
                                nRow = nLastRow;
                                continue;
                            }

                            CRFlags nOld = rDoc.GetRowFlags(nRow, nTab);
                            if (nOld & CRFlags::ManualSize)
                                rDoc.SetRowFlags(nRow, nTab, nOld & ~CRFlags::ManualSize);
                        }
                    }

                    double nPPTX = GetViewData().GetPPTX();
                    double nPPTY = GetViewData().GetPPTY();
                    Fraction aZoomX = GetViewData().GetZoomX();
                    Fraction aZoomY = GetViewData().GetZoomY();

                    ScSizeDeviceProvider aProv(pDocSh);
                    if (aProv.IsPrinter())
                    {
                        nPPTX = aProv.GetPPTX();
                        nPPTY = aProv.GetPPTY();
                        aZoomX = aZoomY = Fraction( 1, 1 );
                    }

                    sc::RowHeightContext aCxt(nPPTX, nPPTY, aZoomX, aZoomY, aProv.GetDevice());
                    aCxt.setForceAutoSize(bAll);
                    aCxt.setExtraHeight(nSizeTwips);
                    rDoc.SetOptimalHeight(aCxt, nStartNo, nEndNo, nTab);
                    if (bAll)
                        rDoc.ShowRows( nStartNo, nEndNo, nTab, true );

                    //  Manual-Flag already (re)set in SetOptimalHeight in case of bAll=sal_True
                    //  (set for Extra-Height, else reset).
                }
                else if ( eMode==SC_SIZE_DIRECT )
                {
                    if (nSizeTwips)
                    {
                        rDoc.SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
                        rDoc.SetManualHeight( nStartNo, nEndNo, nTab, true );          // height was set manually
                    }

                    rDoc.ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );

                    if (!bShow && nStartNo <= nCurY && nCurY <= nEndNo && nTab == nCurTab)
                    {
                        nCurY = -1;
                    }
                }
                else if ( eMode==SC_SIZE_SHOW )
                {
                    rDoc.ShowRows( nStartNo, nEndNo, nTab, true );
                }
            }
            else                                // column width
            {
                for (SCCOL nCol=static_cast<SCCOL>(nStartNo); nCol<=static_cast<SCCOL>(nEndNo); nCol++)
                {
                    if ( eMode != SC_SIZE_VISOPT || !rDoc.ColHidden(nCol, nTab) )
                    {
                        sal_uInt16 nThisSize = nSizeTwips;

                        if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                            nThisSize = nSizeTwips + GetOptimalColWidth( nCol, nTab, bFormula );
                        if ( nThisSize )
                            rDoc.SetColWidth( nCol, nTab, nThisSize );

                        rDoc.ShowCol( nCol, nTab, bShow );

                        if (!bShow && nCol == nCurX && nTab == nCurTab)
                        {
                            nCurX = -1;
                        }
                    }
                }
            }

            //  adjust outline
            if (bWidth)
            {
                if ( rDoc.UpdateOutlineCol( static_cast<SCCOL>(nStartNo),
                            static_cast<SCCOL>(nEndNo), nTab, bShow ) )
                    bOutline = true;
            }
            else
            {
                if ( rDoc.UpdateOutlineRow( nStartNo, nEndNo, nTab, bShow ) )
                    bOutline = true;
            }
        }
        rDoc.SetDrawPageSize(nTab);
    }

    if (!bOutline)
        pUndoTab.reset();

    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoWidthOrHeight>(
                pDocSh, aMarkData, nStart, nCurTab, nEnd, nCurTab,
                std::move(pUndoDoc), aUndoRanges, std::move(pUndoTab), eMode, nSizeTwips, bWidth));
    }

    if (nCurX < 0)
    {
        MoveCursorRel( 1, 0, SC_FOLLOW_LINE, false );
    }

    if (nCurY < 0)
    {
        MoveCursorRel( 0, 1, SC_FOLLOW_LINE, false );
    }

    // fdo#36247 Ensure that the drawing layer's map mode scaling factors match
    // the new heights and widths.
    GetViewData().GetView()->RefreshZoom();

    for (const SCTAB& nTab : aMarkData)
        rDoc.UpdatePageBreaks( nTab );

    GetViewData().GetView()->UpdateScrollBars(bWidth ? COLUMN_HEADER : ROW_HEADER);

    {
        for (const SCTAB& nTab : aMarkData)
        {
            if (bWidth)
            {
                if (rDoc.HasAttrib( static_cast<SCCOL>(nStart),0,nTab,
                            static_cast<SCCOL>(nEnd),MAXROW,nTab,
                            HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
                    nStart = 0;
                if (nStart > 0)             // go upwards because of Lines and cursor
                    --nStart;
                pDocSh->PostPaint( static_cast<SCCOL>(nStart), 0, nTab,
                        MAXCOL, MAXROW, nTab, PaintPartFlags::Grid | PaintPartFlags::Top );
            }
            else
            {
                if (rDoc.HasAttrib( 0,nStart,nTab, MAXCOL,nEnd,nTab, HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
                    nStart = 0;
                if (nStart != 0)
                    --nStart;
                pDocSh->PostPaint( 0, nStart, nTab, MAXCOL, MAXROW, nTab, PaintPartFlags::Grid | PaintPartFlags::Left );
            }
        }

        pDocSh->UpdateOle(&GetViewData());
        if( !pDocSh->IsReadOnly() )
        aModificator.SetDocumentModified();
    }

    if ( bWidth )
    {
        if (ScModelObj* pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh))
        {
            ScRangeList aChangeRanges;
            for (const SCTAB& nTab : aMarkData)
            {
                for (const sc::ColRowSpan & rRange : rRanges)
                {
                    SCCOL nStartCol = rRange.mnStart;
                    SCCOL nEndCol   = rRange.mnEnd;
                    for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
                    {
                        aChangeRanges.push_back( ScRange( nCol, 0, nTab ) );
                    }
                }
            }
            HelperNotifyChanges::Notify(*pModelObj, aChangeRanges, "column-resize");
        }
    }
}

//  column width/row height (via marked range)

void ScViewFunc::SetMarkedWidthOrHeight( bool bWidth, ScSizeMode eMode, sal_uInt16 nSizeTwips )
{
    ScMarkData& rMark = GetViewData().GetMarkData();

    rMark.MarkToMulti();
    if (!rMark.IsMultiMarked())
    {
        SCCOL nCol = GetViewData().GetCurX();
        SCROW nRow = GetViewData().GetCurY();
        SCTAB nTab = GetViewData().GetTabNo();
        DoneBlockMode();
        InitOwnBlockMode();
        rMark.SetMultiMarkArea( ScRange( nCol,nRow,nTab ) );
        MarkDataChanged();
    }

    std::vector<sc::ColRowSpan> aRanges =
        bWidth ? rMark.GetMarkedColSpans() : rMark.GetMarkedRowSpans();

    SetWidthOrHeight(bWidth, aRanges, eMode, nSizeTwips);

    rMark.MarkToSimple();
}

void ScViewFunc::ModifyCellSize( ScDirection eDir, bool bOptimal )
{
    //! step size adjustable
    //  step size is also minimum
    sal_uInt16 nStepX = STD_COL_WIDTH / 5;
    sal_uInt16 nStepY = ScGlobal::nStdRowHeight;

    ScModule* pScMod = SC_MOD();
    bool bAnyEdit = pScMod->IsInputMode();
    SCCOL nCol = GetViewData().GetCurX();
    SCROW nRow = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();

    bool bAllowed, bOnlyMatrix;
    if ( eDir == DIR_LEFT || eDir == DIR_RIGHT )
        bAllowed = rDoc.IsBlockEditable( nTab, nCol,0, nCol,MAXROW, &bOnlyMatrix );
    else
        bAllowed = rDoc.IsBlockEditable( nTab, 0,nRow, MAXCOL,nRow, &bOnlyMatrix );
    if ( !bAllowed && !bOnlyMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    HideAllCursors();

    sal_uInt16 nWidth = rDoc.GetColWidth( nCol, nTab );
    sal_uInt16 nHeight = rDoc.GetRowHeight( nRow, nTab );
    std::vector<sc::ColRowSpan> aRange(1, sc::ColRowSpan(0,0));
    if ( eDir == DIR_LEFT || eDir == DIR_RIGHT )
    {
        if (bOptimal)               // width of this single cell
        {
            if ( bAnyEdit )
            {
                //  when editing the actual entered width
                ScInputHandler* pHdl = pScMod->GetInputHdl( GetViewData().GetViewShell() );
                if (pHdl)
                {
                    long nEdit = pHdl->GetTextSize().Width();       // in 0.01 mm

                    const ScPatternAttr* pPattern = rDoc.GetPattern( nCol, nRow, nTab );
                    const SvxMarginItem& rMItem = pPattern->GetItem(ATTR_MARGIN);
                    sal_uInt16 nMargin = rMItem.GetLeftMargin() + rMItem.GetRightMargin();
                    if ( pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue() == SvxCellHorJustify::Left )
                        nMargin = sal::static_int_cast<sal_uInt16>(
                            nMargin + pPattern->GetItem(ATTR_INDENT).GetValue() );

                    nWidth = static_cast<sal_uInt16>(nEdit * pDocSh->GetOutputFactor() / HMM_PER_TWIPS)
                                + nMargin + STD_EXTRA_WIDTH;
                }
            }
            else
            {
                double nPPTX = GetViewData().GetPPTX();
                double nPPTY = GetViewData().GetPPTY();
                Fraction aZoomX = GetViewData().GetZoomX();
                Fraction aZoomY = GetViewData().GetZoomY();

                ScSizeDeviceProvider aProv(pDocSh);
                if (aProv.IsPrinter())
                {
                    nPPTX = aProv.GetPPTX();
                    nPPTY = aProv.GetPPTY();
                    aZoomX = aZoomY = Fraction( 1, 1 );
                }

                long nPixel = rDoc.GetNeededSize( nCol, nRow, nTab, aProv.GetDevice(),
                                            nPPTX, nPPTY, aZoomX, aZoomY, true );
                sal_uInt16 nTwips = static_cast<sal_uInt16>( nPixel / nPPTX );
                if (nTwips != 0)
                    nWidth = nTwips + STD_EXTRA_WIDTH;
                else
                    nWidth = STD_COL_WIDTH;
            }
        }
        else                        // increment / decrement
        {
            if ( eDir == DIR_RIGHT )
                nWidth = sal::static_int_cast<sal_uInt16>( nWidth + nStepX );
            else if ( nWidth > nStepX )
                nWidth = sal::static_int_cast<sal_uInt16>( nWidth - nStepX );
            if ( nWidth < nStepX ) nWidth = nStepX;
            if ( nWidth > MAX_COL_WIDTH ) nWidth = MAX_COL_WIDTH;
        }
        aRange[0].mnStart = nCol;
        aRange[0].mnEnd = nCol;
        SetWidthOrHeight(true, aRange, SC_SIZE_DIRECT, nWidth);

        //  adjust height of this row if width demands/allows this

        if (!bAnyEdit)
        {
            const ScPatternAttr* pPattern = rDoc.GetPattern( nCol, nRow, nTab );
            bool bNeedHeight =
                    pPattern->GetItem( ATTR_LINEBREAK ).GetValue() ||
                    pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue() == SvxCellHorJustify::Block;
            if (bNeedHeight)
                AdjustRowHeight( nRow, nRow );
        }
    }
    else
    {
        ScSizeMode eMode;
        if (bOptimal)
        {
            eMode = SC_SIZE_OPTIMAL;
            nHeight = 0;
        }
        else
        {
            eMode = SC_SIZE_DIRECT;
            if ( eDir == DIR_BOTTOM )
                nHeight = sal::static_int_cast<sal_uInt16>( nHeight + nStepY );
            else if ( nHeight > nStepY )
                nHeight = sal::static_int_cast<sal_uInt16>( nHeight - nStepY );
            if ( nHeight < nStepY ) nHeight = nStepY;
            if ( nHeight > MAX_ROW_HEIGHT ) nHeight = MAX_ROW_HEIGHT;
        }
        aRange[0].mnStart = nRow;
        aRange[0].mnEnd = nRow;
        SetWidthOrHeight(false, aRange, eMode, nHeight);
    }

    if ( bAnyEdit )
    {
        UpdateEditView();
        if ( rDoc.HasAttrib( nCol, nRow, nTab, nCol, nRow, nTab, HasAttrFlags::NeedHeight ) )
        {
            ScInputHandler* pHdl = pScMod->GetInputHdl( GetViewData().GetViewShell() );
            if (pHdl)
                pHdl->SetModified();    // so that the height is adjusted with Enter
        }
    }

    ShowAllCursors();
}

void ScViewFunc::ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect )
{
    if (nTab == TABLEID_DOC)
        return;

    ScMarkData& rMark = GetViewData().GetMarkData();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    bool bUndo(rDoc.IsUndoEnabled());

    //  modifying several tabs is handled here

    if (bUndo)
    {
        OUString aUndo = ScResId( STR_UNDO_PROTECT_TAB );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId() );
    }

    for (const auto& rTab : rMark)
    {
        rFunc.ProtectSheet(rTab, rProtect);
        SetTabProtectionSymbol(rTab, true);
    }

    if (bUndo)
        pDocSh->GetUndoManager()->LeaveListAction();

    UpdateLayerLocks();         //! broadcast to all views
}

void ScViewFunc::Protect( SCTAB nTab, const OUString& rPassword )
{
    ScMarkData& rMark = GetViewData().GetMarkData();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    bool bUndo(rDoc.IsUndoEnabled());

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
        rFunc.Protect( nTab, rPassword );
    else
    {
        //  modifying several tabs is handled here

        if (bUndo)
        {
            OUString aUndo = ScResId( STR_UNDO_PROTECT_TAB );
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId() );
        }

        for (const auto& rTab : rMark)
            rFunc.Protect( rTab, rPassword );

        if (bUndo)
            pDocSh->GetUndoManager()->LeaveListAction();
    }

    UpdateLayerLocks();         //! broadcast to all views
}

bool ScViewFunc::Unprotect( SCTAB nTab, const OUString& rPassword )
{
    ScMarkData& rMark = GetViewData().GetMarkData();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    bool bChanged = false;
    bool bUndo (rDoc.IsUndoEnabled());

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
    {
        bChanged = rFunc.Unprotect( nTab, rPassword, false );
        if (bChanged && nTab != TABLEID_DOC)
            SetTabProtectionSymbol(nTab, false);
    }
    else
    {
        //  modifying several tabs is handled here

        if (bUndo)
        {
            OUString aUndo = ScResId( STR_UNDO_UNPROTECT_TAB );
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId() );
        }

        for (const auto& rTab : rMark)
        {
            if ( rFunc.Unprotect( rTab, rPassword, false ) )
            {
                bChanged = true;
                SetTabProtectionSymbol( rTab, false);
            }
        }

        if (bUndo)
            pDocSh->GetUndoManager()->LeaveListAction();
    }

    if (bChanged)
        UpdateLayerLocks();     //! broadcast to all views

    return bChanged;
}

void ScViewFunc::SetNoteText( const ScAddress& rPos, const OUString& rNoteText )
{
    GetViewData().GetDocShell()->GetDocFunc().SetNoteText( rPos, rNoteText, false );
}

void ScViewFunc::ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate )
{
    GetViewData().GetDocShell()->GetDocFunc().ReplaceNote( rPos, rNoteText, pAuthor, pDate, false );
}

void ScViewFunc::SetNumberFormat( SvNumFormatType nFormatType, sal_uLong nAdd )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    sal_uInt32          nNumberFormat = 0;
    ScViewData&         rViewData = GetViewData();
    ScDocument*         pDoc = rViewData.GetDocument();
    SvNumberFormatter*  pNumberFormatter = pDoc->GetFormatTable();
    LanguageType        eLanguage = ScGlobal::eLnge;
    ScPatternAttr       aNewAttrs( pDoc->GetPool() );

    //  always take language from cursor position, even if there is a selection

    sal_uInt32 nCurrentNumberFormat;
    pDoc->GetNumberFormat( rViewData.GetCurX(),
                           rViewData.GetCurY(),
                           rViewData.GetTabNo(),
                           nCurrentNumberFormat );
    const SvNumberformat* pEntry = pNumberFormatter->GetEntry( nCurrentNumberFormat );
    if (pEntry)
        eLanguage = pEntry->GetLanguage();      // else keep ScGlobal::eLnge

    nNumberFormat = pNumberFormatter->GetStandardFormat( nFormatType, eLanguage ) + nAdd;

    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumberFormat ) );
    //  ATTR_LANGUAGE_FORMAT not
    ApplySelectionPattern( aNewAttrs );
}

void ScViewFunc::SetNumFmtByStr( const OUString& rCode )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScViewData&         rViewData = GetViewData();
    ScDocument*         pDoc = rViewData.GetDocument();
    SvNumberFormatter*  pFormatter = pDoc->GetFormatTable();

    //  language always from cursor position

    sal_uInt32 nCurrentNumberFormat;
    pDoc->GetNumberFormat( rViewData.GetCurX(), rViewData.GetCurY(),
                           rViewData.GetTabNo(), nCurrentNumberFormat );
    const SvNumberformat* pEntry = pFormatter->GetEntry( nCurrentNumberFormat );
    LanguageType eLanguage = pEntry ? pEntry->GetLanguage() : ScGlobal::eLnge;

    //  determine index for String

    bool bOk = true;
    sal_uInt32 nNumberFormat = pFormatter->GetEntryKey( rCode, eLanguage );
    if ( nNumberFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        //  enter new

        OUString    aFormat = rCode;    // will be changed
        sal_Int32   nErrPos = 0;
        SvNumFormatType nType   = SvNumFormatType::ALL;        //! ???
        bOk = pFormatter->PutEntry( aFormat, nErrPos, nType, nNumberFormat, eLanguage );
    }

    if ( bOk )          // valid format?
    {
        ScPatternAttr aNewAttrs( pDoc->GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumberFormat ) );
        rSet.Put( SvxLanguageItem( eLanguage, ATTR_LANGUAGE_FORMAT ) );
        ApplySelectionPattern( aNewAttrs );
    }

    //! else return error / issue warning ???
}

void ScViewFunc::ChangeNumFmtDecimals( bool bIncrement )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScDocument*         pDoc = GetViewData().GetDocument();
    SvNumberFormatter*  pFormatter = pDoc->GetFormatTable();

    SCCOL nCol = GetViewData().GetCurX();
    SCROW nRow = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();

    sal_uInt32 nOldFormat;
    pDoc->GetNumberFormat( nCol, nRow, nTab, nOldFormat );
    const SvNumberformat* pOldEntry = pFormatter->GetEntry( nOldFormat );
    if (!pOldEntry)
    {
        OSL_FAIL("numberformat not found !!!");
        return;
    }

    //  what have we got here?

    sal_uInt32 nNewFormat = nOldFormat;
    bool bError = false;

    LanguageType eLanguage = pOldEntry->GetLanguage();
    bool bThousand, bNegRed;
    sal_uInt16 nPrecision, nLeading;
    pOldEntry->GetFormatSpecialInfo( bThousand, bNegRed, nPrecision, nLeading );

    SvNumFormatType nOldType = pOldEntry->GetType();
    if ( SvNumFormatType::ALL == ( nOldType & (
                SvNumFormatType::NUMBER |  SvNumFormatType::CURRENCY | SvNumFormatType::PERCENT | SvNumFormatType::SCIENTIFIC ) ) )
    {
        //  date, time, fraction, logical, text can not be changed
        bError = true;
    }

    //! SvNumberformat has a Member bStandard, but doesn't disclose it
    bool bWasStandard = ( nOldFormat == pFormatter->GetStandardIndex( eLanguage ) );
    OUString sExponentialStandardFormat = "";
    if (bWasStandard)
    {
        //  with "Standard" the decimal places depend on cell content
        //  0 if empty or text -> no decimal places
        double nVal = pDoc->GetValue( ScAddress( nCol, nRow, nTab ) );

        //  the ways of the Numberformatters are unfathomable, so try:
        OUString aOut;
        Color* pCol;
        const_cast<SvNumberformat*>(pOldEntry)->GetOutputString( nVal, aOut, &pCol );

        nPrecision = 0;
        // 'E' for exponential is fixed in Numberformatter
        sal_Int32 nIndexE = aOut.indexOf('E');
        if ( nIndexE >= 0 )
        {
          sExponentialStandardFormat = aOut.copy( nIndexE ).replace( '-', '+' );
          for ( sal_Int32 i=1 ; i<sExponentialStandardFormat.getLength() ; i++ )
          {
            if ( sExponentialStandardFormat[i] >= '1' && sExponentialStandardFormat[i] <= '9' )
              sExponentialStandardFormat = sExponentialStandardFormat.replaceAt( i, 1, "0" );
          }
          aOut = aOut.copy( 0, nIndexE ); // remove exponential part
        }
        OUString aDecSep( pFormatter->GetFormatDecimalSep( nOldFormat ) );
        sal_Int32 nPos = aOut.indexOf( aDecSep );
        if ( nPos >= 0 )
            nPrecision = aOut.getLength() - nPos - aDecSep.getLength();
        // else keep 0
    }
    else
    {
        if ( (nOldType & SvNumFormatType::SCIENTIFIC) && !bThousand &&
             (pOldEntry->GetFormatIntegerDigits()%3 == 0) && pOldEntry->GetFormatIntegerDigits() > 0 )
            bThousand =  true;
    }

    if (!bError)
    {
        if (bIncrement)
        {
            if (nPrecision<20)
                ++nPrecision;           // increment
            else
                bError = true;          // 20 is maximum
        }
        else
        {
            if (nPrecision)
                --nPrecision;           // decrement
            else
                bError = true;          // 0 is minimum
        }
    }

    if (!bError)
    {
        OUString aNewPicture = pFormatter->GenerateFormat(nOldFormat, eLanguage,
                                                          bThousand, bNegRed,
                                                          nPrecision, nLeading)
                                + sExponentialStandardFormat;

        nNewFormat = pFormatter->GetEntryKey( aNewPicture, eLanguage );
        if ( nNewFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_Int32 nErrPos = 0;
            SvNumFormatType nNewType = SvNumFormatType::ALL;
            bool bOk = pFormatter->PutEntry( aNewPicture, nErrPos,
                                                nNewType, nNewFormat, eLanguage );
            OSL_ENSURE( bOk, "incorrect numberformat generated" );
            if (!bOk)
                bError = true;
        }
    }

    if (!bError)
    {
        ScPatternAttr aNewAttrs( pDoc->GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
        //  ATTR_LANGUAGE_FORMAT not
        ApplySelectionPattern( aNewAttrs );
    }
}

void ScViewFunc::ChangeIndent( bool bIncrement )
{
    ScViewData& rViewData = GetViewData();
    ScDocShell* pDocSh  = rViewData.GetDocShell();
    ScMarkData& rMark   = rViewData.GetMarkData();

    ScMarkData aWorkMark = rMark;
    ScViewUtil::UnmarkFiltered( aWorkMark, &pDocSh->GetDocument() );
    aWorkMark.MarkToMulti();
    if (!aWorkMark.IsMultiMarked())
    {
        SCCOL nCol = rViewData.GetCurX();
        SCROW nRow = rViewData.GetCurY();
        SCTAB nTab = rViewData.GetTabNo();
        aWorkMark.SetMultiMarkArea( ScRange(nCol,nRow,nTab) );
    }

    bool bSuccess = pDocSh->GetDocFunc().ChangeIndent( aWorkMark, bIncrement, false );
    if (bSuccess)
    {
        pDocSh->UpdateOle(&rViewData);
        StartFormatArea();

        // stuff for sidebar panels
        SfxBindings& rBindings = GetViewData().GetBindings();
        rBindings.Invalidate( SID_H_ALIGNCELL );
        rBindings.Invalidate( SID_ATTR_ALIGN_INDENT );
    }
}

bool ScViewFunc::InsertName( const OUString& rName, const OUString& rSymbol,
                                const OUString& rType )
{
    //  Type = P,R,C,F (and combinations)
    //! undo...

    bool bOk = false;
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData().GetTabNo();
    ScRangeName* pList = rDoc.GetRangeName();

    ScRangeData::Type nType = ScRangeData::Type::Name;
    auto pNewEntry = std::make_unique<ScRangeData>(
        &rDoc, rName, rSymbol, ScAddress( GetViewData().GetCurX(),
        GetViewData().GetCurY(), nTab), nType );
    OUString aUpType = rType.toAsciiUpperCase();
    if ( aUpType.indexOf( 'P' ) != -1 )
        nType |= ScRangeData::Type::PrintArea;
    if ( aUpType.indexOf( 'R' ) != -1 )
        nType |= ScRangeData::Type::RowHeader;
    if ( aUpType.indexOf( 'C' ) != -1 )
        nType |= ScRangeData::Type::ColHeader;
    if ( aUpType.indexOf( 'F' ) != -1 )
        nType |= ScRangeData::Type::Criteria;
    pNewEntry->AddType(nType);

    if ( pNewEntry->GetErrCode() == FormulaError::NONE )     //  text valid?
    {
        ScDocShellModificator aModificator( *pDocSh );

        rDoc.PreprocessRangeNameUpdate();

        // input available yet? Then remove beforehand (=change)
        ScRangeData* pData = pList->findByUpperName(ScGlobal::pCharClass->uppercase(rName));
        if (pData)
        {                                   // take old Index
            pNewEntry->SetIndex(pData->GetIndex());
            pList->erase(*pData);
        }

        // don't delete, insert took ownership, even on failure!
        if ( pList->insert( pNewEntry.release() ) )
            bOk = true;

        rDoc.CompileHybridFormula();

        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreasChanged ) );
    }

    return bOk;
}

void ScViewFunc::CreateNames( CreateNameFlags nFlags )
{
    bool bDone = false;
    ScRange aRange;
    if ( GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE )
        bDone = GetViewData().GetDocShell()->GetDocFunc().CreateNames( aRange, nFlags, false );

    if (!bDone)
        ErrorMessage(STR_CREATENAME_MARKERR);
}

CreateNameFlags ScViewFunc::GetCreateNameFlags()
{
    CreateNameFlags nFlags = CreateNameFlags::NONE;

    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    SCTAB nDummy;
    if (GetViewData().GetSimpleArea(nStartCol,nStartRow,nDummy,nEndCol,nEndRow,nDummy) == SC_MARK_SIMPLE)
    {
        ScDocument* pDoc = GetViewData().GetDocument();
        SCTAB nTab = GetViewData().GetTabNo();
        bool bOk;
        SCCOL i;
        SCROW j;

        bOk = true;
        SCCOL nFirstCol = nStartCol;
        SCCOL nLastCol  = nEndCol;
        if (nStartCol+1 < nEndCol) { ++nFirstCol; --nLastCol; }
        for (i=nFirstCol; i<=nLastCol && bOk; i++)
            if (!pDoc->HasStringData( i,nStartRow,nTab ))
                bOk = false;
        if (bOk)
            nFlags |= CreateNameFlags::Top;
        else                            // Bottom only if not Top
        {
            bOk = true;
            for (i=nFirstCol; i<=nLastCol && bOk; i++)
                if (!pDoc->HasStringData( i,nEndRow,nTab ))
                    bOk = false;
            if (bOk)
                nFlags |= CreateNameFlags::Bottom;
        }

        bOk = true;
        SCROW nFirstRow = nStartRow;
        SCROW nLastRow  = nEndRow;
        if (nStartRow+1 < nEndRow) { ++nFirstRow; --nLastRow; }
        for (j=nFirstRow; j<=nLastRow && bOk; j++)
            if (!pDoc->HasStringData( nStartCol,j,nTab ))
                bOk = false;
        if (bOk)
            nFlags |= CreateNameFlags::Left;
        else                            // Right only if not Left
        {
            bOk = true;
            for (j=nFirstRow; j<=nLastRow && bOk; j++)
                if (!pDoc->HasStringData( nEndCol,j,nTab ))
                    bOk = false;
            if (bOk)
                nFlags |= CreateNameFlags::Right;
        }
    }

    if (nStartCol == nEndCol)
        nFlags &= ~CreateNameFlags( CreateNameFlags::Left | CreateNameFlags::Right );
    if (nStartRow == nEndRow)
        nFlags &= ~CreateNameFlags( CreateNameFlags::Top | CreateNameFlags::Bottom );

    return nFlags;
}

void ScViewFunc::InsertNameList()
{
    ScAddress aPos( GetViewData().GetCurX(), GetViewData().GetCurY(), GetViewData().GetTabNo() );
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    if ( pDocSh->GetDocFunc().InsertNameList( aPos, false ) )
        pDocSh->UpdateOle(&GetViewData());
}

void ScViewFunc::UpdateSelectionArea( const ScMarkData& rSel, ScPatternAttr* pAttr  )
{
    ScDocShell* pDocShell = GetViewData().GetDocShell();
    ScRange aMarkRange;
    if (rSel.IsMultiMarked() )
        rSel.GetMultiMarkArea( aMarkRange );
    else
        rSel.GetMarkArea( aMarkRange );

    bool bSetLines = false;
    bool bSetAlign = false;
    if ( pAttr )
    {
        const SfxItemSet& rNewSet = pAttr->GetItemSet();
        bSetLines = rNewSet.GetItemState( ATTR_BORDER ) == SfxItemState::SET ||
        rNewSet.GetItemState( ATTR_SHADOW ) == SfxItemState::SET;
        bSetAlign = rNewSet.GetItemState( ATTR_HOR_JUSTIFY ) == SfxItemState::SET;
    }

    sal_uInt16 nExtFlags = 0;
    if ( bSetLines )
        nExtFlags |= SC_PF_LINES;
    if ( bSetAlign )
        nExtFlags |= SC_PF_WHOLEROWS;

    SCCOL nStartCol = aMarkRange.aStart.Col();
    SCROW nStartRow = aMarkRange.aStart.Row();
    SCTAB nStartTab = aMarkRange.aStart.Tab();
    SCCOL nEndCol = aMarkRange.aEnd.Col();
    SCROW nEndRow = aMarkRange.aEnd.Row();
    SCTAB nEndTab = aMarkRange.aEnd.Tab();
    pDocShell->PostPaint( nStartCol, nStartRow, nStartTab,
        nEndCol,   nEndRow,   nEndTab,
        PaintPartFlags::Grid, nExtFlags | SC_PF_TESTMERGE );
    ScTabViewShell* pTabViewShell = GetViewData().GetViewShell();
    pTabViewShell->AdjustBlockHeight(false, const_cast<ScMarkData*>(&rSel));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
