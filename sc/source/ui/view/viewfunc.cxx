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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/justifyitem.hxx>
#include <sfx2/bindings.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/wrkwin.hxx>
#include <stdlib.h>

#include "viewfunc.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "undocell.hxx"
#include "undoblk.hxx"
#include "undotab.hxx"
#include "refundo.hxx"
#include "dbdata.hxx"
#include "olinetab.hxx"
#include "rangeutl.hxx"
#include "rangenam.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "stlsheet.hxx"
#include "editutil.hxx"
#include "formulacell.hxx"
#include "scresid.hxx"
#include "inputhdl.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"
#include "compiler.hxx"
#include "docfunc.hxx"
#include "appoptio.hxx"
#include "dociter.hxx"
#include "sizedev.hxx"
#include "editable.hxx"
#include "scui_def.hxx"
#include "funcdesc.hxx"
#include "docuno.hxx"
#include "cellsuno.hxx"
#include "tokenarray.hxx"
#include <rowheightcontext.hxx>
#include <boost/scoped_ptr.hpp>

static void lcl_PostRepaintCondFormat( const ScConditionalFormat *pCondFmt, ScDocShell *pDocSh )
{
    if( pCondFmt )
    {
        const ScRangeList& rRanges = pCondFmt->GetRange();

        pDocSh->PostPaint( rRanges, PAINT_ALL );
    }
}


ScViewFunc::ScViewFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
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
    bool bOk = (GetViewData()->GetSimpleArea( aMarkRange ) == SC_MARK_SIMPLE);
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
    //  (discard old Area ). If not not wanted, discard if-statement
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
                                   bool bAttrChanged, bool bAddUndo )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    if (bAddUndo && !pDoc->IsUndoEnabled())
        bAddUndo = false;

    const ScPatternAttr* pSource = pDoc->GetPattern(
                            aFormatSource.Col(), aFormatSource.Row(), nTab );
    if ( !((const ScMergeAttr&)pSource->GetItem(ATTR_MERGE)).IsMerged() )
    {
        ScRange aRange( nCol, nRow, nTab, nCol, nRow, nTab );
        ScMarkData aMark;
        aMark.SetMarkArea( aRange );

        ScDocFunc &rFunc = GetViewData()->GetDocFunc();

        // pOldPattern is only valid until call to ApplyAttributes!
        const ScPatternAttr* pOldPattern = pDoc->GetPattern( nCol, nRow, nTab );
        const ScStyleSheet* pSrcStyle = pSource->GetStyleSheet();
        if ( pSrcStyle && pSrcStyle != pOldPattern->GetStyleSheet() )
            rFunc.ApplyStyle( aMark, pSrcStyle->GetName(), true, false );

        rFunc.ApplyAttributes( aMark, *pSource, true, false );
    }

    if ( bAttrChanged )                             // value entered with number format?
        aFormatSource.Set( nCol, nRow, nTab );      // then set a new source
}

//      additional routines

sal_uInt16 ScViewFunc::GetOptimalColWidth( SCCOL nCol, SCTAB nTab, bool bFormula )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();

    double nPPTX = GetViewData()->GetPPTX();
    double nPPTY = GetViewData()->GetPPTY();
    Fraction aZoomX = GetViewData()->GetZoomX();
    Fraction aZoomY = GetViewData()->GetZoomY();

    ScSizeDeviceProvider aProv(pDocSh);
    if (aProv.IsPrinter())
    {
        nPPTX = aProv.GetPPTX();
        nPPTY = aProv.GetPPTY();
        aZoomX = aZoomY = Fraction( 1, 1 );
    }

    sal_uInt16 nTwips = pDoc->GetOptimalColWidth( nCol, nTab, aProv.GetDevice(),
                                nPPTX, nPPTY, aZoomX, aZoomY, bFormula, &rMark );
    return nTwips;
}

bool ScViewFunc::SelectionEditable( bool* pOnlyNotBecauseOfMatrix /* = NULL */ )
{
    bool bRet;
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    if (rMark.IsMarked() || rMark.IsMultiMarked())
        bRet = pDoc->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix );
    else
    {
        SCCOL nCol = GetViewData()->GetCurX();
        SCROW nRow = GetViewData()->GetCurY();
        SCTAB nTab = GetViewData()->GetTabNo();
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

    sal_uInt16 nNewCount = std::min( (sal_uInt16)(nOldCount + 1), (sal_uInt16)LRU_MAX );
    sal_uInt16 nNewList[LRU_MAX];
    nNewList[0] = nOpCode;
    for (nPos=1; nPos<nNewCount; nPos++)
        nNewList[nPos] = pOldList[nPos-1];
    rAppOpt.SetLRUFuncList( nNewList, nNewCount );

    return true;                                // list has changed
}

namespace HelperNotifyChanges
{
    void NotifyIfChangesListeners(ScDocShell &rDocShell, ScMarkData& rMark, SCCOL nCol, SCROW nRow,
        const OUString &rType = OUString("cell-change"))
    {
        if (ScModelObj *pModelObj = getMustPropagateChangesModel(rDocShell))
        {
            ScRangeList aChangeRanges;
            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd; ++itr)
                aChangeRanges.Append( ScRange( nCol, nRow, *itr ) );

            HelperNotifyChanges::Notify(*pModelObj, aChangeRanges, rType);
        }
    }
}

//      actual functions

//  input - undo OK

void ScViewFunc::EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab,
                            const OUString& rString,
                            const EditTextObject* pData )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    bool bRecord = pDoc->IsUndoEnabled();
    SCTAB i;

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocFunc &rFunc = GetViewData()->GetDocFunc();
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
        ScCompiler aComp( pDoc, aPos);
        aComp.SetGrammar(pDoc->GetGrammar());
//2do: enable/disable autoCorrection via calcoptions
        aComp.SetAutoCorrection( true );
        if ( rString[0] == '+' || rString[0] == '-' )
        {
            aComp.SetExtendedErrorDetection( ScCompiler::EXTENDED_ERROR_DETECTION_NAME_BREAK );
        }
        OUString aFormula( rString );
        ScTokenArray* pArr;
        bool bAgain;
        do
        {
            bAgain = false;
            bool bAddEqual = false;
            ScTokenArray* pArrFirst = pArr = aComp.CompileString( aFormula );
            bool bCorrected = aComp.IsCorrected();
            if ( bCorrected )
            {   // try to parse with first parser-correction
                pArr = aComp.CompileString( aComp.GetCorrectedFormula() );
            }
            if ( !pArr->GetCodeError() )
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
                    nResult = QueryBox( GetViewData()->GetDialogParent(),
                                            WinBits(WB_YES_NO | WB_DEF_YES),
                                            aMessage ).Execute();
                }
                if ( nResult == RET_YES )
                {
                    aFormula = aCorrectedFormula;
                    if ( pArr != pArrFirst )
                        delete pArrFirst;
                    bAgain = true;
                }
                else
                {
                    if ( pArr != pArrFirst )
                    {
                        delete pArr;
                        pArr = pArrFirst;
                    }
                }
            }
        } while ( bAgain );
        // to be used in multiple tabs, the formula must be compiled anew
        // via ScFormulaCell copy-ctor because of RangeNames,
        // the same code-array for all cells is not possible.
        // If the array has an error, (it) must be RPN-erased in the newly generated
        // cellst and the error be set explicitly, so that
        // via FormulaCell copy-ctor and Interpreter it will be, when possible,
        // ironed out again, too intelligent.. e.g.: =1))
        sal_uInt16 nError = pArr->GetCodeError();
        if ( !nError )
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
                pScMod->RecentFunctionsChanged();
            }
        }

        ScFormulaCell aCell(pDoc, aPos, *pArr, formula::FormulaGrammar::GRAM_DEFAULT, MM_NONE);
        delete pArr;

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd; ++itr)
        {
            i = *itr;
            aPos.SetTab( i );
            sal_uLong nIndex = (sal_uLong) ((SfxUInt32Item*) pDoc->GetAttr(
                nCol, nRow, i, ATTR_VALUE_FORMAT ))->GetValue();
            if ( pFormatter->GetType( nIndex ) == NUMBERFORMAT_TEXT ||
                 ( ( rString[0] == '+' || rString[0] == '-' ) && nError && rString == aFormula ) )
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
                if ( nError )
                {
                    pCell->GetCode()->DelRPN();
                    pCell->SetErrCode( nError );
                    if(pCell->GetCode()->IsHyperLink())
                        pCell->GetCode()->SetHyperLink(false);
                }
                rFunc.SetFormulaCell(aPos, pCell, true);
            }
        }
    }
    else
    {
        ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
        for ( ; itr != itrEnd; ++itr )
        {
            bool bNumFmtSet = false;
            rFunc.SetNormalString( bNumFmtSet, ScAddress( nCol, nRow, *itr ), rString, false );
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
        DoAutoAttributes(nCol, nRow, nTab, bNumFmtChanged, bRecord);

    pDocSh->UpdateOle(GetViewData());

    HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, rMark, nCol, nRow);

    if ( bRecord )
        rFunc.EndListAction();

    aModificator.SetDocumentModified();
    lcl_PostRepaintCondFormat( pDoc->GetCondFormat( nCol, nRow, nTab ), pDocSh );
}

// enter value in single cell (on nTab only)

void ScViewFunc::EnterValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rValue )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();

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
                    new ScUndoEnterValue(pDocSh, aPos, aUndoCell, rValue));
            }

            pDocSh->PostPaintCell( aPos );
            pDocSh->UpdateOle(GetViewData());
            aModificator.SetDocumentModified();
        }
        else
            ErrorMessage(aTester.GetMessageId());
    }
}

void ScViewFunc::EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab,
                            const EditTextObject& rData, bool bTestSimple )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocument* pDoc = pDocSh->GetDocument();
    bool bRecord = pDoc->IsUndoEnabled();

    ScDocShellModificator aModificator( *pDocSh );

    ScEditableTester aTester( pDoc, nTab, nCol,nRow, nCol,nRow );
    if (aTester.IsEditable())
    {

        //      test for attribute

        bool bSimple = false;
        bool bCommon = false;
        boost::scoped_ptr<ScPatternAttr> pCellAttrs;
        OUString aString;

        const ScPatternAttr* pOldPattern = pDoc->GetPattern( nCol, nRow, nTab );
        ScTabEditEngine aEngine( *pOldPattern, pDoc->GetEnginePool() );
        aEngine.SetText(rData);

        if (bTestSimple)                    // test, if simple string without attribute
        {
            ScEditAttrTester aAttrTester( &aEngine );
            bSimple = !aAttrTester.NeedsObject();
            bCommon = aAttrTester.NeedsCellAttr();

            // formulas have to be recognized even if they're formatted
            // (but commmon attributes are still collected)

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


        EditTextObject* pUndoData = NULL;
        ScUndoEnterData::ValuesType aOldValues;

        if (bRecord && !bSimple)
        {
            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd; ++itr)
            {
                ScUndoEnterData::Value aOldValue;
                aOldValue.mnTab = *itr;
                aOldValue.maCell.assign(*pDoc, ScAddress(nCol, nRow, *itr));
                aOldValues.push_back(aOldValue);
            }

            pUndoData = rData.Clone();
        }


        //      enter data


        if (bCommon)
            pDoc->ApplyPattern(nCol,nRow,nTab,*pCellAttrs);         //! undo

        if (bSimple)
        {
            if (bCommon)
                AdjustRowHeight(nRow,nRow);

            EnterData(nCol,nRow,nTab,aString);
        }
        else
        {
            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd; ++itr)
            {
                ScAddress aPos(nCol, nRow, *itr);
                pDoc->SetEditText(aPos, rData, pDoc->GetEditPool());
            }

            if ( bRecord )
            {   //  because of ChangeTrack current first
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoEnterData(pDocSh, ScAddress(nCol,nRow,nTab), aOldValues, aString, pUndoData));
            }

            HideAllCursors();

            AdjustRowHeight(nRow,nRow);

            itr = rMark.begin();
            for (; itr != itrEnd; ++itr)
                pDocSh->PostPaintCell( nCol, nRow, *itr );

            ShowAllCursors();

            pDocSh->UpdateOle(GetViewData());

            HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, rMark, nCol, nRow);

            aModificator.SetDocumentModified();
        }
        lcl_PostRepaintCondFormat( pDoc->GetCondFormat( nCol, nRow, nTab ), pDocSh );
    }
    else
    {
        ErrorMessage(aTester.GetMessageId());
        PaintArea( nCol, nRow, nCol, nRow );        // possibly the edit-engine is still painted there
    }
}

void ScViewFunc::EnterDataAtCursor( const OUString& rString )
{
    SCCOL nPosX = GetViewData()->GetCurX();
    SCROW nPosY = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();

    EnterData( nPosX, nPosY, nTab, rString );
}

void ScViewFunc::EnterMatrix( const OUString& rString, ::formula::FormulaGrammar::Grammar eGram )
{
    ScViewData* pData = GetViewData();
    const ScMarkData& rMark = pData->GetMarkData();
    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        //  nothing marked -> temporarily calculate block
        //  with size of result formula to get the size

        ScDocument* pDoc = pData->GetDocument();
        SCCOL nCol = pData->GetCurX();
        SCROW nRow = pData->GetCurY();
        SCTAB nTab = pData->GetTabNo();
        ScFormulaCell aFormCell( pDoc, ScAddress(nCol,nRow,nTab), rString, eGram, MM_FORMULA );

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
    if (pData->GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = pData->GetDocShell();
        bool bSuccess = pDocSh->GetDocFunc().EnterMatrix(
            aRange, &rMark, NULL, rString, false, false, EMPTY_OUSTRING, eGram );
        if (bSuccess)
            pDocSh->UpdateOle(GetViewData());
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

sal_uInt8 ScViewFunc::GetSelectionScriptType()
{
    sal_uInt8 nScript = 0;

    ScDocument* pDoc = GetViewData()->GetDocument();
    const ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        // no selection -> cursor

        nScript = pDoc->GetScriptType( GetViewData()->GetCurX(),
                            GetViewData()->GetCurY(), GetViewData()->GetTabNo());
    }
    else
    {
        ScRangeList aRanges;
        rMark.FillRangeListWithMarks( &aRanges, false );
        nScript = pDoc->GetRangeScriptType(aRanges);
    }

    if (nScript == 0)
        nScript = ScGlobal::GetDefaultScriptType();

    return nScript;
}

const ScPatternAttr* ScViewFunc::GetSelectionPattern()
{
    // Don't use UnmarkFiltered in slot state functions, for performance reasons.
    // The displayed state is always that of the whole selection including filtered rows.

    const ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        //  MarkToMulti is no longer necessary for pDoc->GetSelectionPattern
        const ScPatternAttr* pAttr = pDoc->GetSelectionPattern( rMark );
        return pAttr;
    }
    else
    {
        SCCOL  nCol = GetViewData()->GetCurX();
        SCROW  nRow = GetViewData()->GetCurY();
        SCTAB  nTab = GetViewData()->GetTabNo();

        ScMarkData aTempMark( rMark );      // copy sheet selection
        aTempMark.SetMarkArea( ScRange( nCol, nRow, nTab ) );
        const ScPatternAttr* pAttr = pDoc->GetSelectionPattern( aTempMark );
        return pAttr;
    }
}

void ScViewFunc::GetSelectionFrame( SvxBoxItem&     rLineOuter,
                                    SvxBoxInfoItem& rLineInner )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    const ScMarkData& rMark = GetViewData()->GetMarkData();

    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        if ( rMark.IsMultiMarked() )
        {
            ScMarkData aNewMark( rMark );   // use local copy for MarkToSimple
            aNewMark.MarkToSimple();        // simple block is needed for GetSelectionFrame
            pDoc->GetSelectionFrame( aNewMark, rLineOuter, rLineInner );
        }
        else
            pDoc->GetSelectionFrame( rMark, rLineOuter, rLineInner );
    }
    else
    {
        const ScPatternAttr* pAttrs =
                    pDoc->GetPattern( GetViewData()->GetCurX(),
                                      GetViewData()->GetCurY(),
                                      GetViewData()->GetTabNo() );

        rLineOuter = (const SvxBoxItem&)    (pAttrs->GetItem( ATTR_BORDER ));
        rLineInner = (const SvxBoxInfoItem&)(pAttrs->GetItem( ATTR_BORDER_INNER ));
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
                                  bool              bRecord )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aOldAttrs( new SfxItemSet(*pOldSet) );
    ScPatternAttr aNewAttrs( new SfxItemSet(*pDialogSet) );
    aNewAttrs.DeleteUnchanged( &aOldAttrs );

    if ( pDialogSet->GetItemState( ATTR_VALUE_FORMAT ) == SFX_ITEM_SET )
    {   // don't reset to default SYSTEM GENERAL if not intended
        sal_uInt32 nOldFormat =
            ((const SfxUInt32Item&)pOldSet->Get( ATTR_VALUE_FORMAT )).GetValue();
        sal_uInt32 nNewFormat =
            ((const SfxUInt32Item&)pDialogSet->Get( ATTR_VALUE_FORMAT )).GetValue();
        if ( nNewFormat != nOldFormat )
        {
            SvNumberFormatter* pFormatter =
                GetViewData()->GetDocument()->GetFormatTable();
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
                     nNewMod <= SV_MAX_ANZ_STANDARD_FORMATE )
                    aNewAttrs.GetItemSet().ClearItem( ATTR_VALUE_FORMAT );
            }
        }
    }

    const SvxBoxItem*     pOldOuter = (const SvxBoxItem*)     &pOldSet->Get( ATTR_BORDER );
    const SvxBoxItem*     pNewOuter = (const SvxBoxItem*)     &pDialogSet->Get( ATTR_BORDER );
    const SvxBoxInfoItem* pOldInner = (const SvxBoxInfoItem*) &pOldSet->Get( ATTR_BORDER_INNER );
    const SvxBoxInfoItem* pNewInner = (const SvxBoxInfoItem*) &pDialogSet->Get( ATTR_BORDER_INNER );
    SfxItemSet&           rNewSet   = aNewAttrs.GetItemSet();
    SfxItemPool*          pNewPool  = rNewSet.GetPool();

    pNewPool->Put( *pNewOuter );        // don't delete yet
    pNewPool->Put( *pNewInner );
    rNewSet.ClearItem( ATTR_BORDER );
    rNewSet.ClearItem( ATTR_BORDER_INNER );

    /*
     * establish whether border attribute is to be set:
     * 1. new != old
     * 2. is one of the borders not-DontCare (since 238.f: IsxxValid())
     *
     */

    bool bFrame =    (pDialogSet->GetItemState( ATTR_BORDER ) != SFX_ITEM_DEFAULT)
                  || (pDialogSet->GetItemState( ATTR_BORDER_INNER ) != SFX_ITEM_DEFAULT);

    if ( pNewOuter==pOldOuter && pNewInner==pOldInner )
        bFrame = false;

    //  this should be intercepted by the pool: ?!??!??

    if ( bFrame && pNewOuter && pNewInner )
        if ( *pNewOuter == *pOldOuter && *pNewInner == *pOldInner )
            bFrame = false;

    if ( pNewInner )
    {
        bFrame =   bFrame
                && (   pNewInner->IsValid(VALID_LEFT)
                    || pNewInner->IsValid(VALID_RIGHT)
                    || pNewInner->IsValid(VALID_TOP)
                    || pNewInner->IsValid(VALID_BOTTOM)
                    || pNewInner->IsValid(VALID_HORI)
                    || pNewInner->IsValid(VALID_VERT) );
    }
    else
        bFrame = false;

    if (!bFrame)
        ApplySelectionPattern( aNewAttrs, bRecord );                // standard only
    else
    {
        // if new items are default-items, overwrite the old items:

        bool bDefNewOuter = ( SFX_ITEMS_STATICDEFAULT == pNewOuter->GetKind() );
        bool bDefNewInner = ( SFX_ITEMS_STATICDEFAULT == pNewInner->GetKind() );

        ApplyPatternLines( aNewAttrs,
                           bDefNewOuter ? pOldOuter : pNewOuter,
                           bDefNewInner ? pOldInner : pNewInner,
                           bRecord );
    }

    pNewPool->Remove( *pNewOuter );         // release
    pNewPool->Remove( *pNewInner );

    //  adjust height
    AdjustBlockHeight();

    // CellContentChanged is called in ApplySelectionPattern / ApplyPatternLines
}

void ScViewFunc::ApplyAttr( const SfxPoolItem& rAttrItem )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aNewAttrs( new SfxItemSet( *GetViewData()->GetDocument()->GetPool(),
                                            ATTR_PATTERN_START, ATTR_PATTERN_END ) );

    aNewAttrs.GetItemSet().Put( rAttrItem );
    //  if justify is set (with Buttons), always indentation 0
    if ( rAttrItem.Which() == ATTR_HOR_JUSTIFY )
        aNewAttrs.GetItemSet().Put( SfxUInt16Item( ATTR_INDENT, 0 ) );
    ApplySelectionPattern( aNewAttrs );

    AdjustBlockHeight();

    // CellContentChanged is called in ApplySelectionPattern
}


//  patterns and borders

void ScViewFunc::ApplyPatternLines( const ScPatternAttr& rAttr, const SvxBoxItem* pNewOuter,
                                    const SvxBoxInfoItem* pNewInner, bool bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData aFuncMark( GetViewData()->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScRange aMarkRange;
    aFuncMark.MarkToSimple();
    bool bMulti = aFuncMark.IsMultiMarked();
    if (bMulti)
        aFuncMark.GetMultiMarkArea( aMarkRange );
    else if (aFuncMark.IsMarked())
        aFuncMark.GetMarkArea( aMarkRange );
    else
    {
        aMarkRange = ScRange( GetViewData()->GetCurX(),
                            GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
        DoneBlockMode();
        InitOwnBlockMode();
        aFuncMark.SetMarkArea(aMarkRange);
        MarkDataChanged();
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    if (bRecord)
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = pDoc->GetTableCount();
        pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
        ScMarkData::iterator itr = aFuncMark.begin(), itrEnd = aFuncMark.end();
        for (; itr != itrEnd; ++itr)
            if (*itr != nStartTab)
                pUndoDoc->AddUndoTab( *itr, *itr );

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, pUndoDoc, &aFuncMark );

        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoSelectionAttr(
            pDocSh, aFuncMark,
            aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), aMarkRange.aStart.Tab(),
            aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), aMarkRange.aEnd.Tab(),
            pUndoDoc, bMulti, &rAttr, pNewOuter, pNewInner ) );
    }

    sal_uInt16 nExt = SC_PF_TESTMERGE;
    pDocSh->UpdatePaintExt( nExt, aMarkRange ); // content before the change

    pDoc->ApplySelectionFrame( aFuncMark, pNewOuter, pNewInner );

    pDocSh->UpdatePaintExt( nExt, aMarkRange ); // content after the change

    aFuncMark.MarkToMulti();
    pDoc->ApplySelectionPattern( rAttr, aFuncMark );

    pDocSh->PostPaint( aMarkRange, PAINT_GRID, nExt );
    pDocSh->UpdateOle(GetViewData());
    aModificator.SetDocumentModified();
    CellContentChanged();

    StartFormatArea();
}

//  pattern only

void ScViewFunc::ApplySelectionPattern( const ScPatternAttr& rAttr,
                                            bool bRecord, bool bCursorOnly )
{
    ScViewData* pViewData   = GetViewData();
    ScDocShell* pDocSh      = pViewData->GetDocShell();
    ScDocument* pDoc        = pDocSh->GetDocument();
    ScMarkData aFuncMark( pViewData->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    //  State from old ItemSet doesn't matter for paint flags, as any change will be
    //  from SFX_ITEM_SET in the new ItemSet (default is ignored in ApplyPattern).
    //  New alignment is checked (check in PostPaint isn't enough) in case a right
    //  alignment is changed to left.
    const SfxItemSet& rNewSet = rAttr.GetItemSet();
    bool bSetLines = rNewSet.GetItemState( ATTR_BORDER, true ) == SFX_ITEM_SET ||
                     rNewSet.GetItemState( ATTR_SHADOW, true ) == SFX_ITEM_SET;
    bool bSetAlign = rNewSet.GetItemState( ATTR_HOR_JUSTIFY, true ) == SFX_ITEM_SET;

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
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();
        aFuncMark.SetMarkArea(ScRange(nCol,nRow,nTab));
        aFuncMark.MarkToMulti();
    }

    ScRangeList aChangeRanges;

    if (aFuncMark.IsMultiMarked() && !bCursorOnly)
    {
        ScRange aMarkRange;
        aFuncMark.GetMultiMarkArea( aMarkRange );
        SCTAB nTabCount = pDoc->GetTableCount();
        ScMarkData::iterator itr = aFuncMark.begin(), itrEnd = aFuncMark.end();
        for (; itr != itrEnd; ++itr)
        {
            ScRange aChangeRange( aMarkRange );
            aChangeRange.aStart.SetTab( *itr );
            aChangeRange.aEnd.SetTab( *itr );
            aChangeRanges.Append( aChangeRange );
        }

        SCCOL nStartCol = aMarkRange.aStart.Col();
        SCROW nStartRow = aMarkRange.aStart.Row();
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCCOL nEndCol = aMarkRange.aEnd.Col();
        SCROW nEndRow = aMarkRange.aEnd.Row();
        SCTAB nEndTab = aMarkRange.aEnd.Tab();

        ScUndoSelectionAttr* pUndoAttr = NULL;
        ScEditDataArray* pEditDataArray = NULL;
        if (bRecord)
        {
            ScRange aCopyRange = aMarkRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);

            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
            itr = aFuncMark.begin();
            for (; itr != itrEnd; ++itr)
                if (*itr != nStartTab)
                    pUndoDoc->AddUndoTab( *itr, *itr );
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, pUndoDoc, &aFuncMark );

            aFuncMark.MarkToMulti();

            pUndoAttr = new ScUndoSelectionAttr(
                pDocSh, aFuncMark, nStartCol, nStartRow, nStartTab,
                nEndCol, nEndRow, nEndTab, pUndoDoc, bMulti, &rAttr );
            pDocSh->GetUndoManager()->AddUndoAction(pUndoAttr);
            pEditDataArray = pUndoAttr->GetDataArray();
        }

        pDoc->ApplySelectionPattern( rAttr, aFuncMark, pEditDataArray );

        pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                           nEndCol,   nEndRow,   nEndTab,
                           PAINT_GRID, nExtFlags | SC_PF_TESTMERGE );
        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
    }
    else                            // single cell - simpler undo
    {
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();

        EditTextObject* pOldEditData = NULL;
        EditTextObject* pNewEditData = NULL;
        ScAddress aPos(nCol, nRow, nTab);
        if (pDoc->GetCellType(aPos) == CELLTYPE_EDIT)
        {
            pOldEditData = pDoc->GetEditText(aPos)->Clone();
            pDoc->RemoveEditTextCharAttribs(aPos, rAttr);
            pNewEditData = pDoc->GetEditText(aPos)->Clone();
        }

        aChangeRanges.Append(aPos);
        boost::scoped_ptr<ScPatternAttr> pOldPat(new ScPatternAttr(*pDoc->GetPattern( nCol, nRow, nTab )));

        pDoc->ApplyPattern( nCol, nRow, nTab, rAttr );

        const ScPatternAttr* pNewPat = pDoc->GetPattern( nCol, nRow, nTab );

        if (bRecord)
        {
            ScUndoCursorAttr* pUndo = new ScUndoCursorAttr(
                pDocSh, nCol, nRow, nTab, pOldPat.get(), pNewPat, &rAttr, false );
            pUndo->SetEditData(pOldEditData, pNewEditData);
            pDocSh->GetUndoManager()->AddUndoAction(pUndo);
        }
        pOldPat.reset();     // is copied in undo (Pool)

        pDocSh->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID, nExtFlags | SC_PF_TESTMERGE );
        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
    }

    ScModelObj* pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh);
    if (pModelObj)
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aProperties;
        sal_Int32 nCount = 0;
        const SfxItemPropertyMap& rMap = ScCellObj::GetCellPropertyMap();
        PropertyEntryVector_t aPropVector = rMap.getPropertyEntries();
        for ( sal_uInt16 nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; ++nWhich )
        {
            const SfxPoolItem* pItem = 0;
            if ( rNewSet.GetItemState( nWhich, true, &pItem ) == SFX_ITEM_SET && pItem )
            {
                PropertyEntryVector_t::const_iterator aIt = aPropVector.begin();
                while ( aIt != aPropVector.end())
                {
                    if ( aIt->nWID == nWhich )
                    {
                        ::com::sun::star::uno::Any aVal;
                        pItem->QueryValue( aVal, aIt->nMemberId );
                        aProperties.realloc( nCount + 1 );
                        aProperties[ nCount ].Name = aIt->sName;
                        aProperties[ nCount ].Value <<= aVal;
                        ++nCount;
                    }
                    ++aIt;
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

    ScPatternAttr aNewAttrs( GetViewData()->GetDocument()->GetPool() );
    SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
    rNewSet.Put( rItemSet, false );
    ApplySelectionPattern( aNewAttrs );

    AdjustBlockHeight();
}

const SfxStyleSheet* ScViewFunc::GetStyleSheetFromMarked()
{
    // Don't use UnmarkFiltered in slot state functions, for performance reasons.
    // The displayed state is always that of the whole selection including filtered rows.

    const ScStyleSheet* pSheet      = NULL;
    ScViewData*         pViewData   = GetViewData();
    ScDocument*         pDoc        = pViewData->GetDocument();
    ScMarkData&         rMark       = pViewData->GetMarkData();

    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
        pSheet = pDoc->GetSelectionStyle( rMark );                  // MarkToMulti isn't necessary
    else
        pSheet = pDoc->GetStyle( pViewData->GetCurX(),
                                 pViewData->GetCurY(),
                                 pViewData->GetTabNo() );

    return pSheet;
}

void ScViewFunc::SetStyleSheetToMarked( SfxStyleSheet* pStyleSheet, bool bRecord )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    if ( !pStyleSheet) return;


    ScViewData* pViewData   = GetViewData();
    ScDocShell* pDocSh      = pViewData->GetDocShell();
    ScDocument* pDoc        = pDocSh->GetDocument();
    ScMarkData aFuncMark( pViewData->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
    SCTAB nTabCount     = pDoc->GetTableCount();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScDocShellModificator aModificator( *pDocSh );

    if ( aFuncMark.IsMarked() || aFuncMark.IsMultiMarked() )
    {
        ScRange aMarkRange;
        aFuncMark.MarkToMulti();
        aFuncMark.GetMultiMarkArea( aMarkRange );

        if ( bRecord )
        {
            SCTAB nTab = pViewData->GetTabNo();
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
            ScMarkData::iterator itr = aFuncMark.begin(), itrEnd = aFuncMark.end();
            for (; itr != itrEnd; ++itr)
                if (*itr != nTab)
                    pUndoDoc->AddUndoTab( *itr, *itr );

            ScRange aCopyRange = aMarkRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, true, pUndoDoc, &aFuncMark );
            aFuncMark.MarkToMulti();

            OUString aName = pStyleSheet->GetName();
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSelectionStyle( pDocSh, aFuncMark, aMarkRange, aName, pUndoDoc ) );
        }

        pDoc->ApplySelectionStyle( (ScStyleSheet&)*pStyleSheet, aFuncMark );

        if (!AdjustBlockHeight())
            pViewData->GetDocShell()->PostPaint( aMarkRange, PAINT_GRID );

        aFuncMark.MarkToSimple();
    }
    else
    {
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();

        if ( bRecord )
        {
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
            ScMarkData::iterator itr = aFuncMark.begin(), itrEnd = aFuncMark.end();
            for (; itr != itrEnd; ++itr)
                if (*itr != nTab)
                    pUndoDoc->AddUndoTab( *itr, *itr );

            ScRange aCopyRange( nCol, nRow, 0, nCol, nRow, nTabCount-1 );
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, false, pUndoDoc );

            ScRange aMarkRange ( nCol, nRow, nTab );
            ScMarkData aUndoMark = aFuncMark;
            aUndoMark.SetMultiMarkArea( aMarkRange );

            OUString aName = pStyleSheet->GetName();
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSelectionStyle( pDocSh, aUndoMark, aMarkRange, aName, pUndoDoc ) );
        }

        ScMarkData::iterator itr = aFuncMark.begin(), itrEnd = aFuncMark.end();
        for (; itr != itrEnd; ++itr)
            pDoc->ApplyStyle( nCol, nRow, *itr, (ScStyleSheet&)*pStyleSheet );

        if (!AdjustBlockHeight())
            pViewData->GetDocShell()->PostPaintCell( nCol, nRow, nTab );

    }

    aModificator.SetDocumentModified();

    StartFormatArea();
}


void ScViewFunc::RemoveStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet )
{
    if ( !pStyleSheet) return;


    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScDocShell* pDocSh      = pViewData->GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    VirtualDevice aVirtDev;
    aVirtDev.SetMapMode(MAP_PIXEL);
    pDoc->StyleSheetChanged( pStyleSheet, true, &aVirtDev,
                                pViewData->GetPPTX(),
                                pViewData->GetPPTY(),
                                pViewData->GetZoomX(),
                                pViewData->GetZoomY() );

    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
    aModificator.SetDocumentModified();

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if (pHdl)
        pHdl->ForgetLastPattern();
}

void ScViewFunc::UpdateStyleSheetInUse( const SfxStyleSheetBase* pStyleSheet )
{
    if ( !pStyleSheet) return;


    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScDocShell* pDocSh      = pViewData->GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    VirtualDevice aVirtDev;
    aVirtDev.SetMapMode(MAP_PIXEL);
    pDoc->StyleSheetChanged( pStyleSheet, false, &aVirtDev,
                                pViewData->GetPPTX(),
                                pViewData->GetPPTY(),
                                pViewData->GetZoomX(),
                                pViewData->GetZoomY() );

    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_LEFT );
    aModificator.SetDocumentModified();

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
    if (pHdl)
        pHdl->ForgetLastPattern();
}

//  insert cells - undo OK

bool ScViewFunc::InsertCells( InsCellCmd eCmd, bool bRecord, bool bPartOfPaste )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();
        bool bSuccess = pDocSh->GetDocFunc().InsertCells( aRange, &rMark, eCmd, bRecord, false, bPartOfPaste );
        if (bSuccess)
        {
            pDocSh->UpdateOle(GetViewData());
            CellContentChanged();
            ResetAutoSpell();

            if ( eCmd == INS_INSROWS || eCmd == INS_INSCOLS )
            {
                OUString aOperation = ( eCmd == INS_INSROWS) ?
                    OUString("insert-rows"):
                    OUString("insert-columns");
                HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, aRange, aOperation);
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

void ScViewFunc::DeleteCells( DelCellCmd eCmd, bool bRecord )
{
    ScRange aRange;
    if ( GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
        // #i94841# [Collaboration] if deleting rows is rejected, the content is sometimes wrong
        if ( pDocSh->IsDocShared() && ( eCmd == DEL_DELROWS || eCmd == DEL_DELCOLS ) )
        {
            ScRange aDelRange( aRange.aStart );
            SCCOLROW nCount = 0;
            if ( eCmd == DEL_DELROWS )
            {
                nCount = sal::static_int_cast< SCCOLROW >( aRange.aEnd.Row() - aRange.aStart.Row() + 1 );
            }
            else
            {
                nCount = sal::static_int_cast< SCCOLROW >( aRange.aEnd.Col() - aRange.aStart.Col() + 1 );
            }
            while ( nCount > 0 )
            {
                pDocSh->GetDocFunc().DeleteCells( aDelRange, &rMark, eCmd, bRecord, false );
                --nCount;
            }
        }
        else
#endif
        {
            pDocSh->GetDocFunc().DeleteCells( aRange, &rMark, eCmd, bRecord, false );
        }

        pDocSh->UpdateOle(GetViewData());
        CellContentChanged();
        ResetAutoSpell();

        if ( eCmd == DEL_DELROWS || eCmd == DEL_DELCOLS )
        {
            OUString aOperation = ( eCmd == DEL_DELROWS) ?
              OUString("delete-rows"):
              OUString("delete-columns");
            HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, aRange, aOperation);
        }

        //  put cursor directly behind deleted range
        SCCOL nCurX = GetViewData()->GetCurX();
        SCROW nCurY = GetViewData()->GetCurY();
        if ( eCmd==DEL_CELLSLEFT || eCmd==DEL_DELCOLS )
            nCurX = aRange.aStart.Col();
        else
            nCurY = aRange.aStart.Row();
        SetCursor( nCurX, nCurY );
    }
    else
    {
        if (eCmd == DEL_DELCOLS)
            DeleteMulti( false, bRecord );
        else if (eCmd == DEL_DELROWS)
            DeleteMulti( true, bRecord );
        else
            ErrorMessage(STR_NOMULTISELECT);
    }

    Unmark();
}

void ScViewFunc::DeleteMulti( bool bRows, bool bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData aFuncMark( GetViewData()->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );

    if (!pDoc->IsUndoEnabled())
        bRecord = false;

    std::vector<sc::ColRowSpan> aSpans;
    if (bRows)
        aSpans = aFuncMark.GetMarkedRowSpans();
    else
        aSpans = aFuncMark.GetMarkedColSpans();

    if (aSpans.empty())
    {
        SCCOLROW nCurPos = bRows ? GetViewData()->GetCurY() : GetViewData()->GetCurX();
        aSpans.push_back(sc::ColRowSpan(nCurPos, nCurPos));
    }

    //  test if allowed

    sal_uInt16 nErrorId = 0;
    bool bNeedRefresh = false;
    for (size_t i = 0, n = aSpans.size(); i < n && !nErrorId; ++i)
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
            ScEditableTester aTester( pDoc, nTab, nStartCol, nStartRow, MAXCOL, MAXROW );
            if (!aTester.IsEditable())
                nErrorId = aTester.GetMessageId();
        }

        // merged cells
        SCCOL nMergeStartX = nStartCol;
        SCROW nMergeStartY = nStartRow;
        SCCOL nMergeEndX   = nEndCol;
        SCROW nMergeEndY   = nEndRow;
        pDoc->ExtendMerge( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );
        pDoc->ExtendOverlapped( nMergeStartX, nMergeStartY, nMergeEndX, nMergeEndY, nTab );

        if ( nMergeStartX != nStartCol || nMergeStartY != nStartRow )
        {
            // Disallow deleting parts of a merged cell.
            // Deleting the start is allowed (merge is removed), so the end doesn't have to be checked.

            nErrorId = STR_MSSG_DELETECELLS_0;
        }
        if ( nMergeEndX != nEndCol || nMergeEndY != nEndRow )
        {
            // detect if the start of a merged cell is deleted, so the merge flags can be refreshed

            bNeedRefresh = true;
        }
    }

    if ( nErrorId )
    {
        ErrorMessage( nErrorId );
        return;
    }

    //  proceed

    WaitObject aWait( GetFrameWin() );      // important for TrackFormulas in UpdateReference

    ScDocument* pUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab, !bRows, bRows );      // row height

        for (size_t i = 0, n = aSpans.size(); i < n; ++i)
        {
            SCCOLROW nStart = aSpans[i].mnStart;
            SCCOLROW nEnd = aSpans[i].mnEnd;
            if (bRows)
                pDoc->CopyToDocument( 0,nStart,nTab, MAXCOL,nEnd,nTab, IDF_ALL,false,pUndoDoc );
            else
                pDoc->CopyToDocument( static_cast<SCCOL>(nStart),0,nTab,
                        static_cast<SCCOL>(nEnd),MAXROW,nTab,
                        IDF_ALL,false,pUndoDoc );
        }

        //  all Formulas because of references
        SCTAB nTabCount = pDoc->GetTableCount();
        pUndoDoc->AddUndoTab( 0, nTabCount-1, false, false );
        pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA,false,pUndoDoc );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    std::vector<sc::ColRowSpan>::const_reverse_iterator ri = aSpans.rbegin(), riEnd = aSpans.rend();
    for (; ri != riEnd; ++ri)
    {
        SCCOLROW nEnd = ri->mnEnd;
        SCCOLROW nStart = ri->mnStart;

        if (bRows)
            pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, nStart, static_cast<SCSIZE>(nEnd-nStart+1) );
        else
            pDoc->DeleteCol( 0,nTab, MAXROW,nTab, static_cast<SCCOL>(nStart), static_cast<SCSIZE>(nEnd-nStart+1) );
    }

    if (bNeedRefresh)
    {
        SCCOLROW nFirstStart = aSpans[0].mnStart;
        SCCOL nStartCol = bRows ? 0 : static_cast<SCCOL>(nFirstStart);
        SCROW nStartRow = bRows ? static_cast<SCROW>(nFirstStart) : 0;
        SCCOL nEndCol = MAXCOL;
        SCROW nEndRow = MAXROW;

        pDoc->RemoveFlagsTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );
        pDoc->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab, true );
    }

    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoDeleteMulti(
                pDocSh, bRows, bNeedRefresh, nTab, aSpans, pUndoDoc, pUndoData));
    }

    if (!AdjustRowHeight(0, MAXROW))
    {
        if (bRows)
        {
            pDocSh->PostPaint(
                0, aSpans[0].mnStart, nTab,
                MAXCOL, MAXROW, nTab, (PAINT_GRID | PAINT_LEFT));
        }
        else
        {
            pDocSh->PostPaint(
                static_cast<SCCOL>(aSpans[0].mnStart), 0, nTab,
                MAXCOL, MAXROW, nTab, (PAINT_GRID | PAINT_TOP));
        }
    }

    ResetAutoSpell();
    aModificator.SetDocumentModified();

    CellContentChanged();

    //  put cursor directly behind the first deleted range
    SCCOL nCurX = GetViewData()->GetCurX();
    SCROW nCurY = GetViewData()->GetCurY();
    if ( bRows )
        nCurY = aSpans[0].mnStart;
    else
        nCurX = static_cast<SCCOL>(aSpans[0].mnStart);
    SetCursor( nCurX, nCurY );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

//  delete contents

void ScViewFunc::DeleteContents( sal_uInt16 nFlags, bool bRecord )
{
    ScViewData* pViewData = GetViewData();
    pViewData->SetPasteMode( SC_PASTE_NONE );
    pViewData->GetViewShell()->UpdateCopySourceOverlay();

    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    bool bEditable = SelectionEditable( &bOnlyNotBecauseOfMatrix );
    if ( !bEditable )
    {
        if ( !(bOnlyNotBecauseOfMatrix &&
                ((nFlags & (IDF_ATTRIB | IDF_EDITATTR)) == nFlags)) )
        {
            ErrorMessage(bOnlyNotBecauseOfMatrix ? STR_MATRIXFRAGMENTERR : STR_PROTECTIONERR);
            return;
        }
    }

    ScRange aMarkRange;
    bool bSimple = false;

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData aFuncMark( GetViewData()->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScDocShellModificator aModificator( *pDocSh );

    if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
    {
        aMarkRange.aStart.SetCol(GetViewData()->GetCurX());
        aMarkRange.aStart.SetRow(GetViewData()->GetCurY());
        aMarkRange.aStart.SetTab(GetViewData()->GetTabNo());
        aMarkRange.aEnd = aMarkRange.aStart;
        if ( pDoc->HasAttrib( aMarkRange, HASATTR_MERGED ) )
        {
            aFuncMark.SetMarkArea( aMarkRange );
        }
        else
            bSimple = true;
    }

    aFuncMark.SetMarking(false);        // for MarkToMulti
    aFuncMark.MarkToSimple();           // before bMulti test below

    OSL_ENSURE( aFuncMark.IsMarked() || aFuncMark.IsMultiMarked() || bSimple, "delete what?" );

    ScDocument* pUndoDoc = NULL;
    bool bMulti = !bSimple && aFuncMark.IsMultiMarked();
    if (!bSimple)
    {
        aFuncMark.MarkToMulti();
        aFuncMark.GetMultiMarkArea( aMarkRange );
    }
    ScRange aExtendedRange(aMarkRange);
    if (!bSimple)
    {
        if ( pDoc->ExtendMerge( aExtendedRange, true ) )
            bMulti = false;
    }

    // no objects on protected tabs
    bool bObjects = false;
    if ( nFlags & IDF_OBJECTS )
    {
        bObjects = true;
        ScMarkData::iterator itr = aFuncMark.begin(), itrEnd = aFuncMark.end();
        for (; itr != itrEnd; ++itr)
            if (pDoc->IsTabProtected(*itr))
                bObjects = false;
    }

    sal_uInt16 nExtFlags = 0;       // extra flags are needed only if attributes are deleted
    if ( nFlags & IDF_ATTRIB )
        pDocSh->UpdatePaintExt( nExtFlags, aMarkRange );

    //  order op opeeration:
    //  1) BeginDrawUndo
    //  2) delete objects (DrawUndo is filled)
    //  3) copy contents for undo
    //  4) delete contents
    //  5) add undo-action

    bool bDrawUndo = bObjects || ( nFlags & IDF_NOTE );     // needed for shown notes
    if ( bDrawUndo && bRecord )
        pDoc->BeginDrawUndo();

    if (bObjects)
    {
        if (bMulti)
            pDoc->DeleteObjectsInSelection( aFuncMark );
        else
            pDoc->DeleteObjectsInArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
/*!*/                                  aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                                       aFuncMark );
    }

    // To keep track of all non-empty cells within the deleted area.
    boost::shared_ptr<ScSimpleUndo::DataSpansType> pDataSpans;

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nTab = aMarkRange.aStart.Tab();
        pUndoDoc->InitUndo( pDoc, nTab, nTab );
        SCTAB nTabCount = pDoc->GetTableCount();
        ScMarkData::iterator itr = aFuncMark.begin(), itrEnd = aFuncMark.end();
        for (; itr != itrEnd; ++itr)
            if (*itr != nTab)
                pUndoDoc->AddUndoTab( *itr, *itr );
        ScRange aCopyRange = aExtendedRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);

        //  in case of "Format/Standard" copy all attributes, because CopyToDocument
        //  with IDF_HARDATTR only is too time-consuming:
        sal_uInt16 nUndoDocFlags = nFlags;
        if (nFlags & IDF_ATTRIB)
            nUndoDocFlags |= IDF_ATTRIB;
        if (nFlags & IDF_EDITATTR)          // Edit-Engine-Attribute
            nUndoDocFlags |= IDF_STRING;    // -> cells will be changed
        if (nFlags & IDF_NOTE)
            nUndoDocFlags |= IDF_CONTENTS;  // copy all cells with their notes
        // do not copy note captions to undo document
        nUndoDocFlags |= IDF_NOCAPTIONS;
        pDoc->CopyToDocument( aCopyRange, nUndoDocFlags, bMulti, pUndoDoc, &aFuncMark );

        pDataSpans.reset(new ScSimpleUndo::DataSpansType);

        for (itr = aFuncMark.begin(); itr != itrEnd; ++itr)
        {
            nTab = *itr;

            SCCOL nCol1 = aCopyRange.aStart.Col(), nCol2 = aCopyRange.aEnd.Col();
            SCROW nRow1 = aCopyRange.aStart.Row(), nRow2 = aCopyRange.aEnd.Row();

            std::pair<ScSimpleUndo::DataSpansType::iterator,bool> r =
                pDataSpans->insert(nTab, new sc::ColumnSpanSet(false));

            if (r.second)
            {
                ScSimpleUndo::DataSpansType::iterator it = r.first;
                sc::ColumnSpanSet* pSet = it->second;
                pSet->scan(*pDoc, nTab, nCol1, nRow1, nCol2, nRow2, true);
            }
        }
    }

    HideAllCursors();   // for if summary is cancelled
    if (bSimple)
        pDoc->DeleteArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                          aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                          aFuncMark, nFlags );
    else
    {
        pDoc->DeleteSelection( nFlags, aFuncMark );
    }

    if ( bRecord )
    {
        ScUndoDeleteContents* pUndo =
            new ScUndoDeleteContents(
                pDocSh, aFuncMark, aExtendedRange, pUndoDoc, bMulti, nFlags, bDrawUndo);
        pUndo->SetDataSpans(pDataSpans);

        pDocSh->GetUndoManager()->AddUndoAction(pUndo);
    }

    if (!AdjustRowHeight( aExtendedRange.aStart.Row(), aExtendedRange.aEnd.Row() ))
        pDocSh->PostPaint( aExtendedRange, PAINT_GRID, nExtFlags );

    pDocSh->UpdateOle(GetViewData());

    if (ScModelObj *pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh))
    {
        ScRangeList aChangeRanges;
        if ( bSimple )
        {
            aChangeRanges.Append( aMarkRange );
        }
        else
        {
            aFuncMark.FillRangeListWithMarks( &aChangeRanges, false );
        }
        HelperNotifyChanges::Notify(*pModelObj, aChangeRanges);
    }

    aModificator.SetDocumentModified();
    CellContentChanged();
    ShowAllCursors();

    if ( nFlags & IDF_ATTRIB )
    {
        if ( nFlags & IDF_CONTENTS )
            ForgetFormatArea();
        else
            StartFormatArea();              // delete attribute is also attribute-change
    }
}

//  column width/row height (via header) - undo OK

void ScViewFunc::SetWidthOrHeight(
    bool bWidth, const std::vector<sc::ColRowSpan>& rRanges, ScSizeMode eMode,
    sal_uInt16 nSizeTwips, bool bRecord, bool bPaint, ScMarkData* pMarkData )
{
    if (rRanges.empty())
        return;

    // use view's mark if none specified
    if ( !pMarkData )
        pMarkData = &GetViewData()->GetMarkData();

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nFirstTab = pMarkData->GetFirstSelected();
    SCTAB nCurTab = GetViewData()->GetTabNo();
    SCTAB nTab;
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScDocShellModificator aModificator( *pDocSh );

    bool bAllowed = true;
    ScMarkData::iterator itr = pMarkData->begin(), itrEnd = pMarkData->end();
    for (; itr != itrEnd && bAllowed; ++itr)
    {
        for (size_t i = 0, n = rRanges.size(); i < n && bAllowed; ++i)
        {
            bool bOnlyMatrix;
            if (bWidth)
            {
                bAllowed = pDoc->IsBlockEditable(
                    *itr, rRanges[i].mnStart, 0, rRanges[i].mnEnd, MAXROW,
                        &bOnlyMatrix ) || bOnlyMatrix;
            }
            else
            {
                bAllowed = pDoc->IsBlockEditable(
                    *itr, 0, rRanges[i].mnStart, MAXCOL,rRanges[i].mnEnd, &bOnlyMatrix) || bOnlyMatrix;
            }
        }
    }

    // Allow users to resize cols/rows in readonly docs despite the r/o state.
    // It is frustrating to be unable to see content in mis-sized cells.
    if( !bAllowed && !pDocSh->IsReadOnly() )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    SCCOLROW nStart = rRanges[0].mnStart;
    SCCOLROW nEnd = rRanges[0].mnEnd;

    bool bFormula = false;
    if ( eMode == SC_SIZE_OPTIMAL )
    {
        const ScViewOptions& rOpts = GetViewData()->GetOptions();
        bFormula = rOpts.GetOption( VOPT_FORMULAS );
    }

    ScDocument*     pUndoDoc = NULL;
    ScOutlineTable* pUndoTab = NULL;
    std::vector<sc::ColRowSpan> aUndoRanges;

    if ( bRecord )
    {
        pDoc->BeginDrawUndo();                          // Drawing Updates

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        itr = pMarkData->begin();
        for (; itr != itrEnd; ++itr)
        {
            if (bWidth)
            {
                if ( *itr == nFirstTab )
                    pUndoDoc->InitUndo( pDoc, *itr, *itr, true, false );
                else
                    pUndoDoc->AddUndoTab( *itr, *itr, true, false );
                pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, *itr,
                        static_cast<SCCOL>(nEnd), MAXROW, *itr, IDF_NONE,
                        false, pUndoDoc );
            }
            else
            {
                if ( *itr == nFirstTab )
                    pUndoDoc->InitUndo( pDoc, *itr, *itr, false, true );
                else
                    pUndoDoc->AddUndoTab( *itr, *itr, false, true );
                pDoc->CopyToDocument( 0, nStart, *itr, MAXCOL, nEnd, *itr, IDF_NONE, false, pUndoDoc );
            }
        }

        aUndoRanges = rRanges;

        //! outlines from all tab?
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nCurTab );
        if (pTable)
            pUndoTab = new ScOutlineTable( *pTable );
    }

    if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
        pMarkData->MarkToMulti();

    bool bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    bool bOutline = false;

    itr = pMarkData->begin();
    for (; itr != itrEnd; ++itr)
    {
        nTab = *itr;

        for (size_t i = 0, n = rRanges.size(); i < n; ++i)
        {
            SCCOLROW nStartNo = rRanges[i].mnStart;
            SCCOLROW nEndNo = rRanges[i].mnEnd;

            if ( !bWidth )                      // height always blockwise
            {
                if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                {
                    bool bAll = ( eMode==SC_SIZE_OPTIMAL );
                    if (!bAll)
                    {
                        //  delete CR_MANUALSIZE for all in range,
                        //  then SetOptimalHeight with bShrink = FALSE
                        for (SCROW nRow = nStartNo; nRow <= nEndNo; ++nRow)
                        {
                            SCROW nLastRow = nRow;
                            if (pDoc->RowHidden(nRow, nTab, NULL, &nLastRow))
                            {
                                nRow = nLastRow;
                                continue;
                            }

                            sal_uInt8 nOld = pDoc->GetRowFlags(nRow, nTab);
                            if (nOld & CR_MANUALSIZE)
                                pDoc->SetRowFlags(nRow, nTab, nOld & ~CR_MANUALSIZE);
                        }
                    }

                    double nPPTX = GetViewData()->GetPPTX();
                    double nPPTY = GetViewData()->GetPPTY();
                    Fraction aZoomX = GetViewData()->GetZoomX();
                    Fraction aZoomY = GetViewData()->GetZoomY();

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
                    pDoc->SetOptimalHeight(aCxt, nStartNo, nEndNo, nTab);
                    if (bAll)
                        pDoc->ShowRows( nStartNo, nEndNo, nTab, true );

                    //  Manual-Flag already (re)set in SetOptimalHeight in case of bAll=sal_True
                    //  (set for Extra-Height, else reset).
                }
                else if ( eMode==SC_SIZE_DIRECT )
                {
                    if (nSizeTwips)
                    {
                        pDoc->SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
                        pDoc->SetManualHeight( nStartNo, nEndNo, nTab, true );          // height was set manually
                    }
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );
                }
                else if ( eMode==SC_SIZE_SHOW )
                {
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, true );
                }
            }
            else                                // column width
            {
                for (SCCOL nCol=static_cast<SCCOL>(nStartNo); nCol<=static_cast<SCCOL>(nEndNo); nCol++)
                {
                    if ( eMode != SC_SIZE_VISOPT || !pDoc->ColHidden(nCol, nTab) )
                    {
                        sal_uInt16 nThisSize = nSizeTwips;

                        if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                            nThisSize = nSizeTwips + GetOptimalColWidth( nCol, nTab, bFormula );
                        if ( nThisSize )
                            pDoc->SetColWidth( nCol, nTab, nThisSize );

                        pDoc->ShowCol( nCol, nTab, bShow );
                    }
                }
            }

                                    //  adjust outline

            if (bWidth)
            {
                if ( pDoc->UpdateOutlineCol( static_cast<SCCOL>(nStartNo),
                            static_cast<SCCOL>(nEndNo), nTab, bShow ) )
                    bOutline = true;
            }
            else
            {
                if ( pDoc->UpdateOutlineRow( nStartNo, nEndNo, nTab, bShow ) )
                    bOutline = true;
            }
        }
        pDoc->SetDrawPageSize(nTab);
    }


    if (!bOutline)
        DELETEZ(pUndoTab);

    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoWidthOrHeight(
                pDocSh, *pMarkData, nStart, nCurTab, nEnd, nCurTab,
                pUndoDoc, aUndoRanges, pUndoTab, eMode, nSizeTwips, bWidth));
    }

    // fdo#36247 Ensure that the drawing layer's map mode scaling factors match
    // the new heights and widths.
    GetViewData()->GetView()->RefreshZoom();

    itr = pMarkData->begin();
    for (; itr != itrEnd; ++itr)
        pDoc->UpdatePageBreaks( *itr );

    GetViewData()->GetView()->UpdateScrollBars();

    if (bPaint)
    {
        itr = pMarkData->begin();
        for (; itr != itrEnd; ++itr)
        {
            nTab = *itr;
            if (bWidth)
            {
                if (pDoc->HasAttrib( static_cast<SCCOL>(nStart),0,nTab,
                            static_cast<SCCOL>(nEnd),MAXROW,nTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ))
                    nStart = 0;
                if (nStart > 0)             // go upwards because of Lines and cursor
                    --nStart;
                pDocSh->PostPaint( static_cast<SCCOL>(nStart), 0, nTab,
                        MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_TOP );
            }
            else
            {
                if (pDoc->HasAttrib( 0,nStart,nTab, MAXCOL,nEnd,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
                    nStart = 0;
                if (nStart != 0)
                    --nStart;
                pDocSh->PostPaint( 0, nStart, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_LEFT );
            }
        }

        pDocSh->UpdateOle(GetViewData());
        if( !pDocSh->IsReadOnly() )
        aModificator.SetDocumentModified();
    }

    if ( bWidth )
    {
        if (ScModelObj* pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh))
        {
            ScRangeList aChangeRanges;
            itr = pMarkData->begin();
            for (; itr != itrEnd; ++itr)
            {
                nTab = *itr;
                for (size_t i = 0, n = rRanges.size(); i < n; ++i)
                {
                    SCCOL nStartCol = rRanges[i].mnStart;
                    SCCOL nEndCol   = rRanges[i].mnEnd;
                    for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
                    {
                        aChangeRanges.Append( ScRange( nCol, 0, nTab ) );
                    }
                }
            }
            HelperNotifyChanges::Notify(*pModelObj, aChangeRanges, "column-resize");
        }
    }
}

//  column width/row height (via marked range)

void ScViewFunc::SetMarkedWidthOrHeight( bool bWidth, ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                        bool bRecord, bool bPaint )
{
    ScMarkData& rMark = GetViewData()->GetMarkData();

    rMark.MarkToMulti();
    if (!rMark.IsMultiMarked())
    {
        SCCOL nCol = GetViewData()->GetCurX();
        SCROW nRow = GetViewData()->GetCurY();
        SCTAB nTab = GetViewData()->GetTabNo();
        DoneBlockMode();
        InitOwnBlockMode();
        rMark.SetMultiMarkArea( ScRange( nCol,nRow,nTab ), true );
        MarkDataChanged();
    }

    std::vector<sc::ColRowSpan> aRanges =
        bWidth ? rMark.GetMarkedColSpans() : rMark.GetMarkedRowSpans();

    SetWidthOrHeight(bWidth, aRanges, eMode, nSizeTwips, bRecord, bPaint);

    rMark.MarkToSimple();
}

void ScViewFunc::ModifyCellSize( ScDirection eDir, bool bOptimal )
{
    //! step size adjustable
    //  step size is also minumum
    sal_uInt16 nStepX = STD_COL_WIDTH / 5;
    sal_uInt16 nStepY = ScGlobal::nStdRowHeight;

    ScModule* pScMod = SC_MOD();
    bool bAnyEdit = pScMod->IsInputMode();
    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

    bool bAllowed, bOnlyMatrix;
    if ( eDir == DIR_LEFT || eDir == DIR_RIGHT )
        bAllowed = pDoc->IsBlockEditable( nTab, nCol,0, nCol,MAXROW, &bOnlyMatrix );
    else
        bAllowed = pDoc->IsBlockEditable( nTab, 0,nRow, MAXCOL,nRow, &bOnlyMatrix );
    if ( !bAllowed && !bOnlyMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    HideAllCursors();

    sal_uInt16 nWidth = pDoc->GetColWidth( nCol, nTab );
    sal_uInt16 nHeight = pDoc->GetRowHeight( nRow, nTab );
    std::vector<sc::ColRowSpan> aRange(1, sc::ColRowSpan(0,0));
    if ( eDir == DIR_LEFT || eDir == DIR_RIGHT )
    {
        if (bOptimal)               // width of this single cell
        {
            if ( bAnyEdit )
            {
                //  when editing the actual entered width
                ScInputHandler* pHdl = pScMod->GetInputHdl( GetViewData()->GetViewShell() );
                if (pHdl)
                {
                    long nEdit = pHdl->GetTextSize().Width();       // in 0.01 mm

                    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
                    const SvxMarginItem& rMItem =
                            (const SvxMarginItem&)pPattern->GetItem(ATTR_MARGIN);
                    sal_uInt16 nMargin = rMItem.GetLeftMargin() + rMItem.GetRightMargin();
                    if ( ((const SvxHorJustifyItem&) pPattern->
                            GetItem( ATTR_HOR_JUSTIFY )).GetValue() == SVX_HOR_JUSTIFY_LEFT )
                        nMargin = sal::static_int_cast<sal_uInt16>(
                            nMargin + ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue() );

                    nWidth = (sal_uInt16)(nEdit * pDocSh->GetOutputFactor() / HMM_PER_TWIPS)
                                + nMargin + STD_EXTRA_WIDTH;
                }
            }
            else
            {
                double nPPTX = GetViewData()->GetPPTX();
                double nPPTY = GetViewData()->GetPPTY();
                Fraction aZoomX = GetViewData()->GetZoomX();
                Fraction aZoomY = GetViewData()->GetZoomY();

                ScSizeDeviceProvider aProv(pDocSh);
                if (aProv.IsPrinter())
                {
                    nPPTX = aProv.GetPPTX();
                    nPPTY = aProv.GetPPTY();
                    aZoomX = aZoomY = Fraction( 1, 1 );
                }

                long nPixel = pDoc->GetNeededSize( nCol, nRow, nTab, aProv.GetDevice(),
                                            nPPTX, nPPTY, aZoomX, aZoomY, true );
                sal_uInt16 nTwips = (sal_uInt16)( nPixel / nPPTX );
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
            const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            bool bNeedHeight =
                    ((const SfxBoolItem&)pPattern->GetItem( ATTR_LINEBREAK )).GetValue() ||
                    ((const SvxHorJustifyItem&)pPattern->
                        GetItem( ATTR_HOR_JUSTIFY )).GetValue() == SVX_HOR_JUSTIFY_BLOCK;
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
        if ( pDoc->HasAttrib( nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_NEEDHEIGHT ) )
        {
            ScInputHandler* pHdl = pScMod->GetInputHdl( GetViewData()->GetViewShell() );
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

    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    bool bUndo(pDoc->IsUndoEnabled());

    //  modifying several tabs is handled here

    if (bUndo)
    {
        OUString aUndo = ScGlobal::GetRscString( STR_UNDO_PROTECT_TAB );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
    }

    ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd; ++itr)
        rFunc.ProtectSheet(*itr, rProtect);

    if (bUndo)
        pDocSh->GetUndoManager()->LeaveListAction();

    UpdateLayerLocks();         //! broadcast to all views
}

void ScViewFunc::Protect( SCTAB nTab, const OUString& rPassword )
{
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    bool bUndo(pDoc->IsUndoEnabled());

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
        rFunc.Protect( nTab, rPassword, false );
    else
    {
        //  modifying several tabs is handled here

        if (bUndo)
        {
            OUString aUndo = ScGlobal::GetRscString( STR_UNDO_PROTECT_TAB );
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
        }

        ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd; ++itr)
            rFunc.Protect( *itr, rPassword, false );

        if (bUndo)
            pDocSh->GetUndoManager()->LeaveListAction();
    }

    UpdateLayerLocks();         //! broadcast to all views
}

bool ScViewFunc::Unprotect( SCTAB nTab, const OUString& rPassword )
{
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    bool bChanged = false;
    bool bUndo (pDoc->IsUndoEnabled());

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
        bChanged = rFunc.Unprotect( nTab, rPassword, false );
    else
    {
        //  modifying several tabs is handled here

        if (bUndo)
        {
            OUString aUndo = ScGlobal::GetRscString( STR_UNDO_UNPROTECT_TAB );
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
        }

        ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd; ++itr)
            if ( rFunc.Unprotect( *itr, rPassword, false ) )
                    bChanged = true;

        if (bUndo)
            pDocSh->GetUndoManager()->LeaveListAction();
    }

    if (bChanged)
        UpdateLayerLocks();     //! broadcast to all views

    return bChanged;
}

void ScViewFunc::SetNoteText( const ScAddress& rPos, const OUString& rNoteText )
{
    GetViewData()->GetDocShell()->GetDocFunc().SetNoteText( rPos, rNoteText, false );
}

void ScViewFunc::ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate )
{
    GetViewData()->GetDocShell()->GetDocFunc().ReplaceNote( rPos, rNoteText, pAuthor, pDate, false );
}

void ScViewFunc::SetNumberFormat( short nFormatType, sal_uLong nAdd )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    sal_uInt32          nNumberFormat = 0;
    ScViewData*         pViewData = GetViewData();
    ScDocument*         pDoc = pViewData->GetDocument();
    SvNumberFormatter*  pNumberFormatter = pDoc->GetFormatTable();
    LanguageType        eLanguage = ScGlobal::eLnge;
    ScPatternAttr       aNewAttrs( pDoc->GetPool() );

    //  always take language from cursor position, even if there is a selection

    sal_uInt32 nCurrentNumberFormat;
    pDoc->GetNumberFormat( pViewData->GetCurX(),
                           pViewData->GetCurY(),
                           pViewData->GetTabNo(),
                           nCurrentNumberFormat );
    const SvNumberformat* pEntry = pNumberFormatter->GetEntry( nCurrentNumberFormat );
    if (pEntry)
        eLanguage = pEntry->GetLanguage();      // else keep ScGlobal::eLnge

    nNumberFormat = pNumberFormatter->GetStandardFormat( nFormatType, eLanguage ) + nAdd;

    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumberFormat ) );
    //  ATTR_LANGUAGE_FORMAT not
    ApplySelectionPattern( aNewAttrs, true );
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

    ScViewData*         pViewData = GetViewData();
    ScDocument*         pDoc = pViewData->GetDocument();
    SvNumberFormatter*  pFormatter = pDoc->GetFormatTable();

    //  language always from cursor position

    sal_uInt32 nCurrentNumberFormat;
    pDoc->GetNumberFormat( pViewData->GetCurX(), pViewData->GetCurY(),
                           pViewData->GetTabNo(), nCurrentNumberFormat );
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
        short       nType   = 0;        //! ???
        bOk = pFormatter->PutEntry( aFormat, nErrPos, nType, nNumberFormat, eLanguage );
    }

    if ( bOk )          // valid format?
    {
        ScPatternAttr aNewAttrs( pDoc->GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumberFormat ) );
        rSet.Put( SvxLanguageItem( eLanguage, ATTR_LANGUAGE_FORMAT ) );
        ApplySelectionPattern( aNewAttrs, true );
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

    ScDocument*         pDoc = GetViewData()->GetDocument();
    SvNumberFormatter*  pFormatter = pDoc->GetFormatTable();

    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();

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

    short nOldType = pOldEntry->GetType();
    if ( 0 == ( nOldType & (
                NUMBERFORMAT_NUMBER |  NUMBERFORMAT_CURRENCY | NUMBERFORMAT_PERCENT ) ) )
    {
        //  date, time, fraction, logical, text can not be changed
        //! in case of scientific the Numberformatter also can't
        bError = true;
    }

    //! SvNumberformat has a Member bStandard, but doesn't disclose it
    bool bWasStandard = ( nOldFormat == pFormatter->GetStandardIndex( eLanguage ) );
    if (bWasStandard)
    {
        //  with "Standard" the decimal places depend on cell content
        //  0 if empty or text -> no decimal places
        double nVal = pDoc->GetValue( ScAddress( nCol, nRow, nTab ) );

        //  the ways of the Numberformatters are unfathomable, so try:
        OUString aOut;
        Color* pCol;
        ((SvNumberformat*)pOldEntry)->GetOutputString( nVal, aOut, &pCol );

        nPrecision = 0;
        // 'E' for exponential is fixed in Numberformatter
        if ( aOut.indexOf((sal_Unicode)'E') >= 0 )
            bError = true;                              // exponential not changed
        else
        {
            OUString aDecSep( pFormatter->GetFormatDecimalSep( nOldFormat ) );
            sal_Int32 nPos = aOut.indexOf( aDecSep );
            if ( nPos >= 0 )
                nPrecision = aOut.getLength() - nPos - aDecSep.getLength();
            // else keep 0
        }
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
                bError = true;          // 0 is minumum
        }
    }

    if (!bError)
    {
        OUString aNewPicture = pFormatter->GenerateFormat(nOldFormat, eLanguage,
                                                          bThousand, bNegRed,
                                                          nPrecision, nLeading);

        nNewFormat = pFormatter->GetEntryKey( aNewPicture, eLanguage );
        if ( nNewFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_Int32 nErrPos = 0;
            short nNewType = 0;
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
        ApplySelectionPattern( aNewAttrs, true );
    }
}

void ScViewFunc::ChangeIndent( bool bIncrement )
{
    ScViewData* pViewData = GetViewData();
    ScDocShell* pDocSh  = pViewData->GetDocShell();
    ScMarkData& rMark   = pViewData->GetMarkData();

    ScMarkData aWorkMark = rMark;
    ScViewUtil::UnmarkFiltered( aWorkMark, pDocSh->GetDocument() );
    aWorkMark.MarkToMulti();
    if (!aWorkMark.IsMultiMarked())
    {
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();
        aWorkMark.SetMultiMarkArea( ScRange(nCol,nRow,nTab) );
    }

    bool bSuccess = pDocSh->GetDocFunc().ChangeIndent( aWorkMark, bIncrement, false );
    if (bSuccess)
    {
        pDocSh->UpdateOle(pViewData);
        StartFormatArea();

        // stuff for sidebar panels
        SfxBindings& rBindings = GetViewData()->GetBindings();
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
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScRangeName* pList = pDoc->GetRangeName();

    RangeType nType = RT_NAME;
    ScRangeData* pNewEntry = new ScRangeData( pDoc, rName, rSymbol,
            ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                nTab), nType );
    OUString aUpType = rType.toAsciiUpperCase();
    if ( aUpType.indexOf( 'P' ) != -1 )
        nType |= RT_PRINTAREA;
    if ( aUpType.indexOf( 'R' ) != -1 )
        nType |= RT_ROWHEADER;
    if ( aUpType.indexOf( 'C' ) != -1 )
        nType |= RT_COLHEADER;
    if ( aUpType.indexOf( 'F' ) != -1 )
        nType |= RT_CRITERIA;
    pNewEntry->AddType(nType);

    if ( !pNewEntry->GetErrCode() )     //  text valid?
    {
        ScDocShellModificator aModificator( *pDocSh );

        pDoc->PreprocessRangeNameUpdate();

        // input available yet? Then remove beforehand (=change)
        ScRangeData* pData = pList->findByUpperName(ScGlobal::pCharClass->uppercase(rName));
        if (pData)
        {                                   // take old Index
            pNewEntry->SetIndex(pData->GetIndex());
            pList->erase(*pData);
        }

        if ( pList->insert( pNewEntry ) )
            bOk = true;
        pNewEntry = NULL;   // never delete, insert took ownership

        pDoc->CompileHybridFormula();

        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
    }

    delete pNewEntry;       // if it wasn't inserted
    return bOk;
}

void ScViewFunc::CreateNames( sal_uInt16 nFlags )
{
    bool bDone = false;
    ScRange aRange;
    if ( GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE )
        bDone = GetViewData()->GetDocShell()->GetDocFunc().CreateNames( aRange, nFlags, false );

    if (!bDone)
        ErrorMessage(STR_CREATENAME_MARKERR);
}

sal_uInt16 ScViewFunc::GetCreateNameFlags()
{
    sal_uInt16 nFlags = 0;

    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    SCTAB nDummy;
    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nDummy,nEndCol,nEndRow,nDummy) == SC_MARK_SIMPLE)
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        SCTAB nTab = GetViewData()->GetTabNo();
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
            nFlags |= NAME_TOP;
        else                            // Bottom only if not Top
        {
            bOk = true;
            for (i=nFirstCol; i<=nLastCol && bOk; i++)
                if (!pDoc->HasStringData( i,nEndRow,nTab ))
                    bOk = false;
            if (bOk)
                nFlags |= NAME_BOTTOM;
        }

        bOk = true;
        SCROW nFirstRow = nStartRow;
        SCROW nLastRow  = nEndRow;
        if (nStartRow+1 < nEndRow) { ++nFirstRow; --nLastRow; }
        for (j=nFirstRow; j<=nLastRow && bOk; j++)
            if (!pDoc->HasStringData( nStartCol,j,nTab ))
                bOk = false;
        if (bOk)
            nFlags |= NAME_LEFT;
        else                            // Right only if not Left
        {
            bOk = true;
            for (j=nFirstRow; j<=nLastRow && bOk; j++)
                if (!pDoc->HasStringData( nEndCol,j,nTab ))
                    bOk = false;
            if (bOk)
                nFlags |= NAME_RIGHT;
        }
    }

    if (nStartCol == nEndCol)
        nFlags &= ~( NAME_LEFT | NAME_RIGHT );
    if (nStartRow == nEndRow)
        nFlags &= ~( NAME_TOP | NAME_BOTTOM );

    return nFlags;
}

void ScViewFunc::InsertNameList()
{
    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    if ( pDocSh->GetDocFunc().InsertNameList( aPos, false ) )
        pDocSh->UpdateOle(GetViewData());
}

void ScViewFunc::UpdateSelectionArea( const ScMarkData& rSel, ScPatternAttr* pAttr  )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
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
        bSetLines = rNewSet.GetItemState( ATTR_BORDER, true ) == SFX_ITEM_SET ||
        rNewSet.GetItemState( ATTR_SHADOW, true ) == SFX_ITEM_SET;
        bSetAlign = rNewSet.GetItemState( ATTR_HOR_JUSTIFY, true ) == SFX_ITEM_SET;
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
        PAINT_GRID, nExtFlags | SC_PF_TESTMERGE );
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    pTabViewShell->CellContentChanged();
    pTabViewShell->AdjustBlockHeight(false, const_cast<ScMarkData*>(&rSel));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
