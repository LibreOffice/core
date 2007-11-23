/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drwtxtex.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:26:35 $
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
#include "precompiled_sw.hxx"

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <tools/shl.hxx>

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_PARAITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX
#ifndef _SVX_FLDITEM_HXX //autogen
#include <svx/flditem.hxx>
#endif
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _SVX_WRITINGMODEITEM_HXX
#include <svx/writingmodeitem.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _PARDLG_HXX
#include <pardlg.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _DRWTXTSH_HXX
#include <drwtxtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>               // fuer SpellPointer
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

#include "swabstdlg.hxx"
#include "chrdlg.hrc"

//modified on Jul. 30th
#include <svtools/languageoptions.hxx>
#include <svx/langitem.hxx>
#include <svtools/langtab.hxx>
#include <svtools/slstitm.hxx>
#include <string.h>

#include <svx/eeitem.hxx>
#include <svx/editeng.hxx>
#include <svx/editdata.hxx>
#include <svx/outliner.hxx>
#include <vcl/window.hxx>
#include <svx/editview.hxx>
#include <vcl/outdev.hxx>


using namespace ::com::sun::star;

static void lcl_SelectPara( EditView &rEditView, const ESelection &rCurSel )
{
    ESelection aParaSel( rCurSel.nStartPara, 0, rCurSel.nStartPara, USHRT_MAX );
    rEditView.SetSelection( aParaSel );
}

static void lcl_SetLanguage(SwWrtShell &rWrtSh, EditEngine* pEditEngine,ESelection aSelection, const String &rLangText, bool bIsForSelection, SfxItemSet &rCoreSet)
{
    const LanguageType nLang = SvtLanguageTable().GetType( rLangText );
    if (nLang != LANGUAGE_DONTKNOW)
    {
        USHORT nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLang );

        //get ScriptType
        USHORT nLangWhichId = 0;
        bool bIsSingleScriptType = true;
        switch (nScriptType)
        {
             case SCRIPTTYPE_LATIN :    nLangWhichId = EE_CHAR_LANGUAGE; break;
             case SCRIPTTYPE_ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
             case SCRIPTTYPE_COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
             default:
                bIsSingleScriptType = false;
                DBG_ERROR( "unexpected case" );
        }
        if (bIsSingleScriptType)
        {
            if (bIsForSelection)
            {
                // apply language to current selection
                rCoreSet.Put( SvxLanguageItem( nLang, nLangWhichId ));
                pEditEngine->QuickSetAttribs( rCoreSet, aSelection);
            }
            else // change document language
            {
                // set document default language
                switch (nLangWhichId)
                {
                     case EE_CHAR_LANGUAGE :      nLangWhichId = RES_CHRATR_LANGUAGE; break;
                     case EE_CHAR_LANGUAGE_CJK :  nLangWhichId = RES_CHRATR_CJK_LANGUAGE; break;
                     case EE_CHAR_LANGUAGE_CTL :  nLangWhichId = RES_CHRATR_CTL_LANGUAGE; break;
                }
                rWrtSh.SetDefault( SvxLanguageItem( nLang, nLangWhichId ) );

                // set respective language attribute in text document to default
                // (for all text in the document - which should be selected by now...)
                SvUShortsSort aAttribs;
                aAttribs.Insert( nLangWhichId );
                rWrtSh.ResetAttr( &aAttribs );
            }
        }
    }
}

static void lcl_SetLanguage_None(SwWrtShell &rWrtSh, EditEngine* pEditEngine,ESelection aSelection, bool bIsForSelection, SfxItemSet &rCoreSet )
{
    const USHORT aLangWhichId[3] =
    {
        EE_CHAR_LANGUAGE,
        EE_CHAR_LANGUAGE_CJK,
        EE_CHAR_LANGUAGE_CTL
    };

    const USHORT aLangWhichId_res[3] =
    {
        RES_CHRATR_LANGUAGE,
        RES_CHRATR_CJK_LANGUAGE,
        RES_CHRATR_CTL_LANGUAGE
    };

    if (bIsForSelection)
    {
        // apply language to current selection
        for (sal_uInt16 i = 0; i < 3; ++i)
            rCoreSet.Put( SvxLanguageItem( LANGUAGE_NONE, aLangWhichId[i] ));
        pEditEngine->QuickSetAttribs( rCoreSet, aSelection);
    }
    else // change document language
    {
        // set document default languages
        for (sal_uInt16 i = 0; i < 3; ++i)
            rWrtSh.SetDefault( SvxLanguageItem( SvxLanguageItem( LANGUAGE_NONE, aLangWhichId_res[i] ) ) );

        // set all language attributes in text document to default
        // (for all text in the document - which should be selected by now...)
        SvUShortsSort aAttribs;
        aAttribs.Insert( RES_CHRATR_LANGUAGE );
        aAttribs.Insert( RES_CHRATR_CJK_LANGUAGE );
        aAttribs.Insert( RES_CHRATR_CTL_LANGUAGE );
        rWrtSh.ResetAttr( &aAttribs );

    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDrawTextShell::Execute( SfxRequest &rReq )
{
    SwWrtShell &rSh = GetShell();

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    sal_uInt16 nSlot = rReq.GetSlot();

    sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    const SfxItemSet *pNewAttrs = rReq.GetArgs();

    bool bRestoreSelection = false;
    ESelection aOldSelection;

    sal_uInt16 nEEWhich = 0;
    switch (nSlot)
    {
        case SID_LANGUAGE_STATUS:
        {
            aOldSelection = pOLV->GetSelection();
            if (!pOLV->GetEditView().HasSelection())
            {
                bRestoreSelection   = true;
                pOLV->GetEditView().SelectCurrentWord();
            }

            ESelection   aSelection  = pOLV->GetSelection();
            EditView   & rEditView   = pOLV->GetEditView();
            EditEngine * pEditEngine = rEditView.GetEditEngine();

            // get the language
            String aNewLangTxt;

            SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, SID_LANGUAGE_STATUS , sal_False );
            if (pItem)
                aNewLangTxt = pItem->GetValue();

            //!! Remember the view frame right now...
            //!! (call to GetView().GetViewFrame() will break if the
            //!! SwTextShell got destroyed meanwhile.)
            SfxViewFrame *pViewFrame = GetView().GetViewFrame();

            if (aNewLangTxt.EqualsAscii( "*" ))
            {
                // open the dialog "Tools/Options/Language Settings - Language"
                SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                if (pFact)
                {
                    VclAbstractDialog* pDlg = pFact->CreateVclDialog( GetView().GetWindow(), SID_LANGUAGE_OPTIONS );
                    pDlg->Execute();
                    delete pDlg;
                }
            }
            else
            {
                // setting the new language...
                if (aNewLangTxt.Len() > 0)
                {
                    const String aSelectionLangPrefix( String::CreateFromAscii("Current_") );
                    const String aParagraphLangPrefix( String::CreateFromAscii("Paragraph_") );
                    const String aDocumentLangPrefix( String::CreateFromAscii("Default_") );
                    const String aStrNone( String::CreateFromAscii("LANGUAGE_NONE") );

                    xub_StrLen nPos = 0;
                    bool bForSelection = true;
                    bool bForParagraph = false;
                    if (STRING_NOTFOUND != (nPos = aNewLangTxt.Search( aSelectionLangPrefix, 0 )))
                    {
                        // ... for the current selection
                        aNewLangTxt = aNewLangTxt.Erase( nPos, aSelectionLangPrefix.Len() );
                        bForSelection = true;
                    }
                    else if (STRING_NOTFOUND != (nPos = aNewLangTxt.Search( aParagraphLangPrefix , 0 )))
                    {
                        // ... for the current paragraph language
                        aNewLangTxt = aNewLangTxt.Erase( nPos, aParagraphLangPrefix.Len() );
                        bForSelection = true;
                        bForParagraph = true;
                    }
                    else if (STRING_NOTFOUND != (nPos = aNewLangTxt.Search( aDocumentLangPrefix , 0 )))
                    {
                        // ... as default document language
                        aNewLangTxt = aNewLangTxt.Erase( nPos, aDocumentLangPrefix.Len() );
                        bForSelection = false;
                    }

                    if (bForParagraph)
                    {
                        bRestoreSelection = true;
                        lcl_SelectPara( rEditView, aSelection );
                        aSelection = pOLV->GetSelection();
                    }
                    if (!bForSelection) // document language to be changed...
                    {
                        rSh.StartAction();
                        rSh.LockView( TRUE );
                        rSh.Push();

                        // prepare to apply new language to all text in document
                        rSh.SelAll();
                    }

                    if (aNewLangTxt != aStrNone)
                        lcl_SetLanguage( rSh, pEditEngine, aSelection, aNewLangTxt, bForSelection, aEditAttr );
                    else
                        lcl_SetLanguage_None( rSh, pEditEngine, aSelection, bForSelection, aEditAttr );

                    if (!bForSelection)
                    {
                        // need to release view and restore selection...
                        rSh.Pop( FALSE );
                        rSh.LockView( FALSE );
                        rSh.EndAction();
                    }
                }
            }

            // invalidate slot to get the new language displayed
            pViewFrame->GetBindings().Invalidate( nSlot );

            rReq.Done();
            break;
        }
        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
        {
            SfxItemPool* pPool2 = aEditAttr.GetPool()->GetSecondaryPool();
            if( !pPool2 )
                pPool2 = aEditAttr.GetPool();
            SvxScriptSetItem aSetItem( nSlot, *pPool2 );

            // #i78017 establish the same behaviour as in Writer
            USHORT nScriptTypes = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
            if (nSlot == SID_ATTR_CHAR_FONT)
                nScriptTypes = pOLV->GetSelectedScriptType();

            aSetItem.PutItemForScriptType( nScriptTypes, pNewAttrs->Get( nWhich ) );
            aNewAttr.Put( aSetItem.GetItemSet() );
        }
        break;

        case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;

        case SID_ATTR_CHAR_UNDERLINE:
        {
             FontUnderline eFU = ((const SvxUnderlineItem&)aEditAttr.Get(EE_CHAR_UNDERLINE)).GetUnderline();
            aNewAttr.Put(SvxUnderlineItem(eFU == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_UNDERLINE));
        }
        break;

        case SID_ATTR_CHAR_CONTOUR:     nEEWhich = EE_CHAR_OUTLINE; break;
        case SID_ATTR_CHAR_SHADOWED:    nEEWhich = EE_CHAR_SHADOW; break;
        case SID_ATTR_CHAR_STRIKEOUT:   nEEWhich = EE_CHAR_STRIKEOUT; break;
        case SID_ATTR_CHAR_WORDLINEMODE: nEEWhich = EE_CHAR_WLM; break;
        case SID_ATTR_CHAR_RELIEF      : nEEWhich = EE_CHAR_RELIEF;  break;
        case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
        case SID_ATTR_CHAR_KERNING     : nEEWhich = EE_CHAR_KERNING; break;
        case SID_ATTR_CHAR_SCALEWIDTH:   nEEWhich = EE_CHAR_FONTWIDTH; break;
        case SID_ATTR_CHAR_AUTOKERN  :   nEEWhich = EE_CHAR_PAIRKERNING; break;
        case SID_ATTR_CHAR_ESCAPEMENT:   nEEWhich = EE_CHAR_ESCAPEMENT; break;
        case SID_ATTR_PARA_ADJUST_LEFT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_LEFT, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_RIGHT, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_BLOCK, EE_PARA_JUST));
        break;

        case SID_ATTR_PARA_LINESPACE_10:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_LINE, EE_PARA_SBL);
            aItem.SetPropLineSpace(100);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_15:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_POINT_FIVE_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(150);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_20:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_TWO_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(200);
            aNewAttr.Put(aItem);
        }
        break;

        case FN_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
        }
        break;
        case FN_SET_SUB_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
        }
        break;

        case SID_CHAR_DLG:
        case SID_CHAR_DLG_FOR_PARAGRAPH:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if( !pArgs )
            {
                aOldSelection = pOLV->GetSelection();
                if (nSlot == SID_CHAR_DLG_FOR_PARAGRAPH)
                {
                    // select current paragraph (and restore selection later on...)
                    EditView & rEditView = pOLV->GetEditView();
                    lcl_SelectPara( rEditView, rEditView.GetSelection() );
                    bRestoreSelection = true;
                }

                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< UINT16 >(eMetric)) );
                SfxItemSet aDlgAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);

                // util::Language gibts an der EditEngine nicht! Daher nicht im Set.

                aDlgAttr.Put( aEditAttr );
                aDlgAttr.Put( SvxKerningItem(0, RES_CHRATR_KERNING) );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

                SfxAbstractTabDialog* pDlg = pFact->CreateSwCharDlg( pView->GetWindow(), *pView, aDlgAttr, DLG_CHAR,0, sal_True );
                DBG_ASSERT(pDlg, "Dialogdiet fail!");
                USHORT nRet = pDlg->Execute();
                if(RET_OK == nRet )
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                }
                delete( pDlg );
                if(RET_OK != nRet)
                    return ;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;

        case SID_PARA_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (!pArgs)
            {
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< UINT16 >(eMetric)) );
                SfxItemSet aDlgAttr(GetPool(),
                                    EE_ITEMS_START, EE_ITEMS_END,
                                    SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                                    SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                                    SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                                    SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                                    0);

                aDlgAttr.Put(aEditAttr);

                // Die Werte sind erst einmal uebernommen worden, um den Dialog anzuzeigen.
                // Muss natuerlich noch geaendert werden
                // aDlgAttr.Put( SvxParaDlgLimitsItem( 567 * 50, 5670) );

                aDlgAttr.Put( SvxHyphenZoneItem( sal_False, RES_PARATR_HYPHENZONE) );
                aDlgAttr.Put( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
                aDlgAttr.Put( SvxFmtSplitItem( sal_True, RES_PARATR_SPLIT ) );
                aDlgAttr.Put( SvxWidowsItem( 0, RES_PARATR_WIDOWS ) );
                aDlgAttr.Put( SvxOrphansItem( 0, RES_PARATR_ORPHANS ) );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

                SfxAbstractTabDialog* pDlg = pFact->CreateSwParaDlg( GetView().GetWindow(), GetView(), aDlgAttr,DLG_STD, DLG_PARA, 0, sal_True );
                DBG_ASSERT(pDlg, "Dialogdiet fail!");
                USHORT nRet = pDlg->Execute();
                if(RET_OK == nRet)
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                }
                delete( pDlg );
                if(RET_OK != nRet)
                    return;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;
        case SID_AUTOSPELL_MARKOFF:
        case SID_AUTOSPELL_CHECK:
        {
//!! JP 16.03.2001: why??           pSdrView = rSh.GetDrawView();
//!! JP 16.03.2001: why??           pOutliner = pSdrView->GetTextEditOutliner();
            SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
            sal_uInt32 nCtrl = pOutliner->GetControlWord();

            sal_Bool bSet = ((const SfxBoolItem&)rReq.GetArgs()->Get(
                                                    nSlot)).GetValue();
            if(nSlot == SID_AUTOSPELL_MARKOFF)
            {
                if(bSet)
                    nCtrl |= EE_CNTRL_NOREDLINES;
                else
                    nCtrl &= ~EE_CNTRL_NOREDLINES;
            }
            else
            {
                if(bSet)
                    nCtrl |= EE_CNTRL_ONLINESPELLING|EE_CNTRL_ALLOWBIGOBJS;
                else
                    nCtrl &= ~EE_CNTRL_ONLINESPELLING;
            }
            pOutliner->SetControlWord(nCtrl);

            SW_MOD()->ExecuteSlot(rReq);
        }
        break;
        case SID_HYPERLINK_SETLINK:
        {
            const SfxPoolItem* pItem = 0;
            if(pNewAttrs)
                pNewAttrs->GetItemState(nSlot, sal_False, &pItem);

            if(pItem)
            {
                const SvxHyperlinkItem& rHLinkItem = *(const SvxHyperlinkItem *)pItem;
                SvxURLField aFld(rHLinkItem.GetURL(), rHLinkItem.GetName(), SVXURLFORMAT_APPDEFAULT);
                aFld.SetTargetFrame(rHLinkItem.GetTargetFrame());

                const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

                if (pFieldItem && pFieldItem->GetField()->ISA(SvxURLField))
                {
                    // Feld selektieren, so dass es beim Insert geloescht wird
                    ESelection aSel = pOLV->GetSelection();
                    aSel.nEndPos++;
                    pOLV->SetSelection(aSel);
                }
                pOLV->InsertField(SvxFieldItem(aFld, EE_FEATURE_FIELD));
            }
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            // Shellwechsel!
            {
                SdrObject* pTmpObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                SdrPageView* pTmpPV = pSdrView->GetSdrPageView();
                SdrView* pTmpView = pSdrView;

                pSdrView->SdrEndTextEdit(sal_True);

                SfxItemSet aAttr( *aNewAttr.GetPool(),
                            SDRATTR_TEXTDIRECTION,
                            SDRATTR_TEXTDIRECTION );

                aAttr.Put( SvxWritingModeItem(
                    nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                        text::WritingMode_LR_TB
                        : text::WritingMode_TB_RL, SDRATTR_TEXTDIRECTION ) );
                pTmpView->SetAttributes( aAttr );

                rSh.GetView().BeginTextEdit( pTmpObj, pTmpPV, &rSh.GetView().GetEditWin(), sal_False);
                rSh.GetView().AttrChangedNotify( &rSh );
            }
            return;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            SdrObject* pTmpObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            SdrPageView* pTmpPV = pSdrView->GetSdrPageView();
            SdrView* pTmpView = pSdrView;

            pSdrView->SdrEndTextEdit(sal_True);
            sal_Bool bLeftToRight = nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT;

            const SfxPoolItem* pPoolItem;
            if( pNewAttrs && SFX_ITEM_SET == pNewAttrs->GetItemState( nSlot, TRUE, &pPoolItem ) )
            {
                if( !( (SfxBoolItem*)pPoolItem)->GetValue() )
                    bLeftToRight = !bLeftToRight;
            }
            SfxItemSet aAttr( *aNewAttr.GetPool(),
                        EE_PARA_JUST, EE_PARA_JUST,
                        EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                        0 );

            USHORT nAdjust = SVX_ADJUST_LEFT;
            if( SFX_ITEM_ON == aEditAttr.GetItemState(EE_PARA_JUST, TRUE, &pPoolItem ) )
                nAdjust = ( (SvxAdjustItem*)pPoolItem)->GetEnumValue();

            if( bLeftToRight )
            {
                aAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );
                if( nAdjust == SVX_ADJUST_RIGHT )
                    aAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
            }
            else
            {
                aAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
                if( nAdjust == SVX_ADJUST_LEFT )
                    aAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
            }
            pTmpView->SetAttributes( aAttr );
            rSh.GetView().BeginTextEdit( pTmpObj, pTmpPV, &rSh.GetView().GetEditWin(), sal_False );
            rSh.GetView().AttrChangedNotify( &rSh );
        }
        return;
        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
    if(nEEWhich && pNewAttrs)
        aNewAttr.Put(pNewAttrs->Get(nWhich), nEEWhich);

    SetAttrToMarked(aNewAttr);

    GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    if (IsTextEdit() && pOLV->GetOutliner()->IsModified())
        rSh.SetModified();

    if (bRestoreSelection)
    {
        // restore selection
        pOLV->GetEditView().SetSelection( aOldSelection );
    }
}


static String lcl_GetTextForLanguageGuessing(EditEngine* rEditEngine, ESelection aDocSelection )
{
    // string for guessing language
    String aText;

    aText = rEditEngine->GetText(aDocSelection);
    if (aText.Len() > 0)
    {
        xub_StrLen nStt = 0;
        xub_StrLen nEnd = aDocSelection.nEndPos;
        // at most 100 chars to the left...
        nStt = nEnd > 100 ? nEnd - 100 : 0;
        // ... and 100 to the right of the cursor position
        nEnd = aText.Len() - nEnd > 100 ? nEnd + 100 : aText.Len();
        aText = aText.Copy( nStt, nEnd - nStt );
    }

    return aText;
}


/// @returns : the language for the selected text that is set for the
///     specified attribute (script type).
///     If there are more than one languages used LANGUAGE_DONTKNOW will be returned.
/// @param nLangWhichId : one of
///     EE_CHRATR_LANGUAGE, EE_CHRATR_CJK_LANGUAGE, EE_CHRATR_CTL_LANGUAGE,
static LanguageType lcl_GetLanguage( SfxItemSet aSet, USHORT nLangWhichId )
{

    LanguageType nLang = LANGUAGE_SYSTEM;

    //SfxItemSet aSet( rSh.GetAttrPool(), nLangWhichId, nLangWhichId );
    //rSh.GetAttr( aSet );

    const SfxPoolItem *pItem = 0;
    SfxItemState nState = aSet.GetItemState( nLangWhichId, TRUE, &pItem );
    if (nState > SFX_ITEM_DEFAULT && pItem)
    {
        // the item is set and can be used
        nLang = (dynamic_cast< const SvxLanguageItem* >(pItem))->GetLanguage();
    }
    else if (nState == SFX_ITEM_DEFAULT)
    {
        // since the attribute is not set: retrieve the default value
        nLang = (dynamic_cast< const SvxLanguageItem& >(aSet.GetPool()->GetDefaultItem( nLangWhichId ))).GetLanguage();
    }
    else if (nState == SFX_ITEM_DONTCARE)
    {
        // there is more than one language...
        nLang = LANGUAGE_DONTKNOW;
    }
    DBG_ASSERT( nLang != LANGUAGE_SYSTEM, "failed to get the language?" );

    return nLang;
}

/// @returns: the language in use for the selected text.
///     'In use' means the language(s) matching the script type(s) of the
///     selected text. Or in other words, the language a spell checker would use.
///     If there is more than one language LANGUAGE_DONTKNOW will be returned.
static LanguageType lcl_GetCurrentLanguage(SfxItemSet aSet,USHORT nScriptType )
{
    // get all script types used in current selection
    //const USHORT nScriptType = rSh.GetScriptType();

    //set language attribute to use according to the script type
    USHORT nLangWhichId = 0;
    bool bIsSingleScriptType = true;
    switch (nScriptType)
    {
         case SCRIPTTYPE_LATIN :    nLangWhichId = EE_CHAR_LANGUAGE; break;
         case SCRIPTTYPE_ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
         case SCRIPTTYPE_COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
         default: bIsSingleScriptType = false;
    }

    // get language according to the script type(s) in use
    LanguageType nCurrentLang = LANGUAGE_SYSTEM;
    if (bIsSingleScriptType)
        nCurrentLang = lcl_GetLanguage( aSet, nLangWhichId );
    else
    {
        // check if all script types are set to LANGUAGE_NONE and return
        // that if this is the case. Otherwise, having multiple script types
        // in use always means there are several languages in use...
        const USHORT aScriptTypes[3] =
        {
            EE_CHAR_LANGUAGE,
            EE_CHAR_LANGUAGE_CJK,
            EE_CHAR_LANGUAGE_CTL
        };
        nCurrentLang = LANGUAGE_NONE;
        for (sal_uInt16 i = 0; i < 3; ++i)
        {
            LanguageType nTmpLang = lcl_GetLanguage( aSet, aScriptTypes[i] );
            if (nTmpLang != LANGUAGE_NONE)
            {
                nCurrentLang = LANGUAGE_DONTKNOW;
                break;
            }
        }
    }
    DBG_ASSERT( nCurrentLang != LANGUAGE_SYSTEM, "failed to get the language?" );

    return nCurrentLang;
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDrawTextShell::GetState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Sonst manchmal Absturz!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    SfxItemSet aEditAttr( pOLV->GetAttribs() );
    const SfxPoolItem *pAdjust = 0, *pLSpace = 0, *pEscItem = 0;
    int eAdjust, nLSpace, nEsc;

    while(nWhich)
    {
        USHORT nSlotId = GetPool().GetSlotId( nWhich );
        BOOL bFlag = FALSE;
        switch( nSlotId )
        {
            case SID_LANGUAGE_STATUS://20412:
            {
                ESelection aSelection = pOLV->GetSelection();
                EditView& rEditView=pOLV->GetEditView();
                EditEngine* pEditEngine=rEditView.GetEditEngine();

                // the value of used script types
                const USHORT nScriptType =pOLV->GetSelectedScriptType();
                String aScriptTypesInUse( String::CreateFromInt32( nScriptType ) );//pEditEngine->GetScriptType(aSelection)

                SvtLanguageTable aLangTable;

                // get keyboard language
                String aKeyboardLang;
                LanguageType nLang = LANGUAGE_DONTKNOW;

                Window* pWin = rEditView.GetWindow();
                if(pWin)
                    nLang = pWin->GetInputLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aKeyboardLang = aLangTable.GetString( nLang );


                // get the language that is in use
                const String aMultipleLanguages = String::CreateFromAscii("*");
                String aCurrentLang = aMultipleLanguages;
                SfxItemSet aSet(pOLV->GetAttribs());
                nLang = lcl_GetCurrentLanguage( aSet,nScriptType );
                if (nLang != LANGUAGE_DONTKNOW)
                    aCurrentLang = aLangTable.GetString( nLang );

                // build sequence for status value
                uno::Sequence< ::rtl::OUString > aSeq( 4 );
                aSeq[0] = aCurrentLang;
                aSeq[1] = aScriptTypesInUse;
                aSeq[2] = aKeyboardLang;
                aSeq[3] = lcl_GetTextForLanguageGuessing( pEditEngine, aSelection );

                // set sequence as status value
                SfxStringListItem aItem( SID_LANGUAGE_STATUS );
                aItem.SetStringList( aSeq );
                rSet.Put( aItem, SID_LANGUAGE_STATUS );
                nSlotId = 0;
            }
            break;
         case SID_ATTR_PARA_ADJUST_LEFT:    eAdjust = SVX_ADJUST_LEFT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_RIGHT:    eAdjust = SVX_ADJUST_RIGHT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_CENTER:   eAdjust = SVX_ADJUST_CENTER; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_BLOCK:    eAdjust = SVX_ADJUST_BLOCK; goto ASK_ADJUST;
ASK_ADJUST:
            {
                if( !pAdjust )
                    aEditAttr.GetItemState( EE_PARA_JUST, sal_False, &pAdjust);

                if( !pAdjust || IsInvalidItem( pAdjust ))
                    rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                else
                    bFlag = eAdjust == ((SvxAdjustItem*)pAdjust)->GetAdjust();
            }
            break;

        case SID_ATTR_PARA_LINESPACE_10:    nLSpace = 100;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_15:    nLSpace = 150;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_20:    nLSpace = 200;  goto ASK_LINESPACE;
ASK_LINESPACE:
            {
                if( !pLSpace )
                    aEditAttr.GetItemState( EE_PARA_SBL, sal_False, &pLSpace );

                if( !pLSpace || IsInvalidItem( pLSpace ))
                    rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                else if( nLSpace == ((const SvxLineSpacingItem*)pLSpace)->
                                                GetPropLineSpace() )
                    bFlag = sal_True;
                else
                    nSlotId = 0;
            }
            break;

        case FN_SET_SUPER_SCRIPT:   nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
                                    goto ASK_ESCAPE;
        case FN_SET_SUB_SCRIPT:     nEsc = SVX_ESCAPEMENT_SUBSCRIPT;
                                    goto ASK_ESCAPE;
ASK_ESCAPE:
            {
                if( !pEscItem )
                    pEscItem = &aEditAttr.Get( EE_CHAR_ESCAPEMENT );

                if( nEsc == ((const SvxEscapementItem*)
                                                pEscItem)->GetEnumValue() )
                    bFlag = sal_True;
                else
                    nSlotId = 0;
            }
            break;

        case FN_THESAURUS_DLG:
        {
            // disable "Thesaurus" if the language is not supported
            const SfxPoolItem &rItem = GetShell().GetDoc()->GetDefault(
                            GetWhichOfScript( RES_CHRATR_LANGUAGE,
                            GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage())) );
            LanguageType nLang = ((const SvxLanguageItem &)
                                                    rItem).GetLanguage();
            //
            uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
            if (!xThes.is() || nLang == LANGUAGE_NONE ||
                !xThes->hasLocale( SvxCreateLocale( nLang ) ))
                rSet.DisableItem( FN_THESAURUS_DLG );
            nSlotId = 0;
        }
        break;
        case SID_HANGUL_HANJA_CONVERSION:
        case SID_CHINESE_CONVERSION:
        {
            if (!SvtCJKOptions().IsAnyEnabled())
            {
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_False );
                rSet.DisableItem(nWhich);
            }
            else
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_True );
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            if ( !SvtLanguageOptions().IsVerticalTextEnabled() )
            {
                rSet.DisableItem( nSlotId );
                nSlotId = 0;
            }
            else
            {
                SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
                if( pOutliner )
                    bFlag = pOutliner->IsVertical() ==
                            (SID_TEXTDIRECTION_TOP_TO_BOTTOM == nSlotId);
                else
                {
                    text::WritingMode eMode = (text::WritingMode)
                                    ( (const SvxWritingModeItem&) aEditAttr.Get( SDRATTR_TEXTDIRECTION ) ).GetValue();

                    if( nSlotId == SID_TEXTDIRECTION_LEFT_TO_RIGHT )
                    {
                        bFlag = eMode == text::WritingMode_LR_TB;
                    }
                    else
                    {
                        bFlag = eMode != text::WritingMode_TB_RL;
                    }
                }
            }
            break;
        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            if ( !SvtLanguageOptions().IsCTLFontEnabled() )
            {
                rSet.DisableItem( nWhich );
                nSlotId = 0;
            }
            else
            {
                SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
                if(pOutliner && pOutliner->IsVertical())
                {
                    rSet.DisableItem( nWhich );
                    nSlotId = 0;
                }
                else
                {
                    switch( ( ( (SvxFrameDirectionItem&) aEditAttr.Get( EE_PARA_WRITINGDIR ) ) ).GetValue() )
                    {
                        case FRMDIR_HORI_LEFT_TOP:
                            bFlag = nWhich == SID_ATTR_PARA_LEFT_TO_RIGHT;
                        break;

                        case FRMDIR_HORI_RIGHT_TOP:
                            bFlag = nWhich != SID_ATTR_PARA_LEFT_TO_RIGHT;
                        break;
                    }
                }
            }
        }
        break;
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAGANA:
        {
            SvtCJKOptions aCJKOptions;
            if(!aCJKOptions.IsChangeCaseMapEnabled())
            {
                rSet.DisableItem(nWhich);
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_False );
            }
            else
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_True );
        }
        break;
        default:
            nSlotId = 0;                // don't know this slot
            break;
        }

        if( nSlotId )
            rSet.Put( SfxBoolItem( nWhich, bFlag ));

        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::GetDrawTxtCtrlState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    USHORT nScriptType = pOLV->GetSelectedScriptType();
    while(nWhich)
    {
        sal_uInt16 nEEWhich = 0;
        USHORT nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_POSTURE:
            {
                SfxItemPool* pEditPool = aEditAttr.GetPool()->GetSecondaryPool();
                if( !pEditPool )
                    pEditPool = aEditAttr.GetPool();
                SvxScriptSetItem aSetItem( nSlotId, *pEditPool );
                aSetItem.GetItemSet().Put( aEditAttr, FALSE );
                const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                if( pI )
                    rSet.Put( *pI, nWhich );
                else
                    rSet.InvalidateItem( nWhich );
            }
            break;
            case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;
            case SID_ATTR_CHAR_UNDERLINE: nEEWhich = EE_CHAR_UNDERLINE;break;
            case SID_ATTR_CHAR_CONTOUR: nEEWhich = EE_CHAR_OUTLINE; break;
            case SID_ATTR_CHAR_SHADOWED:  nEEWhich = EE_CHAR_SHADOW;break;
            case SID_ATTR_CHAR_STRIKEOUT: nEEWhich = EE_CHAR_STRIKEOUT;break;
            case SID_AUTOSPELL_MARKOFF:
            case SID_AUTOSPELL_CHECK:
            {
                const SfxPoolItem* pState = SW_MOD()->GetSlotState(nWhich);
                if (pState)
                    rSet.Put(SfxBoolItem(nWhich, ((const SfxBoolItem*)pState)->GetValue()));
                else
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_ATTR_CHAR_WORDLINEMODE: nEEWhich = EE_CHAR_WLM; break;
            case SID_ATTR_CHAR_RELIEF      : nEEWhich = EE_CHAR_RELIEF;  break;
            case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
            case SID_ATTR_CHAR_KERNING     : nEEWhich = EE_CHAR_KERNING; break;
            case SID_ATTR_CHAR_SCALEWIDTH:   nEEWhich = EE_CHAR_FONTWIDTH;break;
            case SID_ATTR_CHAR_AUTOKERN  :   nEEWhich = EE_CHAR_PAIRKERNING; break;
            case SID_ATTR_CHAR_ESCAPEMENT:   nEEWhich = EE_CHAR_ESCAPEMENT; break;
        }
        if(nEEWhich)
            rSet.Put(aEditAttr.Get(nEEWhich, sal_True), nWhich);

        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
void SwDrawTextShell::ExecClpbrd(SfxRequest &rReq)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_CUT:
            pOLV->Cut();
            return;

        case SID_COPY:
            pOLV->Copy();
            return;

        case SID_PASTE:
            pOLV->PasteSpecial();
            break;

        default:
            DBG_ERROR("falscher Dispatcher");
            return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   ClipBoard-Status
 --------------------------------------------------------------------*/



void SwDrawTextShell::StateClpbrd(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    ESelection aSel(pOLV->GetSelection());
    const sal_Bool bCopy = (aSel.nStartPara != aSel.nEndPara) ||
                           (aSel.nStartPos != aSel.nEndPos);


    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_CUT:
            case SID_COPY:
                if( !bCopy )
                    rSet.DisableItem( nWhich );
                break;

            case SID_PASTE:
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                &GetView().GetEditWin() ) );

                    if( !aDataHelper.GetXTransferable().is() ||
                        !SwTransferable::IsPaste( GetShell(), aDataHelper ))
                        rSet.DisableItem( SID_PASTE );
                }
                break;

            case FN_PASTESPECIAL:
                rSet.DisableItem( FN_PASTESPECIAL );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Hyperlink-Status
 --------------------------------------------------------------------*/

void SwDrawTextShell::StateInsert(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    aHLinkItem.SetInsertMode(HLINK_FIELD);

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
                    else
                    {
                        String sSel(pOLV->GetSelected());
                        sSel.Erase(255);
                        sSel.EraseTrailingChars();
                        aHLinkItem.SetName(sSel);
                    }

                    sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}




