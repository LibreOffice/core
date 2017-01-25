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
#include <sot/storage.hxx>
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
#include <svl/sharedstringpool.hxx>
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
#include "tokenarray.hxx"
#include <refupdatecontext.hxx>
#include <gridwin.hxx>

using namespace com::sun::star;

bool bPasteIsDrop = false;

void ScViewFunc::PasteRTF( SCCOL nStartCol, SCROW nStartRow,
                                const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable )
{
    TransferableDataHelper aDataHelper( rxTransferable );
    if ( aDataHelper.HasFormat( SotClipboardFormatId::EDITENGINE ) )
    {
        HideAllCursors();

        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = GetViewData().GetTabNo();
        const bool bRecord (rDoc.IsUndoEnabled());

        const ScPatternAttr* pPattern = rDoc.GetPattern( nStartCol, nStartRow, nTab );
        std::unique_ptr<ScTabEditEngine> pEngine(new ScTabEditEngine( *pPattern, rDoc.GetEnginePool() ));
        pEngine->EnableUndo( false );

        vcl::Window* pActWin = GetActiveWin();
        if (pActWin)
        {
            pEngine->SetPaperSize(Size(100000,100000));
            ScopedVclPtrInstance< vcl::Window > aWin( pActWin );
            EditView aEditView( pEngine.get(), aWin.get() );
            aEditView.SetOutputArea(Rectangle(0,0,100000,100000));

            // same method now for clipboard or drag&drop
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            aEditView.InsertText( rxTransferable, OUString(), true );
        }

        sal_Int32 nParCnt = pEngine->GetParagraphCount();
        if (nParCnt)
        {
            SCROW nEndRow = nStartRow + static_cast<SCROW>(nParCnt) - 1;
            if (nEndRow > MAXROW)
                nEndRow = MAXROW;

            ScDocument* pUndoDoc = nullptr;
            if (bRecord)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo( &rDoc, nTab, nTab );
                rDoc.CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, InsertDeleteFlags::ALL, false, *pUndoDoc );
            }

            SCROW nRow = nStartRow;

            // Temporarily turn off undo generation for this lot
            bool bUndoEnabled = rDoc.IsUndoEnabled();
            rDoc.EnableUndo( false );
            for( sal_Int32 n = 0; n < nParCnt; n++ )
            {
                std::unique_ptr<EditTextObject> pObject(pEngine->CreateTextObject(n));
                EnterData(nStartCol, nRow, nTab, *pObject, true);
                if( ++nRow > MAXROW )
                    break;
            }
            rDoc.EnableUndo(bUndoEnabled);

            if (bRecord)
            {
                ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
                pRedoDoc->InitUndo( &rDoc, nTab, nTab );
                rDoc.CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, InsertDeleteFlags::ALL|InsertDeleteFlags::NOCAPTIONS, false, *pRedoDoc );

                ScRange aMarkRange(nStartCol, nStartRow, nTab, nStartCol, nEndRow, nTab);
                ScMarkData aDestMark;
                aDestMark.SetMarkArea( aMarkRange );
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoPaste( pDocSh, aMarkRange, aDestMark,
                                     pUndoDoc, pRedoDoc, InsertDeleteFlags::ALL, nullptr));
            }
        }

        pEngine.reset();

        ShowAllCursors();
    }
    else
    {
        HideAllCursors();
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScImportExport aImpEx( &pDocSh->GetDocument(),
            ScAddress( nStartCol, nStartRow, GetViewData().GetTabNo() ) );

        OUString aStr;
        tools::SvRef<SotStorageStream> xStream;
        if ( aDataHelper.GetSotStorageStream( SotClipboardFormatId::RTF, xStream ) && xStream.is() )
            // mba: clipboard always must contain absolute URLs (could be from alien source)
            aImpEx.ImportStream( *xStream, OUString(), SotClipboardFormatId::RTF );
        else if ( aDataHelper.GetString( SotClipboardFormatId::RTF, aStr ) )
            aImpEx.ImportString( aStr, SotClipboardFormatId::RTF );
        else if ( aDataHelper.GetSotStorageStream( SotClipboardFormatId::RICHTEXT, xStream ) && xStream.is() )
            aImpEx.ImportStream( *xStream, OUString(), SotClipboardFormatId::RICHTEXT );
        else if ( aDataHelper.GetString( SotClipboardFormatId::RICHTEXT, aStr ) )
            aImpEx.ImportString( aStr, SotClipboardFormatId::RICHTEXT );

        AdjustRowHeight( nStartRow, aImpEx.GetRange().aEnd.Row() );
        pDocSh->UpdateOle(&GetViewData());
        ShowAllCursors();
    }
}
void ScViewFunc::DoRefConversion()
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();
    SCTAB nTabCount = pDoc->GetTableCount();
    bool bRecord = true;
    if (!pDoc->IsUndoEnabled())
        bRecord = false;

    ScRange aMarkRange;
    rMark.MarkToSimple();
    bool bMulti = rMark.IsMultiMarked();
    if (bMulti)
        rMark.GetMultiMarkArea( aMarkRange );
    else if (rMark.IsMarked())
        rMark.GetMarkArea( aMarkRange );
    else
    {
        aMarkRange = ScRange( GetViewData().GetCurX(),
            GetViewData().GetCurY(), GetViewData().GetTabNo() );
    }
    ScEditableTester aTester( pDoc, aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                            aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(),rMark );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    bool bOk = false;

    ScDocument* pUndoDoc = nullptr;
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
        pDoc->CopyToDocument( aCopyRange, InsertDeleteFlags::ALL, bMulti, *pUndoDoc, &rMark );
    }

    ScRangeListRef xRanges;
    GetViewData().GetMultiArea( xRanges );
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
                    OUString aNew = aFinder.GetText();
                    ScCompiler aComp( pDoc, aPos);
                    aComp.SetGrammar(pDoc->GetGrammar());
                    std::unique_ptr<ScTokenArray> pArr(aComp.CompileString(aNew));
                    ScFormulaCell* pNewCell =
                        new ScFormulaCell(
                            pDoc, aPos, *pArr, formula::FormulaGrammar::GRAM_DEFAULT, MM_NONE);

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
        pDoc->CopyToDocument( aCopyRange, InsertDeleteFlags::ALL, bMulti, *pRedoDoc, &rMark );

        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoRefConversion( pDocSh,
                                    aMarkRange, rMark, pUndoDoc, pRedoDoc, bMulti) );
    }

    pDocSh->PostPaint( aMarkRange, PaintPartFlags::Grid );
    pDocSh->UpdateOle(&GetViewData());
    pDocSh->SetDocumentModified();
    CellContentChanged();

    if (!bOk)
        ErrorMessage(STR_ERR_NOREF);
}
//  Thesaurus - Undo ok
void ScViewFunc::DoThesaurus()
{
    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();
    ScSplitPos eWhich = GetViewData().GetActivePart();
    EESpellState eState;
    EditView* pEditView = nullptr;
    std::unique_ptr<ESelection> pEditSel;
    std::unique_ptr<ScEditEngineDefaulter> pThesaurusEngine;
    bool bIsEditMode = GetViewData().HasEditView(eWhich);
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;
    if (bIsEditMode)                                            // edit mode active
    {
        GetViewData().GetEditView(eWhich, pEditView, nCol, nRow);
        pEditSel.reset(new ESelection(pEditView->GetSelection()));
        SC_MOD()->InputEnterHandler();
        GetViewData().GetBindings().Update();          // otherwise the Sfx becomes mixed-up...
    }
    else
    {
        nCol = GetViewData().GetCurX();
        nRow = GetViewData().GetCurY();
    }
    nTab = GetViewData().GetTabNo();

    ScAddress aPos(nCol, nRow, nTab);
    ScEditableTester aTester( &rDoc, nCol, nRow, nCol, nRow, rMark );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    ScCellValue aOldText;
    aOldText.assign(rDoc, aPos);
    if (aOldText.meType != CELLTYPE_STRING && aOldText.meType != CELLTYPE_EDIT)
    {
        ErrorMessage(STR_THESAURUS_NO_STRING);
        return;
    }

    uno::Reference<linguistic2::XSpellChecker1> xSpeller = LinguMgr::GetSpellChecker();

    pThesaurusEngine.reset(new ScEditEngineDefaulter(rDoc.GetEnginePool()));
    pThesaurusEngine->SetEditTextObjectPool( rDoc.GetEditPool() );
    pThesaurusEngine->SetRefDevice(GetViewData().GetActiveWin());
    pThesaurusEngine->SetSpeller(xSpeller);
    MakeEditView(pThesaurusEngine.get(), nCol, nRow );
    const ScPatternAttr* pPattern = nullptr;
    std::unique_ptr<SfxItemSet> pEditDefaults(
        new SfxItemSet(pThesaurusEngine->GetEmptyItemSet()));
    pPattern = rDoc.GetPattern(nCol, nRow, nTab);
    if (pPattern)
    {
        pPattern->FillEditItemSet( pEditDefaults.get() );
        pThesaurusEngine->SetDefaults( *pEditDefaults );
    }

    if (aOldText.meType == CELLTYPE_EDIT)
        pThesaurusEngine->SetText(*aOldText.mpEditText);
    else
        pThesaurusEngine->SetText(aOldText.getString(&rDoc));

    pEditView = GetViewData().GetEditView(GetViewData().GetActivePart());
    if (pEditSel)
        pEditView->SetSelection(*pEditSel);
    else
        pEditView->SetSelection(ESelection(0,0,0,0));

    pThesaurusEngine->ClearModifyFlag();

    //  language is now in EditEngine attributes -> no longer passed to StartThesaurus

    eState = pEditView->StartThesaurus();
    OSL_ENSURE(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

    if (eState == EE_SPELL_ERRORFOUND)              // should happen later through Wrapper!
    {
        LanguageType eLnge = ScViewUtil::GetEffLanguage( &rDoc, ScAddress( nCol, nRow, nTab ) );
        OUString aErr = SvtLanguageTable::GetLanguageString(eLnge);
        aErr += ScGlobal::GetRscString( STR_SPELLING_NO_LANG );
        ScopedVclPtrInstance< InfoBox > aBox( GetViewData().GetDialogParent(), aErr );
        aBox->Execute();
    }
    if (pThesaurusEngine->IsModified())
    {
        ScCellValue aNewText;

        if (aOldText.meType == CELLTYPE_EDIT)
        {
            // The cell will own the text object instance.
            EditTextObject* pText = pThesaurusEngine->CreateTextObject();
            if (rDoc.SetEditText(ScAddress(nCol,nRow,nTab), pText))
                aNewText.set(*pText);
        }
        else
        {
            OUString aStr = pThesaurusEngine->GetText();
            aNewText.set(rDoc.GetSharedStringPool().intern(aStr));
            rDoc.SetString(nCol, nRow, nTab, aStr);
        }

        pDocSh->SetDocumentModified();
        if (bRecord)
        {
            GetViewData().GetDocShell()->GetUndoManager()->AddUndoAction(
                new ScUndoThesaurus(
                    GetViewData().GetDocShell(), nCol, nRow, nTab, aOldText, aNewText));
        }
    }

    KillEditView(true);
    pDocSh->PostPaintGridAll();
}

void ScViewFunc::DoHangulHanjaConversion()
{
    ScConversionParam aConvParam( SC_CONVERSION_HANGULHANJA, LANGUAGE_KOREAN, 0, true );
    DoSheetConversion( aConvParam );
}

void ScViewFunc::DoSheetConversion( const ScConversionParam& rConvParam )
{
    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    ScViewData& rViewData = GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData& rMark = rViewData.GetMarkData();
    ScSplitPos eWhich = rViewData.GetActivePart();
    EditView* pEditView = nullptr;
    bool bIsEditMode = rViewData.HasEditView(eWhich);
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;
    if (bIsEditMode)                                            // edit mode active
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
    bool bMarked = rMark.IsMultiMarked();
    if (bMarked)
    {
        ScEditableTester aTester( &rDoc, rMark );
        if (!aTester.IsEditable())
        {
            ErrorMessage(aTester.GetMessageId());
            return;
        }
    }

    ScDocument* pUndoDoc = nullptr;
    ScDocument* pRedoDoc = nullptr;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( &rDoc, nTab, nTab );
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( &rDoc, nTab, nTab );

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

    // from here no return

    bool bOldEnabled = rDoc.IsIdleEnabled();
    rDoc.EnableIdle(false);   // stop online spelling

    // *** create and init the edit engine *** --------------------------------

    std::unique_ptr<ScConversionEngineBase> pEngine;
    switch( rConvParam.GetType() )
    {
        case SC_CONVERSION_SPELLCHECK:
            pEngine.reset(new ScSpellingEngine(
                rDoc.GetEnginePool(), rViewData, pUndoDoc, pRedoDoc, LinguMgr::GetSpellChecker() ));
        break;
        case SC_CONVERSION_HANGULHANJA:
        case SC_CONVERSION_CHINESE_TRANSL:
            pEngine.reset(new ScTextConversionEngine(
                rDoc.GetEnginePool(), rViewData, rConvParam, pUndoDoc, pRedoDoc ));
        break;
        default:
            OSL_FAIL( "ScViewFunc::DoSheetConversion - unknown conversion type" );
    }

    MakeEditView( pEngine.get(), nCol, nRow );
    pEngine->SetRefDevice( rViewData.GetActiveWin() );
                                        // simulate dummy cell:
    pEditView = rViewData.GetEditView( rViewData.GetActivePart() );
    rViewData.SetSpellingView( pEditView );
    Rectangle aRect( Point( 0, 0 ), Point( 0, 0 ) );
    pEditView->SetOutputArea( aRect );
    pEngine->SetControlWord( EEControlBits::USECHARATTRIBS );
    pEngine->EnableUndo( false );
    pEngine->SetPaperSize( aRect.GetSize() );
    pEngine->SetText( EMPTY_OUSTRING );

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

        sc::SetFormulaDirtyContext aCxt;
        rDoc.SetAllFormulasDirty(aCxt);

        pDocSh->SetDocumentModified();
    }
    else
    {
        delete pUndoDoc;
        delete pRedoDoc;
    }

    // *** final cleanup *** --------------------------------------------------

    rViewData.SetSpellingView( nullptr );
    KillEditView(true);
    pEngine.reset();
    pDocSh->PostPaintGridAll();
    rViewData.GetViewShell()->UpdateInputHandler();
    rDoc.EnableIdle(bOldEnabled);
}

// past from SotClipboardFormatId::FILE items
// is not called directly from Drop, but asynchronously -> dialogs are allowed

bool ScViewFunc::PasteFile( const Point& rPos, const OUString& rFile, bool bLink )
{
    INetURLObject aURL;
    aURL.SetSmartURL( rFile );
    OUString aStrURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    // is it a media URL?
    if( ::avmedia::MediaWindow::isMediaURL( aStrURL, ""/*TODO?*/ ) )
    {
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, aStrURL );
        return ( nullptr != GetViewData().GetDispatcher().ExecuteList(
                                SID_INSERT_AVMEDIA, SfxCallMode::SYNCHRON,
                                { &aMediaURLItem }) );
    }

    if (!bLink)     // for bLink only graphics or URL
    {
        // 1. can I open the file?
        std::shared_ptr<const SfxFilter> pFlt;

        // search only for its own filters, without selection box (as in ScDocumentLoader)
        SfxFilterMatcher aMatcher( ScDocShell::Factory().GetFilterContainer()->GetName() );
        SfxMedium aSfxMedium( aStrURL, (StreamMode::READ | StreamMode::SHARE_DENYNONE) );
        // #i73992# GuessFilter no longer calls UseInteractionHandler.
        // This is UI, so it can be called here.
        aSfxMedium.UseInteractionHandler(true);
        ErrCode nErr = aMatcher.GuessFilter( aSfxMedium, pFlt );

        if ( pFlt && !nErr )
        {
            // code stolen from the SFX!
            SfxDispatcher &rDispatcher = GetViewData().GetDispatcher();
            SfxStringItem aFileNameItem( SID_FILE_NAME, aStrURL );
            SfxStringItem aFilterItem( SID_FILTER_NAME, pFlt->GetName() );
            // #i69524# add target, as in SfxApplication when the Open dialog is used
            SfxStringItem aTargetItem( SID_TARGETNAME, OUString("_default") );

            // Open Asynchronously, because it can also happen from D&D
            // and that is not so good for the MAC...
            return (nullptr != rDispatcher.ExecuteList(SID_OPENDOC,
                                SfxCallMode::ASYNCHRON,
                                { &aFileNameItem, &aFilterItem, &aTargetItem}));
        }
    }

    // 2. can the file be inserted using the graphics filter?
    // (as a link, since the Gallery provides it in this way)

    sal_uInt16 nFilterFormat;
    Graphic aGraphic;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    if (!rGraphicFilter.ImportGraphic(aGraphic, aURL,
            GRFILTER_FORMAT_DONTKNOW, &nFilterFormat ))
    {
        if ( bLink )
        {
            OUString aFltName = rGraphicFilter.GetImportFormatName(nFilterFormat);
            return PasteGraphic( rPos, aGraphic, aStrURL, aFltName );
        }
        else
        {
            // #i76709# if bLink isn't set, pass empty URL/filter, so a non-linked image is inserted
            return PasteGraphic( rPos, aGraphic, EMPTY_OUSTRING, EMPTY_OUSTRING );
        }
    }

    if (bLink)                      // for bLink everything, which is not graphics, as URL
    {
        Rectangle aRect( rPos, Size(0,0) );
        ScRange aRange = GetViewData().GetDocument()->
                            GetRange( GetViewData().GetTabNo(), aRect );
        SCCOL nPosX = aRange.aStart.Col();
        SCROW nPosY = aRange.aStart.Row();

        InsertBookmark( aStrURL, aStrURL, nPosX, nPosY );
        return true;
    }
    else
    {
        // 3. can the file be inserted as OLE?
        // also non-storages, for instance sounds (#38282#)
        uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();

        //TODO/LATER: what about "bLink"?

        uno::Sequence < beans::PropertyValue > aMedium(1);
        aMedium[0].Name = "URL";
        aMedium[0].Value <<= OUString( aStrURL );

        comphelper::EmbeddedObjectContainer aCnt( xStorage );
        OUString aName;
        uno::Reference < embed::XEmbeddedObject > xObj = aCnt.InsertEmbeddedObject( aMedium, aName );
        if( xObj.is() )
            return PasteObject( rPos, xObj, nullptr );

        // If an OLE object can't be created, insert a URL button

        GetViewData().GetViewShell()->InsertURLButton( aStrURL, aStrURL, EMPTY_OUSTRING, &rPos );
        return true;
    }
}

bool ScViewFunc::PasteBookmark( SotClipboardFormatId nFormatId,
                                const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable,
                                SCCOL nPosX, SCROW nPosY )
{
    INetBookmark aBookmark;
    TransferableDataHelper aDataHelper( rxTransferable );
    if ( !aDataHelper.GetINetBookmark( nFormatId, aBookmark ) )
        return false;

    InsertBookmark( aBookmark.GetDescription(), aBookmark.GetURL(), nPosX, nPosY );
    return true;
}

void ScViewFunc::InsertBookmark( const OUString& rDescription, const OUString& rURL,
                                    SCCOL nPosX, SCROW nPosY, const OUString* pTarget,
                                    bool bTryReplace )
{
    ScViewData& rViewData = GetViewData();
    if ( rViewData.HasEditView( rViewData.GetActivePart() ) &&
            nPosX >= rViewData.GetEditStartCol() && nPosX <= rViewData.GetEditEndCol() &&
            nPosY >= rViewData.GetEditStartRow() && nPosY <= rViewData.GetEditEndRow() )
    {
        // insert into the cell which just got edited

        OUString aTargetFrame;
        if (pTarget)
            aTargetFrame = *pTarget;
        rViewData.GetViewShell()->InsertURLField( rDescription, rURL, aTargetFrame );
        return;
    }

    // insert into not edited cell

    ScDocument* pDoc = GetViewData().GetDocument();
    SCTAB nTab = GetViewData().GetTabNo();
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
    sal_Int32 nTxtLen = aEngine.GetTextLen(nPara);
    ESelection aInsSel( nPara, nTxtLen, nPara, nTxtLen );

    if ( bTryReplace && HasBookmarkAtCursor( nullptr ) )
    {
        //  if called from hyperlink slot and cell contains only a URL,
        //  replace old URL with new one

        aInsSel = ESelection( 0, 0, 0, 1 );     // replace first character (field)
    }

    SvxURLField aField( rURL, rDescription, SVXURLFORMAT_APPDEFAULT );
    if (pTarget)
        aField.SetTargetFrame(*pTarget);
    aEngine.QuickInsertField( SvxFieldItem( aField, EE_FEATURE_FIELD ), aInsSel );

    std::unique_ptr<EditTextObject> pData(aEngine.CreateTextObject());
    EnterData(nPosX, nPosY, nTab, *pData);
}

bool ScViewFunc::HasBookmarkAtCursor( SvxHyperlinkItem* pContent )
{
    ScAddress aPos( GetViewData().GetCurX(), GetViewData().GetCurY(), GetViewData().GetTabNo() );
    ScDocument& rDoc = GetViewData().GetDocShell()->GetDocument();

    const EditTextObject* pData = rDoc.GetEditText(aPos);
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

    if (pField->GetClassId() != css::text::textfield::Type::URL)
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
