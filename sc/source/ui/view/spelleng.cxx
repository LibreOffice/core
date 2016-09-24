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

#include "spelleng.hxx"
#include <com/sun/star/i18n/TextConversionOption.hpp>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/langitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include "spelldialog.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "cellvalue.hxx"
#include "cellform.hxx"
#include "formulacell.hxx"
#include "patattr.hxx"
#include "waitoff.hxx"
#include "globstr.hrc"
#include "markdata.hxx"

#include <memory>

using namespace ::com::sun::star;

ScConversionEngineBase::ScConversionEngineBase(
        SfxItemPool* pEnginePoolP, ScViewData& rViewData,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc ) :
    ScEditEngineDefaulter( pEnginePoolP ),
    mrViewData( rViewData ),
    mrDocShell( *rViewData.GetDocShell() ),
    mrDoc( rViewData.GetDocShell()->GetDocument() ),
    maSelState( rViewData ),
    mpUndoDoc( pUndoDoc ),
    mpRedoDoc( pRedoDoc ),
    meCurrLang( LANGUAGE_ENGLISH_US ),
    mbIsAnyModified( false ),
    mbInitialState( true ),
    mbWrappedInTable( false ),
    mbFinished( false )
{
    maSelState.GetCellCursor().GetVars( mnStartCol, mnStartRow, mnStartTab );
    // start with cell A1 in cell/range/multi-selection, will seek to first selected
    if( maSelState.GetSelectionType() == SC_SELECTTYPE_SHEET )
    {
        mnStartCol = 0;
        mnStartRow = 0;
    }
    mnCurrCol = mnStartCol;
    mnCurrRow = mnStartRow;
}

ScConversionEngineBase::~ScConversionEngineBase()
{
}

bool ScConversionEngineBase::FindNextConversionCell()
{
    ScMarkData& rMark = mrViewData.GetMarkData();
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    const ScPatternAttr* pPattern = nullptr;
    const ScPatternAttr* pLastPattern = nullptr;

    std::unique_ptr<SfxItemSet> pEditDefaults(new SfxItemSet(GetEmptyItemSet()));

    if( IsModified() )
    {
        mbIsAnyModified = true;

        OUString aNewStr = GetText();

        bool bMultiTab = (rMark.GetSelectCount() > 1);
        OUString aVisibleStr;
        if( bMultiTab )
            aVisibleStr = mrDoc.GetString(mnCurrCol, mnCurrRow, mnStartTab);

        for( SCTAB nTab = 0, nTabCount = mrDoc.GetTableCount(); nTab < nTabCount; ++nTab )
        {
            //  always change the cell on the visible tab,
            //  on the other selected tabs only if they contain the same text

            if ((nTab == mnStartTab) ||
                (bMultiTab && rMark.GetTableSelect(nTab) && mrDoc.GetString(mnCurrCol, mnCurrRow, nTab) == aVisibleStr))
            {
                ScAddress aPos( mnCurrCol, mnCurrRow, nTab );
                CellType eCellType = mrDoc.GetCellType( aPos );
                bool bEmptyCell = eCellType == CELLTYPE_NONE;

                if (mpUndoDoc && !bEmptyCell)
                    mrDoc.CopyCellToDocument(aPos, aPos, *mpUndoDoc);

                if (eCellType == CELLTYPE_EDIT)
                {
                    std::unique_ptr<EditTextObject> pEditObj(CreateTextObject());
                    mrDoc.SetEditText(aPos, *pEditObj, GetEditTextObjectPool());
                }
                else
                    mrDoc.SetString(aPos, aNewStr);

                if (mpRedoDoc && !bEmptyCell)
                    mrDoc.CopyCellToDocument(aPos, aPos, *mpRedoDoc);

                mrDocShell.PostPaintCell(aPos);
            }
        }
    }

    SCCOL nNewCol = mnCurrCol;
    SCROW nNewRow = mnCurrRow;

    if( mbInitialState )
    {
        /*  On very first call, decrement row to let GetNextSpellingCell() find
            the first cell of current range. */
        mbInitialState = false;
        --nNewRow;
    }

    bool bSheetSel = maSelState.GetSelectionType() == SC_SELECTTYPE_SHEET;
    bool bLoop = true;
    bool bFound = false;
    while( bLoop && !bFound )
    {
        bLoop = mrDoc.GetNextSpellingCell( nNewCol, nNewRow, mnStartTab, bSheetSel, rMark );
        if( bLoop )
        {
            FillFromCell( mnCurrCol, mnCurrRow, mnStartTab );

            if( mbWrappedInTable && ((nNewCol > mnStartCol) || ((nNewCol == mnStartCol) && (nNewRow >= mnStartRow))) )
            {
                ShowFinishDialog();
                bLoop = false;
                mbFinished = true;
            }
            else if( nNewCol > MAXCOL )
            {
                // no more cells in the sheet - try to restart at top of sheet

                if( bSheetSel || ((mnStartCol == 0) && (mnStartRow == 0)) )
                {
                    // conversion started at cell A1 or in selection, do not query to restart at top
                    ShowFinishDialog();
                    bLoop = false;
                    mbFinished = true;
                }
                else if( ShowTableWrapDialog() )
                {
                    // conversion started anywhere but in cell A1, user wants to restart
                    nNewRow = MAXROW + 2;
                    mbWrappedInTable = true;
                }
                else
                {
                    bLoop = false;
                    mbFinished = true;
                }
            }
            else
            {
                pPattern = mrDoc.GetPattern( nNewCol, nNewRow, mnStartTab );
                if( pPattern && (pPattern != pLastPattern) )
                {
                    pPattern->FillEditItemSet( pEditDefaults.get() );
                    SetDefaults( *pEditDefaults );
                    pLastPattern = pPattern;
                }

                // language changed?
                const SfxPoolItem* pItem = mrDoc.GetAttr( nNewCol, nNewRow, mnStartTab, ATTR_FONT_LANGUAGE );
                if( const SvxLanguageItem* pLangItem = dynamic_cast<const SvxLanguageItem*>( pItem )  )
                {
                    LanguageType eLang = static_cast< LanguageType >( pLangItem->GetValue() );
                    if( eLang == LANGUAGE_SYSTEM )
                        eLang = Application::GetSettings().GetLanguageTag().getLanguageType();   // never use SYSTEM for spelling
                    if( eLang != meCurrLang )
                    {
                        meCurrLang = eLang;
                        SetDefaultLanguage( eLang );
                    }
                }

                FillFromCell( nNewCol, nNewRow, mnStartTab );

                bFound = bLoop && NeedsConversion();
            }
        }
    }

    if( bFound )
    {
        pViewShell->AlignToCursor( nNewCol, nNewRow, SC_FOLLOW_JUMP );
        pViewShell->SetCursor( nNewCol, nNewRow, true );
        mrViewData.GetView()->MakeEditView( this, nNewCol, nNewRow );
        EditView* pEditView = mrViewData.GetSpellingView();
        // maSelState.GetEditSelection() returns (0,0) if not in edit mode -> ok
        pEditView->SetSelection( maSelState.GetEditSelection() );

        ClearModifyFlag();
        mnCurrCol = nNewCol;
        mnCurrRow = nNewRow;
    }

    return bFound;
}

void ScConversionEngineBase::RestoreCursorPos()
{
    const ScAddress& rPos = maSelState.GetCellCursor();
    mrViewData.GetViewShell()->SetCursor( rPos.Col(), rPos.Row() );
}

bool ScConversionEngineBase::ShowTableWrapDialog()
{
    // default: no dialog, always restart at top
    return true;
}

void ScConversionEngineBase::ShowFinishDialog()
{
    // default: no dialog
}

// private --------------------------------------------------------------------

void ScConversionEngineBase::FillFromCell( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScAddress aPos(nCol, nRow, nTab);

    ScRefCellValue aCell(mrDoc, aPos);
    switch (aCell.meType)
    {
        case CELLTYPE_STRING:
        {
            SvNumberFormatter* pFormatter = mrDoc.GetFormatTable();
            sal_uLong nNumFmt = mrDoc.GetNumberFormat(aPos);
            OUString aText;
            Color* pColor;
            ScCellFormat::GetString(aCell, nNumFmt, aText, &pColor, *pFormatter, &mrDoc);

            SetText(aText);
        }
        break;
        case CELLTYPE_EDIT:
        {
            const EditTextObject* pNewEditObj = aCell.mpEditText;
            SetText(*pNewEditObj);
        }
        break;
        default:
            SetText(EMPTY_OUSTRING);
    }
}

ScSpellingEngine::ScSpellingEngine(
        SfxItemPool* pEnginePoolP, ScViewData& rViewData,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc,
        css::uno::Reference< css::linguistic2::XSpellChecker1 > const & xSpeller ) :
    ScConversionEngineBase( pEnginePoolP, rViewData, pUndoDoc, pRedoDoc )
{
    SetSpeller( xSpeller );
}

void ScSpellingEngine::ConvertAll( EditView& rEditView )
{
    EESpellState eState = EE_SPELL_OK;
    if( FindNextConversionCell() )
        eState = rEditView.StartSpeller( true );

    OSL_ENSURE( eState != EE_SPELL_NOSPELLER, "ScSpellingEngine::Convert - no spell checker" );
    if( eState == EE_SPELL_NOLANGUAGE )
    {
        vcl::Window* pParent = GetDialogParent();
        ScWaitCursorOff aWaitOff( pParent );
        ScopedVclPtrInstance<InfoBox>( pParent, ScGlobal::GetRscString( STR_NOLANGERR ) )->Execute();
    }
}

bool ScSpellingEngine::SpellNextDocument()
{
    return FindNextConversionCell();
}

bool ScSpellingEngine::NeedsConversion()
{
    return HasSpellErrors() != EE_SPELL_OK;
}

bool ScSpellingEngine::ShowTableWrapDialog()
{
    vcl::Window* pParent = GetDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    ScopedVclPtrInstance<MessBox> aMsgBox( pParent, WinBits( WB_YES_NO | WB_DEF_YES ),
        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
        ScGlobal::GetRscString( STR_SPELLING_BEGIN_TAB) );
    return aMsgBox->Execute() == RET_YES;
}

void ScSpellingEngine::ShowFinishDialog()
{
    vcl::Window* pParent = GetDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    ScopedVclPtrInstance<InfoBox>( pParent, ScGlobal::GetRscString( STR_SPELLING_STOP_OK ) )->Execute();
}

vcl::Window* ScSpellingEngine::GetDialogParent()
{
    sal_uInt16 nWinId = ScSpellDialogChildWindow::GetChildWindowId();
    SfxViewFrame* pViewFrm = mrViewData.GetViewShell()->GetViewFrame();
    if( pViewFrm->HasChildWindow( nWinId ) )
        if( SfxChildWindow* pChild = pViewFrm->GetChildWindow( nWinId ) )
            if( vcl::Window* pWin = pChild->GetWindow() )
                if( pWin->IsVisible() )
                    return pWin;

    // fall back to standard dialog parent
    return ScDocShell::GetActiveDialogParent();
}

ScConversionParam::ScConversionParam( ScConversionType eConvType ) :
    meConvType( eConvType ),
    meSourceLang( LANGUAGE_NONE ),
    meTargetLang( LANGUAGE_NONE ),
    mnOptions( 0 ),
    mbUseTargetFont( false ),
    mbIsInteractive( false )
{
}

ScConversionParam::ScConversionParam( ScConversionType eConvType,
        LanguageType eLang, sal_Int32 nOptions, bool bIsInteractive ) :
    meConvType( eConvType ),
    meSourceLang( eLang ),
    meTargetLang( eLang ),
    mnOptions( nOptions ),
    mbUseTargetFont( false ),
    mbIsInteractive( bIsInteractive )
{
    if (LANGUAGE_KOREAN == eLang)
        mnOptions = i18n::TextConversionOption::CHARACTER_BY_CHARACTER;
}

ScConversionParam::ScConversionParam( ScConversionType eConvType,
        LanguageType eSourceLang, LanguageType eTargetLang, const vcl::Font& rTargetFont,
        sal_Int32 nOptions, bool bIsInteractive ) :
    meConvType( eConvType ),
    meSourceLang( eSourceLang ),
    meTargetLang( eTargetLang ),
    maTargetFont( rTargetFont ),
    mnOptions( nOptions ),
    mbUseTargetFont( true ),
    mbIsInteractive( bIsInteractive )
{
    if (LANGUAGE_KOREAN == meSourceLang && LANGUAGE_KOREAN == meTargetLang)
        mnOptions = i18n::TextConversionOption::CHARACTER_BY_CHARACTER;
}

ScTextConversionEngine::ScTextConversionEngine(
        SfxItemPool* pEnginePoolP, ScViewData& rViewData,
        const ScConversionParam& rConvParam,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc ) :
    ScConversionEngineBase( pEnginePoolP, rViewData, pUndoDoc, pRedoDoc ),
    maConvParam( rConvParam )
{
}

void ScTextConversionEngine::ConvertAll( EditView& rEditView )
{
    if( FindNextConversionCell() )
    {
        rEditView.StartTextConversion(
            maConvParam.GetSourceLang(), maConvParam.GetTargetLang(), maConvParam.GetTargetFont(),
            maConvParam.GetOptions(), maConvParam.IsInteractive(), true );
        // #i34769# restore initial cursor position
        RestoreCursorPos();
    }
}

bool ScTextConversionEngine::ConvertNextDocument()
{
    return FindNextConversionCell();
}

bool ScTextConversionEngine::NeedsConversion()
{
    return HasConvertibleTextPortion( maConvParam.GetSourceLang() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
