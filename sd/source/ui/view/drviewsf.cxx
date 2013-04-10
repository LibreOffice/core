/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _GLOBL3D_HXX
#include <svx/globl3d.hxx>
#endif
#include <svx/hlnkitem.hxx>
#include <editeng/eeitem.hxx>
#ifndef _FLDITEM_HXX
#include <editeng/flditem.hxx>
#endif
#include <sfx2/viewfrm.hxx>
#include <svl/whiter.hxx>
#include <svl/eitem.hxx>
#include <svl/itempool.hxx>
#include <sfx2/tplpitem.hxx>
#ifndef _BINDING_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#include <sfx2/app.hxx>
#include <sfx2/templdlg.hxx>
#include <svx/xdef.hxx>
#include <svx/svddef.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <tools/urlobj.hxx>
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

#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/adjitem.hxx>
#include <svx/nbdtmgfact.hxx>
#include <svx/nbdtmg.hxx>

using namespace svx::sidebar;
using ::rtl::OUString;
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
                        const OUString sButtonType( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" ) );
                        if(xPropInfo->hasPropertyByName( sButtonType ) && (xPropSet->getPropertyValue( sButtonType ) >>= eButtonType ) )
                        {
                            OUString aString;

                            // Label
                            const OUString sLabel( RTL_CONSTASCII_USTRINGPARAM( "Label" ) );
                            if(xPropInfo->hasPropertyByName(sLabel))
                            {
                                if( xPropSet->getPropertyValue(sLabel) >>= aString )
                                    aHLinkItem.SetName(String( aString ));
                            }

                            // URL
                            const OUString sTargetURL(RTL_CONSTASCII_USTRINGPARAM( "TargetURL" ));
                            if(xPropInfo->hasPropertyByName(sTargetURL))
                            {
                                if( xPropSet->getPropertyValue(sTargetURL) >>= aString )
                                    aHLinkItem.SetURL(String( aString ));
                            }

                            // Target
                            const OUString sTargetFrame( RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" ) );
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

    // Ausgabequalitaet
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


/*************************************************************************
|*
|* Status der Attribut-Items
|*
\************************************************************************/

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
            case SID_ATTR_PARA_ADJUST_LEFT:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );

                SvxAdjustItem aItem= ( (const SvxAdjustItem&) aAttrs.Get( EE_PARA_JUST ) );
                SvxAdjust eAdj = aItem.GetAdjust();
                if ( eAdj == SVX_ADJUST_LEFT)
                {
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_LEFT, sal_True ) );
                }

                bAttr = sal_True;

                Invalidate(nSlotId);
            }
            break;
            case SID_ATTR_PARA_ADJUST_CENTER:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );

                SvxAdjustItem aItem= ( (const SvxAdjustItem&) aAttrs.Get( EE_PARA_JUST ) );
                SvxAdjust eAdj = aItem.GetAdjust();
                if ( eAdj == SVX_ADJUST_CENTER)
                {
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_CENTER, sal_True ) );
                }

                bAttr = sal_True;

                Invalidate(nSlotId);
            }
            break;
            case SID_ATTR_PARA_ADJUST_RIGHT:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );

                SvxAdjustItem aItem= ( (const SvxAdjustItem&) aAttrs.Get( EE_PARA_JUST ) );
                SvxAdjust eAdj = aItem.GetAdjust();
                if ( eAdj == SVX_ADJUST_RIGHT)
                {
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, sal_True ) );
                }

                bAttr = sal_True;

                Invalidate(nSlotId);
            }
            break;
            case SID_ATTR_PARA_ADJUST_BLOCK:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );

                SvxAdjustItem aItem= ( (const SvxAdjustItem&) aAttrs.Get( EE_PARA_JUST ) );
                SvxAdjust eAdj = aItem.GetAdjust();
                if ( eAdj == SVX_ADJUST_BLOCK)
                {
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, sal_True ) );
                }

                bAttr = sal_True;

                Invalidate(nSlotId);
            }
            break;
            case SID_ATTR_PARA_LRSPACE:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );
                SvxLRSpaceItem aLRSpace = ( (const SvxLRSpaceItem&) aAttrs.Get( EE_PARA_LRSPACE ) );
                aLRSpace.SetWhich(SID_ATTR_PARA_LRSPACE);
                rSet.Put(aLRSpace);
                bAttr = sal_True;
                Invalidate(SID_ATTR_PARA_LRSPACE);
            }
            break;
            case SID_ATTR_PARA_LINESPACE:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );
                SvxLineSpacingItem aLineLR = ( (const SvxLineSpacingItem&) aAttrs.Get( EE_PARA_SBL ) );
                rSet.Put(aLineLR);
                bAttr = sal_True;
                Invalidate(SID_ATTR_PARA_LINESPACE);
            }
            break;
            case SID_ATTR_PARA_ULSPACE:
            {
                SfxItemSet aAttrs( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aAttrs );
                SvxULSpaceItem aULSP = ( (const SvxULSpaceItem&) aAttrs.Get( EE_PARA_ULSPACE ) );
                aULSP.SetWhich(SID_ATTR_PARA_ULSPACE);
                rSet.Put(aULSP);
                bAttr = sal_True;
                Invalidate(SID_ATTR_PARA_ULSPACE);
            }
            break;
            case SID_ATTR_FILL_STYLE:
            case SID_ATTR_FILL_COLOR:
            case SID_ATTR_FILL_GRADIENT:
            case SID_ATTR_FILL_HATCH:
            case SID_ATTR_FILL_BITMAP:
            case SID_ATTR_FILL_SHADOW:
            case SID_ATTR_FILL_TRANSPARENCE:
            case SID_ATTR_FILL_FLOATTRANSPARENCE:
            case SID_ATTR_LINE_STYLE:
            case SID_ATTR_LINE_DASH:
            case SID_ATTR_LINE_WIDTH:
            case SID_ATTR_LINE_COLOR:
            case SID_ATTR_LINE_TRANSPARENCE:
            case SID_ATTR_LINE_JOINT:
            case SID_ATTR_LINE_CAP:
            case SID_ATTR_TEXT_FITTOSIZE:
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_SHADOWED:
            case SID_ATTR_CHAR_POSTURE:
            case SID_ATTR_CHAR_UNDERLINE:
            case SID_ATTR_CHAR_STRIKEOUT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_KERNING:
            case SID_SET_SUB_SCRIPT:
            case SID_SET_SUPER_SCRIPT:
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
            case FN_BUL_NUM_RULE_INDEX:
            case FN_NUM_NUM_RULE_INDEX:
            {
                SfxItemSet aEditAttr( GetDoc()->GetPool() );
                mpDrawView->GetAttributes( aEditAttr );

                SfxItemSet aNewAttr( GetPool(), EE_ITEMS_START, EE_ITEMS_END );
                aNewAttr.Put( aEditAttr, sal_False );


                sal_uInt16 nActNumLvl = (sal_uInt16)0xFFFF;
                SvxNumRule* pNumRule = NULL;
                const SfxPoolItem* pTmpItem=NULL;
                sal_uInt16 nNumItemId = SID_ATTR_NUMBERING_RULE;

                //if(SFX_ITEM_SET == aNewAttr.GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pTmpItem))
                //  nActNumLvl = ((const SfxUInt16Item*)pTmpItem)->GetValue();
                rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,DEFAULT_NONE));
                rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,DEFAULT_NONE));
                nActNumLvl = mpDrawView->GetSelectionLevel();
                pTmpItem=GetNumBulletItem(aNewAttr, nNumItemId);

                if (pTmpItem)
                    pNumRule = new SvxNumRule(*((SvxNumBulletItem*)pTmpItem)->GetNumRule());

                if ( pNumRule )
                {
                    sal_uInt16 nMask = 1;
                    sal_uInt16 nCount = 0;
                    sal_uInt16 nCurLevel = (sal_uInt16)0xFFFF;
                    for(sal_uInt16 i = 0; i < pNumRule->GetLevelCount(); i++)
                    {
                        if(nActNumLvl & nMask)
                        {
                            nCount++;
                            nCurLevel = i;
                        }
                        nMask <<= 1;
                    }
                    if ( nCount == 1 )
                    {
                        sal_Bool bBullets = sal_False;
                        const SvxNumberFormat* pNumFmt = pNumRule->Get(nCurLevel);
                        if ( pNumFmt )
                        {
                            switch(pNumFmt->GetNumberingType())
                            {
                                case SVX_NUM_CHAR_SPECIAL:
                                case SVX_NUM_BITMAP:
                                    bBullets = sal_True;
                                    break;

                                default:
                                    bBullets = sal_False;
                            }

                            rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,(sal_uInt16)0xFFFF));
                            rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,(sal_uInt16)0xFFFF));
                            if ( bBullets )
                            {
                                NBOTypeMgrBase* pBullets = NBOutlineTypeMgrFact::CreateInstance(eNBOType::MIXBULLETS);
                                if ( pBullets )
                                {
                                    sal_uInt16 nBulIndex = pBullets->GetNBOIndexForNumRule(*pNumRule,nActNumLvl);
                                     rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,nBulIndex));
                                }
                            }else
                            {
                                NBOTypeMgrBase* pNumbering = NBOutlineTypeMgrFact::CreateInstance(eNBOType::NUMBERING);
                                if ( pNumbering )
                                {
                                    sal_uInt16 nBulIndex = pNumbering->GetNBOIndexForNumRule(*pNumRule,nActNumLvl);
                                     rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,nBulIndex));
                                }
                            }
                        }
                    }
                }
            }
                    break;
            //End
            // Added by Li Hui for story 179.
            case FN_NUM_BULLET_ON:
            case FN_NUM_NUMBERING_ON:
            {
                sal_Bool bEnable = sal_False;
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                const sal_uInt32 nMarkCount = rMarkList.GetMarkCount();
                for (sal_uInt32 nIndex = 0; nIndex < nMarkCount; nIndex++)
                {
                    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(rMarkList.GetMark(nIndex)->GetMarkedSdrObj());
                    if (pTextObj && pTextObj->GetObjInventor() == SdrInventor)
                    {
                        if (pTextObj->GetObjIdentifier() != OBJ_OLE2)
                        {
                            bEnable = sal_True;
                            break;
                        }
                    }
                }
                if (bEnable)
                {
                    rSet.Put(SfxBoolItem(FN_NUM_BULLET_ON, sal_False));
                    rSet.Put(SfxBoolItem(FN_NUM_NUMBERING_ON, sal_False));
                }
                else
                {
                    rSet.DisableItem(FN_NUM_BULLET_ON);
                    rSet.DisableItem(FN_NUM_NUMBERING_ON);
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

        SfxItemState eState = pSet->GetItemState( EE_PARA_LRSPACE );
        if ( eState == SFX_ITEM_DONTCARE )
        {
            rSet.InvalidateItem(EE_PARA_LRSPACE);
            rSet.InvalidateItem(SID_ATTR_PARA_LRSPACE);
        }
        eState = pSet->GetItemState( EE_PARA_SBL );
        if ( eState == SFX_ITEM_DONTCARE )
        {
            rSet.InvalidateItem(EE_PARA_SBL);
            rSet.InvalidateItem(SID_ATTR_PARA_LINESPACE);
        }
        eState = pSet->GetItemState( EE_PARA_ULSPACE );
        if ( eState == SFX_ITEM_DONTCARE )
        {
            rSet.InvalidateItem(EE_PARA_ULSPACE);
            rSet.InvalidateItem(SID_ATTR_PARA_ULSPACE);
        }

        SvxEscapement eEsc = (SvxEscapement) ( (const SvxEscapementItem&)
                        pSet->Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();
        if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
        {
            rSet.Put( SfxBoolItem( SID_SET_SUPER_SCRIPT, sal_True ) );
        }
        else if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
        {
            rSet.Put( SfxBoolItem( SID_SET_SUB_SCRIPT, sal_True ) );
        }

        eState = pSet->GetItemState( EE_CHAR_KERNING, sal_True );
        if ( eState == SFX_ITEM_DONTCARE )
        {
            rSet.InvalidateItem(EE_CHAR_KERNING);
            rSet.InvalidateItem(SID_ATTR_CHAR_KERNING);
        }
        delete pSet;
    }
}


/*************************************************************************
|*
|* Text der Selektion zurueckgeben
|*
\************************************************************************/

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
