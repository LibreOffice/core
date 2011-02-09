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
#include "precompiled_sw.hxx"

#include <hintids.hxx>

#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <tools/shl.hxx>
#include <i18npool/mslangid.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <svl/stritem.hxx>
#include <svl/slstitm.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoutl.hxx>
#include <svl/whiter.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editstat.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/htmlmode.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/writingmodeitem.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <uitool.hxx>
#include <popup.hrc>
#include <pardlg.hxx>
#include <swdtflvr.hxx>
#include <drwtxtsh.hxx>
#include <swmodule.hxx>
#include <initui.hxx>
#include <edtwin.hxx>
#include <swwait.hxx>
#include <docstat.hxx>

#include <cmdid.h>
#include <globals.hrc>
#include <shells.hrc>
#include <breakit.hxx>
#include "annotsh.hxx"
#include "view.hxx"
#include <PostItMgr.hxx>
#include <SidebarWin.hxx>

#include "swtypes.hxx"

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <svx/svxids.hrc>
#include <svl/itempool.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

#include <svl/languageoptions.hxx>
#include <editeng/langitem.hxx>
#include <svtools/langtab.hxx>
#include <svl/slstitm.hxx>

#include <docsh.hxx>
#include <svl/undo.hxx>
#include "swabstdlg.hxx"
#include "chrdlg.hrc"
#include "misc.hrc"
#include <app.hrc>

#include <comphelper/processfactory.hxx>

#include <cppuhelper/bootstrap.hxx>

#include <langhelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;

#define SwAnnotationShell

#include <sfx2/msg.hxx>
#include <swslots.hxx>

SFX_IMPL_INTERFACE(SwAnnotationShell, SfxShell, SW_RES(STR_SHELLNAME_DRAW_TEXT))
{
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TEXT_TOOLBOX));
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_ANNOTATION_POPUPMENU));
}

TYPEINIT1(SwAnnotationShell,SfxShell)

SwAnnotationShell::SwAnnotationShell( SwView& r )
: rView(r)
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SetPool(rSh.GetAttrPool().GetSecondaryPool());
}

SwAnnotationShell::~SwAnnotationShell()
{
}

::svl::IUndoManager* SwAnnotationShell::GetUndoManager()
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr ||
         !pPostItMgr->HasActiveSidebarWin() )
    {
        DBG_ASSERT(pPostItMgr,"PostItMgr::Layout(): We are looping forever");
        return 0;
    }
    return &pPostItMgr->GetActiveSidebarWin()->GetOutlinerView()->GetOutliner()->GetUndoManager();
}

void SwAnnotationShell::Exec( SfxRequest &rReq )
{
    //TODO: clean this up!!!!
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    sal_uInt16 nSlot = rReq.GetSlot();
    sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    const SfxItemSet *pNewAttrs = rReq.GetArgs();
    sal_uInt16 nEEWhich = 0;
    switch (nSlot)
    {
        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
            {
                SfxItemPool* pSecondPool = aEditAttr.GetPool()->GetSecondaryPool();
                if( !pSecondPool )
                    pSecondPool = aEditAttr.GetPool();
                SvxScriptSetItem aSetItem( nSlot, *pSecondPool );
                aSetItem.PutItemForScriptType( pOLV->GetSelectedScriptType(), pNewAttrs->Get( nWhich ));
                aNewAttr.Put( aSetItem.GetItemSet() );
                rReq.Done();
                break;
            }
        case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;
        case SID_ATTR_CHAR_UNDERLINE:
        {
             FontUnderline eFU = ((const SvxUnderlineItem&)aEditAttr.Get(EE_CHAR_UNDERLINE)).GetLineStyle();
            aNewAttr.Put(SvxUnderlineItem(eFU == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_UNDERLINE));
            break;
        }
        case SID_ATTR_CHAR_OVERLINE:
        {
             FontUnderline eFO = ((const SvxOverlineItem&)aEditAttr.Get(EE_CHAR_OVERLINE)).GetLineStyle();
            aNewAttr.Put(SvxOverlineItem(eFO == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_OVERLINE));
            break;
        }
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
        case SID_SELECTALL:
        {
            Outliner * pOutliner = pOLV->GetOutliner();
            if(pOutliner)
            {
                sal_uLong nParaCount = pOutliner->GetParagraphCount();
                if (nParaCount > 0)
                    pOLV->SelectRange(0L, sal_uInt16(nParaCount) );
            }
            break;
        }
        case FN_FORMAT_RESET:
        {
            pPostItMgr->GetActiveSidebarWin()->ResetAttributes();
            rReq.Done();
            break;
        }
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
                if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                    pOLV->InsertField(SvxFieldItem(aFld, EE_FEATURE_FIELD));
            }
            break;
        }
        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        {
            sal_Unicode cIns = 0;
            switch(rReq.GetSlot())
            {
                case FN_INSERT_SOFT_HYPHEN: cIns = CHAR_SOFTHYPHEN; break;
                case FN_INSERT_HARDHYPHEN: cIns = CHAR_HARDHYPHEN; break;
                case FN_INSERT_HARD_SPACE: cIns = CHAR_HARDBLANK; break;
                case SID_INSERT_RLM : cIns = CHAR_RLM ; break;
                case SID_INSERT_LRM : cIns = CHAR_LRM ; break;
                case SID_INSERT_ZWSP : cIns = CHAR_ZWSP ; break;
                case SID_INSERT_ZWNBSP: cIns = CHAR_ZWNBSP; break;
            }
            pOLV->InsertText( String(cIns));
            rReq.Done();
            break;
        }
        case SID_CHARMAP:
        {
            if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                InsertSymbol(rReq);
            break;
        }
            case FN_INSERT_STRING:
                {
            const SfxPoolItem* pItem = 0;
            if(pNewAttrs)
                pNewAttrs->GetItemState(nSlot, sal_False, &pItem );
                        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                                pOLV->InsertText(((const SfxStringItem *)pItem)->GetValue());
                        break;
                }

        case FN_FORMAT_FOOTNOTE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDlg = pFact->CreateSwFootNoteOptionDlg( rView.GetWindow(), rView.GetWrtShell(), DLG_DOC_FOOTNOTE );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            break;
        }
        case FN_NUMBERING_OUTLINE_DLG:
        {
            SfxItemSet aTmp(GetPool(), FN_PARAM_1, FN_PARAM_1);
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            SfxAbstractTabDialog* pDlg = pFact->CreateSwTabDialog( DLG_TAB_OUTLINE,
                                                        rView.GetWindow(), &aTmp, rView.GetWrtShell());
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            rReq.Done();
        }
        break;
        case SID_OPEN_XML_FILTERSETTINGS:
        {
            try
            {
                uno::Reference < ui::dialogs::XExecutableDialog > xDialog(::comphelper::getProcessServiceFactory()->createInstance(rtl::OUString::createFromAscii("com.sun.star.comp.ui.XSLTFilterDialog")), uno::UNO_QUERY);
                if( xDialog.is() )
                {
                    xDialog->execute();
                }
            }
            catch( uno::Exception& )
            {
            }
            rReq.Ignore ();
        }
        break;
        case FN_WORDCOUNT_DIALOG:
        {
            SwWrtShell &rSh = rView.GetWrtShell();
            SwDocStat aCurr;
            SwDocStat aDocStat( rSh.getIDocumentStatistics()->GetDocStat() );
            {
                SwWait aWait( *rView.GetDocShell(), sal_True );
                rSh.StartAction();
                rSh.CountWords( aCurr );
                rSh.UpdateDocStat( aDocStat );
                rSh.EndAction();
            }

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSwWordCountDialog* pDialog = pFact->CreateSwWordCountDialog( rView.GetWindow() );
            pDialog->SetValues(aCurr, aDocStat );
            pDialog->Execute();
            delete pDialog;
        }
        break;
        case SID_CHAR_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if( !pArgs )
            {
                /* mod
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                */
                SfxItemSet aDlgAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);

                // util::Language gibts an der EditEngine nicht! Daher nicht im Set.

                aDlgAttr.Put( aEditAttr );
                aDlgAttr.Put( SvxKerningItem(0, RES_CHRATR_KERNING) );

                //CHINA001 SwCharDlg* pDlg = new SwCharDlg(pView->GetWindow(), *pView, aDlgAttr, 0, sal_True);
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
                DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001

                SfxAbstractTabDialog* pDlg = pFact->CreateSwCharDlg( rView.GetWindow(), rView, aDlgAttr, DLG_CHAR,0, sal_True );
                DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                sal_uInt16 nRet = pDlg->Execute();
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
        break;
        }
        case SID_PARA_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (!pArgs)
            {
                /* mod todo ???
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                */
                SfxItemSet aDlgAttr(GetPool(),
                                    EE_ITEMS_START, EE_ITEMS_END,
                                    SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                                    SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                                    SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                                    SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                                    0);

                aDlgAttr.Put(aEditAttr);


                aDlgAttr.Put( SvxHyphenZoneItem( sal_False, RES_PARATR_HYPHENZONE) );
                aDlgAttr.Put( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
                aDlgAttr.Put( SvxFmtSplitItem( sal_True, RES_PARATR_SPLIT ) );
                aDlgAttr.Put( SvxWidowsItem( 0, RES_PARATR_WIDOWS ) );
                aDlgAttr.Put( SvxOrphansItem( 0, RES_PARATR_ORPHANS ) );

                //CHINA001 SwParaDlg* pDlg = new SwParaDlg(GetView().GetWindow(), GetView(), aDlgAttr, DLG_STD, 0, sal_True);

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
                DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001

                SfxAbstractTabDialog* pDlg = pFact->CreateSwParaDlg( rView.GetWindow(), rView, aDlgAttr,DLG_STD, DLG_PARA, 0, sal_True );
                DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                sal_uInt16 nRet = pDlg->Execute();
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
            break;
        }

        case SID_AUTOSPELL_CHECK:
        {
            rView.ExecuteSlot(rReq);
            break;
        }
        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            sal_Bool bLeftToRight = nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT;

            const SfxPoolItem* pPoolItem;
            if( pNewAttrs && SFX_ITEM_SET == pNewAttrs->GetItemState( nSlot, sal_True, &pPoolItem ) )
            {
                if( !( (SfxBoolItem*)pPoolItem)->GetValue() )
                    bLeftToRight = !bLeftToRight;
            }
            SfxItemSet aAttr( *aNewAttr.GetPool(),
                        EE_PARA_JUST, EE_PARA_JUST,
                        EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                        0 );

            sal_uInt16 nAdjust = SVX_ADJUST_LEFT;
            if( SFX_ITEM_ON == aEditAttr.GetItemState(EE_PARA_JUST, sal_True, &pPoolItem ) )
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
            pOLV->SetAttribs(aAttr);
            break;
        }
    }

    if(nEEWhich && pNewAttrs)
        aNewAttr.Put(pNewAttrs->Get(nWhich), nEEWhich);

    Rectangle aNullRect;
    Rectangle aOutRect = pOLV->GetOutputArea();
    if (aNullRect != aOutRect)
        pOLV->SetAttribs(aNewAttr);

    rView.GetViewFrame()->GetBindings().InvalidateAll(sal_False);
    if ( pOLV->GetOutliner()->IsModified() )
        rView.GetWrtShell().SetModified();

}

void SwAnnotationShell::GetState(SfxItemSet& rSet)
{
    //TODO: clean this up!!!
    // FN_SET_SUPER_SCRIPT
    //SID_ATTR_PARA_ADJUST
    //SID_ATTR_PARA_ADJUST_BLOCK

    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        sal_uInt16 nEEWhich = 0;
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_POSTURE:
                {
                    if ( pOLV )
                    {
                        sal_uInt16 nScriptType = pOLV->GetSelectedScriptType();
                        SfxItemPool* pSecondPool = aEditAttr.GetPool()->GetSecondaryPool();
                        if( !pSecondPool )
                            pSecondPool = aEditAttr.GetPool();
                        SvxScriptSetItem aSetItem( nSlotId, *pSecondPool );
                        aSetItem.GetItemSet().Put( aEditAttr, sal_False );
                        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                        if( pI )
                            rSet.Put( *pI, nWhich );
                        else
                            rSet.InvalidateItem( nWhich );
                    }
                    else
                        rSet.InvalidateItem( nWhich );
                }
                break;
            case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;
            case SID_ATTR_CHAR_UNDERLINE: nEEWhich = EE_CHAR_UNDERLINE;break;
            case SID_ATTR_CHAR_OVERLINE: nEEWhich = EE_CHAR_OVERLINE;break;
            case SID_ATTR_CHAR_CONTOUR: nEEWhich = EE_CHAR_OUTLINE; break;
            case SID_ATTR_CHAR_SHADOWED:  nEEWhich = EE_CHAR_SHADOW;break;
            case SID_ATTR_CHAR_STRIKEOUT: nEEWhich = EE_CHAR_STRIKEOUT;break;
            case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
            case FN_SET_SUPER_SCRIPT:
            case FN_SET_SUB_SCRIPT:
            {
                sal_uInt16 nEsc = 0;
                const SfxPoolItem *pEscItem = 0;
                if (nWhich==FN_SET_SUPER_SCRIPT)
                    nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
                else
                    nEsc = SVX_ESCAPEMENT_SUBSCRIPT;

                if( !pEscItem )
                    pEscItem = &aEditAttr.Get( EE_CHAR_ESCAPEMENT );

                if( nEsc == ((const SvxEscapementItem*)pEscItem)->GetEnumValue() )
                    rSet.Put( SfxBoolItem( nWhich, sal_True ));
                else
                    rSet.InvalidateItem( nWhich );
                break;
            }
            case SID_ATTR_PARA_ADJUST_LEFT:
            case SID_ATTR_PARA_ADJUST_RIGHT:
            case SID_ATTR_PARA_ADJUST_CENTER:
            case SID_ATTR_PARA_ADJUST_BLOCK:
                {
                    const SfxPoolItem *pAdjust = 0;
                    int eAdjust = 0;

                    if (nWhich==SID_ATTR_PARA_ADJUST_LEFT)
                        eAdjust = SVX_ADJUST_LEFT;
                    else
                    if (nWhich==SID_ATTR_PARA_ADJUST_RIGHT)
                        eAdjust = SVX_ADJUST_RIGHT;
                    else
                    if (nWhich==SID_ATTR_PARA_ADJUST_CENTER)
                        eAdjust = SVX_ADJUST_CENTER;
                    else
                    if (nWhich==SID_ATTR_PARA_ADJUST_BLOCK)
                        eAdjust = SVX_ADJUST_BLOCK;

                    if( !pAdjust )
                        aEditAttr.GetItemState( EE_PARA_JUST, sal_False, &pAdjust);

                    if( !pAdjust || IsInvalidItem( pAdjust ))
                        rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                    else
                    {
                        if ( eAdjust == ((const SvxAdjustItem*)pAdjust)->GetAdjust())
                            rSet.Put( SfxBoolItem( nWhich, sal_True ));
                        else
                            rSet.InvalidateItem( nWhich );
                    }
                    break;
                }
            case SID_ATTR_PARA_LINESPACE_10:
            case SID_ATTR_PARA_LINESPACE_15:
            case SID_ATTR_PARA_LINESPACE_20:
                {
                    const SfxPoolItem *pLSpace = 0;
                    int nLSpace = 0;

                    if (nWhich==SID_ATTR_PARA_LINESPACE_10)
                        nLSpace = 100;
                    else
                    if (nWhich==SID_ATTR_PARA_LINESPACE_15)
                        nLSpace = 150;
                    else
                    if (nWhich==SID_ATTR_PARA_LINESPACE_20)
                        nLSpace = 200;

                    if( !pLSpace )
                        aEditAttr.GetItemState( EE_PARA_SBL, sal_False, &pLSpace );

                    if( !pLSpace || IsInvalidItem( pLSpace ))
                        rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                    else
                    {
                        if( nLSpace == ((const SvxLineSpacingItem*)pLSpace)->GetPropLineSpace() )
                            rSet.Put( SfxBoolItem( nWhich, sal_True ));
                        else
                            rSet.InvalidateItem( nWhich );
                    }
                    break;
                }
            case SID_AUTOSPELL_CHECK:
            {
                const SfxPoolItem* pState = rView.GetSlotState(nWhich);
                if (pState)
                    rSet.Put(SfxBoolItem(nWhich, ((const SfxBoolItem*)pState)->GetValue()));
                else
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_ATTR_PARA_LEFT_TO_RIGHT:
            case SID_ATTR_PARA_RIGHT_TO_LEFT:
            {
                if ( !SvtLanguageOptions().IsCTLFontEnabled() )
                    rSet.DisableItem( nWhich );
                else
                {
                    if(pOLV->GetOutliner() && pOLV->GetOutliner()->IsVertical())
                        rSet.DisableItem( nWhich );
                    else
                    {
                        sal_Bool bFlag = sal_False;
                        switch( ( ( (SvxFrameDirectionItem&) aEditAttr.Get( EE_PARA_WRITINGDIR ) ) ).GetValue() )
                        {
                            case FRMDIR_HORI_LEFT_TOP:
                            {
                                bFlag = nWhich == SID_ATTR_PARA_LEFT_TO_RIGHT;
                                rSet.Put( SfxBoolItem( nWhich, bFlag ));
                                break;
                            }
                            case FRMDIR_HORI_RIGHT_TOP:
                            {
                                bFlag = nWhich != SID_ATTR_PARA_LEFT_TO_RIGHT;
                                rSet.Put( SfxBoolItem( nWhich, bFlag ));
                                break;
                            }
                        }
                    }
                }
            }
            break;
            case SID_INSERT_RLM :
            case SID_INSERT_LRM :
            case SID_INSERT_ZWNBSP :
            case SID_INSERT_ZWSP:
            {
                SvtCTLOptions aCTLOptions;
                sal_Bool bEnabled = aCTLOptions.IsCTLFontEnabled();
                rView.GetViewFrame()->GetBindings().SetVisibleState( nWhich, bEnabled );
                if(!bEnabled)
                    rSet.DisableItem(nWhich);
            }
            break;
            default:
                rSet.InvalidateItem( nWhich );
                break;
        }

        if(nEEWhich)
            rSet.Put(aEditAttr.Get(nEEWhich, sal_True), nWhich);

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecSearch(SfxRequest& rReq, sal_Bool bNoMessage)
{
    rView.ExecSearch(rReq,bNoMessage);
}

void SwAnnotationShell::StateSearch(SfxItemSet &rSet)
{
    rView.StateSearch(rSet);
}

void SwAnnotationShell::ExecClpbrd(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    long aOldHeight = pPostItMgr->GetActiveSidebarWin()->GetPostItTextHeight();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_CUT:
            if ( (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED) && pOLV->HasSelection() )
                pOLV->Cut();
            break;
        case SID_COPY:
            if( pOLV->HasSelection() )
                pOLV->Copy();
            break;
        case SID_PASTE:
            if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                pOLV->Paste();
            break;
        case SID_PASTE_SPECIAL:
        {
            if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( &rView.GetEditWin() );

                pDlg->Insert( SOT_FORMAT_STRING, aEmptyStr );
                pDlg->Insert( SOT_FORMAT_RTF,    aEmptyStr );

                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( &rView.GetEditWin() ) );

                sal_uLong nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );

                if (nFormat > 0)
                {
                    if (nFormat == SOT_FORMAT_STRING)
                        pOLV->Paste();
                    else
                        pOLV->PasteSpecial();
                }
                delete pDlg;
            }
            break;
        }
        case SID_CLIPBOARD_FORMAT_ITEMS:
        {
            sal_uLong nFormat = 0;
            const SfxPoolItem* pItem;
            if ( rReq.GetArgs() && rReq.GetArgs()->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET &&
                                    pItem->ISA(SfxUInt32Item) )
            {
                nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
            }

            if ( nFormat )
            {
                if (SOT_FORMAT_STRING == nFormat)
                    pOLV->Paste();
                else
                    pOLV->PasteSpecial();
            }
            break;
        }
    }
    pPostItMgr->GetActiveSidebarWin()->ResizeIfNeccessary(aOldHeight,pPostItMgr->GetActiveSidebarWin()->GetPostItTextHeight());
}

void SwAnnotationShell::StateClpbrd(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;
    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( &rView.GetEditWin() ) );
    bool bPastePossible = ( aDataHelper.HasFormat( SOT_FORMAT_STRING ) || aDataHelper.HasFormat( SOT_FORMAT_RTF ) );
    bPastePossible = bPastePossible &&  (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED);

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_CUT:
            {
                if ( (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED) || !pOLV->HasSelection() )
                    rSet.DisableItem( nWhich );
            }
            case SID_COPY:
            {
                if( !pOLV->HasSelection() )
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_PASTE:
            case SID_PASTE_SPECIAL:
                {
                    if( !bPastePossible )
                        rSet.DisableItem( nWhich );
                    break;
                }
            case SID_CLIPBOARD_FORMAT_ITEMS:
                {
                    if ( bPastePossible )
                    {
                        SvxClipboardFmtItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                        if ( aDataHelper.HasFormat( SOT_FORMAT_RTF ) )
                            aFormats.AddClipbrdFormat( SOT_FORMAT_RTF );
                        //if ( aDataHelper.HasFormat( SOT_FORMAT_STRING ) )
                            aFormats.AddClipbrdFormat( SOT_FORMAT_STRING );
                        rSet.Put( aFormats );
                    }
                    else
                        rSet.DisableItem( nWhich );
                    break;
                }
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateStatusLine(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        switch( nWhich )
        {
            case FN_STAT_SELMODE:
            {
                rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 0));
                rSet.DisableItem( nWhich );
                break;
            }
            case FN_STAT_TEMPLATE:
            {
                rSet.DisableItem( nWhich );
                break;
            }
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateInsert(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
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

                    sal_uInt16 nHtmlMode = ::GetHtmlMode(rView.GetDocShell());
                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;
        }

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}


void SwAnnotationShell::NoteExec(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr )
        return;

    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case FN_REPLY:
        case FN_POSTIT:
        case FN_DELETE_COMMENT:
            if ( pPostItMgr->HasActiveSidebarWin() )
                pPostItMgr->GetActiveSidebarWin()->ExecuteCommand(nSlot);
            break;
        case FN_DELETE_ALL_NOTES:
            pPostItMgr->Delete();
            break;
        case FN_DELETE_NOTE_AUTHOR:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, nSlot, sal_False);
            if ( pItem )
                pPostItMgr->Delete( pItem->GetValue() );
            break;
        }
        case FN_HIDE_NOTE:
            /*
            if ( Mgr()->GetActiveSidebarWin() == this )
            {
                Mgr()->SetActivePostIt(0);
                // put the cursor back into the document
                SwitchToFieldPos();
            }
            Mgr()->Hide(mpFld);
            */
            break;
        case FN_HIDE_ALL_NOTES:
            pPostItMgr->Hide();
            break;
        case FN_HIDE_NOTE_AUTHOR:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, nSlot, sal_False);
            if ( pItem )
                pPostItMgr->Hide( pItem->GetValue() );
        }
    }
}

void SwAnnotationShell::GetNoteState(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
            case FN_POSTIT:
            case FN_DELETE_NOTE_AUTHOR:
            case FN_DELETE_ALL_NOTES:
            case FN_HIDE_NOTE:
            case FN_HIDE_NOTE_AUTHOR:
            case FN_HIDE_ALL_NOTES:
            case FN_DELETE_COMMENT:
            {
                if ( !pPostItMgr ||
                     !pPostItMgr->HasActiveAnnotationWin() )
                {
                    rSet.DisableItem(nWhich);
                }
                break;
            }
            case FN_REPLY:
            {
                if ( !pPostItMgr ||
                     !pPostItMgr->HasActiveAnnotationWin() )
                {
                    rSet.DisableItem(nWhich);
                }
                else
                {
                    SvtUserOptions aUserOpt;
                    String sAuthor;
                    if( !(sAuthor = aUserOpt.GetFullName()).Len())
                            if( !(sAuthor = aUserOpt.GetID()).Len() )
                        sAuthor = String( SW_RES( STR_REDLINE_UNKNOWN_AUTHOR ));
                    if (sAuthor == pPostItMgr->GetActiveSidebarWin()->GetAuthor())
                        rSet.DisableItem(nWhich);
                }
                break;
            }
            default:
                rSet.InvalidateItem( nWhich );
                break;
        }

        if (pPostItMgr->HasActiveSidebarWin())
        {
            if ( (pPostItMgr->GetActiveSidebarWin()->IsProtected()) &&
                    ( (nSlotId==FN_DELETE_COMMENT) || (nSlotId==FN_REPLY) ) )
                rSet.DisableItem( nWhich );
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecLingu(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    sal_uInt16 nSlot = rReq.GetSlot();
    SwWrtShell &rSh = rView.GetWrtShell();
    bool bRestoreSelection = false;
    ESelection aOldSelection;

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

            bRestoreSelection = SwLangHelper::SetLanguageStatus(pOLV,rReq,rView,rSh);
            break;
        }
        case SID_THES:
        {
            String aReplaceText;
            SFX_REQUEST_ARG( rReq, pItem2, SfxStringItem, SID_THES, sal_False );
            if (pItem2)
                aReplaceText = pItem2->GetValue();
            if (aReplaceText.Len() > 0)
                ReplaceTextWithSynonym( pOLV->GetEditView(), aReplaceText );
            break;
        }
        case SID_THESAURUS:
        {
            pOLV->StartThesaurus();
            break;
        }
        case SID_HANGUL_HANJA_CONVERSION:
            pOLV->StartTextConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, NULL,
                    i18n::TextConversionOption::CHARACTER_BY_CHARACTER, sal_True, sal_False );
            break;

        case SID_CHINESE_CONVERSION:
        {
                //open ChineseTranslationDialog
                Reference< XComponentContext > xContext(
                    ::cppu::defaultBootstrap_InitialComponentContext() ); //@todo get context from calc if that has one
                if(xContext.is())
                {
                    Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
                    if(xMCF.is())
                    {
                        Reference< ui::dialogs::XExecutableDialog > xDialog(
                                xMCF->createInstanceWithContext(
                                    rtl::OUString::createFromAscii("com.sun.star.linguistic2.ChineseTranslationDialog")
                                    , xContext), UNO_QUERY);
                        Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                        if( xInit.is() )
                        {
                            //  initialize dialog
                            Reference< awt::XWindow > xDialogParentWindow(0);
                            Sequence<Any> aSeq(1);
                            Any* pArray = aSeq.getArray();
                            PropertyValue aParam;
                            aParam.Name = rtl::OUString::createFromAscii("ParentWindow");
                            aParam.Value <<= makeAny(xDialogParentWindow);
                            pArray[0] <<= makeAny(aParam);
                            xInit->initialize( aSeq );

                            //execute dialog
                            sal_Int16 nDialogRet = xDialog->execute();
                            if( RET_OK == nDialogRet )
                            {
                                //get some parameters from the dialog
                                sal_Bool bToSimplified = sal_True;
                                sal_Bool bUseVariants = sal_True;
                                sal_Bool bCommonTerms = sal_True;
                                Reference< beans::XPropertySet >  xProp( xDialog, UNO_QUERY );
                                if( xProp.is() )
                                {
                                    try
                                    {
                                        xProp->getPropertyValue( C2U("IsDirectionToSimplified") ) >>= bToSimplified;
                                        xProp->getPropertyValue( C2U("IsUseCharacterVariants") ) >>= bUseVariants;
                                        xProp->getPropertyValue( C2U("IsTranslateCommonTerms") ) >>= bCommonTerms;
                                    }
                                    catch( Exception& )
                                    {
                                    }
                                }

                                //execute translation
                                sal_Int16 nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                                sal_Int16 nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                                sal_Int32 nOptions    = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                                if( !bCommonTerms )
                                    nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                                Font aTargetFont = pOLV->GetWindow()->GetDefaultFont( DEFAULTFONT_CJK_TEXT,
                                            nTargetLang, DEFAULTFONT_FLAGS_ONLYONE );

                                pOLV->StartTextConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, sal_False, sal_False );
                            }
                        }
                        Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY );
                        if( xComponent.is() )
                            xComponent->dispose();
                    }
                }
            }
            break;
    }

    if (bRestoreSelection)
    {
        // restore selection
        pOLV->GetEditView().SetSelection( aOldSelection );
    }
}

void SwAnnotationShell::GetLinguState(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        switch (nWhich)
        {
            case SID_LANGUAGE_STATUS:
            {
                SwLangHelper::GetLanguageStatus(pOLV,rSet);
                break;
            }

            case SID_THES:
            {
                String          aStatusVal;
                LanguageType    nLang = LANGUAGE_NONE;
                bool bIsLookUpWord = GetStatusValueForThesaurusFromContext( aStatusVal, nLang, pOLV->GetEditView() );
                rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

                // disable "Thesaurus" context menu entry if there is nothing to look up
                uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                lang::Locale aLocale( SvxCreateLocale( nLang ) );
                if (!bIsLookUpWord ||
                    !xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( aLocale ))
                    rSet.DisableItem( SID_THES );
                break;
            }

            // disable "Thesaurus" if the language is not supported
            case SID_THESAURUS:
            {
                const SfxPoolItem &rItem = rView.GetWrtShell().GetDoc()->GetDefault(
                            GetWhichOfScript( RES_CHRATR_LANGUAGE,
                            GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage())) );
                LanguageType nLang = ((const SvxLanguageItem &)
                                                        rItem).GetLanguage();
                uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                if (!xThes.is() || nLang == LANGUAGE_NONE ||
                    !xThes->hasLocale( SvxCreateLocale( nLang ) ))
                    rSet.DisableItem( SID_THESAURUS );
            }
            break;
            case SID_HANGUL_HANJA_CONVERSION:
            case SID_CHINESE_CONVERSION:
            {
                if (!SvtCJKOptions().IsAnyEnabled())
                {
                    rView.GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_False );
                    rSet.DisableItem(nWhich);
                }
                else
                    rView.GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_True );
            }
            break;
        }

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecTransliteration(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();

    using namespace ::com::sun::star::i18n;
    {
        sal_uInt32 nMode = 0;

        switch( rReq.GetSlot() )
        {
            case SID_TRANSLITERATE_SENTENCE_CASE:
                nMode = TransliterationModulesExtra::SENTENCE_CASE;
                break;
            case SID_TRANSLITERATE_TITLE_CASE:
                nMode = TransliterationModulesExtra::TITLE_CASE;
                break;
            case SID_TRANSLITERATE_TOGGLE_CASE:
                nMode = TransliterationModulesExtra::TOGGLE_CASE;
                break;
            case SID_TRANSLITERATE_UPPER:
                nMode = TransliterationModules_LOWERCASE_UPPERCASE;
                break;
            case SID_TRANSLITERATE_LOWER:
                nMode = TransliterationModules_UPPERCASE_LOWERCASE;
                break;
            case SID_TRANSLITERATE_HALFWIDTH:
                nMode = TransliterationModules_FULLWIDTH_HALFWIDTH;
                break;
            case SID_TRANSLITERATE_FULLWIDTH:
                nMode = TransliterationModules_HALFWIDTH_FULLWIDTH;
                break;
            case SID_TRANSLITERATE_HIRAGANA:
                nMode = TransliterationModules_KATAKANA_HIRAGANA;
                break;
            case SID_TRANSLITERATE_KATAGANA:
                nMode = TransliterationModules_HIRAGANA_KATAKANA;
                break;

            default:
                ASSERT(!this, "falscher Dispatcher");
        }

        if( nMode )
        {
            pOLV->TransliterateText( nMode );
        }
    }
}

void SwAnnotationShell::ExecUndo(SfxRequest &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    ::svl::IUndoManager* pUndoManager = GetUndoManager();
    SwWrtShell &rSh = rView.GetWrtShell();

    long aOldHeight = rView.GetPostItMgr()->HasActiveSidebarWin()
                      ? rView.GetPostItMgr()->GetActiveSidebarWin()->GetPostItTextHeight()
                      : 0;

    sal_uInt16 nId = rReq.GetSlot();
    sal_uInt16 nCnt = 1;
    const SfxPoolItem* pItem=0;
    if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nId, sal_False, &pItem ) )
        nCnt = ((SfxUInt16Item*)pItem)->GetValue();
    switch( nId )
    {
        case SID_UNDO:
        {
            if ( pUndoManager )
            {
                sal_uInt16 nCount = pUndoManager->GetUndoActionCount();
                sal_uInt16 nSteps = nCnt;
                if ( nCount < nCnt )
                {
                    nCnt = nCnt - nCount;
                    nSteps = nCount;
                }
                else
                    nCnt = 0;

                while( nSteps-- )
                    pUndoManager->Undo();
            }

            if ( nCnt )
                rSh.Do( SwWrtShell::UNDO, nCnt );

            break;
        }

        case SID_REDO:
        {
            if ( pUndoManager )
            {
                sal_uInt16 nCount = pUndoManager->GetRedoActionCount();
                sal_uInt16 nSteps = nCnt;
                if ( nCount < nCnt )
                {
                    nCnt = nCnt - nCount;
                    nSteps = nCount;
                }
                else
                    nCnt = 0;

                while( nSteps-- )
                    pUndoManager->Redo();
            }

            if ( nCnt )
                rSh.Do( SwWrtShell::REDO, nCnt );

            break;
        }
    }

    rView.GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    if (rView.GetPostItMgr()->HasActiveSidebarWin())
        rView.GetPostItMgr()->GetActiveSidebarWin()->ResizeIfNeccessary(aOldHeight,rView.GetPostItMgr()->GetActiveSidebarWin()->GetPostItTextHeight());
}

void SwAnnotationShell::StateUndo(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    ::svl::IUndoManager* pUndoManager = GetUndoManager();
    SfxViewFrame *pSfxViewFrame = rView.GetViewFrame();
    SwWrtShell &rSh = rView.GetWrtShell();

    while( nWhich )
    {
        switch ( nWhich )
        {
        case SID_UNDO:
            {
                sal_uInt16 nCount = pUndoManager->GetUndoActionCount();
                if ( nCount )
                    pSfxViewFrame->GetSlotState( nWhich, pSfxViewFrame->GetInterface(), &rSet );
                else if (rSh.GetLastUndoInfo(0, 0))
                {
                    rSet.Put( SfxStringItem( nWhich, rSh.GetDoString(SwWrtShell::UNDO)) );
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
        case SID_REDO:
            {
                sal_uInt16 nCount = pUndoManager->GetRedoActionCount();
                if ( nCount )
                    pSfxViewFrame->GetSlotState( nWhich, pSfxViewFrame->GetInterface(), &rSet );
                else if (rSh.GetFirstRedoInfo(0))
                {
                    rSet.Put(SfxStringItem( nWhich, rSh.GetDoString(SwWrtShell::REDO)) );
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
        case SID_GETUNDOSTRINGS:
        case SID_GETREDOSTRINGS:
            {
                if( pUndoManager )
                {
                    UniString (::svl::IUndoManager:: *fnGetComment)( size_t, bool const ) const;

                    sal_uInt16 nCount;
                    if( SID_GETUNDOSTRINGS == nWhich )
                    {
                        nCount = pUndoManager->GetUndoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetUndoActionComment;
                    }
                    else
                    {
                        nCount = pUndoManager->GetRedoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetRedoActionComment;
                    }

                    String sList;
                    if( nCount )
                    {
                        for( sal_uInt16 n = 0; n < nCount; ++n )
                            ( sList += (pUndoManager->*fnGetComment)( n, ::svl::IUndoManager::TopLevel ) )
                                    += '\n';
                    }

                    SfxStringListItem aItem( nWhich );
                    if ((nWhich == SID_GETUNDOSTRINGS) &&
                        rSh.GetLastUndoInfo(0, 0))
                    {
                        rSh.GetDoStrings( SwWrtShell::UNDO, aItem );
                    }
                    else if ((nWhich == SID_GETREDOSTRINGS) &&
                             (rSh.GetFirstRedoInfo(0)))
                    {
                        rSh.GetDoStrings( SwWrtShell::UNDO, aItem );
                    }

                    sList += aItem.GetString();
                    aItem.SetString( sList );
                    rSet.Put( aItem );
                }
                else
                    rSet.DisableItem( nWhich );
            }
            break;

        default:
            {
                pSfxViewFrame->GetSlotState( nWhich, pSfxViewFrame->GetInterface(), &rSet );
                break;
            }

        }

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::InsertSymbol(SfxRequest& rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    if( pArgs )
        pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), sal_False, &pItem);

    String sSym;
    String sFontName;
    if ( pItem )
    {
        sSym = ((const SfxStringItem*)pItem)->GetValue();
        const SfxPoolItem* pFtItem = NULL;
        pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), sal_False, &pFtItem);
        const SfxStringItem* pFontItem = PTR_CAST( SfxStringItem, pFtItem );
        if ( pFontItem )
            sFontName = pFontItem->GetValue();
    }

    SfxItemSet aSet(pOLV->GetAttribs());
    sal_uInt16 nScript = pOLV->GetSelectedScriptType();
    SvxFontItem aSetDlgFont( RES_CHRATR_FONT );
    {
        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, sal_False );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
            aSetDlgFont = *(SvxFontItem*)pI;
        else
            aSetDlgFont = (SvxFontItem&)aSet.Get( GetWhichOfScript(
                        SID_ATTR_CHAR_FONT,
                        GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() ) ));
        if (!sFontName.Len())
            sFontName = aSetDlgFont.GetFamilyName();
    }

    Font aFont(sFontName, Size(1,1));
    if( !sSym.Len() )
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

        SfxAllItemSet aAllSet( GetPool() );
        aAllSet.Put( SfxBoolItem( FN_PARAM_1, sal_False ) );

        SwViewOption aOpt(*rView.GetWrtShell().GetViewOptions());
        String sSymbolFont = aOpt.GetSymbolFont();
        if( sSymbolFont.Len() )
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, sSymbolFont ) );
        else
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, aSetDlgFont.GetFamilyName() ) );

        // Wenn Zeichen selektiert ist kann es angezeigt werden
        SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( rView.GetWindow(), aAllSet,
            rView.GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP );

        sal_uInt16 nResult = pDlg->Execute();
        if( nResult == RET_OK )
        {
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pCItem, SfxStringItem, SID_CHARMAP, sal_False );
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFontItem, SvxFontItem, SID_ATTR_CHAR_FONT, sal_False );
            if ( pFontItem )
            {
                aFont.SetName( pFontItem->GetFamilyName() );
                aFont.SetStyleName( pFontItem->GetStyleName() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
            }

            if ( pCItem )
            {
                sSym  = pCItem->GetValue();
                aOpt.SetSymbolFont(aFont.GetName());
                SW_MOD()->ApplyUsrPref(aOpt, &rView);
            }
        }

        delete( pDlg );
    }

    if( sSym.Len() )
    {
        // nicht flackern
        pOLV->HideCursor();
        Outliner * pOutliner = pOLV->GetOutliner();
        pOutliner->SetUpdateMode(sal_False);

        SfxItemSet aOldSet( pOLV->GetAttribs() );
        SfxItemSet aFontSet( *aOldSet.GetPool(),
                            EE_CHAR_FONTINFO, EE_CHAR_FONTINFO,
                            EE_CHAR_FONTINFO_CJK, EE_CHAR_FONTINFO_CJK,
                            EE_CHAR_FONTINFO_CTL, EE_CHAR_FONTINFO_CTL,
                            0 );
        aFontSet.Set( aOldSet );

        // String einfuegen
        pOLV->InsertText( sSym);

        // attributieren (Font setzen)
        SfxItemSet aSetFont( *aFontSet.GetPool(), aFontSet.GetRanges() );
        SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
                                aFont.GetStyleName(), aFont.GetPitch(),
                                aFont.GetCharSet(),
                                EE_CHAR_FONTINFO );
        sal_uInt16 nScriptBreak = pBreakIt->GetAllScriptsOfText( sSym );
        if( SCRIPTTYPE_LATIN & nScriptBreak )
            aSetFont.Put( aFontItem, EE_CHAR_FONTINFO );
        if( SCRIPTTYPE_ASIAN & nScriptBreak )
            aSetFont.Put( aFontItem, EE_CHAR_FONTINFO_CJK );
        if( SCRIPTTYPE_COMPLEX & nScriptBreak )
            aSetFont.Put( aFontItem, EE_CHAR_FONTINFO_CTL );
        pOLV->SetAttribs(aSetFont);

        // Selektion loeschen
        ESelection aSel(pOLV->GetSelection());
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOLV->SetSelection(aSel);

        // Alten Font restaurieren
        pOLV->SetAttribs( aFontSet );

        // ab jetzt wieder anzeigen
        pOutliner->SetUpdateMode(sal_True);
        pOLV->ShowCursor();

        rReq.AppendItem( SfxStringItem( GetPool().GetWhich(SID_CHARMAP), sSym ) );
        if(aFont.GetName().Len())
            rReq.AppendItem( SfxStringItem( SID_ATTR_SPECIALCHAR, aFont.GetName() ) );
        rReq.Done();
    }
}
