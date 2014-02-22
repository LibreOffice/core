/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

        
        
        pImpl->m_pClipEvtLstnr->ClearCallbackLink();

        pImpl->m_pClipEvtLstnr->release();
    }

    delete pImpl->m_pLinkedDlg;
    delete pImpl->m_pRequest;
    delete pImpl;
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
        bool bNeedEdit = true;      
        switch ( nWhich )
        {
            case FID_FILL_TO_BOTTOM:    
            {
                bDisable = !bSimpleArea || (nRow1 == 0 && nRow2 == 0);
                if ( !bDisable && bEditable )
                {   
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                        nCol1, nRow1, nCol2, nRow1, rMark );    
                }
            }
            break;
            case FID_FILL_TO_TOP:
            {
                bDisable = (!bSimpleArea) || (nRow1 == MAXROW && nRow2 == MAXROW);
                if ( !bDisable && bEditable )
                {   
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                        nCol1, nRow2, nCol2, nRow2, rMark );    
                }
            }
            break;
            case FID_FILL_TO_RIGHT:     
            {
                bDisable = !bSimpleArea || (nCol1 == 0 && nCol2 == 0);
                if ( !bDisable && bEditable )
                {   
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol1, nRow2, rMark );    
                }
            }
            break;
            case FID_FILL_TO_LEFT:
            {
                bDisable = (!bSimpleArea) || (nCol1 == MAXCOL && nCol2 == MAXCOL);
                if ( !bDisable && bEditable )
                {   
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                        nCol2, nRow1, nCol2, nRow2, rMark );    
                }
            }
            break;

            case SID_RANDOM_NUMBER_GENERATOR_DIALOG:
            case SID_SAMPLING_DIALOG:
            case SID_DESCRIPTIVE_STATISTICS_DIALOG:
            case SID_ANALYSIS_OF_VARIANCE_DIALOG:
            case SID_CORRELATION_DIALOG:
            case SID_COVARIANCE_DIALOG:
            {
                bDisable = !bSimpleArea;
            }
            break;

            case FID_FILL_SERIES:       
            case SID_OPENDLG_TABOP:     
                if (pDoc->GetChangeTrack()!=NULL &&nWhich ==SID_OPENDLG_TABOP)
                    bDisable = true;
                else
                    bDisable = (!bSimpleArea) || (nCol1 == nCol2 && nRow1 == nRow2);

                if ( !bDisable && bEditable && nWhich == FID_FILL_SERIES )
                {   
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol2, nRow1, rMark )     
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow2, nCol2, nRow2, rMark )     
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol1, nRow2, rMark )     
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol2, nRow1, nCol2, nRow2, rMark );    
                }
                break;

            case SID_CUT:               
            case FID_INS_CELL:          
                bDisable = (!bSimpleArea);
                break;

            case FID_INS_ROW:           
            case FID_INS_CELLSDOWN:
                bDisable = (!bSimpleArea) || GetViewData()->SimpleColMarked();
                break;

            case FID_INS_COLUMN:        
            case FID_INS_CELLSRIGHT:
                bDisable = (!bSimpleArea) || GetViewData()->SimpleRowMarked();
                break;

            case SID_COPY:              
                
                
                
                
                if ( !(!bEditable && bOnlyNotBecauseOfMatrix) )
                    bNeedEdit = false;          
                break;

            case SID_AUTOFORMAT:        
                bDisable =    (!bSimpleArea)
                           || ((nCol2 - nCol1) < 2) || ((nRow2 - nRow1) < 2);
                break;

            case SID_CELL_FORMAT_RESET :
            case FID_CELL_FORMAT :
            case SID_ENABLE_HYPHENATION :
                
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
            case SID_TRANSLITERATE_KATAGANA:
                ScViewUtil::HideDisabledSlot( rSet, GetViewData()->GetBindings(), nWhich );
            break;
        }
        if (!bDisable && bNeedEdit && !bEditable)
            bDisable = true;

        if (bDisable)
            rSet.DisableItem(nWhich);
        else if (nWhich == SID_ENABLE_HYPHENATION)
        {
            
            rSet.Put( SfxBoolItem( nWhich, false ) );
        }
        nWhich = aIter.NextWhich();
    }
}





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
        sal_Bool bNeedEdit = sal_True;      
        switch ( nWhich )
        {
            case SID_THESAURUS:
                {
                    CellType eType = pDoc->GetCellType( aCursor );
                    bDisable = ( eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT);
                    if (!bDisable)
                    {
                        
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
        
        
        
        OUString aStrVal;
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
            OUString aSource;
            SvPasteObjectHelper::GetEmbeddedName( rDataHelper, aStrVal, aSource, nFormatId );
        }

        if ( !aStrVal.isEmpty() )
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
    lcl_TestFormat( rFormats, aDataHelper, SOT_FORMATSTR_ID_PNG );
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



static sal_Bool lcl_IsCellPastePossible( const TransferableDataHelper& rData )
{
    sal_Bool bPossible = false;
    if ( ScTransferObj::GetOwnClipboard( NULL ) || ScDrawTransferObj::GetOwnClipboard( NULL ) )
        bPossible = sal_True;
    else
    {
        if ( rData.HasFormat( SOT_FORMATSTR_ID_PNG ) ||
             rData.HasFormat( SOT_FORMAT_BITMAP ) ||
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
        
        if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
            return false;
    }

    

    ScDocument* pDoc = rViewData.GetDocument();
    Window* pWin = rViewData.GetActiveWin();
    if (!pWin)
        return false;

    ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(pWin);
    if (!pOwnClip)
        
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




    if ( !pImpl->m_pClipEvtLstnr )
    {
        
        pImpl->m_pClipEvtLstnr = new TransferableClipboardListener( LINK( this, ScCellShell, ClipboardChanged ) );
        pImpl->m_pClipEvtLstnr->acquire();
        Window* pWin = GetViewData()->GetActiveWin();
        pImpl->m_pClipEvtLstnr->AddRemoveListener( pWin, sal_True );

        
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
        bPastePossible = lcl_IsCellPastePossible( aDataHelper );
    }

    bool bDisable = !bPastePossible;

    

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



void ScCellShell::GetHLinkState( SfxItemSet& rSet )
{
    
    

    SvxHyperlinkItem aHLinkItem;
    if ( !GetViewData()->GetView()->HasBookmarkAtCursor( &aHLinkItem ) )
    {
        
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
                        sal_uInt16 nFlags = SCA_VALID | SCA_TAB_3D;
                        OUString aStr(aRange.Format(nFlags,pDoc));
                        rSet.Put( SfxStringItem( nWhich, aStr ) );
                    }
                }
                break;

            case SID_RANGE_NOTETEXT:
                {
                    
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

            case SID_RANGE_VALUE:
                {
                    double nValue;
                    pDoc->GetValue( nPosX, nPosY, nTab, nValue );
                    rSet.Put( ScDoubleItem( nWhich, nValue ) );
                }
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
                        case KEY_MOD1:  nMode = 2;  break; 
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

            

            
            
            case SID_TABLE_CELL:
                {
                    
                    

                    
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

                        OUString aFuncStr;
                        if ( pTabViewShell->GetFunction( aFuncStr, nErrCode ) )
                            rSet.Put( SfxStringItem( nWhich, aFuncStr ) );
                    }
                }
                break;

            case SID_DATA_SELECT:
                
                
                if ( !pDoc->HasSelectionData( nPosX, nPosY, nTab ) &&
                     !pTabViewShell->HasPageFieldDataAtCursor() )
                    rSet.DisableItem( nWhich );
                break;

            case SID_STATUS_SUM:
                {
                    OUString aFuncStr;
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
                    
                }
                else if (!pTabViewShell->OutlinePossible(false))
                    rSet.DisableItem( nWhich );
                break;

            case SID_OUTLINE_HIDE:
                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    
                }
                else if (!pTabViewShell->OutlinePossible(true))
                    rSet.DisableItem( nWhich );
                break;

            case SID_OUTLINE_REMOVE:
                {
                    if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                            GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                    {
                        
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

                    
                }
                break;

            case FID_ROW_HEIGHT:
                {
                    
                    SfxUInt16Item aHeightItem( FID_ROW_HEIGHT, pDoc->GetRowHeight( nPosY , nTab) );
                    rSet.Put( aHeightItem );
                    
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

            case SID_SCENARIOS:                                     
                if (!(rMark.IsMarked() || rMark.IsMultiMarked()))   
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
                        
                        const ScPostIt* pNote = pDoc->GetNote(nPosX, nPosY, nTab);
                        if ( pNote && pDoc->IsBlockEditable( nTab, nPosX,nPosY, nPosX,nPosY ) )
                            if ( pNote->IsCaptionShown() != bSearchForHidden)
                                bEnable = true;
                    }
                    else
                    {
                        
                        ScRangeListRef aRangesRef;
                        pData->GetMultiArea(aRangesRef);
                        ScRangeList aRanges = *aRangesRef;
                        std::vector<sc::NoteEntry> aNotes;
                        pDoc->GetNotesInRange(aRanges, aNotes);
                        for(std::vector<sc::NoteEntry>::const_iterator itr = aNotes.begin(),
                                itrEnd = aNotes.end(); itr != itrEnd; ++itr)
                        {
                            const ScAddress& rAdr = itr->maPos;
                            if( pDoc->IsBlockEditable( rAdr.Tab(), rAdr.Col(), rAdr.Row(), rAdr.Col(), rAdr.Row() ))
                            {
                                if (itr->mpNote->IsCaptionShown() != bSearchForHidden)
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

            case SID_DELETE_NOTE:
                {
                    bool bEnable = false;
                    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
                    {
                        if ( pDoc->IsSelectionEditable( rMark ) )
                        {
                            
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

        } 
        nWhich = aIter.NextWhich();
    } 
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
