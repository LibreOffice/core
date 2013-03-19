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


#include "DrawViewShell.hxx"
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/string.hxx>
#include <svx/svxids.hrc>
#include <svx/globl3d.hxx>
#include <svx/hlnkitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/whiter.hxx>
#include <svl/eitem.hxx>
#include <svl/itempool.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/templdlg.hxx>
#include <svx/xdef.hxx>
#include <svx/svddef.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/fmshell.hxx>
#include <svl/cjkoptions.hxx>

#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "Outliner.hxx"
#include "app.hrc"

#include "app.hxx"
#include "stlsheet.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include "ViewShellBase.hxx"
#include "FormShellManager.hxx"
#include "cfgids.hxx"
#include "anminfo.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;

namespace sd {

/**
 * Set state of controller SfxSlots
 */
void DrawViewShell::GetCtrlState(SfxItemSet &rSet)
{
    if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
    {
        // let "last version" of SFx en/disable
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
                aHLinkItem.SetName(comphelper::string::stripEnd(sReturn, ' '));
            }
        }
        else
        {
            if (mpDrawView->GetMarkedObjectList().GetMarkCount() > 0)
            {
                bool bFound = false;

                SdrObject* pMarkedObj = mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                if( pMarkedObj && (FmFormInventor == pMarkedObj->GetObjInventor()) )
                {
                    SdrUnoObj* pUnoCtrl = dynamic_cast< SdrUnoObj* >( pMarkedObj );

                    if(pUnoCtrl) try
                    {
                        uno::Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), uno::UNO_QUERY_THROW );
                        uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );
                        uno::Reference< beans::XPropertySetInfo > xPropInfo( xPropSet->getPropertySetInfo(), uno::UNO_QUERY_THROW );

                        form::FormButtonType eButtonType = form::FormButtonType_URL;
                        const OUString sButtonType( "ButtonType" );
                        if(xPropInfo->hasPropertyByName( sButtonType ) && (xPropSet->getPropertyValue( sButtonType ) >>= eButtonType ) )
                        {
                            OUString aString;

                            // Label
                            const OUString sLabel( "Label" );
                            if(xPropInfo->hasPropertyByName(sLabel))
                            {
                                if( xPropSet->getPropertyValue(sLabel) >>= aString )
                                    aHLinkItem.SetName(String( aString ));
                            }

                            // URL
                            const OUString sTargetURL( "TargetURL" );
                            if(xPropInfo->hasPropertyByName(sTargetURL))
                            {
                                if( xPropSet->getPropertyValue(sTargetURL) >>= aString )
                                    aHLinkItem.SetURL(String( aString ));
                            }

                            // Target
                            const OUString sTargetFrame( "TargetFrame" );
                            if(xPropInfo->hasPropertyByName(sTargetFrame) )
                            {
                                if( xPropSet->getPropertyValue(sTargetFrame) >>= aString )
                                    aHLinkItem.SetTargetFrame(String( aString ));
                            }

                            aHLinkItem.SetInsertMode(HLINK_BUTTON);
                            bFound = true;
                        }
                    }
                    catch( uno::Exception& )
                    {
                    }
                }

                // try interaction link
                if( !bFound && pMarkedObj )
                {
                    SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pMarkedObj);
                    if( pInfo && (pInfo->meClickAction == presentation::ClickAction_DOCUMENT) )
                        aHLinkItem.SetURL( pInfo->GetBookmark());
                    aHLinkItem.SetInsertMode(HLINK_BUTTON);
                }
            }
        }

        rSet.Put(aHLinkItem);
    }
    rSet.Put( SfxBoolItem( SID_READONLY_MODE, mbReadOnly ) );

    // output quality
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_COLOR ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_GRAYSCALE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_BLACKWHITE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTPUT_QUALITY_CONTRAST ) )
    {
        const sal_uLong nMode = (sal_Int32)GetActiveWindow()->GetDrawMode();
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_COLOR, (sal_Bool)((sal_uLong)OUTPUT_DRAWMODE_COLOR == nMode) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_GRAYSCALE, (sal_Bool)((sal_uLong)OUTPUT_DRAWMODE_GRAYSCALE == nMode) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_BLACKWHITE, (sal_Bool)((sal_uLong)OUTPUT_DRAWMODE_BLACKWHITE == nMode) ) );
        rSet.Put( SfxBoolItem( SID_OUTPUT_QUALITY_CONTRAST, (sal_Bool)((sal_uLong)OUTPUT_DRAWMODE_CONTRAST == nMode) ) );
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) )
    {
        rSet.Put( SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, sal_True ) );
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_ATTR_YEAR2000) )
    {
        FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
        if (pFormShell != NULL)
        {
            sal_uInt16 nState = 0;
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

        rSet.DisableItem( SID_TRANSLITERATE_SENTENCE_CASE );
        rSet.DisableItem( SID_TRANSLITERATE_TITLE_CASE );
        rSet.DisableItem( SID_TRANSLITERATE_TOGGLE_CASE );
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



void DrawViewShell::GetAttrState( SfxItemSet& rSet )
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    sal_Bool    bAttr = sal_False;
    SfxAllItemSet aAllSet( *rSet.GetPool() );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
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
                bAttr = sal_True;
            }
            break;

            case SID_HYPHENATION:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );
                if( aAttrs.GetItemState( EE_PARA_HYPHENATE ) >= SFX_ITEM_AVAILABLE )
                {
                    sal_Bool bValue = ( (const SfxBoolItem&) aAttrs.Get( EE_PARA_HYPHENATE ) ).GetValue();
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
                    rSet.Put(SfxBoolItem(nWhich,sal_False));
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
                // It is not possible to create PseudoStyleSheets 'by Example';
                // normal style sheets need a selected object for that
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
                // if there is no (yet) a designer, we have to go back into the
                // view state; an actual set family can not be considered
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
        rSet.Put( *pSet, sal_False );
    }

    rSet.Put( aAllSet, sal_False );

    // there were changes at area and/or line attributes
    if( bAttr && pSet )
    {
        // if the view owns selected objects, corresponding items have to be
        // changed from SFX_ITEM_DEFAULT (_ON) to SFX_ITEM_DISABLED
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

}



String DrawViewShell::GetSelectionText(sal_Bool bCompleteWords)
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

            pOl->SetWordDelimiters( OUString(" .,;\"'" ));
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


sal_Bool DrawViewShell::HasSelection(sal_Bool bText) const
{
    sal_Bool bReturn = sal_False;

    if (bText)
    {
        OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();

        if (pOlView && pOlView->GetSelected().Len() != 0)
        {
            bReturn = sal_True;
        }
    }
    else if (mpDrawView->GetMarkedObjectList().GetMarkCount() != 0)
    {
        bReturn = sal_True;
    }

    return bReturn;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
