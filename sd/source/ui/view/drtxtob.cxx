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


#include "TextObjectBar.hxx"

#include <svx/svxids.hrc>

#include <i18nlangtag/mslangid.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/kernitem.hxx>
#include <vcl/vclenum.hxx>
#include <sfx2/app.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/tplpitem.hxx>
#include <editeng/escapementitem.hxx>
#include <svx/svdoutl.hxx>
#include <svl/intitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/fhgtitem.hxx>


#include <sfx2/objface.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include "futempl.hxx"
#include "sdresid.hxx"
#include "Window.hxx"
#include "OutlineView.hxx"


using namespace sd;
using namespace ::com::sun::star;

#define TextObjectBar
#include "sdslots.hxx"

namespace sd {

/**
 * Declare default interface (Slotmap must not be empty, therefore enter
 * something that (hopefully) never occurs.
 */

SFX_IMPL_INTERFACE( TextObjectBar, SfxShell, SdResId(STR_TEXTOBJECTBARSHELL) )
{
}

TYPEINIT1( TextObjectBar, SfxShell );


TextObjectBar::TextObjectBar (
    ViewShell* pSdViewSh,
    SfxItemPool& rItemPool,
    ::sd::View* pSdView )
    : SfxShell(pSdViewSh->GetViewShell()),
      mpViewShell( pSdViewSh ),
      mpView( pSdView )
{
    SetPool(&rItemPool);

    if( mpView )
    {
        OutlineView* pOutlinerView = dynamic_cast< OutlineView* >( mpView );
        if( pOutlinerView )
        {
            SetUndoManager(&pOutlinerView->GetOutliner()->GetUndoManager());
        }
        else
        {
            DrawDocShell* pDocShell = mpView->GetDoc().GetDocSh();
            if( pDocShell )
            {
                SetUndoManager(pDocShell->GetUndoManager());
                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( pSdViewSh );
                if ( pDrawViewShell )
                    SetRepeatTarget(pSdView);
            }
        }
    }

    SetName( OUString( "TextObjectBar" ));

    
}


TextObjectBar::~TextObjectBar()
{
    SetRepeatTarget(NULL);
}

void TextObjectBar::GetCharState( SfxItemSet& rSet )
{
    SfxItemSet  aCharAttrSet( mpView->GetDoc().GetPool() );
    mpView->GetAttributes( aCharAttrSet );

    SfxItemSet aNewAttr( mpViewShell->GetPool(),EE_ITEMS_START,EE_ITEMS_END);

    aNewAttr.Put(aCharAttrSet, false);
    rSet.Put(aNewAttr, false);

    SvxKerningItem aKern = ( (const SvxKerningItem&) aCharAttrSet.Get( EE_CHAR_KERNING ) );
    
    rSet.Put(aKern);

    SfxItemState eState = aCharAttrSet.GetItemState( EE_CHAR_KERNING, true );
    if ( eState == SFX_ITEM_DONTCARE )
    {
        rSet.InvalidateItem(EE_CHAR_KERNING);
    }
}

/**
 * Status of attribute items.
 */
void TextObjectBar::GetAttrState( SfxItemSet& rSet )
{
    SfxWhichIter        aIter( rSet );
    sal_uInt16              nWhich = aIter.FirstWhich();
    SfxItemSet          aAttrSet( mpView->GetDoc().GetPool() );
    SvtLanguageOptions  aLangOpt;
    sal_Bool            bDisableParagraphTextDirection = !aLangOpt.IsCTLFontEnabled();
    sal_Bool            bDisableVerticalText = !aLangOpt.IsVerticalTextEnabled();

    mpView->GetAttributes( aAttrSet );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;

        switch ( nSlotId )
        {
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_POSTURE:
        case SID_ATTR_CHAR_SHADOWED:
            case SID_ATTR_CHAR_STRIKEOUT:
            {
                sal_uInt16 stretchX = 100;
                sal_uInt16 stretchY = 100;
                SvxScriptSetItem aSetItem( nSlotId, GetPool() );
                aSetItem.GetItemSet().Put( aAttrSet, false );

                sal_uInt16 nScriptType = mpView->GetScriptType();

                if( (nSlotId == SID_ATTR_CHAR_FONT) || (nSlotId == SID_ATTR_CHAR_FONTHEIGHT) )
                {
                    
                    
                    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
                    SdrOutliner *pOutliner = mpView->GetTextEditOutliner();

                    if (mpView->ISA(OutlineView))
                    {
                        pOLV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
                            mpViewShell->GetActiveWindow());
                    }

                    if( pOutliner )
                        pOutliner->GetGlobalCharStretching( stretchX, stretchY );

                    if(pOLV && !pOLV->GetSelection().HasRange())
                    {
                        if( mpViewShell && mpViewShell->GetViewShell() && mpViewShell->GetViewShell()->GetWindow() )
                        {
                            LanguageType nInputLang = mpViewShell->GetViewShell()->GetWindow()->GetInputLanguage();
                            if(nInputLang != LANGUAGE_DONTKNOW && nInputLang != LANGUAGE_SYSTEM)
                                nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nInputLang );
                        }
                    }
                }

                const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                if( pI )
                {
                    if( nSlotId == SID_ATTR_CHAR_FONTHEIGHT )
                    {
                        SvxFontHeightItem aFontItem = *(dynamic_cast<const SvxFontHeightItem *>(pI));
                        aFontItem.SetHeight(aFontItem.GetHeight(), stretchX, aFontItem.GetPropUnit());
                        aAttrSet.Put( aFontItem, nWhich );
                    }
                    else
                    {
                        aAttrSet.Put( *pI, nWhich );
                    }
                }
                else
                {
                    aAttrSet.InvalidateItem( nWhich );
                }
            }
            break;


            case SID_STYLE_APPLY:
            case SID_STYLE_FAMILY2:
            {
                SfxStyleSheet* pStyleSheet = mpView->GetStyleSheetFromMarked();
                if( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nWhich, pStyleSheet->GetName() ) );
                else
                {
                    rSet.Put( SfxTemplateItem( nWhich, OUString() ) );
                }
            }
            break;

            case SID_OUTLINE_LEFT:
            case SID_OUTLINE_RIGHT:
            case SID_OUTLINE_UP:
            case SID_OUTLINE_DOWN:
            {
                sal_Bool bDisableLeft     = sal_True;
                sal_Bool bDisableRight    = sal_True;
                sal_Bool bDisableUp       = sal_True;
                sal_Bool bDisableDown     = sal_True;
                OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                if (mpView->ISA(OutlineView))
                {
                    pOLV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
                        mpViewShell->GetActiveWindow());
                }

                sal_Bool bOutlineViewSh = mpViewShell->ISA(OutlineViewShell);

                if (pOLV &&
                    ( pOLV->GetOutliner()->GetMode() == OUTLINERMODE_OUTLINEOBJECT || bOutlineViewSh ) )
                {
                    
                    ::Outliner* pOutl = pOLV->GetOutliner();

                    std::vector<Paragraph*> aSelList;
                    pOLV->CreateSelectionList(aSelList);
                    Paragraph* pPara = aSelList.empty() ? NULL : *(aSelList.begin());

                    
                    sal_Bool bIsOutlineView(OUTLINERMODE_OUTLINEVIEW == pOLV->GetOutliner()->GetMode());

                    
                    if(bIsOutlineView)
                    {
                        
                        
                        if(pOutl->GetAbsPos(pPara) > 1 || ( pOutl->HasParaFlag(pPara,PARAFLAG_ISPAGE) && pOutl->GetAbsPos(pPara) > 0 ) )
                        {
                            
                            bDisableUp = sal_False;
                        }
                    }
                    else
                    {
                        
                        if(pOutl->GetAbsPos(pPara) > 0)
                        {
                            
                            bDisableUp = sal_False;
                        }
                    }

                    for (std::vector<Paragraph*>::const_iterator iter = aSelList.begin(); iter != aSelList.end(); ++iter)
                    {
                        pPara = *iter;

                        sal_Int16 nDepth = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );

                        if (nDepth > 0 || (bOutlineViewSh && (nDepth <= 0) && !pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE )) )
                        {
                            
                            bDisableLeft = sal_False;
                        }

                        if( (nDepth < pOLV->GetOutliner()->GetMaxDepth() && ( !bOutlineViewSh || pOutl->GetAbsPos(pPara) != 0 )) ||
                            (bOutlineViewSh && (nDepth <= 0) && pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE ) && pOutl->GetAbsPos(pPara) != 0) )
                        {
                            
                            bDisableRight = sal_False;
                        }
                    }

                    if ( ( pOutl->GetAbsPos(pPara) < pOutl->GetParagraphCount() - 1 ) &&
                         ( pOutl->GetParagraphCount() > 1 || !bOutlineViewSh) )
                    {
                        
                        bDisableDown = sal_False;
                    }

                    
                    pPara = aSelList.empty() ? NULL : *(aSelList.begin());

                    if(!bDisableDown && bIsOutlineView
                        && pPara
                        && 0 == pOutl->GetAbsPos(pPara)
                        && pOutl->GetParagraphCount() > 1
                        && !pOutl->HasParaFlag( pOutl->GetParagraph(1), PARAFLAG_ISPAGE ) )
                    {
                        
                        bDisableDown = sal_True;
                    }
                }

                if (bDisableLeft)
                    rSet.DisableItem(SID_OUTLINE_LEFT);
                if (bDisableRight)
                    rSet.DisableItem(SID_OUTLINE_RIGHT);
                if (bDisableUp)
                    rSet.DisableItem(SID_OUTLINE_UP);
                if (bDisableDown)
                    rSet.DisableItem(SID_OUTLINE_DOWN);
            }
            break;

            case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
            case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            {
                if ( bDisableVerticalText )
                {
                    rSet.DisableItem( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
                    rSet.DisableItem( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
                }
                else
                {
                    sal_Bool bLeftToRight = sal_True;

                    SdrOutliner* pOutl = mpView->GetTextEditOutliner();
                    if( pOutl )
                    {
                        if( pOutl->IsVertical() )
                            bLeftToRight = sal_False;
                    }
                    else
                        bLeftToRight = ( (const SvxWritingModeItem&) aAttrSet.Get( SDRATTR_TEXTDIRECTION ) ).GetValue() == com::sun::star::text::WritingMode_LR_TB;

                    rSet.Put( SfxBoolItem( SID_TEXTDIRECTION_LEFT_TO_RIGHT, bLeftToRight ) );
                    rSet.Put( SfxBoolItem( SID_TEXTDIRECTION_TOP_TO_BOTTOM, !bLeftToRight ) );

                    if( !bLeftToRight )
                        bDisableParagraphTextDirection = sal_True;
                }
            }
            break;

            case SID_GROW_FONT_SIZE:
            case SID_SHRINK_FONT_SIZE:
            {
                
            }
            break;

            case SID_THES:
            {
                if( mpView && mpView->GetTextEditOutlinerView() )
                {
                    EditView & rEditView = mpView->GetTextEditOutlinerView()->GetEditView();
                    OUString        aStatusVal;
                    LanguageType    nLang = LANGUAGE_NONE;
                    bool bIsLookUpWord = GetStatusValueForThesaurusFromContext( aStatusVal, nLang, rEditView );
                    rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

                    
                    uno::Reference< linguistic2::XThesaurus > xThes( LinguMgr::GetThesaurus() );
                    if (!bIsLookUpWord ||
                        !xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( LanguageTag( nLang). getLocale() ))
                        rSet.DisableItem( SID_THES );
                }
                else
                {
                    rSet.DisableItem( SID_THES );
                }
                
                nSlotId = 0;
            }
            break;

            default:
            break;
        }

        nWhich = aIter.NextWhich();
    }

    rSet.Put( aAttrSet, false ); 


    
    if (!mpViewShell->ISA(DrawViewShell))
    {
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_LEFT );
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_RIGHT );
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_CENTER );
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_BLOCK );
        rSet.DisableItem( SID_ATTR_PARA_LINESPACE_10 );
        rSet.DisableItem( SID_ATTR_PARA_LINESPACE_15 );
        rSet.DisableItem( SID_ATTR_PARA_LINESPACE_20 );
        rSet.DisableItem( SID_PARASPACE_INCREASE );
        rSet.DisableItem( SID_PARASPACE_DECREASE );
        rSet.DisableItem( SID_TEXTDIRECTION_TOP_TO_BOTTOM );
        rSet.DisableItem( SID_TEXTDIRECTION_LEFT_TO_RIGHT );
        rSet.DisableItem( SID_ATTR_PARA_LEFT_TO_RIGHT );
        rSet.DisableItem( SID_ATTR_PARA_RIGHT_TO_LEFT );
    }
    else
    {
        
        OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
        if( pOLV )
        {
            ESelection aSel = pOLV->GetSelection();
            aSel.Adjust();
            sal_Int32 nStartPara = aSel.nStartPara;
            sal_Int32 nEndPara = aSel.nEndPara;
            if( !aSel.HasRange() )
            {
                nStartPara = 0;
                nEndPara = pOLV->GetOutliner()->GetParagraphCount() - 1;
            }
            long nUpper = 0L;
            for( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
            {
                const SfxItemSet& rItems = pOLV->GetOutliner()->GetParaAttribs( nPara );
                const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) rItems.Get( EE_PARA_ULSPACE );
                nUpper = std::max( nUpper, (long)rItem.GetUpper() );
            }
            if( nUpper == 0L )
                rSet.DisableItem( SID_PARASPACE_DECREASE );
        }
        else
        {
            
            
            
        }

        
        SvxLRSpaceItem aLR = ( (const SvxLRSpaceItem&) aAttrSet.Get( EE_PARA_LRSPACE ) );
        rSet.Put(aLR);
        SvxAdjust eAdj = ( (const SvxAdjustItem&) aAttrSet.Get( EE_PARA_JUST ) ).GetAdjust();
        switch( eAdj )
        {
            case SVX_ADJUST_LEFT:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_LEFT, true ) );
            break;
            case SVX_ADJUST_CENTER:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_CENTER, true ) );
            break;
            case SVX_ADJUST_RIGHT:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, true ) );
            break;
            case SVX_ADJUST_BLOCK:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, true ) );
            break;
            default:
            break;
        }

        Invalidate(SID_ATTR_PARA_ADJUST_LEFT);
        Invalidate(SID_ATTR_PARA_ADJUST_CENTER);
        Invalidate(SID_ATTR_PARA_ADJUST_RIGHT);
        Invalidate(SID_ATTR_PARA_ADJUST_BLOCK);
        Invalidate(SID_ATTR_PARA_LINESPACE);
        Invalidate(SID_ATTR_PARA_ULSPACE);

        
        if( bDisableParagraphTextDirection )
        {
            rSet.DisableItem( SID_ATTR_PARA_LEFT_TO_RIGHT );
            rSet.DisableItem( SID_ATTR_PARA_RIGHT_TO_LEFT );
        }
        else
        {
            switch( ( ( (SvxFrameDirectionItem&) aAttrSet.Get( EE_PARA_WRITINGDIR ) ) ).GetValue() )
            {
                case FRMDIR_VERT_TOP_LEFT:
                case FRMDIR_VERT_TOP_RIGHT:
                {
                    rSet.DisableItem( SID_ATTR_PARA_LEFT_TO_RIGHT );
                    rSet.DisableItem( SID_ATTR_PARA_RIGHT_TO_LEFT );
                }
                break;

                case FRMDIR_HORI_LEFT_TOP:
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_LEFT_TO_RIGHT, true ) );
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_RIGHT_TO_LEFT, false ) );
                break;

                case FRMDIR_HORI_RIGHT_TOP:
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_LEFT_TO_RIGHT, false ) );
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_RIGHT_TO_LEFT, true ) );
                break;

                
                case FRMDIR_ENVIRONMENT:
                {
                    SdDrawDocument& rDoc = mpView->GetDoc();
                    ::com::sun::star::text::WritingMode eMode = rDoc.GetDefaultWritingMode();
                    sal_Bool bIsLeftToRight(sal_False);

                    if(::com::sun::star::text::WritingMode_LR_TB == eMode
                        || ::com::sun::star::text::WritingMode_TB_RL == eMode)
                    {
                        bIsLeftToRight = sal_True;
                    }

                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_LEFT_TO_RIGHT, bIsLeftToRight ) );
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_RIGHT_TO_LEFT, !bIsLeftToRight ) );
                }
                break;
            }
        }

        SvxLRSpaceItem aLRSpace = ( (const SvxLRSpaceItem&) aAttrSet.Get( EE_PARA_LRSPACE ) );
        aLRSpace.SetWhich(SID_ATTR_PARA_LRSPACE);
        rSet.Put(aLRSpace);
        Invalidate(SID_ATTR_PARA_LRSPACE);
        
        SfxItemState eState = aAttrSet.GetItemState( EE_PARA_LRSPACE );
        if ( eState == SFX_ITEM_DONTCARE )
        {
            rSet.InvalidateItem(EE_PARA_LRSPACE);
            rSet.InvalidateItem(SID_ATTR_PARA_LRSPACE);
        }
        sal_uInt16 nLineSpace = (sal_uInt16) ( (const SvxLineSpacingItem&) aAttrSet.
                            Get( EE_PARA_SBL ) ).GetPropLineSpace();
        switch( nLineSpace )
        {
            case 100:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_10, true ) );
            break;
            case 150:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_15, true ) );
            break;
            case 200:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_20, true ) );
            break;
        }
    }

    
    SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                    aAttrSet.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

    if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
        rSet.Put( SfxBoolItem( SID_SET_SUPER_SCRIPT, true ) );
    else if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
        rSet.Put( SfxBoolItem( SID_SET_SUB_SCRIPT, true ) );
}

/**
 * Command event
 */

void TextObjectBar::Command( const CommandEvent& )
{
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
