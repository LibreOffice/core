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
#include <vcl/graphicfilter.hxx>
#include <svl/stritem.hxx>
#include <svtools/transfer.hxx>
#include <svl/urlbmk.hxx>
#include <vcl/msgbox.hxx>
#include <avmedia/mediawindow.hxx>

#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>

#include "viewfunc.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "undoblk.hxx"
#include "undocell.hxx"
#include "formulacell.hxx"
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

#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

sal_Bool bPasteIsDrop = false;

//==================================================================

void ScViewFunc::PasteRTF( SCCOL nStartCol, SCROW nStartRow,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::datatransfer::XTransferable >& rxTransferable )
{
    TransferableDataHelper aDataHelper( rxTransferable );
    if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_EDITENGINE ) )
    {
        HideAllCursors();


        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        SCTAB nTab = GetViewData()->GetTabNo();
        const sal_Bool bRecord (pDoc->IsUndoEnabled());

        const ScPatternAttr* pPattern = pDoc->GetPattern( nStartCol, nStartRow, nTab );
        ScTabEditEngine* pEngine = new ScTabEditEngine( *pPattern, pDoc->GetEnginePool() );
        pEngine->EnableUndo( false );

        Window* pActWin = GetActiveWin();
        if (pActWin)
        {
            pEngine->SetPaperSize(Size(100000,100000));
            Window aWin( pActWin );
            EditView aEditView( pEngine, &aWin );
            aEditView.SetOutputArea(Rectangle(0,0,100000,100000));

            // same method now for clipboard or drag&drop
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            aEditView.InsertText( rxTransferable, String(), sal_True );
        }

        sal_Int32 nParCnt = pEngine->GetParagraphCount();
        if (nParCnt)
        {
            SCROW nEndRow = nStartRow + static_cast<SCROW>(nParCnt) - 1;
            if (nEndRow > MAXROW)
                nEndRow = MAXROW;

            ScDocument* pUndoDoc = NULL;
            if (bRecord)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, IDF_ALL, false, pUndoDoc );
            }

            SCROW nRow = nStartRow;

            // Temporarily turn off undo generation for this lot
            bool bUndoEnabled = pDoc->IsUndoEnabled();
            pDoc->EnableUndo( false );
            for( sal_Int32 n = 0; n < nParCnt; n++ )
            {
                boost::scoped_ptr<EditTextObject> pObject(pEngine->CreateTextObject(n));
                EnterData(nStartCol, nRow, nTab, *pObject, true);
                if( ++nRow > MAXROW )
                    break;
            }
            pDoc->EnableUndo(bUndoEnabled);

            if (bRecord)
            {
                ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
                pRedoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, IDF_ALL|IDF_NOCAPTIONS, false, pRedoDoc );

                ScRange aMarkRange(nStartCol, nStartRow, nTab, nStartCol, nEndRow, nTab);
                ScMarkData aDestMark;
                aDestMark.SetMarkArea( aMarkRange );
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoPaste( pDocSh, aMarkRange, aDestMark,
                                     pUndoDoc, pRedoDoc, IDF_ALL, NULL));
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

        OUString aStr;
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
void ScViewFunc::DoRefConversion( sal_Bool bRecord )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    SCTAB nTabCount = pDoc->GetTableCount();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    ScRange aMarkRange;
    rMark.MarkToSimple();
    sal_Bool bMulti = rMark.IsMultiMarked();
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
    sal_Bool bOk = false;

    ScDocument* pUndoDoc = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nTab = aMarkRange.aStart.Tab();
        pUndoDoc->InitUndo( pDoc, nTab, nTab );

        if ( rMark.GetSelectCount() > 1 )
        {
            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd; ++itr)
                if ( *itr != nTab )
                    pUndoDoc->AddUndoTab( *itr, *itr );
        }
        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, IDF_ALL, bMulti, pUndoDoc, &rMark );
    }

    ScRangeListRef xRanges;
    GetViewData()->GetMultiArea( xRanges );
    size_t nCount = xRanges->size();

    ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd; ++itr)
    {
        SCTAB i = *itr;
        for (size_t j = 0; j < nCount; ++j)
        {
            ScRange aRange = *(*xRanges)[j];
            aRange.aStart.SetTab(i);
            aRange.aEnd.SetTab(i);
            ScCellIterator aIter( pDoc, aRange );
            for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
            {
                if (aIter.getType() != CELLTYPE_FORMULA)
                    continue;

                ScFormulaCell* pCell = aIter.getFormulaCell();
                OUString aOld;
                pCell->GetFormula(aOld);
                sal_Int32 nLen = aOld.getLength();
                ScRefFinder aFinder( aOld, aIter.GetPos(), pDoc, pDoc->GetAddressConvention() );
                aFinder.ToggleRel( 0, nLen );
                if (aFinder.GetFound())
                {
                    ScAddress aPos = pCell->aPos;
                    String aNew = aFinder.GetText();
                    ScCompiler aComp( pDoc, aPos);
                    aComp.SetGrammar(pDoc->GetGrammar());
                    ScTokenArray* pArr = aComp.CompileString( aNew );
                    ScFormulaCell* pNewCell = new ScFormulaCell( pDoc, aPos,
                                                pArr,formula::FormulaGrammar::GRAM_DEFAULT, MM_NONE );
                    pDoc->SetFormulaCell(aPos, pNewCell);
                    bOk = true;
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
            itr = rMark.begin();
            for (; itr != itrEnd; ++itr)
                if ( *itr != nTab )
                    pRedoDoc->AddUndoTab( *itr, *itr );
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
void ScViewFunc::DoThesaurus( sal_Bool bRecord )
{
    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScSplitPos eWhich = GetViewData()->GetActivePart();
    EESpellState eState;
    String sOldText, sNewString;
    EditTextObject* pOldTObj = NULL;
    const EditTextObject* pTObject = NULL;
    EditView* pEditView = NULL;
    boost::scoped_ptr<ESelection> pEditSel;
    ScEditEngineDefaulter* pThesaurusEngine;
    sal_Bool bIsEditMode = GetViewData()->HasEditView(eWhich);
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    if (bIsEditMode)                                            // Edit-Mode aktiv
    {
        GetViewData()->GetEditView(eWhich, pEditView, nCol, nRow);
        pEditSel.reset(new ESelection(pEditView->GetSelection()));
        SC_MOD()->InputEnterHandler();
        GetViewData()->GetBindings().Update();          // sonst kommt der Sfx durcheinander...
    }
    else
    {
        nCol = GetViewData()->GetCurX();
        nRow = GetViewData()->GetCurY();
    }
    nTab = GetViewData()->GetTabNo();

    ScAddress aPos(nCol, nRow, nTab);
    ScEditableTester aTester( pDoc, nCol, nRow, nCol, nRow, rMark );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    CellType eCellType = pDoc->GetCellType(aPos);
    if (eCellType != CELLTYPE_STRING && eCellType != CELLTYPE_EDIT)
    {
        ErrorMessage(STR_THESAURUS_NO_STRING);
        return;
    }

    uno::Reference<linguistic2::XSpellChecker1> xSpeller = LinguMgr::GetSpellChecker();

    pThesaurusEngine = new ScEditEngineDefaulter( pDoc->GetEnginePool() );
    pThesaurusEngine->SetEditTextObjectPool( pDoc->GetEditPool() );
    pThesaurusEngine->SetRefDevice(GetViewData()->GetActiveWin());
    pThesaurusEngine->SetSpeller(xSpeller);
    MakeEditView(pThesaurusEngine, nCol, nRow );
    const ScPatternAttr* pPattern = NULL;
    SfxItemSet* pEditDefaults = new SfxItemSet(pThesaurusEngine->GetEmptyItemSet());
    pPattern = pDoc->GetPattern(nCol, nRow, nTab);
    if (pPattern)
    {
        pPattern->FillEditItemSet( pEditDefaults );
        pThesaurusEngine->SetDefaults( *pEditDefaults );
    }

    if (eCellType == CELLTYPE_STRING)
    {
        sOldText = pDoc->GetString(aPos);
        pThesaurusEngine->SetText(sOldText);
    }
    else if (eCellType == CELLTYPE_EDIT)
    {
        pTObject = pDoc->GetEditText(aPos);
        if (pTObject)
        {
            pOldTObj = pTObject->Clone();
            pThesaurusEngine->SetText(*pTObject);
        }
    }
    else
    {
        OSL_FAIL("DoThesaurus: Keine String oder Editzelle");
    }
    pEditView = GetViewData()->GetEditView(GetViewData()->GetActivePart());
    if (pEditSel)
        pEditView->SetSelection(*pEditSel);
    else
        pEditView->SetSelection(ESelection(0,0,0,0));

    pThesaurusEngine->ClearModifyFlag();

    //  language is now in EditEngine attributes -> no longer passed to StartThesaurus

    eState = pEditView->StartThesaurus();
    OSL_ENSURE(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

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
        if (pTObject)
        {
            // The cell will own the text object instance.
            pDoc->SetEditText(
                ScAddress(nCol,nRow,nTab), pThesaurusEngine->CreateTextObject());
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
    KillEditView(sal_True);
    delete pEditDefaults;
    delete pThesaurusEngine;
    delete pOldTObj;
    pDocSh->PostPaintGridAll();
}

void ScViewFunc::DoHangulHanjaConversion( sal_Bool bRecord )
{
    ScConversionParam aConvParam( SC_CONVERSION_HANGULHANJA, LANGUAGE_KOREAN, 0, true );
    DoSheetConversion( aConvParam, bRecord );
}

void ScViewFunc::DoSheetConversion( const ScConversionParam& rConvParam, sal_Bool bRecord )
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
    sal_Bool bIsEditMode = rViewData.HasEditView(eWhich);
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
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
    sal_Bool bMarked = rMark.IsMultiMarked();
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
            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd; ++itr)
                if ( *itr != nTab )
                {
                    pUndoDoc->AddUndoTab( *itr, *itr );
                    pRedoDoc->AddUndoTab( *itr, *itr );
                }
        }
    }

    //  ab hier kein return mehr

    bool bOldEnabled = pDoc->IsIdleEnabled();
    pDoc->EnableIdle(false);   // stop online spelling

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
            OSL_FAIL( "ScViewFunc::DoSheetConversion - unknown conversion type" );
    }

    MakeEditView( pEngine, nCol, nRow );
    pEngine->SetRefDevice( rViewData.GetActiveWin() );
                                        // dummy Zelle simulieren:
    pEditView = rViewData.GetEditView( rViewData.GetActivePart() );
    rViewData.SetSpellingView( pEditView );
    Rectangle aRect( Point( 0, 0 ), Point( 0, 0 ) );
    pEditView->SetOutputArea( aRect );
    pEngine->SetControlWord( EE_CNTRL_USECHARATTRIBS );
    pEngine->EnableUndo( false );
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
    KillEditView(sal_True);
    delete pEngine;
    pDocSh->PostPaintGridAll();
    rViewData.GetViewShell()->UpdateInputHandler();
    pDoc->EnableIdle(bOldEnabled);
}

// Pasten von FORMAT_FILE-Items
//  wird nicht direkt aus Drop aufgerufen, sondern asynchron -> Dialoge sind erlaubt

sal_Bool ScViewFunc::PasteFile( const Point& rPos, const String& rFile, sal_Bool bLink )
{
    INetURLObject aURL;
    aURL.SetSmartURL( rFile );
    String aStrURL = aURL.GetMainURL( INetURLObject::NO_DECODE );

    // is it a media URL?
    if( ::avmedia::MediaWindow::isMediaURL( aStrURL ) )
    {
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, aStrURL );
        return sal_Bool( 0 != GetViewData()->GetDispatcher().Execute(
                                SID_INSERT_AVMEDIA, SFX_CALLMODE_SYNCHRON,
                                &aMediaURLItem, 0L ) );
    }

    if (!bLink)     // bei bLink nur Grafik oder URL
    {
        // 1. Kann ich die Datei oeffnen?
        const SfxFilter* pFlt = NULL;

        // nur nach eigenen Filtern suchen, ohne Auswahlbox (wie in ScDocumentLoader)
        SfxFilterMatcher aMatcher( ScDocShell::Factory().GetFilterContainer()->GetName() );
        SfxMedium aSfxMedium( aStrURL, (STREAM_READ | STREAM_SHARE_DENYNONE) );
        // #i73992# GuessFilter no longer calls UseInteractionHandler.
        // This is UI, so it can be called here.
        aSfxMedium.UseInteractionHandler(sal_True);
        ErrCode nErr = aMatcher.GuessFilter( aSfxMedium, &pFlt );

        if ( pFlt && !nErr )
        {
            // Code aus dem SFX geklaut!
            SfxDispatcher &rDispatcher = GetViewData()->GetDispatcher();
            SfxStringItem aFileNameItem( SID_FILE_NAME, aStrURL );
            SfxStringItem aFilterItem( SID_FILTER_NAME, pFlt->GetName() );
            // #i69524# add target, as in SfxApplication when the Open dialog is used
            SfxStringItem aTargetItem( SID_TARGETNAME, OUString("_default") );

            // Asynchron oeffnen, kann naemlich auch aus D&D heraus passieren
            // und das bekommt dem MAC nicht so gut ...
            return sal_Bool( 0 != rDispatcher.Execute( SID_OPENDOC,
                                    SFX_CALLMODE_ASYNCHRON, &aFileNameItem, &aFilterItem, &aTargetItem, 0L) );
        }
    }

    // 2. Kann die Datei ueber die Grafik-Filter eingefuegt werden?
    // (als Link, weil Gallery das so anbietet)

    sal_uInt16 nFilterFormat;
    Graphic aGraphic;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();


    if (!rGraphicFilter.ImportGraphic(aGraphic, aURL,
            GRFILTER_FORMAT_DONTKNOW, &nFilterFormat ))
    {
        if ( bLink )
        {
            String aFltName = rGraphicFilter.GetImportFormatName(nFilterFormat);
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
        return sal_True;
    }
    else
    {
        // 3. Kann die Datei als OLE eingefuegt werden?
        // auch nicht-Storages, z.B. Sounds (#38282#)
        uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();

        //TODO/LATER: what about "bLink"?

        uno::Sequence < beans::PropertyValue > aMedium(1);
        aMedium[0].Name = OUString( "URL" );
        aMedium[0].Value <<= OUString( aStrURL );

        comphelper::EmbeddedObjectContainer aCnt( xStorage );
        OUString aName;
        uno::Reference < embed::XEmbeddedObject > xObj = aCnt.InsertEmbeddedObject( aMedium, aName );
        if( xObj.is() )
            return PasteObject( rPos, xObj );

        // If an OLE object can't be created, insert a URL button

        GetViewData()->GetViewShell()->InsertURLButton( aStrURL, aStrURL, EMPTY_STRING, &rPos );
        return sal_True;
    }
}

sal_Bool ScViewFunc::PasteBookmark( sal_uLong nFormatId,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY )
{
    INetBookmark aBookmark;
    TransferableDataHelper aDataHelper( rxTransferable );
    if ( !aDataHelper.GetINetBookmark( nFormatId, aBookmark ) )
        return false;

    InsertBookmark( aBookmark.GetDescription(), aBookmark.GetURL(), nPosX, nPosY );
    return sal_True;
}

void ScViewFunc::InsertBookmark( const String& rDescription, const String& rURL,
                                    SCCOL nPosX, SCROW nPosY, const String* pTarget,
                                    sal_Bool bTryReplace )
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
    EditEngine aEngine( pDoc->GetEnginePool() );

    const EditTextObject* pOld = pDoc->GetEditText(aCellPos);
    if (pOld)
        aEngine.SetText(*pOld);
    else
    {
        OUString aOld;
        pDoc->GetInputString(nPosX, nPosY, nTab, aOld);
        if (!aOld.isEmpty())
            aEngine.SetText(aOld);
    }

    sal_Int32 nPara = aEngine.GetParagraphCount();
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

    boost::scoped_ptr<EditTextObject> pData(aEngine.CreateTextObject());
    EnterData(nPosX, nPosY, nTab, *pData);
}

bool ScViewFunc::HasBookmarkAtCursor( SvxHyperlinkItem* pContent )
{
    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
    ScDocument* pDoc = GetViewData()->GetDocShell()->GetDocument();

    const EditTextObject* pData = pDoc->GetEditText(aPos);
    if (!pData)
        return false;

    if (!pData->IsFieldObject())
        // not a field object.
        return false;

    const SvxFieldItem* pFieldItem = pData->GetField();
    if (!pFieldItem)
        // doesn't have a field item.
        return false;

    const SvxFieldData* pField = pFieldItem->GetField();
    if (!pField)
        // doesn't have a field item data.
        return false;

    if (pField->GetClassId() != com::sun::star::text::textfield::Type::URL)
        // not a URL field.
        return false;

    if (pContent)
    {
        const SvxURLField* pURLField = static_cast<const SvxURLField*>(pField);
        pContent->SetName( pURLField->GetRepresentation() );
        pContent->SetURL( pURLField->GetURL() );
        pContent->SetTargetFrame( pURLField->GetTargetFrame() );
    }
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
