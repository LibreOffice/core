/*************************************************************************
 *
 *  $RCSfile: viewfunc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-24 18:07:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#define _MACRODLG_HXX
#define _BIGINT_HXX
#define _SVDXOUT_HXX
#define _SVDATTR_HXX
#define _SVDSURO_HXX

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/editobj.hxx>
#include <svx/editview.hxx>
#include <svx/eeitem.hxx>
#include <svx/langitem.hxx>
#include <svx/scripttypeitem.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
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
#include "dbcolect.hxx"
#include "olinetab.hxx"
#include "rangeutl.hxx"
#include "rangenam.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "stlsheet.hxx"
#include "editutil.hxx"
#include "namecrea.hxx"         // wegen Flags
#include "cell.hxx"
#include "scresid.hxx"
#include "inputhdl.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"
#include "compiler.hxx"
#include "docfunc.hxx"
#include "appoptio.hxx"
#include "dociter.hxx"

// STATIC DATA -----------------------------------------------------------


//==================================================================

ScViewFunc::ScViewFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    ScTabView( pParent, rDocSh, pViewShell ),
    bFormatValid( FALSE )
{
}

ScViewFunc::ScViewFunc( Window* pParent, const ScViewFunc& rViewFunc, ScTabViewShell* pViewShell ) :
    ScTabView( pParent, rViewFunc, pViewShell ),
    bFormatValid( FALSE )
{
}

ScViewFunc::~ScViewFunc()
{
}

//------------------------------------------------------------------------------------

void ScViewFunc::StartFormatArea()
{
    //  ueberhaupt aktiviert?
    if ( !SC_MOD()->GetInputOptions().GetExtendFormat() )
        return;

    USHORT nTab = GetViewData()->GetTabNo();
    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab );
    BOOL bOk = TRUE;

    ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( rMark.IsMultiMarked() )
        rMark.MarkToSimple();
    if ( rMark.IsMultiMarked() )
        bOk = FALSE;
    else if ( rMark.IsMarked() )
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        if ( aMarkRange.aStart == aMarkRange.aEnd )
            aPos = aMarkRange.aStart;
        else
            bOk = FALSE;
    }

    if (bOk)
    {
        bFormatValid = TRUE;
        aFormatSource = aPos;
        aFormatArea = ScRange( aPos );
    }
    else
        bFormatValid = FALSE;       // keinen alten Bereich behalten
}

BOOL ScViewFunc::TestFormatArea( USHORT nCol, USHORT nRow, USHORT nTab, BOOL bAttrChanged )
{
    //  ueberhaupt aktiviert?
    if ( !SC_MOD()->GetInputOptions().GetExtendFormat() )
        return FALSE;

    //  Test: Eingabe mit Zahlformat (bAttrChanged) immer als neue Attributierung behandeln
    //  (alte Area verwerfen). Wenn das nicht gewollt ist, den if-Teil weglassen:
    if ( bAttrChanged )
    {
        StartFormatArea();
        return FALSE;
    }

    //! Abfrage, ob Zelle leer war ???

    BOOL bFound = FALSE;
    ScRange aNewRange = aFormatArea;
    if ( bFormatValid && nTab == aFormatSource.Tab() )
    {
        if ( nRow >= aFormatArea.aStart.Row() && nRow <= aFormatArea.aEnd.Row() )
        {
            //  innerhalb ?
            if ( nCol >= aFormatArea.aStart.Col() && nCol <= aFormatArea.aEnd.Col() )
            {
                bFound = TRUE;          // Bereich nicht aendern
            }
            //  links ?
            if ( nCol+1 == aFormatArea.aStart.Col() )
            {
                bFound = TRUE;
                aNewRange.aStart.SetCol( nCol );
            }
            //  rechts ?
            if ( nCol == aFormatArea.aEnd.Col()+1 )
            {
                bFound = TRUE;
                aNewRange.aEnd.SetCol( nCol );
            }
        }
        if ( nCol >= aFormatArea.aStart.Col() && nCol <= aFormatArea.aEnd.Col() )
        {
            //  oben ?
            if ( nRow+1 == aFormatArea.aStart.Row() )
            {
                bFound = TRUE;
                aNewRange.aStart.SetRow( nRow );
            }
            //  unten ?
            if ( nRow == aFormatArea.aEnd.Row()+1 )
            {
                bFound = TRUE;
                aNewRange.aEnd.SetRow( nRow );
            }
        }
    }

    if (bFound)
        aFormatArea = aNewRange;    // erweitern
    else
    {
        bFormatValid = FALSE;       // ausserhalb -> abbrechen
        if ( bAttrChanged )         // Wert mit Zahlformat eingegeben?
            StartFormatArea();      // dann ggf. neu starten
    }

    return bFound;
}

void ScViewFunc::DoAutoAttributes( USHORT nCol, USHORT nRow, USHORT nTab,
                                    BOOL bAttrChanged, BOOL bAddUndo )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

    const ScPatternAttr* pSource = pDoc->GetPattern(
                            aFormatSource.Col(), aFormatSource.Row(), nTab );
    if ( !((const ScMergeAttr&)pSource->GetItem(ATTR_MERGE)).IsMerged() )
    {
        const ScPatternAttr* pDocOld = pDoc->GetPattern( nCol, nRow, nTab );
        //  pDocOld ist nur bis zum Apply... gueltig!

        ScPatternAttr* pOldPattern = NULL;
        if ( bAddUndo )
            pOldPattern = new ScPatternAttr( *pDocOld );

        const ScStyleSheet* pSrcStyle = pSource->GetStyleSheet();
        if ( pSrcStyle && pSrcStyle != pDocOld->GetStyleSheet() )
            pDoc->ApplyStyle( nCol, nRow, nTab, *pSrcStyle );
        pDoc->ApplyPattern( nCol, nRow, nTab, *pSource );
        AdjustRowHeight( nRow, nRow, TRUE );                //! nicht doppelt ?

        if ( bAddUndo )
        {
            const ScPatternAttr* pNewPattern = pDoc->GetPattern( nCol, nRow, nTab );

            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoCursorAttr( pDocSh, nCol, nRow, nTab,
                                              pOldPattern, pNewPattern, pSource,
                                              TRUE ) );

            delete pOldPattern;     // wird im Undo kopiert (Pool)
        }
    }

    if ( bAttrChanged )                             // Wert mit Zahlformat eingegeben?
        aFormatSource.Set( nCol, nRow, nTab );      // dann als neue Quelle
}

//------------------------------------------------------------------------------------

//      Hilfsroutinen

USHORT ScViewFunc::GetOptimalColWidth( USHORT nCol, USHORT nTab, BOOL bFormula )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    VirtualDevice aVirtDev;
    aVirtDev.SetMapMode(MAP_PIXEL);
    USHORT nTwips = pDoc->GetOptimalColWidth( nCol, nTab, &aVirtDev,
                                        GetViewData()->GetPPTX(),
                                        GetViewData()->GetPPTY(),
                                        GetViewData()->GetZoomX(),
                                        GetViewData()->GetZoomY(),
                                        bFormula, &rMark );
    return nTwips;
}

BOOL ScViewFunc::SelectionEditable( BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ )
{
    BOOL bRet;
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    if (rMark.IsMarked() || rMark.IsMultiMarked())
        bRet = pDoc->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix );
    else
    {
        USHORT nCol = GetViewData()->GetCurX();
        USHORT nRow = GetViewData()->GetCurY();
        USHORT nTab = GetViewData()->GetTabNo();
        bRet = pDoc->IsBlockEditable( nTab, nCol, nRow, nCol, nRow,
            pOnlyNotBecauseOfMatrix );
    }
    return bRet;
}

#ifndef LRU_MAX
#define LRU_MAX 10
#endif

BOOL lcl_FunctionKnown( USHORT nOpCode )
{
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        ULONG nCount = pFuncList->GetCount();
        for (ULONG i=0; i<nCount; i++)
            if ( pFuncList->GetFunction(i)->nFIndex == nOpCode )
                return TRUE;
    }
    return FALSE;
}

BOOL lcl_AddFunction( ScAppOptions& rAppOpt, USHORT nOpCode )
{
    USHORT nOldCount = rAppOpt.GetLRUFuncListCount();
    USHORT* pOldList = rAppOpt.GetLRUFuncList();
    USHORT nPos;
    for (nPos=0; nPos<nOldCount; nPos++)
        if (pOldList[nPos] == nOpCode)          // is the function already in the list?
        {
            if ( nPos == 0 )
                return FALSE;                   // already at the top -> no change

            //  count doesn't change, so the original array is modified

            for (USHORT nCopy=nPos; nCopy>0; nCopy--)
                pOldList[nCopy] = pOldList[nCopy-1];
            pOldList[0] = nOpCode;

            return TRUE;                        // list has changed
        }

    if ( !lcl_FunctionKnown( nOpCode ) )
        return FALSE;                           // not in function list -> no change

    USHORT nNewCount = Min( (USHORT)(nOldCount + 1), (USHORT)LRU_MAX );
    USHORT nNewList[LRU_MAX];
    nNewList[0] = nOpCode;
    for (nPos=1; nPos<nNewCount; nPos++)
        nNewList[nPos] = pOldList[nPos-1];
    rAppOpt.SetLRUFuncList( nNewList, nNewCount );

    return TRUE;                                // list has changed
}

//      eigentliche Funktionen

//  Eingabe - Undo OK

void ScViewFunc::EnterData( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString,
                            BOOL bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    SvNumberFormatter& rFormatter = *pDoc->GetFormatTable();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nSelCount = rMark.GetSelectCount();
    USHORT i;

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    if (pDoc->IsSelectedBlockEditable( nCol,nRow, nCol,nRow, rMark ))
    {
        BOOL bEditDeleted = FALSE;
        BYTE nOldScript = 0;

        ScBaseCell** ppOldCells = NULL;
        BOOL* pHasFormat        = NULL;
        ULONG* pOldFormats      = NULL;
        USHORT* pTabs           = NULL;
        USHORT nUndoPos = 0;
        if ( bRecord )
        {
            ppOldCells      = new ScBaseCell*[nSelCount];
            pHasFormat      = new BOOL[nSelCount];
            pOldFormats     = new ULONG[nSelCount];
            pTabs           = new USHORT[nSelCount];
            nUndoPos = 0;

            for (i=0; i<nTabCount; i++)
                if (rMark.GetTableSelect(i))
                {
                    pTabs[nUndoPos] = i;
                    ScBaseCell* pDocCell;
                    pDoc->GetCell( nCol, nRow, i, pDocCell );
                    if ( pDocCell )
                    {
                        ppOldCells[nUndoPos] = pDocCell->Clone(pDoc);
                        if ( pDocCell->GetCellType() == CELLTYPE_EDIT )
                            bEditDeleted = TRUE;

                        BYTE nDocScript = pDoc->GetScriptType( nCol, nRow, i, pDocCell );
                        if ( nOldScript == 0 )
                            nOldScript = nDocScript;
                        else if ( nDocScript != nOldScript )
                            bEditDeleted = TRUE;
                    }
                    else
                    {
                        ppOldCells[nUndoPos] = NULL;
                    }

                    const SfxPoolItem* pItem;
                    const ScPatternAttr* pPattern = pDoc->GetPattern(nCol, nRow, i);
                    if ( SFX_ITEM_SET == pPattern->GetItemSet().GetItemState(
                                            ATTR_VALUE_FORMAT,FALSE,&pItem) )
                    {
                        pHasFormat[nUndoPos] = TRUE;
                        pOldFormats[nUndoPos] = ((const SfxUInt32Item*)pItem)->GetValue();
                    }
                    else
                        pHasFormat[nUndoPos] = FALSE;

                    ++nUndoPos;
                }

            DBG_ASSERT( nUndoPos==nSelCount, "nUndoPos!=nSelCount" );
        }

        BOOL bNumFmtChanged = FALSE;
        // einzelnes '=' ist String (wird fuer Spezialfilter so gebraucht)
        if ( rString.GetChar(0) == '=' && rString.Len() > 1 )
        {   // Formel, compile mit AutoCorrection
            for (i=0; i<nTabCount; i++)
                if (rMark.GetTableSelect(i))
                    break;
            ScAddress aPos( nCol, nRow, i );
            ScCompiler aComp( pDoc, aPos );
//2do: AutoCorrection via CalcOptions abschaltbar machen
            aComp.SetAutoCorrection( TRUE );
            String aFormula( rString );
            ScTokenArray* pArr;
            BOOL bAgain;
            do
            {
                bAgain = FALSE;
                BOOL bAddEqual = FALSE;
                ScTokenArray* pArrFirst = pArr = aComp.CompileString( aFormula );
                BOOL bCorrected = aComp.IsCorrected();
                if ( bCorrected )
                {   // probieren, mit erster Parser-Korrektur neu zu parsen
                    pArr = aComp.CompileString( aComp.GetCorrectedFormula() );
                }
                if ( !pArr->GetError() )
                {
                    bAddEqual = TRUE;
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
                        nResult = RET_NO;   // leere Formel, nur '='
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
                        bAgain = TRUE;
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
            // um in mehreren Tabellen eingesetzt zu werden, muss die Formel
            // via ScFormulaCell copy-ctor evtl. wegen RangeNames neu kompiliert
            // werden, gleiches Code-Array fuer alle Zellen geht nicht.
            // Wenn das Array einen Fehler enthaelt, muss in den neu erzeugten
            // Zellen RPN geloescht und der Fehler explizit gesetzt werden, da
            // via FormulaCell copy-ctor und Interpreter das, wenn moeglich,
            // wieder glattgebuegelt wird, zu intelligent.. z.B.: =1))
            USHORT nError = pArr->GetError();
            if ( !nError )
            {
                //  #68693# update list of recent functions with all functions that
                //  are not within parentheses

                ScModule* pScMod = SC_MOD();
                ScAppOptions aAppOpt = pScMod->GetAppOptions();
                BOOL bOptChanged = FALSE;

                ScToken** ppToken = pArr->GetArray();
                USHORT nTokens = pArr->GetLen();
                USHORT nLevel = 0;
                for (USHORT nTP=0; nTP<nTokens; nTP++)
                {
                    ScToken* pTok = ppToken[nTP];
                    OpCode eOp = pTok->GetOpCode();
                    if ( eOp == ocOpen )
                        ++nLevel;
                    else if ( eOp == ocClose && nLevel )
                        --nLevel;
                    if ( nLevel == 0 && pTok->IsFunction() && lcl_AddFunction( aAppOpt, eOp ) )
                        bOptChanged = TRUE;
                }

                if ( bOptChanged )
                {
                    pScMod->SetAppOptions(aAppOpt);
                    pScMod->RecentFunctionsChanged();
                }
            }

            ScFormulaCell aCell( pDoc, aPos, pArr, 0 );
            delete pArr;
            BOOL bAutoCalc = pDoc->GetAutoCalc();
            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
            for ( ; i<nTabCount; i++)
            {
                if (rMark.GetTableSelect(i))
                {
                    aPos.SetTab( i );
                    ULONG nIndex = (ULONG) ((SfxUInt32Item*) pDoc->GetAttr(
                        nCol, nRow, i, ATTR_VALUE_FORMAT ))->GetValue();
                    if ( pFormatter->GetType(nIndex) == NUMBERFORMAT_TEXT )
                    {
                        ScStringCell* pCell = new ScStringCell( aFormula );
                        pDoc->PutCell( aPos, pCell );
                    }
                    else
                    {
                        ScFormulaCell* pCell = new ScFormulaCell( pDoc, aPos, aCell );
                        if ( nError )
                        {
                            pCell->GetCode()->DelRPN();
                            pCell->SetErrCode( nError );
                        }
                        pDoc->PutCell( aPos, pCell );
                        if ( !bAutoCalc )
                        {   // einmal nur die Zelle berechnen und wieder dirty setzen
                            pCell->Interpret();
                            pCell->SetDirtyVar();
                            pDoc->PutInFormulaTree( pCell );
                        }
                    }

                }
            }
        }
        else
        {
            for (i=0; i<nTabCount; i++)
                if (rMark.GetTableSelect(i))
                    if (pDoc->SetString( nCol, nRow, i, rString ))
                        bNumFmtChanged = TRUE;
        }

        //  row height must be changed if new text has a different script type
        for (i=0; i<nTabCount && !bEditDeleted; i++)
            if (rMark.GetTableSelect(i))
                if ( pDoc->GetScriptType( nCol, nRow, i ) != nOldScript )
                    bEditDeleted = TRUE;

        HideAllCursors();

        if (bEditDeleted || pDoc->HasAttrib( nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_NEEDHEIGHT ))
            AdjustRowHeight(nRow,nRow);

        BOOL bAutoFormat = TestFormatArea(nCol, nRow, nTab, bNumFmtChanged);
        if (bAutoFormat)
            DoAutoAttributes(nCol, nRow, nTab, bNumFmtChanged, bRecord);

        if ( bRecord )
        {   // wg. ChangeTrack erst jetzt
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoEnterData( pDocSh, nCol, nRow, nTab, nUndoPos, pTabs,
                                     ppOldCells, pHasFormat, pOldFormats,
                                     rString, NULL ) );
        }

        for (i=0; i<nTabCount; i++)
            if (rMark.GetTableSelect(i))
                pDocSh->PostPaintCell( nCol, nRow, i );

        ShowAllCursors();

        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();
    }
    else
    {
        ErrorMessage(STR_PROTECTIONERR);
        PaintArea( nCol, nRow, nCol, nRow );        // da steht evtl. noch die Edit-Engine
    }
}

//  Wert in einzele Zelle eintragen (nur auf nTab)
//! umbenennen in EnterValue !!!!

void ScViewFunc::EnterData( USHORT nCol, USHORT nRow, USHORT nTab, const double& rValue )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();

    if ( pDoc && pDocSh )
    {
        ScDocShellModificator aModificator( *pDocSh );

        if (pDoc->IsBlockEditable( nTab, nCol,nRow, nCol,nRow ))
        {
            ScBaseCell* pOldCell;
            pDoc->GetCell( nCol, nRow, nTab, pOldCell );
            BOOL bNeedHeight = ( pOldCell && pOldCell->GetCellType() == CELLTYPE_EDIT )
                                || pDoc->HasAttrib(
                                    nCol,nRow,nTab, nCol,nRow,nTab, HASATTR_NEEDHEIGHT );

            //  Undo
            ScBaseCell* pUndoCell = pOldCell ? pOldCell->Clone(pDoc) : NULL;

            pDoc->SetValue( nCol, nRow, nTab, rValue );

            // wg. ChangeTrack nach Aenderung im Dokument
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoEnterValue( pDocSh, ScAddress(nCol,nRow,nTab),
                                        pUndoCell, rValue, bNeedHeight ) );

/*!             Zeilenhoehe anpassen? Dann auch bei Undo...
            if (bNeedHeight)
                AdjustRowHeight(nRow,nRow);
*/

            pDocSh->PostPaintCell( nCol, nRow, nTab );
            pDocSh->UpdateOle(GetViewData());
            aModificator.SetDocumentModified();
        }
        else
            ErrorMessage(STR_PROTECTIONERR);
    }
}

void ScViewFunc::EnterData( USHORT nCol, USHORT nRow, USHORT nTab, const EditTextObject* pData,
                            BOOL bRecord, BOOL bTestSimple )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocument* pDoc = pDocSh->GetDocument();

    ScDocShellModificator aModificator( *pDocSh );

    if (pDoc->IsBlockEditable( nTab, nCol,nRow, nCol,nRow ))
    {
        //
        //      Test auf Attribute
        //
        BOOL bSimple = FALSE;
        BOOL bCommon = FALSE;
        ScPatternAttr* pCellAttrs = NULL;
        EditTextObject* pNewData = NULL;
        String aString;
        if (bTestSimple)                    // Testen, ob einfacher String ohne Attribute
        {
            const ScPatternAttr* pOldPattern = pDoc->GetPattern( nCol, nRow, nTab );
            ScTabEditEngine aEngine( *pOldPattern, pDoc->GetEnginePool() );
            aEngine.SetText(*pData);

            ScEditAttrTester aTester( &aEngine );
            bSimple = !aTester.NeedsObject();
            bCommon = aTester.NeedsCellAttr();

            // formulas have to be recognized even if they're formatted
            // (but commmon attributes are still collected)

            if ( !bSimple && aEngine.GetParagraphCount() == 1 )
            {
                String aParStr = aEngine.GetText( (USHORT) 0 );
                if ( aParStr.GetChar(0) == '=' )
                    bSimple = TRUE;
            }

            if (bCommon)                // Attribute fuer Tabelle
            {
                pCellAttrs = new ScPatternAttr( *pOldPattern );
                pCellAttrs->GetFromEditItemSet( &aTester.GetAttribs() );
                //! remove common attributes from EditEngine?
            }

            if (bSimple)
                aString = aEngine.GetText();
        }

        //
        //      Undo
        //

        USHORT nTabCount = pDoc->GetTableCount();
        USHORT nSelCount = rMark.GetSelectCount();
        USHORT i;
        ScBaseCell** ppOldCells = NULL;
        USHORT* pTabs           = NULL;
        USHORT nPos = 0;
        EditTextObject* pUndoData = NULL;
        if (bRecord && !bSimple)
        {
            ppOldCells  = new ScBaseCell*[nSelCount];
            pTabs       = new USHORT[nSelCount];
            nPos = 0;

            for (i=0; i<nTabCount; i++)
                if (rMark.GetTableSelect(i))
                {
                    pTabs[nPos] = i;
                    ScBaseCell* pDocCell;
                    pDoc->GetCell( nCol, nRow, i, pDocCell );
                    if ( pDocCell )
                        ppOldCells[nPos] = pDocCell->Clone( pDoc );
                    else
                        ppOldCells[nPos] = NULL;
                    ++nPos;
                }

            DBG_ASSERT( nPos==nSelCount, "nPos!=nSelCount" );

            pUndoData = pData->Clone();
        }

        //
        //      Daten eintragen
        //

        if (bCommon)
            pDoc->ApplyPattern(nCol,nRow,nTab,*pCellAttrs);         //! Undo

        if (bSimple)
        {
            if (bCommon)
                AdjustRowHeight(nRow,nRow);

            EnterData(nCol,nRow,nTab,aString,bRecord);
        }
        else
        {
            for (i=0; i<nTabCount; i++)
                if (rMark.GetTableSelect(i))
                    pDoc->PutCell( nCol, nRow, i, new ScEditCell( pData, pDoc, NULL ) );

            if ( bRecord )
            {   // wg. ChangeTrack erst jetzt
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoEnterData( pDocSh, nCol, nRow, nTab, nPos, pTabs,
                                        ppOldCells, NULL, NULL, String(),
                                        pUndoData ) );
            }

            HideAllCursors();

            AdjustRowHeight(nRow,nRow);

            for (i=0; i<nTabCount; i++)
                if (rMark.GetTableSelect(i))
                    pDocSh->PostPaintCell( nCol, nRow, i );

            ShowAllCursors();

            pDocSh->UpdateOle(GetViewData());
            aModificator.SetDocumentModified();
        }

        delete pCellAttrs;
        delete pNewData;
    }
    else
    {
        ErrorMessage(STR_PROTECTIONERR);
        PaintArea( nCol, nRow, nCol, nRow );        // da steht evtl. noch die Edit-Engine
    }
}

void ScViewFunc::EnterDataAtCursor( const String& rString )
{
    USHORT nPosX = GetViewData()->GetCurX();
    USHORT nPosY = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();

    EnterData( nPosX, nPosY, nTab, rString );
}

void ScViewFunc::EnterMatrix( const String& rString )
{
    ScViewData* pData = GetViewData();
    const ScMarkData& rMark = pData->GetMarkData();
    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        //  nichts markiert -> automatisch Block mit Groesse des Ergebnisses
        //  Formel temporaer berechnen, um an die Groesse heranzukommen

        ScDocument* pDoc = pData->GetDocument();
        USHORT nCol = pData->GetCurX();
        USHORT nRow = pData->GetCurY();
        USHORT nTab = pData->GetTabNo();
        ScFormulaCell aFormCell( pDoc, ScAddress(nCol,nRow,nTab), rString, MM_FORMULA );

        USHORT nSizeX, nSizeY;
        aFormCell.GetResultDimensions( nSizeX, nSizeY );
        if ( nSizeX && nSizeY && nCol+nSizeX-1 <= MAXCOL && nRow+nSizeY-1 <= MAXROW )
        {
            ScRange aResult( nCol, nRow, nTab, nCol+nSizeX-1, nRow+nSizeY-1, nTab );
            MarkRange( aResult, FALSE );
        }
    }

    ScRange aRange;
    if (pData->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = pData->GetDocShell();
        BOOL bSuccess = pDocSh->GetDocFunc().EnterMatrix( aRange, &rMark, rString, FALSE );
        if (bSuccess)
            pDocSh->UpdateOle(GetViewData());
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

BYTE ScViewFunc::GetSelectionScriptType()
{
    BYTE nScript = 0;

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
        rMark.FillRangeListWithMarks( &aRanges, FALSE );
        ULONG nCount = aRanges.Count();
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange = *aRanges.GetObject(i);
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
        nScript = SCRIPTTYPE_LATIN;

    return nScript;
}

const ScPatternAttr* ScViewFunc::GetSelectionPattern()
{
    const ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        //  MarkToMulti ist fuer pDoc->GetSelectionPattern nicht mehr noetig
        const ScPatternAttr* pAttr = pDoc->GetSelectionPattern( rMark );
        return pAttr;
    }
    else
    {
        USHORT  nCol = GetViewData()->GetCurX();
        USHORT  nRow = GetViewData()->GetCurY();
        USHORT  nTab = GetViewData()->GetTabNo();

        ScMarkData aTempMark( rMark );      // Tabellen kopieren
        aTempMark.SetMarkArea( ScRange( nCol, nRow, nTab ) );
        const ScPatternAttr* pAttr = pDoc->GetSelectionPattern( aTempMark );
        return pAttr;
    }
}

void ScViewFunc::GetSelectionFrame( SvxBoxItem&     rLineOuter,
                                    SvxBoxInfoItem& rLineInner )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();

    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
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
        rLineInner.SetTable(FALSE);
        rLineInner.SetDist((BOOL)FALSE);
        rLineInner.SetMinDist(FALSE);
    }
}

//
//  Attribute anwenden - Undo OK
//
//  kompletter Set ( ATTR_STARTINDEX, ATTR_ENDINDEX )
//

void ScViewFunc::ApplyAttributes( const SfxItemSet* pDialogSet,
                                  const SfxItemSet* pOldSet,
                                  BOOL              bRecord )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aOldAttrs( new SfxItemSet(*pOldSet) );
    ScPatternAttr aNewAttrs( new SfxItemSet(*pDialogSet) );
    aNewAttrs.DeleteUnchanged( &aOldAttrs );

    ULONG nOldFormat =
        ((const SfxUInt32Item&)pOldSet->Get( ATTR_VALUE_FORMAT )).GetValue();
    ULONG nNewFormat =
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

            //  #40606# nur die Sprache geaendert -> Zahlformat-Attribut nicht anfassen
            ULONG nNewMod = nNewFormat % SV_COUNTRY_LANGUAGE_OFFSET;
            if ( nNewMod == ( nOldFormat % SV_COUNTRY_LANGUAGE_OFFSET ) &&
                 nNewMod <= SV_MAX_ANZ_STANDARD_FORMATE )
                aNewAttrs.GetItemSet().ClearItem( ATTR_VALUE_FORMAT );
        }
    }

    const SvxBoxItem*     pOldOuter = (const SvxBoxItem*)     &pOldSet->Get( ATTR_BORDER );
    const SvxBoxItem*     pNewOuter = (const SvxBoxItem*)     &pDialogSet->Get( ATTR_BORDER );
    const SvxBoxInfoItem* pOldInner = (const SvxBoxInfoItem*) &pOldSet->Get( ATTR_BORDER_INNER );
    const SvxBoxInfoItem* pNewInner = (const SvxBoxInfoItem*) &pDialogSet->Get( ATTR_BORDER_INNER );
    SfxItemSet&           rNewSet   = aNewAttrs.GetItemSet();
    SfxItemPool*          pNewPool  = rNewSet.GetPool();

    pNewPool->Put( *pNewOuter );        // noch nicht loeschen
    pNewPool->Put( *pNewInner );
    rNewSet.ClearItem( ATTR_BORDER );
    rNewSet.ClearItem( ATTR_BORDER_INNER );

    /*
     * Feststellen, ob Rahmenattribute zu setzen sind:
     * 1. Neu != Alt
     * 2. Ist eine der Linien nicht-DontCare (seit 238.f: IsxxValid())
     *
     */

    BOOL bFrame =    (pDialogSet->GetItemState( ATTR_BORDER ) != SFX_ITEM_DEFAULT)
                  || (pDialogSet->GetItemState( ATTR_BORDER_INNER ) != SFX_ITEM_DEFAULT);

    if ( pNewOuter==pOldOuter && pNewInner==pOldInner )
        bFrame = FALSE;

    //  das sollte doch der Pool abfangen: ?!??!??

    if ( bFrame && pNewOuter && pNewInner )
        if ( *pNewOuter == *pOldOuter && *pNewInner == *pOldInner )
            bFrame = FALSE;

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
        bFrame = FALSE;

    if (!bFrame)
        ApplySelectionPattern( aNewAttrs, bRecord );                // nur normale
    else
    {
        // wenn neue Items Default-Items sind, so muessen die
        // alten Items geputtet werden:

        BOOL bDefNewOuter = ( SFX_ITEMS_STATICDEFAULT == pNewOuter->GetRef() );
        BOOL bDefNewInner = ( SFX_ITEMS_STATICDEFAULT == pNewInner->GetRef() );

        ApplyPatternLines( aNewAttrs,
                           bDefNewOuter ? pOldOuter : pNewOuter,
                           bDefNewInner ? pOldInner : pNewInner,
                           bRecord );
    }

    pNewPool->Remove( *pNewOuter );         // freigeben
    pNewPool->Remove( *pNewInner );

    //  Hoehen anpassen
    AdjustBlockHeight();

    // CellContentChanged wird von ApplySelectionPattern / ApplyPatternLines gerufen
}

void ScViewFunc::ApplyAttr( const SfxPoolItem& rAttrItem )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aNewAttrs( new SfxItemSet( *GetViewData()->GetDocument()->GetPool(),
                                            ATTR_PATTERN_START, ATTR_PATTERN_END ) );

    aNewAttrs.GetItemSet().Put( rAttrItem );
    //  Wenn Ausrichtung eingestellt wird (ueber Buttons), immer Einzug 0
    if ( rAttrItem.Which() == ATTR_HOR_JUSTIFY )
        aNewAttrs.GetItemSet().Put( SfxUInt16Item( ATTR_INDENT, 0 ) );
    ApplySelectionPattern( aNewAttrs );

    AdjustBlockHeight();

    // CellContentChanged wird von ApplySelectionPattern gerufen
}


//  Pattern und Rahmen

void ScViewFunc::ApplyPatternLines( const ScPatternAttr& rAttr, const SvxBoxItem* pNewOuter,
                                    const SvxBoxInfoItem* pNewInner, BOOL bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();

    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;

    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab,TRUE))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();

        ScDocShellModificator aModificator( *pDocSh );

        if (!rMark.IsMarked())
        {
            DoneBlockMode();
            InitOwnBlockMode();
            rMark.SetMarkArea( ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab ) );
        }

        if (bRecord)
        {
            USHORT nTabCount = pDoc->GetTableCount();
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
            for (USHORT i=0; i<nTabCount; i++)
                if (i != nStartTab && rMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );
            pDoc->CopyToDocument( nStartCol, nStartRow, 0, nEndCol, nEndRow, nTabCount-1,
                                    IDF_ATTRIB, FALSE, pUndoDoc );

            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSelectionAttr( pDocSh, rMark,
                                         nStartCol, nStartRow, nStartTab,
                                         nEndCol,   nEndRow,   nEndTab,
                                         pUndoDoc, FALSE, &rAttr, pNewOuter, pNewInner ) );
        }

        BOOL bOldLines = pDoc->HasAttrib( nStartCol, nStartRow, nStartTab,
                                          nEndCol,   nEndRow,   nEndTab,
                                          HASATTR_PAINTEXT );

        pDoc->ApplySelectionFrame( rMark, pNewOuter, pNewInner );

        BOOL bNewLines = pDoc->HasAttrib( nStartCol, nStartRow, nStartTab,
                                          nEndCol,   nEndRow,   nEndTab,
                                          HASATTR_PAINTEXT );

        rMark.MarkToMulti();
        pDoc->ApplySelectionPattern( rAttr, rMark );

        USHORT nExt = SC_PF_TESTMERGE;
        if (bOldLines || bNewLines) nExt |= SC_PF_LINES;
        pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                           nEndCol,   nEndRow,   nEndTab,
                           PAINT_GRID, nExt );
        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
        rMark.MarkToSimple();
    }
    else
    {       // "Rahmen nicht auf Mehrfachselektion"
        ErrorMessage(STR_MSSG_APPLYPATTLINES_0);
    }

    StartFormatArea();
}

//  nur Pattern

void ScViewFunc::ApplySelectionPattern( const ScPatternAttr& rAttr,
                                            BOOL bRecord, BOOL bCursorOnly )
{
    ScViewData* pViewData   = GetViewData();
    ScDocShell* pDocSh      = pViewData->GetDocShell();
    ScDocument* pDoc        = pDocSh->GetDocument();
    ScMarkData& rMark       = pViewData->GetMarkData();

    ScDocShellModificator aModificator( *pDocSh );

    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;

    BOOL bMulti = rMark.IsMultiMarked();
    rMark.MarkToMulti();
    BOOL bOnlyTab = (!rMark.IsMultiMarked() && !bCursorOnly && rMark.GetSelectCount() > 1);
    if (bOnlyTab)
    {
        USHORT nCol = pViewData->GetCurX();
        USHORT nRow = pViewData->GetCurY();
        USHORT nTab = pViewData->GetTabNo();
        ScAddress aCursor(nCol,nRow,nTab);
        rMark.SetMarkArea(ScRange(aCursor,aCursor));
        rMark.MarkToMulti();
    }

    if (rMark.IsMultiMarked() && !bCursorOnly)
    {
        if (bRecord)
        {
            ScRange aMarkRange;
            rMark.GetMultiMarkArea( aMarkRange );
            nStartCol = aMarkRange.aStart.Col();
            nStartRow = aMarkRange.aStart.Row();
            nStartTab = aMarkRange.aStart.Tab();
            nEndCol = aMarkRange.aEnd.Col();
            nEndRow = aMarkRange.aEnd.Row();
            nEndTab = aMarkRange.aEnd.Tab();

            ScRange aCopyRange = aMarkRange;
            USHORT nTabCount = pDoc->GetTableCount();
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);

            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nStartTab );
            for (USHORT i=0; i<nTabCount; i++)
                if (i != nStartTab && rMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, pUndoDoc, &rMark );

            rMark.MarkToMulti();

            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSelectionAttr(
                            pDocSh, rMark,
                            nStartCol, nStartRow, nStartTab,
                            nEndCol, nEndRow, nEndTab,
                            pUndoDoc, bMulti, &rAttr ) );
        }

        pDoc->ApplySelectionPattern( rAttr, rMark );

        if (bOnlyTab)
            rMark.ResetMark();
        else
            rMark.MarkToSimple();

        pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                           nEndCol,   nEndRow,   nEndTab,
                           PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );
        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
    }
    else                            // einzelne Zelle - Undo einfacher
    {
        USHORT nCol = pViewData->GetCurX();
        USHORT nRow = pViewData->GetCurY();
        USHORT nTab = pViewData->GetTabNo();
        ScPatternAttr* pOldPat = new ScPatternAttr(*pDoc->GetPattern( nCol, nRow, nTab ));

        const SfxItemSet& rOldSet = pOldPat->GetItemSet();
        BOOL bOldLines = rOldSet.GetItemState( ATTR_BORDER, TRUE ) == SFX_ITEM_SET ||
                         rOldSet.GetItemState( ATTR_SHADOW, TRUE ) == SFX_ITEM_SET;
        const SfxItemSet& rNewSet = rAttr.GetItemSet();
        BOOL bNewLines = rNewSet.GetItemState( ATTR_BORDER, TRUE ) == SFX_ITEM_SET ||
                         rNewSet.GetItemState( ATTR_SHADOW, TRUE ) == SFX_ITEM_SET;

        pDoc->ApplyPattern( nCol, nRow, nTab, rAttr );

        const ScPatternAttr* pNewPat = pDoc->GetPattern( nCol, nRow, nTab );

        if (bRecord)
        {
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoCursorAttr( pDocSh,
                                              nCol, nRow, nTab,
                                              pOldPat, pNewPat, &rAttr,
                                              FALSE ) );    // FALSE = nicht automatisch
        }
        delete pOldPat;     // wird im Undo kopiert (Pool)

        USHORT nFlags = 0;
        if ( bOldLines || bNewLines )
            nFlags |= SC_PF_LINES;

        pDocSh->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );
        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();
        CellContentChanged();
    }

    StartFormatArea();
}

void ScViewFunc::ApplyUserItemSet( const SfxItemSet& rItemSet )
{
    //  ItemSet from UI, may have different pool

    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScPatternAttr aNewAttrs( GetViewData()->GetDocument()->GetPool() );
    SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
    rNewSet.Put( rItemSet, FALSE );
    ApplySelectionPattern( aNewAttrs );

    AdjustBlockHeight();
}

const SfxStyleSheet* ScViewFunc::GetStyleSheetFromMarked()
{
    const ScStyleSheet* pSheet      = NULL;
    ScViewData*         pViewData   = GetViewData();
    ScDocument*         pDoc        = pViewData->GetDocument();
    ScMarkData&         rMark       = pViewData->GetMarkData();

    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
        pSheet = pDoc->GetSelectionStyle( rMark );                  // MarkToMulti nicht noetig !!
    else
        pSheet = pDoc->GetStyle( pViewData->GetCurX(),
                                 pViewData->GetCurY(),
                                 pViewData->GetTabNo() );

    return pSheet;
}

void ScViewFunc::SetStyleSheetToMarked( SfxStyleSheet* pStyleSheet, BOOL bRecord )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
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
    ScMarkData& rMark       = pViewData->GetMarkData();
    USHORT nTabCount        = pDoc->GetTableCount();

    ScDocShellModificator aModificator( *pDocSh );

    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        ScRange aMarkRange;
        rMark.MarkToMulti();
        rMark.GetMultiMarkArea( aMarkRange );

        if ( bRecord )
        {
            USHORT nTab = pViewData->GetTabNo();
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
            for (USHORT i=0; i<nTabCount; i++)
                if (i != nTab && rMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );

            ScRange aCopyRange = aMarkRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, TRUE, pUndoDoc, &rMark );
            rMark.MarkToMulti();

            String aName = pStyleSheet->GetName();
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSelectionStyle( pDocSh, rMark, aMarkRange, aName, pUndoDoc ) );
        }

        pDoc->ApplySelectionStyle( (ScStyleSheet&)*pStyleSheet, rMark );

        if (!AdjustBlockHeight())
            pViewData->GetDocShell()->PostPaint( aMarkRange, PAINT_GRID );

        rMark.MarkToSimple();
    }
    else
    {
        USHORT nCol = pViewData->GetCurX();
        USHORT nRow = pViewData->GetCurY();
        USHORT nTab = pViewData->GetTabNo();

        if ( bRecord )
        {
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab );
            for (USHORT i=0; i<nTabCount; i++)
                if (i != nTab && rMark.GetTableSelect(i))
                    pUndoDoc->AddUndoTab( i, i );

            ScRange aCopyRange( nCol, nRow, 0, nCol, nRow, nTabCount-1 );
            pDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, FALSE, pUndoDoc );

            ScRange aMarkRange ( nCol, nRow, nTab );
            ScMarkData aUndoMark = rMark;
            aUndoMark.SetMultiMarkArea( aMarkRange );

            String aName = pStyleSheet->GetName();
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSelectionStyle( pDocSh, aUndoMark, aMarkRange, aName, pUndoDoc ) );
        }

        for (USHORT i=0; i<nTabCount; i++)
            if (rMark.GetTableSelect(i))
                pDoc->ApplyStyle( nCol, nRow, i, (ScStyleSheet&)*pStyleSheet );

        if (!AdjustBlockHeight())
            pViewData->GetDocShell()->PostPaintCell( nCol, nRow, nTab );

    }

    aModificator.SetDocumentModified();

    StartFormatArea();
}


void ScViewFunc::RemoveStyleSheetInUse( SfxStyleSheet* pStyleSheet )
{
    if ( !pStyleSheet) return;
    // -------------------------------------------------------------------

    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScDocShell* pDocSh      = pViewData->GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    VirtualDevice aVirtDev;
    aVirtDev.SetMapMode(MAP_PIXEL);
    pDoc->StyleSheetChanged( pStyleSheet, TRUE, &aVirtDev,
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

void ScViewFunc::UpdateStyleSheetInUse( SfxStyleSheet* pStyleSheet )
{
    if ( !pStyleSheet) return;
    // -------------------------------------------------------------------

    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScDocShell* pDocSh      = pViewData->GetDocShell();

    ScDocShellModificator aModificator( *pDocSh );

    VirtualDevice aVirtDev;
    aVirtDev.SetMapMode(MAP_PIXEL);
    pDoc->StyleSheetChanged( pStyleSheet, FALSE, &aVirtDev,
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

//  Zellen einfuegen - Undo OK

BOOL ScViewFunc::InsertCells( InsCellCmd eCmd, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        BOOL bSuccess = pDocSh->GetDocFunc().InsertCells( aRange, eCmd, bRecord, FALSE );
        if (bSuccess)
        {
            pDocSh->UpdateOle(GetViewData());
            CellContentChanged();
        }
        return bSuccess;
    }
    else
    {
        ErrorMessage(STR_NOMULTISELECT);
        return FALSE;
    }
}

//  Zellen loeschen - Undo OK

void ScViewFunc::DeleteCells( DelCellCmd eCmd, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        pDocSh->GetDocFunc().DeleteCells( aRange, eCmd, bRecord, FALSE );

        pDocSh->UpdateOle(GetViewData());
        CellContentChanged();

        //  #58106# Cursor direkt hinter den geloeschten Bereich setzen
        USHORT nCurX = GetViewData()->GetCurX();
        USHORT nCurY = GetViewData()->GetCurY();
        if ( eCmd==DEL_CELLSLEFT || eCmd==DEL_DELCOLS )
            nCurX = aRange.aStart.Col();
        else
            nCurY = aRange.aStart.Row();
        SetCursor( nCurX, nCurY );
    }
    else
    {
        if (eCmd == DEL_DELCOLS)
            DeleteMulti( FALSE, bRecord );
        else if (eCmd == DEL_DELROWS)
            DeleteMulti( TRUE, bRecord );
        else
            ErrorMessage(STR_NOMULTISELECT);
    }

    Unmark();
}

void ScViewFunc::DeleteMulti( BOOL bRows, BOOL bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );
    USHORT nTab = GetViewData()->GetTabNo();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT* pRanges = new USHORT[MAXROW+1];
    USHORT nRangeCnt = bRows ? rMark.GetMarkRowRanges( pRanges ) :
                                rMark.GetMarkColumnRanges( pRanges );
    if (!nRangeCnt)
    {
        pRanges[0] = pRanges[1] = bRows ? GetViewData()->GetCurY() : GetViewData()->GetCurX();
        nRangeCnt = 1;
    }

    //  Test ob erlaubt

    USHORT* pOneRange = pRanges;
    BOOL bAllowed = TRUE;
    USHORT nRangeNo;
    for (nRangeNo=0; nRangeNo<nRangeCnt && bAllowed; nRangeNo++)
    {
        USHORT nStart = *(pOneRange++);
        USHORT nEnd = *(pOneRange++);

        //! ...
    }

    //  ausfuehren

    WaitObject aWait( GetFrameWin() );      // wichtig wegen TrackFormulas bei UpdateReference

    ScDocument* pUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab, !bRows, bRows );      // Zeilenhoehen

        pOneRange = pRanges;
        for (nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
        {
            USHORT nStart = *(pOneRange++);
            USHORT nEnd = *(pOneRange++);
            if (bRows)
                pDoc->CopyToDocument( 0,nStart,nTab, MAXCOL,nEnd,nTab, IDF_ALL,FALSE,pUndoDoc );
            else
                pDoc->CopyToDocument( nStart,0,nTab, nEnd,MAXROW,nTab, IDF_ALL,FALSE,pUndoDoc );
        }

                //  alle Formeln wegen Referenzen
        USHORT nTabCount = pDoc->GetTableCount();
        pUndoDoc->AddUndoTab( 0, nTabCount-1, FALSE, FALSE );
        pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA,FALSE,pUndoDoc );

        pUndoData = new ScRefUndoData( pDoc );

        pDoc->BeginDrawUndo();
    }

    pOneRange = &pRanges[2*nRangeCnt];      // rueckwaerts
    for (nRangeNo=0; nRangeNo<nRangeCnt && bAllowed; nRangeNo++)
    {
        USHORT nEnd = *(--pOneRange);
        USHORT nStart = *(--pOneRange);

        if (bRows)
            pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, nStart, nEnd-nStart+1 );
        else
            pDoc->DeleteCol( 0,nTab, MAXROW,nTab, nStart, nEnd-nStart+1 );
    }

    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoDeleteMulti( pDocSh, bRows, nTab, pRanges, nRangeCnt,
                                    pUndoDoc, pUndoData ) );
    }

    if (!AdjustRowHeight(0, MAXROW))
        if (bRows)
            pDocSh->PostPaint( 0,pRanges[0],nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_LEFT );
        else
            pDocSh->PostPaint( pRanges[0],0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_TOP );
    aModificator.SetDocumentModified();

    CellContentChanged();

    //  #58106# Cursor direkt hinter den ersten geloeschten Bereich setzen
    USHORT nCurX = GetViewData()->GetCurX();
    USHORT nCurY = GetViewData()->GetCurY();
    if ( bRows )
        nCurY = pRanges[0];
    else
        nCurX = pRanges[0];
    SetCursor( nCurX, nCurY );

    delete[] pRanges;
}

//  Inhalte loeschen

void ScViewFunc::DeleteContents( USHORT nFlags, BOOL bRecord )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    BOOL bEditable = SelectionEditable( &bOnlyNotBecauseOfMatrix );
    if ( !bEditable )
    {
        if ( !(bOnlyNotBecauseOfMatrix &&
                ((nFlags & (IDF_ATTRIB | IDF_EDITATTR)) == nFlags)) )
        {
            ErrorMessage(STR_PROTECTIONERR);
            return;
        }
    }

    ScRange aMarkRange;
    BOOL bSimple = FALSE;

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();

    ScDocShellModificator aModificator( *pDocSh );

    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        aMarkRange.aStart.SetCol(GetViewData()->GetCurX());
        aMarkRange.aStart.SetRow(GetViewData()->GetCurY());
        aMarkRange.aStart.SetTab(GetViewData()->GetTabNo());
        aMarkRange.aEnd = aMarkRange.aStart;
        if ( pDoc->HasAttrib( aMarkRange, HASATTR_MERGED ) )
        {
            InitOwnBlockMode();
            rMark.SetMarkArea( aMarkRange );
        }
        else
            bSimple = TRUE;
    }

    rMark.SetMarking(FALSE);        // fuer MarkToMulti

    DBG_ASSERT( rMark.IsMarked() || rMark.IsMultiMarked() || bSimple, "was denn loeschen ???" )

    ScDocument* pUndoDoc = NULL;
    BOOL bMulti = !bSimple && rMark.IsMultiMarked();
    if (!bSimple)
    {
        rMark.MarkToMulti();
        rMark.GetMultiMarkArea( aMarkRange );
    }
    ScRange aExtendedRange(aMarkRange);
    if (!bSimple)
    {
        if ( pDoc->ExtendMerge( aExtendedRange, TRUE ) )
            bMulti = FALSE;
    }

    // keine Objekte auf geschuetzten Tabellen
    BOOL bObjects = FALSE;
    if ( nFlags & IDF_OBJECTS )
    {
        bObjects = TRUE;
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            if (rMark.GetTableSelect(nTab) && pDoc->IsTabProtected(nTab))
                bObjects = FALSE;
    }

    USHORT nExtFlags = 0;                       // Linien interessieren nur, wenn Attribute
    if ( nFlags & IDF_ATTRIB )                  // geloescht werden
        if (pDoc->HasAttrib( aMarkRange, HASATTR_PAINTEXT ))
            nExtFlags |= SC_PF_LINES;

    //  Reihenfolge:
    //  1) BeginDrawUndo
    //  2) Objekte loeschen (DrawUndo wird gefuellt)
    //  3) Inhalte fuer Undo kopieren
    //  4) Inhalte loeschen
    //  5) Undo-Aktion anlegen

    if (bObjects)
    {
        if (bRecord)
            pDoc->BeginDrawUndo();

        if (bMulti)
            pDoc->DeleteObjectsInSelection( rMark );
        else
            pDoc->DeleteObjectsInArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
/*!*/                                  aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                                       rMark );
    }

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        USHORT nTab = aMarkRange.aStart.Tab();
        pUndoDoc->InitUndo( pDoc, nTab, nTab );
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT i=0; i<nTabCount; i++)
            if (i != nTab && rMark.GetTableSelect(i))
                pUndoDoc->AddUndoTab( i, i );
        ScRange aCopyRange = aExtendedRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);

        //  bei "Format/Standard" alle Attribute kopieren, weil CopyToDocument
        //  nur mit IDF_HARDATTR zu langsam ist:
        USHORT nUndoDocFlags = nFlags;
        if (nFlags & IDF_ATTRIB)
            nUndoDocFlags |= IDF_ATTRIB;
        if (nFlags & IDF_EDITATTR)          // Edit-Engine-Attribute
            nUndoDocFlags |= IDF_STRING;    // -> Zellen werden geaendert
        if (nFlags & IDF_NOTE)
            nUndoDocFlags |= IDF_CONTENTS;  // #68795# copy all cells with their notes
        pDoc->CopyToDocument( aCopyRange, nUndoDocFlags, bMulti, pUndoDoc, &rMark );
    }

    HideAllCursors();   // falls Zusammenfassung aufgehoben wird
    if (bSimple)
        pDoc->DeleteArea( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                          aMarkRange.aEnd.Col(),   aMarkRange.aEnd.Row(),
                          rMark, nFlags );
    else
    {
        pDoc->DeleteSelection( nFlags, rMark );
        rMark.MarkToSimple();
    }

    if ( bRecord )
    {
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoDeleteContents( pDocSh, rMark, aExtendedRange,
                                      pUndoDoc, bMulti, nFlags, bObjects ) );
    }

    if (!AdjustRowHeight( aExtendedRange.aStart.Row(), aExtendedRange.aEnd.Row() ))
        pDocSh->PostPaint( aExtendedRange, PAINT_GRID, nExtFlags );

    pDocSh->UpdateOle(GetViewData());
    aModificator.SetDocumentModified();
    CellContentChanged();
    ShowAllCursors();

    if ( nFlags & IDF_ATTRIB )
    {
        if ( nFlags & IDF_CONTENTS )
            ForgetFormatArea();
        else
            StartFormatArea();              // Attribute loeschen ist auch Attributierung
    }
}

//  Spaltenbreiten/Zeilenhoehen (ueber Header) - Undo OK

void ScViewFunc::SetWidthOrHeight( BOOL bWidth, USHORT nRangeCnt, USHORT* pRanges,
                                    ScSizeMode eMode, USHORT nSizeTwips,
                                    BOOL bRecord, BOOL bPaint, ScMarkData* pMarkData )
{
    if (!nRangeCnt)
        return;

    // use view's mark if none specified
    if ( !pMarkData )
        pMarkData = &GetViewData()->GetMarkData();

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nFirstTab = pMarkData->GetFirstSelected();
    USHORT nCurTab = GetViewData()->GetTabNo();
    USHORT nTab;

    ScDocShellModificator aModificator( *pDocSh );

    BOOL bAllowed = TRUE;
    for (nTab=0; nTab<nTabCount && bAllowed; nTab++)
        if (pMarkData->GetTableSelect(nTab))
        {
            for ( USHORT i=0; i<nRangeCnt && bAllowed; i++ )
            {
                BOOL bOnlyMatrix;
                if (bWidth)
                    bAllowed = pDoc->IsBlockEditable( nTab,
                                        pRanges[2*i],0, pRanges[2*i+1],MAXROW, &bOnlyMatrix ) || bOnlyMatrix;
                else
                    bAllowed = pDoc->IsBlockEditable( nTab,
                                        0,pRanges[2*i], MAXCOL,pRanges[2*i+1], &bOnlyMatrix ) || bOnlyMatrix;
            }
        }
    if ( !bAllowed )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    USHORT nStart = pRanges[0];
    USHORT nEnd = pRanges[2*nRangeCnt-1];

    BOOL bFormula = FALSE;
    if ( eMode == SC_SIZE_OPTIMAL )
    {
        const ScViewOptions& rOpts = GetViewData()->GetOptions();
        bFormula = rOpts.GetOption( VOPT_FORMULAS );
    }

    ScDocument*     pUndoDoc = NULL;
    ScOutlineTable* pUndoTab = NULL;
    USHORT*         pUndoRanges = NULL;

    if ( bRecord )
    {
        pDoc->BeginDrawUndo();                          // Drawing Updates

        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        for (nTab=0; nTab<nTabCount; nTab++)
            if (pMarkData->GetTableSelect(nTab))
            {
                if (bWidth)
                {
                    if ( nTab == nFirstTab )
                        pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
                    else
                        pUndoDoc->AddUndoTab( nTab, nTab, TRUE, FALSE );
                    pDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
                }
                else
                {
                    if ( nTab == nFirstTab )
                        pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
                    else
                        pUndoDoc->AddUndoTab( nTab, nTab, FALSE, TRUE );
                    pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pUndoDoc );
                }
            }

        pUndoRanges = new USHORT[ 2*nRangeCnt ];
        memmove( pUndoRanges, pRanges, 2*nRangeCnt*sizeof(USHORT) );

        //! outlines from all tables?
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nCurTab );
        if (pTable)
            pUndoTab = new ScOutlineTable( *pTable );
    }

    if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
        pMarkData->MarkToMulti();

    BOOL bShow = nSizeTwips > 0 || eMode != SC_SIZE_DIRECT;
    BOOL bOutline = FALSE;

    for (nTab=0; nTab<nTabCount; nTab++)
        if (pMarkData->GetTableSelect(nTab))
        {
            const USHORT* pTabRanges = pRanges;

            pDoc->IncSizeRecalcLevel( nTab );       // nicht fuer jede Spalte einzeln
            for (USHORT nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
            {
                USHORT nStartNo = *(pTabRanges++);
                USHORT nEndNo = *(pTabRanges++);

                if ( !bWidth )                      // Hoehen immer blockweise
                {
                    if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                    {
                        BOOL bAll = ( eMode==SC_SIZE_OPTIMAL );
                        if (!bAll)
                        {
                            //  fuer alle eingeblendeten CR_MANUALSIZE loeschen,
                            //  dann SetOptimalHeight mit bShrink = FALSE
                            for (USHORT nRow=nStartNo; nRow<=nEndNo; nRow++)
                            {
                                BYTE nOld = pDoc->GetRowFlags(nRow,nTab);
                                if ( (nOld & CR_HIDDEN) == 0 && ( nOld & CR_MANUALSIZE ) )
                                    pDoc->SetRowFlags( nRow, nTab, nOld & ~CR_MANUALSIZE );
                            }
                        }
                        VirtualDevice aVirtDev;
                        pDoc->SetOptimalHeight( nStartNo, nEndNo, nTab, nSizeTwips, &aVirtDev,
                                                    GetViewData()->GetPPTX(),
                                                    GetViewData()->GetPPTY(),
                                                    GetViewData()->GetZoomX(),
                                                    GetViewData()->GetZoomY(), bAll );
                        if (bAll)
                            pDoc->ShowRows( nStartNo, nEndNo, nTab, TRUE );

                        //  Manual-Flag wird bei bAll=TRUE schon in SetOptimalHeight gesetzt
                        //  (an bei Extra-Height, sonst aus).
                    }
                    else if ( eMode==SC_SIZE_DIRECT )
                    {
                        if (nSizeTwips)
                            pDoc->SetRowHeightRange( nStartNo, nEndNo, nTab, nSizeTwips );
                        pDoc->ShowRows( nStartNo, nEndNo, nTab, nSizeTwips != 0 );
                        pDoc->SetManualHeight( nStartNo, nEndNo, nTab, TRUE );      // Manual-Flag
                    }
                    else if ( eMode==SC_SIZE_SHOW )
                    {
                        pDoc->ShowRows( nStartNo, nEndNo, nTab, TRUE );
                    }
                }
                else                                // Spaltenbreiten
                {
                    for (USHORT nCol=nStartNo; nCol<=nEndNo; nCol++)
                    {
                        if ( eMode != SC_SIZE_VISOPT ||
                             (pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN) == 0 )
                        {
                            USHORT nThisSize = nSizeTwips;

                            if ( eMode==SC_SIZE_OPTIMAL || eMode==SC_SIZE_VISOPT )
                                nThisSize = nSizeTwips + GetOptimalColWidth( nCol, nTab, bFormula );
                            if ( nThisSize )
                                pDoc->SetColWidth( nCol, nTab, nThisSize );

                            pDoc->ShowCol( nCol, nTab, bShow );
                        }
                    }
                }

                                    //  Outline anpassen

                if (bWidth)
                {
                    if ( pDoc->UpdateOutlineCol( nStartNo, nEndNo, nTab, bShow ) )
                        bOutline = TRUE;
                }
                else
                {
                    if ( pDoc->UpdateOutlineRow( nStartNo, nEndNo, nTab, bShow ) )
                        bOutline = TRUE;
                }
            }
            pDoc->DecSizeRecalcLevel( nTab );       // nicht fuer jede Spalte einzeln
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

    for (nTab=0; nTab<nTabCount; nTab++)
        if (pMarkData->GetTableSelect(nTab))
            pDoc->UpdatePageBreaks( nTab );

    GetViewData()->GetView()->UpdateScrollBars();

    if (bPaint)
    {
        HideCursor();

        for (nTab=0; nTab<nTabCount; nTab++)
            if (pMarkData->GetTableSelect(nTab))
            {
                if (bWidth)
                {
                    if (pDoc->HasAttrib( nStart,0,nTab, nEnd,MAXROW,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
                        nStart = 0;
                    if (nStart)             // weiter oben anfangen wegen Linien und Cursor
                        --nStart;
                    pDocSh->PostPaint( nStart, 0, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_TOP );
                }
                else
                {
                    if (pDoc->HasAttrib( 0,nStart,nTab, MAXCOL,nEnd,nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
                        nStart = 0;
                    if (nStart)
                        --nStart;
                    pDocSh->PostPaint( 0, nStart, nTab, MAXCOL, MAXROW, nTab, PAINT_GRID | PAINT_LEFT );
                }
            }

        pDocSh->UpdateOle(GetViewData());
        aModificator.SetDocumentModified();

        ShowCursor();
    }
}

//  Spaltenbreiten/Zeilenhoehen (ueber Blockmarken)

void ScViewFunc::SetMarkedWidthOrHeight( BOOL bWidth, ScSizeMode eMode, USHORT nSizeTwips,
                                        BOOL bRecord, BOOL bPaint )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();

    rMark.MarkToMulti();
    if (!rMark.IsMultiMarked())
    {
        USHORT nCol = GetViewData()->GetCurX();
        USHORT nRow = GetViewData()->GetCurY();
        USHORT nTab = GetViewData()->GetTabNo();
        DoneBlockMode();
        InitOwnBlockMode();
        rMark.SetMultiMarkArea( ScRange( nCol,nRow,nTab ), TRUE );
    }

    USHORT* pRanges = new USHORT[MAXROW+1];
    USHORT nRangeCnt = 0;

    if ( bWidth )
        nRangeCnt = rMark.GetMarkColumnRanges( pRanges );
    else
        nRangeCnt = rMark.GetMarkRowRanges( pRanges );

    SetWidthOrHeight( bWidth, nRangeCnt, pRanges, eMode, nSizeTwips, bRecord, bPaint );

    delete[] pRanges;
    rMark.MarkToSimple();
}

void ScViewFunc::ModifyCellSize( ScDirection eDir, BOOL bOptimal )
{
    //! Schrittweiten einstellbar
    //  Schrittweite ist auch Minimum
    USHORT nStepX = STD_COL_WIDTH / 5;
    USHORT nStepY = ScGlobal::nStdRowHeight;

    ScModule* pScMod = SC_MOD();
    BOOL bAnyEdit = pScMod->IsInputMode();
    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();
    ScDocument* pDoc = GetViewData()->GetDocument();

    BOOL bAllowed, bOnlyMatrix;
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

    USHORT nWidth = pDoc->GetColWidth( nCol, nTab );
    USHORT nHeight = pDoc->GetRowHeight( nRow, nTab );
    USHORT nRange[2];
    if ( eDir == DIR_LEFT || eDir == DIR_RIGHT )
    {
        if (bOptimal)               // Breite dieser einen Zelle
        {
            if ( bAnyEdit )
            {
                //  beim Editieren die aktuelle Breite der Eingabe
                ScInputHandler* pHdl = pScMod->GetInputHdl( GetViewData()->GetViewShell() );
                if (pHdl)
                {
                    long nEdit = pHdl->GetTextSize().Width();       // in 1/100mm

                    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
                    const SvxMarginItem& rMItem =
                            (const SvxMarginItem&)pPattern->GetItem(ATTR_MARGIN);
                    USHORT nMargin = rMItem.GetLeftMargin() + rMItem.GetRightMargin();
                    if ( ((const SvxHorJustifyItem&) pPattern->
                            GetItem( ATTR_HOR_JUSTIFY )).GetValue() == SVX_HOR_JUSTIFY_LEFT )
                        nMargin += ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();

                    ScDocShell* pDocSh = GetViewData()->GetDocShell();
                    nWidth = (USHORT)(nEdit * pDocSh->GetOutputFactor() / HMM_PER_TWIPS)
                                + nMargin + STD_EXTRA_WIDTH;
                }
            }
            else
            {
                VirtualDevice aVirtDev;
                aVirtDev.SetMapMode(MAP_PIXEL);
                long nPixel = pDoc->GetNeededSize( nCol, nRow, nTab, &aVirtDev,
                                GetViewData()->GetPPTX(), GetViewData()->GetPPTY(),
                                GetViewData()->GetZoomX(), GetViewData()->GetZoomY(),
                                TRUE );
                USHORT nTwips = (USHORT)( nPixel / GetViewData()->GetPPTX() );
                if (nTwips != 0)
                    nWidth = nTwips + STD_EXTRA_WIDTH;
                else
                    nWidth = STD_COL_WIDTH;
            }
        }
        else                        // vergroessern / verkleinern
        {
            if ( eDir == DIR_RIGHT )
                nWidth += nStepX;
            else if ( nWidth > nStepX )
                nWidth -= nStepX;
            if ( nWidth < nStepX ) nWidth = nStepX;
            if ( nWidth > MAX_COL_WIDTH ) nWidth = MAX_COL_WIDTH;
        }
        nRange[0] = nRange[1] = nCol;
        SetWidthOrHeight( TRUE, 1, nRange, SC_SIZE_DIRECT, nWidth );

        //  hier bei Breite auch Hoehe anpassen (nur die eine Zeile)

        if (!bAnyEdit)
        {
            const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            BOOL bNeedHeight =
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
                nHeight += nStepY;
            else if ( nHeight > nStepY )
                nHeight -= nStepY;
            if ( nHeight < nStepY ) nHeight = nStepY;
            if ( nHeight > MAX_COL_HEIGHT ) nHeight = MAX_COL_HEIGHT;
            //! MAX_COL_HEIGHT umbenennen in MAX_ROW_HEIGHT in global.hxx !!!!!!
        }
        nRange[0] = nRange[1] = nRow;
        SetWidthOrHeight( FALSE, 1, nRange, eMode, nHeight );
    }

    if ( bAnyEdit )
    {
        UpdateEditView();
        if ( pDoc->HasAttrib( nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_NEEDHEIGHT ) )
        {
            ScInputHandler* pHdl = pScMod->GetInputHdl( GetViewData()->GetViewShell() );
            if (pHdl)
                pHdl->SetModified();    // damit bei Enter die Hoehe angepasst wird
        }
    }

    ShowAllCursors();
}

void ScViewFunc::Protect( USHORT nTab, const String& rPassword )
{
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocFunc aFunc(*pDocSh);

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
        aFunc.Protect( nTab, rPassword, FALSE );
    else
    {
        //  modifying several tables is handled here

        String aUndo = ScGlobal::GetRscString( STR_UNDO_PROTECT_TAB );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

        USHORT nCount = pDocSh->GetDocument()->GetTableCount();
        for ( USHORT i=0; i<nCount; i++ )
            if ( rMark.GetTableSelect(i) )
                aFunc.Protect( i, rPassword, FALSE );

        pDocSh->GetUndoManager()->LeaveListAction();
    }

    UpdateLayerLocks();         //! broadcast to all views
}

BOOL ScViewFunc::Unprotect( USHORT nTab, const String& rPassword )
{
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocFunc aFunc(*pDocSh);
    BOOL bChanged = FALSE;

    if ( nTab == TABLEID_DOC || rMark.GetSelectCount() <= 1 )
        bChanged = aFunc.Unprotect( nTab, rPassword, FALSE );
    else
    {
        //  modifying several tables is handled here

        String aUndo = ScGlobal::GetRscString( STR_UNDO_UNPROTECT_TAB );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

        USHORT nCount = pDocSh->GetDocument()->GetTableCount();
        for ( USHORT i=0; i<nCount; i++ )
            if ( rMark.GetTableSelect(i) )
                if ( aFunc.Unprotect( i, rPassword, FALSE ) )
                    bChanged = TRUE;

        pDocSh->GetUndoManager()->LeaveListAction();
    }

    if (bChanged)
        UpdateLayerLocks();     //! broadcast to all views

    return bChanged;
}

void ScViewFunc::SetNote( USHORT nCol, USHORT nRow, USHORT nTab, const ScPostIt& rNote )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    pDocSh->GetDocFunc().SetNote( ScAddress(nCol,nRow,nTab), rNote, FALSE );
}

void ScViewFunc::SetNumberFormat( short nFormatType, ULONG nAdd )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ULONG               nNumberFormat = 0;
    ScViewData*         pViewData = GetViewData();
    ScDocument*         pDoc = pViewData->GetDocument();
    SvNumberFormatter*  pNumberFormatter = pDoc->GetFormatTable();
    LanguageType        eLanguage = ScGlobal::eLnge;
    ScPatternAttr       aNewAttrs( pDoc->GetPool() );

    //  #67936# always take language from cursor position, even if there is a selection

    ULONG nCurrentNumberFormat;
    pDoc->GetNumberFormat( pViewData->GetCurX(),
                           pViewData->GetCurY(),
                           pViewData->GetTabNo(),
                           nCurrentNumberFormat );
    const SvNumberformat* pEntry = pNumberFormatter->GetEntry( nCurrentNumberFormat );
    if (pEntry)
        eLanguage = pEntry->GetLanguage();      // sonst ScGlobal::eLnge behalten

    nNumberFormat = pNumberFormatter->GetStandardFormat( nFormatType, eLanguage ) + nAdd;

    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumberFormat ) );
    //  ATTR_LANGUAGE_FORMAT nicht
    ApplySelectionPattern( aNewAttrs, TRUE );
}

void ScViewFunc::SetNumFmtByStr( const String& rCode )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScViewData*         pViewData = GetViewData();
    ScDocument*         pDoc = pViewData->GetDocument();
    SvNumberFormatter*  pFormatter = pDoc->GetFormatTable();

    //  Sprache immer von Cursorposition

    ULONG nCurrentNumberFormat;
    pDoc->GetNumberFormat( pViewData->GetCurX(), pViewData->GetCurY(),
                           pViewData->GetTabNo(), nCurrentNumberFormat );
    const SvNumberformat* pEntry = pFormatter->GetEntry( nCurrentNumberFormat );
    LanguageType eLanguage = pEntry ? pEntry->GetLanguage() : ScGlobal::eLnge;

    //  Index fuer String bestimmen

    BOOL bOk = TRUE;
    ULONG nNumberFormat = pFormatter->GetEntryKey( rCode, eLanguage );
    if ( nNumberFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        //  neu eintragen

        String      aFormat = rCode;    // wird veraendert
        xub_StrLen  nErrPos = 0;
        short       nType   = 0;        //! ???
        bOk = pFormatter->PutEntry( aFormat, nErrPos, nType, nNumberFormat, eLanguage );
    }

    if ( bOk )          // gueltiges Format?
    {
        ScPatternAttr aNewAttrs( pDoc->GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumberFormat ) );
        rSet.Put( SvxLanguageItem( eLanguage, ATTR_LANGUAGE_FORMAT ) );
        ApplySelectionPattern( aNewAttrs, TRUE );
    }

    //! sonst Fehler zuerueckgeben / Meldung ausgeben ???
}

void ScViewFunc::ChangeNumFmtDecimals( BOOL bIncrement )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScDocument*         pDoc = GetViewData()->GetDocument();
    SvNumberFormatter*  pFormatter = pDoc->GetFormatTable();

    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();

    ULONG nOldFormat;
    pDoc->GetNumberFormat( nCol, nRow, nTab, nOldFormat );
    const SvNumberformat* pOldEntry = pFormatter->GetEntry( nOldFormat );
    if (!pOldEntry)
    {
        DBG_ERROR("Zahlformat nicht gefunden !!!");
        return;
    }

    //  was haben wir denn da?

    ULONG nNewFormat = nOldFormat;
    BOOL bError = FALSE;

    LanguageType eLanguage = pOldEntry->GetLanguage();
    BOOL bThousand, bNegRed;
    USHORT nPrecision, nLeading;
    //  GetFormatSpecialInfo ist nicht-const
    ((SvNumberformat*)pOldEntry)->GetFormatSpecialInfo(
                                    bThousand, bNegRed, nPrecision, nLeading );

    short nOldType = pOldEntry->GetType();
    if ( 0 == ( nOldType & (
                NUMBERFORMAT_NUMBER |  NUMBERFORMAT_CURRENCY | NUMBERFORMAT_PERCENT ) ) )
    {
        //  Datum, Zeit, Bruch, logisch, Text kann nicht angepasst werden
        //! bei Wisssenschaftlich kann es der Numberformatter auch nicht
        bError = TRUE;
    }

    //! Das SvNumberformat hat einen Member bStandard, verraet ihn aber nicht
    BOOL bWasStandard = ( nOldFormat == pFormatter->GetStandardIndex( eLanguage ) );
    if (bWasStandard)
    {
        //  bei "Standard" die Nachkommastellen abhaengig vom Zellinhalt
        //  0 bei leer oder Text -> keine Nachkommastellen
        double nVal = pDoc->GetValue( ScAddress( nCol, nRow, nTab ) );

        //  Die Wege des Numberformatters sind unergruendlich, darum ausprobieren:
        String aOut;
        Color* pCol;
        ((SvNumberformat*)pOldEntry)->GetOutputString( nVal, aOut, &pCol );

        nPrecision = 0;
        // 'E' fuer Exponential ist fest im Numberformatter
        if ( aOut.Search('E') != STRING_NOTFOUND )
            bError = TRUE;                              // Exponential nicht veraendern
        else
        {
            sal_Unicode cDecSep;
            if ( eLanguage == ScGlobal::pScInternational->GetLanguage() )
                cDecSep = ScGlobal::pScInternational->GetNumDecimalSep();
            else
                cDecSep = International(eLanguage).GetNumDecimalSep();
            xub_StrLen nPos = aOut.Search( cDecSep );
            if ( nPos != STRING_NOTFOUND )
                nPrecision = aOut.Len() - nPos - 1;
            // sonst 0 behalten
        }
    }

    if (!bError)
    {
        if (bIncrement)
        {
            if (nPrecision<20)
                ++nPrecision;           // erhoehen
            else
                bError = TRUE;          // 20 ist Maximum
        }
        else
        {
            if (nPrecision)
                --nPrecision;           // vermindern
            else
                bError = TRUE;          // weniger als 0 geht nicht
        }
    }

    if (!bError)
    {
        String aNewPicture;
        pFormatter->GenerateFormat( aNewPicture, nOldFormat, eLanguage,
                                    bThousand, bNegRed, nPrecision, nLeading );

        nNewFormat = pFormatter->GetEntryKey( aNewPicture, eLanguage );
        if ( nNewFormat == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            xub_StrLen nErrPos = 0;
            short nNewType = 0;
            BOOL bOk = pFormatter->PutEntry( aNewPicture, nErrPos,
                                                nNewType, nNewFormat, eLanguage );
            DBG_ASSERT( bOk, "falsches Zahlformat generiert" );
            if (!bOk)
                bError = TRUE;
        }
    }

    if (!bError)
    {
        ScPatternAttr aNewAttrs( pDoc->GetPool() );
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
        //  ATTR_LANGUAGE_FORMAT nicht
        ApplySelectionPattern( aNewAttrs, TRUE );
    }
    else
        Sound::Beep();              // war nix
}

void ScViewFunc::ChangeIndent( BOOL bIncrement )
{
    ScViewData* pViewData = GetViewData();
    ScDocShell* pDocSh  = pViewData->GetDocShell();
    ScMarkData& rMark   = pViewData->GetMarkData();

    ScMarkData aWorkMark = rMark;
    aWorkMark.MarkToMulti();
    if (!aWorkMark.IsMultiMarked())
    {
        USHORT nCol = pViewData->GetCurX();
        USHORT nRow = pViewData->GetCurY();
        USHORT nTab = pViewData->GetTabNo();
        aWorkMark.SetMultiMarkArea( ScRange(nCol,nRow,nTab) );
    }

    BOOL bSuccess = pDocSh->GetDocFunc().ChangeIndent( aWorkMark, bIncrement, FALSE );
    if (bSuccess)
    {
        pDocSh->UpdateOle(pViewData);
        StartFormatArea();
    }
}

BOOL ScViewFunc::InsertName( const String& rName, const String& rSymbol,
                                const String& rType )
{
    //  Type = P,R,C,F (und Kombinationen)
    //! Undo...

    BOOL bOk = FALSE;
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nTab = GetViewData()->GetTabNo();
    ScRangeName* pList = pDoc->GetRangeName();

    RangeType nType = RT_NAME;
    ScRangeData* pNewEntry = new ScRangeData( pDoc, rName, rSymbol,
                                    GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab,
                                    nType );
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

    if ( !pNewEntry->GetErrCode() )     //  Text gueltig?
    {
        ScDocShellModificator aModificator( *pDocSh );

        pDoc->CompileNameFormula( TRUE );   // CreateFormulaString

        // Eintrag bereits vorhanden? Dann vorher entfernen (=Aendern)
        USHORT nFoundAt;
        if ( pList->SearchName( rName, nFoundAt ) )
        {                                   // alten Index uebernehmen
            pNewEntry->SetIndex( ((ScRangeData*)pList->At(nFoundAt))->GetIndex() );
            pList->AtFree( nFoundAt );
        }

        if ( pList->Insert( pNewEntry ) )
        {
            pNewEntry = NULL;   // nicht loeschen
            bOk = TRUE;
        }

        pDoc->CompileNameFormula( FALSE );  // CompileFormulaString
        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
    }

    delete pNewEntry;       // wenn er nicht eingefuegt wurde
    return bOk;
}

void ScViewFunc::CreateNames( USHORT nFlags )
{
    BOOL bDone = FALSE;
    ScRange aRange;
    if ( GetViewData()->GetSimpleArea(aRange) )
        bDone = GetViewData()->GetDocShell()->GetDocFunc().CreateNames( aRange, nFlags, FALSE );

    if (!bDone)
        ErrorMessage(STR_CREATENAME_MARKERR);
}

USHORT ScViewFunc::GetCreateNameFlags()
{
    USHORT nFlags = 0;

    USHORT nStartCol,nStartRow,nEndCol,nEndRow;
    USHORT nDummy;
    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nDummy,nEndCol,nEndRow,nDummy))
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        USHORT nTab = GetViewData()->GetTabNo();
        USHORT i;
        BOOL bOk;

        bOk = TRUE;
        USHORT nFirstCol = nStartCol;
        USHORT nLastCol  = nEndCol;
        if (nStartCol+1 < nEndCol) { ++nFirstCol; --nLastCol; }
        for (i=nFirstCol; i<=nLastCol && bOk; i++)
            if (!pDoc->HasStringData( i,nStartRow,nTab ))
                bOk = FALSE;
        if (bOk)
            nFlags |= NAME_TOP;
        else                            // Bottom nur wenn nicht Top
        {
            bOk = TRUE;
            for (i=nFirstCol; i<=nLastCol && bOk; i++)
                if (!pDoc->HasStringData( i,nEndRow,nTab ))
                    bOk = FALSE;
            if (bOk)
                nFlags |= NAME_BOTTOM;
        }

        bOk = TRUE;
        USHORT nFirstRow = nStartRow;
        USHORT nLastRow  = nEndRow;
        if (nStartRow+1 < nEndRow) { ++nFirstRow; --nLastRow; }
        for (i=nFirstRow; i<=nLastRow && bOk; i++)
            if (!pDoc->HasStringData( nStartCol,i,nTab ))
                bOk = FALSE;
        if (bOk)
            nFlags |= NAME_LEFT;
        else                            // Right nur wenn nicht Left
        {
            bOk = TRUE;
            for (i=nFirstRow; i<=nLastRow && bOk; i++)
                if (!pDoc->HasStringData( nEndCol,i,nTab ))
                    bOk = FALSE;
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
    if ( pDocSh->GetDocFunc().InsertNameList( aPos, FALSE ) )
        pDocSh->UpdateOle(GetViewData());
}




