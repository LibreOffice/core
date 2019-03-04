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

#include <scitems.hxx>

#include <comphelper/lok.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/cliplistener.hxx>
#include <svtools/insdlg.hxx>
#include <sot/formats.hxx>
#include <svx/hlnkitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/EnumContext.hxx>
#include <svx/clipfmtitem.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
#include <editeng/langitem.hxx>

#include <cellsh.hxx>
#include <sc.hrc>
#include <docsh.hxx>
#include <attrib.hxx>
#include <tabvwsh.hxx>
#include <impex.hxx>
#include <formulacell.hxx>
#include <scmod.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <transobj.hxx>
#include <drwtrans.hxx>
#include <scabstdlg.hxx>
#include <dociter.hxx>
#include <postit.hxx>
#include <cliputil.hxx>
#include <clipparam.hxx>
#include <markdata.hxx>
#include <gridwin.hxx>

#define ShellClass_ScCellShell
#define ShellClass_CellMovement
#include <scslots.hxx>


SFX_IMPL_INTERFACE(ScCellShell, ScFormatShell)

void ScCellShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Objectbar_Format);

    GetStaticInterface()->RegisterPopupMenu("cell");
}

ScCellShell::ScCellShell(ScViewData* pData, const VclPtr<vcl::Window>& frameWin) :
    ScFormatShell(pData),
    pImpl( new CellShell_Impl() ),
    bPastePossible(false),
    pFrameWin(frameWin)
{
    SetName("Cell");
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Cell));
}

ScCellShell::~ScCellShell()
{
    if ( pImpl->m_xClipEvtLstnr.is() )
    {
        pImpl->m_xClipEvtLstnr->RemoveListener( GetViewData()->GetActiveWin() );

        //  The listener may just now be waiting for the SolarMutex and call the link
        //  afterwards, in spite of RemoveListener. So the link has to be reset, too.
        pImpl->m_xClipEvtLstnr->ClearCallbackLink();

        pImpl->m_xClipEvtLstnr.clear();
    }

    pImpl->m_pLinkedDlg.disposeAndClear();
    delete pImpl->m_pRequest;
}

void ScCellShell::GetBlockState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScRange aMarkRange;
    ScMarkType eMarkType = GetViewData()->GetSimpleArea( aMarkRange );
    bool bSimpleArea = (eMarkType == SC_MARK_SIMPLE);
    bool bOnlyNotBecauseOfMatrix;
    bool bEditable = pTabViewShell->SelectionEditable( &bOnlyNotBecauseOfMatrix );
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    nCol1 = aMarkRange.aStart.Col();
    nRow1 = aMarkRange.aStart.Row();
    nCol2 = aMarkRange.aEnd.Col();
    nRow2 = aMarkRange.aEnd.Row();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        bool bDisable = false;
        bool bNeedEdit = true;      // need selection be editable?
        switch ( nWhich )
        {
            case FID_FILL_TO_BOTTOM:    // fill to top / bottom
            {
                bDisable = !bSimpleArea || (nRow1 == 0 && nRow2 == 0);
                if (!bDisable && GetViewData()->SelectionForbidsCellFill())
                    bDisable = true;
                if ( !bDisable && bEditable )
                {   // do not damage matrix
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                        nCol1, nRow1, nCol2, nRow1, rMark );    // first row
                }
            }
            break;
            case FID_FILL_TO_TOP:
            {
                bDisable = (!bSimpleArea) || (nRow1 == MAXROW && nRow2 == MAXROW);
                if (!bDisable && GetViewData()->SelectionForbidsCellFill())
                    bDisable = true;
                if ( !bDisable && bEditable )
                {   // do not damage matrix
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                        nCol1, nRow2, nCol2, nRow2, rMark );    // last row
                }
            }
            break;
            case FID_FILL_TO_RIGHT:     // fill to left / right
            {
                bDisable = !bSimpleArea || (nCol1 == 0 && nCol2 == 0);
                if (!bDisable && GetViewData()->SelectionForbidsCellFill())
                    bDisable = true;
                if ( !bDisable && bEditable )
                {   // do not damage matrix
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol1, nRow2, rMark );    // first column
                }
            }
            break;
            case FID_FILL_TO_LEFT:
            {
                bDisable = (!bSimpleArea) || (nCol1 == MAXCOL && nCol2 == MAXCOL);
                if (!bDisable && GetViewData()->SelectionForbidsCellFill())
                    bDisable = true;
                if ( !bDisable && bEditable )
                {   // do not damage matrix
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                        nCol2, nRow1, nCol2, nRow2, rMark );    // last column
                }
            }
            break;

            case SID_RANDOM_NUMBER_GENERATOR_DIALOG:
                bDisable = !bSimpleArea || GetViewData()->SelectionForbidsCellFill();
            break;
            case SID_SAMPLING_DIALOG:
            case SID_DESCRIPTIVE_STATISTICS_DIALOG:
            case SID_ANALYSIS_OF_VARIANCE_DIALOG:
            case SID_CORRELATION_DIALOG:
            case SID_COVARIANCE_DIALOG:
            {
                bDisable = !bSimpleArea;
            }
            break;

            case FID_FILL_SERIES:       // fill block
            case SID_OPENDLG_TABOP:     // multiple-cell operations, are at least 2 cells marked?
                if (pDoc->GetChangeTrack()!=nullptr &&nWhich ==SID_OPENDLG_TABOP)
                    bDisable = true;
                else
                    bDisable = (!bSimpleArea) || (nCol1 == nCol2 && nRow1 == nRow2);

                if (!bDisable && GetViewData()->SelectionForbidsCellFill())
                    bDisable = true;

                if ( !bDisable && bEditable && nWhich == FID_FILL_SERIES )
                {   // do not damage matrix
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol2, nRow1, rMark )     // first row
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow2, nCol2, nRow2, rMark )     // last row
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol1, nRow2, rMark )     // first column
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol2, nRow1, nCol2, nRow2, rMark );    // last column
                }
                break;
            case FID_FILL_SINGLE_EDIT:
                bDisable = false;
                break;
            case SID_CUT:               // cut
            case FID_INS_CELL:          // insert cells, just simple selection
                bDisable = (!bSimpleArea);
                break;

            case SID_PASTE:
            case SID_PASTE_SPECIAL:
            case SID_PASTE_UNFORMATTED:
            case SID_PASTE_ONLY_VALUE:
            case SID_PASTE_ONLY_TEXT:
            case SID_PASTE_ONLY_FORMULA:
                bDisable = GetViewData()->SelectionForbidsCellFill();
                break;

            case FID_INS_ROW:
            case FID_INS_ROWS_BEFORE:           // insert rows
            case FID_INS_ROWS_AFTER:
            {
                sc::ColRowEditAction eAction = sc::ColRowEditAction::InsertRowsBefore;
                if (nWhich == FID_INS_ROWS_AFTER)
                    eAction = sc::ColRowEditAction::InsertRowsAfter;

                bDisable = (!bSimpleArea) || GetViewData()->SimpleColMarked();
                if (!bEditable && nCol1 == 0 && nCol2 == MAXCOL)
                {
                    // See if row insertions are allowed.
                    bEditable = pDoc->IsEditActionAllowed(eAction, rMark, nRow1, nRow2);
                }
                break;
            }
            case FID_INS_CELLSDOWN:
                bDisable = (!bSimpleArea) || GetViewData()->SimpleColMarked();
                break;

            case FID_INS_COLUMN:
            case FID_INS_COLUMNS_BEFORE:        // insert columns
            case FID_INS_COLUMNS_AFTER:
            {
                sc::ColRowEditAction eAction = sc::ColRowEditAction::InsertColumnsBefore;
                if (nWhich == FID_INS_COLUMNS_AFTER)
                    eAction = sc::ColRowEditAction::InsertColumnsAfter;

                bDisable = (!bSimpleArea) || GetViewData()->SimpleRowMarked();
                if (!bEditable && nRow1 == 0 && nRow2 == MAXROW)
                {
                    // See if row insertions are allowed.
                    bEditable = pDoc->IsEditActionAllowed(eAction, rMark, nCol1, nCol2);
                }
                break;
            }
            case FID_INS_CELLSRIGHT:
                bDisable = (!bSimpleArea) || GetViewData()->SimpleRowMarked();
                break;

            case SID_COPY:              // copy
                // not editable because of matrix only? Do not damage matrix
                //! is not called, when protected AND matrix, we will have
                //! to live with this... is caught in Copy-Routine, otherwise
                //! work is to be done once more
                if ( !(!bEditable && bOnlyNotBecauseOfMatrix) )
                    bNeedEdit = false;          // allowed when protected/ReadOnly
                break;

            case SID_AUTOFORMAT:        // Autoformat, at least 3x3 selected
                bDisable =    (!bSimpleArea)
                           || ((nCol2 - nCol1) < 2) || ((nRow2 - nRow1) < 2);
                break;

            case SID_CELL_FORMAT_RESET :
            case FID_CELL_FORMAT :
            case SID_ENABLE_HYPHENATION :
                // not editable because of matrix only? Attribute ok nonetheless
                if ( !bEditable && bOnlyNotBecauseOfMatrix )
                    bNeedEdit = false;
                break;

            case FID_VALIDATION:
                {
                    if ( pDocShell && pDocShell->IsDocShared() )
                    {
                        bDisable = true;
                    }
                }
                break;
            case SID_TRANSLITERATE_HALFWIDTH:
            case SID_TRANSLITERATE_FULLWIDTH:
            case SID_TRANSLITERATE_HIRAGANA:
            case SID_TRANSLITERATE_KATAKANA:
                ScViewUtil::HideDisabledSlot( rSet, GetViewData()->GetBindings(), nWhich );
            break;
            case SID_CONVERT_FORMULA_TO_VALUE:
            {
                // Check and see if the marked range has at least one formula cell.
                bDisable = !pDoc->HasFormulaCell(aMarkRange);
            }
            break;
        }
        if (!bDisable && bNeedEdit && !bEditable)
            bDisable = true;

        if (bDisable)
            rSet.DisableItem(nWhich);
        else if (nWhich == SID_ENABLE_HYPHENATION)
        {
            // toggle slots need a bool item
            rSet.Put( SfxBoolItem( nWhich, false ) );
        }
        nWhich = aIter.NextWhich();
    }
}

// functions, disabled depending on cursor position
// Default:
//     SID_INSERT_POSTIT, SID_CHARMAP, SID_OPENDLG_FUNCTION

void ScCellShell::GetCellState( SfxItemSet& rSet )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScDocument& rDoc = GetViewData()->GetDocShell()->GetDocument();
    ScAddress aCursor( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                        GetViewData()->GetTabNo() );
    bool isLOKNoTiledAnnotations = comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        bool bDisable = false;
        bool bNeedEdit = true;      // need cursor position be editable?
        switch ( nWhich )
        {
            case SID_THESAURUS:
                {
                    CellType eType = rDoc.GetCellType( aCursor );
                    bDisable = ( eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT);
                    if (!bDisable)
                    {
                        //  test for available languages
                        LanguageType nLang = ScViewUtil::GetEffLanguage( &rDoc, aCursor );
                        bDisable = !ScModule::HasThesaurusLanguage( nLang );
                    }
                }
                break;
            case SID_OPENDLG_FUNCTION:
                {
                    ScMarkData aMarkData = GetViewData()->GetMarkData();
                    aMarkData.MarkToSimple();
                    ScRange aRange;
                    aMarkData.GetMarkArea(aRange);
                    if(aMarkData.IsMarked())
                    {
                        if (!rDoc.IsBlockEditable( aCursor.Tab(), aRange.aStart.Col(),aRange.aStart.Row(),
                                            aRange.aEnd.Col(),aRange.aEnd.Row() ))
                        {
                            bDisable = true;
                        }
                        bNeedEdit=false;
                    }

                }
                break;
            case SID_INSERT_POSTIT:
                {
                    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                    if( rDoc.GetNote(aPos) )
                    {
                        bDisable = true;
                    }
                    else
                    {
                        bDisable = false;
                        if ( pDocShell && pDocShell->IsDocShared() )
                        {
                            bDisable = true;
                        }
                    }
                }
                break;
            case SID_EDIT_POSTIT:
                {
                    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                    // Allow editing annotation by Id (without selecting the cell) for LOK
                    bDisable = !(isLOKNoTiledAnnotations || rDoc.GetNote(aPos));
                }
                break;
        }
        if (!bDisable && bNeedEdit)
            if (!rDoc.IsBlockEditable( aCursor.Tab(), aCursor.Col(),aCursor.Row(),
                                        aCursor.Col(),aCursor.Row() ))
                bDisable = true;
        if (bDisable)
            rSet.DisableItem(nWhich);
        nWhich = aIter.NextWhich();
    }
}

static bool lcl_TestFormat( SvxClipboardFormatItem& rFormats, const TransferableDataHelper& rDataHelper,
                        SotClipboardFormatId nFormatId )
{
    if ( rDataHelper.HasFormat( nFormatId ) )
    {
        //  translated format name strings are no longer inserted here,
        //  handled by "paste special" dialog / toolbox controller instead.
        //  Only the object type name has to be set here:
        OUString aStrVal;
        if ( nFormatId == SotClipboardFormatId::EMBED_SOURCE )
        {
            TransferableObjectDescriptor aDesc;
            if ( const_cast<TransferableDataHelper&>(rDataHelper).GetTransferableObjectDescriptor(
                                        SotClipboardFormatId::OBJECTDESCRIPTOR, aDesc ) )
                aStrVal = aDesc.maTypeName;
        }
        else if ( nFormatId == SotClipboardFormatId::EMBED_SOURCE_OLE
          || nFormatId == SotClipboardFormatId::EMBEDDED_OBJ_OLE )
        {
            OUString aSource;
            SvPasteObjectHelper::GetEmbeddedName( rDataHelper, aStrVal, aSource, nFormatId );
        }

        if ( !aStrVal.isEmpty() )
            rFormats.AddClipbrdFormat( nFormatId, aStrVal );
        else
            rFormats.AddClipbrdFormat( nFormatId );

        return true;
    }

    return false;
}

void ScCellShell::GetPossibleClipboardFormats( SvxClipboardFormatItem& rFormats )
{
    vcl::Window* pWin = GetViewData()->GetActiveWin();
    bool bDraw = ScDrawTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(pWin)) != nullptr;

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );

    lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::DRAWING );
    lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::SVXB );
    lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::GDIMETAFILE );
    lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::PNG );
    lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::BITMAP );
    lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::EMBED_SOURCE );

    if ( !bDraw )
    {
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::LINK );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::STRING );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::STRING_TSVC );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::DIF );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::RTF );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::RICHTEXT );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::HTML );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::HTML_SIMPLE );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::BIFF_8 );
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::BIFF_5 );
    }

    if ( !lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::EMBED_SOURCE_OLE ) )
        lcl_TestFormat( rFormats, aDataHelper, SotClipboardFormatId::EMBEDDED_OBJ_OLE );
}

//  insert, insert contents

static bool lcl_IsCellPastePossible( const TransferableDataHelper& rData )
{
    bool bPossible = false;
    css::uno::Reference< css::datatransfer::XTransferable2 > xTransferable(rData.GetXTransferable(), css::uno::UNO_QUERY);
    if ( ScTransferObj::GetOwnClipboard(xTransferable) || ScDrawTransferObj::GetOwnClipboard(xTransferable) )
        bPossible = true;
    else
    {
        if ( rData.HasFormat( SotClipboardFormatId::PNG ) ||
             rData.HasFormat( SotClipboardFormatId::BITMAP ) ||
             rData.HasFormat( SotClipboardFormatId::GDIMETAFILE ) ||
             rData.HasFormat( SotClipboardFormatId::SVXB ) ||
             rData.HasFormat( SotClipboardFormatId::PRIVATE ) ||
             rData.HasFormat( SotClipboardFormatId::RTF ) ||
             rData.HasFormat( SotClipboardFormatId::RICHTEXT ) ||
             rData.HasFormat( SotClipboardFormatId::EMBED_SOURCE ) ||
             rData.HasFormat( SotClipboardFormatId::LINK_SOURCE ) ||
             rData.HasFormat( SotClipboardFormatId::EMBED_SOURCE_OLE ) ||
             rData.HasFormat( SotClipboardFormatId::LINK_SOURCE_OLE ) ||
             rData.HasFormat( SotClipboardFormatId::EMBEDDED_OBJ_OLE ) ||
             rData.HasFormat( SotClipboardFormatId::STRING ) ||
             rData.HasFormat( SotClipboardFormatId::STRING_TSVC ) ||
             rData.HasFormat( SotClipboardFormatId::SYLK ) ||
             rData.HasFormat( SotClipboardFormatId::LINK ) ||
             rData.HasFormat( SotClipboardFormatId::HTML ) ||
             rData.HasFormat( SotClipboardFormatId::HTML_SIMPLE ) ||
             rData.HasFormat( SotClipboardFormatId::DIF ) )
        {
            bPossible = true;
        }
    }
    return bPossible;
}

bool ScCellShell::HasClipboardFormat( SotClipboardFormatId nFormatId )
{
    vcl::Window* pWin = GetViewData()->GetActiveWin();
    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ));
    return aDataHelper.HasFormat( nFormatId );
}

IMPL_LINK( ScCellShell, ClipboardChanged, TransferableDataHelper*, pDataHelper, void )
{
    bPastePossible = lcl_IsCellPastePossible( *pDataHelper );

    SfxBindings& rBindings = GetViewData()->GetBindings();
    rBindings.Invalidate( SID_PASTE );
    rBindings.Invalidate( SID_PASTE_SPECIAL );
    rBindings.Invalidate( SID_PASTE_UNFORMATTED );
    rBindings.Invalidate( SID_PASTE_ONLY_VALUE );
    rBindings.Invalidate( SID_PASTE_ONLY_TEXT );
    rBindings.Invalidate( SID_PASTE_ONLY_FORMULA );
    rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
}

namespace {

bool checkDestRanges(ScViewData& rViewData)
{
    ScRange aDummy;
    ScMarkType eMarkType = rViewData.GetSimpleArea( aDummy);
    if (eMarkType != SC_MARK_MULTI)
    {
        // Single destination range.
        if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
            return false;
    }

    if (rViewData.SelectionForbidsCellFill())
        return false;

    // Multiple destination ranges.

    ScDocument* pDoc = rViewData.GetDocument();
    vcl::Window* pWin = rViewData.GetActiveWin();
    if (!pWin)
        return false;

    const ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(pWin));
    if (!pOwnClip)
        // If it's not a Calc document, we won't be picky.
        return true;

    ScDocument* pClipDoc = pOwnClip->GetDocument();
    if (!pClipDoc)
        return false;

    ScRange aSrcRange = pClipDoc->GetClipParam().getWholeRange();
    SCROW nRowSize = aSrcRange.aEnd.Row() - aSrcRange.aStart.Row() + 1;
    SCCOL nColSize = aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1;

    ScMarkData aMark = rViewData.GetMarkData();
    ScRangeList aRanges;
    aMark.MarkToSimple();
    aMark.FillRangeListWithMarks(&aRanges, false);

    return ScClipUtil::CheckDestRanges(pDoc, nColSize, nRowSize, aMark, aRanges);
}

}

void ScCellShell::GetClipState( SfxItemSet& rSet )
{
// SID_PASTE
// SID_PASTE_SPECIAL
// SID_PASTE_UNFORMATTED
// SID_CLIPBOARD_FORMAT_ITEMS

    if ( !pImpl->m_xClipEvtLstnr.is() )
    {
        // create listener
        pImpl->m_xClipEvtLstnr = new TransferableClipboardListener( LINK( this, ScCellShell, ClipboardChanged ) );
        vcl::Window* pWin = GetViewData()->GetActiveWin();
        pImpl->m_xClipEvtLstnr->AddListener( pWin );

        // get initial state
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
        bPastePossible = lcl_IsCellPastePossible( aDataHelper );
    }

    bool bDisable = !bPastePossible;

    //  cell protection / multiple selection

    if (!bDisable)
    {
        SCCOL nCol = GetViewData()->GetCurX();
        SCROW nRow = GetViewData()->GetCurY();
        SCTAB nTab = GetViewData()->GetTabNo();
        ScDocument& rDoc = GetViewData()->GetDocShell()->GetDocument();
        if (!rDoc.IsBlockEditable( nTab, nCol,nRow, nCol,nRow ))
            bDisable = true;

        if (!bDisable && !checkDestRanges(*GetViewData()))
            bDisable = true;
    }

    if (bDisable)
    {
        rSet.DisableItem( SID_PASTE );
        rSet.DisableItem( SID_PASTE_SPECIAL );
        rSet.DisableItem( SID_PASTE_UNFORMATTED );
        rSet.DisableItem( SID_PASTE_ONLY_VALUE );
        rSet.DisableItem( SID_PASTE_ONLY_TEXT );
        rSet.DisableItem( SID_PASTE_ONLY_FORMULA );
        rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    else if ( rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) != SfxItemState::UNKNOWN )
    {
        SvxClipboardFormatItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
        GetPossibleClipboardFormats( aFormats );
        rSet.Put( aFormats );
    }
}

//  only SID_HYPERLINK_GETLINK:

void ScCellShell::GetHLinkState( SfxItemSet& rSet )
{
    //  always return an item (or inserting will be disabled)
    //  if the cell at the cursor contains only a link, return that link

    SvxHyperlinkItem aHLinkItem;
    if ( !GetViewData()->GetView()->HasBookmarkAtCursor( &aHLinkItem ) )
    {
        //! put selected text into item?
    }

    rSet.Put(aHLinkItem);
}

void ScCellShell::GetState(SfxItemSet &rSet)
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScViewData* pData       = GetViewData();
    ScDocument* pDoc        = pData->GetDocument();
    ScMarkData& rMark       = pData->GetMarkData();
    SCCOL       nPosX       = pData->GetCurX();
    SCROW       nPosY       = pData->GetCurY();
    SCTAB       nTab        = pData->GetTabNo();

    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTabSelCount = rMark.GetSelectCount();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_DETECTIVE_REFRESH:
                if (!pDoc->HasDetectiveOperations())
                    rSet.DisableItem( nWhich );
                break;

            case SID_RANGE_ADDRESS:
                {
                    ScRange aRange;
                    if ( pData->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
                    {
                        OUString aStr(aRange.Format(ScRefFlags::VALID | ScRefFlags::TAB_3D,pDoc));
                        rSet.Put( SfxStringItem( nWhich, aStr ) );
                    }
                }
                break;

            case SID_RANGE_NOTETEXT:
                {
                    //  always take cursor position, do not use top-left cell of selection
                    OUString aNoteText;
                    if ( const ScPostIt* pNote = pDoc->GetNote(nPosX, nPosY, nTab) )
                        aNoteText = pNote->GetText();
                    rSet.Put( SfxStringItem( nWhich, aNoteText ) );
                }
                break;

            case SID_RANGE_ROW:
                rSet.Put( SfxInt32Item( nWhich, nPosY+1 ) );
                break;

            case SID_RANGE_COL:
                rSet.Put( SfxInt16Item( nWhich, nPosX+1 ) );
                break;

            case SID_RANGE_TABLE:
                rSet.Put( SfxInt16Item( nWhich, nTab+1 ) );
                break;

            case SID_RANGE_FORMULA:
                {
                    OUString aString;
                    pDoc->GetFormula( nPosX, nPosY, nTab, aString );
                    if( aString.isEmpty() )
                    {
                        pDoc->GetInputString( nPosX, nPosY, nTab, aString );
                    }
                    rSet.Put( SfxStringItem( nWhich, aString ) );
                }
                break;

            case SID_RANGE_TEXTVALUE:
                {
                    OUString aString = pDoc->GetString(nPosX, nPosY, nTab);
                    rSet.Put( SfxStringItem( nWhich, aString ) );
                }
                break;

            case SID_STATUS_SELMODE:
                {
                    /* 0: STD   Click cancels Sel
                     * 1: ER    Click extends selection
                     * 2: ERG   Click defines further selection
                     */
                    sal_uInt16 nMode = pTabViewShell->GetLockedModifiers();

                    switch ( nMode )
                    {
                        case KEY_SHIFT: nMode = 1;  break;
                        case KEY_MOD1:  nMode = 2;  break; // Control-key
                        case 0:
                        default:
                            nMode = 0;
                    }

                    rSet.Put( SfxUInt16Item( nWhich, nMode ) );
                }
                break;

            case SID_STATUS_DOCPOS:
                {
                    OUString aStr = ScResId( STR_TABLE_COUNT );

                    aStr = aStr.replaceFirst("%1", OUString::number( nTab + 1  ) );
                    aStr = aStr.replaceFirst("%2", OUString::number( nTabCount ) );

                    rSet.Put( SfxStringItem( nWhich, aStr ) );                }
                break;

            case SID_ROWCOL_SELCOUNT:
                {
                    ScRange aMarkRange;
                    GetViewData()->GetSimpleArea( aMarkRange );
                    SCCOL nCol1, nCol2;
                    SCROW nRow1, nRow2;
                    nCol1 = aMarkRange.aStart.Col();
                    nRow1 = aMarkRange.aStart.Row();
                    nCol2 = aMarkRange.aEnd.Col();
                    nRow2 = aMarkRange.aEnd.Row();
                    if( nCol2 != nCol1 || nRow1 != nRow2 )
                    {
                        const auto nRows = nRow2 - nRow1 + 1;
                        const auto nCols = nCol2 - nCol1 + 1;
                        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetUILocaleDataWrapper();
                        OUString aRowArg = ScResId(STR_SELCOUNT_ROWARG, nRows).replaceAll("$1", rLocaleData.getNum(nRows, 0));
                        OUString aColArg = ScResId(STR_SELCOUNT_COLARG, nCols).replaceAll("$1", rLocaleData.getNum(nCols, 0));
                        OUString aStr = ScResId(STR_SELCOUNT);
                        aStr = aStr.replaceAll("$1", aRowArg);
                        aStr = aStr.replaceAll("$2", aColArg);
                        rSet.Put( SfxStringItem( nWhich, aStr ) );
                    }
                    else
                    {
                        SCSIZE nSelected, nTotal;
                        pDoc->GetFilterSelCount( nPosX, nPosY, nTab, nSelected, nTotal );
                        if( nTotal )
                        {
                            OUString aStr = ScResId( STR_FILTER_SELCOUNT );
                            aStr = aStr.replaceAll( "$1", OUString::number( nSelected ) );
                            aStr = aStr.replaceAll( "$2", OUString::number( nTotal ) );
                            rSet.Put( SfxStringItem( nWhich, aStr ) );
                        }
                    }
                }
                break;

            //  calculations etc. with date/time/Fail/position&size together

            // #i34458# The SfxStringItem belongs only into SID_TABLE_CELL. It no longer has to be
            // duplicated in SID_ATTR_POSITION or SID_ATTR_SIZE for SvxPosSizeStatusBarControl.
            case SID_TABLE_CELL:
                {
                    //  Test, if error under cursor
                    //  (not pDoc->GetErrCode, to avoid erasing circular references)

                    // In interpreter may happen via rescheduled Basic
                    if ( pDoc->IsInInterpreter() )
                        rSet.Put( SfxStringItem( nWhich, OUString("...") ) );
                    else
                    {
                        FormulaError nErrCode = FormulaError::NONE;
                        ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(nPosX, nPosY, nTab));
                        if (pCell && !pCell->IsRunning())
                            nErrCode = pCell->GetErrCode();

                        OUString aFuncStr;
                        if ( pTabViewShell->GetFunction( aFuncStr, nErrCode ) )
                            rSet.Put( SfxStringItem( nWhich, aFuncStr ) );
                    }
                }
                break;

            case SID_DATA_SELECT:
                // HasSelectionData includes column content and validity,
                // page fields have to be checked separately.
                if ( !pDoc->HasSelectionData( nPosX, nPosY, nTab ) &&
                     !pTabViewShell->HasPageFieldDataAtCursor() )
                    rSet.DisableItem( nWhich );
                break;

            case FID_CURRENTVALIDATION:
                if ( !pDoc->HasValidationData( nPosX, nPosY, nTab ))
                    rSet.DisableItem( nWhich );
                break;

            case SID_STATUS_SUM:
                {
                    OUString aFuncStr;
                    if ( pTabViewShell->GetFunction( aFuncStr, FormulaError::NONE ) )
                        rSet.Put( SfxStringItem( nWhich, aFuncStr ) );
                }
                break;

            case FID_MERGE_ON:
                if ( pDoc->GetChangeTrack() || !pTabViewShell->TestMergeCells() )
                    rSet.DisableItem( nWhich );
                break;

            case FID_MERGE_OFF:
                if ( pDoc->GetChangeTrack() || !pTabViewShell->TestRemoveMerge() )
                    rSet.DisableItem( nWhich );
                break;

            case FID_MERGE_TOGGLE:
                if ( pDoc->GetChangeTrack() )
                    rSet.DisableItem( nWhich );
                else
                {
                    bool bCanMerge = pTabViewShell->TestMergeCells();
                    bool bCanSplit = pTabViewShell->TestRemoveMerge();
                    if( !bCanMerge && !bCanSplit )
                        rSet.DisableItem( nWhich );
                    else
                        rSet.Put( SfxBoolItem( nWhich, bCanSplit ) );
                }
                break;

            case FID_INS_ROWBRK:
                if ( nPosY==0 || (pDoc->HasRowBreak(nPosY, nTab) & ScBreakType::Manual) || pDoc->IsTabProtected(nTab) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_INS_COLBRK:
                if ( nPosX==0 || (pDoc->HasColBreak(nPosX, nTab) & ScBreakType::Manual) || pDoc->IsTabProtected(nTab) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_DEL_ROWBRK:
                if ( nPosY==0 || !(pDoc->HasRowBreak(nPosY, nTab) & ScBreakType::Manual) || pDoc->IsTabProtected(nTab) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_DEL_COLBRK:
                if ( nPosX==0 || !(pDoc->HasColBreak(nPosX, nTab) & ScBreakType::Manual) || pDoc->IsTabProtected(nTab) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_FILL_TAB:
                if ( nTabSelCount < 2 )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_CURRENT_DATE:
            case SID_INSERT_CURRENT_TIME:
                {
                    if ( pDoc->IsTabProtected(nTab) &&
                            pDoc->HasAttrib(nPosX, nPosY, nTab, nPosX, nPosY, nTab, HasAttrFlags::Protected))
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_SELECT_SCENARIO:
                {
                    std::vector<OUString> aList;
                    Color   aDummyCol;

                    if ( !pDoc->IsScenario(nTab) )
                    {
                        OUString aStr;
                        ScScenarioFlags nFlags;
                        SCTAB nScTab = nTab + 1;
                        bool bSheetProtected = pDoc->IsTabProtected(nTab);

                        while ( pDoc->IsScenario(nScTab) )
                        {
                            pDoc->GetName( nScTab, aStr );
                            aList.push_back(aStr);
                            pDoc->GetScenarioData( nScTab, aStr, aDummyCol, nFlags );
                            aList.push_back(aStr);
                            // Protection is sal_True if both Sheet and Scenario are protected
                            aList.push_back((bSheetProtected && (nFlags & ScScenarioFlags::Protected)) ? OUString("1") : OUString("0"));
                            ++nScTab;
                        }
                    }
                    else
                    {
                        OUString aComment;
                        ScScenarioFlags nDummyFlags;
                        pDoc->GetScenarioData( nTab, aComment, aDummyCol, nDummyFlags );
                        OSL_ENSURE( aList.empty(), "List not empty!" );
                        aList.push_back(aComment);
                    }

                    rSet.Put( SfxStringListItem( nWhich, &aList ) );
                }
                break;

            case FID_ROW_HIDE:
            case FID_ROW_SHOW:
            case FID_COL_HIDE:
            case FID_COL_SHOW:
            case FID_COL_OPT_WIDTH:
            case FID_ROW_OPT_HEIGHT:
            case FID_DELETE_CELL:
                if ( pDoc->IsTabProtected(nTab) || pDocSh->IsReadOnly())
                    rSet.DisableItem( nWhich );
                break;

            case SID_OUTLINE_MAKE:
                {
                    if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                            GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                    {
                        //! test for data pilot operation
                    }
                    else if (pDoc->GetChangeTrack()!=nullptr || GetViewData()->IsMultiMarked())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
            case SID_OUTLINE_SHOW:
                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    //! test for data pilot operation
                }
                else if (!pTabViewShell->OutlinePossible(false))
                    rSet.DisableItem( nWhich );
                break;

            case SID_OUTLINE_HIDE:
                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    //! test for data pilot operation
                }
                else if (!pTabViewShell->OutlinePossible(true))
                    rSet.DisableItem( nWhich );
                break;

            case SID_OUTLINE_REMOVE:
                {
                    if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                            GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                    {
                        //! test for data pilot operation
                    }
                    else
                    {
                        bool bCol, bRow;
                        pTabViewShell->TestRemoveOutline( bCol, bRow );
                        if ( !bCol && !bRow )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_COL_WIDTH:
                {
                    SfxUInt16Item aWidthItem( FID_COL_WIDTH, pDoc->GetColWidth( nPosX , nTab) );
                    rSet.Put( aWidthItem );
                    if ( pDocSh->IsReadOnly())
                        rSet.DisableItem( nWhich );

                    //XXX disable if not conclusive
                }
                break;

            case FID_ROW_HEIGHT:
                {
                    SfxUInt16Item aHeightItem( FID_ROW_HEIGHT, pDoc->GetRowHeight( nPosY , nTab) );
                    rSet.Put( aHeightItem );
                    //XXX disable if not conclusive
                    if ( pDocSh->IsReadOnly())
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DETECTIVE_FILLMODE:
                rSet.Put(SfxBoolItem( nWhich, pTabViewShell->IsAuditShell() ));
                break;

            case FID_INPUTLINE_STATUS:
                OSL_FAIL( "Old update method. Use ScTabViewShell::UpdateInputHandler()." );
                break;

            case SID_SCENARIOS:                                     // scenarios:
                if (!(rMark.IsMarked() || rMark.IsMultiMarked()))   // only, if something selected
                    rSet.DisableItem( nWhich );
                break;

            case FID_NOTE_VISIBLE:
                {
                    const ScPostIt* pNote = pDoc->GetNote(nPosX, nPosY, nTab);
                    if ( pNote && pDoc->IsBlockEditable( nTab, nPosX,nPosY, nPosX,nPosY ) )
                        rSet.Put( SfxBoolItem( nWhich, pNote->IsCaptionShown() ) );
                    else
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_HIDE_NOTE:
            case FID_SHOW_NOTE:
                {
                    bool bEnable = false;
                    bool bSearchForHidden = nWhich == FID_SHOW_NOTE;
                    if (!rMark.IsMarked() && !rMark.IsMultiMarked())
                    {
                        // Check current cell
                        const ScPostIt* pNote = pDoc->GetNote(nPosX, nPosY, nTab);
                        if ( pNote && pDoc->IsBlockEditable( nTab, nPosX,nPosY, nPosX,nPosY ) )
                            if ( pNote->IsCaptionShown() != bSearchForHidden)
                                bEnable = true;
                    }
                    else
                    {
                        // Check selection range
                        ScRangeListRef aRangesRef;
                        pData->GetMultiArea(aRangesRef);
                        ScRangeList aRanges = *aRangesRef;
                        std::vector<sc::NoteEntry> aNotes;
                        pDoc->GetNotesInRange(aRanges, aNotes);
                        for(const auto& rNote : aNotes)
                        {
                            const ScAddress& rAdr = rNote.maPos;
                            if( pDoc->IsBlockEditable( rAdr.Tab(), rAdr.Col(), rAdr.Row(), rAdr.Col(), rAdr.Row() ))
                            {
                                if (rNote.mpNote->IsCaptionShown() != bSearchForHidden)
                                {
                                    bEnable = true;
                                    break;
                                }
                            }
                        }

                    }
                    if ( !bEnable )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_SHOW_ALL_NOTES:
            case FID_HIDE_ALL_NOTES:
            case FID_DELETE_ALL_NOTES:
                {
                    bool bHasNotes = false;

                    for (auto const& rTab : rMark.GetSelectedTabs())
                    {
                        if (pDoc->HasTabNotes( rTab ))
                        {
                            bHasNotes = true;
                            break;
                        }
                    }

                    if ( !bHasNotes )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_TOGGLE_NOTES:
                {
                    bool bHasNotes = false;
                    ScRangeList aRanges;

                    for (auto const& rTab : rMark.GetSelectedTabs())
                    {
                        if (pDoc->HasTabNotes( rTab ))
                        {
                            bHasNotes = true;
                            aRanges.push_back(ScRange(0,0,rTab,MAXCOL,MAXROW,rTab));
                        }
                    }

                    if ( !bHasNotes )
                        rSet.DisableItem( nWhich );
                    else
                    {
                         CommentCaptionState eState = pDoc->GetAllNoteCaptionsState( aRanges );
                         bool bAllNotesInShown = !(eState == ALLHIDDEN || eState == MIXED);
                         rSet.Put( SfxBoolItem( SID_TOGGLE_NOTES, bAllNotesInShown) );
                    }
                }
                break;

            case SID_DELETE_NOTE:
                {
                    bool bEnable = false;
                    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
                    {
                        if ( pDoc->IsSelectionEditable( rMark ) )
                        {
                            // look for at least one note in selection
                            ScRangeList aRanges;
                            rMark.FillRangeListWithMarks( &aRanges, false );
                            bEnable = pDoc->ContainsNotesInRange( aRanges );
                        }
                    }
                    else
                    {
                        bEnable = pDoc->IsBlockEditable( nTab, nPosX,nPosY, nPosX,nPosY ) &&
                                  pDoc->GetNote(nPosX, nPosY, nTab);
                    }
                    if ( !bEnable )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_OPENDLG_CONSOLIDATE:
            case SCITEM_CONSOLIDATEDATA:
                {
                    if(pDoc->GetChangeTrack()!=nullptr)
                                rSet.DisableItem( nWhich);
                }
                break;

            case SID_CHINESE_CONVERSION:
            case SID_HANGUL_HANJA_CONVERSION:
                ScViewUtil::HideDisabledSlot( rSet, pData->GetBindings(), nWhich );
            break;

            case FID_USE_NAME:
                {
                    if ( pDocSh && pDocSh->IsDocShared() )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        ScRange aRange;
                        if ( pData->GetSimpleArea( aRange ) != SC_MARK_SIMPLE )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_DEFINE_NAME:
            case FID_INSERT_NAME:
            case FID_ADD_NAME:
            case SID_DEFINE_COLROWNAMERANGES:
                {
                    if ( pDocSh && pDocSh->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_DEFINE_CURRENT_NAME:
            {
                ScAddress aCurrentAddress = ScAddress( nPosX, nPosY, nTab );

                if ( pDoc &&
                     !pDoc->IsAddressInRangeName( RangeNameScope::GLOBAL, aCurrentAddress ) &&
                     !pDoc->IsAddressInRangeName( RangeNameScope::SHEET, aCurrentAddress ))
                {
                    rSet.DisableItem( nWhich );
                }
            }
            break;

            case SID_SPELL_DIALOG:
                {
                    if ( pDoc && pData && pDoc->IsTabProtected( pData->GetTabNo() ) )
                    {
                        bool bVisible = false;
                        SfxViewFrame* pViewFrame = ( pTabViewShell ? pTabViewShell->GetViewFrame() : nullptr );
                        if ( pViewFrame && pViewFrame->HasChildWindow( nWhich ) )
                        {
                            SfxChildWindow* pChild = pViewFrame->GetChildWindow( nWhich );
                            vcl::Window* pWin = ( pChild ? pChild->GetWindow() : nullptr );
                            if ( pWin && pWin->IsVisible() )
                            {
                                bVisible = true;
                            }
                        }
                        if ( !bVisible )
                        {
                            rSet.DisableItem( nWhich );
                        }
                    }
                }
                break;

            case SID_OPENDLG_CURRENTCONDFRMT:
            case SID_OPENDLG_CURRENTCONDFRMT_MANAGER:
                {
                    if ( pDoc )
                    {
                        const SfxPoolItem* pItem = pDoc->GetAttr( nPosX, nPosY, nTab, ATTR_CONDITIONAL );
                        const ScCondFormatItem* pCondFormatItem = static_cast<const ScCondFormatItem*>(pItem);

                        if ( pCondFormatItem->GetCondFormatData().empty() )
                            rSet.DisableItem( nWhich );
                        else if ( pCondFormatItem->GetCondFormatData().size() == 1 )
                            rSet.DisableItem( SID_OPENDLG_CURRENTCONDFRMT_MANAGER );
                        else if ( pCondFormatItem->GetCondFormatData().size() > 1 )
                            rSet.DisableItem( SID_OPENDLG_CURRENTCONDFRMT );
                    }
                }
                break;

        } // switch ( nWitch )
        nWhich = aIter.NextWhich();
    } // while ( nWitch )
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
