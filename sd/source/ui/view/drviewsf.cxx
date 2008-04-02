/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drviewsf.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:49:50 $
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

#include "DrawViewShell.hxx"

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
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif

#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "Outliner.hxx"
#include "app.hrc"

#include "app.hxx"
#include "stlsheet.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_FORM_SHELL_MANAGER_HXX
#include "FormShellManager.hxx"
#endif
#include "cfgids.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;


namespace sd {

/*************************************************************************
|*
|* Status von Controller-SfxSlots setzen
|*
\************************************************************************/

void DrawViewShell::GetCtrlState(SfxItemSet &rSet)
{
    if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
    {
        // "Letzte Version" vom SFx en/disablen lassen
        GetViewFrame()->GetSlotState (SID_RELOAD, NULL, &rSet);
    }

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_HYPERLINK_GETLINK))
    {
        SvxHyperlinkItem aHLinkItem;

        OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

        if (pOLV)
        {
            bool bField = false;
            const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();
            if (pFieldItem)
            {
                ESelection aSel = pOLV->GetSelection();
                if ( abs( aSel.nEndPos - aSel.nStartPos ) == 1 )
                {
                    const SvxFieldData* pField = pFieldItem->GetField();
                    if (pField->ISA(SvxURLField))
                    {
                        aHLinkItem.SetName(((const SvxURLField*) pField)->GetRepresentation());
                        aHLinkItem.SetURL(((const SvxURLField*) pField)->GetURL());
                        aHLinkItem.SetTargetFrame(((const SvxURLField*) pField)->GetTargetFrame());
                        bField = true;
                    }
                }
            }
            if (!bField)
            {
                // use selected text as name for urls
                String sReturn = pOLV->GetSelected();
                sReturn.Erase(255);
                sReturn.EraseTrailingChars();
                aHLinkItem.SetName(sReturn);
            }
        }
        else
        {
            if (mpDrawView->GetMarkedObjectList().GetMarkCount() > 0)
            {
                SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj());

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
    rSet.Put( SfxBoolItem( SID_READONLY_MODE, mbReadOnly ) );

    // Ausgabequalitaet
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_COLOR ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_GRAYSCALE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_BLACKWHITE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_CONTRAST ) )
    {
        const ULONG nMode = (sal_Int32)GetActiveWindow()->GetDrawMode();
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_COLOR, (BOOL)((ULONG)OUTPUT_DRAWMODE_COLOR == nMode) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_GRAYSCALE, (BOOL)((ULONG)OUTPUT_DRAWMODE_GRAYSCALE == nMode) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_BLACKWHITE, (BOOL)((ULONG)OUTPUT_DRAWMODE_BLACKWHITE == nMode) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_CONTRAST, (BOOL)((ULONG)OUTPUT_DRAWMODE_CONTRAST == nMode) ) );
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) )
    {
        rSet.Put( SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, TRUE ) );
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_ATTR_YEAR2000) )
    {
        FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager().GetFormShell();
        if (pFormShell != NULL)
        {
            UINT16 nState = 0;
            if (pFormShell->GetY2KState(nState))
                rSet.Put( SfxUInt16Item( SID_ATTR_YEAR2000, nState ) );
            else
                rSet.DisableItem( SID_ATTR_YEAR2000 );
        }
    }

    if ( !GetView()->GetTextEditOutliner() )
    {
        SvtCJKOptions aCJKOptions;
        if( !aCJKOptions.IsChangeCaseMapEnabled() )
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, sal_False );
        }
        else
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, sal_True );
        }

        rSet.DisableItem( SID_TRANSLITERATE_UPPER );
        rSet.DisableItem( SID_TRANSLITERATE_LOWER );
        rSet.DisableItem( SID_TRANSLITERATE_HALFWIDTH );
        rSet.DisableItem( SID_TRANSLITERATE_FULLWIDTH );
        rSet.DisableItem( SID_TRANSLITERATE_HIRAGANA );
        rSet.DisableItem( SID_TRANSLITERATE_KATAGANA );
    }
    else
    {
        SvtCJKOptions aCJKOptions;
        if( !aCJKOptions.IsChangeCaseMapEnabled() )
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, sal_False );
            rSet.DisableItem( SID_TRANSLITERATE_HALFWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_FULLWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_HIRAGANA );
            rSet.DisableItem( SID_TRANSLITERATE_KATAGANA );
        }
        else
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, sal_True );
        }
    }
}


/*************************************************************************
|*
|* Status der Attribut-Items
|*
\************************************************************************/

void DrawViewShell::GetAttrState( SfxItemSet& rSet )
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
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );
                if( aAttrs.GetItemState( EE_PARA_HYPHENATE ) >= SFX_ITEM_AVAILABLE )
                {
                    BOOL bValue = ( (const SfxBoolItem&) aAttrs.Get( EE_PARA_HYPHENATE ) ).GetValue();
                    rSet.Put( SfxBoolItem( SID_HYPHENATION, bValue ) );
                }
            }
            break;

            case SID_STYLE_FAMILY2:
            case SID_STYLE_FAMILY3:
            case SID_STYLE_FAMILY5:
            case SID_STYLE_APPLY: // StyleControl
            {
                SfxStyleSheet* pStyleSheet = mpDrawView->GetStyleSheet();
                if( pStyleSheet )
                {
                    if( nSlotId != SID_STYLE_APPLY && !mpDrawView->AreObjectsMarked() )
                    {
                        SfxTemplateItem aTmpItem( nWhich, String() );
                        aAllSet.Put( aTmpItem, aTmpItem.Which()  );
                    }
                    else
                    {
                        if (pStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE)
                            pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

                        if( pStyleSheet )
                        {
                            SfxStyleFamily eFamily = pStyleSheet->GetFamily();

                            if ((eFamily == SD_STYLE_FAMILY_GRAPHICS &&     nSlotId == SID_STYLE_FAMILY2)       ||
                                (eFamily == SD_STYLE_FAMILY_CELL     && nSlotId == SID_STYLE_FAMILY3)       ||
                                (eFamily == SD_STYLE_FAMILY_PSEUDO &&   nSlotId == SID_STYLE_FAMILY5))
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
                if( !mpDrawView->GetMarkedObjectList().GetMarkCount() ||
                    ( !mpDrawView->IsTextEdit() && !mpDrawView->GetStyleSheet() )
                  )
                    rSet.DisableItem( nWhich );
            }
            break;

            case SID_STYLE_WATERCAN:
            {
                ISfxTemplateCommon* pTemplateCommon = SFX_APP()->GetCurrentTemplateCommon(GetViewFrame()->GetBindings());
                if (pTemplateCommon && pTemplateCommon->GetActualFamily() == SD_STYLE_FAMILY_PSEUDO)
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
                if (pTemplateCommon && pTemplateCommon->GetActualFamily() == SD_STYLE_FAMILY_PSEUDO)
                    rSet.DisableItem(nWhich);
            }
            break;

            case SID_STYLE_DRAGHIERARCHIE:
            {
                ISfxTemplateCommon* pTemplateCommon = SFX_APP()->GetCurrentTemplateCommon(GetViewFrame()->GetBindings());
                if (pTemplateCommon && pTemplateCommon->GetActualFamily() == SD_STYLE_FAMILY_PSEUDO)
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
                    if (pTemplCommon->GetActualFamily() == SD_STYLE_FAMILY_PSEUDO)
                    {
                        rSet.DisableItem(nWhich);
                    }
                    else if (pTemplCommon->GetActualFamily() == SD_STYLE_FAMILY_GRAPHICS)
                    {
                        if (!mpDrawView->AreObjectsMarked())
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
                    if (!mpDrawView->AreObjectsMarked())
                    {
                        rSet.DisableItem(nWhich);
                    }
                }

            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                if (!mpDrawView->AreObjectsMarked())
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
        pSet = new SfxItemSet( GetDoc()->GetPool() );
        mpDrawView->GetAttributes( *pSet );
        rSet.Put( *pSet, FALSE );
    }

    rSet.Put( aAllSet, FALSE );

    // Flaechen und/oder Linienattribute wurden geaendert
    if( bAttr && pSet )
    {
        // Wenn die View selektierte Objekte besitzt, muessen entspr. Items
        // von SFX_ITEM_DEFAULT (_ON) auf SFX_ITEM_DISABLED geaendert werden
        if( mpDrawView->AreObjectsMarked() )
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

//    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
//    ULONG nMarkCount = rMarkList.GetMarkCount();
//    BOOL bDisabled = FALSE;
//
//    for (ULONG i = 0;
//         i < nMarkCount && !bDisabled && i < 50; i++)
//    {
//        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
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

String DrawViewShell::GetSelectionText(BOOL bCompleteWords)
{
    String aStrSelection;
    ::Outliner* pOl = mpDrawView->GetTextEditOutliner();
    OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();

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

BOOL DrawViewShell::HasSelection(BOOL bText) const
{
    BOOL bReturn = FALSE;

    if (bText)
    {
        OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();

        if (pOlView && pOlView->GetSelected().Len() != 0)
        {
            bReturn = TRUE;
        }
    }
    else if (mpDrawView->GetMarkedObjectList().GetMarkCount() != 0)
    {
        bReturn = TRUE;
    }

    return bReturn;
}

} // end of namespace sd
