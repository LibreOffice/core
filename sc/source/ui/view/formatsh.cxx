/*************************************************************************
 *
 *  $RCSfile: formatsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

//------------------------------------------------------------------

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SI_NOSBXCONTROLS
#define _VCONT_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS

//------------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/request.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>

#define _ZFORLIST_DECLARE_TABLE
#include <svtools/stritem.hxx>
#include <svtools/zformat.hxx>
#include <svx/boxitem.hxx>
#include <svx/langitem.hxx>
#include <svx/numinf.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/tplpitem.hxx>
#include <svx/svxenum.hxx>
#include <svx/algitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/postitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/bolnitem.hxx>
#include <svx/colritem.hxx>
#include <svx/brshitem.hxx>

#include "formatsh.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
#include "styledlg.hxx"
#include "attrdlg.hrc"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "printfun.hxx"
#include "docpool.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"
#include "undostyl.hxx"


#define ScFormatShell
#define TableFont
#define FormatForSelection
#include "scslots.hxx"

#define Interior
#include <svx/svxslots.hxx>


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
    SetUndoManager( pViewData->GetSfxDocShell()->GetUndoManager() );
    SetHelpId(HID_SCSHELL_FORMATSH);
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Format")));
}

ScFormatShell::~ScFormatShell()
{
}

//------------------------------------------------------------------

void __EXPORT ScFormatShell::GetStyleState( SfxItemSet& rSet )
{
    ScDocument*             pDoc        = GetViewData()->GetDocument();
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxStyleSheetBasePool*  pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*      pStyleSheet = NULL;

    BOOL bProtected = FALSE;
    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT i=0; i<nTabCount; i++)
        if (pDoc->IsTabProtected(i))                // ueberhaupt eine Tabelle geschuetzt?
            bProtected = TRUE;

    SfxWhichIter    aIter(rSet);
    USHORT          nWhich = aIter.FirstWhich();
    USHORT          nSlotId = 0;

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

            case SID_STYLE_FAMILY2:     // Zellvorlagen
            {
                SfxStyleSheet* pStyleSheet = (SfxStyleSheet*)
                                             pTabViewShell->GetStyleSheetFromMarked();

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, pStyleSheet->GetName() ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, String() ) );
            }
            break;

            case SID_STYLE_FAMILY4:     // Seitenvorlagen
            {
                USHORT          nCurTab     = GetViewData()->GetTabNo();
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
                BOOL bPage = pDesigner && SFX_STYLE_FAMILY_PAGE == pDesigner->GetActualFamily();

                if ( bProtected || bPage )
                    rSet.DisableItem( nSlotId );
            }
            break;

            case SID_STYLE_EDIT:
            case SID_STYLE_DELETE:
            {
                ISfxTemplateCommon* pDesigner = SFX_APP()->
                        GetCurrentTemplateCommon(pTabViewShell->GetViewFrame()->GetBindings());
                BOOL bPage = pDesigner && SFX_STYLE_FAMILY_PAGE == pDesigner->GetActualFamily();

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

void __EXPORT ScFormatShell::ExecuteStyle( SfxRequest& rReq )
{
    // Wenn ToolBar vertikal :
    if ( !rReq.GetArgs() )
    {
        SFX_DISPATCHER().Execute( SID_STYLE_DESIGNER, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        return;
    }

    //--------------------------------------------------------------------
    SfxBindings&        rBindings   = SFX_BINDINGS();
    const SfxItemSet*   pArgs       = rReq.GetArgs();
    const USHORT        nSlotId     = rReq.GetSlot();
    const USHORT        nCurTab     = GetViewData()->GetTabNo();
    ScDocShell*         pDocSh      = GetViewData()->GetDocShell();
    ScTabViewShell*     pTabViewShell= GetViewData()->GetViewShell();
    ScDocument*         pDoc        = pDocSh->GetDocument();
    ScMarkData&         rMark       = GetViewData()->GetMarkData();
    ScModule*           pScMod      = SC_MOD();
    String              aRefName;

    if (   (nSlotId == SID_STYLE_NEW)
        || (nSlotId == SID_STYLE_EDIT)
        || (nSlotId == SID_STYLE_DELETE)
        || (nSlotId == SID_STYLE_APPLY)
        || (nSlotId == SID_STYLE_WATERCAN)
        || (nSlotId == SID_STYLE_FAMILY)
        || (nSlotId == SID_STYLE_NEW_BY_EXAMPLE)
        || (nSlotId == SID_STYLE_UPDATE_BY_EXAMPLE) )
    {
        SfxStyleSheetBasePool*  pStylePool  = pDoc->GetStyleSheetPool();
        SfxStyleSheetBase*      pStyleSheet = NULL;

        BOOL bAddUndo = FALSE;          // add ScUndoModifyStyle (style modified)
        ScStyleSaveData aOldData;       // for undo/redo
        ScStyleSaveData aNewData;

        SfxStyleFamily eFamily = SFX_STYLE_FAMILY_PARA;
        const SfxPoolItem* pFamItem;
        if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILY, TRUE, &pFamItem ) )
            eFamily = (SfxStyleFamily)((const SfxUInt16Item*)pFamItem)->GetValue();

        String                  aStyleName;
        USHORT                  nRetMask = 0xffff;
        BOOL                    bGrabFocus = ( SID_STYLE_APPLY == nSlotId );

        pStylePool->SetSearchMask( eFamily, SFXSTYLEBIT_ALL );

        switch ( nSlotId )
        {
            case SID_STYLE_NEW:
                {
                    const SfxPoolItem* pNameItem;
                    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( nSlotId, TRUE, &pNameItem ))
                        aStyleName  = ((const SfxStringItem*)pNameItem)->GetValue();

                    const SfxPoolItem* pRefItem=NULL;
                    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_REFERENCE, TRUE, &pRefItem ))
                    {
                        if(pRefItem!=NULL)
                            aRefName  = ((const SfxStringItem*)pRefItem)->GetValue();
                    }

                    pStyleSheet = &(pStylePool->Make( aStyleName, eFamily,
                                                      SFXSTYLEBIT_USERDEF ) );

                    if(pStyleSheet!=NULL) pStyleSheet->SetParent(aRefName);
                }
                break;

            case SID_STYLE_EDIT:
            case SID_STYLE_DELETE:
            case SID_STYLE_APPLY:
            case SID_STYLE_NEW_BY_EXAMPLE:
                {
                    const SfxPoolItem* pNameItem;
                    if (pArgs && SFX_ITEM_SET == pArgs->GetItemState( nSlotId, TRUE, &pNameItem ))
                        aStyleName  = ((const SfxStringItem*)pNameItem)->GetValue();
                    pStyleSheet = pStylePool->Find( aStyleName, eFamily );

                    aOldData.InitFromStyle( pStyleSheet );
                }
                break;

            case SID_STYLE_WATERCAN:
            {
                BOOL bWaterCan = pScMod->GetIsWaterCan();

                if( !bWaterCan )
                {
                    const SfxPoolItem* pItem;

                    if ( SFX_ITEM_SET ==
                         pArgs->GetItemState( nSlotId, TRUE, &pItem ) )
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
                    pScMod->SetWaterCan( TRUE );
                    pTabViewShell->SetActivePointer( Pointer(POINTER_FILL) );
                    rReq.Done();
                }
                else
                {
                    pScMod->SetWaterCan( FALSE );
                    pTabViewShell->SetActivePointer( Pointer(POINTER_ARROW) );
                    rReq.Done();
                }
            }
            break;

            default:
                break;
        }

        // Neuen Style fuer WaterCan-Mode setzen
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
                            pTabViewShell->RemoveStyleSheetInUse( (SfxStyleSheet*)pStyleSheet );
                            pStylePool->Erase( pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                            nRetMask = TRUE;
                            bAddUndo = TRUE;
                            rReq.Done();
                        }
                        else
                            nRetMask = FALSE;
                    }
                    break;

                    case SID_STYLE_APPLY:
                    {
                        if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                        {
                            // Anwenden der Vorlage auf das Dokument
                            pTabViewShell->SetStyleSheetToMarked( (SfxStyleSheet*)pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_NEW_BY_EXAMPLE:
                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                    {
                        // Vorlage erzeugen/ersetzen durch Attribute
                        // an der Cursor-Position:

                        const ScPatternAttr* pAttrItem = NULL;

                        // Die Abfrage, ob markiert ist, war hier immer falsch,
                        // darum jetzt gar nicht mehr, und einfach vom Cursor.
                        // Wenn Attribute aus der Selektion genommen werden sollen,
                        // muss noch darauf geachtet werden, Items aus Vorlagen nicht
                        // zu uebernehmen (GetSelectionPattern sammelt auch Items aus
                        // Vorlagen zusammen) (#44748#)
                        //      pAttrItem = GetSelectionPattern();

                        ScViewData* pViewData = GetViewData();
                        USHORT      nCol = pViewData->GetCurX();
                        USHORT      nRow = pViewData->GetCurY();
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
                            BOOL            bConvertBack = FALSE;
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
                                bConvertBack = TRUE;
                                pStylePool->Erase(pStyleSheet);
                            }

                            // ...und neu anlegen
                            pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                             SFXSTYLEBIT_USERDEF );

                            // wenn ein Style vorhanden ist, so wird dieser
                            // Parent der neuen Vorlage:
                            if ( pSheetInUse )
                                pStyleSheet->SetParent( pSheetInUse->GetName() );

                            if ( bConvertBack )
                                // Namen zu Style-Pointer
                                pDoc->UpdStlShtPtrsFrmNms();

                            // Attribute uebernehmen und Style anwenden
                            pStyleSheet->GetItemSet().Put( aAttrSet );
                            pTabViewShell->UpdateStyleSheetInUse( (SfxStyleSheet*)pStyleSheet );
                            pTabViewShell->SetStyleSheetToMarked( (SfxStyleSheet*)pStyleSheet );

                            pTabViewShell->InvalidateAttribs();
                        }
                        else // ( nSlotId == SID_STYLE_UPDATE_BY_EXAMPLE )
                        {
                            pStyleSheet = (SfxStyleSheet*)pTabViewShell->GetStyleSheetFromMarked();

                            if ( pStyleSheet )
                            {
                                pStyleSheet->GetItemSet().Put( aAttrSet );
                                pTabViewShell->UpdateStyleSheetInUse( (SfxStyleSheet*)pStyleSheet );
                                pTabViewShell->SetStyleSheetToMarked( (SfxStyleSheet*)pStyleSheet );
                                pTabViewShell->InvalidateAttribs();
                            }
                        }

                        aNewData.InitFromStyle( pStyleSheet );
                        bAddUndo = TRUE;
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
                                ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(), nCurTab ).UpdatePages();
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );
                            }
                            pStylePool->Erase( pStyleSheet );
                            rBindings.Invalidate( SID_STYLE_FAMILY4 );
                            pDocSh->SetDocumentModified();
                            bAddUndo = TRUE;
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_APPLY:
                    {
                        nRetMask = ( NULL != pStyleSheet );
                        if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                        {
                            String aOldName = pDoc->GetPageStyle( nCurTab );
                            if ( aOldName != aStyleName )
                            {
                                pDoc->SetPageStyle( nCurTab, aStyleName );
                                ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(), nCurTab ).UpdatePages();
                                pDocSh->SetDocumentModified();
                                rBindings.Invalidate( SID_STYLE_FAMILY4 );
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );

                                pDocSh->GetUndoManager()->AddUndoAction(
                                        new ScUndoApplyPageStyle( pDocSh,
                                            nCurTab, aOldName, aStyleName ) );
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
                            USHORT              nInTab;
                            BOOL                bUsed = pDoc->IsPageStyleInUse( aStyleName, &nInTab );

                            // wenn bereits vorhanden, erstmal entfernen...
                            if ( pStyleSheet )
                                pStylePool->Erase( pStyleSheet );

                            // ...und neu anlegen
                            pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                             SFXSTYLEBIT_USERDEF );

                            // Attribute uebernehmen
                            pStyleSheet->GetItemSet().Put( aAttrSet );
                            pDocSh->SetDocumentModified();

                            // wenn in Verwendung -> Update
                            if ( bUsed )
                                ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(), nInTab ).UpdatePages();

                            aNewData.InitFromStyle( pStyleSheet );
                            bAddUndo = TRUE;
                            rReq.Done();
                            nRetMask = TRUE;
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
                ScDocument*     pDoc    = GetViewData()->GetDocument();
                ScDocShell*     pDocSh  = GetViewData()->GetDocShell();
                ScStyleDlg*     pDlg    = NULL;
                USHORT          nRsc    = 0;

                //  #37034#/#37245# alte Items aus der Vorlage merken
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
                                    FALSE, &pItem ) == SFX_ITEM_SET )
                            {
                                // NumberFormat Value aus Value und Language
                                // erzeugen und eintueten
                                ULONG nFormat =
                                    ((SfxUInt32Item*)pItem)->GetValue();
                                LanguageType eLang =
                                    ((SvxLanguageItem*)&rSet.Get(
                                    ATTR_LANGUAGE_FORMAT ))->GetLanguage();
                                ULONG nLangFormat = pDoc->GetFormatTable()->
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

                            //  auf jeden Fall ein SvxBoxInfoItem mit Table = FALSE im Set:
                            //  (wenn gar kein Item da ist, loescht der Dialog auch das
                            //   BORDER_OUTER SvxBoxItem aus dem Vorlagen-Set)

                            if ( rSet.GetItemState( ATTR_BORDER_INNER, FALSE ) != SFX_ITEM_SET )
                            {
                                SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
                                aBoxInfoItem.SetTable(FALSE);       // keine inneren Linien
                                aBoxInfoItem.SetDist(FALSE);
                                aBoxInfoItem.SetMinDist(FALSE);
                                rSet.Put( aBoxInfoItem );
                            }
                        }
                        break;
                }

                //  If GetDefDialogParent is a dialog, it must be used
                //  (style catalog)

                Window* pParent = Application::GetDefDialogParent();
                if ( !pParent || !pParent->IsDialog() )
                    pParent = pTabViewShell->GetDialogParent();

                pTabViewShell->SetInFormatDialog(TRUE);

                pDlg = new ScStyleDlg( pParent, *pStyleSheet, nRsc );
                short nResult = pDlg->Execute();
                pTabViewShell->SetInFormatDialog(FALSE);

                if ( nResult == RET_OK )
                {
                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                    if ( pOutSet )
                    {
                        nRetMask = pStyleSheet->GetMask();

                        //  #37034#/#37245# Attribut-Vergleiche (frueher in ModifyStyleSheet)
                        //  jetzt hier mit den alten Werten (Style ist schon veraendert)

                        if ( SFX_STYLE_FAMILY_PARA == eFam )
                        {
//                          pDoc->CellStyleChanged( *pStyleSheet, aOldSet );

                            SfxItemSet& rNewSet = pStyleSheet->GetItemSet();
                            BOOL bNumFormatChanged;
                            if ( ScGlobal::CheckWidthInvalidate(
                                                bNumFormatChanged, aOldSet, rNewSet ) )
                                pDoc->InvalidateTextWidth( NULL, NULL, bNumFormatChanged );

                            ULONG nOldFormat = ((const SfxUInt32Item&)aOldSet.
                                                    Get( ATTR_VALUE_FORMAT )).GetValue();
                            ULONG nNewFormat = ((const SfxUInt32Item&)rNewSet.
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
                            pTabViewShell->UpdateNumberFormatter( pDoc,
                                (const SvxNumberInfoItem&)
                                    *(pDocSh->GetItem(SID_ATTR_NUMBERFORMAT_INFO)) );

                            pTabViewShell->UpdateStyleSheetInUse( (SfxStyleSheet*)pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                        }

                        aNewData.InitFromStyle( pStyleSheet );
                        bAddUndo = TRUE;
                    }
                }
                else
                {
                    if ( nSlotId == SID_STYLE_NEW )
                        pStylePool->Erase( pStyleSheet );
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

//      if ( nRetMask != 0xffff )// Irgendein Wert MUSS geliefert werden JN
            rReq.SetReturnValue( SfxUInt16Item( nSlotId, nRetMask ) );

        if ( bGrabFocus )
            pTabViewShell->GetActiveWin()->GrabFocus();

        if ( bAddUndo )
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoModifyStyle( pDocSh, eFamily, aOldData, aNewData ) );
    }
    else
    {
        DBG_ERROR( "Unknown slot (ScViewShell::ExecuteStyle)" );
    }
}

void ScFormatShell::ExecuteNumFormat( SfxRequest& rReq )
{
    ScModule*           pScMod      = SC_MOD();
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    SfxBindings&        rBindings   = pTabViewShell->GetViewFrame()->GetBindings();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();

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
            case FID_USE_NAME:
            case FID_INSERT_NAME:
            case SID_SPELLING:

            pScMod->InputEnterHandler();
            pTabViewShell->UpdateInputHandler();
            break;

            default:
            break;
        }
    }

    switch ( nSlot )
    {
        case SID_NUMBER_TWODEC:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER, 4 );       // Standard+4 = #.##0,00
            rReq.Done();
            break;
        case SID_NUMBER_SCIENTIFIC:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_SCIENTIFIC );
            rReq.Done();
            break;
        case SID_NUMBER_DATE:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_DATE );
            rReq.Done();
            break;
        case SID_NUMBER_TIME:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_TIME );
            rReq.Done();
            break;
        case SID_NUMBER_CURRENCY:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_CURRENCY );
            rReq.Done();
            break;
        case SID_NUMBER_PERCENT:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_PERCENT );
            rReq.Done();
            break;
        case SID_NUMBER_STANDARD:
            pTabViewShell->SetNumberFormat( NUMBERFORMAT_NUMBER );
            rReq.Done();
            break;
        case SID_NUMBER_INCDEC:
            pTabViewShell->ChangeNumFmtDecimals( TRUE );
            rReq.Done();
            break;
        case SID_NUMBER_DECDEC:
            pTabViewShell->ChangeNumFmtDecimals( FALSE );
            rReq.Done();
            break;

        case SID_NUMBER_FORMAT:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if(pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET)
                {
                    String aCode = ((const SfxStringItem*)pItem)->GetValue();
                    pTabViewShell->SetNumFmtByStr( aCode );
                }
            }
            break;

        default:
            DBG_ERROR("falscher Slot bei ExecuteEdit");
            break;
    }
}


//------------------------------------------------------------------

#define APPLY_HOR_JUSTIFY(j) \
    {                                                                       \
        if ( !pHorJustify || (eHorJustify != (j) ) )                        \
            pTabViewShell->ApplyAttr( SvxHorJustifyItem( (j) ) );                          \
        else                                                                \
            pTabViewShell->ApplyAttr( SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD ) );     \
    }

#define APPLY_VER_JUSTIFY(j) \
    {                                                                       \
        if ( !pVerJustify || (eVerJustify != (j) ) )                        \
            pTabViewShell->ApplyAttr( SvxVerJustifyItem( (j) ) );                          \
        else                                                                \
            pTabViewShell->ApplyAttr( SvxVerJustifyItem( SVX_VER_JUSTIFY_STANDARD ) );     \
    }

void ScFormatShell::ExecuteAlignment( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&            rBindings   = SFX_BINDINGS();
    const SfxItemSet*       pSet        = rReq.GetArgs();
    USHORT                  nSlot       = rReq.GetSlot();

    pTabViewShell->HideListBox();   // Autofilter-DropDown-Listbox

    if( pSet )
    {
        const SfxPoolItem* pItem = NULL;
        if( pSet->GetItemState((nSlot), TRUE, &pItem  ) == SFX_ITEM_SET )
        {

            switch ( nSlot )
            {
                case SID_H_ALIGNCELL:
                {
                    pTabViewShell->ApplyAttr( SvxHorJustifyItem( (SvxCellHorJustify)((const SvxHorJustifyItem*)pItem)->GetValue() ) );
                }
                break;
                case SID_V_ALIGNCELL:
                {
                    pTabViewShell->ApplyAttr( SvxVerJustifyItem( (SvxCellVerJustify)((const SvxVerJustifyItem*)pItem)->GetValue() ) );
                }
                break;
                default:
                DBG_ERROR( "ExecuteAlignment: invalid slot" );
                return;
            }

            rBindings.Invalidate( SID_ALIGNLEFT );
            rBindings.Invalidate( SID_ALIGNRIGHT );
            rBindings.Invalidate( SID_ALIGNCENTERHOR );
            rBindings.Invalidate( SID_ALIGNBLOCK );
            rBindings.Invalidate( SID_ALIGNTOP );
            rBindings.Invalidate( SID_ALIGNBOTTOM );
            rBindings.Invalidate( SID_ALIGNCENTERVER );
            rBindings.Invalidate( SID_V_ALIGNCELL );
            rBindings.Invalidate( SID_H_ALIGNCELL );
            rBindings.Update();

            if( ! rReq.IsAPI() )
                rReq.Done();
        }
    }
}

void ScFormatShell::ExecuteTextAttr( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&            rBindings   = SFX_BINDINGS();
    const ScPatternAttr*    pAttrs      = pTabViewShell->GetSelectionPattern();
    const SfxItemSet*       pSet        = rReq.GetArgs();
    const SfxItemSet*       pReqArgs    = rReq.GetArgs();
    USHORT                  nSlot       = rReq.GetSlot();
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
                FontWeight      eWeight;

                if( pSet )
                {
                    const SvxWeightItem& rWeight = (const SvxWeightItem&)pSet->Get( ATTR_FONT_WEIGHT );

                    if( rWeight.ISA(SvxWeightItem) )
                    {
                        pTabViewShell->ApplyAttr( rWeight );
                        pNewSet->Put( rWeight,rWeight.Which() );
                    }
                }
                else
                {
                    SvxWeightItem aWeightItem( (const SvxWeightItem&)
                                               pAttrs->GetItem( ATTR_FONT_WEIGHT ) );

                    eWeight = (WEIGHT_BOLD == aWeightItem.GetWeight())
                                ? WEIGHT_NORMAL
                                : WEIGHT_BOLD;

                    aWeightItem.SetWeight( eWeight );
                    pTabViewShell->ApplyAttr( aWeightItem );
                    pNewSet->Put( aWeightItem, aWeightItem.Which() );
                }
            }
            break;

            case SID_ATTR_CHAR_POSTURE:
            {
                FontItalic      eItalic;

                if( pSet )
                {
                    const SvxPostureItem& rPosture = (const SvxPostureItem&)pSet->Get( ATTR_FONT_POSTURE );

                    if( rPosture.ISA(SvxPostureItem) )
                    {
                        pTabViewShell->ApplyAttr( rPosture );
                        pNewSet->Put( rPosture,rPosture.Which() );
                    }
                }
                else
                {
                    SvxPostureItem  aPosture( (const SvxPostureItem&)
                                              pAttrs->GetItem( ATTR_FONT_POSTURE ) );

                    eItalic = (ITALIC_NORMAL == aPosture.GetPosture())
                                ? ITALIC_NONE
                                : ITALIC_NORMAL;

                    aPosture.SetPosture( eItalic );
                    pTabViewShell->ApplyAttr( aPosture );
                    pNewSet->Put( aPosture,aPosture.Which() );
                }
            }
            break;

            case SID_ATTR_CHAR_UNDERLINE:
                {
                    FontUnderline       eUnderline;

                    if( pSet )
                    {
                        const SvxUnderlineItem& rUnderline = (const SvxUnderlineItem&)pSet->Get( ATTR_FONT_UNDERLINE );

                        if( rUnderline.ISA(SvxUnderlineItem) )
                        {
                            pTabViewShell->ApplyAttr( rUnderline );
                            pNewSet->Put( rUnderline,rUnderline.Which() );
                        }
                    }
                    else
                    {
                        SvxUnderlineItem    aUnderline( (const SvxUnderlineItem&)
                                                        pAttrs->GetItem(
                                                            ATTR_FONT_UNDERLINE ) );
                        eUnderline = (UNDERLINE_NONE != aUnderline.GetUnderline())
                                    ? UNDERLINE_NONE
                                    : UNDERLINE_SINGLE;
                        aUnderline.SetUnderline( eUnderline );
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
                                            pAttrs->GetItem(ATTR_FONT_UNDERLINE)).GetUnderline();
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

        if (rAttrSet.GetItemState(ATTR_HOR_JUSTIFY, TRUE,&pItem ) == SFX_ITEM_SET)
        {
            pHorJustify = (const SvxHorJustifyItem*)pItem;
            eHorJustify = SvxCellHorJustify( pHorJustify->GetValue() );
        }
        if (rAttrSet.GetItemState(ATTR_VER_JUSTIFY, TRUE,&pItem ) == SFX_ITEM_SET)
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
//              APPLY_HOR_JUSTIFY( SVX_HOR_JUSTIFY_LEFT );
                break;

            case SID_ALIGNRIGHT:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_RIGHT) ?
                    SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
//              APPLY_HOR_JUSTIFY( SVX_HOR_JUSTIFY_RIGHT );
                break;

            case SID_ALIGNCENTERHOR:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_CENTER) ?
                    SVX_HOR_JUSTIFY_CENTER : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
//              APPLY_HOR_JUSTIFY( SVX_HOR_JUSTIFY_CENTER );
                break;

            case SID_ALIGNBLOCK:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_BLOCK) ?
                    SVX_HOR_JUSTIFY_BLOCK : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
//              APPLY_HOR_JUSTIFY( SVX_HOR_JUSTIFY_BLOCK );
                break;

            case SID_ALIGNTOP:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_TOP) ?
                    SVX_VER_JUSTIFY_TOP : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
//              APPLY_VER_JUSTIFY( SVX_VER_JUSTIFY_TOP );
                break;

            case SID_ALIGNBOTTOM:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_BOTTOM) ?
                    SVX_VER_JUSTIFY_BOTTOM : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
//              APPLY_VER_JUSTIFY( SVX_VER_JUSTIFY_BOTTOM );
                break;

            case SID_ALIGNCENTERVER:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_CENTER) ?
                    SVX_VER_JUSTIFY_CENTER : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;
//              APPLY_VER_JUSTIFY( SVX_VER_JUSTIFY_CENTER );
                break;

            default:
            break;
        }

    }

    rBindings.Update();
//  rReq.Done();

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

#undef APPLY_HOR_JUSTIFY
#undef APPLY_VER_JUSTIFY

//------------------------------------------------------------------

void ScFormatShell::ExecuteAttr( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&        rBindings = SFX_BINDINGS();
    const SfxItemSet*   pNewAttrs = rReq.GetArgs();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    if ( !pNewAttrs )
    {
        USHORT nSlot = rReq.GetSlot();

        switch ( nSlot )
        {
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
                pTabViewShell->ExecuteCellFormatDlg( rReq, TP_FONT );       // wenn ToolBar vertikal
                break;

            case SID_ATTR_ALIGN_LINEBREAK:                  // ohne Parameter als Toggle
                {
                    const ScPatternAttr* pAttrs = pTabViewShell->GetSelectionPattern();
                    BOOL bOld = ((const SfxBoolItem&)pAttrs->GetItem(ATTR_LINEBREAK)).GetValue();
                    SfxBoolItem aBreakItem( ATTR_LINEBREAK, !bOld );
                    pTabViewShell->ApplyAttr( aBreakItem );

                    SfxAllItemSet aNewSet( GetPool() );
                    aNewSet.Put( aBreakItem,aBreakItem.Which() );
                    rReq.Done( aNewSet );

                    rBindings.Invalidate( nSlot );
                }
                break;

            case SID_BACKGROUND_COLOR:
                {
                    //  SID_BACKGROUND_COLOR without arguments -> set transparent background

                    SvxBrushItem        aBrushItem( (const SvxBrushItem&)
                                            pTabViewShell->GetSelectionPattern()->
                                                GetItem( ATTR_BACKGROUND ) );

                    aBrushItem.SetColor( COL_TRANSPARENT );

                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;
        }
    }
    else
    {
        USHORT nSlot = rReq.GetSlot();

        switch ( nSlot )
        {
            case SID_ATTR_ALIGN_LINEBREAK:
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_COLOR:
                pTabViewShell->ApplyAttr( pNewAttrs->Get( pNewAttrs->GetPool()->GetWhich( nSlot ) ) );
                rBindings.Invalidate( nSlot );
                rBindings.Update( nSlot );
                break;

            case SID_FRAME_LINESTYLE:
                {
                    // Default-Linie aktualisieren
                    const SvxBorderLine* pLine =
                            ((const SvxLineItem&)
                                pNewAttrs->Get( SID_FRAME_LINESTYLE )).
                                GetLine();

                    if ( pLine )
                    {
                        SvxBorderLine* pDefLine = pTabViewShell->GetDefaultFrameLine();

                        if ( pDefLine )
                        {
                            pDefLine->SetOutWidth( pLine->GetOutWidth() );
                            pDefLine->SetInWidth ( pLine->GetInWidth() );
                            pDefLine->SetDistance( pLine->GetDistance() );
                            pTabViewShell->SetSelectionFrameLines( pDefLine, FALSE );
                        }
                        else
                        {
                            pTabViewShell->SetDefaultFrameLine( pLine );
                            pTabViewShell->GetDefaultFrameLine()->SetColor( COL_BLACK );
                            pTabViewShell->SetSelectionFrameLines( pLine, FALSE );
                        }
                    }
                    else
                    {
                        Color           aColorBlack( COL_BLACK );
                        SvxBorderLine   aDefLine( &aColorBlack, 20, 0, 0 );
                        pTabViewShell->SetDefaultFrameLine( &aDefLine );
                        pTabViewShell->SetSelectionFrameLines( NULL, FALSE );
                    }
                }
                break;

            case SID_FRAME_LINECOLOR:
                {
                    SvxBorderLine*  pDefLine = pTabViewShell->GetDefaultFrameLine();
                    const Color&    rColor = ((const SvxColorItem&)
                                        pNewAttrs->Get( SID_FRAME_LINECOLOR )).
                                            GetValue();

                    // Default-Linie aktualisieren
                    if ( pDefLine )
                    {
                        pDefLine->SetColor( rColor );
                        pTabViewShell->SetSelectionFrameLines( pDefLine, TRUE );
                    }
                    else
                    {
                        SvxBorderLine   aDefLine( &rColor, 20, 0, 0 );
                        pTabViewShell->SetDefaultFrameLine( &aDefLine );
                        pTabViewShell->SetSelectionFrameLines( &aDefLine, FALSE );
                    }
                }
                break;

            case SID_ATTR_BORDER_OUTER:
            case SID_ATTR_BORDER:
                {
                    SvxBorderLine*          pDefLine = pTabViewShell->GetDefaultFrameLine();
                    const ScPatternAttr*    pOldAttrs = pTabViewShell->GetSelectionPattern();
                    ScDocument*             pDoc = GetViewData()->GetDocument();
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

                    if ( pNewAttrs->GetItemState( ATTR_BORDER, TRUE, &pItem )
                         == SFX_ITEM_SET )
                    {
                        SvxBoxItem aBoxItem ( *(const SvxBoxItem*)pItem );
                        if ( aBoxItem.GetTop() )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_TOP );
                        if ( aBoxItem.GetBottom() )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_BOTTOM );
                        if ( aBoxItem.GetLeft() )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_LEFT );
                        if ( aBoxItem.GetRight() )
                            aBoxItem.SetLine( pDefLine, BOX_LINE_RIGHT );
                        pNewSet->Put( aBoxItem );
                    }

                    if ( pNewAttrs->GetItemState( ATTR_BORDER_INNER, TRUE, &pItem )
                         == SFX_ITEM_SET )
                    {
                        SvxBoxInfoItem aBoxInfoItem( *(const SvxBoxInfoItem*)pItem );
                        if ( aBoxInfoItem.GetHori() )
                            aBoxInfoItem.SetLine( pDefLine, BOXINFO_LINE_HORI );
                        if ( aBoxInfoItem.GetVert() )
                            aBoxInfoItem.SetLine( pDefLine, BOXINFO_LINE_VERT );
                        pNewSet->Put( aBoxInfoItem );
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
    const SfxItemSet*    pParentSet = rAttrSet.GetParent();
    const SvxBorderLine* pLine      = pTabViewShell->GetDefaultFrameLine();
    const SvxBrushItem&  rBrushItem = (const SvxBrushItem&)rAttrSet.Get( ATTR_BACKGROUND );
    UINT8                nTrans     = rBrushItem.GetColor().GetTransparency();
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();

    rSet.Put( rAttrSet, FALSE );

    while ( nWhich )
    {
        switch(nWhich)
        {
            case SID_BACKGROUND_COLOR:
            {
                Color aColor;
                if ( nTrans == 255 )
                    aColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
                else
                    aColor = rBrushItem.GetColor();
                rSet.Put( SvxColorItem( aColor, SID_BACKGROUND_COLOR ) );
            }
            break;
            case SID_FRAME_LINECOLOR:
            {
                rSet.Put( SvxColorItem( pLine ? pLine->GetColor() : Color(),
                                        SID_FRAME_LINECOLOR ) );
            }
            break;
            case SID_ATTR_BRUSH:
            {
                rSet.Put( rBrushItem, GetPool().GetWhich(nWhich) );
            }
            break;
/*          case SID_ATTR_ALIGN_LINEBREAK:
            {
                const SfxBoolItem& rBreakItem = (const SfxBoolItem&)rAttrSet.Get( ATTR_LINEBREAK );
                rSet.Put( rBreakItem, GetPool().GetWhich(nWhich) );
            }
            break;
*/
        }
        nWhich = aIter.NextWhich();
    }
}

//------------------------------------------------------------------

void ScFormatShell::GetTextAttrState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
    rSet.Put( rAttrSet, FALSE ); // ItemStates mitkopieren

    SfxItemState eState;
//  const SfxPoolItem* pItem;

    //--------------------------------------------------------------------
    // eigene Kontrolle ueber RadioButton-Funktionalitaet:
    //--------------------------------------------------------------------
    // Unterstreichung
    //------------------------

    eState = rAttrSet.GetItemState( ATTR_FONT_UNDERLINE, TRUE );
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
                    rAttrSet.Get(ATTR_FONT_UNDERLINE)).GetUnderline();
        USHORT nId = SID_ULINE_VAL_NONE;
        switch (eUnderline)
        {
            case UNDERLINE_SINGLE:  nId = SID_ULINE_VAL_SINGLE; break;
            case UNDERLINE_DOUBLE:  nId = SID_ULINE_VAL_DOUBLE; break;
            case UNDERLINE_DOTTED:  nId = SID_ULINE_VAL_DOTTED; break;
            default:
                break;
        }
        rSet.Put( SfxBoolItem( nId, TRUE ) );
    }

    //------------------------
    // horizontale Ausrichtung
    //------------------------

    const SvxHorJustifyItem* pHorJustify = NULL;
    const SvxVerJustifyItem* pVerJustify = NULL;
    SvxCellHorJustify        eHorJustify = SVX_HOR_JUSTIFY_STANDARD;
    SvxCellVerJustify        eVerJustify = SVX_VER_JUSTIFY_STANDARD;
    USHORT                   nWhich      = 0;
    BOOL                     bJustifyStd = FALSE;
    SfxBoolItem              aBoolItem   ( 0, TRUE );

    eState   = rAttrSet.GetItemState( ATTR_HOR_JUSTIFY, TRUE,
                                        (const SfxPoolItem**)&pHorJustify );
    switch ( eState )
    {
        case SFX_ITEM_SET:
            {
                eHorJustify = SvxCellHorJustify( pHorJustify->GetValue() );

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
                        bJustifyStd = TRUE;
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
            bJustifyStd = TRUE;
            break;
    }

    if ( nWhich )
    {
        aBoolItem.SetWhich( nWhich );
        rSet.Put( aBoolItem );
    }
    else if ( bJustifyStd )
    {
        aBoolItem.SetValue( FALSE );
        aBoolItem.SetWhich( SID_ALIGNLEFT );      rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNRIGHT );     rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNCENTERHOR ); rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNBLOCK );     rSet.Put( aBoolItem );
        bJustifyStd = FALSE;
    }

    //------------------------
    // vertikale Ausrichtung
    //------------------------

    nWhich = 0;
    aBoolItem.SetValue( TRUE );

    eState = rAttrSet.GetItemState( ATTR_VER_JUSTIFY, TRUE,
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
                        bJustifyStd = TRUE;
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
            bJustifyStd = TRUE;
            break;
    }

    if ( nWhich )
    {
        aBoolItem.SetWhich( nWhich );
        rSet.Put( aBoolItem );
    }
    else if ( bJustifyStd )
    {
        aBoolItem.SetValue( FALSE );
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
    USHORT          nWhich = aIter.FirstWhich();
    SvxCellHorJustify        eHorJustify = SVX_HOR_JUSTIFY_STANDARD;
    SvxCellVerJustify        eVerJustify = SVX_VER_JUSTIFY_STANDARD;


    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_H_ALIGNCELL:
            {
                if ( rAttrSet.GetItemState( ATTR_HOR_JUSTIFY ) != SFX_ITEM_DONTCARE )
                {
                    SvxCellHorJustify eHorJustify = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                        rAttrSet.Get( ATTR_HOR_JUSTIFY )).GetValue();
                    rSet.Put( SvxHorJustifyItem( eHorJustify, nWhich ));
                }
            }
            break;
            case SID_V_ALIGNCELL:
            {
                if ( rAttrSet.GetItemState( ATTR_VER_JUSTIFY ) != SFX_ITEM_DONTCARE )
                {
                    SvxCellVerJustify eVerJustify = (SvxCellVerJustify)((const SvxVerJustifyItem&)
                                        rAttrSet.Get( ATTR_VER_JUSTIFY )).GetValue();
                    rSet.Put( SvxVerJustifyItem( eVerJustify, nWhich ));
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::ExecBckCol( SfxRequest& rReq )
{
    ExecuteAttr( rReq );
}

void ScFormatShell::GetBckColState( SfxItemSet& rSet )
{
    GetAttrState( rSet );
}
void ScFormatShell::GetNumFormatState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    BOOL bOle = pTabViewShell->GetViewFrame()->ISA(SfxInPlaceFrame);
    BOOL bTabProt = GetViewData()->GetDocument()->IsTabProtected(GetViewData()->GetTabNo());
    SfxApplication* pSfxApp = SFX_APP();

    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScMarkData& rMark       = pViewData->GetMarkData();
    USHORT      nPosX       = pViewData->GetCurX();
    USHORT      nPosY       = pViewData->GetCurY();
    USHORT      nTab        = pViewData->GetTabNo();
    USHORT      nMyId       = 0;

    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nTabSelCount = rMark.GetSelectCount();



    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_NUMBER_FORMAT:
                {
                    String aFormatCode;         // bleibt leer, wenn dont-care

                    const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
                    if ( rAttrSet.GetItemState( ATTR_VALUE_FORMAT ) != SFX_ITEM_DONTCARE )
                    {
                        ULONG nNumberFormat = ((const SfxUInt32Item&)rAttrSet.Get(
                                                    ATTR_VALUE_FORMAT )).GetValue();

                        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                        const SvNumberformat* pFormatEntry = pFormatter->GetEntry( nNumberFormat );
                        if ( pFormatEntry )
                            aFormatCode = pFormatEntry->GetFormatstring();
                    }

                    rSet.Put( SfxStringItem( nWhich, aFormatCode ) );
                }
                break;

        }
        nWhich = aIter.NextWhich();
    }
}



