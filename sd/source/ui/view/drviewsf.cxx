/*************************************************************************
 *
 *  $RCSfile: drviewsf.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:12:21 $
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

#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _GLOBL3D_HXX
#include <svx/globl3d.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX
#include <svx/hlnkitem.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _FLDITEM_HXX
#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _BINDING_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <svx/xdef.hxx>
#endif
#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX //autogen wg. SfxTopViewFrame
#include <sfx2/topfrm.hxx>
#endif

#include "frmview.hxx"
#include "sdoutl.hxx"
#include "app.hrc"

#include "app.hxx"
#include "drviewsh.hxx"
#include "stlsheet.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"

#include "sdwindow.hxx"
#include "prevchld.hxx"
#include "preview.hxx"
#include "cfgids.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;


/*************************************************************************
|*
|* Status von Controller-SfxSlots setzen
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::GetCtrlState(SfxItemSet &rSet)
{
//  rSet.Put(SfxUInt16Item(SID_SWITCHPAGE, aTabControl.GetCurPageId()));
//  rSet.Put(SfxUInt16Item(SID_SWITCHLAYER, aLayerTab.GetCurPageId()));

    if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
    {
        // "Letzte Version" vom SFx en/disablen lassen
        SfxViewFrame* pViewFrame = GetViewFrame();
        DBG_ASSERT(pViewFrame, "ViewFrame nicht gefunden");
        if (pViewFrame->ISA(SfxTopViewFrame))
        {
            pViewFrame->GetSlotState (SID_RELOAD, NULL, &rSet);
        }
        else        // MI sagt: kein MDIFrame --> disablen
        {
            rSet.DisableItem(SID_RELOAD);
        }
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_HYPERLINK_GETLINK))
    {
        SvxHyperlinkItem aHLinkItem;

        OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();

        if (pOLV)
        {
            const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

            if (pFieldItem)
            {
                const SvxFieldData* pField = pFieldItem->GetField();

                if (pField->ISA(SvxURLField))
                {
                    aHLinkItem.SetName(((const SvxURLField*) pField)->GetRepresentation());
                    aHLinkItem.SetURL(((const SvxURLField*) pField)->GetURL());
                    aHLinkItem.SetTargetFrame(((const SvxURLField*) pField)->GetTargetFrame());
                }
            }
        }
        else
        {
            if (pDrView->GetMarkList().GetMarkCount() > 0)
            {
                SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pDrView->GetMarkList().GetMark(0)->GetObj());

                if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
                {
                    uno::Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );

                    if( !xControlModel.is() )
                        return;

                    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
                    if( !xPropSet.is())
                        return;

                    uno::Reference< beans::XPropertySetInfo > xPropInfo( xPropSet->getPropertySetInfo() );

                    uno::Any aTmp;

                    form::FormButtonType eButtonType = form::FormButtonType_URL;
                    if(!xPropInfo->hasPropertyByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" ))))
                        return;

                    aTmp = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" )));
                    if( aTmp >>= eButtonType )
                    {
                        OUString aString;

                        // Label
                        if(!xPropInfo->hasPropertyByName(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" ))))
                            return;
                        aTmp = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" )));

                        if( aTmp >>= aString )
                            aHLinkItem.SetName(String( aString ));

                        // URL
                        if(!xPropInfo->hasPropertyByName(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" ))))
                            return;
                        aTmp = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" )));
                        if( aTmp >>= aString )
                            aHLinkItem.SetURL(String( aString ));

                        // Target
                        if(!xPropInfo->hasPropertyByName(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" ))))
                            return;
                        aTmp = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" )));
                        if( aTmp >>= aString )
                            aHLinkItem.SetTargetFrame(String( aString ));

                        aHLinkItem.SetInsertMode(HLINK_BUTTON);
                    }
                }
            }
        }

        rSet.Put(aHLinkItem);
    }

    // Ausgabequalitaet
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_COLOR ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_GRAYSCALE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_BLACKWHITE ) )
    {
        ULONG nMode = pWindow->GetDrawMode();
        UINT16 nQuality = 3;
        if( nMode == DRAWMODE_DEFAULT )
            nQuality = 0;
        else if( nMode == ( DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT |
                            DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ) )
            nQuality = 1;
        else if( nMode == ( DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL |
                    DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT ) )
            nQuality = 2;

        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_COLOR, (BOOL)(nQuality == 0) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_GRAYSCALE, (BOOL)(nQuality == 1) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_BLACKWHITE, (BOOL)(nQuality == 2) ) );
    }

    // Preview-Fenster
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_COLOR ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_GRAYSCALE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_BLACKWHITE ) )
    {
        USHORT nId = SdPreviewChildWindow::GetChildWindowId();
        if( GetViewFrame()->GetChildWindow( nId ) )
        {
            ULONG nMode = pFrameView->GetPreviewDrawMode();
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_COLOR, (BOOL)(nMode == PREVIEW_DRAWMODE_COLOR) ) );
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_GRAYSCALE, (BOOL)(nMode == PREVIEW_DRAWMODE_GRAYSCALE) ) );
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_BLACKWHITE, (BOOL)(nMode == PREVIEW_DRAWMODE_BLACKWHITE) ) );
        }
        else
        {
            rSet.DisableItem( SID_PREVIEW_QUALITY_COLOR );
            rSet.DisableItem( SID_PREVIEW_QUALITY_GRAYSCALE );
            rSet.DisableItem( SID_PREVIEW_QUALITY_BLACKWHITE );
        }
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) )
    {
        rSet.Put( SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, TRUE ) );
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_ATTR_YEAR2000) )
    {

        FmFormShell* pShell = (FmFormShell*) aShellTable.Get(RID_FORMLAYER_TOOLBOX);
        if( pShell )
        {
            UINT16 nState = 0;
            if ( pShell->GetY2KState(nState) )
                rSet.Put( SfxUInt16Item( SID_ATTR_YEAR2000, nState ) );
            else
                rSet.DisableItem( SID_ATTR_YEAR2000 );
        }
    }
}


/*************************************************************************
|*
|* Status der Attribut-Items
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::GetAttrState( SfxItemSet& rSet )
{
    SfxWhichIter    aIter( rSet );
    USHORT          nWhich = aIter.FirstWhich();

    BOOL    bAttr = FALSE;
    SfxAllItemSet aAllSet( *rSet.GetPool() );

    while ( nWhich )
    {
        USHORT nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_ATTR_FILL_STYLE:
            case SID_ATTR_FILL_COLOR:
            case SID_ATTR_FILL_GRADIENT:
            case SID_ATTR_FILL_HATCH:
            case SID_ATTR_FILL_BITMAP:
            case SID_ATTR_FILL_SHADOW:
            case SID_ATTR_LINE_STYLE:
            case SID_ATTR_LINE_DASH:
            case SID_ATTR_LINE_WIDTH:
            case SID_ATTR_LINE_COLOR:
            case SID_ATTR_TEXT_FITTOSIZE:
            {
                bAttr = TRUE;
            }
            break;

            case SID_HYPHENATION:
            {
                SfxItemSet aAttrs( pDoc->GetPool() );
                pDrView->GetAttributes( aAttrs );
                if( aAttrs.GetItemState( EE_PARA_HYPHENATE ) >= SFX_ITEM_AVAILABLE )
                {
                    BOOL bValue = ( (const SfxBoolItem&) aAttrs.Get( EE_PARA_HYPHENATE ) ).GetValue();
                    rSet.Put( SfxBoolItem( SID_HYPHENATION, bValue ) );
                }
            }
            break;

            case SID_STYLE_FAMILY2:
            case SID_STYLE_FAMILY5:
            case SID_STYLE_APPLY: // StyleControl
            {
                SfxStyleSheet* pStyleSheet = pDrView->GetStyleSheet();
                if( pStyleSheet )
                {
                    if( nSlotId != SID_STYLE_APPLY && !pDrView->HasMarkedObj() )
                    {
                        SfxTemplateItem aTmpItem( nWhich, String() );
                        aAllSet.Put( aTmpItem, aTmpItem.Which()  );
                    }
                    else
                    {
                        if (pStyleSheet->GetFamily() == SD_LT_FAMILY)
                            pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

                        if( pStyleSheet )
                        {
                            SfxStyleFamily eFamily = pStyleSheet->GetFamily();

                            if ((eFamily == SFX_STYLE_FAMILY_PARA &&
                                nSlotId == SID_STYLE_FAMILY2)        ||
                                (eFamily == SFX_STYLE_FAMILY_PSEUDO &&
                                nSlotId == SID_STYLE_FAMILY5))
                            {
                                SfxTemplateItem aTmpItem ( nWhich, pStyleSheet->GetName() );
                                aAllSet.Put( aTmpItem, aTmpItem.Which()  );
                            }
                            else
                            {
                                SfxTemplateItem aTmpItem(nWhich, String());
                                aAllSet.Put(aTmpItem,aTmpItem.Which()  );
                            }
                        }
                    }
                }
                else
                {   SfxTemplateItem aItem( nWhich, String() );
                    aAllSet.Put( aItem, aItem.Which() );
                    // rSet.DisableItem( nWhich );
                }
            }
            break;

            case SID_SET_DEFAULT:
            {
                if( !pDrView->GetStyleSheet() )
                    rSet.DisableItem( nWhich );
            }
            break;

            case SID_STYLE_WATERCAN:
            {
                ISfxTemplateCommon* pTemplateCommon = SFX_APP()->GetCurrentTemplateCommon(GetViewFrame()->GetBindings());
                if (pTemplateCommon && pTemplateCommon->GetActualFamily() == SFX_STYLE_FAMILY_PSEUDO)
                    rSet.Put(SfxBoolItem(nWhich,FALSE));
                else
                {
                    SfxBoolItem aItem(nWhich, SD_MOD()->GetWaterCan());
                    aAllSet.Put( aItem, aItem.Which());
                }
            }
            break;

            case SID_STYLE_NEW:
            {
                ISfxTemplateCommon* pTemplateCommon = SFX_APP()->GetCurrentTemplateCommon(GetViewFrame()->GetBindings());
                if (pTemplateCommon && pTemplateCommon->GetActualFamily() == SFX_STYLE_FAMILY_PSEUDO)
                    rSet.DisableItem(nWhich);
            }
            break;

            case SID_STYLE_DRAGHIERARCHIE:
            {
                ISfxTemplateCommon* pTemplateCommon = SFX_APP()->GetCurrentTemplateCommon(GetViewFrame()->GetBindings());
                if (pTemplateCommon && pTemplateCommon->GetActualFamily() == SFX_STYLE_FAMILY_PSEUDO)
                    rSet.DisableItem(nWhich);
            }
            break;

            case SID_STYLE_NEW_BY_EXAMPLE:
            {
                // PseudoStyleSheets koennen nicht 'by Example' erzeugt werden;
                // normale StyleSheets brauchen dafuer ein selektiertes Objekt
                ISfxTemplateCommon* pTemplCommon = SFX_APP()->GetCurrentTemplateCommon(GetViewFrame()->GetBindings());
                if (pTemplCommon)
                {
                    if (pTemplCommon->GetActualFamily() == SFX_STYLE_FAMILY_PSEUDO)
                    {
                        rSet.DisableItem(nWhich);
                    }
                    else if (pTemplCommon->GetActualFamily() == SFX_STYLE_FAMILY_PARA)
                    {
                        if (!pDrView->HasMarkedObj())
                        {
                            rSet.DisableItem(nWhich);
                        }
                    }
                }
                // falls (noch) kein Gestalter da ist, muessen wir uns auf den
                // View-Zustand zurueckziehen; eine aktuell eingestellte Familie
                // kann nicht beruecksichtigt werden
                else
                {
                    if (!pDrView->HasMarkedObj())
                    {
                        rSet.DisableItem(nWhich);
                    }
                }

            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                if (!pDrView->HasMarkedObj())
                {
                    rSet.DisableItem(nWhich);
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }

    SfxItemSet* pSet = NULL;

    if( bAttr )
    {
        pSet = new SfxItemSet( pDoc->GetPool() );
        pDrView->GetAttributes( *pSet );
        rSet.Put( *pSet, FALSE );
    }

    rSet.Put( aAllSet, FALSE );

    // Flaechen und/oder Linienattribute wurden geaendert
    if( bAttr && pSet )
    {
        // Wenn die View selektierte Objekte besitzt, muessen entspr. Items
        // von SFX_ITEM_DEFAULT (_ON) auf SFX_ITEM_DISABLED geaendert werden
        if( pDrView->HasMarkedObj() )
        {
            SfxWhichIter aNewIter( *pSet, XATTR_LINE_FIRST, XATTR_FILL_LAST );
            nWhich = aNewIter.FirstWhich();
            while( nWhich )
            {
                if( SFX_ITEM_DEFAULT == pSet->GetItemState( nWhich ) )
                {
                    rSet.ClearItem( nWhich );
                    rSet.DisableItem( nWhich );
                }
                nWhich = aNewIter.NextWhich();
            }
        }
        delete pSet;
    }

//    const SdrMarkList& rMarkList = pDrView->GetMarkList();
//    ULONG nMarkCount = rMarkList.GetMarkCount();
//    BOOL bDisabled = FALSE;
//
//    for (ULONG i = 0;
//         i < nMarkCount && !bDisabled && i < 50; i++)
//    {
//        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
//
//        if (pObj->GetObjInventor() == E3dInventor)
//        {
//            bDisabled = TRUE;
//            rSet.ClearItem(SDRATTR_SHADOW);
//            rSet.DisableItem(SDRATTR_SHADOW);
//        }
//    }
}


/*************************************************************************
|*
|* Text der Selektion zurueckgeben
|*
\************************************************************************/

String __EXPORT SdDrawViewShell::GetSelectionText(BOOL bCompleteWords)
{
    String aStrSelection;
    Outliner* pOl = pDrView->GetTextEditOutliner();
    OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();

    if (pOl && pOlView)
    {
        if (bCompleteWords)
        {
            ESelection aSel = pOlView->GetSelection();
            String aStrCurrentDelimiters = pOl->GetWordDelimiters();

            pOl->SetWordDelimiters( String( RTL_CONSTASCII_USTRINGPARAM( " .,;\"'" )));
            aStrSelection = pOl->GetWord( aSel.nEndPara, aSel.nEndPos );
            pOl->SetWordDelimiters( aStrCurrentDelimiters );
        }
        else
        {
            aStrSelection = pOlView->GetSelected();
        }
    }

    return (aStrSelection);
}

/*************************************************************************
|*
|* Ist etwas selektiert?
|*
\************************************************************************/

BOOL SdDrawViewShell::HasSelection(BOOL bText) const
{
    BOOL bReturn = FALSE;

    if (bText)
    {
        OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();

        if (pOlView && pOlView->GetSelected().Len() != 0)
        {
            bReturn = TRUE;
        }
    }
    else if (pDrView->GetMarkList().GetMarkCount() != 0)
    {
        bReturn = TRUE;
    }

    return bReturn;
}


