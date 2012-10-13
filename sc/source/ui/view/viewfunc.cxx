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
#include <stdlib.h>             // qsort

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
#include "cell.hxx"
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
//==================================================================

static void lcl_PostRepaintCondFormat( const ScConditionalFormat *pCondFmt, ScDocShell *pDocSh )
{
    if( pCondFmt )
    {
        const ScRangeList& rRanges = pCondFmt->GetRange();

        pDocSh->PostPaint( rRanges, PAINT_ALL );
    }
}


//==================================================================

ScViewFunc::ScViewFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    ScTabView( pParent, rDocSh, pViewShell ),
    bFormatValid( false )
{
}

ScViewFunc::~ScViewFunc()
{
}

//------------------------------------------------------------------------------------

void ScViewFunc::StartFormatArea()
{
    //  anything to do?
    if ( !SC_MOD()->GetInputOptions().GetExtendFormat() )
        return;

    //  start only with single cell (marked or cursor position)
    ScRange aMarkRange;
    sal_Bool bOk = (GetViewData()->GetSimpleArea( aMarkRange ) == SC_MARK_SIMPLE);
    if ( bOk && aMarkRange.aStart != aMarkRange.aEnd )
        bOk = false;

    if (bOk)
    {
        bFormatValid = sal_True;
        aFormatSource = aMarkRange.aStart;
        aFormatArea = ScRange( aFormatSource );
    }
    else
        bFormatValid = false;       // discard old range
}

sal_Bool ScViewFunc::TestFormatArea( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bAttrChanged )
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

    sal_Bool bFound = false;
    ScRange aNewRange = aFormatArea;
    if ( bFormatValid && nTab == aFormatSource.Tab() )
    {
        if ( nRow >= aFormatArea.aStart.Row() && nRow <= aFormatArea.aEnd.Row() )
        {
            //  within range?
            if ( nCol >= aFormatArea.aStart.Col() && nCol <= aFormatArea.aEnd.Col() )
            {
                bFound = sal_True;          // do not change range
            }
            //  left ?
            if ( nCol+1 == aFormatArea.aStart.Col() )
            {
                bFound = sal_True;
                aNewRange.aStart.SetCol( nCol );
            }
            //  right ?
            if ( nCol == aFormatArea.aEnd.Col()+1 )
            {
                bFound = sal_True;
                aNewRange.aEnd.SetCol( nCol );
            }
        }
        if ( nCol >= aFormatArea.aStart.Col() && nCol <= aFormatArea.aEnd.Col() )
        {
            //  top ?
            if ( nRow+1 == aFormatArea.aStart.Row() )
            {
                bFound = sal_True;
                aNewRange.aStart.SetRow( nRow );
            }
            //  bottom ?
            if ( nRow == aFormatArea.aEnd.Row()+1 )
            {
                bFound = sal_True;
                aNewRange.aEnd.SetRow( nRow );
            }
        }
    }

    if (bFound)
        aFormatArea = aNewRange;    // extend
    else
    {
        bFormatValid = false;       // outdside of range -> break
        if ( bAttrChanged )         // if value entered with numberformat?
            StartFormatArea();      // then start again
    }

    return bFound;
}

void ScViewFunc::DoAutoAttributes( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                   sal_Bool bAttrChanged, sal_Bool bAddUndo )
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
            rFunc.ApplyStyle( aMark, pSrcStyle->GetName(), sal_True, sal_False );

        rFunc.ApplyAttributes( aMark, *pSource, sal_True, sal_False );
    }

    if ( bAttrChanged )                             // value entered with number format?
        aFormatSource.Set( nCol, nRow, nTab );      // then set a new source
}

//------------------------------------------------------------------------------------

//      additional routines

sal_uInt16 ScViewFunc::GetOptimalColWidth( SCCOL nCol, SCTAB nTab, sal_Bool bFormula )
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

sal_Bool ScViewFunc::SelectionEditable( bool* pOnlyNotBecauseOfMatrix /* = NULL */ )
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

static sal_Bool lcl_FunctionKnown( sal_uInt16 nOpCode )
{
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uLong nCount = pFuncList->GetCount();
        for (sal_uLong i=0; i<nCount; i++)
            if ( pFuncList->GetFunction(i)->nFIndex == nOpCode )
                return sal_True;
    }
    return false;
}

static sal_Bool lcl_AddFunction( ScAppOptions& rAppOpt, sal_uInt16 nOpCode )
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

            return sal_True;                        // list has changed
        }

    if ( !lcl_FunctionKnown( nOpCode ) )
        return false;                           // not in function list -> no change

    sal_uInt16 nNewCount = Min( (sal_uInt16)(nOldCount + 1), (sal_uInt16)LRU_MAX );
    sal_uInt16 nNewList[LRU_MAX];
    nNewList[0] = nOpCode;
    for (nPos=1; nPos<nNewCount; nPos++)
        nNewList[nPos] = pOldList[nPos-1];
    rAppOpt.SetLRUFuncList( nNewList, nNewCount );

    return sal_True;                                // list has changed
}

//      actual functions

//  input - undo OK

void ScViewFunc::EnterData( SCCOL nCol, SCROW nRow, SCTAB nTab,
                            const String& rString,
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
    if (aTester.IsEditable())
    {
        if ( bRecord )
            rFunc.EnterListAction( STR_UNDO_ENTERDATA );

        bool bFormula = false;

        // a single '=' character is handled as string (needed for special filters)
        if ( rString.Len() > 1 )
        {
            if ( rString.GetChar(0) == '=' )
            {
                // handle as formula
                bFormula = true;
            }
            else if ( rString.GetChar(0) == '+' || rString.GetChar(0) == '-' )
            {
                // if there is more than one leading '+' or '-' character, remove the additional ones
                String aString( rString );
                xub_StrLen nIndex = 1;
                xub_StrLen nLen = aString.Len();
                while ( nIndex < nLen && ( aString.GetChar( nIndex ) == '+' || aString.GetChar( nIndex ) == '-' ) )
                {
                    ++nIndex;
                }
                aString.Erase( 1, nIndex - 1 );

                // if the remaining part without the leading '+' or '-' character
                // is non-empty and not a number, handle as formula
                if ( aString.Len() > 1 )
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

        sal_Bool bNumFmtChanged = false;
        if ( bFormula )
        {   // formula, compile with autoCorrection
            i = rMark.GetFirstSelected();
            ScAddress aPos( nCol, nRow, i );
            ScCompiler aComp( pDoc, aPos);
            aComp.SetGrammar(pDoc->GetGrammar());
//2do: enable/disable autoCorrection via calcoptions
            aComp.SetAutoCorrection( sal_True );
            if ( rString.GetChar(0) == '+' || rString.GetChar(0) == '-' )
            {
                aComp.SetExtendedErrorDetection( ScCompiler::EXTENDED_ERROR_DETECTION_NAME_BREAK );
            }
            String aFormula( rString );
            ScTokenArray* pArr;
            sal_Bool bAgain;
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
                    String aCorrectedFormula;
                    if ( bAddEqual )
                    {
                        aCorrectedFormula = '=';
                        aCorrectedFormula += aComp.GetCorrectedFormula();
                    }
                    else
                        aCorrectedFormula = aComp.GetCorrectedFormula();
                    short nResult;
                    if ( aCorrectedFormula.Len() == 1 )
                        nResult = RET_NO;   // empty formula, just '='
                    else
                    {
                        String aMessage( ScResId( SCSTR_FORMULA_AUTOCORRECTION ) );
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
                        bAgain = sal_True;
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
                sal_Bool bOptChanged = false;

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
                        bOptChanged = sal_True;
                }

                if ( bOptChanged )
                {
                    pScMod->SetAppOptions(aAppOpt);
                    pScMod->RecentFunctionsChanged();
                }
            }

            ScFormulaCell aCell( pDoc, aPos, pArr,formula::FormulaGrammar::GRAM_DEFAULT, MM_NONE );
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
                     ( ( rString.GetChar(0) == '+' || rString.GetChar(0) == '-' ) && nError && rString.Equals( aFormula ) ) )
                {
                    ScBaseCell *pCell;
                    if ( pData )
                        pCell = new ScEditCell( pData, pDoc, NULL );
                    else
                        pCell = new ScStringCell( aFormula );
                    rFunc.PutCell( aPos, pCell, sal_False );
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
                    rFunc.PutCell( aPos, pCell, sal_False );
                }
            }
        }
        else
        {
            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for ( ; itr != itrEnd; ++itr )
            {
                bool bNumFmtSet = false;
                rFunc.SetNormalString( bNumFmtSet, ScAddress( nCol, nRow, *itr ), rString, sal_False );
                if (bNumFmtSet)
                {
                    /* FIXME: if set on any sheet results in changed only on
                     * sheet nTab for TestFormatArea() and DoAutoAttributes() */
                    bNumFmtChanged = true;
                }
            }
        }

        sal_Bool bAutoFormat = TestFormatArea(nCol, nRow, nTab, bNumFmtChanged);

        if (bAutoFormat)
            DoAutoAttributes(nCol, nRow, nTab, bNumFmtChanged, bRecord);

        pDocSh->UpdateOle(GetViewData());

        // #i97876# Spreadsheet data changes are not notified
        ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
        if ( pModelObj && pModelObj->HasChangesListeners() )
        {
            ScRangeList aChangeRanges;
            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd; ++itr)
                aChangeRanges.Append( ScRange( nCol, nRow, *itr ) );

            pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
        }

        if ( bRecord )
            rFunc.EndListAction();

        aModificator.SetDocumentModified();
        lcl_PostRepaintCondFormat( pDoc->GetCondFormat( nCol, nRow, nTab ), pDocSh );
    }
    else
    {
        ErrorMessage(aTester.GetMessageId());
        PaintArea( nCol, nRow, nCol, nRow );        // possibly the edit-engine is still painted there
    }
}

// enter value in single cell (on nTab only)

void ScViewFunc::EnterValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rValue )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();

    if ( pDoc && pDocSh )
    {
        sal_Bool bUndo(pDoc->IsUndoEnabled());
        ScDocShellModificator aModificator( *pDocSh );

        ScEditableTester aTester( pDoc, nTab, nCol,nRow, nCol,nRow );
        if (aTester.IsEditable())
        {
            ScAddress aPos( nCol, nRow, nTab );
            ScBaseCell* pOldCell = pDoc->GetCell( aPos );

            //  undo
            ScBaseCell* pUndoCell = (bUndo && pOldCell) ? pOldCell->Clone( *pDoc ) : 0;

            pDoc->SetValue( nCol, nRow, nTab, rValue );

            // because of ChangeTrack after change in document
            if (bUndo)
            {
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoEnterValue( pDocSh, aPos, pUndoCell, rValue ) );
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
                            const EditTextObject* pData, bool bTestSimple )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocument* pDoc = pDocSh->GetDocument();
    bool bRecord = pDoc->IsUndoEnabled();

    ScDocShellModificator aModificator( *pDocSh );

    ScEditableTester aTester( pDoc, nTab, nCol,nRow, nCol,nRow );
    if (aTester.IsEditable())
    {
        //
        //      test for attribute
        //
        sal_Bool bSimple = false;
        sal_Bool bCommon = false;
        ScPatternAttr* pCellAttrs = NULL;
        EditTextObject* pNewData = NULL;
        String aString;

        const ScPatternAttr* pOldPattern = pDoc->GetPattern( nCol, nRow, nTab );
        ScTabEditEngine aEngine( *pOldPattern, pDoc->GetEnginePool() );
        aEngine.SetText(*pData);

        if (bTestSimple)                    // test, if simple string without attribute
        {
            ScEditAttrTester aAttrTester( &aEngine );
            bSimple = !aAttrTester.NeedsObject();
            bCommon = aAttrTester.NeedsCellAttr();

            // formulas have to be recognized even if they're formatted
            // (but commmon attributes are still collected)

            if ( !bSimple && aEngine.GetParagraphCount() == 1 )
            {
                String aParStr = aEngine.GetText( (sal_uInt16) 0 );
                if ( aParStr.GetChar(0) == '=' )
                    bSimple = sal_True;
            }

            if (bCommon)                // attribute for tab
            {
                pCellAttrs = new ScPatternAttr( *pOldPattern );
                pCellAttrs->GetFromEditItemSet( &aAttrTester.GetAttribs() );
                //! remove common attributes from EditEngine?
            }
        }

        // #i97726# always get text for "repeat" of undo action
        aString = ScEditUtil::GetSpaceDelimitedString(aEngine);

        //
        //      undo
        //

        SCTAB nSelCount = rMark.GetSelectCount();
        ScBaseCell** ppOldCells = NULL;
        SCTAB* pTabs            = NULL;
        SCTAB nPos = 0;
        EditTextObject* pUndoData = NULL;
        if (bRecord && !bSimple)
        {
            ppOldCells  = new ScBaseCell*[nSelCount];
            pTabs       = new SCTAB[nSelCount];
            nPos = 0;

            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd; ++itr)
            {
                pTabs[nPos] = *itr;
                ScBaseCell* pDocCell;
                pDoc->GetCell( nCol, nRow, *itr, pDocCell );
                ppOldCells[nPos] = pDocCell ? pDocCell->Clone( *pDoc ) : 0;
                ++nPos;
            }

            OSL_ENSURE( nPos==nSelCount, "nPos!=nSelCount" );

            pUndoData = pData->Clone();
        }

        //
        //      enter data
        //

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
                pDoc->PutCell( nCol, nRow, *itr, new ScEditCell( pData, pDoc, NULL ) );

            if ( bRecord )
            {   //  because of ChangeTrack current first
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoEnterData( pDocSh, nCol, nRow, nTab, nPos, pTabs,
                                        ppOldCells, NULL, NULL, aString,
                                        pUndoData ) );
            }

            HideAllCursors();

            AdjustRowHeight(nRow,nRow);

            itr = rMark.begin();
            for (; itr != itrEnd; ++itr)
                pDocSh->PostPaintCell( nCol, nRow, *itr );

            ShowAllCursors();

            pDocSh->UpdateOle(GetViewData());

            // #i97876# Spreadsheet data changes are not notified
            ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
            if ( pModelObj && pModelObj->HasChangesListeners() )
            {
                ScRangeList aChangeRanges;
                itr = rMark.begin();
                for (; itr != itrEnd; ++itr)
                {
                    aChangeRanges.Append( ScRange( nCol, nRow, *itr ) );
                }
                pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
            }

            aModificator.SetDocumentModified();
        }
        lcl_PostRepaintCondFormat( pDoc->GetCondFormat( nCol, nRow, nTab ), pDocSh );

        delete pCellAttrs;
        delete pNewData;
    }
    else
    {
        ErrorMessage(aTester.GetMessageId());
        PaintArea( nCol, nRow, nCol, nRow );        // possibly the edit-engine is still painted there
    }
}

void ScViewFunc::EnterDataAtCursor( const String& rString )
{
    SCCOL nPosX = GetViewData()->GetCurX();
    SCROW nPosY = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();

    EnterData( nPosX, nPosY, nTab, rString );
}

void ScViewFunc::EnterMatrix( const String& rString, ::formula::FormulaGrammar::Grammar eGram )
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
            aRange, &rMark, NULL, rString, false, false, EMPTY_STRING, eGram );
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
                            GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
    }
    else
    {
        ScRangeList aRanges;
        rMark.FillRangeListWithMarks( &aRanges, false );
        size_t nCount = aRanges.size();
        for ( size_t i=0; i < nCount; i++ )
        {
            ScRange aRange = *aRanges[i];
            ScCellIterator aIter( pDoc, aRange );
            ScBaseCell* pCell = aIter.GetFirst();
            while ( pCell )
            {
                nScript |= pDoc->GetScriptType( aIter.GetCol(), aIter.GetRow(), aIter.GetTab(), pCell );
                pCell = aIter.GetNext();
            }
        }
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
        rLineInner.SetDist(sal_True);
        rLineInner.SetMinDist(false);
    }
}

//
//  apply attribute - undo OK
//
//  complete set ( ATTR_STARTINDEX, ATTR_ENDINDEX )
//

void ScViewFunc::ApplyAttributes( const SfxItemSet* pDialogSet,
                                  const SfxItemSet* pOldSet,
                                  sal_Bool              bRecord )
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

    sal_Bool bFrame =    (pDialogSet->GetItemState( ATTR_BORDER ) != SFX_ITEM_DEFAULT)
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

        sal_Bool bDefNewOuter = ( SFX_ITEMS_STATICDEFAULT == pNewOuter->GetKind() );
        sal_Bool bDefNewInner = ( SFX_ITEMS_STATICDEFAULT == pNewInner->GetKind() );

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
                                    const SvxBoxInfoItem* pNewInner, sal_Bool bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData aFuncMark( GetViewData()->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScRange aMarkRange;
    aFuncMark.MarkToSimple();
    sal_Bool bMulti = aFuncMark.IsMultiMarked();
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
                                            sal_Bool bRecord, sal_Bool bCursorOnly )
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
    sal_Bool bSetLines = rNewSet.GetItemState( ATTR_BORDER, sal_True ) == SFX_ITEM_SET ||
                     rNewSet.GetItemState( ATTR_SHADOW, sal_True ) == SFX_ITEM_SET;
    sal_Bool bSetAlign = rNewSet.GetItemState( ATTR_HOR_JUSTIFY, sal_True ) == SFX_ITEM_SET;

    sal_uInt16 nExtFlags = 0;
    if ( bSetLines )
        nExtFlags |= SC_PF_LINES;
    if ( bSetAlign )
        nExtFlags |= SC_PF_WHOLEROWS;

    ScDocShellModificator aModificator( *pDocSh );

    sal_Bool bMulti = aFuncMark.IsMultiMarked();
    aFuncMark.MarkToMulti();
    sal_Bool bOnlyTab = (!aFuncMark.IsMultiMarked() && !bCursorOnly && aFuncMark.GetSelectCount() > 1);
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

        ScBaseCell* pCell;
        pDoc->GetCell(nCol, nRow, nTab, pCell);
        EditTextObject* pOldEditData = NULL;
        EditTextObject* pNewEditData = NULL;
        if (pCell && pCell->GetCellType() == CELLTYPE_EDIT)
        {
            ScEditCell* pEditCell = static_cast<ScEditCell*>(pCell);
            pOldEditData = pEditCell->GetData()->Clone();
            pEditCell->RemoveCharAttribs(rAttr);
            pNewEditData = pEditCell->GetData()->Clone();
        }

        aChangeRanges.Append( ScRange( nCol, nRow, nTab ) );
        ScPatternAttr* pOldPat = new ScPatternAttr(*pDoc->GetPattern( nCol, nRow, nTab ));

        pDoc->ApplyPattern( nCol, nRow, nTab, rAttr );

        const ScPatternAttr* pNewPat = pDoc->GetPattern( nCol, nRow, nTab );

        if (bRecord)
        {
            ScUndoCursorAttr* pUndo = new ScUndoCursorAttr(
                pDocSh, nCol, nRow, nTab, pOldPat, pNewPat, &rAttr, false );
            pUndo->SetEditData(pOldEditData, pNewEditData);
            pDocSh->GetUndoManager()->AddUndoAction(pUndo);
        }
        delete pOldPat;     // is copied in undo (Pool)

        pDocSh->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID, nExtFlags | SC_PF_TESTMERGE );
        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
    }

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
    if ( pModelObj && pModelObj->HasChangesListeners() )
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aProperties;
        sal_Int32 nCount = 0;
        const SfxItemPropertyMap& rMap = ScCellObj::GetCellPropertyMap();
        PropertyEntryVector_t aPropVector = rMap.getPropertyEntries();
        for ( sal_uInt16 nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; ++nWhich )
        {
            const SfxPoolItem* pItem = 0;
            if ( rNewSet.GetItemState( nWhich, sal_True, &pItem ) == SFX_ITEM_SET && pItem )
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
        pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "attribute" ) ), aChangeRanges, aProperties );
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

void ScViewFunc::SetStyleSheetToMarked( SfxStyleSheet* pStyleSheet, sal_Bool bRecord )
{
    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    if ( !pStyleSheet) return;
    // -------------------------------------------------------------------

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
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, sal_True, pUndoDoc, &aFuncMark );
            aFuncMark.MarkToMulti();

            String aName = pStyleSheet->GetName();
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

            String aName = pStyleSheet->GetName();
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
    // -------------------------------------------------------------------

    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScDocShell* pDocSh      = pViewData->GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    VirtualDevice aVirtDev;
    aVirtDev.SetMapMode(MAP_PIXEL);
    pDoc->StyleSheetChanged( pStyleSheet, sal_True, &aVirtDev,
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
    // -------------------------------------------------------------------

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

sal_Bool ScViewFunc::InsertCells( InsCellCmd eCmd, sal_Bool bRecord, sal_Bool bPartOfPaste )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();
        sal_Bool bSuccess = pDocSh->GetDocFunc().InsertCells( aRange, &rMark, eCmd, bRecord, false, bPartOfPaste );
        if (bSuccess)
        {
            pDocSh->UpdateOle(GetViewData());
            CellContentChanged();

            // #i97876# Spreadsheet data changes are not notified
            ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
            if ( pModelObj && pModelObj->HasChangesListeners() )
            {
                if ( eCmd == INS_INSROWS || eCmd == INS_INSCOLS )
                {
                    ScRangeList aChangeRanges;
                    aChangeRanges.Append( aRange );
                    ::rtl::OUString aOperation = ( eCmd == INS_INSROWS ?
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert-rows" ) ) :
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert-columns" ) ) );
                    pModelObj->NotifyChanges( aOperation, aChangeRanges );
                }
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

void ScViewFunc::DeleteCells( DelCellCmd eCmd, sal_Bool bRecord )
{
    ScRange aRange;
    if ( GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();

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
        {
            pDocSh->GetDocFunc().DeleteCells( aRange, &rMark, eCmd, bRecord, false );
        }

        pDocSh->UpdateOle(GetViewData());
        CellContentChanged();

        // #i97876# Spreadsheet data changes are not notified
        ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
        if ( pModelObj && pModelObj->HasChangesListeners() )
        {
            if ( eCmd == DEL_DELROWS || eCmd == DEL_DELCOLS )
            {
                ScRangeList aChangeRanges;
                aChangeRanges.Append( aRange );
                ::rtl::OUString aOperation = ( eCmd == DEL_DELROWS ?
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete-rows" ) ) :
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete-columns" ) ) );
                pModelObj->NotifyChanges( aOperation, aChangeRanges );
            }
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
            DeleteMulti( sal_True, bRecord );
        else
            ErrorMessage(STR_NOMULTISELECT);
    }

    Unmark();
}

void ScViewFunc::DeleteMulti( sal_Bool bRows, sal_Bool bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );
    SCTAB nTab = GetViewData()->GetTabNo();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData aFuncMark( GetViewData()->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    SCCOLROW* pRanges = new SCCOLROW[MAXCOLROWCOUNT];
    SCCOLROW nRangeCnt = bRows ? aFuncMark.GetMarkRowRanges( pRanges ) :
                                aFuncMark.GetMarkColumnRanges( pRanges );
    if (nRangeCnt == 0)
    {
        pRanges[0] = pRanges[1] = bRows ? static_cast<SCCOLROW>(GetViewData()->GetCurY()) : static_cast<SCCOLROW>(GetViewData()->GetCurX());
        nRangeCnt = 1;
    }

    //  test if allowed

    SCCOLROW* pOneRange = pRanges;
    sal_uInt16 nErrorId = 0;
    sal_Bool bNeedRefresh = false;
    SCCOLROW nRangeNo;
    for (nRangeNo=0; nRangeNo<nRangeCnt && !nErrorId; nRangeNo++)
    {
        SCCOLROW nStart = *(pOneRange++);
        SCCOLROW nEnd = *(pOneRange++);

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
        if ( nRangeNo == 0 )
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

            bNeedRefresh = sal_True;
        }
    }

    if ( nErrorId )
    {
        ErrorMessage( nErrorId );
        delete[] pRanges;
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

        pOneRange = pRanges;
        for (nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
        {
            SCCOLROW nStart = *(pOneRange++);
            SCCOLROW nEnd = *(pOneRange++);
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

    pOneRange = &pRanges[2*nRangeCnt];      // backwards
    for (nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        SCCOLROW nEnd = *(--pOneRange);
        SCCOLROW nStart = *(--pOneRange);

        if (bRows)
            pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, nStart, static_cast<SCSIZE>(nEnd-nStart+1) );
        else
            pDoc->DeleteCol( 0,nTab, MAXROW,nTab, static_cast<SCCOL>(nStart), static_cast<SCSIZE>(nEnd-nStart+1) );
    }

    if (bNeedRefresh)
    {
        SCCOLROW nFirstStart = pRanges[0];
        SCCOL nStartCol = bRows ? 0 : static_cast<SCCOL>(nFirstStart);
        SCROW nStartRow = bRows ? static_cast<SCROW>(nFirstStart) : 0;
        SCCOL nEndCol = MAXCOL;
        SCROW nEndRow = MAXROW;

        pDoc->RemoveFlagsTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );
        pDoc->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab, sal_True );
    }

    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoDeleteMulti( pDocSh, bRows, bNeedRefresh, nTab, pRanges, nRangeCnt,
                                    pUndoDoc, pUndoData ) );
    }

    if (!AdjustRowHeight(0, MAXROW))
    {
        if (bRows)
            pDocSh->PostPaint( 0,pRanges[0],nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_LEFT );
        else
            pDocSh->PostPaint( static_cast<SCCOL>(pRanges[0]),0,nTab,
                    MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_TOP );
    }
    aModificator.SetDocumentModified();

    CellContentChanged();

    //  put cursor directly behind the first deleted range
    SCCOL nCurX = GetViewData()->GetCurX();
    SCROW nCurY = GetViewData()->GetCurY();
    if ( bRows )
        nCurY = pRanges[0];
    else
        nCurX = static_cast<SCCOL>(pRanges[0]);
    SetCursor( nCurX, nCurY );

    delete[] pRanges;

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

//  delete contents

void ScViewFunc::DeleteContents( sal_uInt16 nFlags, sal_Bool bRecord )
{
    ScViewData* pViewData = GetViewData();
    pViewData->SetPasteMode( SC_PASTE_NONE );
    pViewData->GetViewShell()->UpdateCopySourceOverlay();

    // not editable because of matrix only? attribute OK nonetheless
    bool bOnlyNotBecauseOfMatrix;
    sal_Bool bEditable = SelectionEditable( &bOnlyNotBecauseOfMatrix );
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
    sal_Bool bSimple = false;

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
            bSimple = sal_True;
    }

    aFuncMark.SetMarking(false);        // for MarkToMulti
    aFuncMark.MarkToSimple();           // before bMulti test below

    OSL_ENSURE( aFuncMark.IsMarked() || aFuncMark.IsMultiMarked() || bSimple, "delete what?" );

    ScDocument* pUndoDoc = NULL;
    sal_Bool bMulti = !bSimple && aFuncMark.IsMultiMarked();
    if (!bSimple)
    {
        aFuncMark.MarkToMulti();
        aFuncMark.GetMultiMarkArea( aMarkRange );
    }
    ScRange aExtendedRange(aMarkRange);
    if (!bSimple)
    {
        if ( pDoc->ExtendMerge( aExtendedRange, sal_True ) )
            bMulti = false;
    }

    // no objects on protected tabs
    sal_Bool bObjects = false;
    if ( nFlags & IDF_OBJECTS )
    {
        bObjects = sal_True;
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

    sal_Bool bDrawUndo = bObjects || ( nFlags & IDF_NOTE );     // needed for shown notes
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
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoDeleteContents( pDocSh, aFuncMark, aExtendedRange,
                                      pUndoDoc, bMulti, nFlags, bDrawUndo ) );
    }

    if (!AdjustRowHeight( aExtendedRange.aStart.Row(), aExtendedRange.aEnd.Row() ))
        pDocSh->PostPaint( aExtendedRange, PAINT_GRID, nExtFlags );

    pDocSh->UpdateOle(GetViewData());

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
    if ( pModelObj && pModelObj->HasChangesListeners() )
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
        pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
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

void ScViewFunc::SetWidthOrHeight( sal_Bool bWidth, SCCOLROW nRangeCnt, SCCOLROW* pRanges,
                                    ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                    sal_Bool bRecord, sal_Bool bPaint, ScMarkData* pMarkData )
{
    if (nRangeCnt == 0)
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
        for ( SCCOLROW i=0; i<nRangeCnt && bAllowed; i++ )
        {
            bool bOnlyMatrix;
            if (bWidth)
                bAllowed = pDoc->IsBlockEditable( *itr,
                        static_cast<SCCOL>(pRanges[2*i]),0,
                        static_cast<SCCOL>(pRanges[2*i+1]),MAXROW,
                        &bOnlyMatrix ) || bOnlyMatrix;
            else
                bAllowed = pDoc->IsBlockEditable( *itr, 0,pRanges[2*i],
                        MAXCOL,pRanges[2*i+1], &bOnlyMatrix ) ||
                    bOnlyMatrix;
        }

    // Allow users to resize cols/rows in readonly docs despite the r/o state.
    // It is frustrating to be unable to see content in mis-sized cells.
    if( !bAllowed && !pDocSh->IsReadOnly() )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    SCCOLROW nStart = pRanges[0];
    SCCOLROW nEnd = pRanges[2*nRangeCnt-1];

    sal_Bool bFormula = false;
    if ( eMode == SC_SIZE_OPTIMAL )
    {
        const ScViewOptions& rOpts = GetViewData()->GetOptions();
        bFormula = rOpts.GetOption( VOPT_FORMULAS );
    }

    ScDocument*     pUndoDoc = NULL;
    ScOutlineTable* pUndoTab = NULL;
    SCCOLROW*       pUndoRanges = NULL;

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

        pUndoRanges = new SCCOLROW[ 2*nRangeCnt ];
        memmove( pUndoRanges, pRanges, 2*nRangeCnt*sizeof(SCCOLROW) );

        //! outlines from all tab?
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nCurTab );
        if (pTable)
            pUndoTab = new ScOutlineTable( *pTable );
    }

    if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
        pMarkData->MarkToMulti();

    sal_Bool bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    sal_Bool bOutline = false;

    itr = pMarkData->begin();
    for (; itr != itrEnd; ++itr)
    {
        nTab = *itr;
        const SCCOLROW* pTabRanges = pRanges;

        pDoc->InitializeNoteCaptions( nTab );
        for (SCCOLROW nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
        {
            SCCOLROW nStartNo = *(pTabRanges++);
            SCCOLROW nEndNo = *(pTabRanges++);

            if ( !bWidth )                      // height always blockwise
            {
                if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                {
                    sal_Bool bAll = ( eMode==SC_SIZE_OPTIMAL );
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

                    pDoc->SetOptimalHeight( nStartNo, nEndNo, nTab, nSizeTwips, aProv.GetDevice(),
                                                nPPTX, nPPTY, aZoomX, aZoomY, bAll );
                    if (bAll)
                        pDoc->ShowRows( nStartNo, nEndNo, nTab, sal_True );

                    //  Manual-Flag already (re)set in SetOptimalHeight in case of bAll=sal_True
                    //  (set for Extra-Height, else reset).
                }
                else if ( eMode==SC_SIZE_DIRECT )
                {
                    if (nSizeTwips)
                    {
                        pDoc->SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
                        pDoc->SetManualHeight( nStartNo, nEndNo, nTab, sal_True );          // height was set manually
                    }
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );
                }
                else if ( eMode==SC_SIZE_SHOW )
                {
                    pDoc->ShowRows( nStartNo, nEndNo, nTab, sal_True );
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
                    bOutline = sal_True;
            }
            else
            {
                if ( pDoc->UpdateOutlineRow( nStartNo, nEndNo, nTab, bShow ) )
                    bOutline = sal_True;
            }
        }
        pDoc->SetDrawPageSize(nTab);
    }


    if (!bOutline)
        DELETEZ(pUndoTab);

    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoWidthOrHeight( pDocSh, *pMarkData,
                                     nStart, nCurTab, nEnd, nCurTab,
                                     pUndoDoc, nRangeCnt, pUndoRanges,
                                     pUndoTab, eMode, nSizeTwips, bWidth ) );
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

    // #i97876# Spreadsheet data changes are not notified
    if ( bWidth )
    {
        ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
        if ( pModelObj && pModelObj->HasChangesListeners() )
        {
            ScRangeList aChangeRanges;
            itr = pMarkData->begin();
            for (; itr != itrEnd; ++itr)
            {
                nTab = *itr;
                const SCCOLROW* pTabRanges = pRanges;
                for ( SCCOLROW nRange = 0; nRange < nRangeCnt; ++nRange )
                {
                    SCCOL nStartCol = static_cast< SCCOL >( *(pTabRanges++) );
                    SCCOL nEndCol = static_cast< SCCOL >( *(pTabRanges++) );
                    for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
                    {
                        aChangeRanges.Append( ScRange( nCol, 0, nTab ) );
                    }
                }
            }
            pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "column-resize" ) ), aChangeRanges );
        }
    }
}

//  column width/row height (via marked range)

void ScViewFunc::SetMarkedWidthOrHeight( sal_Bool bWidth, ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                        sal_Bool bRecord, sal_Bool bPaint )
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
        rMark.SetMultiMarkArea( ScRange( nCol,nRow,nTab ), sal_True );
        MarkDataChanged();
    }

    SCCOLROW* pRanges = new SCCOLROW[MAXCOLROWCOUNT];
    SCCOLROW nRangeCnt = 0;

    if ( bWidth )
        nRangeCnt = rMark.GetMarkColumnRanges( pRanges );
    else
        nRangeCnt = rMark.GetMarkRowRanges( pRanges );

    SetWidthOrHeight( bWidth, nRangeCnt, pRanges, eMode, nSizeTwips, bRecord, bPaint );

    delete[] pRanges;
    rMark.MarkToSimple();
}

void ScViewFunc::ModifyCellSize( ScDirection eDir, sal_Bool bOptimal )
{
    //! step size adjustable
    //  step size is also minumum
    sal_uInt16 nStepX = STD_COL_WIDTH / 5;
    sal_uInt16 nStepY = ScGlobal::nStdRowHeight;

    ScModule* pScMod = SC_MOD();
    sal_Bool bAnyEdit = pScMod->IsInputMode();
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
    SCCOLROW nRange[2];
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
                                            nPPTX, nPPTY, aZoomX, aZoomY, sal_True );
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
        nRange[0] = nRange[1] = nCol;
        SetWidthOrHeight( sal_True, 1, nRange, SC_SIZE_DIRECT, nWidth );

        //  adjust height of this row if width demands/allows this

        if (!bAnyEdit)
        {
            const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            sal_Bool bNeedHeight =
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
        nRange[0] = nRange[1] = nRow;
        SetWidthOrHeight( false, 1, nRange, eMode, nHeight );
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
        String aUndo = ScGlobal::GetRscString( STR_UNDO_PROTECT_TAB );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
    }

    ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd; ++itr)
        rFunc.ProtectSheet(*itr, rProtect);

    if (bUndo)
        pDocSh->GetUndoManager()->LeaveListAction();

    UpdateLayerLocks();         //! broadcast to all views
}

void ScViewFunc::Protect( SCTAB nTab, const String& rPassword )
{
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    sal_Bool bUndo(pDoc->IsUndoEnabled());

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
        rFunc.Protect( nTab, rPassword, false );
    else
    {
        //  modifying several tabs is handled here

        if (bUndo)
        {
            String aUndo = ScGlobal::GetRscString( STR_UNDO_PROTECT_TAB );
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

sal_Bool ScViewFunc::Unprotect( SCTAB nTab, const String& rPassword )
{
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScDocFunc &rFunc = pDocSh->GetDocFunc();
    sal_Bool bChanged = false;
    sal_Bool bUndo (pDoc->IsUndoEnabled());

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
        bChanged = rFunc.Unprotect( nTab, rPassword, false );
    else
    {
        //  modifying several tabs is handled here

        if (bUndo)
        {
            String aUndo = ScGlobal::GetRscString( STR_UNDO_UNPROTECT_TAB );
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
        }

        ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd; ++itr)
            if ( rFunc.Unprotect( *itr, rPassword, false ) )
                    bChanged = sal_True;

        if (bUndo)
            pDocSh->GetUndoManager()->LeaveListAction();
    }

    if (bChanged)
        UpdateLayerLocks();     //! broadcast to all views

    return bChanged;
}

void ScViewFunc::SetNoteText( const ScAddress& rPos, const String& rNoteText )
{
    GetViewData()->GetDocShell()->GetDocFunc().SetNoteText( rPos, rNoteText, false );
}

void ScViewFunc::ReplaceNote( const ScAddress& rPos, const String& rNoteText, const String* pAuthor, const String* pDate )
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
    ApplySelectionPattern( aNewAttrs, sal_True );
}

void ScViewFunc::SetNumFmtByStr( const String& rCode )
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

    sal_Bool bOk = sal_True;
    sal_uInt32 nNumberFormat = pFormatter->GetEntryKey( rCode, eLanguage );
    if ( nNumberFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        //  enter new

        String      aFormat = rCode;    // will be changed
        xub_StrLen  nErrPos = 0;
        short       nType   = 0;        //! ???
        bOk = pFormatter->PutEntry( aFormat, nErrPos, nType, nNumberFormat, eLanguage );
    }

    if ( bOk )          // valid format?
    {
        ScPatternAttr aNewAttrs( pDoc->GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumberFormat ) );
        rSet.Put( SvxLanguageItem( eLanguage, ATTR_LANGUAGE_FORMAT ) );
        ApplySelectionPattern( aNewAttrs, sal_True );
    }

    //! else return error / issue warning ???
}

void ScViewFunc::ChangeNumFmtDecimals( sal_Bool bIncrement )
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
    sal_Bool bError = false;

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
        bError = sal_True;
    }

    //! SvNumberformat has a Member bStandard, but doesn't disclose it
    sal_Bool bWasStandard = ( nOldFormat == pFormatter->GetStandardIndex( eLanguage ) );
    if (bWasStandard)
    {
        //  with "Standard" the decimal places depend on cell content
        //  0 if empty or text -> no decimal places
        double nVal = pDoc->GetValue( ScAddress( nCol, nRow, nTab ) );

        //  the ways of the Numberformatters are unfathomable, so try:
        String aOut;
        Color* pCol;
        ((SvNumberformat*)pOldEntry)->GetOutputString( nVal, aOut, &pCol );

        nPrecision = 0;
        // 'E' for exponential is fixed in Numberformatter
        if ( aOut.Search('E') != STRING_NOTFOUND )
            bError = sal_True;                              // exponential not changed
        else
        {
            String aDecSep( pFormatter->GetFormatDecimalSep( nOldFormat ) );
            xub_StrLen nPos = aOut.Search( aDecSep );
            if ( nPos != STRING_NOTFOUND )
                nPrecision = aOut.Len() - nPos - aDecSep.Len();
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
                bError = sal_True;          // 20 is maximum
        }
        else
        {
            if (nPrecision)
                --nPrecision;           // decrement
            else
                bError = sal_True;          // 0 is minumum
        }
    }

    if (!bError)
    {
        String aNewPicture = pFormatter->GenerateFormat(nOldFormat, eLanguage,
                                    bThousand, bNegRed, nPrecision, nLeading);

        nNewFormat = pFormatter->GetEntryKey( aNewPicture, eLanguage );
        if ( nNewFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            xub_StrLen nErrPos = 0;
            short nNewType = 0;
            sal_Bool bOk = pFormatter->PutEntry( aNewPicture, nErrPos,
                                                nNewType, nNewFormat, eLanguage );
            OSL_ENSURE( bOk, "incorrect numberformat generated" );
            if (!bOk)
                bError = sal_True;
        }
    }

    if (!bError)
    {
        ScPatternAttr aNewAttrs( pDoc->GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
        //  ATTR_LANGUAGE_FORMAT not
        ApplySelectionPattern( aNewAttrs, sal_True );
    }
}

void ScViewFunc::ChangeIndent( sal_Bool bIncrement )
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

    sal_Bool bSuccess = pDocSh->GetDocFunc().ChangeIndent( aWorkMark, bIncrement, false );
    if (bSuccess)
    {
        pDocSh->UpdateOle(pViewData);
        StartFormatArea();
    }
}

sal_Bool ScViewFunc::InsertName( const String& rName, const String& rSymbol,
                                const String& rType )
{
    //  Type = P,R,C,F (and combinations)
    //! undo...

    sal_Bool bOk = false;
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScRangeName* pList = pDoc->GetRangeName();

    RangeType nType = RT_NAME;
    ScRangeData* pNewEntry = new ScRangeData( pDoc, rName, rSymbol,
            ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                nTab), nType );
    String aUpType = rType;
    aUpType.ToUpperAscii();
    if ( aUpType.Search( 'P' ) != STRING_NOTFOUND )
        nType |= RT_PRINTAREA;
    if ( aUpType.Search( 'R' ) != STRING_NOTFOUND )
        nType |= RT_ROWHEADER;
    if ( aUpType.Search( 'C' ) != STRING_NOTFOUND )
        nType |= RT_COLHEADER;
    if ( aUpType.Search( 'F' ) != STRING_NOTFOUND )
        nType |= RT_CRITERIA;
    pNewEntry->AddType(nType);

    if ( !pNewEntry->GetErrCode() )     //  text valid?
    {
        ScDocShellModificator aModificator( *pDocSh );

        pDoc->CompileNameFormula( sal_True );   // CreateFormulaString

        // input available yet? Then remove beforehand (=change)
        ScRangeData* pData = pList->findByUpperName(ScGlobal::pCharClass->uppercase(rName));
        if (pData)
        {                                   // take old Index
            pNewEntry->SetIndex(pData->GetIndex());
            pList->erase(*pData);
        }

        if ( pList->insert( pNewEntry ) )
            bOk = sal_True;
        pNewEntry = NULL;   // never delete, insert took ownership

        pDoc->CompileNameFormula( false );  // CompileFormulaString
        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
    }

    delete pNewEntry;       // if it wasn't inserted
    return bOk;
}

void ScViewFunc::CreateNames( sal_uInt16 nFlags )
{
    sal_Bool bDone = false;
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
        sal_Bool bOk;
        SCCOL i;
        SCROW j;

        bOk = sal_True;
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
            bOk = sal_True;
            for (i=nFirstCol; i<=nLastCol && bOk; i++)
                if (!pDoc->HasStringData( i,nEndRow,nTab ))
                    bOk = false;
            if (bOk)
                nFlags |= NAME_BOTTOM;
        }

        bOk = sal_True;
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
            bOk = sal_True;
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




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
