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

#include <spelleng.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>

#include <scitems.hxx>

#include <editeng/langitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <utility>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

#include <spelldialog.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <cellvalue.hxx>
#include <cellform.hxx>
#include <patattr.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <markdata.hxx>
#include <docpool.hxx>

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

    SfxItemSet aEditDefaults(GetEmptyItemSet());

    if( IsModified() )
    {
        mbIsAnyModified = true;

        OUString aNewStr = GetText();

        // Check if the user has changed the language. If the new language is
        // applied to the entire string length, we will set the language as cell
        // attribute. Otherwise we will commit this as an edit-engine string.
        editeng::LanguageSpan aLang = GetLanguage(0, 0);

        bool bSimpleString = GetParagraphCount() == 1 &&
            aLang.nLang != LANGUAGE_DONTKNOW &&
            aLang.nStart == 0 &&
            aLang.nEnd == aNewStr.getLength();

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

                if (!bSimpleString || eCellType == CELLTYPE_EDIT)
                {
                    std::unique_ptr<EditTextObject> pEditObj(CreateTextObject());
                    mrDoc.SetEditText(aPos, *pEditObj, GetEditTextObjectPool());
                }
                else
                {
                    // Set the new string and update the language with the cell.
                    mrDoc.SetString(aPos, aNewStr);

                    const ScPatternAttr* pAttr(mrDoc.GetPattern(aPos));
                    ScPatternAttr* pNewAttr(nullptr);

                    if (nullptr != pAttr)
                        pNewAttr = new ScPatternAttr(*pAttr);
                    else
                        pNewAttr = new ScPatternAttr(mrDoc.getCellAttributeHelper());

                    pNewAttr->GetItemSet().Put(SvxLanguageItem(aLang.nLang, ATTR_FONT_LANGUAGE));
                    mrDoc.SetPattern(aPos, CellAttributeHolder(pNewAttr, true));
                }

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
            else if( nNewCol >= mrDoc.GetAllocatedColumnsCount(mnStartTab) )
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
                    nNewRow = mrDoc.MaxRow() + 2;
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
                // GetPattern may implicitly allocates the column if not exists,
                pPattern = mrDoc.GetPattern( nNewCol, nNewRow, mnStartTab );
                if( pPattern && !ScPatternAttr::areSame(pPattern, pLastPattern) )
                {
                    pPattern->FillEditItemSet( &aEditDefaults );
                    SetDefaults( aEditDefaults );
                    pLastPattern = pPattern;
                }

                // language changed?
                const SfxPoolItem* pItem = mrDoc.GetAttr( nNewCol, nNewRow, mnStartTab, ATTR_FONT_LANGUAGE );
                if( const SvxLanguageItem* pLangItem = dynamic_cast<const SvxLanguageItem*>( pItem )  )
                {
                    LanguageType eLang = pLangItem->GetValue();
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
    switch (aCell.getType())
    {
        case CELLTYPE_STRING:
        {
            sal_uInt32 nNumFmt = mrDoc.GetNumberFormat(aPos);
            const Color* pColor;
            OUString aText = ScCellFormat::GetString(aCell, nNumFmt, &pColor, nullptr, mrDoc);

            SetTextCurrentDefaults(aText);
        }
        break;
        case CELLTYPE_EDIT:
        {
            const EditTextObject* pNewEditObj = aCell.getEditText();
            SetTextCurrentDefaults(*pNewEditObj);
        }
        break;
        default:
            SetTextCurrentDefaults(OUString());
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

void ScSpellingEngine::ConvertAll(weld::Widget* pDialogParent, EditView& rEditView)
{
    EESpellState eState = EESpellState::Ok;
    if( FindNextConversionCell() )
        eState = rEditView.StartSpeller(pDialogParent, true);

    OSL_ENSURE( eState != EESpellState::NoSpeller, "ScSpellingEngine::Convert - no spell checker" );
}

bool ScSpellingEngine::SpellNextDocument()
{
    return FindNextConversionCell();
}

bool ScSpellingEngine::NeedsConversion()
{
    return HasSpellErrors() != EESpellState::Ok;
}

bool ScSpellingEngine::ShowTableWrapDialog()
{
    weld::Widget* pParent = GetDialogParent();
    weld::WaitObject aWaitOff(pParent);

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                              VclMessageType::Question, VclButtonsType::YesNo,
                                              ScResId(STR_SPELLING_BEGIN_TAB))); // "delete data?"
    xBox->set_title(ScResId(STR_MSSG_DOSUBTOTALS_0));
    xBox->set_default_response(RET_YES);
    return xBox->run() == RET_YES;
}

void ScSpellingEngine::ShowFinishDialog()
{
    weld::Widget* pParent = GetDialogParent();
    weld::WaitObject aWaitOff(pParent);
    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                  ScResId(STR_SPELLING_STOP_OK)));
    xInfoBox->run();
}

weld::Widget* ScSpellingEngine::GetDialogParent()
{
    sal_uInt16 nWinId = ScSpellDialogChildWindow::GetChildWindowId();
    SfxViewFrame& rViewFrm = mrViewData.GetViewShell()->GetViewFrame();
    if( rViewFrm.HasChildWindow( nWinId ) )
    {
        if( SfxChildWindow* pChild = rViewFrm.GetChildWindow( nWinId ) )
        {
            auto xController = pChild->GetController();
            if (xController)
            {
                if (weld::Window *pRet = xController->getDialog())
                {
                    if (pRet->get_visible())
                        return pRet;
                }
            }
        }
    }

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
        LanguageType eSourceLang, LanguageType eTargetLang, vcl::Font aTargetFont,
        sal_Int32 nOptions, bool bIsInteractive ) :
    meConvType( eConvType ),
    meSourceLang( eSourceLang ),
    meTargetLang( eTargetLang ),
    maTargetFont(std::move( aTargetFont )),
    mnOptions( nOptions ),
    mbUseTargetFont( true ),
    mbIsInteractive( bIsInteractive )
{
    if (LANGUAGE_KOREAN == meSourceLang && LANGUAGE_KOREAN == meTargetLang)
        mnOptions = i18n::TextConversionOption::CHARACTER_BY_CHARACTER;
}

ScTextConversionEngine::ScTextConversionEngine(
        SfxItemPool* pEnginePoolP, ScViewData& rViewData,
        ScConversionParam aConvParam,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc ) :
    ScConversionEngineBase( pEnginePoolP, rViewData, pUndoDoc, pRedoDoc ),
    maConvParam(std::move( aConvParam ))
{
}

void ScTextConversionEngine::ConvertAll(weld::Widget* pDialogParent, EditView& rEditView)
{
    if( FindNextConversionCell() )
    {
        rEditView.StartTextConversion(pDialogParent,
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
