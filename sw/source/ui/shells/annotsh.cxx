/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: annotsh.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:57:51 $
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

#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif
#ifndef _SFXOBJFACE_HXX
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
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
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXSLSTITM_HXX //autogen
#include <svtools/slstitm.hxx>
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
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
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
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
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
#ifndef _POPUP_HRC
#include <popup.hrc>
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
#include <initui.hxx>
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
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#include "annotsh.hxx"
#include "view.hxx"
#include "PostItMgr.hxx"
#include "postit.hxx"


#include "cmdid.h"
#include "swtypes.hxx"

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <svx/svxids.hrc>
#include <svtools/itempool.hxx>
#include <svx/outliner.hxx>
#include <svx/editeng.hxx>

#include <docsh.hxx>
#include <svtools/undo.hxx>
#include "swabstdlg.hxx" //CHINA001
#include "chrdlg.hrc" //CHINA001
using namespace ::com::sun::star;

#define SwAnnotationShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif


SFX_IMPL_INTERFACE(SwAnnotationShell, SfxShell, SW_RES(STR_SHELLNAME_DRAW_TEXT))
{
    //SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAW_TEXT_TOOLBOX));
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

SfxUndoManager* SwAnnotationShell::GetUndoManager()
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
    {
        DBG_ASSERT(pPostItMgr,"PostItMgr::Layout(): We are looping forever")
        return 0;
    }
    return &pPostItMgr->GetActivePostIt()->Engine()->GetUndoManager();
}

void SwAnnotationShell::Exec( SfxRequest &rReq )
{
    //TODO: clean this up!!!
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();
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
             FontUnderline eFU = ((const SvxUnderlineItem&)aEditAttr.Get(EE_CHAR_UNDERLINE)).GetUnderline();
            aNewAttr.Put(SvxUnderlineItem(eFU == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_UNDERLINE));
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
                ULONG nParaCount = pOutliner->GetParagraphCount();
                if (nParaCount > 0)
                    pOLV->SelectRange(0L, USHORT(nParaCount) );
            }
            break;
        }
        case FN_FORMAT_RESET:
        {
            pPostItMgr->GetActivePostIt()->ResetAttributes();
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
                pOLV->InsertField(SvxFieldItem(aFld, EE_FEATURE_FIELD));
            }
            break;
        }
        case FN_INSERT_SYMBOL:
            InsertSymbol(rReq);
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
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
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
                    if ( pOLV )
                    {
                        USHORT nScriptType = pOLV->GetSelectedScriptType();
                        SfxItemPool* pSecondPool = aEditAttr.GetPool()->GetSecondaryPool();
                        if( !pSecondPool )
                            pSecondPool = aEditAttr.GetPool();
                        SvxScriptSetItem aSetItem( nSlotId, *pSecondPool );
                        aSetItem.GetItemSet().Put( aEditAttr, FALSE );
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
            case SID_ATTR_CHAR_CONTOUR: nEEWhich = EE_CHAR_OUTLINE; break;
            case SID_ATTR_CHAR_SHADOWED:  nEEWhich = EE_CHAR_SHADOW;break;
            case SID_ATTR_CHAR_STRIKEOUT: nEEWhich = EE_CHAR_STRIKEOUT;break;
            case FN_SET_SUPER_SCRIPT:
            case FN_SET_SUB_SCRIPT:
            {
                USHORT nEsc = 0;
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
            default:
                rSet.InvalidateItem( nWhich );
                break;
        }


        if(nEEWhich)
            rSet.Put(aEditAttr.Get(nEEWhich, sal_True), nWhich);

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecClpbrd(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    sal_uInt16 nSlot = rReq.GetSlot();
    //sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    //const SfxItemSet *pNewAttrs = rReq.GetArgs();
    //sal_uInt16 nEEWhich = 0;
    switch (nSlot)
    {
        case SID_CUT:
            pOLV->Cut();
            break;
        case SID_COPY:
            pOLV->Copy();
            break;
        case SID_PASTE:
            pOLV->Paste();
            break;
    }
}

void SwAnnotationShell::StateClpbrd(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;
    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();
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
                                &rView.GetEditWin() ) );

                    if( !aDataHelper.GetXTransferable().is() ||
                        !SwTransferable::IsPaste( rView.GetWrtShell(), aDataHelper ))
                        rSet.DisableItem( SID_PASTE );
                }
                break;
            /*
            case FN_PASTESPECIAL:
                {
                }
                break;
            */
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateInsert(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();
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
        nWhich = aIter.NextWhich();
    }
}


void SwAnnotationShell::NoteExec(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case FN_DELETE_NOTE:
        case FN_DELETE_NOTE_AUTHOR:
        case FN_DELETE_ALL_NOTES:
            {
                pPostItMgr->GetActivePostIt()->Delete(nSlot);
                break;
            }
        case FN_HIDE_NOTE:
        case FN_HIDE_NOTE_AUTHOR:
        case FN_HIDE_ALL_NOTES:
            {
                pPostItMgr->GetActivePostIt()->Hide(nSlot);
                break;
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
        USHORT nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
            case FN_DELETE_NOTE:
            case FN_DELETE_NOTE_AUTHOR:
            case FN_DELETE_ALL_NOTES:
            case FN_HIDE_NOTE:
            case FN_HIDE_NOTE_AUTHOR:
            case FN_HIDE_ALL_NOTES:
                {
                    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
                        rSet.InvalidateItem( nWhich );
                }
                break;
            default:
                rSet.InvalidateItem( nWhich );
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecLingu(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case FN_THESAURUS_DLG:
        {
            pOLV->StartThesaurus();
            break;
        }
    }
}

void SwAnnotationShell::GetLinguState(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {

        switch (nWhich)
        {
            // disable "Thesaurus" if the language is not supported
            case FN_THESAURUS_DLG:
            {
                const SfxPoolItem &rItem = rView.GetWrtShell().GetDoc()->GetDefault(
                            GetWhichOfScript( RES_CHRATR_LANGUAGE,
                            GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage())) );
            LanguageType nLang = ((const SvxLanguageItem &)
                                                    rItem).GetLanguage();
            uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
            if (!xThes.is() || nLang == LANGUAGE_NONE ||
                !xThes->hasLocale( SvxCreateLocale( nLang ) ))
                rSet.DisableItem( FN_THESAURUS_DLG );
            }
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecTransliteration(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();

    using namespace ::com::sun::star::i18n;
    {
        sal_uInt32 nMode = 0;

        switch( rReq.GetSlot() )
        {
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
    SfxUndoManager* pUndoManager = GetUndoManager();
    SwWrtShell &rSh = rView.GetWrtShell();

    USHORT nId = rReq.GetSlot();
    sal_uInt16 nCnt = 1;
    const SfxPoolItem* pItem=0;
    if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nId, FALSE, &pItem ) )
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
                    pUndoManager->Undo(0);
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
                    pUndoManager->Redo(0);
            }

            if ( nCnt )
                rSh.Do( SwWrtShell::REDO, nCnt );

            break;
        }
    }
}

void SwAnnotationShell::StateUndo(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    SfxUndoManager* pUndoManager = GetUndoManager();
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
                else if( rSh.GetUndoIds() )
                    rSet.Put( SfxStringItem( nWhich, rSh.GetDoString(SwWrtShell::UNDO)) );
                else
                    rSet.DisableItem(nWhich);
                break;
            }
        case SID_REDO:
            {
                sal_uInt16 nCount = pUndoManager->GetRedoActionCount();
                if ( nCount )
                    pSfxViewFrame->GetSlotState( nWhich, pSfxViewFrame->GetInterface(), &rSet );
                else if(rSh.GetRedoIds())
                    rSet.Put(SfxStringItem( nWhich, rSh.GetDoString(SwWrtShell::REDO)) );
                else
                    rSet.DisableItem(nWhich);
                break;
            }
        case SID_GETUNDOSTRINGS:
        case SID_GETREDOSTRINGS:
            {
                if( pUndoManager )
                {
                    UniString (SfxUndoManager:: *fnGetComment)( USHORT ) const;

                    sal_uInt16 nCount;
                    if( SID_GETUNDOSTRINGS == nWhich )
                    {
                        nCount = pUndoManager->GetUndoActionCount();
                        fnGetComment = &SfxUndoManager::GetUndoActionComment;
                    }
                    else
                    {
                        nCount = pUndoManager->GetRedoActionCount();
                        fnGetComment = &SfxUndoManager::GetRedoActionComment;
                    }

                    String sList;
                    if( nCount )
                    {
                        for( sal_uInt16 n = 0; n < nCount; ++n )
                            ( sList += (pUndoManager->*fnGetComment)( n ) )
                                    += '\n';
                    }

                    SfxStringListItem aItem( nWhich );
                    if( nWhich == SID_GETUNDOSTRINGS && rSh.GetUndoIds() )
                    {
                        rSh.GetDoStrings( SwWrtShell::UNDO, aItem );
                    }
                    else if( nWhich == SID_GETREDOSTRINGS && rSh.GetRedoIds() )
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
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::InsertSymbol(SfxRequest& rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->GetActivePostIt() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActivePostIt()->View();

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    if( pArgs )
        pArgs->GetItemState(GetPool().GetWhich(FN_INSERT_SYMBOL), FALSE, &pItem);

    String sSym;
    String sFontName;
    if ( pItem )
    {
        sSym = ((const SfxStringItem*)pItem)->GetValue();
        const SfxPoolItem* pFtItem = NULL;
        pArgs->GetItemState( GetPool().GetWhich(FN_PARAM_1), FALSE, &pFtItem);
        const SfxStringItem* pFontItem = PTR_CAST( SfxStringItem, pFtItem );
        if ( pFontItem )
            sFontName = pFontItem->GetValue();
    }

    SfxItemSet aSet(pOLV->GetAttribs());
    USHORT nScript = pOLV->GetSelectedScriptType();
    SvxFontItem aSetDlgFont( RES_CHRATR_FONT );
    {
        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, FALSE );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
            aSetDlgFont = *(SvxFontItem*)pI;
        else
            aSetDlgFont = (SvxFontItem&)aSet.Get( GetWhichOfScript(
                        SID_ATTR_CHAR_FONT,
                        GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() ) ));
    }


    Font aFont(sFontName, Size(1,1));
    if(!sSym.Len())
    {
        //CHINA001 SvxCharacterMap* pDlg = new SvxCharacterMap( NULL, FALSE );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
        AbstractSvxCharacterMap* pDlg = pFact->CreateSvxCharacterMap( NULL, RID_SVXDLG_CHARMAP, FALSE );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001

        Font aDlgFont( pDlg->GetCharFont() );
        SwViewOption aOpt(*rView.GetWrtShell().GetViewOptions());
        String sSymbolFont = aOpt.GetSymbolFont();
        if(sSymbolFont.Len())
            aDlgFont.SetName(sSymbolFont);
        else
            aDlgFont.SetName( aSetDlgFont.GetFamilyName() );

        // Wenn Zeichen selektiert ist kann es angezeigt werden
        pDlg->SetFont( aDlgFont );
        USHORT nResult = pDlg->Execute();
        if( nResult == RET_OK )
        {
            aFont = pDlg->GetCharFont();
            sSym  = pDlg->GetCharacters();
            aOpt.SetSymbolFont(aFont.GetName());
            SW_MOD()->ApplyUsrPref(aOpt, &rView);
        }
        delete( pDlg );
    }

    if( sSym.Len() )
    {
        // nicht flackern
        pOLV->HideCursor();
        Outliner * pOutliner = pOLV->GetOutliner();
        pOutliner->SetUpdateMode(FALSE);

        SfxItemSet aOldSet( pOLV->GetAttribs() );
        SfxItemSet aFontSet( *aOldSet.GetPool(),
                            EE_CHAR_FONTINFO, EE_CHAR_FONTINFO,
                            EE_CHAR_FONTINFO_CJK, EE_CHAR_FONTINFO_CJK,
                            EE_CHAR_FONTINFO_CTL, EE_CHAR_FONTINFO_CTL,
                            0 );
        aFontSet.Set( aOldSet );

        // String einfuegen
        pOLV->InsertText( sSym, TRUE );

        // attributieren (Font setzen)
        SfxItemSet aSetFont( *aFontSet.GetPool(), aFontSet.GetRanges() );
        SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
                                aFont.GetStyleName(), aFont.GetPitch(),
                                aFont.GetCharSet(),
                                EE_CHAR_FONTINFO );
        USHORT nScriptBreak = pBreakIt->GetAllScriptsOfText( sSym );
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
        pOutliner->SetUpdateMode(TRUE);
        pOLV->ShowCursor();

        rReq.AppendItem( SfxStringItem( GetPool().GetWhich(FN_INSERT_SYMBOL), sSym ) );
        if(aFont.GetName().Len())
            rReq.AppendItem( SfxStringItem( FN_PARAM_1, aFont.GetName() ) );
        rReq.Done();
    }
}
