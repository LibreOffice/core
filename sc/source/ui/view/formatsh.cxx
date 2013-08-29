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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include "scitems.hxx"
#include <editeng/borderline.hxx>
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include <svl/stritem.hxx>
#include <svl/zformat.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/numinf.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/tplpitem.hxx>
#include <editeng/svxenum.hxx>
#include <svx/algitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/justifyitem.hxx>

#include "formatsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "docsh.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "printfun.hxx"
#include "docpool.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"
#include "undostyl.hxx"
#include "markdata.hxx"
#include "markarr.hxx"


#define ScFormatShell
#define TableFont
#define FormatForSelection
#include "scslots.hxx"

#include "scabstdlg.hxx"
#include <editeng/fontitem.hxx>

using namespace ::com::sun::star;

namespace {

SvxCellHorJustify lclConvertSlotToHAlign( sal_uInt16 nSlot )
{
    SvxCellHorJustify eHJustify = SVX_HOR_JUSTIFY_STANDARD;
    switch( nSlot )
    {
        case SID_ALIGN_ANY_HDEFAULT:    eHJustify = SVX_HOR_JUSTIFY_STANDARD;   break;
        case SID_ALIGN_ANY_LEFT:        eHJustify = SVX_HOR_JUSTIFY_LEFT;       break;
        case SID_ALIGN_ANY_HCENTER:     eHJustify = SVX_HOR_JUSTIFY_CENTER;     break;
        case SID_ALIGN_ANY_RIGHT:       eHJustify = SVX_HOR_JUSTIFY_RIGHT;      break;
        case SID_ALIGN_ANY_JUSTIFIED:   eHJustify = SVX_HOR_JUSTIFY_BLOCK;      break;
        default:    OSL_FAIL( "lclConvertSlotToHAlign - invalid slot" );
    }
    return eHJustify;
}

SvxCellVerJustify lclConvertSlotToVAlign( sal_uInt16 nSlot )
{
    SvxCellVerJustify eVJustify = SVX_VER_JUSTIFY_STANDARD;
    switch( nSlot )
    {
        case SID_ALIGN_ANY_VDEFAULT:    eVJustify = SVX_VER_JUSTIFY_STANDARD;   break;
        case SID_ALIGN_ANY_TOP:         eVJustify = SVX_VER_JUSTIFY_TOP;        break;
        case SID_ALIGN_ANY_VCENTER:     eVJustify = SVX_VER_JUSTIFY_CENTER;     break;
        case SID_ALIGN_ANY_BOTTOM:      eVJustify = SVX_VER_JUSTIFY_BOTTOM;     break;
        default:    OSL_FAIL( "lclConvertSlotToVAlign - invalid slot" );
    }
    return eVJustify;
}

} // namespace

TYPEINIT1( ScFormatShell, SfxShell );

SFX_IMPL_INTERFACE(ScFormatShell, SfxShell, ScResId(SCSTR_FORMATSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                ScResId(RID_OBJECTBAR_FORMAT));

}


ScFormatShell::ScFormatShell(ScViewData* pData) :
    SfxShell(pData->GetViewShell()),
    pViewData(pData)
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();

    SetPool( &pTabViewShell->GetPool() );
    ::svl::IUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !pViewData->GetDocument()->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetHelpId(HID_SCSHELL_FORMATSH);
    SetName(OUString("Format"));
}

ScFormatShell::~ScFormatShell()
{
}

//------------------------------------------------------------------

void ScFormatShell::GetStyleState( SfxItemSet& rSet )
{
    ScDocument*             pDoc        = GetViewData()->GetDocument();
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxStyleSheetBasePool*  pStylePool  = pDoc->GetStyleSheetPool();

    bool bProtected = false;
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nTabCount; i++)
        if (pDoc->IsTabProtected(i))                // look after protected table
            bProtected = true;

    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();
    sal_uInt16          nSlotId = 0;

    while ( nWhich )
    {
        nSlotId = SfxItemPool::IsWhich( nWhich )
                    ? GetPool().GetSlotId( nWhich )
                    : nWhich;

        switch ( nSlotId )
        {
            case SID_STYLE_APPLY:
                if ( !pStylePool )
                    rSet.DisableItem( nSlotId );
                break;

            case SID_STYLE_FAMILY2:     // cell style sheets
            {
                SfxStyleSheet* pStyleSheet = (SfxStyleSheet*)
                                             pTabViewShell->GetStyleSheetFromMarked();

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, pStyleSheet->GetName() ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, String() ) );
            }
            break;

            case SID_STYLE_FAMILY4:     // page style sheets
            {
                SCTAB           nCurTab     = GetViewData()->GetTabNo();
                String          aPageStyle  = pDoc->GetPageStyle( nCurTab );
                SfxStyleSheet*  pStyleSheet = (SfxStyleSheet*)pStylePool->
                                    Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, aPageStyle ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, String() ) );
            }
            break;

            case SID_STYLE_WATERCAN:
            {
                rSet.Put( SfxBoolItem( nSlotId, SC_MOD()->GetIsWaterCan() ) );
            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                ISfxTemplateCommon* pDesigner = SFX_APP()->
                        GetCurrentTemplateCommon(pTabViewShell->GetViewFrame()->GetBindings());
                bool bPage = pDesigner && SFX_STYLE_FAMILY_PAGE == pDesigner->GetActualFamily();

                if ( bProtected || bPage )
                    rSet.DisableItem( nSlotId );
            }
            break;

            case SID_STYLE_EDIT:
            case SID_STYLE_DELETE:
            case SID_STYLE_HIDE:
            case SID_STYLE_SHOW:
            {
                ISfxTemplateCommon* pDesigner = SFX_APP()->
                        GetCurrentTemplateCommon(pTabViewShell->GetViewFrame()->GetBindings());
                bool bPage = pDesigner && SFX_STYLE_FAMILY_PAGE == pDesigner->GetActualFamily();

                if ( bProtected && !bPage )
                    rSet.DisableItem( nSlotId );
            }
            break;

            default:
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

//------------------------------------------------------------------

void ScFormatShell::ExecuteStyle( SfxRequest& rReq )
{
    // in case of vertical toolbar
    if ( !rReq.GetArgs() )
    {
        pViewData->GetDispatcher().Execute( SID_STYLE_DESIGNER, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        return;
    }

    //--------------------------------------------------------------------
    SfxBindings&        rBindings   = pViewData->GetBindings();
    const SfxItemSet*   pArgs       = rReq.GetArgs();
    const sal_uInt16        nSlotId     = rReq.GetSlot();
    const SCTAB         nCurTab     = GetViewData()->GetTabNo();
    ScDocShell*         pDocSh      = GetViewData()->GetDocShell();
    ScTabViewShell*     pTabViewShell= GetViewData()->GetViewShell();
    ScDocument*         pDoc        = pDocSh->GetDocument();
    ScMarkData&         rMark       = GetViewData()->GetMarkData();
    ScModule*           pScMod      = SC_MOD();
    String              aRefName;
    bool                bUndo       = pDoc->IsUndoEnabled();
    SfxStyleSheetBasePool*  pStylePool  = pDoc->GetStyleSheetPool();

    if ( (nSlotId == SID_STYLE_PREVIEW)
        || (nSlotId ==  SID_STYLE_END_PREVIEW) )
    {
        if (nSlotId == SID_STYLE_PREVIEW)
        {
            SfxStyleFamily eFamily = SFX_STYLE_FAMILY_PARA;
            const SfxPoolItem* pFamItem;
            if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILY, sal_True, &pFamItem ) )
                eFamily = (SfxStyleFamily)((const SfxUInt16Item*)pFamItem)->GetValue();
            const SfxPoolItem* pNameItem;
            OUString aStyleName;
            if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( nSlotId, sal_True, &pNameItem ))
                aStyleName  = ((const SfxStringItem*)pNameItem)->GetValue();
            if ( eFamily == SFX_STYLE_FAMILY_PARA ) // CellStyles
            {
                ScMarkData aFuncMark( pViewData->GetMarkData() );
                ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
                aFuncMark.MarkToMulti();

                if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
                {
                    SCCOL nCol = pViewData->GetCurX();
                    SCROW nRow = pViewData->GetCurY();
                    SCTAB nTab = pViewData->GetTabNo();
                    ScRange aRange( nCol, nRow, nTab );
                    aFuncMark.SetMarkArea( aRange );
                }
                pDoc->SetPreviewSelection( aFuncMark );
                ScStyleSheet* pPreviewStyle = static_cast<ScStyleSheet*>( pStylePool->Find( aStyleName, eFamily ) );
                pDoc->SetPreviewCellStyle( pPreviewStyle  );
                ScPatternAttr aAttr( *pDoc->GetSelectionPattern( aFuncMark ) );
                aAttr.SetStyleSheet( pPreviewStyle );

                SfxItemSet aItemSet( GetPool() );

                ScPatternAttr aNewAttrs( GetViewData()->GetDocument()->GetPool() );
                SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
                rNewSet.Put( aItemSet, false );

                pDoc->ApplySelectionPattern( aNewAttrs, pDoc->GetPreviewSelection() );
                pTabViewShell->UpdateSelectionArea( aFuncMark, &aAttr );
            }
        }
        else
        {
            ScPatternAttr aAttr( *pDoc->GetSelectionPattern( pDoc->GetPreviewSelection() ) );
            if ( ScStyleSheet* pPreviewStyle = pDoc->GetPreviewCellStyle() )
                aAttr.SetStyleSheet( pPreviewStyle );
            pDoc->SetPreviewCellStyle(NULL);

            SfxItemSet aItemSet( GetPool() );

            ScPatternAttr aNewAttrs( GetViewData()->GetDocument()->GetPool() );
            SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
            rNewSet.Put( aItemSet, false );
            pDoc->ApplySelectionPattern( aNewAttrs, pDoc->GetPreviewSelection() );
            pTabViewShell->UpdateSelectionArea(  pDoc->GetPreviewSelection(), &aAttr );
        }
    }
    else if (   (nSlotId == SID_STYLE_NEW)
        || (nSlotId == SID_STYLE_EDIT)
        || (nSlotId == SID_STYLE_DELETE)
        || (nSlotId == SID_STYLE_HIDE)
        || (nSlotId == SID_STYLE_SHOW)
        || (nSlotId == SID_STYLE_APPLY)
        || (nSlotId == SID_STYLE_WATERCAN)
        || (nSlotId == SID_STYLE_FAMILY)
        || (nSlotId == SID_STYLE_NEW_BY_EXAMPLE)
        || (nSlotId == SID_STYLE_UPDATE_BY_EXAMPLE) )
    {
        SfxStyleSheetBase*      pStyleSheet = NULL;

        bool bStyleToMarked = false;
        bool bListAction = false;
        bool bAddUndo = false;          // add ScUndoModifyStyle (style modified)
        ScStyleSaveData aOldData;       // for undo/redo
        ScStyleSaveData aNewData;

        SfxStyleFamily eFamily = SFX_STYLE_FAMILY_PARA;
        const SfxPoolItem* pFamItem;
        if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILY, sal_True, &pFamItem ) )
            eFamily = (SfxStyleFamily)((const SfxUInt16Item*)pFamItem)->GetValue();
        else if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILYNAME, sal_True, &pFamItem ) )
        {
            String sFamily = ((const SfxStringItem*)pFamItem)->GetValue();
            if (sFamily.CompareToAscii("CellStyles") == COMPARE_EQUAL)
                eFamily = SFX_STYLE_FAMILY_PARA;
            else if (sFamily.CompareToAscii("PageStyles") == COMPARE_EQUAL)
                eFamily = SFX_STYLE_FAMILY_PAGE;
        }

        String                  aStyleName;
        sal_uInt16                  nRetMask = 0xffff;

        pStylePool->SetSearchMask( eFamily, SFXSTYLEBIT_ALL );

        switch ( nSlotId )
        {
            case SID_STYLE_NEW:
                {
                    const SfxPoolItem* pNameItem;
                    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( nSlotId, sal_True, &pNameItem ))
                        aStyleName  = ((const SfxStringItem*)pNameItem)->GetValue();

                    const SfxPoolItem* pRefItem=NULL;
                    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_REFERENCE, sal_True, &pRefItem ))
                    {
                        if(pRefItem!=NULL)
                            aRefName  = ((const SfxStringItem*)pRefItem)->GetValue();
                    }

                    pStyleSheet = &(pStylePool->Make( aStyleName, eFamily,
                                                      SFXSTYLEBIT_USERDEF ) );

                    if ( pStyleSheet && pStyleSheet->HasParentSupport() )
                        pStyleSheet->SetParent(aRefName);
                }
                break;

            case SID_STYLE_APPLY:
            {
                SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, SID_APPLY_STYLE, false );
                SFX_REQUEST_ARG( rReq, pFamilyItem, SfxStringItem, SID_STYLE_FAMILYNAME, false );
                if ( pFamilyItem && pNameItem )
                {
                    com::sun::star::uno::Reference< com::sun::star::style::XStyleFamiliesSupplier > xModel(pDocSh->GetModel(), com::sun::star::uno::UNO_QUERY);
                    try
                    {
                        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xStyles;
                        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > xCont = xModel->getStyleFamilies();
                        xCont->getByName(pFamilyItem->GetValue()) >>= xStyles;
                        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xInfo;
                        xStyles->getByName( pNameItem->GetValue() ) >>= xInfo;
                        OUString aUIName;
                        xInfo->getPropertyValue("DisplayName") >>= aUIName;
                        if ( !aUIName.isEmpty() )
                            rReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aUIName ) );
                    }
                    catch( com::sun::star::uno::Exception& )
                    {
                    }
                }
            }
            case SID_STYLE_EDIT:
            case SID_STYLE_DELETE:
            case SID_STYLE_HIDE:
            case SID_STYLE_SHOW:
            case SID_STYLE_NEW_BY_EXAMPLE:
                {
                    const SfxPoolItem* pNameItem;
                    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( nSlotId, sal_True, &pNameItem ))
                        aStyleName  = ((const SfxStringItem*)pNameItem)->GetValue();
                    pStyleSheet = pStylePool->Find( aStyleName, eFamily );

                    aOldData.InitFromStyle( pStyleSheet );
                }
                break;

            case SID_STYLE_WATERCAN:
            {
                bool bWaterCan = pScMod->GetIsWaterCan();

                if( !bWaterCan )
                {
                    const SfxPoolItem* pItem;

                    if ( SFX_ITEM_SET ==
                         pArgs->GetItemState( nSlotId, sal_True, &pItem ) )
                    {
                        const SfxStringItem* pStrItem = PTR_CAST(SfxStringItem,pItem);
                        if ( pStrItem )
                        {
                            aStyleName  = pStrItem->GetValue();
                            pStyleSheet = pStylePool->Find( aStyleName, eFamily );

                            if ( pStyleSheet )
                            {
                                ((ScStyleSheetPool*)pStylePool)->
                                        SetActualStyleSheet( pStyleSheet );
                                rReq.Done();
                            }
                        }
                    }
                }

                if ( !bWaterCan && pStyleSheet )
                {
                    pScMod->SetWaterCan( true );
                    pTabViewShell->SetActivePointer( Pointer(POINTER_FILL) );
                    rReq.Done();
                }
                else
                {
                    pScMod->SetWaterCan( false );
                    pTabViewShell->SetActivePointer( Pointer(POINTER_ARROW) );
                    rReq.Done();
                }
            }
            break;

            default:
                break;
        }

        // set new style for WaterCan-Mode
        if ( nSlotId == SID_STYLE_APPLY && pScMod->GetIsWaterCan() && pStyleSheet )
            ((ScStyleSheetPool*)pStylePool)->SetActualStyleSheet( pStyleSheet );

        switch ( eFamily )
        {
            case SFX_STYLE_FAMILY_PARA:
            {
                switch ( nSlotId )
                {
                    case SID_STYLE_DELETE:
                    {
                        if ( pStyleSheet )
                        {
                            pTabViewShell->RemoveStyleSheetInUse( pStyleSheet );
                            pStylePool->Remove( pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                            nRetMask = true;
                            bAddUndo = true;
                            rReq.Done();
                        }
                        else
                            nRetMask = false;
                    }
                    break;

                    case SID_STYLE_HIDE:
                    case SID_STYLE_SHOW:
                    {
                        if ( pStyleSheet )
                        {
                            pStyleSheet->SetHidden( nSlotId == SID_STYLE_HIDE );
                            pTabViewShell->InvalidateAttribs();
                            rReq.Done();
                        }
                        else
                            nRetMask = false;
                    }
                    break;

                    case SID_STYLE_APPLY:
                    {
                        if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                        {
                            // apply style sheet to document
                            pTabViewShell->SetStyleSheetToMarked( (SfxStyleSheet*)pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_NEW_BY_EXAMPLE:
                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                    {
                        // create/replace style sheet by attributes
                        // at cursor position:

                        const ScPatternAttr* pAttrItem = NULL;

                        // Die Abfrage, ob markiert ist, war hier immer falsch,
                        // darum jetzt gar nicht mehr, und einfach vom Cursor.
                        // Wenn Attribute aus der Selektion genommen werden sollen,
                        // muss noch darauf geachtet werden, Items aus Vorlagen nicht
                        // zu uebernehmen (GetSelectionPattern sammelt auch Items aus
                        // Vorlagen zusammen) (#44748#)
                        //      pAttrItem = GetSelectionPattern();

                        // ScViewData* pViewData = GetViewData();
                        SCCOL       nCol = pViewData->GetCurX();
                        SCROW       nRow = pViewData->GetCurY();
                        pAttrItem = pDoc->GetPattern( nCol, nRow, nCurTab );

                        SfxItemSet aAttrSet = pAttrItem->GetItemSet();
                        aAttrSet.ClearItem( ATTR_MERGE );
                        aAttrSet.ClearItem( ATTR_MERGE_FLAG );
                        //  bedingte Formatierung und Gueltigkeit nicht uebernehmen,
                        //  weil sie in der Vorlage nicht editiert werden koennen
                        aAttrSet.ClearItem( ATTR_VALIDDATA );
                        aAttrSet.ClearItem( ATTR_CONDITIONAL );

                        if ( SID_STYLE_NEW_BY_EXAMPLE == nSlotId )
                        {
                            if ( bUndo )
                            {
                                String aUndo = ScGlobal::GetRscString( STR_UNDO_EDITCELLSTYLE );
                                pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
                                bListAction = true;
                            }

                            bool bConvertBack = false;
                            SfxStyleSheet*  pSheetInUse = (SfxStyleSheet*)
                                                          pTabViewShell->GetStyleSheetFromMarked();

                            // wenn neuer Style vorhanden und in der Selektion
                            // verwendet wird, so darf der Parent nicht uebernommen
                            // werden:

                            if ( pStyleSheet && pSheetInUse && pStyleSheet == pSheetInUse )
                                pSheetInUse = NULL;

                            // wenn bereits vorhanden, erstmal entfernen...
                            if ( pStyleSheet )
                            {
                                // Style-Pointer zu Namen vor Erase,
                                // weil Zellen sonst ungueltige Pointer
                                // enthalten.
                                //!!! bei Gelenheit mal eine Methode, die
                                //    das fuer einen bestimmten Style macht
                                pDoc->StylesToNames();
                                bConvertBack = true;
                                pStylePool->Remove(pStyleSheet);
                            }

                            // ...und neu anlegen
                            pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                             SFXSTYLEBIT_USERDEF );

                            // wenn ein Style vorhanden ist, so wird dieser
                            // Parent der neuen Vorlage:
                            if ( pSheetInUse && pStyleSheet->HasParentSupport() )
                                pStyleSheet->SetParent( pSheetInUse->GetName() );

                            if ( bConvertBack )
                                // Namen zu Style-Pointer
                                pDoc->UpdStlShtPtrsFrmNms();
                            else
                                pDoc->GetPool()->CellStyleCreated( aStyleName );

                            // Attribute uebernehmen und Style anwenden
                            pStyleSheet->GetItemSet().Put( aAttrSet );
                            pTabViewShell->UpdateStyleSheetInUse( pStyleSheet );

                            //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle
                            //  (pStyleSheet pointer is used!)
                            bStyleToMarked = true;
                        }
                        else // ( nSlotId == SID_STYLE_UPDATE_BY_EXAMPLE )
                        {
                            pStyleSheet = (SfxStyleSheet*)pTabViewShell->GetStyleSheetFromMarked();

                            if ( pStyleSheet )
                            {
                                aOldData.InitFromStyle( pStyleSheet );

                                if ( bUndo )
                                {
                                    String aUndo = ScGlobal::GetRscString( STR_UNDO_EDITCELLSTYLE );
                                    pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
                                    bListAction = true;
                                }

                                pStyleSheet->GetItemSet().Put( aAttrSet );
                                pTabViewShell->UpdateStyleSheetInUse( pStyleSheet );

                                //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle
                                //  (pStyleSheet pointer is used!)
                                bStyleToMarked = true;
                            }
                        }

                        aNewData.InitFromStyle( pStyleSheet );
                        bAddUndo = true;
                        rReq.Done();
                    }
                    break;

                    default:
                        break;
                }
            } // case SFX_STYLE_FAMILY_PARA:
            break;

            case SFX_STYLE_FAMILY_PAGE:
            {
                switch ( nSlotId )
                {
                    case SID_STYLE_DELETE:
                    {
                        nRetMask = ( NULL != pStyleSheet );
                        if ( pStyleSheet )
                        {
                            if ( pDoc->RemovePageStyleInUse( pStyleSheet->GetName() ) )
                            {
                                ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(true), nCurTab ).UpdatePages();
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );
                            }
                            pStylePool->Remove( pStyleSheet );
                            rBindings.Invalidate( SID_STYLE_FAMILY4 );
                            pDocSh->SetDocumentModified();
                            bAddUndo = true;
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_HIDE:
                    case SID_STYLE_SHOW:
                    {
                        nRetMask = ( NULL != pStyleSheet );
                        if ( pStyleSheet )
                        {
                            pStyleSheet->SetHidden( nSlotId == SID_STYLE_HIDE );
                            rBindings.Invalidate( SID_STYLE_FAMILY4 );
                            pDocSh->SetDocumentModified();
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_APPLY:
                    {
                        nRetMask = ( NULL != pStyleSheet );
                        if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                        {
                            ScUndoApplyPageStyle* pUndoAction = 0;
                            SCTAB nTabCount = pDoc->GetTableCount();
                            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
                            for (; itr != itrEnd && *itr < nTabCount; ++itr)
                            {
                                String aOldName = pDoc->GetPageStyle( *itr );
                                if ( aOldName != aStyleName )
                                {
                                    pDoc->SetPageStyle( *itr, aStyleName );
                                    ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(true), *itr ).UpdatePages();
                                    if( !pUndoAction )
                                        pUndoAction = new ScUndoApplyPageStyle( pDocSh, aStyleName );
                                    pUndoAction->AddSheetAction( *itr, aOldName );
                                }
                            }
                            if( pUndoAction )
                            {
                                pDocSh->GetUndoManager()->AddUndoAction( pUndoAction );
                                pDocSh->SetDocumentModified();
                                rBindings.Invalidate( SID_STYLE_FAMILY4 );
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );
                            }
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_NEW_BY_EXAMPLE:
                    {
                        const String& rStrCurStyle = pDoc->GetPageStyle( nCurTab );

                        if ( rStrCurStyle != aStyleName )
                        {
                            SfxStyleSheetBase*  pCurStyle = pStylePool->Find( rStrCurStyle, eFamily );
                            SfxItemSet          aAttrSet  = pCurStyle->GetItemSet();
                            SCTAB               nInTab;
                            bool                bUsed = pDoc->IsPageStyleInUse( aStyleName, &nInTab );

                            // wenn bereits vorhanden, erstmal entfernen...
                            if ( pStyleSheet )
                                pStylePool->Remove( pStyleSheet );

                            // ...und neu anlegen
                            pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                             SFXSTYLEBIT_USERDEF );

                            // Attribute uebernehmen
                            pStyleSheet->GetItemSet().Put( aAttrSet );
                            pDocSh->SetDocumentModified();

                            // wenn in Verwendung -> Update
                            if ( bUsed )
                                ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(true), nInTab ).UpdatePages();

                            aNewData.InitFromStyle( pStyleSheet );
                            bAddUndo = true;
                            rReq.Done();
                            nRetMask = true;
                        }
                    }
                    break;

                    default:
                        break;
                } // switch ( nSlotId )
            } // case SFX_STYLE_FAMILY_PAGE:
            break;

            default:
                break;
        } // switch ( eFamily )

        // Neu anlegen oder bearbeiten ueber Dialog:
        if ( nSlotId == SID_STYLE_NEW || nSlotId == SID_STYLE_EDIT )
        {
            if ( pStyleSheet )
            {
                SvxNumberInfoItem* pNumberInfoItem = NULL;

                SfxStyleFamily  eFam    = pStyleSheet->GetFamily();
                SfxAbstractTabDialog* pDlg    = NULL;
                sal_uInt16          nRsc    = 0;

                // alte Items aus der Vorlage merken
                SfxItemSet aOldSet = pStyleSheet->GetItemSet();
                String aOldName = pStyleSheet->GetName();

                switch ( eFam )
                {
                    case SFX_STYLE_FAMILY_PAGE:
                        nRsc = RID_SCDLG_STYLES_PAGE;
                        break;

                    case SFX_STYLE_FAMILY_PARA:
                    default:
                        {
                            SfxItemSet& rSet = pStyleSheet->GetItemSet();

                            const SfxPoolItem* pItem;
                            if ( rSet.GetItemState( ATTR_VALUE_FORMAT,
                                    false, &pItem ) == SFX_ITEM_SET )
                            {
                                // NumberFormat Value aus Value und Language
                                // erzeugen und eintueten
                                sal_uLong nFormat =
                                    ((SfxUInt32Item*)pItem)->GetValue();
                                LanguageType eLang =
                                    ((SvxLanguageItem*)&rSet.Get(
                                    ATTR_LANGUAGE_FORMAT ))->GetLanguage();
                                sal_uLong nLangFormat = pDoc->GetFormatTable()->
                                    GetFormatForLanguageIfBuiltIn( nFormat, eLang );
                                if ( nLangFormat != nFormat )
                                {
                                    SfxUInt32Item aNewItem( ATTR_VALUE_FORMAT, nLangFormat );
                                    rSet.Put( aNewItem );
                                    aOldSet.Put( aNewItem );
                                    // auch in aOldSet fuer Vergleich nach dem Dialog,
                                    // sonst geht evtl. eine Aenderung der Sprache verloren
                                }
                            }

                            pTabViewShell->MakeNumberInfoItem( pDoc, GetViewData(), &pNumberInfoItem );
                            pDocSh->PutItem( *pNumberInfoItem );
                            nRsc = RID_SCDLG_STYLES_PAR;

                            //  auf jeden Fall ein SvxBoxInfoItem mit Table = sal_False im Set:
                            //  (wenn gar kein Item da ist, loescht der Dialog auch das
                            //   BORDER_OUTER SvxBoxItem aus dem Vorlagen-Set)

                            if ( rSet.GetItemState( ATTR_BORDER_INNER, false ) != SFX_ITEM_SET )
                            {
                                SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
                                aBoxInfoItem.SetTable(false);       // keine inneren Linien
                                aBoxInfoItem.SetDist(true);
                                aBoxInfoItem.SetMinDist(false);
                                rSet.Put( aBoxInfoItem );
                            }
                        }
                        break;
                }

                //  If GetDefDialogParent is a dialog, it must be used
                //  (style catalog)

                Window* pParent = Application::GetDefDialogParent();
                if ( !pParent || !pParent->IsDialog() )
                {
                    //  GetDefDialogParent currently doesn't return the window
                    //  that was set with SetDefDialogParent (but dynamically finds the
                    //  topmost parent of the focus window), so IsDialog above is FALSE
                    //  even if called from the style catalog.
                    //  -> Use NULL if a modal dialog is open, to enable the Dialog's
                    //  default parent handling.
                    if ( Application::IsInModalMode() )
                        pParent = NULL;
                    else
                        pParent = pTabViewShell->GetDialogParent();
                }

                pTabViewShell->SetInFormatDialog(true);

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                pDlg = pFact->CreateScStyleDlg( pParent, *pStyleSheet, nRsc, nRsc );
                OSL_ENSURE(pDlg, "Dialog create fail!");
                short nResult = pDlg->Execute();
                pTabViewShell->SetInFormatDialog(false);

                if ( nResult == RET_OK )
                {
                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                    if ( pOutSet )
                    {
                        nRetMask = pStyleSheet->GetMask();

                        //  Attribut-Vergleiche (frueher in ModifyStyleSheet)
                        //  jetzt hier mit den alten Werten (Style ist schon veraendert)

                        if ( SFX_STYLE_FAMILY_PARA == eFam )
                        {
                            SfxItemSet& rNewSet = pStyleSheet->GetItemSet();
                            bool bNumFormatChanged;
                            if ( ScGlobal::CheckWidthInvalidate(
                                                bNumFormatChanged, aOldSet, rNewSet ) )
                                pDoc->InvalidateTextWidth( NULL, NULL, bNumFormatChanged );

                            SCTAB nTabCount = pDoc->GetTableCount();
                            for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                                if (pDoc->IsStreamValid(nTab))
                                    pDoc->SetStreamValid(nTab, false);

                            sal_uLong nOldFormat = ((const SfxUInt32Item&)aOldSet.
                                                    Get( ATTR_VALUE_FORMAT )).GetValue();
                            sal_uLong nNewFormat = ((const SfxUInt32Item&)rNewSet.
                                                    Get( ATTR_VALUE_FORMAT )).GetValue();
                            if ( nNewFormat != nOldFormat )
                            {
                                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                                const SvNumberformat* pOld = pFormatter->GetEntry( nOldFormat );
                                const SvNumberformat* pNew = pFormatter->GetEntry( nNewFormat );
                                if ( pOld && pNew && pOld->GetLanguage() != pNew->GetLanguage() )
                                    rNewSet.Put( SvxLanguageItem(
                                                    pNew->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
                            }

                            pDoc->GetPool()->CellStyleCreated( pStyleSheet->GetName() );
                        }
                        else
                        {
                            //! auch fuer Seitenvorlagen die Abfragen hier

                            String aNewName = pStyleSheet->GetName();
                            if ( aNewName != aOldName &&
                                    pDoc->RenamePageStyleInUse( aOldName, aNewName ) )
                            {
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );
                            }

                            pDoc->ModifyStyleSheet( *pStyleSheet, *pOutSet );
                            rBindings.Invalidate( FID_RESET_PRINTZOOM );
                        }

                        pDocSh->SetDocumentModified();

                        if ( SFX_STYLE_FAMILY_PARA == eFam )
                        {
                            pTabViewShell->UpdateNumberFormatter(
                                (const SvxNumberInfoItem&)
                                    *(pDocSh->GetItem(SID_ATTR_NUMBERFORMAT_INFO)) );

                            pTabViewShell->UpdateStyleSheetInUse( pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                        }

                        aNewData.InitFromStyle( pStyleSheet );
                        bAddUndo = true;
                    }
                }
                else
                {
                    if ( nSlotId == SID_STYLE_NEW )
                        pStylePool->Remove( pStyleSheet );
                    else
                    {
                        //  falls zwischendurch etwas mit dem temporaer geaenderten
                        //  ItemSet gepainted wurde:
                        pDocSh->PostPaintGridAll();
                    }
                }
                delete pDlg;
            }
        }

            rReq.SetReturnValue( SfxUInt16Item( nSlotId, nRetMask ) );

        if ( bAddUndo && bUndo)
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoModifyStyle( pDocSh, eFamily, aOldData, aNewData ) );

        if ( bStyleToMarked )
        {
            //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle,
            //  so redo will find the modified style
            pTabViewShell->SetStyleSheetToMarked( (SfxStyleSheet*)pStyleSheet );
            pTabViewShell->InvalidateAttribs();
        }

        if ( bListAction )
            pDocSh->GetUndoManager()->LeaveListAction();
    }
    else
    {
        OSL_FAIL( "Unknown slot (ScViewShell::ExecuteStyle)" );
    }
}

void ScFormatShell::ExecuteNumFormat( SfxRequest& rReq )
{
    ScModule*           pScMod      = SC_MOD();
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();
    SfxBindings& rBindings          = pTabViewShell->GetViewFrame()->GetBindings();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

                                    // Eingabe beenden
    if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
    {
        switch ( nSlot )
        {
            case SID_NUMBER_TWODEC:
            case SID_NUMBER_SCIENTIFIC:
            case SID_NUMBER_DATE:
            case SID_NUMBER_CURRENCY:
            case SID_NUMBER_PERCENT:
            case SID_NUMBER_STANDARD:
            case SID_NUMBER_FORMAT:
            case SID_NUMBER_INCDEC:
            case SID_NUMBER_DECDEC:
            case FID_DEFINE_NAME:
            case FID_ADD_NAME:
            case FID_USE_NAME:
            case FID_INSERT_NAME:
            case SID_SPELL_DIALOG:
            case SID_HANGUL_HANJA_CONVERSION:

            pScMod->InputEnterHandler();
            pTabViewShell->UpdateInputHandler();
            break;

            default:
            break;
        }
    }

    short nType = GetCurrentNumberFormatType();
    SfxItemSet aSet( GetPool(), nSlot, nSlot );
    switch ( nSlot )
    {
        case SID_NUMBER_TWODEC:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER, 4 );       // Standard+4 = #.##0,00
            rReq.Done();
            break;
        case SID_NUMBER_SCIENTIFIC:
            if ((nType & NUMBERFORMAT_SCIENTIFIC))
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER );
            else
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_SCIENTIFIC );
            aSet.Put( SfxBoolItem(nSlot, !(nType & NUMBERFORMAT_SCIENTIFIC)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_DATE:
            if ((nType & NUMBERFORMAT_DATE))
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER );
            else
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_DATE );
            aSet.Put( SfxBoolItem(nSlot, !(nType & NUMBERFORMAT_DATE)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_TIME:
            if ((nType & NUMBERFORMAT_TIME))
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER );
            else
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_TIME );
            aSet.Put( SfxBoolItem(nSlot, !(nType & NUMBERFORMAT_TIME)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_CURRENCY:
            if ((nType & NUMBERFORMAT_CURRENCY))
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER );
            else
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_CURRENCY );
            aSet.Put( SfxBoolItem(nSlot, !(nType & NUMBERFORMAT_CURRENCY)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_PERCENT:
            if ((nType & NUMBERFORMAT_PERCENT))
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER );
            else
                pTabViewShell->SetNumberFormat( NUMBERFORMAT_PERCENT );
            aSet.Put( SfxBoolItem(nSlot, !(nType & NUMBERFORMAT_PERCENT)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_STANDARD:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER );
            rReq.Done();
            break;
        case SID_NUMBER_INCDEC:
            pTabViewShell->ChangeNumFmtDecimals( true );
            rReq.Done();
            break;
        case SID_NUMBER_DECDEC:
            pTabViewShell->ChangeNumFmtDecimals( false );
            rReq.Done();
            break;

        case SID_NUMBER_FORMAT:
            //if ( pReqArgs )
            //{
            //  const SfxPoolItem* pItem;
            //  if(pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET)
            //  {
            //      String aCode = ((const SfxStringItem*)pItem)->GetValue();
            //      pTabViewShell->SetNumFmtByStr( aCode );
            //  }
            //}

            // symphony version with format interpretation
            if(pReqArgs)
            {
                const SfxPoolItem* pItem;
                ScDocument* pDoc = pViewData->GetDocument();
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                LanguageType eLanguage = ScGlobal::eLnge;
                sal_Int16 eType = -1;
                sal_uInt32 nCurrentNumberFormat;

                pDoc->GetNumberFormat(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo(), nCurrentNumberFormat);
                const SvNumberformat* pEntry = pFormatter->GetEntry(nCurrentNumberFormat);

                if(pEntry)
                {
                    eLanguage = pEntry->GetLanguage();
                    eType = pEntry->GetType();
                }

                //Just use eType to judge whether the command is fired for NUMBER/PERCENT/CURRENCY
                //In sidebar, users can fire SID_NUMBER_FORMAT command by operating the related UI controls before they are disable
                switch(eType)
                {
                case NUMBERFORMAT_ALL:
                case NUMBERFORMAT_NUMBER:
                case NUMBERFORMAT_NUMBER| NUMBERFORMAT_DEFINED:
                case NUMBERFORMAT_PERCENT:
                case NUMBERFORMAT_PERCENT| NUMBERFORMAT_DEFINED:
                case NUMBERFORMAT_CURRENCY:
                case NUMBERFORMAT_CURRENCY|NUMBERFORMAT_DEFINED:
                    eType = 0;
                    break;
                default:
                    eType =-1;
                }

                if(SFX_ITEM_SET == pReqArgs->GetItemState(nSlot, true, &pItem) && eType != -1)
                {
                    String aCode = ((const SfxStringItem*)pItem)->GetValue();
                    sal_uInt16 aLen = aCode.Len();
                    String* sFormat = new String[4];
                    String sTmpStr = OUString();
                    sal_uInt16 nCount(0);
                    sal_uInt16 nStrCount(0);

                    while(nCount < aLen)
                    {
                        sal_Unicode cChar = aCode.GetChar(nCount);

                        if(cChar == sal_Unicode(','))
                        {
                            sFormat[nStrCount] = sTmpStr;
                            sTmpStr = OUString();
                            nStrCount++;
                        }
                        else
                        {
                            sTmpStr += cChar;
                        }

                        nCount++;

                        if(nStrCount > 3)
                            break;
                    }

                    const sal_Bool bThousand = (sal_Bool)sFormat[0].ToInt32();
                    const sal_Bool bNegRed = (sal_Bool)sFormat[1].ToInt32();
                    const sal_uInt16 nPrecision = (sal_uInt16)sFormat[2].ToInt32();
                    const sal_uInt16 nLeadZeroes = (sal_uInt16)sFormat[3].ToInt32();

                    aCode = pFormatter->GenerateFormat(
                        nCurrentNumberFormat,//modify
                        eLanguage,
                        bThousand,
                        bNegRed,
                        nPrecision,
                        nLeadZeroes);
                    pTabViewShell->SetNumFmtByStr(aCode);
                    delete[] sFormat;
                }
            }
            break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( ATTR_VALUE_FORMAT, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    // We have to accomplish this using ApplyAttributes()
                    // because we also need the language information to be
                    // considered.
                    const SfxItemSet& rOldSet =
                        pTabViewShell->GetSelectionPattern()->GetItemSet();
                    SfxItemPool* pDocPool = GetViewData()->GetDocument()->GetPool();
                    SfxItemSet aNewSet( *pDocPool, ATTR_PATTERN_START, ATTR_PATTERN_END );
                    aNewSet.Put( *pItem );
                    pTabViewShell->ApplyAttributes( &aNewSet, &rOldSet, sal_True );
                }
            }
            break;

        case SID_NUMBER_TYPE_FORMAT:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    sal_uInt16 nFormat = ((SfxInt16Item *)pItem)->GetValue();
                    switch(nFormat)
                    {
                    case 0:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER); //Modify
                        break;
                    case 1:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER, 2 ); //Modify
                        break;
                    case 2:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_PERCENT );
                        break;
                    case 3:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_CURRENCY );
                        break;
                    case 4:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_DATE );
                        break;
                    case 5:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_TIME );
                        break;
                    case 6:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_SCIENTIFIC );
                        break;
                    case 7:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_FRACTION );
                        break;
                    case 8:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_LOGICAL );
                        break;
                    case 9:
                        pTabViewShell->SetNumberFormat( NUMBERFORMAT_TEXT );
                        break;
                    default:
                        ;
                    }
                    rReq.Done();
                }
            }
            break;

        default:
            OSL_FAIL("falscher Slot bei ExecuteEdit");
            break;
    }
}


//------------------------------------------------------------------

void ScFormatShell::ExecuteAlignment( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&            rBindings   = pViewData->GetBindings();
    const SfxItemSet*       pSet        = rReq.GetArgs();
    sal_uInt16                  nSlot       = rReq.GetSlot();

    pTabViewShell->HideListBox();   // Autofilter-DropDown-Listbox

    switch( nSlot )
    {
        // pseudo slots for Format menu
        case SID_ALIGN_ANY_HDEFAULT:
        case SID_ALIGN_ANY_LEFT:
        case SID_ALIGN_ANY_HCENTER:
        case SID_ALIGN_ANY_RIGHT:
        case SID_ALIGN_ANY_JUSTIFIED:
            pTabViewShell->ApplyAttr( SvxHorJustifyItem( lclConvertSlotToHAlign( nSlot ), ATTR_HOR_JUSTIFY ) );
        break;
        case SID_ALIGN_ANY_VDEFAULT:
        case SID_ALIGN_ANY_TOP:
        case SID_ALIGN_ANY_VCENTER:
        case SID_ALIGN_ANY_BOTTOM:
            pTabViewShell->ApplyAttr( SvxVerJustifyItem( lclConvertSlotToVAlign( nSlot ), ATTR_VER_JUSTIFY ) );
        break;

        default:
            if( pSet )
            {
                const SfxPoolItem* pItem = NULL;
                if( pSet->GetItemState(GetPool().GetWhich(nSlot), sal_True, &pItem  ) == SFX_ITEM_SET )
                {

                    switch ( nSlot )
                    {
                        case SID_ATTR_ALIGN_HOR_JUSTIFY:
                        case SID_ATTR_ALIGN_VER_JUSTIFY:
                        case SID_ATTR_ALIGN_INDENT:
                        case SID_ATTR_ALIGN_HYPHENATION:
                        case SID_ATTR_ALIGN_DEGREES:
                        case SID_ATTR_ALIGN_LOCKPOS:
                        case SID_ATTR_ALIGN_MARGIN:
                        case SID_ATTR_ALIGN_STACKED:
                            pTabViewShell->ApplyAttr( *pItem );
                        break;

                        case SID_H_ALIGNCELL:
                        {
                            SvxCellHorJustify eJust = (SvxCellHorJustify)((const SvxHorJustifyItem*)pItem)->GetValue();
                            // #i78476# update alignment of text in cell edit mode
                            pTabViewShell->UpdateInputHandlerCellAdjust( eJust );
                            pTabViewShell->ApplyAttr( SvxHorJustifyItem( eJust, ATTR_HOR_JUSTIFY ) );
                        }
                        break;
                        case SID_V_ALIGNCELL:
                            pTabViewShell->ApplyAttr( SvxVerJustifyItem( (SvxCellVerJustify)((const SvxVerJustifyItem*)pItem)->GetValue(), ATTR_VER_JUSTIFY ) );
                        break;
                        default:
                            OSL_FAIL( "ExecuteAlignment: invalid slot" );
                            return;
                    }
                }
            }
    }
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ALIGNTOP );
    rBindings.Invalidate( SID_ALIGNBOTTOM );
    rBindings.Invalidate( SID_ALIGNCENTERVER );
    rBindings.Invalidate( SID_V_ALIGNCELL );
    rBindings.Invalidate( SID_H_ALIGNCELL );
    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_HDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    rBindings.Invalidate( SID_ALIGN_ANY_VDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_TOP );
    rBindings.Invalidate( SID_ALIGN_ANY_VCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_BOTTOM );
    rBindings.Update();

    if( ! rReq.IsAPI() )
        rReq.Done();
}

void ScFormatShell::ExecuteTextAttr( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&            rBindings   = pViewData->GetBindings();
    const ScPatternAttr*    pAttrs      = pTabViewShell->GetSelectionPattern();
    const SfxItemSet*       pSet        = rReq.GetArgs();
    sal_uInt16                  nSlot       = rReq.GetSlot();
    SfxAllItemSet*          pNewSet = 0;

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    if (  (nSlot == SID_ATTR_CHAR_WEIGHT)
        ||(nSlot == SID_ATTR_CHAR_POSTURE)
        ||(nSlot == SID_ATTR_CHAR_UNDERLINE)
        ||(nSlot == SID_ULINE_VAL_NONE)
        ||(nSlot == SID_ULINE_VAL_SINGLE)
        ||(nSlot == SID_ULINE_VAL_DOUBLE)
        ||(nSlot == SID_ULINE_VAL_DOTTED) )
    {
        pNewSet = new SfxAllItemSet( GetPool() );

        switch ( nSlot )
        {
            case SID_ATTR_CHAR_WEIGHT:
            {
                // #i78017 establish the same behaviour as in Writer
                sal_uInt8 nScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;

                SfxItemPool& rPool = GetPool();
                SvxScriptSetItem aSetItem( nSlot, rPool );
                if ( pSet )
                    aSetItem.PutItemForScriptType( nScript, pSet->Get( ATTR_FONT_WEIGHT ) );
                else
                {
                    //  toggle manually

                    FontWeight eWeight = WEIGHT_BOLD;
                    SvxScriptSetItem aOldSetItem( nSlot, rPool );
                    aOldSetItem.GetItemSet().Put( pAttrs->GetItemSet(), false );
                    const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                    if ( pCore && ((const SvxWeightItem*)pCore)->GetWeight() == WEIGHT_BOLD )
                        eWeight = WEIGHT_NORMAL;

                    aSetItem.PutItemForScriptType( nScript, SvxWeightItem( eWeight, ATTR_FONT_WEIGHT ) );
                }
                pTabViewShell->ApplyUserItemSet( aSetItem.GetItemSet() );
                pNewSet->Put( aSetItem.GetItemSet(), false );
            }
            break;

            case SID_ATTR_CHAR_POSTURE:
            {
                // #i78017 establish the same behaviour as in Writer
                sal_uInt8 nScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;

                SfxItemPool& rPool = GetPool();
                SvxScriptSetItem aSetItem( nSlot, rPool );
                if ( pSet )
                    aSetItem.PutItemForScriptType( nScript, pSet->Get( ATTR_FONT_POSTURE ) );
                else
                {
                    //  toggle manually

                    FontItalic eItalic = ITALIC_NORMAL;
                    SvxScriptSetItem aOldSetItem( nSlot, rPool );
                    aOldSetItem.GetItemSet().Put( pAttrs->GetItemSet(), false );
                    const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                    if ( pCore && ((const SvxPostureItem*)pCore)->GetPosture() == ITALIC_NORMAL )
                        eItalic = ITALIC_NONE;

                    aSetItem.PutItemForScriptType( nScript, SvxPostureItem( eItalic, ATTR_FONT_POSTURE ) );
                }
                pTabViewShell->ApplyUserItemSet( aSetItem.GetItemSet() );
                pNewSet->Put( aSetItem.GetItemSet(), false );
            }
            break;

            case SID_ATTR_CHAR_UNDERLINE:
                {
                    FontUnderline       eUnderline;

                    if( pSet )
                    {
                        const SfxPoolItem& rUnderline = pSet->Get( ATTR_FONT_UNDERLINE );

                        if( rUnderline.ISA(SvxUnderlineItem) )
                        {
                            pTabViewShell->ApplyAttr( rUnderline );
                            pNewSet->Put( rUnderline,rUnderline.Which() );
                        }
                        else if ( rUnderline.ISA(SvxTextLineItem) )
                        {
                            // #i106580# also allow SvxTextLineItem (base class of SvxUnderlineItem)
                            const SvxTextLineItem& rTextLineItem = static_cast<const SvxTextLineItem&>(rUnderline);
                            SvxUnderlineItem aNewItem( rTextLineItem.GetLineStyle(), rTextLineItem.Which() );
                            aNewItem.SetColor( rTextLineItem.GetColor() );
                            pTabViewShell->ApplyAttr( aNewItem );
                            pNewSet->Put( aNewItem, aNewItem.Which() );
                        }
                    }
                    else
                    {
                        SvxUnderlineItem aUnderline( (const SvxUnderlineItem&)
                                                        pAttrs->GetItem(
                                                            ATTR_FONT_UNDERLINE ) );
                        eUnderline = (UNDERLINE_NONE != aUnderline.GetLineStyle())
                                    ? UNDERLINE_NONE
                                    : UNDERLINE_SINGLE;
                        aUnderline.SetLineStyle( eUnderline );
                        pTabViewShell->ApplyAttr( aUnderline );
                        pNewSet->Put( aUnderline,aUnderline.Which() );
                    }
                }
                break;

            case SID_ULINE_VAL_NONE:
                pTabViewShell->ApplyAttr( SvxUnderlineItem( UNDERLINE_NONE, ATTR_FONT_UNDERLINE ) );
                break;
            case SID_ULINE_VAL_SINGLE:      // Toggles
            case SID_ULINE_VAL_DOUBLE:
            case SID_ULINE_VAL_DOTTED:
                {
                    FontUnderline eOld = ((const SvxUnderlineItem&)
                                            pAttrs->GetItem(ATTR_FONT_UNDERLINE)).GetLineStyle();
                    FontUnderline eNew = eOld;
                    switch (nSlot)
                    {
                        case SID_ULINE_VAL_SINGLE:
                            eNew = ( eOld == UNDERLINE_SINGLE ) ? UNDERLINE_NONE : UNDERLINE_SINGLE;
                            break;
                        case SID_ULINE_VAL_DOUBLE:
                            eNew = ( eOld == UNDERLINE_DOUBLE ) ? UNDERLINE_NONE : UNDERLINE_DOUBLE;
                            break;
                        case SID_ULINE_VAL_DOTTED:
                            eNew = ( eOld == UNDERLINE_DOTTED ) ? UNDERLINE_NONE : UNDERLINE_DOTTED;
                            break;
                    }
                    pTabViewShell->ApplyAttr( SvxUnderlineItem( eNew, ATTR_FONT_UNDERLINE ) );
                }
                break;

            default:
                break;
        }
        rBindings.Invalidate( nSlot );
    }
    else
    {
        /*
         * "Selbstgemachte" RadioButton-Funktionalitaet
         * Beim Toggle gibt es den Standard-State, d.h. kein
         * Button ist gedrueckt
         */

        const SfxItemSet&        rAttrSet   = pTabViewShell->GetSelectionPattern()->GetItemSet();
        const SfxPoolItem*       pItem       = NULL;
        const SvxHorJustifyItem* pHorJustify = NULL;
        const SvxVerJustifyItem* pVerJustify = NULL;
        SvxCellHorJustify        eHorJustify = SVX_HOR_JUSTIFY_STANDARD;
        SvxCellVerJustify        eVerJustify = SVX_VER_JUSTIFY_STANDARD;

        if (rAttrSet.GetItemState(ATTR_HOR_JUSTIFY, sal_True,&pItem ) == SFX_ITEM_SET)
        {
            pHorJustify = (const SvxHorJustifyItem*)pItem;
            eHorJustify = SvxCellHorJustify( pHorJustify->GetValue() );
        }
        if (rAttrSet.GetItemState(ATTR_VER_JUSTIFY, sal_True,&pItem ) == SFX_ITEM_SET)
        {
            pVerJustify = (const SvxVerJustifyItem*)pItem;
            eVerJustify = SvxCellVerJustify( pVerJustify->GetValue() );
        }

        switch ( nSlot )
        {
            case SID_ALIGNLEFT:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_LEFT) ?
                    SVX_HOR_JUSTIFY_LEFT : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
                //break;

            case SID_ALIGNRIGHT:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_RIGHT) ?
                    SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
                //break;

            case SID_ALIGNCENTERHOR:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_CENTER) ?
                    SVX_HOR_JUSTIFY_CENTER : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
                //break;

            case SID_ALIGNBLOCK:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_BLOCK) ?
                    SVX_HOR_JUSTIFY_BLOCK : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
                //break;

            case SID_ALIGNTOP:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_TOP) ?
                    SVX_VER_JUSTIFY_TOP : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
                //break;

            case SID_ALIGNBOTTOM:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_BOTTOM) ?
                    SVX_VER_JUSTIFY_BOTTOM : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
                //break;

            case SID_ALIGNCENTERVER:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_CENTER) ?
                    SVX_VER_JUSTIFY_CENTER : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
                //break;

            default:
            break;
        }

    }

    rBindings.Update();

    if( pNewSet )
    {
        rReq.Done( *pNewSet );
        delete pNewSet;
    }
    else
    {
        rReq.Done();
    }

}

//------------------------------------------------------------------

void ScFormatShell::ExecuteAttr( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&        rBindings = pViewData->GetBindings();
    const SfxItemSet*   pNewAttrs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox
    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( !pNewAttrs )
    {
        switch ( nSlot )
        {
            case SID_ATTR_CHAR_ENDPREVIEW_FONT:
            {
                pDoc->SetPreviewFont(NULL);
                pTabViewShell->UpdateSelectionArea( pDoc->GetPreviewSelection() );
                break;
            }
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
                pTabViewShell->ExecuteCellFormatDlg(rReq, "font");       // wenn ToolBar vertikal
                break;

            case SID_BACKGROUND_COLOR:
                {
                    SvxBrushItem aBrushItem( (const SvxBrushItem&)
                                     pTabViewShell->GetSelectionPattern()->GetItem( ATTR_BACKGROUND ) );
                    aBrushItem.SetColor( COL_TRANSPARENT );
                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;

            case SID_ATTR_ALIGN_LINEBREAK:                  // ohne Parameter als Toggle
                {
                    const ScPatternAttr* pAttrs = pTabViewShell->GetSelectionPattern();
                    bool bOld = ((const SfxBoolItem&)pAttrs->GetItem(ATTR_LINEBREAK)).GetValue();
                    SfxBoolItem aBreakItem( ATTR_LINEBREAK, !bOld );
                    pTabViewShell->ApplyAttr( aBreakItem );

                    SfxAllItemSet aNewSet( GetPool() );
                    aNewSet.Put( aBreakItem,aBreakItem.Which() );
                    rReq.Done( aNewSet );

                    rBindings.Invalidate( nSlot );
                }
                break;
        }
    }
    else
    {
        switch ( nSlot )
        {
            case SID_ATTR_CHAR_PREVIEW_FONT:
            {
                SfxItemPool& rPool = GetPool();
                sal_uInt16 nWhich = rPool.GetWhich( nSlot );
                const SvxFontItem& rFont = ( const SvxFontItem&)pNewAttrs->Get( nWhich );
                SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, rPool );
                sal_uInt8 nScript = pTabViewShell->GetSelectionScriptType();
                aSetItem.PutItemForScriptType( nScript, rFont );

                ScMarkData aFuncMark( pViewData->GetMarkData() );
                ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
                pDoc->SetPreviewFont( aSetItem.GetItemSet().Clone() );
                aFuncMark.MarkToMulti();

                if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
                {
                    SCCOL nCol = pViewData->GetCurX();
                    SCROW nRow = pViewData->GetCurY();
                    SCTAB nTab = pViewData->GetTabNo();
                    ScRange aRange( nCol, nRow, nTab );
                    aFuncMark.SetMarkArea( aRange );
                }
                pDoc->SetPreviewSelection( aFuncMark );
                pTabViewShell->UpdateSelectionArea( aFuncMark );
                break;
            }
            case SID_ATTR_CHAR_OVERLINE:
            case SID_ATTR_CHAR_STRIKEOUT:
            case SID_ATTR_ALIGN_LINEBREAK:
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_CONTOUR:
            case SID_ATTR_CHAR_SHADOWED:
            case SID_ATTR_CHAR_RELIEF:
            case SID_SCATTR_PROTECTION :
                pTabViewShell->ApplyAttr( pNewAttrs->Get( pNewAttrs->GetPool()->GetWhich( nSlot ) ) );
                rBindings.Invalidate( nSlot );
                rBindings.Update( nSlot );
                break;

            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
                {
                    // #i78017 establish the same behaviour as in Writer
                    sal_uInt8 nScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
                    if (nSlot == SID_ATTR_CHAR_FONT)
                        nScript = pTabViewShell->GetSelectionScriptType();

                    SfxItemPool& rPool = GetPool();
                    SvxScriptSetItem aSetItem( nSlot, rPool );
                    sal_uInt16 nWhich = rPool.GetWhich( nSlot );
                    aSetItem.PutItemForScriptType( nScript, pNewAttrs->Get( nWhich ) );

                    pTabViewShell->ApplyUserItemSet( aSetItem.GetItemSet() );

                    rBindings.Invalidate( nSlot );
                    rBindings.Update( nSlot );
                }
                break;

            case SID_FRAME_LINESTYLE:
                {
                    // Default-Linie aktualisieren
                    const ::editeng::SvxBorderLine* pLine =
                            ((const SvxLineItem&)
                                pNewAttrs->Get( SID_FRAME_LINESTYLE )).
                                GetLine();

                    if ( pLine )
                    {
                        ::editeng::SvxBorderLine* pDefLine = pTabViewShell->GetDefaultFrameLine();

                        if ( pDefLine )
                        {
                            pDefLine->SetBorderLineStyle(
                                    pLine->GetBorderLineStyle());
                            pDefLine->SetWidth( pLine->GetWidth( ) );
                            pTabViewShell->SetSelectionFrameLines( pDefLine, false );
                        }
                        else
                        {
                            pTabViewShell->SetDefaultFrameLine( pLine );
                            pTabViewShell->GetDefaultFrameLine()->SetColor( COL_BLACK );
                            pTabViewShell->SetSelectionFrameLines( pLine, false );
                        }
                    }
                    else
                    {
                        Color           aColorBlack( COL_BLACK );
                        ::editeng::SvxBorderLine aDefLine( &aColorBlack, 20,
                                table::BorderLineStyle::SOLID );
                        pTabViewShell->SetDefaultFrameLine( &aDefLine );
                        pTabViewShell->SetSelectionFrameLines( NULL, false );
                    }
                }
                break;

            case SID_FRAME_LINECOLOR:
                {
                    ::editeng::SvxBorderLine*  pDefLine = pTabViewShell->GetDefaultFrameLine();
                    const Color&    rColor = ((const SvxColorItem&)
                                        pNewAttrs->Get( SID_FRAME_LINECOLOR )).
                                            GetValue();

                    // Default-Linie aktualisieren
                    if ( pDefLine )
                    {
                        pDefLine->SetColor( rColor );
                        pTabViewShell->SetSelectionFrameLines( pDefLine, sal_True );
                    }
                    else
                    {
                        ::editeng::SvxBorderLine aDefLine( &rColor, 20,
                                table::BorderLineStyle::SOLID );
                        pTabViewShell->SetDefaultFrameLine( &aDefLine );
                        pTabViewShell->SetSelectionFrameLines( &aDefLine, false );
                    }
                }
                break;

            case SID_ATTR_BORDER_OUTER:
            case SID_ATTR_BORDER:
                {
                    ::editeng::SvxBorderLine*          pDefLine = pTabViewShell->GetDefaultFrameLine();
                    const ScPatternAttr*    pOldAttrs = pTabViewShell->GetSelectionPattern();
                    SfxItemSet*             pOldSet =
                                                new SfxItemSet(
                                                        *(pDoc->GetPool()),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END );
                    SfxItemSet*             pNewSet =
                                                new SfxItemSet(
                                                        *(pDoc->GetPool()),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END );
                    const SfxPoolItem&      rBorderAttr =
                                                pOldAttrs->GetItemSet().
                                                    Get( ATTR_BORDER );

                    // Border-Items vom Controller auswerten:
                    const SfxPoolItem* pItem = 0;

                    if ( pNewAttrs->GetItemState( ATTR_BORDER, sal_True, &pItem )
                         == SFX_ITEM_SET )
                    {
                        //  The SvxFrameToolBoxControl toolbox controller uses a default
                        //  SvxBorderLine (all widths 0) to mark the lines that should be set.
                        //  Macro recording uses a SvxBoxItem with the real values (OutWidth > 0)
                        //  or NULL pointers for no lines.
                        //  -> Substitute existing lines with pDefLine only if widths are 0.
                        SvxBoxItem aBoxItem ( *(const SvxBoxItem*)pItem );
                        if ( aBoxItem.GetTop() && aBoxItem.GetTop()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_TOP );
                        if ( aBoxItem.GetBottom() && aBoxItem.GetBottom()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_BOTTOM );
                        if ( aBoxItem.GetLeft() && aBoxItem.GetLeft()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_LEFT );
                        if ( aBoxItem.GetRight() && aBoxItem.GetRight()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_RIGHT );
                        pNewSet->Put( aBoxItem );
                        rReq.AppendItem( aBoxItem );
                    }

                    if ( pNewAttrs->GetItemState( ATTR_BORDER_INNER, sal_True, &pItem )
                         == SFX_ITEM_SET )
                    {
                        SvxBoxInfoItem aBoxInfoItem( *(const SvxBoxInfoItem*)pItem );
                        if ( aBoxInfoItem.GetHori() && aBoxInfoItem.GetHori()->GetOutWidth() == 0 )
                            aBoxInfoItem.SetLine( pDefLine, BOXINFO_LINE_HORI );
                        if ( aBoxInfoItem.GetVert() && aBoxInfoItem.GetVert()->GetOutWidth() == 0 )
                            aBoxInfoItem.SetLine( pDefLine, BOXINFO_LINE_VERT );
                        pNewSet->Put( aBoxInfoItem );
                        rReq.AppendItem( aBoxInfoItem );
                    }
                    else
                    {
                        SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
                        aBoxInfoItem.SetLine( NULL, BOXINFO_LINE_HORI );
                        aBoxInfoItem.SetLine( NULL, BOXINFO_LINE_VERT );
                        pNewSet->Put( aBoxInfoItem );
                    }

                    pOldSet->Put( rBorderAttr );
                    pTabViewShell->ApplyAttributes( pNewSet, pOldSet );

                    delete pOldSet;
                    delete pNewSet;
                }
                break;

            case SID_ATTR_BORDER_DIAG_TLBR:
            case SID_ATTR_BORDER_DIAG_BLTR:
                {
                    const ScPatternAttr* pOldAttrs = pTabViewShell->GetSelectionPattern();
                    SfxItemSet* pOldSet = new SfxItemSet(pOldAttrs->GetItemSet());
                    SfxItemSet* pNewSet = new SfxItemSet(pOldAttrs->GetItemSet());
                    const SfxPoolItem* pItem = 0;

                    if(SID_ATTR_BORDER_DIAG_TLBR == nSlot)
                    {
                        if(SFX_ITEM_SET == pNewAttrs->GetItemState(ATTR_BORDER_TLBR, true, &pItem))
                        {
                            SvxLineItem aItem(ATTR_BORDER_TLBR);
                            aItem.SetLine(((const SvxLineItem&)pNewAttrs->Get(ATTR_BORDER_TLBR)).GetLine());
                            pNewSet->Put(aItem);
                            rReq.AppendItem(aItem);
                            pTabViewShell->ApplyAttributes(pNewSet, pOldSet);
                        }
                    }
                    else // if( nSlot == SID_ATTR_BORDER_DIAG_BLTR )
                    {
                        if(SFX_ITEM_SET == pNewAttrs->GetItemState(ATTR_BORDER_BLTR, true, &pItem ))
                        {
                            SvxLineItem aItem(ATTR_BORDER_BLTR);
                            aItem.SetLine(((const SvxLineItem&)pNewAttrs->Get(ATTR_BORDER_BLTR)).GetLine());
                            pNewSet->Put(aItem);
                            rReq.AppendItem(aItem);
                            pTabViewShell->ApplyAttributes(pNewSet, pOldSet);
                        }
                    }

                    delete pOldSet;
                    delete pNewSet;
                    rBindings.Invalidate(nSlot);
                }
                break;

            // ATTR_BACKGROUND (=SID_ATTR_BRUSH) muss ueber zwei IDs
            // gesetzt werden:
            case SID_BACKGROUND_COLOR:
                {
                    const SvxColorItem  rNewColorItem = (const SvxColorItem&)
                                            pNewAttrs->Get( SID_BACKGROUND_COLOR );

                    SvxBrushItem        aBrushItem( (const SvxBrushItem&)
                                            pTabViewShell->GetSelectionPattern()->
                                                GetItem( ATTR_BACKGROUND ) );

                    aBrushItem.SetColor( rNewColorItem.GetValue() );

                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;

                case SID_ATTR_BRUSH:
                {
                    SvxBrushItem        aBrushItem( (const SvxBrushItem&)
                                            pTabViewShell->GetSelectionPattern()->
                                                GetItem( ATTR_BACKGROUND ) );
                    const SvxBrushItem& rNewBrushItem = (const SvxBrushItem&)
                                            pNewAttrs->Get( GetPool().GetWhich(nSlot) );
                    aBrushItem.SetColor(rNewBrushItem.GetColor());
                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;

            case SID_ATTR_BORDER_SHADOW:
                {
                    const SvxShadowItem& rNewShadowItem = (const SvxShadowItem&)
                                            pNewAttrs->Get( ATTR_SHADOW );
                    pTabViewShell->ApplyAttr( rNewShadowItem );
                }
                break;

            default:
            break;
        }

        if( ! rReq.IsAPI() )
            if( ! rReq.IsDone() )
                rReq.Done();
    }
}

void ScFormatShell::GetAttrState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet&    rAttrSet   = pTabViewShell->GetSelectionPattern()->GetItemSet();
    //const ::editeng::SvxBorderLine* pLine      = pTabViewShell->GetDefaultFrameLine();
    const SvxBrushItem&  rBrushItem = (const SvxBrushItem&)rAttrSet.Get( ATTR_BACKGROUND );
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    rSet.Put( rAttrSet, false );

    //  choose font info according to selection script type
    sal_uInt8 nScript = 0;      // GetSelectionScriptType never returns 0
    if ( rSet.GetItemState( ATTR_FONT ) != SFX_ITEM_UNKNOWN )
    {
        if (!nScript) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT, nScript );
    }
    if ( rSet.GetItemState( ATTR_FONT_HEIGHT ) != SFX_ITEM_UNKNOWN )
    {
        if (!nScript) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_HEIGHT, nScript );
    }

    while ( nWhich )
    {
        switch(nWhich)
        {
            case SID_BACKGROUND_COLOR:
            {
                rSet.Put( SvxColorItem( rBrushItem.GetColor(), SID_BACKGROUND_COLOR ) );

                if(SFX_ITEM_DONTCARE == rAttrSet.GetItemState(ATTR_BACKGROUND))
                {
                    rSet.InvalidateItem(SID_BACKGROUND_COLOR);
                }
            }
            break;
            case SID_FRAME_LINESTYLE:
            case SID_FRAME_LINECOLOR:
            {
                // handled together because both need the cell border information for decisions
                // rSet.Put( SvxColorItem( pLine ? pLine->GetColor() : Color(), SID_FRAME_LINECOLOR ) );
                Color aCol = 0;
                editeng::SvxBorderLine aLine(0,0,0,0);
                bool bCol = 0;
                bool bColDisable = 0, bStyleDisable = 0;
                SvxBoxItem aBoxItem(ATTR_BORDER);
                SvxBoxInfoItem aInfoItem(ATTR_BORDER_INNER);

                pTabViewShell->GetSelectionFrame(aBoxItem, aInfoItem);

                if( aBoxItem.GetTop() )
                {
                    bCol = 1;
                    aCol = aBoxItem.GetTop()->GetColor() ;
                    aLine.SetColor(aCol);
                    aLine.SetWidth( aBoxItem.GetTop()->GetWidth());
                    aLine.SetBorderLineStyle( aBoxItem.GetTop()->GetBorderLineStyle());
                }

                if( aBoxItem.GetBottom() )
                {
                    if(bCol == 0)
                    {
                        bCol = 1;
                        aCol = aBoxItem.GetBottom()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem.GetBottom()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem.GetBottom()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem.GetBottom()->GetColor() )
                            bColDisable = 1;
                        if(!( aLine == *(aBoxItem.GetBottom())) )
                            bStyleDisable = 1;
                    }
                }

                if( aBoxItem.GetLeft() )
                {
                    if(bCol == 0)
                    {
                        bCol = 1;
                        aCol = aBoxItem.GetLeft()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem.GetLeft()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem.GetLeft()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem.GetLeft()->GetColor() )
                            bColDisable = 1;
                        if(!( aLine == *(aBoxItem.GetLeft())) )
                            bStyleDisable = 1;
                    }
                }

                if( aBoxItem.GetRight() )
                {
                    if(bCol == 0)
                    {
                        bCol = 1;
                        aCol = aBoxItem.GetRight()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem.GetRight()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem.GetRight()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem.GetRight()->GetColor() )
                            bColDisable = 1;
                        if(!( aLine == *(aBoxItem.GetRight())) )
                            bStyleDisable = 1;
                    }
                }

                if( aInfoItem.GetVert())
                {
                    if(bCol == 0)
                    {
                        bCol = 1;
                        aCol = aInfoItem.GetVert()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aInfoItem.GetVert()->GetWidth());
                        aLine.SetBorderLineStyle( aInfoItem.GetVert()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aInfoItem.GetVert()->GetColor() )
                            bColDisable = 1;
                        if(!( aLine == *(aInfoItem.GetVert())) )
                            bStyleDisable = 1;
                    }
                }

                if( aInfoItem.GetHori())
                {
                    if(bCol == 0)
                    {
                        bCol = 1;
                        aCol = aInfoItem.GetHori()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aInfoItem.GetHori()->GetWidth());
                        aLine.SetBorderLineStyle( aInfoItem.GetHori()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aInfoItem.GetHori()->GetColor() )
                            bColDisable = 1;
                        if(!( aLine == *(aInfoItem.GetHori())) )
                            bStyleDisable = 1;
                    }
                }

                if( !aInfoItem.IsValid( VALID_VERT )
                    || !aInfoItem.IsValid( VALID_HORI )
                    || !aInfoItem.IsValid( VALID_LEFT )
                    || !aInfoItem.IsValid( VALID_RIGHT )
                    || !aInfoItem.IsValid( VALID_TOP )
                    || !aInfoItem.IsValid( VALID_BOTTOM ) )
                {
                    bColDisable = 1;
                    bStyleDisable = 1;
                }

                if(SID_FRAME_LINECOLOR == nWhich)
                {
                    if(bColDisable) // if different lines have differernt colors
                    {
                        aCol = COL_TRANSPARENT;
                        rSet.Put( SvxColorItem(aCol, SID_FRAME_LINECOLOR ) );
                        rSet.InvalidateItem(SID_FRAME_LINECOLOR);
                    }
                    else if( bCol == 0 && bColDisable == 0) // if no line available
                    {
                        aCol = COL_AUTO;
                        rSet.Put( SvxColorItem(aCol, SID_FRAME_LINECOLOR ) );
                    }
                    else
                        rSet.Put( SvxColorItem(aCol, SID_FRAME_LINECOLOR ) );
                }
                else // if( nWhich == SID_FRAME_LINESTYLE)
                {
                    if(bStyleDisable) // if have several lines but don't have same style
                    {
                        aLine.SetWidth( 1 );
                        SvxLineItem aItem(SID_FRAME_LINESTYLE);
                        aItem.SetLine(&aLine);
                        rSet.Put( aItem );
                        rSet.InvalidateItem(SID_FRAME_LINESTYLE);
                    }
                    else // all the lines have same style or no line availavle, use initial value (0,0,0,0)
                    {
                        SvxLineItem aItem(SID_FRAME_LINESTYLE);
                        aItem.SetLine(&aLine);
                        rSet.Put( aItem );
                    }
                }
            }
            break;
            case SID_ATTR_BRUSH:
            {
                rSet.Put( rBrushItem, GetPool().GetWhich(nWhich) );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }

    // stuff for sidebar panels
    Invalidate(SID_ATTR_ALIGN_DEGREES);
    Invalidate(SID_ATTR_ALIGN_STACKED);
}

//------------------------------------------------------------------

void ScFormatShell::GetTextAttrState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
    rSet.Put( rAttrSet, false ); // ItemStates mitkopieren

    //  choose font info according to selection script type
    sal_uInt8 nScript = 0;      // GetSelectionScriptType never returns 0
    if ( rSet.GetItemState( ATTR_FONT_WEIGHT ) != SFX_ITEM_UNKNOWN )
    {
        if (!nScript) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_WEIGHT, nScript );
    }
    if ( rSet.GetItemState( ATTR_FONT_POSTURE ) != SFX_ITEM_UNKNOWN )
    {
        if (!nScript) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_POSTURE, nScript );
    }

    SfxItemState eState;
//  const SfxPoolItem* pItem;

    //--------------------------------------------------------------------
    // eigene Kontrolle ueber RadioButton-Funktionalitaet:
    //--------------------------------------------------------------------
    // Unterstreichung
    //------------------------

    eState = rAttrSet.GetItemState( ATTR_FONT_UNDERLINE, sal_True );
    if ( eState == SFX_ITEM_DONTCARE )
    {
        rSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontUnderline eUnderline = ((const SvxUnderlineItem&)
                    rAttrSet.Get(ATTR_FONT_UNDERLINE)).GetLineStyle();
        sal_uInt16 nId = SID_ULINE_VAL_NONE;
        switch (eUnderline)
        {
            case UNDERLINE_SINGLE:  nId = SID_ULINE_VAL_SINGLE; break;
            case UNDERLINE_DOUBLE:  nId = SID_ULINE_VAL_DOUBLE; break;
            case UNDERLINE_DOTTED:  nId = SID_ULINE_VAL_DOTTED; break;
            default:
                break;
        }
        rSet.Put( SfxBoolItem( nId, sal_True ) );
    }

    //------------------------
    // horizontale Ausrichtung
    //------------------------

    const SvxHorJustifyItem* pHorJustify = NULL;
    const SvxVerJustifyItem* pVerJustify = NULL;
    SvxCellVerJustify        eVerJustify = SVX_VER_JUSTIFY_STANDARD;
    sal_uInt16                   nWhich      = 0;
    bool                     bJustifyStd = false;
    SfxBoolItem              aBoolItem   ( 0, sal_True );

    eState   = rAttrSet.GetItemState( ATTR_HOR_JUSTIFY, sal_True,
                                        (const SfxPoolItem**)&pHorJustify );
    switch ( eState )
    {
        case SFX_ITEM_SET:
            {
                switch ( SvxCellHorJustify( pHorJustify->GetValue() ) )
                {
                    case SVX_HOR_JUSTIFY_STANDARD:
                        break;

                    case SVX_HOR_JUSTIFY_LEFT:
                        nWhich = SID_ALIGNLEFT;
                        break;

                    case SVX_HOR_JUSTIFY_RIGHT:
                        nWhich = SID_ALIGNRIGHT;
                        break;

                    case SVX_HOR_JUSTIFY_CENTER:
                        nWhich = SID_ALIGNCENTERHOR;
                        break;

                    case SVX_HOR_JUSTIFY_BLOCK:
                        nWhich = SID_ALIGNBLOCK;
                        break;

                    case SVX_HOR_JUSTIFY_REPEAT:
                    default:
                        bJustifyStd = sal_True;
                        break;
                }
            }
            break;

        case SFX_ITEM_DONTCARE:
            rSet.InvalidateItem( SID_ALIGNLEFT );
            rSet.InvalidateItem( SID_ALIGNRIGHT );
            rSet.InvalidateItem( SID_ALIGNCENTERHOR );
            rSet.InvalidateItem( SID_ALIGNBLOCK );
            break;

        default:
            bJustifyStd = sal_True;
            break;
    }

    if ( nWhich )
    {
        aBoolItem.SetWhich( nWhich );
        rSet.Put( aBoolItem );
    }
    else if ( bJustifyStd )
    {
        aBoolItem.SetValue( false );
        aBoolItem.SetWhich( SID_ALIGNLEFT );      rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNRIGHT );     rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNCENTERHOR ); rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNBLOCK );     rSet.Put( aBoolItem );
        bJustifyStd = false;
    }

    //------------------------
    // vertikale Ausrichtung
    //------------------------

    nWhich = 0;
    aBoolItem.SetValue( sal_True );

    eState = rAttrSet.GetItemState( ATTR_VER_JUSTIFY, sal_True,
                                    (const SfxPoolItem**)&pVerJustify );

    switch ( eState )
    {
        case SFX_ITEM_SET:
            {
                eVerJustify = SvxCellVerJustify( pVerJustify->GetValue() );

                switch ( eVerJustify )
                {
                    case SVX_VER_JUSTIFY_TOP:
                        nWhich = SID_ALIGNTOP;
                        break;

                    case SVX_VER_JUSTIFY_BOTTOM:
                        nWhich = SID_ALIGNBOTTOM;
                        break;

                    case SVX_VER_JUSTIFY_CENTER:
                        nWhich = SID_ALIGNCENTERVER;
                        break;

                    case SVX_VER_JUSTIFY_STANDARD:
                    default:
                        bJustifyStd = sal_True;
                        break;
                }
            }
            break;

        case SFX_ITEM_DONTCARE:
            rSet.InvalidateItem( SID_ALIGNTOP );
            rSet.InvalidateItem( SID_ALIGNBOTTOM );
            rSet.InvalidateItem( SID_ALIGNCENTERVER );
            break;

        default:
            bJustifyStd = sal_True;
            break;
    }

    if ( nWhich )
    {
        aBoolItem.SetWhich( nWhich );
        rSet.Put( aBoolItem );
    }
    else if ( bJustifyStd )
    {
        aBoolItem.SetValue( false );
        aBoolItem.SetWhich( SID_ALIGNTOP );       rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNBOTTOM );    rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNCENTERVER ); rSet.Put( aBoolItem );
    }
}


//------------------------------------------------------------------

void ScFormatShell::GetBorderState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    SvxBoxItem      aBoxItem( ATTR_BORDER );
    SvxBoxInfoItem  aInfoItem( ATTR_BORDER_INNER );

    pTabViewShell->GetSelectionFrame( aBoxItem, aInfoItem );

    if ( rSet.GetItemState( ATTR_BORDER ) != SFX_ITEM_UNKNOWN )
        rSet.Put( aBoxItem );
    if ( rSet.GetItemState( ATTR_BORDER_INNER ) != SFX_ITEM_UNKNOWN )
        rSet.Put( aInfoItem );
}

//------------------------------------------------------------------

void ScFormatShell::GetAlignState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet& rAttrSet    = pTabViewShell->GetSelectionPattern()->GetItemSet();
    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    SvxCellHorJustify eHAlign = SVX_HOR_JUSTIFY_STANDARD;
    bool bHasHAlign = rAttrSet.GetItemState( ATTR_HOR_JUSTIFY ) != SFX_ITEM_DONTCARE;
    if( bHasHAlign )
        eHAlign = (SvxCellHorJustify)((const SvxHorJustifyItem&) rAttrSet.Get( ATTR_HOR_JUSTIFY )).GetValue();

    SvxCellVerJustify eVAlign = SVX_VER_JUSTIFY_STANDARD;
    bool bHasVAlign = rAttrSet.GetItemState( ATTR_VER_JUSTIFY ) != SFX_ITEM_DONTCARE;
    if( bHasVAlign )
        eVAlign = (SvxCellVerJustify)((const SvxVerJustifyItem&) rAttrSet.Get( ATTR_VER_JUSTIFY )).GetValue();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_H_ALIGNCELL:
                if ( bHasHAlign )
                    rSet.Put( SvxHorJustifyItem( eHAlign, nWhich ));
            break;
            case SID_V_ALIGNCELL:
                if ( bHasVAlign )
                    rSet.Put( SvxVerJustifyItem( eVAlign, nWhich ));
            break;

            // pseudo slots for Format menu
            case SID_ALIGN_ANY_HDEFAULT:
            case SID_ALIGN_ANY_LEFT:
            case SID_ALIGN_ANY_HCENTER:
            case SID_ALIGN_ANY_RIGHT:
            case SID_ALIGN_ANY_JUSTIFIED:
                rSet.Put( SfxBoolItem( nWhich, bHasHAlign && (eHAlign == lclConvertSlotToHAlign( nWhich )) ) );
            break;
            case SID_ALIGN_ANY_VDEFAULT:
            case SID_ALIGN_ANY_TOP:
            case SID_ALIGN_ANY_VCENTER:
            case SID_ALIGN_ANY_BOTTOM:
                rSet.Put( SfxBoolItem( nWhich, bHasVAlign && (eVAlign == lclConvertSlotToVAlign( nWhich )) ) );
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::GetNumFormatState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScDocument* pDoc                = pViewData->GetDocument();
    short nType                     = GetCurrentNumberFormatType();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_NUMBER_FORMAT:
                //{
                //  String aFormatCode;         // bleibt leer, wenn dont-care
                //
                //  const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
                //  if ( rAttrSet.GetItemState( ATTR_VALUE_FORMAT ) != SFX_ITEM_DONTCARE )
                //  {
                //      sal_uLong nNumberFormat = ((const SfxUInt32Item&)rAttrSet.Get(
                //                                  ATTR_VALUE_FORMAT )).GetValue();
                //
                //      SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                //      const SvNumberformat* pFormatEntry = pFormatter->GetEntry( nNumberFormat );
                //      if ( pFormatEntry )
                //          aFormatCode = pFormatEntry->GetFormatstring();
                //  }
                //
                //  rSet.Put( SfxStringItem( nWhich, aFormatCode ) );
                //}

                // symphony version with format interpretation
                {
                    const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();

                    if(SFX_ITEM_DONTCARE != rAttrSet.GetItemState(ATTR_VALUE_FORMAT))
                    {
                        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                        sal_uInt32 nNumberFormat = ((const SfxUInt32Item&)rAttrSet.Get(ATTR_VALUE_FORMAT)).GetValue();
                        bool bThousand(false);
                        bool bNegRed(false);
                        sal_uInt16 nPrecision(0);
                        sal_uInt16 nLeadZeroes(0);

                        pFormatter->GetFormatSpecialInfo(nNumberFormat,bThousand, bNegRed, nPrecision, nLeadZeroes);
                        String aFormat;
                        static String sBreak = OUString(",");
                        const String sThousand = OUString::number(static_cast<sal_Int32>(bThousand));
                        const String sNegRed = OUString::number(static_cast<sal_Int32>(bNegRed));
                        const String sPrecision = OUString::number(nPrecision);
                        const String sLeadZeroes = OUString::number(nLeadZeroes);

                        aFormat += sThousand;
                        aFormat += sBreak;
                        aFormat += sNegRed;
                        aFormat += sBreak;
                        aFormat += sPrecision;
                        aFormat += sBreak;
                        aFormat += sLeadZeroes;
                        aFormat += sBreak;

                        rSet.Put(SfxStringItem(nWhich, aFormat));
                    }
                    else
                    {
                        rSet.InvalidateItem( nWhich );
                    }
                }
                break;

            case SID_NUMBER_TYPE_FORMAT:
                {
                    sal_Int16 aFormatCode = -1;
                    const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
                    if ( rAttrSet.GetItemState( ATTR_VALUE_FORMAT ) >= SFX_ITEM_AVAILABLE ) //Modify for more robust
                    {
                        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                        sal_uInt32 nNumberFormat = pTabViewShell->GetSelectionPattern()->GetNumberFormat( pFormatter );
                        const SvNumberformat* pFormatEntry = pFormatter->GetEntry( nNumberFormat );
                        bool bStandard = false;

                        if ( pFormatEntry )
                        {
                            aFormatCode = pFormatEntry->GetType();
                            bStandard = pFormatEntry->IsStandard();
                        }

                        switch(aFormatCode)
                        {
                        case NUMBERFORMAT_NUMBER:
                        case NUMBERFORMAT_NUMBER| NUMBERFORMAT_DEFINED:
                            //use format code and standard format code to judge whether it is General,
                            //if (nNumberFormat == nStandardNumberFormat)
                            if (bStandard)
                                aFormatCode = 0;
                            else
                                aFormatCode = 1;
                            break;
                        case NUMBERFORMAT_PERCENT:
                        case NUMBERFORMAT_PERCENT| NUMBERFORMAT_DEFINED:
                            aFormatCode = 2;
                            break;
                        case NUMBERFORMAT_CURRENCY:
                        case NUMBERFORMAT_CURRENCY| NUMBERFORMAT_DEFINED:
                            aFormatCode = 3;
                            break;
                        case NUMBERFORMAT_DATE:
                        case NUMBERFORMAT_DATE| NUMBERFORMAT_DEFINED:
                            //Add
                        case NUMBERFORMAT_DATETIME:
                        case NUMBERFORMAT_DATETIME | NUMBERFORMAT_DEFINED:
                            aFormatCode = 4;
                            break;
                        case NUMBERFORMAT_TIME:
                        case NUMBERFORMAT_TIME| NUMBERFORMAT_DEFINED:
                            aFormatCode = 5;
                            break;
                        case NUMBERFORMAT_SCIENTIFIC:
                        case NUMBERFORMAT_SCIENTIFIC| NUMBERFORMAT_DEFINED:
                            aFormatCode = 6;
                            break;
                        case NUMBERFORMAT_FRACTION:
                        case NUMBERFORMAT_FRACTION| NUMBERFORMAT_DEFINED:
                            aFormatCode = 7;
                            break;
                        case NUMBERFORMAT_LOGICAL:
                        case NUMBERFORMAT_LOGICAL| NUMBERFORMAT_DEFINED:
                            aFormatCode = 8;
                            break;
                        case NUMBERFORMAT_TEXT:
                        case NUMBERFORMAT_TEXT| NUMBERFORMAT_DEFINED:
                            aFormatCode = 9;
                            break;
                        default:
                            aFormatCode = -1;   //for more roburst
                        }
                        if( aFormatCode == -1 )
                            rSet.InvalidateItem( nWhich );
                        else
                            rSet.Put( SfxInt16Item( nWhich, aFormatCode ) );
                    }
                    else
                    {
                        rSet.InvalidateItem( nWhich );
                    }

                }
                break;
            case SID_NUMBER_SCIENTIFIC:
                rSet.Put( SfxBoolItem(nWhich, (nType & NUMBERFORMAT_SCIENTIFIC)) );
                break;
            case SID_NUMBER_DATE:
                rSet.Put( SfxBoolItem(nWhich, (nType & NUMBERFORMAT_DATE)) );
                break;
            case SID_NUMBER_CURRENCY:
                rSet.Put( SfxBoolItem(nWhich, (nType & NUMBERFORMAT_CURRENCY)) );
                break;
            case SID_NUMBER_PERCENT:
                rSet.Put( SfxBoolItem(nWhich, (nType & NUMBERFORMAT_PERCENT)) );
                break;
            case SID_NUMBER_TIME:
                rSet.Put( SfxBoolItem(nWhich, (nType & NUMBERFORMAT_TIME)) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}


void ScFormatShell::ExecuteTextDirection( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    pTabViewShell->HideListBox();               // Autofilter-DropDown-Listbox
    bool bEditMode = false;
    if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
    {
        bEditMode=true;
        SC_MOD()->InputEnterHandler();
        pTabViewShell->UpdateInputHandler();
    }
    sal_uInt16 nSlot = rReq.GetSlot();
    switch( nSlot )
    {
        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        {
            sal_Bool bVert = (nSlot == SID_TEXTDIRECTION_TOP_TO_BOTTOM);
            ScPatternAttr aAttr( GetViewData()->GetDocument()->GetPool() );
            SfxItemSet& rItemSet = aAttr.GetItemSet();
            rItemSet.Put( SfxBoolItem( ATTR_STACKED, bVert ) );
            rItemSet.Put( SfxBoolItem( ATTR_VERTICAL_ASIAN, bVert ) );
            pTabViewShell->ApplySelectionPattern( aAttr );
            pTabViewShell->AdjustBlockHeight();
        }
        break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            SvxFrameDirection eDirection = ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT ) ?
                                                FRMDIR_HORI_LEFT_TOP : FRMDIR_HORI_RIGHT_TOP;

            String aUndo = ScGlobal::GetRscString( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT ?
                                                    STR_UNDO_L2R : STR_UNDO_R2L );
            ScDocShell* pDocSh = GetViewData()->GetDocShell();
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

            pTabViewShell->ApplyAttr( SvxFrameDirectionItem( eDirection, ATTR_WRITINGDIR ) );

            const SfxPoolItem* pItem = NULL;
            const SvxHorJustifyItem* pHorJustify = NULL;
            SvxCellHorJustify eHorJustify = SVX_HOR_JUSTIFY_STANDARD;
            ScRange aRange;
            GetViewData()->GetMarkData().GetMarkArea(aRange);
            ScMarkData  aMark = GetViewData()->GetMarkData();
            ScDocument * pDoc = GetViewData()->GetDocument();
            if (aMark.IsMultiMarked())
            {
                SCCOL nCol = aRange.aStart.Col();
                SCROW nRow = aRange.aStart.Row();
                for ( ; nCol <= aRange.aEnd.Col(); nCol++ )
                {
                    for ( ; nRow <= aRange.aEnd.Row(); nRow++ )
                    {
                        pItem = pDoc->GetAttr( nCol, nRow, aMark.GetFirstSelected(), ATTR_HOR_JUSTIFY );
                        pHorJustify = (const SvxHorJustifyItem*)pItem;
                        eHorJustify = SvxCellHorJustify( pHorJustify->GetValue() );

                        if( eHorJustify != SVX_HOR_JUSTIFY_CENTER && eHorJustify != SVX_HOR_JUSTIFY_BLOCK )
                        {
                            if( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT )
                                pDoc->ApplyAttr( nCol, nRow, aMark.GetFirstSelected(), SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY ) );
                            else
                                pDoc->ApplyAttr( nCol, nRow, aMark.GetFirstSelected(), SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY ) );
                        }
                    }
                    nRow = 0;
                }
            }
            else
            {
                const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();
                if( rAttrSet.GetItemState(ATTR_HOR_JUSTIFY, sal_True, &pItem) == SFX_ITEM_SET )
                {
                    pHorJustify = (const SvxHorJustifyItem*)pItem;
                    eHorJustify = SvxCellHorJustify( pHorJustify->GetValue() );
                }

                if( eHorJustify != SVX_HOR_JUSTIFY_CENTER && eHorJustify != SVX_HOR_JUSTIFY_BLOCK )
                {
                    if( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT )
                         rReq.AppendItem( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, SID_H_ALIGNCELL ) );
                    else
                         rReq.AppendItem( SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT, SID_H_ALIGNCELL ) );

                    rReq.SetSlot( SID_H_ALIGNCELL );
                    ExecuteSlot( rReq, GetInterface() );
                }
            }

            pDocSh->GetUndoManager()->LeaveListAction();
        }
        break;
    }
    if (bEditMode)
        SC_MOD()->SetInputMode( SC_INPUT_TABLE );
}

void ScFormatShell::GetTextDirectionState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();

    sal_Bool bVertDontCare =
        (rAttrSet.GetItemState( ATTR_VERTICAL_ASIAN ) == SFX_ITEM_DONTCARE) ||
        (rAttrSet.GetItemState( ATTR_STACKED ) == SFX_ITEM_DONTCARE);
    sal_Bool bLeftRight = !bVertDontCare &&
        !((const SfxBoolItem&) rAttrSet.Get( ATTR_STACKED )).GetValue();
    sal_Bool bTopBottom = !bVertDontCare && !bLeftRight &&
        ((const SfxBoolItem&) rAttrSet.Get( ATTR_VERTICAL_ASIAN )).GetValue();

    sal_Bool bBidiDontCare = (rAttrSet.GetItemState( ATTR_WRITINGDIR ) == SFX_ITEM_DONTCARE);
    EEHorizontalTextDirection eBidiDir = EE_HTEXTDIR_DEFAULT;
    if ( !bBidiDontCare )
    {
        SvxFrameDirection eCellDir = (SvxFrameDirection)((const SvxFrameDirectionItem&)
                                        rAttrSet.Get( ATTR_WRITINGDIR )).GetValue();
        if ( eCellDir == FRMDIR_ENVIRONMENT )
            eBidiDir = (EEHorizontalTextDirection)GetViewData()->GetDocument()->
                                GetEditTextDirection( GetViewData()->GetTabNo() );
        else if ( eCellDir == FRMDIR_HORI_RIGHT_TOP )
            eBidiDir = EE_HTEXTDIR_R2L;
        else
            eBidiDir = EE_HTEXTDIR_L2R;
    }

    SvtLanguageOptions  aLangOpt;
    sal_Bool bDisableCTLFont = !aLangOpt.IsCTLFontEnabled();
    sal_Bool bDisableVerticalText = !aLangOpt.IsVerticalTextEnabled();

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
            case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
                if ( bDisableVerticalText )
                    rSet.DisableItem( nWhich );
                else
                {
                    if( bVertDontCare )
                        rSet.InvalidateItem( nWhich );
                    else if ( nWhich == SID_TEXTDIRECTION_LEFT_TO_RIGHT )
                        rSet.Put( SfxBoolItem( nWhich, bLeftRight ) );
                    else
                        rSet.Put( SfxBoolItem( nWhich, bTopBottom ) );
                }
            break;

            case SID_ATTR_PARA_LEFT_TO_RIGHT:
            case SID_ATTR_PARA_RIGHT_TO_LEFT:
                if ( bDisableCTLFont )
                    rSet.DisableItem( nWhich );
                else
                {
                    if ( bTopBottom )
                        rSet.DisableItem( nWhich );
                    else if ( bBidiDontCare )
                        rSet.InvalidateItem( nWhich );
                    else if ( nWhich == SID_ATTR_PARA_LEFT_TO_RIGHT )
                        rSet.Put( SfxBoolItem( nWhich, eBidiDir == EE_HTEXTDIR_L2R ) );
                    else
                        rSet.Put( SfxBoolItem( nWhich, eBidiDir == EE_HTEXTDIR_R2L ) );
                }
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::ExecFormatPaintbrush( SfxRequest& rReq )
{
    ScViewFunc* pView = pViewData->GetView();
    if ( pView->HasPaintBrush() )
    {
        // cancel paintbrush mode
        pView->ResetBrushDocument();
    }
    else
    {
        sal_Bool bLock = false;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
            bLock = static_cast<const SfxBoolItem&>(pArgs->Get(SID_FORMATPAINTBRUSH)).GetValue();

        // in case of multi selection, deselect all and use the cursor position
        ScRange aDummy;
        if ( pViewData->GetSimpleArea(aDummy) != SC_MARK_SIMPLE )
            pView->Unmark();

        ScDocument* pBrushDoc = new ScDocument( SCDOCMODE_CLIP );
        pView->CopyToClip( pBrushDoc, false, true );
        pView->SetBrushDocument( pBrushDoc, bLock );
    }
}

void ScFormatShell::StateFormatPaintbrush( SfxItemSet& rSet )
{
    if ( pViewData->HasEditView( pViewData->GetActivePart() ) )
        rSet.DisableItem( SID_FORMATPAINTBRUSH );
    else
        rSet.Put( SfxBoolItem( SID_FORMATPAINTBRUSH, pViewData->GetView()->HasPaintBrush() ) );
}

short ScFormatShell::GetCurrentNumberFormatType()
{
    short nType = NUMBERFORMAT_ALL;
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData aMark(GetViewData()->GetMarkData());
    const SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    if (!pFormatter)
        return nType;

    // TODO: Find out how to get a selected table range in case multiple tables
    // are selected.  Currently we only check for the current active table.

    if ( aMark.IsMarked() || aMark.IsMultiMarked() )
    {
        aMark.MarkToMulti();
        ScRange aRange;
        aMark.GetMultiMarkArea(aRange);

        const ScMarkArray* pArray = aMark.GetArray();
        if (!pArray)
            return nType;

        short nComboType = NUMBERFORMAT_ALL;
        bool bFirstItem = true;
        for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
        {
            const ScMarkArray& rColArray = pArray[nCol];
            if (!rColArray.HasMarks())
                continue;

            SCROW nRow1, nRow2;
            ScMarkArrayIter aMarkIter(&rColArray);
            while (aMarkIter.Next(nRow1, nRow2))
            {
                ScRange aColRange(nCol, nRow1, aRange.aStart.Tab());
                aColRange.aEnd.SetRow(nRow2);
                sal_uInt32 nNumFmt = pDoc->GetNumberFormat(aColRange);
                const SvNumberformat* pEntry = pFormatter->GetEntry(nNumFmt);
                if (!pEntry)
                    return 0;

                short nThisType = pEntry->GetType();
                if (bFirstItem)
                {
                    bFirstItem = false;
                    nComboType = nThisType;
                }
                else if (nComboType != nThisType)
                    // mixed number format type.
                    return NUMBERFORMAT_ALL;
            }
        }
        nType = nComboType;
    }
    else
    {
        sal_uInt32 nNumFmt;
        pDoc->GetNumberFormat( pViewData->GetCurX(), pViewData->GetCurY(),
                               pViewData->GetTabNo(), nNumFmt );
        const SvNumberformat* pEntry = pFormatter->GetEntry( nNumFmt );
        nType = pEntry ? pEntry->GetType() : 0;
    }
    return nType;
}

void  ScFormatShell::ExecViewOptions( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&        rBindings = pViewData->GetBindings();
    const SfxItemSet*   pNewAttrs = rReq.GetArgs();

    if ( pNewAttrs )
    {
        sal_uInt16 nSlot = rReq.GetSlot();

        if( nSlot  == SID_SCGRIDSHOW)
        {

            ScViewData*             pLclViewData = pTabViewShell->GetViewData();
            const ScViewOptions&    rOldOpt = pLclViewData->GetOptions();
            ScDocShell*             pDocSh  = PTR_CAST(ScDocShell, SfxObjectShell::Current());
            bool bState =   ((const SfxBoolItem &)pNewAttrs->Get( pNewAttrs->GetPool()->GetWhich( nSlot ) )).GetValue();

            if ( (bool)rOldOpt.GetOption( VOPT_GRID ) !=  bState)
            {
                ScViewOptions rNewOpt(rOldOpt);
                rNewOpt.SetOption( VOPT_GRID,  bState);
                pLclViewData->SetOptions( rNewOpt );
                pLclViewData->GetDocument()->SetViewOptions( rNewOpt );
                pDocSh->SetDocumentModified();
                //add , write the change to sc view config
                ScModule*           pScMod      = SC_MOD();
                pScMod->SetViewOptions( rNewOpt );
                //add end
                rBindings.Invalidate( nSlot );
            }
        }
    }

}

void  ScFormatShell::GetViewOptions( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    if( pTabViewShell )
    {
        ScViewOptions   aViewOpt = pTabViewShell->GetViewData()->GetOptions();
        rSet.ClearItem(SID_SCGRIDSHOW);
        SfxBoolItem aItem( SID_SCGRIDSHOW, aViewOpt.GetOption( VOPT_GRID ) );
        rSet.Put(aItem);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
