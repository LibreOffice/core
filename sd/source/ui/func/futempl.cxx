/*************************************************************************
 *
 *  $RCSfile: futempl.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 10:57:53 $
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

#pragma hdrstop

#include <svx/editdata.hxx>
#include <svx/bulitem.hxx>
#include <svx/svxids.hrc>   // fuer SID_OBJECT_SELECT
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#define ITEMID_LRSPACE          EE_PARA_LRSPACE
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVDOPAGE_HXX
#include <svx/svdopage.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif


#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX
#include <svx/xlnstit.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX
#include <svx/xlnedit.hxx>
#endif

#include "stlsheet.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "app.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "drviewsh.hxx"
#include "tabtempl.hxx"
#include "viewshel.hxx"
#include "futempl.hxx"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "prlayout.hxx"         // enum PresentationObjects
#include "prltempl.hrc"         // TAB_PRES_LAYOUT_TEMPLATE_x
#include "prltempl.hxx"
#include "sdresid.hxx"
#include "outlview.hxx"         // class SdOutlineView
#include "strings.hrc"
#include "helpids.h"


TYPEINIT1( FuTemplate, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuTemplate::FuTemplate( SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                        SdDrawDocument* pDoc, SfxRequest& rReq )
       : FuPoor( pViewSh, pWin, pView, pDoc, rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    USHORT nSlotId = rReq.GetSlot();

    // StyleSheet-Parameter holen
    SfxStyleSheetBasePool* pSSPool = pDoc->GetDocSh()->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = NULL;

    USHORT nFamily = ( (const SfxUInt16Item &) pArgs->Get( SID_STYLE_FAMILY ) ).GetValue();
    String aStyleName;
    USHORT nRetMask = 0xffff;

    switch( nSlotId )
    {
        case SID_STYLE_APPLY:
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_FAMILY:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if (pArgs->GetItemState(nSlotId) == SFX_ITEM_SET)
                aStyleName = ( ( (const SfxStringItem &) pArgs->Get( nSlotId ) ).GetValue() );
        }
    }

    switch( nSlotId )
    {
        case SID_STYLE_NEW:
        {
            SfxStyleSheetBase *p = pSSPool->Find(aStyleName, (SfxStyleFamily) nFamily,
                                                    SFXSTYLEBIT_ALL );
            if(p)
            {
                pSSPool->Erase(p);
                p = 0;
            }
            pStyleSheet = &pSSPool->Make( aStyleName, (SfxStyleFamily) nFamily,
                                                    SFXSTYLEBIT_USERDEF );

            if (pArgs->GetItemState(SID_STYLE_REFERENCE) == SFX_ITEM_ON)
            {
                String aParentName(((const SfxStringItem&) pArgs->Get(SID_STYLE_REFERENCE)).GetValue());
                pStyleSheet->SetParent(aParentName);
            }
            else
            {
                pStyleSheet->SetParent(String(SdResId(STR_STANDARD_STYLESHEET_NAME)));
            }
        }
        break;

        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            // Z.Z. geht immer noch der Dialog auf, um den Namen
            // der Vorlage einzugeben.
            if( pView->HasMarkedObj() || TRUE )
            {
                SfxStyleSheetBase *p = pSSPool->Find(aStyleName, (SfxStyleFamily) nFamily,
                                                        SFXSTYLEBIT_ALL );
                if(p) {
                    pSSPool->Erase(p);
                    p = 0;
                }
                pStyleSheet = &pSSPool->Make( aStyleName, (SfxStyleFamily) nFamily,
                                                        SFXSTYLEBIT_USERDEF );
                pStyleSheet->SetParent(String(SdResId(STR_STANDARD_STYLESHEET_NAME)));
            }
        }
        break;

        case SID_STYLE_EDIT:
            pStyleSheet = pSSPool->Find( aStyleName, (SfxStyleFamily) nFamily);
        break;

        case SID_STYLE_DELETE:
            pStyleSheet = pSSPool->Find( aStyleName, (SfxStyleFamily) nFamily);
            if( pStyleSheet )
            {
                pSSPool->Erase( pStyleSheet );
                nRetMask = TRUE;
                pDoc->SetChanged(TRUE);
            }
            nRetMask = FALSE;
        break;

        case SID_STYLE_APPLY:
            // Anwenden der Vorlage auf das Dokument
            pStyleSheet = pSSPool->Find( aStyleName, (SfxStyleFamily) nFamily);

            // keine Praesentationsobjektvorlagen, die werden nur
            // implizit zugewiesen
            if ( pStyleSheet && pStyleSheet->GetFamily() != SFX_STYLE_FAMILY_PSEUDO )
            {
                // Es darf auch keinen Praesentationsobjekten Vorlagen zugewiesen werden
                // Ausnahme: Hintergrundobjekte oder Draw (damit Praesentationsobjektvorlagen ueberteuert werden koennen)
                SfxStyleSheet* pOldStyleSheet = pView->GetStyleSheet();
                String aEmptyStr;
                if( !pOldStyleSheet ||
                    pOldStyleSheet->GetFamily() == SFX_STYLE_FAMILY_PARA ||
                    pOldStyleSheet->GetHelpId( aEmptyStr ) == HID_PSEUDOSHEET_BACKGROUNDOBJECTS ||
                    pDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW )
                {
                    pView->SetStyleSheet( (SfxStyleSheet*) pStyleSheet);
                    pDoc->SetChanged(TRUE);
                    pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
/* #96983# GrabFocus moved to stylist
            pWin->GrabFocus();
*/
        break;

        case SID_STYLE_WATERCAN:
        {
            if( !SD_MOD()->GetWaterCan() )
            {
                if( pArgs->GetItemState( nSlotId ) == SFX_ITEM_SET )
                {
                    aStyleName = ( ( (const SfxStringItem &) pArgs->Get( nSlotId ) ).GetValue() );
                    SD_MOD()->SetWaterCan( TRUE );
                    pStyleSheet = pSSPool->Find( aStyleName, (SfxStyleFamily) nFamily);
                }
                // keine Praesentationsobjektvorlagen, die werden nur
                // implizit zugewiesen
                if( pStyleSheet && pStyleSheet->GetFamily() != SFX_STYLE_FAMILY_PSEUDO )
                {
                    ( (SdStyleSheetPool*) pSSPool )->SetActualStyleSheet( pStyleSheet );

                    // Es wird explizit in den Selektionsmodus geschaltet
                    pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_OBJECT_SELECT,
                                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );

                }
                else
                    SD_MOD()->SetWaterCan( FALSE );
            }
            else
            {
                SD_MOD()->SetWaterCan( FALSE );
                // Werkzeugleiste muss wieder enabled werden
                pViewSh->Invalidate();
            }
        }
        break;

        default:
        break;
    }

    switch( nSlotId )
    {
        case SID_STYLE_NEW:
        case SID_STYLE_EDIT:
        {
            PresentationObjects ePO;

            if( pStyleSheet )
            {
                SdTabTemplateDlg*         pStdDlg  = NULL;
                SdPresLayoutTemplateDlg * pPresDlg = NULL;
                BOOL bOldDocInOtherLanguage = FALSE;
                SfxItemSet aOriSet( pStyleSheet->GetItemSet() );

                SfxStyleFamily eFamily = pStyleSheet->GetFamily();

                if (eFamily == SFX_STYLE_FAMILY_PARA)
                {
                    pStdDlg = new SdTabTemplateDlg( 0, pDoc->GetDocSh(),
                                                    *pStyleSheet, pDoc,
                                                    pView );
                }
                else if (eFamily == SFX_STYLE_FAMILY_PSEUDO)
                {
                    String aStyleName(pStyleSheet->GetName());
                    USHORT nDlgId = 0;

                    if (aStyleName == String(SdResId(STR_PSEUDOSHEET_TITLE)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;
                        ePO    = PO_TITLE;
                    }
                    else if (aStyleName == String(SdResId(STR_PSEUDOSHEET_SUBTITLE)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;
                        ePO    = PO_SUBTITLE;
                    }
                    else if (aStyleName ==
                             String(SdResId(STR_PSEUDOSHEET_BACKGROUND)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_2;
                        ePO    = PO_BACKGROUND;
                    }
                    else if (aStyleName ==
                             String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE;
                        ePO    = PO_BACKGROUNDOBJECTS;
                    }
                    else if (aStyleName ==
                             String(SdResId(STR_PSEUDOSHEET_NOTES)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;
                        ePO    = PO_NOTES;
                    }
                    else if(aStyleName.Search(String(SdResId(STR_PSEUDOSHEET_OUTLINE))) !=
                            STRING_NOTFOUND)
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;

                        String aOutlineStr((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                        SdStyleSheet* pRealStyle =
                            ((SdStyleSheet*)pStyleSheet)->GetRealStyleSheet();
                        // die Nummer ermitteln; ein Leerzeichen zwischen
                        // Name und Nummer beachten
                        String aNumStr(aStyleName.Copy(aOutlineStr.Len() + 1));
                        USHORT nLevel = (USHORT)aNumStr.ToInt32();
                        switch (nLevel)
                        {
                            case 1: ePO = PO_OUTLINE_1; break;
                            case 2: ePO = PO_OUTLINE_2; break;
                            case 3: ePO = PO_OUTLINE_3; break;
                            case 4: ePO = PO_OUTLINE_4; break;
                            case 5: ePO = PO_OUTLINE_5; break;
                            case 6: ePO = PO_OUTLINE_6; break;
                            case 7: ePO = PO_OUTLINE_7; break;
                            case 8: ePO = PO_OUTLINE_8; break;
                            case 9: ePO = PO_OUTLINE_9; break;
                        }
                    }
                    else
                    {
                        DBG_ERROR("Vorlage aus aelterer anderssprachiger Version");
                        bOldDocInOtherLanguage = TRUE;
                    }

                    if( !bOldDocInOtherLanguage )
                        pPresDlg = new SdPresLayoutTemplateDlg( pDocSh, NULL, SdResId(nDlgId), *pStyleSheet, ePO, pSSPool);
                }

                USHORT nResult = RET_CANCEL;
                const SfxItemSet* pOutSet = NULL;
                if (pStdDlg)
                {
                    nResult = pStdDlg->Execute();
                    pOutSet = pStdDlg->GetOutputItemSet();
                }
                else if( pPresDlg )
                {
                    nResult = pPresDlg->Execute();
                    pOutSet = pPresDlg->GetOutputItemSet();
                }

                switch( nResult )
                {
                    case RET_OK:
                    {
                        nRetMask = pStyleSheet->GetMask();

                        if (eFamily == SFX_STYLE_FAMILY_PSEUDO)
                        {
                            SfxItemSet aTempSet(*pOutSet);
                            ((SdStyleSheet*)pStyleSheet)->AdjustToFontHeight(aTempSet);

                            // Sonderbehandlung: die INVALIDS auf NULL-Pointer
                            // zurueckgesetzen (sonst landen INVALIDs oder
                            // Pointer auf die DefaultItems in der Vorlage;
                            // beides wuerde die Attribut-Vererbung unterbinden)
                            aTempSet.ClearInvalidItems();

                            // EE_PARA_NUMBULLET item is only valid in first outline template
                            if( (ePO >= PO_OUTLINE_2) && (ePO <= PO_OUTLINE_9) )
                            {
                                if (aTempSet.GetItemState(EE_PARA_NUMBULLET) == SFX_ITEM_SET)
                                {
                                    SvxNumRule aRule(*((SvxNumBulletItem*)aTempSet.GetItem(EE_PARA_NUMBULLET))->GetNumRule());

                                    String aStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                                    aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
                                    SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SFX_STYLE_FAMILY_PSEUDO);

                                    if(pFirstStyleSheet)
                                    {
                                        pFirstStyleSheet->GetItemSet().Put( SvxNumBulletItem( aRule, EE_PARA_NUMBULLET ));
                                        SdStyleSheet* pRealSheet = ((SdStyleSheet*)pFirstStyleSheet)->GetRealStyleSheet();
                                        pRealSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                                    }

                                    aTempSet.ClearItem( EE_PARA_NUMBULLET );
                                }
                            }

                            String aStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                            aStyleName.Append( sal_Unicode( ' ' ));

                            pStyleSheet->GetItemSet().Put(aTempSet);
                            SdStyleSheet* pRealSheet =((SdStyleSheet*)pStyleSheet)->GetRealStyleSheet();
                            pRealSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));

                            if( (ePO >= PO_OUTLINE_1) && (ePO <= PO_OUTLINE_8) )
                            {
                                for( USHORT n = (ePO - PO_OUTLINE_1 + 2); n < 10; n++ )
                                {
                                    String aName( aStyleName );
                                    aName.Append( String::CreateFromInt32( (sal_Int32) n ));

                                    SfxStyleSheetBase* pSheet = pSSPool->Find( aName, SFX_STYLE_FAMILY_PSEUDO);

                                    if(pSheet)
                                    {
                                        SdStyleSheet* pRealSheet = ((SdStyleSheet*)pSheet)->GetRealStyleSheet();
                                        pRealSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                                    }
                                }
                            }
                        }

                        SfxItemSet& rAttr = pStyleSheet->GetItemSet();
                        if ( rAttr.GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON )
                        {
                            // SvxLRSpaceItem hart gesetzt: NumBulletItem anpassen
                            if ( aOriSet.GetItemState( EE_PARA_LRSPACE ) != SFX_ITEM_ON ||
                                    (const SvxLRSpaceItem&) aOriSet.Get( EE_PARA_LRSPACE ) !=
                                    (const SvxLRSpaceItem&) rAttr.Get( EE_PARA_LRSPACE ) )
                            {
                                SvxNumBulletItem aNumBullet( (const SvxNumBulletItem&) rAttr.Get(EE_PARA_NUMBULLET) );

                                sal_uInt16 nLevel = 0;
                                if( (ePO >= PO_OUTLINE_2) && (ePO <= PO_OUTLINE_9) )
                                    nLevel = ePO - PO_OUTLINE_1 + 1;

                                EditEngine::ImportBulletItem( aNumBullet, nLevel, NULL,
                                                        &(const SvxLRSpaceItem&) rAttr.Get( EE_PARA_LRSPACE ) );

                                // the numbering bullet item is not valid in styles Outline 2 to Outline 9
                                if( nLevel != 0 )
                                {
                                    // so put it into Outline 1 then..
                                    String aStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                                    aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
                                    SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SFX_STYLE_FAMILY_PSEUDO);

                                    if(pFirstStyleSheet)
                                    {
                                        pFirstStyleSheet->GetItemSet().Put( aNumBullet);
                                        SdStyleSheet* pRealSheet = ((SdStyleSheet*)pFirstStyleSheet)->GetRealStyleSheet();
                                        pRealSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                                    }
                                }
                                else
                                {
                                    ( (SfxItemSet&) rAttr).Put( aNumBullet );
                                }
                            }
                        }

                        // check for unique names of named items for xml
                        if( rAttr.GetItemState( XATTR_FILLBITMAP ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLBITMAP );
                            SfxPoolItem* pNewItem = ((XFillBitmapItem*)pOldItem)->checkForUniqueItem( pDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINEDASH ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINEDASH );
                            SfxPoolItem* pNewItem = ((XLineDashItem*)pOldItem)->checkForUniqueItem( pDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINESTART ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINESTART );
                            SfxPoolItem* pNewItem = ((XLineStartItem*)pOldItem)->checkForUniqueItem( pDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINEEND ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINEEND );
                            SfxPoolItem* pNewItem = ((XLineEndItem*)pOldItem)->checkForUniqueItem( pDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLGRADIENT ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLGRADIENT );
                            SfxPoolItem* pNewItem = ((XFillGradientItem*)pOldItem)->checkForUniqueItem( pDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLFLOATTRANSPARENCE ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLFLOATTRANSPARENCE );
                            SfxPoolItem* pNewItem = ((XFillFloatTransparenceItem*)pOldItem)->checkForUniqueItem( pDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLHATCH ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLHATCH );
                            SfxPoolItem* pNewItem = ((XFillHatchItem*)pOldItem)->checkForUniqueItem( pDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }

                        ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

                        if ( pViewSh->ISA( SdDrawViewShell ) )
                        {
                            PageKind ePageKind = ( (SdDrawViewShell*) pViewShell )->GetPageKind();
                            if( ePageKind == PK_NOTES || ePageKind == PK_HANDOUT )
                            {
                                SdPage* pPage = pViewSh->GetActualPage();

                                if( ( (SdDrawViewShell*) pViewShell )->GetEditMode() == EM_MASTERPAGE )
                                    pPage = (SdPage*) pPage->GetMasterPage( 0 );

                                if( pPage )
                                {
                                    SdrObjListIter aIter( *pPage );
                                    while( aIter.IsMore() )
                                    {
                                        SdrObject* pObj = aIter.Next();
                                        if( pObj->ISA(SdrPageObj) )
                                            pObj->SendRepaintBroadcast();
                                    }
                                }
                            }
                        }

                        if( pDoc->GetOnlineSpell() )
                        {
                            const SfxPoolItem* pItem;
                            if( SFX_ITEM_SET == rAttr.GetItemState(EE_CHAR_LANGUAGE, FALSE, &pItem ) ||
                                SFX_ITEM_SET == rAttr.GetItemState(EE_CHAR_LANGUAGE_CJK, FALSE, &pItem ) ||
                                SFX_ITEM_SET == rAttr.GetItemState(EE_CHAR_LANGUAGE_CTL, FALSE, &pItem ) )
                            {
                                pDoc->StopOnlineSpelling();
                                pDoc->StartOnlineSpelling();
                            }
                        }

                        pDoc->SetChanged(TRUE);
                    }
                    break;

                    default:
                    {
                        if( nSlotId == SID_STYLE_NEW )
                            pSSPool->Erase( pStyleSheet );
                        delete pStdDlg;
                        delete pPresDlg;
                    }
                    return; // Abbruch
                }
                delete pStdDlg;
                delete pPresDlg;
            }
        }
        break;

        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if( pStyleSheet )
            {
                nRetMask = pStyleSheet->GetMask();
                SfxItemSet aCoreSet( pDoc->GetPool() );
                pView->GetAttributes( aCoreSet, TRUE );

                // wenn das Objekt eine Vorlage hatte, wird diese Parent
                // der neuen Vorlage
                SfxStyleSheet* pOldStyle = pView->GetStyleSheet();

                // Wenn pOldStyle == pStyleSheet -> Rekursion
                if( pOldStyle != pStyleSheet )
                {
                    if (pOldStyle)
                    {
                        pStyleSheet->SetParent(pOldStyle->GetName());
                    }

                    SfxItemSet* pStyleSet = &pStyleSheet->GetItemSet();
                    pStyleSet->Put(aCoreSet);

                    // Vorlage anwenden (Aber nicht, wenn gerade ein Text
                    // editiert wird, denn dazu muesste die Edit Engine
                    // Vorlagen auf Zeichenebene beherrschen.)
                    if (!pView->GetTextEditObject())
                    {
                        pView->SetStyleSheet( (SfxStyleSheet*) pStyleSheet);
                    }

                    ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
                    pDoc->SetChanged(TRUE);

                    pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
        }
        break;

        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            if (pView->HasMarkedObj() &&
                pView->GetMarkList().GetMarkCount() == 1 ||
                pView->ISA(SdOutlineView))
            {
                pStyleSheet = pView->GetStyleSheet();

                if( pStyleSheet )
                {
                    nRetMask = pStyleSheet->GetMask();
                    SfxItemSet aCoreSet( pDoc->GetPool() );
                    pView->GetAttributes( aCoreSet );

                    SfxItemSet* pStyleSet = &pStyleSheet->GetItemSet();
                    pStyleSet->Put( aCoreSet );

                    pView->SetStyleSheet( (SfxStyleSheet*) pStyleSheet);

                    ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
                    pDoc->SetChanged(TRUE);
                    pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
        }
        break;

    }
    if( nRetMask != 0xffff )
        rReq.SetReturnValue( SfxUInt16Item( nSlotId, nRetMask ) );
}


