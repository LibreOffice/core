/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TextObjectBar.hxx"

#include <svx/svxids.hrc>

#include <i18npool/mslangid.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <vcl/vclenum.hxx>
#include <sfx2/app.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/tplpitem.hxx>
#include <editeng/escpitem.hxx>
#include <svx/svdoutl.hxx>
#include <svl/intitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/frmdiritem.hxx>


#include <sfx2/objface.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "eetext.hxx"

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

/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein, also
|* tragen wir etwas ein, was hier (hoffentlich) nie vorkommt).
|*
\************************************************************************/


SFX_IMPL_INTERFACE( TextObjectBar, SfxShell, SdResId(STR_TEXTOBJECTBARSHELL) )
{
}

TYPEINIT1( TextObjectBar, SfxShell );

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

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
            SdDrawDocument* pDoc = mpView->GetDoc();
            if( pDoc )
            {
                DrawDocShell* pDocShell = pDoc->GetDocSh();
                if( pDocShell )
                {
                    SetUndoManager(pDocShell->GetUndoManager());
                    DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( pSdViewSh );
                    if ( pDrawViewShell )
                        SetRepeatTarget(pSdView);
                }
            }
        }
    }

    SetName( String( RTL_CONSTASCII_USTRINGPARAM( "TextObjectBar" )));

    // SetHelpId( SD_IF_SDDRAWTEXTOBJECTBAR );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

TextObjectBar::~TextObjectBar()
{
    SetRepeatTarget(NULL);
}

/*************************************************************************
|*
|* Status der Attribut-Items
|*
\************************************************************************/

void TextObjectBar::GetAttrState( SfxItemSet& rSet )
{
    SfxWhichIter        aIter( rSet );
    USHORT              nWhich = aIter.FirstWhich();
    SfxItemSet          aAttrSet( mpView->GetDoc()->GetPool() );
    SvtLanguageOptions  aLangOpt;
    sal_Bool            bDisableParagraphTextDirection = !aLangOpt.IsCTLFontEnabled();
    sal_Bool            bDisableVerticalText = !aLangOpt.IsVerticalTextEnabled();

    mpView->GetAttributes( aAttrSet );

    while ( nWhich )
    {
        USHORT nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;

        switch ( nSlotId )
        {
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_POSTURE:
            {
                SvxScriptSetItem aSetItem( nSlotId, GetPool() );
                aSetItem.GetItemSet().Put( aAttrSet, FALSE );

                USHORT nScriptType = mpView->GetScriptType();

                if( (nSlotId == SID_ATTR_CHAR_FONT) || (nSlotId == SID_ATTR_CHAR_FONTHEIGHT) )
                {
                    // input language should be preferred over
                    // current cursor position to detect script type
                    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                    if (mpView->ISA(OutlineView))
                    {
                        pOLV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
                            mpViewShell->GetActiveWindow());
                    }

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
                    aAttrSet.Put( *pI, nWhich );
                else
                    aAttrSet.InvalidateItem( nWhich );
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
                    rSet.Put( SfxTemplateItem( nWhich, String() ) );
                }
            }
            break;

            case SID_OUTLINE_LEFT:
            case SID_OUTLINE_RIGHT:
            case SID_OUTLINE_UP:
            case SID_OUTLINE_DOWN:
            {
                BOOL bDisableLeft     = TRUE;
                BOOL bDisableRight    = TRUE;
                BOOL bDisableUp       = TRUE;
                BOOL bDisableDown     = TRUE;
                OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                if (mpView->ISA(OutlineView))
                {
                    pOLV = static_cast<OutlineView*>(mpView)->GetViewByWindow(
                        mpViewShell->GetActiveWindow());
                }

                BOOL bOutlineViewSh = mpViewShell->ISA(OutlineViewShell);

                if (pOLV &&
                    ( pOLV->GetOutliner()->GetMode() == OUTLINERMODE_OUTLINEOBJECT || bOutlineViewSh ) )
                {
                    // Outliner im Gliederungsmodus
                    ::Outliner* pOutl = pOLV->GetOutliner();

                    std::vector<Paragraph*> aSelList;
                    pOLV->CreateSelectionList(aSelList);
                    Paragraph* pPara = aSelList.empty() ? NULL : *(aSelList.begin());

                    // find out if we are a OutlineView
                    BOOL bIsOutlineView(OUTLINERMODE_OUTLINEVIEW == pOLV->GetOutliner()->GetMode());

                    // This is ONLY for OutlineViews
                    if(bIsOutlineView)
                    {
                        // allow move up if position is 2 or greater OR it
                        // is a title object (and thus depth==1)
                        if(pOutl->GetAbsPos(pPara) > 1 || ( pOutl->HasParaFlag(pPara,PARAFLAG_ISPAGE) && pOutl->GetAbsPos(pPara) > 0 ) )
                        {
                            // Nicht ganz oben
                            bDisableUp = FALSE;
                        }
                    }
                    else
                    {
                        // old behaviour for OUTLINERMODE_OUTLINEOBJECT
                        if(pOutl->GetAbsPos(pPara) > 0)
                        {
                            // Nicht ganz oben
                            bDisableUp = FALSE;
                        }
                    }

                    for (std::vector<Paragraph*>::const_iterator iter = aSelList.begin(); iter != aSelList.end(); ++iter)
                    {
                        pPara = *iter;

                        sal_Int16 nDepth = pOutl->GetDepth( (USHORT) pOutl->GetAbsPos( pPara ) );

                        if (nDepth > 0 || (bOutlineViewSh && (nDepth <= 0) && !pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE )) )
                        {
                            // Nicht minimale Tiefe
                            bDisableLeft = FALSE;
                        }

                        if( (nDepth < pOLV->GetOutliner()->GetMaxDepth() && ( !bOutlineViewSh || pOutl->GetAbsPos(pPara) != 0 )) ||
                            (bOutlineViewSh && (nDepth <= 0) && pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE ) && pOutl->GetAbsPos(pPara) != 0) )
                        {
                            // Nicht maximale Tiefe und nicht ganz oben
                            bDisableRight = FALSE;
                        }
                    }

                    if ( ( pOutl->GetAbsPos(pPara) < pOutl->GetParagraphCount() - 1 ) &&
                         ( pOutl->GetParagraphCount() > 1 || !bOutlineViewSh) )
                    {
                        // Nicht letzter Absatz
                        bDisableDown = FALSE;
                    }

                    // disable when first para and 2nd is not a title
                    pPara = aSelList.empty() ? NULL : *(aSelList.begin());

                    if(!bDisableDown && bIsOutlineView
                        && pPara
                        && 0 == pOutl->GetAbsPos(pPara)
                        && pOutl->GetParagraphCount() > 1
                        && !pOutl->HasParaFlag( pOutl->GetParagraph(1), PARAFLAG_ISPAGE ) )
                    {
                        // Needs to be disabled
                        bDisableDown = TRUE;
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
                    BOOL bLeftToRight = TRUE;

                    SdrOutliner* pOutl = mpView->GetTextEditOutliner();
                    if( pOutl )
                    {
                        if( pOutl->IsVertical() )
                            bLeftToRight = FALSE;
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
                // todo
            }
            break;

            case SID_THES:
            {
                if( mpView && mpView->GetTextEditOutlinerView() )
                {
                    EditView & rEditView = mpView->GetTextEditOutlinerView()->GetEditView();;
                    String          aStatusVal;
                    LanguageType    nLang = LANGUAGE_NONE;
                    bool bIsLookUpWord = GetStatusValueForThesaurusFromContext( aStatusVal, nLang, rEditView );
                    rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

                    // disable "Thesaurus" context menu entry if there is nothing to look up
                    lang::Locale aLocale( SvxCreateLocale( nLang ) );
                    uno::Reference< linguistic2::XThesaurus > xThes( LinguMgr::GetThesaurus() );
                    if (!bIsLookUpWord ||
                        !xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( aLocale ))
                        rSet.DisableItem( SID_THES );
                }
                else
                {
                    rSet.DisableItem( SID_THES );
                }
                //! avoid puting the same item as SfxBoolItem at the end of this function
                nSlotId = 0;
            }
            break;

            default:
            break;
        }

        nWhich = aIter.NextWhich();
    }

    rSet.Put( aAttrSet, FALSE ); // <- FALSE, damit DontCare-Status uebernommen wird


    // die sind im Gliederungsmodus disabled
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
        // Absatzabstand
        OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
        if( pOLV )
        {
            ESelection aSel = pOLV->GetSelection();
            aSel.Adjust();
            ULONG nStartPara = aSel.nStartPara;
            ULONG nEndPara = aSel.nEndPara;
            if( !aSel.HasRange() )
            {
                nStartPara = 0;
                nEndPara = pOLV->GetOutliner()->GetParagraphCount() - 1;
            }
            long nUpper = 0L;
            for( ULONG nPara = nStartPara; nPara <= nEndPara; nPara++ )
            {
                const SfxItemSet& rItems = pOLV->GetOutliner()->GetParaAttribs( (USHORT)nPara );
                const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) rItems.Get( EE_PARA_ULSPACE );
                nUpper = Max( nUpper, (long)rItem.GetUpper() );
            }
            if( nUpper == 0L )
                rSet.DisableItem( SID_PARASPACE_DECREASE );
        }
        else
        {
            // Wird zur Zeit nie disabled !
            //rSet.DisableItem( SID_PARASPACE_INCREASE );
            //rSet.DisableItem( SID_PARASPACE_DECREASE );
        }

        // Absatzausrichtung
        SvxAdjust eAdj = ( (const SvxAdjustItem&) aAttrSet.Get( EE_PARA_JUST ) ).GetAdjust();
        switch( eAdj )
        {
            case SVX_ADJUST_LEFT:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_LEFT, TRUE ) );
            break;
            case SVX_ADJUST_CENTER:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_CENTER, TRUE ) );
            break;
            case SVX_ADJUST_RIGHT:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, TRUE ) );
            break;
            case SVX_ADJUST_BLOCK:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, TRUE ) );
            break;
            default:
            break;
        }

        // paragraph text direction
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
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_LEFT_TO_RIGHT, TRUE ) );
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_RIGHT_TO_LEFT, FALSE ) );
                break;

                case FRMDIR_HORI_RIGHT_TOP:
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_LEFT_TO_RIGHT, FALSE ) );
                    rSet.Put( SfxBoolItem( SID_ATTR_PARA_RIGHT_TO_LEFT, TRUE ) );
                break;

                // The case for the superordinate object is missing.
                case FRMDIR_ENVIRONMENT:
                {
                    SdDrawDocument* pDoc = mpView->GetDoc();
                    ::com::sun::star::text::WritingMode eMode = pDoc->GetDefaultWritingMode();
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

/* #i35937#
        if (aAttrSet.GetItemState(EE_PARA_BULLETSTATE) == SFX_ITEM_ON)
        {
            SfxUInt16Item aBulletState((const SfxUInt16Item&) aAttrSet.Get(EE_PARA_BULLETSTATE));

            if (aBulletState.GetValue() != 0)
            {
                rSet.Put(SfxBoolItem(FN_NUM_BULLET_ON, TRUE));
            }
            else
            {
                rSet.Put(SfxBoolItem(FN_NUM_BULLET_ON, FALSE));
            }
        }
*/
        USHORT nLineSpace = (USHORT) ( (const SvxLineSpacingItem&) aAttrSet.
                            Get( EE_PARA_SBL ) ).GetPropLineSpace();
        switch( nLineSpace )
        {
            case 100:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_10, TRUE ) );
            break;
            case 150:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_15, TRUE ) );
            break;
            case 200:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_20, TRUE ) );
            break;
        }
    }

    // Ausrichtung (hoch/tief) wird auch im Gliederungsmodus gebraucht
    SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                    aAttrSet.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

    if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
        rSet.Put( SfxBoolItem( SID_SET_SUPER_SCRIPT, TRUE ) );
    else if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
        rSet.Put( SfxBoolItem( SID_SET_SUB_SCRIPT, TRUE ) );
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void TextObjectBar::Command( const CommandEvent& )
{
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
