/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: futempl.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:40:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#ifndef  _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#include "futempl.hxx"

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
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVDOPAGE_HXX
#include <svx/svdopage.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

#include <sfx2/viewfrm.hxx>

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
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "res_bmp.hrc"
#include "glob.hrc"
#include "prlayout.hxx"         // enum PresentationObjects
#include "prltempl.hrc"         // TAB_PRES_LAYOUT_TEMPLATE_x
#ifndef _SVX_TAB_AREA_HXX
#include <svx/tabarea.hxx>
#endif
#include "sdresid.hxx"
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "strings.hrc"
#include "helpids.h"
#include "sdabstdlg.hxx"

using rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::style;

namespace sd
{

TYPEINIT1( FuTemplate, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuTemplate::FuTemplate (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor( pViewSh, pWin, pView, pDoc, rReq )
{
}

FunctionReference FuTemplate::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuTemplate( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuTemplate::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    USHORT nSId = rReq.GetSlot();

    // StyleSheet-Parameter holen
    SfxStyleSheetBasePool* pSSPool = mpDoc->GetDocSh()->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = NULL;

    const SfxPoolItem* pItem;
    USHORT nFamily = USHRT_MAX;
    if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILY,
        FALSE, &pItem ))
    {
        nFamily = ( (const SfxUInt16Item &) pArgs->Get( SID_STYLE_FAMILY ) ).GetValue();
    }
    else
    if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILYNAME,
        FALSE, &pItem ))
    {
        String sFamily = ( (const SfxStringItem &) pArgs->Get( SID_STYLE_FAMILYNAME ) ).GetValue();
        if (sFamily.CompareToAscii("graphics") == COMPARE_EQUAL)
            nFamily = SD_STYLE_FAMILY_GRAPHICS;
        else
            nFamily = SD_STYLE_FAMILY_PSEUDO;
    }

    String aStyleName;
    USHORT nRetMask = 0xffff;

    switch( nSId )
    {
        case SID_STYLE_APPLY:
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_FAMILY:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, SID_APPLY_STYLE, sal_False );
            SFX_REQUEST_ARG( rReq, pFamilyItem, SfxStringItem, SID_STYLE_FAMILYNAME, sal_False );
            if ( pFamilyItem && pNameItem )
            {
                try
                {
                    Reference< XStyleFamiliesSupplier > xModel(mpDoc->GetDocSh()->GetModel(), UNO_QUERY_THROW );
                    Reference< XNameAccess > xCont( xModel->getStyleFamilies() );
                    Reference< XNameAccess > xStyles( xCont->getByName(pFamilyItem->GetValue()), UNO_QUERY_THROW );
                    Reference< XPropertySet > xInfo( xStyles->getByName( pNameItem->GetValue() ), UNO_QUERY_THROW );

                    OUString aUIName;
                    xInfo->getPropertyValue( ::rtl::OUString::createFromAscii("DisplayName") ) >>= aUIName;
                    if ( aUIName.getLength() )
                        rReq.AppendItem( SfxStringItem( nSId, aUIName ) );
                }
                catch( Exception& )
                {
                }
            }

            if (pArgs->GetItemState(nSId) == SFX_ITEM_SET)
                aStyleName = ( ( (const SfxStringItem &) pArgs->Get( nSId ) ).GetValue() );
        }
    }

    switch( nSId )
    {
        case SID_STYLE_NEW:
        {
            SfxStyleSheetBase *p = pSSPool->Find(aStyleName, (SfxStyleFamily) nFamily, SFXSTYLEBIT_ALL );
            if(p)
            {
                pSSPool->Remove(p);
                p = 0;
            }
            pStyleSheet = &pSSPool->Make( aStyleName, (SfxStyleFamily) nFamily, SFXSTYLEBIT_USERDEF );

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
            if( mpView->AreObjectsMarked() || TRUE )
            {
                SfxStyleSheetBase *p = pSSPool->Find(aStyleName, (SfxStyleFamily) nFamily, SFXSTYLEBIT_ALL );
                if(p)
                {
                    pSSPool->Remove(p);
                    p = 0;
                }
                pStyleSheet = &pSSPool->Make( aStyleName, (SfxStyleFamily) nFamily, SFXSTYLEBIT_USERDEF );
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
                pSSPool->Remove( pStyleSheet );
                nRetMask = TRUE;
                mpDoc->SetChanged(TRUE);
            }
            else
            {
                nRetMask = FALSE;
            }
        break;

        case SID_STYLE_APPLY:
            // Anwenden der Vorlage auf das Dokument
            pStyleSheet = pSSPool->Find( aStyleName, (SfxStyleFamily) nFamily);

            // do not set presentation styles, they will be set implicit
            if ( pStyleSheet && pStyleSheet->GetFamily() != SD_STYLE_FAMILY_PSEUDO )
            {
                SfxStyleSheet* pOldStyleSheet = mpView->GetStyleSheet();
                String aStr;

                if( // if the object had no style sheet, allow all
                    !pOldStyleSheet ||

                    // allow if old and new style sheet has same family
                    pStyleSheet->GetFamily() == pOldStyleSheet->GetFamily() ||

                    // allow if old was background objects and new is graphics
                    pStyleSheet->GetFamily() == SD_STYLE_FAMILY_GRAPHICS && pOldStyleSheet->GetHelpId( aStr ) == HID_PSEUDOSHEET_BACKGROUNDOBJECTS ||

                    // allow if old was presentation and we are a drawing document
                    pOldStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE && mpDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW )
                {
                    mpView->SetStyleSheet( (SfxStyleSheet*) pStyleSheet);
                    mpDoc->SetChanged(TRUE);
                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
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
                if( pArgs->GetItemState( nSId ) == SFX_ITEM_SET )
                {
                    aStyleName = ( ( (const SfxStringItem &) pArgs->Get( nSId ) ).GetValue() );
                    SD_MOD()->SetWaterCan( TRUE );
                    pStyleSheet = pSSPool->Find( aStyleName, (SfxStyleFamily) nFamily);
                }
                // keine Praesentationsobjektvorlagen, die werden nur
                // implizit zugewiesen
                if( pStyleSheet && pStyleSheet->GetFamily() != SD_STYLE_FAMILY_PSEUDO )
                {
                    ( (SdStyleSheetPool*) pSSPool )->SetActualStyleSheet( pStyleSheet );

                    // Es wird explizit in den Selektionsmodus geschaltet
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_OBJECT_SELECT,
                                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );

                }
                else
                    SD_MOD()->SetWaterCan( FALSE );
            }
            else
            {
                SD_MOD()->SetWaterCan( FALSE );
                // Werkzeugleiste muss wieder enabled werden
                mpViewShell->Invalidate();
            }
        }
        break;

        default:
        break;
    }

    switch( nSId )
    {
        case SID_STYLE_NEW:
        case SID_STYLE_EDIT:
        {
            PresentationObjects ePO = PO_OUTLINE_1;

            if( pStyleSheet )
            {
                SfxAbstractTabDialog*  pStdDlg  = NULL;
                SfxAbstractTabDialog*  pPresDlg = NULL;
                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                BOOL bOldDocInOtherLanguage = FALSE;
                SfxItemSet aOriSet( pStyleSheet->GetItemSet() );

                SfxStyleFamily eFamily = pStyleSheet->GetFamily();

                if (eFamily == SD_STYLE_FAMILY_GRAPHICS)
                {
                    pStdDlg = pFact ? pFact->CreateSdTabTemplateDlg( 0, mpDoc->GetDocSh(), *pStyleSheet, mpDoc, mpView ) : 0;
                }
                else if (eFamily == SD_STYLE_FAMILY_PSEUDO)
                {
                    String aName(pStyleSheet->GetName());
                    USHORT nDlgId = 0;

                    if (aName == String(SdResId(STR_PSEUDOSHEET_TITLE)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;
                        ePO    = PO_TITLE;
                    }
                    else if (aName == String(SdResId(STR_PSEUDOSHEET_SUBTITLE)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;
                        ePO    = PO_SUBTITLE;
                    }
                    else if (aName ==
                             String(SdResId(STR_PSEUDOSHEET_BACKGROUND)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_2;
                        ePO    = PO_BACKGROUND;
                    }
                    else if (aName ==
                             String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE;
                        ePO    = PO_BACKGROUNDOBJECTS;
                    }
                    else if (aName ==
                             String(SdResId(STR_PSEUDOSHEET_NOTES)))
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;
                        ePO    = PO_NOTES;
                    }
                    else if(aName.Search(String(SdResId(STR_PSEUDOSHEET_OUTLINE))) !=
                            STRING_NOTFOUND)
                    {
                        nDlgId = TAB_PRES_LAYOUT_TEMPLATE_3;

                        String aOutlineStr((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                        // die Nummer ermitteln; ein Leerzeichen zwischen
                        // Name und Nummer beachten
                        String aNumStr(aName.Copy(aOutlineStr.Len() + 1));
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
                    {
                        pPresDlg = pFact ? pFact->CreateSdPresLayoutTemplateDlg( mpDocSh, NULL, SdResId(nDlgId), *pStyleSheet, ePO, pSSPool ) : 0;
                    }
                }
                else if (eFamily == SD_STYLE_FAMILY_CELL)
                {
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

                        if (eFamily == SD_STYLE_FAMILY_PSEUDO)
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

                                    String sStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                                    sStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
                                    SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( sStyleName, SD_STYLE_FAMILY_PSEUDO);

                                    if(pFirstStyleSheet)
                                    {
                                        pFirstStyleSheet->GetItemSet().Put( SvxNumBulletItem( aRule, EE_PARA_NUMBULLET ));
                                        SdStyleSheet* pRealSheet = ((SdStyleSheet*)pFirstStyleSheet)->GetRealStyleSheet();
                                        pRealSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                                    }

                                    aTempSet.ClearItem( EE_PARA_NUMBULLET );
                                }
                            }

                            String sStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                            sStyleName.Append( sal_Unicode( ' ' ));

                            pStyleSheet->GetItemSet().Put(aTempSet);
                            SdStyleSheet* pRealSheet =((SdStyleSheet*)pStyleSheet)->GetRealStyleSheet();
                            pRealSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));

                            if( (ePO >= PO_OUTLINE_1) && (ePO <= PO_OUTLINE_8) )
                            {
                                for( USHORT n = (USHORT)(ePO - PO_OUTLINE_1 + 2); n < 10; n++ )
                                {
                                    String aName( sStyleName );
                                    aName.Append( String::CreateFromInt32( (sal_Int32) n ));

                                    SfxStyleSheetBase* pSheet = pSSPool->Find( aName, SD_STYLE_FAMILY_PSEUDO);

                                    if(pSheet)
                                    {
                                        SdStyleSheet* pRealStyleSheet = ((SdStyleSheet*)pSheet)->GetRealStyleSheet();
                                        pRealStyleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
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
                                    nLevel = (sal_uInt16)(ePO - PO_OUTLINE_1 + 1);

                                EditEngine::ImportBulletItem( aNumBullet, nLevel, NULL,
                                                        &(const SvxLRSpaceItem&) rAttr.Get( EE_PARA_LRSPACE ) );

                                // the numbering bullet item is not valid in styles Outline 2 to Outline 9
                                if( nLevel != 0 )
                                {
                                    // so put it into Outline 1 then..
                                    String sStyleName((SdResId(STR_PSEUDOSHEET_OUTLINE)));
                                    sStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
                                    SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( sStyleName, SD_STYLE_FAMILY_PSEUDO);

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
                            SfxPoolItem* pNewItem = ((XFillBitmapItem*)pOldItem)->checkForUniqueItem( mpDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINEDASH ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINEDASH );
                            SfxPoolItem* pNewItem = ((XLineDashItem*)pOldItem)->checkForUniqueItem( mpDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINESTART ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINESTART );
                            SfxPoolItem* pNewItem = ((XLineStartItem*)pOldItem)->checkForUniqueItem( mpDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINEEND ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINEEND );
                            SfxPoolItem* pNewItem = ((XLineEndItem*)pOldItem)->checkForUniqueItem( mpDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLGRADIENT ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLGRADIENT );
                            SfxPoolItem* pNewItem = ((XFillGradientItem*)pOldItem)->checkForUniqueItem( mpDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLFLOATTRANSPARENCE ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLFLOATTRANSPARENCE );
                            SfxPoolItem* pNewItem = ((XFillFloatTransparenceItem*)pOldItem)->checkForUniqueItem( mpDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLHATCH ) == SFX_ITEM_SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLHATCH );
                            SfxPoolItem* pNewItem = ((XFillHatchItem*)pOldItem)->checkForUniqueItem( mpDoc );
                            if( pOldItem != pNewItem )
                            {
                                rAttr.Put( *pNewItem );
                                delete pNewItem;
                            }
                        }

                        ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

                        DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( mpViewShell );
                        if( pDrawViewShell )
                        {
                            PageKind ePageKind = pDrawViewShell->GetPageKind();
                            if( ePageKind == PK_NOTES || ePageKind == PK_HANDOUT )
                            {
                                SdPage* pPage = mpViewShell->GetActualPage();

                                if(pDrawViewShell->GetEditMode() == EM_MASTERPAGE)
                                {
                                    pPage = static_cast<SdPage*>((&(pPage->TRG_GetMasterPage())));
                                }

                                if( pPage )
                                {
                                    SdrObjListIter aIter( *pPage );
                                    while( aIter.IsMore() )
                                    {
                                        SdrObject* pObj = aIter.Next();
                                        if( pObj->ISA(SdrPageObj) )
                                        {
                                            // repaint only
                                            pObj->ActionChanged();
                                            // pObj->SendRepaintBroadcast();
                                        }
                                    }
                                }
                            }
                        }

                        if( mpDoc->GetOnlineSpell() )
                        {
                            const SfxPoolItem* pTempItem;
                            if( SFX_ITEM_SET == rAttr.GetItemState(EE_CHAR_LANGUAGE, FALSE, &pTempItem ) ||
                                SFX_ITEM_SET == rAttr.GetItemState(EE_CHAR_LANGUAGE_CJK, FALSE, &pTempItem ) ||
                                SFX_ITEM_SET == rAttr.GetItemState(EE_CHAR_LANGUAGE_CTL, FALSE, &pTempItem ) )
                            {
                                mpDoc->StopOnlineSpelling();
                                mpDoc->StartOnlineSpelling();
                            }
                        }

                        mpDoc->SetChanged(TRUE);
                    }
                    break;

                    default:
                    {
                        if( nSId == SID_STYLE_NEW )
                            pSSPool->Remove( pStyleSheet );
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
                SfxItemSet aCoreSet( mpDoc->GetPool() );
                mpView->GetAttributes( aCoreSet, TRUE );

                // wenn das Objekt eine Vorlage hatte, wird diese Parent
                // der neuen Vorlage
                SfxStyleSheet* pOldStyle = mpView->GetStyleSheet();

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
                    if (!mpView->GetTextEditObject())
                    {
                        mpView->SetStyleSheet( (SfxStyleSheet*) pStyleSheet);
                    }

                    ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
                    mpDoc->SetChanged(TRUE);

                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
        }
        break;

        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            if (mpView->AreObjectsMarked() &&
                mpView->GetMarkedObjectList().GetMarkCount() == 1 ||
                mpView->ISA(OutlineView))
            {
                pStyleSheet = mpView->GetStyleSheet();

                if( pStyleSheet )
                {
                    nRetMask = pStyleSheet->GetMask();
                    SfxItemSet aCoreSet( mpDoc->GetPool() );
                    mpView->GetAttributes( aCoreSet );

                    SfxItemSet* pStyleSet = &pStyleSheet->GetItemSet();
                    pStyleSet->Put( aCoreSet );

                    mpView->SetStyleSheet( (SfxStyleSheet*) pStyleSheet);

                    ( (SfxStyleSheet*) pStyleSheet )->Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
                    mpDoc->SetChanged(TRUE);
                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
        }
        break;

    }
    if( nRetMask != 0xffff )
        rReq.SetReturnValue( SfxUInt16Item( nSId, nRetMask ) );
}

void FuTemplate::Activate()
{
}

void FuTemplate::Deactivate()
{
}

} // end of namespace sd
