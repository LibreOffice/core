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
#include "spelleng.hxx"
#include <com/sun/star/i18n/TextConversionOption.hpp>

#include <memory>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include <editeng/langitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include "spelldialog.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "waitoff.hxx"
#include "globstr.hrc"


using namespace ::com::sun::star;

// ============================================================================

namespace {

bool lclHasString( ScDocument& rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString )
{
    String aCompStr;
    rDoc.GetString( nCol, nRow, nTab, aCompStr );
    return aCompStr == rString;      //! case-insensitive?
}

} // namespace

// ----------------------------------------------------------------------------

ScConversionEngineBase::ScConversionEngineBase(
        SfxItemPool* pEnginePoolP, ScViewData& rViewData,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc ) :
    ScEditEngineDefaulter( pEnginePoolP ),
    mrViewData( rViewData ),
    mrDocShell( *rViewData.GetDocShell() ),
    mrDoc( *rViewData.GetDocShell()->GetDocument() ),
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
    ScBaseCell* pCell = NULL;
    const ScPatternAttr* pPattern = NULL;
    const ScPatternAttr* pLastPattern = NULL;
    ::std::auto_ptr< SfxItemSet > pEditDefaults( new SfxItemSet( GetEmptyItemSet() ) );

    if( IsModified() )
    {
        mbIsAnyModified = true;

        String aNewStr = GetText();

        sal_Bool bMultiTab = (rMark.GetSelectCount() > 1);
        String aVisibleStr;
        if( bMultiTab )
            mrDoc.GetString( mnCurrCol, mnCurrRow, mnStartTab, aVisibleStr );

        for( SCTAB nTab = 0, nTabCount = mrDoc.GetTableCount(); nTab < nTabCount; ++nTab )
        {
            //  always change the cell on the visible tab,
            //  on the other selected tabs only if they contain the same text

            if( (nTab == mnStartTab) ||
                (bMultiTab && rMark.GetTableSelect( nTab ) &&
                 lclHasString( mrDoc, mnCurrCol, mnCurrRow, nTab, aVisibleStr )) )
            {
                ScAddress aPos( mnCurrCol, mnCurrRow, nTab );
                CellType eCellType = mrDoc.GetCellType( aPos );
                pCell = mrDoc.GetCell( aPos );

                if( mpUndoDoc && pCell )
                {
                    ScBaseCell* pUndoCell = pCell->CloneWithoutNote( *mpUndoDoc );
                    mpUndoDoc->PutCell( aPos, pUndoCell );
                }

                if( eCellType == CELLTYPE_EDIT )
                {
                    if( pCell )
                    {
                        ScEditCell* pEditCell = static_cast< ScEditCell* >( pCell );
                        ::std::auto_ptr< EditTextObject > pEditObj( CreateTextObject() );
                        pEditCell->SetData( pEditObj.get(), GetEditTextObjectPool() );
                    }
                }
                else
                {
                    mrDoc.SetString( mnCurrCol, mnCurrRow, nTab, aNewStr );
                    pCell = mrDoc.GetCell( aPos );
                }

                if( mpRedoDoc && pCell )
                {
                    ScBaseCell* pRedoCell = pCell->CloneWithoutNote( *mpRedoDoc );
                    mpRedoDoc->PutCell( aPos, pRedoCell );
                }

                mrDocShell.PostPaintCell( mnCurrCol, mnCurrRow, nTab );
            }
        }
    }
    pCell = NULL;
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
                if( const SvxLanguageItem* pLangItem = PTR_CAST( SvxLanguageItem, pItem ) )
                {
                    LanguageType eLang = static_cast< LanguageType >( pLangItem->GetValue() );
                    if( eLang == LANGUAGE_SYSTEM )
                        eLang = Application::GetSettings().GetLanguage();   // never use SYSTEM for spelling
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
        pViewShell->SetCursor( nNewCol, nNewRow, sal_True );
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
    CellType eCellType;
    mrDoc.GetCellType( nCol, nRow, nTab, eCellType );

    switch( eCellType )
    {
        case CELLTYPE_STRING:
        {
            String aText;
            mrDoc.GetString( nCol, nRow, nTab, aText );
            SetText( aText );
        }
        break;
        case CELLTYPE_EDIT:
        {
            ScBaseCell* pCell = NULL;
            mrDoc.GetCell( nCol, nRow, nTab, pCell );
            if( pCell )
            {
                const EditTextObject* pNewEditObj = NULL;
                static_cast< ScEditCell* >( pCell )->GetData( pNewEditObj );
                if( pNewEditObj )
                    SetText( *pNewEditObj );
            }
        }
        break;
        default:
            SetText( EMPTY_STRING );
    }
}

// ============================================================================

ScSpellingEngine::ScSpellingEngine(
        SfxItemPool* pEnginePoolP, ScViewData& rViewData,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc,
        XSpellCheckerRef xSpeller ) :
    ScConversionEngineBase( pEnginePoolP, rViewData, pUndoDoc, pRedoDoc )
{
    SetSpeller( xSpeller );
}

void ScSpellingEngine::ConvertAll( EditView& rEditView )
{
    EESpellState eState = EE_SPELL_OK;
    if( FindNextConversionCell() )
        eState = rEditView.StartSpeller( static_cast< sal_Bool >( sal_True ) );

    DBG_ASSERT( eState != EE_SPELL_NOSPELLER, "ScSpellingEngine::Convert - no spell checker" );
    if( eState == EE_SPELL_NOLANGUAGE )
    {
        Window* pParent = GetDialogParent();
        ScWaitCursorOff aWaitOff( pParent );
        InfoBox( pParent, ScGlobal::GetRscString( STR_NOLANGERR ) ).Execute();
    }
}

sal_Bool ScSpellingEngine::SpellNextDocument()
{
    return FindNextConversionCell();
}

bool ScSpellingEngine::NeedsConversion()
{
    return HasSpellErrors() != EE_SPELL_OK;
}

bool ScSpellingEngine::ShowTableWrapDialog()
{
    Window* pParent = GetDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    MessBox aMsgBox( pParent, WinBits( WB_YES_NO | WB_DEF_YES ),
        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
        ScGlobal::GetRscString( STR_SPELLING_BEGIN_TAB) );
    return aMsgBox.Execute() == RET_YES;
}

void ScSpellingEngine::ShowFinishDialog()
{
    Window* pParent = GetDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    InfoBox( pParent, ScGlobal::GetRscString( STR_SPELLING_STOP_OK ) ).Execute();
}

Window* ScSpellingEngine::GetDialogParent()
{
    sal_uInt16 nWinId = ScSpellDialogChildWindow::GetChildWindowId();
    SfxViewFrame* pViewFrm = mrViewData.GetViewShell()->GetViewFrame();
    if( pViewFrm->HasChildWindow( nWinId ) )
        if( SfxChildWindow* pChild = pViewFrm->GetChildWindow( nWinId ) )
            if( Window* pWin = pChild->GetWindow() )
                if( pWin->IsVisible() )
                    return pWin;

    // fall back to standard dialog parent
    return mrDocShell.GetActiveDialogParent();
}

// ============================================================================

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
        LanguageType eSourceLang, LanguageType eTargetLang, const Font& rTargetFont,
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

// ----------------------------------------------------------------------------

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
            maConvParam.GetOptions(), maConvParam.IsInteractive(), sal_True );
        // #i34769# restore initial cursor position
        RestoreCursorPos();
    }
}

sal_Bool ScTextConversionEngine::ConvertNextDocument()
{
    return FindNextConversionCell();
}

bool ScTextConversionEngine::NeedsConversion()
{
    return HasConvertibleTextPortion( maConvParam.GetSourceLang() );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
