/*************************************************************************
 *
 *  $RCSfile: spelleng.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 13:40:55 $
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

#include <memory>

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/langitem.hxx>
#include <svx/editobj.hxx>
#include <svx/editview.hxx>
#include <vcl/msgbox.hxx>
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "spelleng.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "waitoff.hxx"
#include "globstr.hrc"

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
        SfxItemPool* pEnginePool, ScViewData& rViewData,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc,
        ESelection* pEdSelection,
        SCCOL nCol, SCROW nRow, SCTAB nTab,
        bool bCellSelection ) :
    ScEditEngineDefaulter( pEnginePool ),
    mrViewData( rViewData ),
    mrDocShell( *rViewData.GetDocShell() ),
    mrDoc( *rViewData.GetDocShell()->GetDocument() ),
    mpUndoDoc( pUndoDoc ),
    mpRedoDoc( pRedoDoc ),
    mpEditSel( pEdSelection ),
    meCurrLang( LANGUAGE_ENGLISH_US ),
    mnStartCol( nCol ),
    mnStartRow( nRow ),
    mnStartTab( nTab ),
    mnCurrCol( nCol ),
    mnCurrRow( nRow ),
    mbCellSelect( bCellSelection ),
    mbIsAnyModified( false ),
    mbInitialState( true ),
    mbWrappedInTable( false )
{
}

ScConversionEngineBase::~ScConversionEngineBase()
{
}

bool ScConversionEngineBase::FindNextConversionCell()
{
    ScMarkData& rMark = mrViewData.GetMarkData();
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    ScSplitPos eWhich = mrViewData.GetActivePart();
    ScBaseCell* pCell = NULL;
    const ScPatternAttr* pPattern = NULL;
    const ScPatternAttr* pLastPattern = NULL;
    ::std::auto_ptr< SfxItemSet > pEditDefaults( new SfxItemSet( GetEmptyItemSet() ) );

    if( IsModified() )
    {
        mbIsAnyModified = true;

        String aNewStr = GetText();

        BOOL bMultiTab = (rMark.GetSelectCount() > 1);
        String aVisibleStr;
        if( bMultiTab )
            mrDoc.GetString( mnCurrCol, mnCurrRow, mnStartTab, aVisibleStr );

        for( SCTAB nTab = 0, nTabCount = mrDoc.GetTableCount(); nTab < nTabCount; ++nTab )
        {
            //  #69965# always change the cell on the visible tab,
            //  on the other selected tabs only if they contain the same text

            if( (nTab == mnStartTab) ||
                (bMultiTab && rMark.GetTableSelect( nTab ) &&
                 lclHasString( mrDoc, mnCurrCol, mnCurrRow, nTab, aVisibleStr )) )
            {
                CellType eCellType;
                mrDoc.GetCellType( mnCurrCol, mnCurrRow, nTab, eCellType );
                mrDoc.GetCell( mnCurrCol, mnCurrRow, nTab, pCell );

                if( mpUndoDoc && pCell )
                {
                    ScBaseCell* pUndoCell = pCell->Clone( mpUndoDoc );
                    mpUndoDoc->PutCell( mnCurrCol, mnCurrRow, nTab, pUndoCell );
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
                    mrDoc.GetCell( mnCurrCol, mnCurrRow, nTab, pCell );
                }

                if( mpRedoDoc && pCell )
                {
                    ScBaseCell* pRedoCell = pCell->Clone( mpRedoDoc );
                    mpRedoDoc->PutCell( mnCurrCol, mnCurrRow, nTab, pRedoCell );
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

    bool bLoop = true;
    bool bFound = false;
    while( bLoop && !bFound )
    {
        bLoop = mrDoc.GetNextSpellingCell( nNewCol, nNewRow, mnStartTab, mbCellSelect, rMark );
        if( bLoop )
        {
            FillFromCell( mnCurrCol, mnCurrRow, mnStartTab );

            if( mbWrappedInTable && ((nNewCol > mnStartCol) || ((nNewCol == mnStartCol) && (nNewRow >= mnStartRow))) )
            {
                ShowFinishDialog();
                bLoop = false;
            }
            else if( nNewCol > MAXCOL )
            {
                // no more cells in the sheet - try to restart at top of sheet

                if( mbCellSelect || ((mnStartCol == 0) && (mnStartRow == 0)) )
                {
                    // conversion started at cell A1 or in selection, do not query to restart at top
                    ShowFinishDialog();
                    bLoop = false;
                }
                else if( ShowTableWrapDialog() )
                {
                    // conversion started anywhere but in cell A1, user wants to restart
                    nNewRow = MAXROW + 2;
                    mbWrappedInTable = true;
                }
                else
                    bLoop = false;
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
        pViewShell->SetCursor( nNewCol, nNewRow, TRUE );
        mrViewData.GetView()->MakeEditView( this, nNewCol, nNewRow );
        EditView* pEditView = mrViewData.GetSpellingView();
        if( mpEditSel )
        {
            pEditView->SetSelection( *mpEditSel );
            mpEditSel = NULL;
        }
        else
        {
            ESelection aSel;
            pEditView->SetSelection( aSel );
        }

        ClearModifyFlag();
        mnCurrCol = nNewCol;
        mnCurrRow = nNewRow;
    }

    return bFound;
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
        SfxItemPool* pEnginePool, ScViewData& rViewData,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc,
        ESelection* pEdSelection,
        SCCOL nCol, SCROW nRow, SCTAB nTab,
        bool bCellSelection, XSpellCheckerRef xSpeller ) :
    ScConversionEngineBase( pEnginePool, rViewData, pUndoDoc, pRedoDoc, pEdSelection, nCol, nRow, nTab, bCellSelection )
{
    SetSpeller( xSpeller );
}

void ScSpellingEngine::ConvertAll( EditView& rEditView )
{
    EESpellState eState = EE_SPELL_OK;
    if( FindNextConversionCell() )
        eState = rEditView.StartSpeller( static_cast< BOOL >( TRUE ) );

    DBG_ASSERT( eState != EE_SPELL_NOSPELLER, "ScSpellingEngine::Convert - no spell checker" );
    if( eState == EE_SPELL_NOLANGUAGE )
    {
        ScWaitCursorOff aWaitOff( mrDocShell.GetDialogParent() );
        InfoBox( mrViewData.GetDialogParent(), ScGlobal::GetRscString( STR_NOLANGERR ) ).Execute();
    }
}

BOOL ScSpellingEngine::SpellNextDocument()
{
    return FindNextConversionCell();
}

bool ScSpellingEngine::NeedsConversion()
{
    return HasSpellErrors() != EE_SPELL_OK;
}

bool ScSpellingEngine::ShowTableWrapDialog()
{
    ScWaitCursorOff aWaitOff( mrDocShell.GetDialogParent() );
    MessBox aMsgBox( mrViewData.GetDialogParent(), WinBits( WB_YES_NO | WB_DEF_YES ),
        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
        ScGlobal::GetRscString( STR_SPELLING_BEGIN_TAB) );
    return aMsgBox.Execute() == RET_YES;
}

void ScSpellingEngine::ShowFinishDialog()
{
    ScWaitCursorOff aWaitOff( mrDocShell.GetDialogParent() );
    InfoBox( mrViewData.GetDialogParent(), ScGlobal::GetRscString( STR_SPELLING_STOP_OK ) ).Execute();
}

// ============================================================================

ScTextConversionEngine::ScTextConversionEngine(
        SfxItemPool* pEnginePool, ScViewData& rViewData,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc,
        ESelection* pEdSelection,
        SCCOL nCol, SCROW nRow, SCTAB nTab,
        bool bCellSelection, LanguageType eConvLanguage ) :
    ScConversionEngineBase( pEnginePool, rViewData, pUndoDoc, pRedoDoc, pEdSelection, nCol, nRow, nTab, bCellSelection ),
    meConvLang( eConvLanguage )
{
}

void ScTextConversionEngine::ConvertAll( EditView& rEditView )
{
    if( FindNextConversionCell() )
        rEditView.StartTextConversion( meConvLang, TRUE );
}

BOOL ScTextConversionEngine::ConvertNextDocument()
{
    return FindNextConversionCell();
}

bool ScTextConversionEngine::NeedsConversion()
{
    return HasConvertibleTextPortion( meConvLang );
}

// ============================================================================

