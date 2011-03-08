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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <svx/hlnkitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/svxerr.hxx>
#include <editeng/unolingu.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/langtab.hxx>
#include <svtools/filter.hxx>
#include <svl/stritem.hxx>
#include <svtools/transfer.hxx>
#include <svl/urlbmk.hxx>
#include <vcl/msgbox.hxx>
#include <avmedia/mediawindow.hxx>

#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>

#include "viewfunc.hxx"
#include "docsh.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "undoblk.hxx"
#include "undocell.hxx"
#include "cell.hxx"
#include "scmod.hxx"
#include "spelleng.hxx"
#include "patattr.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "editutil.hxx"
#include "editable.hxx"
#include "dociter.hxx"
#include "reffind.hxx"
#include "compiler.hxx"

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

BOOL bPasteIsDrop = FALSE;

//==================================================================

void ScViewFunc::PasteRTF( SCCOL nStartCol, SCROW nStartRow,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::datatransfer::XTransferable >& rxTransferable )
{
    TransferableDataHelper aDataHelper( rxTransferable );
    if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_EDITENGINE ) )
    {
        HideAllCursors();

        ScDocument* pUndoDoc = NULL;

        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        SCTAB nTab = GetViewData()->GetTabNo();
        const BOOL bRecord (pDoc->IsUndoEnabled());

        const ScPatternAttr* pPattern = pDoc->GetPattern( nStartCol, nStartRow, nTab );
        ScTabEditEngine* pEngine = new ScTabEditEngine( *pPattern, pDoc->GetEnginePool() );
        pEngine->EnableUndo( FALSE );

        Window* pActWin = GetActiveWin();
        if (pActWin)
        {
            pEngine->SetPaperSize(Size(100000,100000));
            Window aWin( pActWin );
            EditView aEditView( pEngine, &aWin );
            aEditView.SetOutputArea(Rectangle(0,0,100000,100000));

            // same method now for clipboard or drag&drop
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            aEditView.InsertText( rxTransferable, String(), TRUE );
        }

        ULONG nParCnt = pEngine->GetParagraphCount();
        if (nParCnt)
        {
            SCROW nEndRow = nStartRow + static_cast<SCROW>(nParCnt) - 1;
            if (nEndRow > MAXROW)
                nEndRow = MAXROW;

            if (bRecord)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, IDF_ALL, FALSE, pUndoDoc );
            }

            SCROW nRow = nStartRow;
            for( USHORT n = 0; n < nParCnt; n++ )
            {
                EditTextObject* pObject = pEngine->CreateTextObject( n );
                EnterData( nStartCol, nRow, nTab, pObject, FALSE, TRUE );
                            // kein Undo, auf einfache Strings testen
                delete pObject;
                if( ++nRow > MAXROW )
                    break;
            }

            if (bRecord)
            {
                ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
                pRedoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, IDF_ALL|IDF_NOCAPTIONS, FALSE, pRedoDoc );

                ScMarkData aDestMark;
                aDestMark.SelectOneTable( nTab );
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoPaste( pDocSh, nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab,
                                        aDestMark,
                                        pUndoDoc, pRedoDoc, IDF_ALL, NULL,NULL,NULL,NULL ) );
            }
        }

        delete pEngine;

        ShowAllCursors();
    }
    else
    {
        HideAllCursors();
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScImportExport aImpEx( pDocSh->GetDocument(),
            ScAddress( nStartCol, nStartRow, GetViewData()->GetTabNo() ) );

        ::rtl::OUString aStr;
        SotStorageStreamRef xStream;
        if ( aDataHelper.GetSotStorageStream( SOT_FORMAT_RTF, xStream ) && xStream.Is() )
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            aImpEx.ImportStream( *xStream, String(), SOT_FORMAT_RTF );
        else if ( aDataHelper.GetString( SOT_FORMAT_RTF, aStr ) )
            aImpEx.ImportString( aStr, SOT_FORMAT_RTF );

        AdjustRowHeight( nStartRow, aImpEx.GetRange().aEnd.Row() );
        pDocSh->UpdateOle(GetViewData());
        ShowAllCursors();
    }
}
void ScViewFunc::DoRefConversion( BOOL bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    SCTAB nTabCount = pDoc->GetTableCount();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    ScRange aMarkRange;
    rMark.MarkToSimple();
    BOOL bMulti = rMark.IsMultiMarked();
    if (bMulti)
        rMark.GetMultiMarkArea( aMarkRange );
    else if (rMark.IsMarked())
        rMark.GetMarkArea( aMarkRange );
    else
    {
        aMarkRange = ScRange( GetViewData()->GetCurX(),
            GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
    }
    ScEditableTester aTester( pDoc, aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                            aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(),rMark );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    BOOL bOk = FALSE;

    ScDocument* pUndoDoc = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nTab = aMarkRange.aStart.Tab();
        pUndoDoc->InitUndo( pDoc, nTab, nTab );

        if ( rMark.GetSelectCount() > 1 )
        {
            for (SCTAB i=0; i<nTabCount; i++)
                if ( rMark.GetTableSelect(i) && i != nTab )
                    pUndoDoc->AddUndoTab( i, i );
        }
        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ALL, bMulti, pUndoDoc, &rMark );
    }

    ScRangeListRef xRanges;
    GetViewData()->GetMultiArea( xRanges );
    size_t nCount = xRanges->size();

    for (SCTAB i=0; i<nTabCount; i++)
    {
        if (rMark.GetTableSelect(i))
        {
            for (size_t j = 0; j < nCount; ++j)
            {
                ScRange aRange = *(*xRanges)[j];
                aRange.aStart.SetTab(i);
                aRange.aEnd.SetTab(i);
                ScCellIterator aIter( pDoc, aRange );
                ScBaseCell* pCell = aIter.GetFirst();
                while ( pCell )
                {
                    if (pCell->GetCellType() == CELLTYPE_FORMULA)
                    {
                        String aOld;
                        ((ScFormulaCell*)pCell)->GetFormula(aOld);
                        xub_StrLen nLen = aOld.Len();
                        ScRefFinder aFinder( aOld, pDoc );
                        aFinder.ToggleRel( 0, nLen );
                        if (aFinder.GetFound())
                        {
                            ScAddress aPos = ((ScFormulaCell*)pCell)->aPos;
                            String aNew = aFinder.GetText();
                            ScCompiler aComp( pDoc, aPos);
                            aComp.SetGrammar(pDoc->GetGrammar());
                            ScTokenArray* pArr = aComp.CompileString( aNew );
                            ScFormulaCell* pNewCell = new ScFormulaCell( pDoc, aPos,
                                                        pArr,formula::FormulaGrammar::GRAM_DEFAULT, MM_NONE );
                            pDoc->PutCell( aPos, pNewCell );
                            bOk = TRUE;
                        }
                    }
                    pCell = aIter.GetNext();
                }
            }
        }
    }
    if (bRecord)
    {
        ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nTab = aMarkRange.aStart.Tab();
        pRedoDoc->InitUndo( pDoc, nTab, nTab );

        if ( rMark.GetSelectCount() > 1 )
        {
            for (SCTAB i=0; i<nTabCount; i++)
                if ( rMark.GetTableSelect(i) && i != nTab )
                    pRedoDoc->AddUndoTab( i, i );
        }
        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ALL, bMulti, pRedoDoc, &rMark );

        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoRefConversion( pDocSh,
                                    aMarkRange, rMark, pUndoDoc, pRedoDoc, bMulti, IDF_ALL) );
    }

    pDocSh->PostPaint( aMarkRange, PAINT_GRID );
    pDocSh->UpdateOle(GetViewData());
    pDocSh->SetDocumentModified();
    CellContentChanged();

    if (!bOk)
        ErrorMessage(STR_ERR_NOREF);
}
//  Thesaurus - Undo ok
void ScViewFunc::DoThesaurus( BOOL bRecord )
{
    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScSplitPos eWhich = GetViewData()->GetActivePart();
    CellType eCellType;
    EESpellState eState;
    String sOldText, sNewString;
    EditTextObject* pOldTObj = NULL;
    const EditTextObject* pTObject = NULL;
    ScBaseCell* pCell = NULL;
    EditView* pEditView = NULL;
    ESelection* pEditSel = NULL;
    ScEditEngineDefaulter* pThesaurusEngine;
    BOOL bIsEditMode = GetViewData()->HasEditView(eWhich);
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if (bIsEditMode)                                            // Edit-Mode aktiv
    {
        GetViewData()->GetEditView(eWhich, pEditView, nCol, nRow);
        pEditSel = new ESelection(pEditView->GetSelection());
        SC_MOD()->InputEnterHandler();
        GetViewData()->GetBindings().Update();          // sonst kommt der Sfx durcheinander...
    }
    else
    {
        nCol = GetViewData()->GetCurX();
        nRow = GetViewData()->GetCurY();
    }
    nTab = GetViewData()->GetTabNo();

    ScEditableTester aTester( pDoc, nCol, nRow, nCol, nRow, rMark );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        delete pEditSel;
        return;
    }
    pDoc->GetCellType(nCol, nRow, nTab, eCellType);
    if (eCellType != CELLTYPE_STRING && eCellType != CELLTYPE_EDIT)
    {
        ErrorMessage(STR_THESAURUS_NO_STRING);
        return;
    }

    com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellChecker1>
                                        xSpeller = LinguMgr::GetSpellChecker();
    //! if (...)  // thesaurus not available
    //! {
    //!     ErrorMessage(STR_EXPORT_ASCII_WARNING);
    //!     delete pEditSel;
    //!     return;
    //! }

    pThesaurusEngine = new ScEditEngineDefaulter( pDoc->GetEnginePool() );
    pThesaurusEngine->SetEditTextObjectPool( pDoc->GetEditPool() );
    pThesaurusEngine->SetRefDevice(GetViewData()->GetActiveWin());
    pThesaurusEngine->SetSpeller(xSpeller);
    MakeEditView(pThesaurusEngine, nCol, nRow );
    const ScPatternAttr* pPattern = NULL;
    SfxItemSet* pEditDefaults = new SfxItemSet(pThesaurusEngine->GetEmptyItemSet());
    pPattern = pDoc->GetPattern(nCol, nRow, nTab);
    if (pPattern )
    {
        pPattern->FillEditItemSet( pEditDefaults );
        pThesaurusEngine->SetDefaults( *pEditDefaults );
    }

    if (eCellType == CELLTYPE_STRING)
    {
        pDoc->GetString(nCol, nRow, nTab, sOldText);
        pThesaurusEngine->SetText(sOldText);
    }
    else if (eCellType == CELLTYPE_EDIT)
    {
        pDoc->GetCell(nCol, nRow, nTab, pCell);
        if (pCell)
        {
            ((ScEditCell*) pCell)->GetData(pTObject);
            if (pTObject)
            {
                pOldTObj = pTObject->Clone();
                pThesaurusEngine->SetText(*pTObject);
            }
        }
    }
    else
    {
        OSL_FAIL("DoThesaurus: Keine String oder Editzelle");
    }
    pEditView = GetViewData()->GetEditView(GetViewData()->GetActivePart());;
    if (pEditSel)
        pEditView->SetSelection(*pEditSel);
    else
        pEditView->SetSelection(ESelection(0,0,0,0));

    pThesaurusEngine->ClearModifyFlag();

    //  language is now in EditEngine attributes -> no longer passed to StartThesaurus

    eState = pEditView->StartThesaurus();
    DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

    if (eState == EE_SPELL_ERRORFOUND)              // sollte spaeter durch Wrapper geschehen!
    {
        LanguageType eLnge = ScViewUtil::GetEffLanguage( pDoc, ScAddress( nCol, nRow, nTab ) );
        SvtLanguageTable aLangTab;
        String aErr = aLangTab.GetString(eLnge);
        aErr += ScGlobal::GetRscString( STR_SPELLING_NO_LANG );
        InfoBox aBox( GetViewData()->GetDialogParent(), aErr );
        aBox.Execute();
    }
    if (pThesaurusEngine->IsModified())
    {
        EditTextObject* pNewTObj = NULL;
        if (pCell && pTObject)
        {
            pNewTObj = pThesaurusEngine->CreateTextObject();
            pCell = new ScEditCell( pNewTObj, pDoc,
                pThesaurusEngine->GetEditTextObjectPool() );
            pDoc->PutCell( nCol, nRow, nTab, pCell );
        }
        else
        {
            sNewString = pThesaurusEngine->GetText();
            pDoc->SetString(nCol, nRow, nTab, sNewString);
        }
// erack! it's broadcasted
//      pDoc->SetDirty();
        pDocSh->SetDocumentModified();
        if (bRecord)
        {
            GetViewData()->GetDocShell()->GetUndoManager()->AddUndoAction(
                new ScUndoThesaurus( GetViewData()->GetDocShell(),
                                     nCol, nRow, nTab,
                                     sOldText, pOldTObj, sNewString, pNewTObj));
        }
        delete pNewTObj;
    }
    KillEditView(TRUE);
    delete pEditDefaults;
    delete pThesaurusEngine;
    delete pOldTObj;
    delete pEditSel;
    pDocSh->PostPaintGridAll();
}

void ScViewFunc::DoHangulHanjaConversion( BOOL bRecord )
{
    ScConversionParam aConvParam( SC_CONVERSION_HANGULHANJA, LANGUAGE_KOREAN, 0, true );
    DoSheetConversion( aConvParam, bRecord );
}

void ScViewFunc::DoSheetConversion( const ScConversionParam& rConvParam, BOOL bRecord )
{
    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    ScViewData& rViewData = *GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = rViewData.GetMarkData();
    ScSplitPos eWhich = rViewData.GetActivePart();
    EditView* pEditView = NULL;
    BOOL bIsEditMode = rViewData.HasEditView(eWhich);
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if (bIsEditMode)                                            // Edit-Mode aktiv
    {
        rViewData.GetEditView(eWhich, pEditView, nCol, nRow);
        SC_MOD()->InputEnterHandler();
    }
    else
    {
        nCol = rViewData.GetCurX();
        nRow = rViewData.GetCurY();

        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP);
    }
    nTab = rViewData.GetTabNo();

    rMark.MarkToMulti();
    BOOL bMarked = rMark.IsMultiMarked();
    if (bMarked)
    {
        ScEditableTester aTester( pDoc, rMark );
        if (!aTester.IsEditable())
        {
            ErrorMessage(aTester.GetMessageId());
            return;
        }
    }

    ScDocument* pUndoDoc = NULL;
    ScDocument* pRedoDoc = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab );
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, nTab, nTab );

        if ( rMark.GetSelectCount() > 1 )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB i=0; i<nTabCount; i++)
                if ( rMark.GetTableSelect(i) && i != nTab )
                {
                    pUndoDoc->AddUndoTab( i, i );
                    pRedoDoc->AddUndoTab( i, i );
                }
        }
    }

    //  ab hier kein return mehr

    BOOL bOldDis = pDoc->IsIdleDisabled();
    pDoc->DisableIdle( TRUE );   // stop online spelling

    // *** create and init the edit engine *** --------------------------------

    ScConversionEngineBase* pEngine = NULL;
    switch( rConvParam.GetType() )
    {
        case SC_CONVERSION_SPELLCHECK:
            pEngine = new ScSpellingEngine(
                pDoc->GetEnginePool(), rViewData, pUndoDoc, pRedoDoc, LinguMgr::GetSpellChecker() );
        break;
        case SC_CONVERSION_HANGULHANJA:
        case SC_CONVERSION_CHINESE_TRANSL:
            pEngine = new ScTextConversionEngine(
                pDoc->GetEnginePool(), rViewData, rConvParam, pUndoDoc, pRedoDoc );
        break;
        default:
            DBG_ERRORFILE( "ScViewFunc::DoSheetConversion - unknown conversion type" );
    }

    MakeEditView( pEngine, nCol, nRow );
    pEngine->SetRefDevice( rViewData.GetActiveWin() );
                                        // dummy Zelle simulieren:
    pEditView = rViewData.GetEditView( rViewData.GetActivePart() );
    rViewData.SetSpellingView( pEditView );
    Rectangle aRect( Point( 0, 0 ), Point( 0, 0 ) );
    pEditView->SetOutputArea( aRect );
    pEngine->SetControlWord( EE_CNTRL_USECHARATTRIBS );
    pEngine->EnableUndo( FALSE );
    pEngine->SetPaperSize( aRect.GetSize() );
    pEngine->SetText( EMPTY_STRING );

    // *** do the conversion *** ----------------------------------------------

    pEngine->ClearModifyFlag();
    pEngine->ConvertAll( *pEditView );

    // *** undo/redo *** ------------------------------------------------------

    if( pEngine->IsAnyModified() )
    {
        if (bRecord)
        {
            SCCOL nNewCol = rViewData.GetCurX();
            SCROW nNewRow = rViewData.GetCurY();
            rViewData.GetDocShell()->GetUndoManager()->AddUndoAction(
                new ScUndoConversion(
                        pDocSh, rMark,
                        nCol, nRow, nTab, pUndoDoc,
                        nNewCol, nNewRow, nTab, pRedoDoc, rConvParam ) );
        }
        pDoc->SetDirty();
        pDocSh->SetDocumentModified();
    }
    else
    {
        delete pUndoDoc;
        delete pRedoDoc;
    }

    // *** final cleanup *** --------------------------------------------------

    rViewData.SetSpellingView( NULL );
    KillEditView(TRUE);
    delete pEngine;
    pDocSh->PostPaintGridAll();
    rViewData.GetViewShell()->UpdateInputHandler();
    pDoc->DisableIdle(bOldDis);
}

// Pasten von FORMAT_FILE-Items
//  wird nicht direkt aus Drop aufgerufen, sondern asynchron -> Dialoge sind erlaubt

BOOL ScViewFunc::PasteFile( const Point& rPos, const String& rFile, BOOL bLink )
{
    INetURLObject aURL;
    aURL.SetSmartURL( rFile );
    String aStrURL = aURL.GetMainURL( INetURLObject::NO_DECODE );

    // is it a media URL?
    if( ::avmedia::MediaWindow::isMediaURL( aStrURL ) )
    {
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, aStrURL );
        return BOOL( 0 != GetViewData()->GetDispatcher().Execute(
                                SID_INSERT_AVMEDIA, SFX_CALLMODE_SYNCHRON,
                                &aMediaURLItem, 0L ) );
    }

    if (!bLink)     // bei bLink nur Grafik oder URL
    {
        // 1. Kann ich die Datei oeffnen?
        const SfxFilter* pFlt = NULL;

        // nur nach eigenen Filtern suchen, ohne Auswahlbox (wie in ScDocumentLoader)
        SfxFilterMatcher aMatcher( ScDocShell::Factory().GetFilterContainer()->GetName() );
        SfxMedium aSfxMedium( aStrURL, (STREAM_READ | STREAM_SHARE_DENYNONE), FALSE );
        // #i73992# GuessFilter no longer calls UseInteractionHandler.
        // This is UI, so it can be called here.
        aSfxMedium.UseInteractionHandler(TRUE);
        ErrCode nErr = aMatcher.GuessFilter( aSfxMedium, &pFlt );

        if ( pFlt && !nErr )
        {
            // Code aus dem SFX geklaut!
            SfxDispatcher &rDispatcher = GetViewData()->GetDispatcher();
            SfxStringItem aFileNameItem( SID_FILE_NAME, aStrURL );
            SfxStringItem aFilterItem( SID_FILTER_NAME, pFlt->GetName() );
            // #i69524# add target, as in SfxApplication when the Open dialog is used
            SfxStringItem aTargetItem( SID_TARGETNAME, String::CreateFromAscii("_default") );

            // Asynchron oeffnen, kann naemlich auch aus D&D heraus passieren
            // und das bekommt dem MAC nicht so gut ...
            return BOOL( 0 != rDispatcher.Execute( SID_OPENDOC,
                                    SFX_CALLMODE_ASYNCHRON, &aFileNameItem, &aFilterItem, &aTargetItem, 0L) );
        }
    }

    // 2. Kann die Datei ueber die Grafik-Filter eingefuegt werden?
    // (als Link, weil Gallery das so anbietet)

    USHORT nFilterFormat;
    Graphic aGraphic;
    GraphicFilter* pGraphicFilter = GraphicFilter::GetGraphicFilter();


    if (!pGraphicFilter->ImportGraphic(aGraphic, aURL,
            GRFILTER_FORMAT_DONTKNOW, &nFilterFormat ))
    {
        if ( bLink )
        {
            String aFltName = pGraphicFilter->GetImportFormatName(nFilterFormat);
            return PasteGraphic( rPos, aGraphic, aStrURL, aFltName );
        }
        else
        {
            // #i76709# if bLink isn't set, pass empty URL/filter, so a non-linked image is inserted
            return PasteGraphic( rPos, aGraphic, EMPTY_STRING, EMPTY_STRING );
        }
    }

    if (bLink)                      // bei bLink alles, was nicht Grafik ist, als URL
    {
        Rectangle aRect( rPos, Size(0,0) );
        ScRange aRange = GetViewData()->GetDocument()->
                            GetRange( GetViewData()->GetTabNo(), aRect );
        SCCOL nPosX = aRange.aStart.Col();
        SCROW nPosY = aRange.aStart.Row();

        InsertBookmark( aStrURL, aStrURL, nPosX, nPosY );
        return TRUE;
    }
    else
    {
        // 3. Kann die Datei als OLE eingefuegt werden?
        // auch nicht-Storages, z.B. Sounds (#38282#)
        uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();

        //TODO/LATER: what about "bLink"?

        uno::Sequence < beans::PropertyValue > aMedium(1);
        aMedium[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
        aMedium[0].Value <<= ::rtl::OUString( aStrURL );

        comphelper::EmbeddedObjectContainer aCnt( xStorage );
        ::rtl::OUString aName;
        uno::Reference < embed::XEmbeddedObject > xObj = aCnt.InsertEmbeddedObject( aMedium, aName );
        if( xObj.is() )
            return PasteObject( rPos, xObj );

        // If an OLE object can't be created, insert a URL button

        GetViewData()->GetViewShell()->InsertURLButton( aStrURL, aStrURL, EMPTY_STRING, &rPos );
        return TRUE;
    }
}

BOOL ScViewFunc::PasteBookmark( ULONG nFormatId,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY )
{
    INetBookmark aBookmark;
    TransferableDataHelper aDataHelper( rxTransferable );
    if ( !aDataHelper.GetINetBookmark( nFormatId, aBookmark ) )
        return FALSE;

    InsertBookmark( aBookmark.GetDescription(), aBookmark.GetURL(), nPosX, nPosY );
    return TRUE;
}

void ScViewFunc::InsertBookmark( const String& rDescription, const String& rURL,
                                    SCCOL nPosX, SCROW nPosY, const String* pTarget,
                                    BOOL bTryReplace )
{
    ScViewData* pViewData = GetViewData();
    if ( pViewData->HasEditView( pViewData->GetActivePart() ) &&
            nPosX >= pViewData->GetEditStartCol() && nPosX <= pViewData->GetEditEndCol() &&
            nPosY >= pViewData->GetEditStartRow() && nPosY <= pViewData->GetEditEndRow() )
    {
        //  in die gerade editierte Zelle einfuegen

        String aTargetFrame;
        if (pTarget)
            aTargetFrame = *pTarget;
        pViewData->GetViewShell()->InsertURLField( rDescription, rURL, aTargetFrame );
        return;
    }

    //  in nicht editierte Zelle einfuegen

    ScDocument* pDoc = GetViewData()->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScAddress aCellPos( nPosX, nPosY, nTab );
    ScBaseCell* pCell = pDoc->GetCell( aCellPos );
    EditEngine aEngine( pDoc->GetEnginePool() );
    if (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_EDIT)
        {
            const EditTextObject* pOld = ((ScEditCell*)pCell)->GetData();
            if (pOld)
                aEngine.SetText(*pOld);
        }
        else
        {
            String aOld;
            pDoc->GetInputString( nPosX, nPosY, nTab, aOld );
            if (aOld.Len())
                aEngine.SetText(aOld);
        }
    }

    USHORT nPara = aEngine.GetParagraphCount();
    if (nPara)
        --nPara;
    xub_StrLen nTxtLen = aEngine.GetTextLen(nPara);
    ESelection aInsSel( nPara, nTxtLen, nPara, nTxtLen );

    if ( bTryReplace && HasBookmarkAtCursor( NULL ) )
    {
        //  if called from hyperlink slot and cell contains only a URL,
        //  replace old URL with new one

        aInsSel = ESelection( 0, 0, 0, 1 );     // replace first character (field)
    }

    SvxURLField aField( rURL, rDescription, SVXURLFORMAT_APPDEFAULT );
    if (pTarget)
        aField.SetTargetFrame(*pTarget);
    aEngine.QuickInsertField( SvxFieldItem( aField, EE_FEATURE_FIELD ), aInsSel );

    EditTextObject* pData = aEngine.CreateTextObject();
    EnterData( nPosX, nPosY, nTab, pData );
    delete pData;
}

BOOL ScViewFunc::HasBookmarkAtCursor( SvxHyperlinkItem* pContent )
{
    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
    ScDocument* pDoc = GetViewData()->GetDocShell()->GetDocument();

    ScBaseCell* pCell = pDoc->GetCell( aPos );
    if ( pCell && pCell->GetCellType() == CELLTYPE_EDIT )
    {
        const EditTextObject* pData = ((ScEditCell*)pCell)->GetData();
        if (pData)
        {
            BOOL bField = pData->IsFieldObject();
            if (bField)
            {
                const SvxFieldItem* pFieldItem = pData->GetField();
                if (pFieldItem)
                {
                    const SvxFieldData* pField = pFieldItem->GetField();
                    if ( pField && pField->ISA(SvxURLField) )
                    {
                        if (pContent)
                        {
                            const SvxURLField* pURLField = (const SvxURLField*)pField;
                            pContent->SetName( pURLField->GetRepresentation() );
                            pContent->SetURL( pURLField->GetURL() );
                            pContent->SetTargetFrame( pURLField->GetTargetFrame() );
                        }
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
