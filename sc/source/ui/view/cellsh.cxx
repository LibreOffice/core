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
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/clipfmtitem.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
#include <editeng/langitem.hxx>

#include "cellsh.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "formulacell.hxx"
#include "scmod.hxx"
#include "globstr.hrc"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "scabstdlg.hxx"
#include "dociter.hxx"
#include "postit.hxx"
#include "cliputil.hxx"
#include "clipparam.hxx"
#include "markdata.hxx"

//------------------------------------------------------------------

#define ScCellShell
#define CellMovement
#include "scslots.hxx"

TYPEINIT1( ScCellShell, ScFormatShell );

SFX_IMPL_INTERFACE(ScCellShell, ScFormatShell , ScResId(SCSTR_CELLSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                ScResId(RID_OBJECTBAR_FORMAT));
    SFX_POPUPMENU_REGISTRATION(ScResId(RID_POPUP_CELLS));
}


ScCellShell::ScCellShell(ScViewData* pData) :
    ScFormatShell(pData),
    pImpl( new CellShell_Impl() ),
    bPastePossible(false)
{
    SetHelpId(HID_SCSHELL_CELLSH);
    SetName(OUString("Cell"));
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Cell));
}

ScCellShell::~ScCellShell()
{
    if ( pImpl->m_pClipEvtLstnr )
    {
        pImpl->m_pClipEvtLstnr->AddRemoveListener( GetViewData()->GetActiveWin(), false );

        //  The listener may just now be waiting for the SolarMutex and call the link
        //  afterwards, in spite of RemoveListener. So the link has to be reset, too.
        pImpl->m_pClipEvtLstnr->ClearCallbackLink();

        pImpl->m_pClipEvtLstnr->release();
    }

    delete pImpl->m_pLinkedDlg;
    delete pImpl->m_pRequest;
    delete pImpl;
}

//------------------------------------------------------------------

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
                if ( !bDisable && bEditable )
                {   // do not damage matrix
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                        nCol2, nRow1, nCol2, nRow2, rMark );    // last column
                }
            }
            break;
            case FID_FILL_SERIES:       // fill block
            case SID_OPENDLG_RANDOM_NUMBER_GENERATOR:
            case SID_SAMPLING_DIALOG:
            case SID_DESCRIPTIVE_STATISTICS_DIALOG:
            case SID_OPENDLG_TABOP:     // multiple-cell operations, are at least 2 cells marked?
                if (pDoc->GetChangeTrack()!=NULL &&nWhich ==SID_OPENDLG_TABOP)
                    bDisable = sal_True;
                else
                    bDisable = (!bSimpleArea) || (nCol1 == nCol2 && nRow1 == nRow2);

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

            case SID_CUT:               // cut
            case FID_INS_CELL:          // insert cells, just simple selection
                bDisable = (!bSimpleArea);
                break;

            case FID_INS_ROW:           // insert rows
            case FID_INS_CELLSDOWN:
                bDisable = (!bSimpleArea) || GetViewData()->SimpleColMarked();
                break;

            case FID_INS_COLUMN:        // insert columns
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
                        bDisable = sal_True;
                    }
                }
                break;

            case SID_TRANSLITERATE_HALFWIDTH:
            case SID_TRANSLITERATE_FULLWIDTH:
            case SID_TRANSLITERATE_HIRAGANA:
            case SID_TRANSLITERATE_KATAGANA:
                ScViewUtil::HideDisabledSlot( rSet, GetViewData()->GetBindings(), nWhich );
            break;
        }
        if (!bDisable && bNeedEdit && !bEditable)
            bDisable = sal_True;

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

//  functionen, disabled depending on cursor position
//  Default:
//      SID_INSERT_POSTIT, SID_CHARMAP, SID_OPENDLG_FUNCTION

void ScCellShell::GetCellState( SfxItemSet& rSet )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    ScDocument* pDoc = GetViewData()->GetDocShell()->GetDocument();
    ScAddress aCursor( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                        GetViewData()->GetTabNo() );

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        sal_Bool bDisable = false;
        sal_Bool bNeedEdit = sal_True;      // need cursor position be editable?
        switch ( nWhich )
        {
            case SID_THESAURUS:
                {
                    CellType eType = pDoc->GetCellType( aCursor );
                    bDisable = ( eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT);
                    if (!bDisable)
                    {
                        //  test for available languages
                        sal_uInt16 nLang = ScViewUtil::GetEffLanguage( pDoc, aCursor );
                        bDisable = !ScModule::HasThesaurusLanguage( nLang );
                    }
                }
                break;
            case SID_OPENDLG_FUNCTION:
                {
                    ScMarkData aMarkData=GetViewData()->GetMarkData();
                    aMarkData.MarkToSimple();
                    ScRange aRange;
                    aMarkData.GetMarkArea(aRange);
                    if(aMarkData.IsMarked())
                    {
                        if (!pDoc->IsBlockEditable( aCursor.Tab(), aRange.aStart.Col(),aRange.aStart.Row(),
                                            aRange.aEnd.Col(),aRange.aEnd.Row() ))
                        {
                            bDisable = sal_True;
                        }
                        bNeedEdit=false;
                    }

                }
                break;
            case SID_INSERT_POSTIT:
                {
                    if ( pDocShell && pDocShell->IsDocShared() )
                    {
                        bDisable = sal_True;
                    }
                }
                break;
        }
        if (!bDisable && bNeedEdit)
            if (!pDoc->IsBlockEditable( aCursor.Tab(), aCursor.Col(),aCursor.Row(),
                                        aCursor.Col(),aCursor.Row() ))
                bDisable = sal_True;
        if (bDisable)
            rSet.DisableItem(nWhich);
        nWhich = aIter.NextWhich();
    }
}

static sal_Bool lcl_TestFormat( SvxClipboardFmtItem& rFormats, const TransferableDataHelper& rDataHelper,
                        SotFormatStringId nFormatId )
{
    if ( rDataHelper.HasFormat( nFormatId ) )
    {
        //  translated format name strings are no longer inserted here,
        //  handled by "paste special" dialog / toolbox controller instead.
        //  Only the object type name has to be set here:
        String aStrVal;
        if ( nFormatId == SOT_FORMATSTR_ID_EMBED_SOURCE )
        {
            TransferableObjectDescriptor aDesc;
            if ( ((TransferableDataHelper&)rDataHelper).GetTransferableObjectDescriptor(
                                        SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDesc ) )
                aStrVal = aDesc.maTypeName;
        }
        else if ( nFormatId == SOT_FORMATSTR_ID_EMBED_SOURCE_OLE
          || nFormatId == SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE )
        {
            String aSource;
            SvPasteObjectHelper::GetEmbeddedName( rDataHelper, aStrVal, aSource, nFormatId );
        }

        if ( aStrVal.Len() )
            rFormats.AddClipbrdFormat( nFormatId, aStrVal );
        else
            rFormats.AddClipbrdFormat( nFormatId );

        return sal_True;
    }

    return false;
}

void ScCellShell::GetPossibleClipboardFormats( SvxClipboardFmtItem& rFormats )
{
    Window* pWin = GetViewData()->GetActiveWin();
    sal_Bool bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != NULL );

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );

    lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_DRAWING );
    lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_SVXB );
    lcl_TestFormat( rFormats, aDataHelper, SOT_FORMAT_GDIMETAFILE );
    lcl_TestFormat( rFormats, aDataHelper, SOT_FORMAT_BITMAP );
    lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_EMBED_SOURCE );

    if ( !bDraw )
    {
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_LINK );
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMAT_STRING );
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_DIF );
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMAT_RTF );
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_HTML );
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_HTML_SIMPLE );
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_BIFF_8 );
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_BIFF_5 );
    }

    if ( !lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) )
        lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE );
}

//  insert, insert contents

static sal_Bool lcl_IsCellPastePossible( const TransferableDataHelper& rData )
{
    sal_Bool bPossible = false;
    if ( ScTransferObj::GetOwnClipboard( NULL ) || ScDrawTransferObj::GetOwnClipboard( NULL ) )
        bPossible = sal_True;
    else
    {
        if ( rData.HasFormat( SOT_FORMAT_BITMAP ) ||
             rData.HasFormat( SOT_FORMAT_GDIMETAFILE ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_SVXB ) ||
             rData.HasFormat( FORMAT_PRIVATE ) ||
             rData.HasFormat( SOT_FORMAT_RTF ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ) ||
             rData.HasFormat( SOT_FORMAT_STRING ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_SYLK ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_LINK ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_HTML ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_HTML_SIMPLE ) ||
             rData.HasFormat( SOT_FORMATSTR_ID_DIF ) )
        {
            bPossible = sal_True;
        }
    }
    return bPossible;
}

IMPL_LINK( ScCellShell, ClipboardChanged, TransferableDataHelper*, pDataHelper )
{
    if ( pDataHelper )
    {
        bPastePossible = lcl_IsCellPastePossible( *pDataHelper );

        SfxBindings& rBindings = GetViewData()->GetBindings();
        rBindings.Invalidate( SID_PASTE );
        rBindings.Invalidate( SID_PASTE_SPECIAL );
        rBindings.Invalidate( SID_PASTE_ONLY_VALUE );
        rBindings.Invalidate( SID_PASTE_ONLY_TEXT );
        rBindings.Invalidate( SID_PASTE_ONLY_FORMULA );
        rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    return 0;
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

    // Multiple destination ranges.

    ScDocument* pDoc = rViewData.GetDocument();
    Window* pWin = rViewData.GetActiveWin();
    if (!pWin)
        return false;

    ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(pWin);
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
// SID_CLIPBOARD_FORMAT_ITEMS

    if ( !pImpl->m_pClipEvtLstnr )
    {
        // create listener
        pImpl->m_pClipEvtLstnr = new TransferableClipboardListener( LINK( this, ScCellShell, ClipboardChanged ) );
        pImpl->m_pClipEvtLstnr->acquire();
        Window* pWin = GetViewData()->GetActiveWin();
        pImpl->m_pClipEvtLstnr->AddRemoveListener( pWin, sal_True );

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
        ScDocument* pDoc = GetViewData()->GetDocShell()->GetDocument();
        if (!pDoc->IsBlockEditable( nTab, nCol,nRow, nCol,nRow ))
            bDisable = true;

        if (!checkDestRanges(*GetViewData()))
            bDisable = true;
    }

    if (bDisable)
    {
        rSet.DisableItem( SID_PASTE );
        rSet.DisableItem( SID_PASTE_SPECIAL );
        rSet.DisableItem( SID_PASTE_ONLY_VALUE );
        rSet.DisableItem( SID_PASTE_ONLY_TEXT );
        rSet.DisableItem( SID_PASTE_ONLY_FORMULA );
        rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    else if ( rSet.GetItemState( SID_CLIPBOARD_FORMAT_ITEMS ) != SFX_ITEM_UNKNOWN )
    {
        SvxClipboardFmtItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
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
                        String aStr;
                        sal_uInt16 nFlags = SCA_VALID | SCA_TAB_3D;
                        aRange.Format(aStr,nFlags,pDoc);
                        rSet.Put( SfxStringItem( nWhich, aStr ) );
                    }
                }
                break;

            case SID_RANGE_NOTETEXT:
                {
                    //  always take cursor position, do not use top-left cell of selection
                    String aNoteText;
                    if ( const ScPostIt* pNote = pDoc->GetNotes(nTab)->findByAddress(nPosX, nPosY) )
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

            case SID_RANGE_VALUE:
                {
                    double nValue;
                    pDoc->GetValue( nPosX, nPosY, nTab, nValue );
                    rSet.Put( ScDoubleItem( nWhich, nValue ) );
                }
                break;

            case SID_RANGE_FORMULA:
                {
                    String aString;
                    pDoc->GetFormula( nPosX, nPosY, nTab, aString );
                    if( aString.Len() == 0 )
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
                    OUString aStr = ScGlobal::GetRscString( STR_TABLE ) +
                                    " "   + OUString::number( nTab + 1 ) +
                                    " / " + OUString::number( nTabCount );
                    rSet.Put( SfxStringItem( nWhich, aStr ) );
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
                        sal_uInt16 nErrCode = 0;
                        ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(nPosX, nPosY, nTab));
                        if (pCell)
                        {
                            if (!pCell->IsRunning())
                                nErrCode = pCell->GetErrCode();
                        }

                        String aFuncStr;
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

            case SID_STATUS_SUM:
                {
                    String aFuncStr;
                    if ( pTabViewShell->GetFunction( aFuncStr ) )
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
                if ( nPosY==0 || (pDoc->HasRowBreak(nPosY, nTab) & BREAK_MANUAL) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_INS_COLBRK:
                if ( nPosX==0 || (pDoc->HasColBreak(nPosX, nTab) & BREAK_MANUAL) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_DEL_ROWBRK:
                if ( nPosY==0 || (pDoc->HasRowBreak(nPosY, nTab) & BREAK_MANUAL) == 0 )
                    rSet.DisableItem( nWhich );
                break;

            case FID_DEL_COLBRK:
                if ( nPosX==0 || (pDoc->HasColBreak(nPosX, nTab) & BREAK_MANUAL) == 0 )
                    rSet.DisableItem( nWhich );
                break;

            case FID_FILL_TAB:
                if ( nTabSelCount < 2 )
                    rSet.DisableItem( nWhich );
                break;

            case SID_SELECT_SCENARIO:
                {
                    std::vector<OUString> aList;
                    Color   aDummyCol;

                    if ( !pDoc->IsScenario(nTab) )
                    {
                        OUString aStr;
                        sal_uInt16 nFlags;
                        SCTAB nScTab = nTab + 1;
                        bool bSheetProtected = pDoc->IsTabProtected(nTab);

                        while ( pDoc->IsScenario(nScTab) )
                        {
                            pDoc->GetName( nScTab, aStr );
                            aList.push_back(aStr);
                            pDoc->GetScenarioData( nScTab, aStr, aDummyCol, nFlags );
                            aList.push_back(aStr);
                            // Protection is sal_True if both Sheet and Scenario are protected
                            aList.push_back((bSheetProtected && (nFlags & SC_SCENARIO_PROTECT)) ? OUString("1") : OUString("0"));
                            ++nScTab;
                        }
                    }
                    else
                    {
                        OUString aComment;
                        sal_uInt16  nDummyFlags;
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
                    else if (pDoc->GetChangeTrack()!=NULL || GetViewData()->IsMultiMarked())
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
                else if (!pTabViewShell->OutlinePossible(sal_True))
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
                        sal_Bool bCol, bRow;
                        pTabViewShell->TestRemoveOutline( bCol, bRow );
                        if ( !bCol && !bRow )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_COL_WIDTH:
                {
                    //GetViewData()->GetCurX();
                    SfxUInt16Item aWidthItem( FID_COL_WIDTH, pDoc->GetColWidth( nPosX , nTab) );
                    rSet.Put( aWidthItem );
                    if ( pDocSh->IsReadOnly())
                        rSet.DisableItem( nWhich );

                    //XXX disable if not conclusive
                }
                break;

            case FID_ROW_HEIGHT:
                {
                    //GetViewData()->GetCurY();
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
                    const ScPostIt* pNote = pDoc->GetNotes(nTab)->findByAddress(nPosX, nPosY);
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
                        const ScPostIt* pNote = pDoc->GetNotes(nTab)->findByAddress(nPosX, nPosY);
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
                        size_t nRangeSize = aRanges.size();

                        for ( size_t i = 0; i < nRangeSize && !bEnable; ++i )
                        {
                            const ScRange * pRange = aRanges[i];
                            const SCROW nRow0 = pRange->aStart.Row();
                            const SCROW nRow1 = pRange->aEnd.Row();
                            const SCCOL nCol0 = pRange->aStart.Col();
                            const SCCOL nCol1 = pRange->aEnd.Col();
                            const SCTAB nRangeTab = pRange->aStart.Tab();
                            const size_t nCellNumber = ( nRow1 - nRow0 ) * ( nCol1 - nCol0 );
                            const ScNotes *pNotes = pDoc->GetNotes(nRangeTab);

                            if ( nCellNumber < pNotes->size() )
                            {
                                // Check by each cell
                                for ( SCROW nRow = nRow0; nRow <= nRow1 && !bEnable; ++nRow )
                                {
                                    for ( SCCOL nCol = nCol0; nCol <= nCol1; ++nCol )
                                    {
                                        const ScPostIt* pNote = pNotes->findByAddress(nCol, nRow);
                                        if ( pNote && pDoc->IsBlockEditable( nRangeTab, nCol,nRow, nCol,nRow ) )
                                        {
                                            if ( pNote->IsCaptionShown() != bSearchForHidden)
                                            {
                                                bEnable = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // Check by each document note
                                for (ScNotes::const_iterator itr = pNotes->begin(); itr != pNotes->end(); ++itr)
                                {
                                    SCCOL nCol = itr->first.first;
                                    SCROW nRow = itr->first.second;

                                    if ( nCol <= nCol1 && nRow <= nRow1 && nCol >= nCol0 && nRow >= nRow0 )
                                    {
                                        if ( itr->second->IsCaptionShown() != bSearchForHidden)
                                        {
                                            bEnable = true; //note found
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if ( !bEnable )
                        rSet.DisableItem( nWhich );
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
                            size_t nCount = aRanges.size();
                            for (size_t nPos = 0; nPos < nCount && !bEnable; ++nPos)
                            {
                                ScNotes* pNotes = pDoc->GetNotes( aRanges[nPos]->aStart.Tab() );
                                for (ScNotes::const_iterator itr = pNotes->begin(); itr != pNotes->end(); ++itr)
                                {
                                    SCCOL nCol = itr->first.first;
                                    SCROW nRow = itr->first.second;

                                    if ( nCol <= aRanges[nPos]->aEnd.Col() && nRow <= aRanges[nPos]->aEnd.Row()
                                            && nCol >= aRanges[nPos]->aStart.Col() && nRow >= aRanges[nPos]->aStart.Row() )
                                    {
                                        bEnable = true; //note found
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        bEnable = pDoc->IsBlockEditable( nTab, nPosX,nPosY, nPosX,nPosY ) &&
                                  pDoc->GetNotes(nTab)->findByAddress( nPosX, nPosY );
                    }
                    if ( !bEnable )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_OPENDLG_CONSOLIDATE:
            case SCITEM_CONSOLIDATEDATA:
                {
                    if(pDoc->GetChangeTrack()!=NULL)
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

            case SID_SPELL_DIALOG:
                {
                    if ( pDoc && pData && pDoc->IsTabProtected( pData->GetTabNo() ) )
                    {
                        bool bVisible = false;
                        SfxViewFrame* pViewFrame = ( pTabViewShell ? pTabViewShell->GetViewFrame() : NULL );
                        if ( pViewFrame && pViewFrame->HasChildWindow( nWhich ) )
                        {
                            SfxChildWindow* pChild = pViewFrame->GetChildWindow( nWhich );
                            Window* pWin = ( pChild ? pChild->GetWindow() : NULL );
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

        } // switch ( nWitch )
        nWhich = aIter.NextWhich();
    } // while ( nWitch )
}

//------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
