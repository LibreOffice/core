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
#include <svl/smplhint.hxx>
#include <svl/zforlist.hxx>
#include <svx/numfmtsh.hxx>
#include <svx/numinf.hxx>
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "global.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
#include "uiitems.hxx"
#include "editsh.hxx"
#include "hints.hxx"
#include "cellvalue.hxx"
#include "svl/sharedstring.hxx"



void ScTabViewShell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA(SfxSimpleHint))                       
    {
        sal_uLong nSlot = ((SfxSimpleHint&)rHint).GetId();
        switch ( nSlot )
        {
            case FID_DATACHANGED:
                UpdateFormulas();
                break;

            case FID_REFMODECHANGED:
                {
                    bool bRefMode = SC_MOD()->IsFormulaMode();
                    if (!bRefMode)
                        StopRefMode();
                    else
                    {
                        GetSelEngine()->Reset();
                        GetFunctionSet()->SetAnchorFlag(true);
                        
                    }
                }
                break;

            case FID_KILLEDITVIEW:
            case FID_KILLEDITVIEW_NOPAINT:
                StopEditShell();
                KillEditView( nSlot == FID_KILLEDITVIEW_NOPAINT );
                break;

            case SFX_HINT_DOCCHANGED:
                {
                    ScDocument* pDoc = GetViewData()->GetDocument();
                    if (!pDoc->HasTable( GetViewData()->GetTabNo() ))
                    {
                        SetTabNo(0);
                    }
                }
                break;

            case SC_HINT_DRWLAYER_NEW:
                MakeDrawView();
                break;

            case SC_HINT_DOC_SAVED:
                {
                    
                    
                    
                    
                    
                    

                    UpdateLayerLocks();

                    
                    
                    
                }
                break;

            case SFX_HINT_MODECHANGED:
                
                
                

                if ( GetViewData()->GetSfxDocShell()->IsReadOnly() != (bReadOnly ? 1 : 0) )
                {
                    bReadOnly = GetViewData()->GetSfxDocShell()->IsReadOnly();

                    SfxBoolItem aItem( SID_FM_DESIGN_MODE, !bReadOnly);
                    GetViewData()->GetDispatcher().Execute( SID_FM_DESIGN_MODE, SFX_CALLMODE_ASYNCHRON,
                                                &aItem, 0L );

                    UpdateInputContext();
                }
                break;

            case SC_HINT_SHOWRANGEFINDER:
                PaintRangeFinder();
                break;

            case SC_HINT_FORCESETTAB:
                SetTabNo( GetViewData()->GetTabNo(), true );
                break;

            default:
                break;
        }
    }
    else if (rHint.ISA(ScPaintHint))                    
    {
        ScPaintHint* pHint = (ScPaintHint*) &rHint;
        sal_uInt16 nParts = pHint->GetParts();
        SCTAB nTab = GetViewData()->GetTabNo();
        if (pHint->GetStartTab() <= nTab && pHint->GetEndTab() >= nTab)
        {
            if (nParts & PAINT_EXTRAS)          
                if (PaintExtras())
                    nParts = PAINT_ALL;

            
            
            GetViewData()->GetDocShell()->UpdatePendingRowHeights( GetViewData()->GetTabNo() );

            if (nParts & PAINT_SIZE)
                RepeatResize();                     
            if (nParts & PAINT_GRID)
                PaintArea( pHint->GetStartCol(), pHint->GetStartRow(),
                           pHint->GetEndCol(), pHint->GetEndRow() );
            if (nParts & PAINT_MARKS)
                PaintArea( pHint->GetStartCol(), pHint->GetStartRow(),
                           pHint->GetEndCol(), pHint->GetEndRow(), SC_UPDATE_MARKS );
            if (nParts & PAINT_LEFT)
                PaintLeftArea( pHint->GetStartRow(), pHint->GetEndRow() );
            if (nParts & PAINT_TOP)
                PaintTopArea( pHint->GetStartCol(), pHint->GetEndCol() );

            
            if (nParts & ( PAINT_LEFT | PAINT_TOP ))    
                UpdateAllOverlays();

            HideNoteMarker();
        }
    }
    else if (rHint.ISA(ScEditViewHint))                 
    {
        

        ScEditViewHint* pHint = (ScEditViewHint*) &rHint;
        SCTAB nTab = GetViewData()->GetTabNo();
        if ( pHint->GetTab() == nTab )
        {
            SCCOL nCol = pHint->GetCol();
            SCROW nRow = pHint->GetRow();
            {
                HideNoteMarker();

                MakeEditView( pHint->GetEngine(), nCol, nRow );

                StopEditShell();                    

                ScSplitPos eActive = GetViewData()->GetActivePart();
                if ( GetViewData()->HasEditView(eActive) )
                {
                    
                    
                    

                    EditView* pView = GetViewData()->GetEditView(eActive);  

                    SetEditShell(pView, true);
                }
            }
        }
    }
    else if (rHint.ISA(ScTablesHint))               
    {
            
        SCTAB nActiveTab = GetViewData()->GetTabNo();

        const ScTablesHint& rTabHint = (const ScTablesHint&)rHint;
        SCTAB nTab1 = rTabHint.GetTab1();
        SCTAB nTab2 = rTabHint.GetTab2();
        sal_uInt16 nId  = rTabHint.GetId();
        switch (nId)
        {
            case SC_TAB_INSERTED:
                GetViewData()->InsertTab( nTab1 );
                break;
            case SC_TAB_DELETED:
                GetViewData()->DeleteTab( nTab1 );
                break;
            case SC_TAB_MOVED:
                GetViewData()->MoveTab( nTab1, nTab2 );
                break;
            case SC_TAB_COPIED:
                GetViewData()->CopyTab( nTab1, nTab2 );
                break;
            case SC_TAB_HIDDEN:
                break;
            case SC_TABS_INSERTED:
                GetViewData()->InsertTabs( nTab1, nTab2 );
                break;
            case SC_TABS_DELETED:
                GetViewData()->DeleteTabs( nTab1, nTab2 );
                break;
            default:
                OSL_FAIL("unbekannter ScTablesHint");
        }

        
        

        SCTAB nNewTab = nActiveTab;
        bool bStayOnActiveTab = true;
        switch (nId)
        {
            case SC_TAB_INSERTED:
                if ( nTab1 <= nNewTab )             
                    ++nNewTab;
                break;
            case SC_TAB_DELETED:
                if ( nTab1 < nNewTab )              
                    --nNewTab;
                else if ( nTab1 == nNewTab )        
                    bStayOnActiveTab = false;
                break;
            case SC_TAB_MOVED:
                if ( nNewTab == nTab1 )             
                    nNewTab = nTab2;
                else if ( nTab1 < nTab2 )           
                {
                    if ( nNewTab > nTab1 && nNewTab <= nTab2 )      
                        --nNewTab;
                }
                else                                
                {
                    if ( nNewTab >= nTab2 && nNewTab < nTab1 )      
                        ++nNewTab;
                }
                break;
            case SC_TAB_COPIED:
                if ( nNewTab >= nTab2 )             
                    ++nNewTab;
                break;
            case SC_TAB_HIDDEN:
                if ( nTab1 == nNewTab )             
                    bStayOnActiveTab = false;
                break;
            case SC_TABS_INSERTED:
                if ( nTab1 <= nNewTab )
                    nNewTab += nTab2;
                break;
            case SC_TABS_DELETED:
                if ( nTab1 < nNewTab )
                    nNewTab -= nTab2;
                break;
        }

        ScDocument* pDoc = GetViewData()->GetDocument();
        if ( nNewTab >= pDoc->GetTableCount() )
            nNewTab = pDoc->GetTableCount() - 1;

        bool bForce = !bStayOnActiveTab;
        SetTabNo( nNewTab, bForce, false, bStayOnActiveTab );
    }
    else if (rHint.ISA(ScIndexHint))
    {
        const ScIndexHint& rIndexHint = (const ScIndexHint&)rHint;
        sal_uInt16 nId = rIndexHint.GetId();
        sal_uInt16 nIndex = rIndexHint.GetIndex();
        switch (nId)
        {
            case SC_HINT_SHOWRANGEFINDER:
                PaintRangeFinder( nIndex );
                break;
        }
    }

    SfxViewShell::Notify( rBC, rHint );
}



SvxNumberInfoItem* ScTabViewShell::MakeNumberInfoItem( ScDocument* pDoc, ScViewData* pViewData )
{
    
    
    
    SvxNumberValueType  eValType        = SVX_VALUE_TYPE_UNDEFINED;
    double              nCellValue      = 0;
    OUString aCellString;

    ScRefCellValue aCell;
    aCell.assign(*pDoc, pViewData->GetCurPos());

    switch (aCell.meType)
    {
        case CELLTYPE_VALUE:
        {
            nCellValue = aCell.mfValue;
            eValType = SVX_VALUE_TYPE_NUMBER;
        }
        break;

        case CELLTYPE_STRING:
        {
            aCellString = aCell.mpString->getString();
            eValType = SVX_VALUE_TYPE_STRING;
        }
        break;

        case CELLTYPE_FORMULA:
        {
            if (aCell.mpFormula->IsValue())
            {
                nCellValue = aCell.mpFormula->GetValue();
                eValType = SVX_VALUE_TYPE_NUMBER;
            }
            else
            {
                nCellValue = 0;
                eValType   = SVX_VALUE_TYPE_UNDEFINED;
            }
        }
        break;

        default:
            nCellValue = 0;
            eValType   = SVX_VALUE_TYPE_UNDEFINED;
    }

    switch ( eValType )
    {
        case SVX_VALUE_TYPE_STRING:
            return new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                aCellString,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SVX_VALUE_TYPE_NUMBER:
            return new SvxNumberInfoItem(
                                pDoc->GetFormatTable(),
                                nCellValue,
                                SID_ATTR_NUMBERFORMAT_INFO );

        case SVX_VALUE_TYPE_UNDEFINED:
        default:
            ;
    }

    return new SvxNumberInfoItem(
        pDoc->GetFormatTable(), static_cast<const sal_uInt16>(SID_ATTR_NUMBERFORMAT_INFO));
}



void ScTabViewShell::UpdateNumberFormatter(
                        const SvxNumberInfoItem& rInfoItem )
{
    const sal_uInt32 nDelCount = rInfoItem.GetDelCount();

    if ( nDelCount > 0 )
    {
        const sal_uInt32* pDelArr = rInfoItem.GetDelArray();

        for ( sal_uInt16 i=0; i<nDelCount; i++ )
            rInfoItem.GetNumberFormatter()->DeleteEntry( pDelArr[i] );
    }
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
