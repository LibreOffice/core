/*************************************************************************
 *
 *  $RCSfile: viewstat.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2000-11-24 17:20:05 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
//#ifndef _TWAIN_HXX //autogen
//#include <svtools/twain.hxx>
//#endif
#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SWMODULE_HXX
#include "swmodule.hxx"
#endif


#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _SWGLOBDOCSH_HXX //autogen
#include <globdoc.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif

using namespace ::com::sun::star;
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwView::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    sal_uInt16 eFrmType = FRMTYPE_NONE;
    int bGetFrmType = sal_False;
    sal_Bool bWeb = 0 != PTR_CAST(SwWebView, this);

    while(nWhich)
    {
        switch(nWhich)
        {
            case FN_EDIT_LINK_DLG:
                if( !pWrtShell->GetLinkManager().GetLinks().Count() )
                    rSet.DisableItem(nWhich);
                else if( pWrtShell->IsSelFrmMode() &&
                            pWrtShell->IsSelObjProtected(FLYPROTECT_CONTENT))
                        rSet.DisableItem(nWhich);
                break;

            case FN_INSERT_CAPTION:
            {
                // Captions gibt's fuer Grafiken, OLE-Objekte, Rahmen und Tabellen
                if( !bGetFrmType )
                    eFrmType = pWrtShell->GetFrmType(0,sal_True), bGetFrmType = sal_True;
                if (! ( ((eFrmType & FRMTYPE_FLY_ANY) && nSelectionType != SwWrtShell::SEL_DRW_TXT)||
                        nSelectionType & SwWrtShell::SEL_TBL ||
                        nSelectionType & SwWrtShell::SEL_DRW) )
                    rSet.DisableItem(nWhich);
                else if( pWrtShell->IsTableMode() )
                    rSet.DisableItem(nWhich);
            }
            break;

            case FN_EDIT_FOOTNOTE:
            {
                if( !pWrtShell->GetCurFtn() )
                    rSet.DisableItem(nWhich);
            }
            break;

            case FN_CHANGE_PAGENUM:
            {
                sal_uInt16 nType = pWrtShell->GetFrmType(0,sal_True);
                if( ( FRMTYPE_FLY_ANY | FRMTYPE_HEADER | FRMTYPE_FOOTER |
                      FRMTYPE_FOOTNOTE | FRMTYPE_DRAWOBJ ) & nType )
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxUInt16Item(nWhich, pWrtShell->GetPageOffset()));
            }
            break;
            case SID_PRINTDOC:
            case SID_PRINTDOCDIRECT:
                GetSlotState( nWhich, SfxViewShell::GetInterface(), &rSet );
            break;
            case SID_ATTR_PAGE:
            case SID_ATTR_PAGE_SIZE:
            case SID_ATTR_PAGE_PAPERBIN:
            case RES_PAPER_BIN:
            case FN_PARAM_FTN_INFO:
            {
                const sal_uInt16 nCurIdx = pWrtShell->GetCurPageDesc();
                const SwPageDesc& rDesc = pWrtShell->GetPageDesc( nCurIdx );
                ::PageDescToItemSet( rDesc, rSet);
            }
            break;
            case RES_BACKGROUND:
            case SID_ATTR_BRUSH:
            {
                const sal_uInt16 nCurIdx = pWrtShell->GetCurPageDesc();
                const SwPageDesc& rDesc = pWrtShell->GetPageDesc( nCurIdx );
                const SwFrmFmt& rMaster = rDesc.GetMaster();
                const SvxBrushItem& rBrush = (const SvxBrushItem&)
                                    rMaster.GetAttr(RES_BACKGROUND, sal_True);
                rSet.Put(rBrush);
            }
            break;
            case SID_CLEARHISTORY:
            {
                rSet.Put(SfxBoolItem(nWhich, pWrtShell->GetUndoIds() != 0));
            }
            break;
            case SID_UNDO:
            {
                //JP 21.07.98: Bug 53429 - die muss noch nicht vorhanden sein
                //              also lasse sie mal anlegen:
                if( !pShell )
                    SelectShell();

                const SfxPoolItem* pState = pShell->GetSlotState(SID_UNDO);
                if(pState)
                    rSet.Put(*pState);
                else
                    rSet.DisableItem(nWhich);
            }
            break;
            case FN_INSERT_CTRL:
                rSet.Put(SfxUInt16Item(nWhich,
                    bWeb ? SwView::nWebInsertCtrlState : SwView::nInsertCtrlState));
            break;
            case FN_INSERT_OBJ_CTRL:
            if(bWeb)
                rSet.DisableItem(nWhich);
            else
                rSet.Put(SfxUInt16Item(nWhich, SwView::nInsertObjectCtrlState));
            break;
            case FN_UPDATE_TOX:
                if(!pWrtShell->GetTOXCount())
                    rSet.DisableItem(nWhich);
            break;
            case FN_EDIT_CURRENT_TOX:
            case FN_UPDATE_CUR_TOX:
                if(!pWrtShell->GetCurTOX())
                    rSet.DisableItem(nWhich);
            break;
            case SID_TWAIN_SELECT:
            case SID_TWAIN_TRANSFER:
#if defined WIN || defined WNT || defined UNX
            {
                if(!SW_MOD()->GetScannerManager().is())
                    rSet.DisableItem(nWhich);
            }
#endif
            break;
            case RES_PARATR_TABSTOP:
            case SID_ATTR_DEFTABSTOP:
            {
                const SvxTabStopItem& rDefTabs =
                    (const SvxTabStopItem&)pWrtShell->
                                        GetDefault(RES_PARATR_TABSTOP);
                rSet.Put( SfxUInt16Item( nWhich,
                                                (sal_uInt16)::GetTabDist(rDefTabs)));
            }
            break;
            case SID_ATTR_LANGUAGE:
            {
                rSet.Put((const SvxLanguageItem&)
                    pWrtShell->GetDefault(RES_CHRATR_LANGUAGE), SID_ATTR_LANGUAGE);
            }
            break;
            case RES_CHRATR_CJK_LANGUAGE:
                rSet.Put((const SvxLanguageItem&)
                    pWrtShell->GetDefault(RES_CHRATR_CJK_LANGUAGE), RES_CHRATR_CJK_LANGUAGE);
            break;
            case RES_CHRATR_CTL_LANGUAGE:
                rSet.Put((const SvxLanguageItem&)
                    pWrtShell->GetDefault(RES_CHRATR_CTL_LANGUAGE), RES_CHRATR_CTL_LANGUAGE);
            break;
            case FN_INSERT_FOOTER:
            case FN_INSERT_HEADER:
                if(!pWrtShell->IsBrowseMode())
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxBoolItem(nWhich,
                        nWhich == FN_INSERT_HEADER ?
                            pWrtShell->IsHeadInBrowse() :
                                pWrtShell->IsFootInBrowse()));
            break;
            case FN_HYPHENATE_OPT_DLG:
            {
                sal_Bool bCheck = sal_False;

                if (pWrtShell->GetSelectionType() & (SwWrtShell::SEL_DRW_TXT|SwWrtShell::SEL_DRW))
                    bCheck = IsDrawTextHyphenate();
                rSet.Put(SfxBoolItem(nWhich, bCheck));
            }
            break;
        case FN_REDLINE_ON:
            if( pWrtShell->IsInsMode() )
            {
                rSet.Put( SfxBoolItem( nWhich,
                    (pWrtShell->GetRedlineMode() & REDLINE_ON) != 0));
            }
            else
                rSet.DisableItem( nWhich );
            break;
        case FN_REDLINE_SHOW:
            {
                sal_uInt16 nMask = REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE;
                rSet.Put( SfxBoolItem( nWhich,
                    (pWrtShell->GetRedlineMode() & nMask) == nMask ));
            }
            break;
        case SID_GALLERY :
        case FN_REDLINE_ACCEPT:
            {
                SfxViewFrame* pVFrame = GetViewFrame();
                if (pVFrame->KnowsChildWindow(nWhich))
                    rSet.Put(SfxBoolItem( nWhich, pVFrame->HasChildWindow(nWhich)));
                else
                    rSet.DisableItem(nWhich);
            }
            break;
        case FN_THESAURUS_DLG:
            {
                SwWrtShell  &rSh = GetWrtShell();
                if (2 <= rSh.GetCrsrCnt())  // multi selection?
                    rSet.DisableItem(nWhich);
                else
                {
                    SfxItemPool &rPool = GetPool();
                    SfxItemSet aCoreSet(rPool, RES_CHRATR_LANGUAGE,
                                               RES_CHRATR_LANGUAGE);
                    rSh.GetAttr(aCoreSet);

                    const SfxPoolItem* pItem = 0;
                    SfxItemState eState = aCoreSet.GetItemState(
                            RES_CHRATR_LANGUAGE, sal_False, &pItem);

                    LanguageType nLang = LANGUAGE_NONE;
                    if( SFX_ITEM_DEFAULT == eState )  // provide default if not found
                        pItem = &rPool.GetDefaultItem(RES_CHRATR_LANGUAGE);
                    if( eState >= SFX_ITEM_DEFAULT )
                        nLang = ((SvxLanguageItem* )pItem)->GetLanguage();

                    // disable "Thesaurus" (menu entry and key shortcut) if the
                    // language is not supported (by default it is enabled)
                    uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                    if (!xThes.is() || nLang == LANGUAGE_NONE ||
                        !xThes->hasLocale( SvxCreateLocale( nLang ) ))
                        rSet.DisableItem(nWhich);
                }
            }
            break;
            case SID_MAIL_SCROLLBODY_PAGEDOWN:
                {
                    long nBottom = pWrtShell->GetDocSize().Height() + DOCUMENTBORDER;
                    long nAct = GetVisArea().Bottom();
                    rSet.Put(SfxBoolItem(SID_MAIL_SCROLLBODY_PAGEDOWN, nAct < nBottom ));
                }
                break;

            case SID_DOCUMENT_COMPARE:
            case SID_DOCUMENT_MERGE:
                if( GetDocShell()->IsA( SwGlobalDocShell::StaticType() ) ||
                    pWrtShell->IsAnySectionInDoc( sal_True, sal_True, sal_True ))
                    rSet.DisableItem(nWhich);
            break;
        }
        nWhich = aIter.NextWhich();
    }
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwView::GetDrawState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_Bool bWeb = 0 != PTR_CAST(SwWebView, this);

    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_INSERT_DRAW:
            {
                if(bWeb)
                    rSet.DisableItem(nWhich);
                else
                {
                    SfxAllEnumItem aEnum(SID_INSERT_DRAW, nDrawSfxId);
                    rSet.Put(aEnum);
                }
            }
            break;

            case SID_SHOW_HIDDEN:
            case SID_SHOW_FORMS:
                rSet.DisableItem( nWhich );
                // rSet.Put( SfxBoolItem(nWhich,sal_True ));
                break;

            case SID_DRAW_TEXT_MARQUEE:
                if (::GetHtmlMode(GetDocShell()) & HTMLMODE_SOME_STYLES)
                    rSet.Put( SfxBoolItem(nWhich, nDrawSfxId == nWhich));
                else
                    rSet.DisableItem(nWhich);
                break;
            case SID_OBJECT_SELECT:
                rSet.Put( SfxBoolItem(nWhich, nDrawSfxId == nWhich || nFormSfxId == nWhich));
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SwView::HasUIFeature( sal_uInt32 nFeature )
{
    return pWrtShell->IsLabelDoc();
}

