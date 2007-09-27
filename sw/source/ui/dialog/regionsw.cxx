/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regionsw.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:37:34 $
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
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <svtools/PasswordHelper.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _PASSWD_HXX //autogen
#include <sfx2/passwd.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _LINKMGR_HXX
#include <sfx2/linkmgr.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen

#include <svx/sizeitem.hxx>
#endif

#include <svx/htmlcfg.hxx>

#ifndef _BOOKMRK_HXX //autogen
#include <bookmrk.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _REGIONSW_HXX
#include <regionsw.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>                   // fuer Undo-Ids
#endif
#ifndef _COLUMN_HXX
#include <column.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _REGIONSW_HRC
#include <regionsw.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <svx/dlgutil.hxx>
#endif
#include "swabstdlg.hxx"

/*--------------------------------------------------------------------
    Beschreibung:   Bereiche einfuegen
 --------------------------------------------------------------------*/


void SwBaseShell::InsertRegionDialog(SfxRequest& rReq)
{
    SwWrtShell& rSh = GetShell();
    const SfxItemSet *pSet = rReq.GetArgs();

    SfxItemSet aSet(GetPool(),
            RES_COL, RES_COL,
            RES_LR_SPACE, RES_LR_SPACE,
            RES_COLUMNBALANCE, RES_FRAMEDIR,
            RES_BACKGROUND, RES_BACKGROUND,
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_FTN_AT_TXTEND, RES_END_AT_TXTEND,
            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
            0);

    if (!pSet || pSet->Count()==0)
    {
        SwRect aRect;
        rSh.CalcBoundRect(aRect, FLY_IN_CNTNT);

        long nWidth = aRect.Width();
        aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));

        // Hoehe=Breite fuer konsistentere Vorschau (analog zu Bereich bearbeiten)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog( DLG_INSERT_SECTION,
                                                        &GetView().GetViewFrame()->GetWindow(), aSet , rSh);
        DBG_ASSERT(aTabDlg, "Dialogdiet fail!");
        aTabDlg->Execute();
        rReq.Ignore();
        delete aTabDlg;
    }
    else
    {
        const SfxPoolItem *pItem = 0;
        String aTmpStr;
        if ( SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_REGION_NAME, TRUE, &pItem) )
            aTmpStr = rSh.GetUniqueSectionName(
                    &((const SfxStringItem *)pItem)->GetValue() );
        else
            aTmpStr = rSh.GetUniqueSectionName();

        SwSection   aSection(CONTENT_SECTION,aTmpStr);
        rReq.SetReturnValue(SfxStringItem(FN_INSERT_REGION, aTmpStr));

        aSet.Put( *pSet );
        if(SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_COLUMNS, FALSE, &pItem)||
            SFX_ITEM_SET == pSet->GetItemState(FN_INSERT_REGION, FALSE, &pItem))
        {
            SwFmtCol aCol;
            SwRect aRect;
            rSh.CalcBoundRect(aRect, FLY_IN_CNTNT);
            long nWidth = aRect.Width();

            USHORT nCol = ((SfxUInt16Item *)pItem)->GetValue();
            if(nCol)
            {
                aCol.Init( nCol, 0, static_cast< USHORT >(nWidth) );
                aSet.Put(aCol);
            }
        }
        else if(SFX_ITEM_SET == pSet->GetItemState(RES_COL, FALSE, &pItem))
        {
            aSet.Put(*pItem);
        }

        const BOOL bHidden = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_HIDDEN, TRUE, &pItem)?
            (BOOL)((const SfxBoolItem *)pItem)->GetValue():FALSE;
        const BOOL bProtect = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_PROTECT, TRUE, &pItem)?
            (BOOL)((const SfxBoolItem *)pItem)->GetValue():FALSE;
        // --> FME 2004-06-22 #114856# edit in readonly sections
        const BOOL bEditInReadonly = SFX_ITEM_SET ==
            pSet->GetItemState(FN_PARAM_REGION_EDIT_IN_READONLY, TRUE, &pItem)?
            (BOOL)((const SfxBoolItem *)pItem)->GetValue():FALSE;
        // <--

        aSection.SetProtect(bProtect);
        aSection.SetHidden(bHidden);
        // --> FME 2004-06-22 #114856# edit in readonly sections
        aSection.SetEditInReadonly(bEditInReadonly);
        // <--

        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_REGION_CONDITION, TRUE, &pItem))
            aSection.SetCondition(((const SfxStringItem *)pItem)->GetValue());

        String aFile, aSub;
        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_1, TRUE, &pItem))
            aFile = ((const SfxStringItem *)pItem)->GetValue();

        if(SFX_ITEM_SET ==
                pSet->GetItemState(FN_PARAM_3, TRUE, &pItem))
            aSub = ((const SfxStringItem *)pItem)->GetValue();


        if(aFile.Len() || aSub.Len())
        {
            String sLinkFileName(sfx2::cTokenSeperator);
            sLinkFileName += sfx2::cTokenSeperator;
            sLinkFileName.SetToken(0, sfx2::cTokenSeperator,aFile);

            if(SFX_ITEM_SET ==
                    pSet->GetItemState(FN_PARAM_2, TRUE, &pItem))
                sLinkFileName.SetToken(1, sfx2::cTokenSeperator,
                    ((const SfxStringItem *)pItem)->GetValue());

            sLinkFileName += aSub;
            aSection.SetType( FILE_LINK_SECTION );
            aSection.SetLinkFileName(sLinkFileName);
        }
        rSh.InsertSection(aSection, aSet.Count() ? &aSet : 0);
        rReq.Done();
    }
}

IMPL_STATIC_LINK( SwWrtShell, InsertRegionDialog, SwSection*, pSect )
{
    if( pSect )
    {
        SfxItemSet aSet(pThis->GetView().GetPool(),
                RES_COL, RES_COL,
                RES_BACKGROUND, RES_BACKGROUND,
                RES_FRM_SIZE, RES_FRM_SIZE,
                SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                0);
        SwRect aRect;
        pThis->CalcBoundRect(aRect, FLY_IN_CNTNT);
        long nWidth = aRect.Width();
        aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));
        // Hoehe=Breite fuer konsistentere Vorschau (analog zu Bereich bearbeiten)
        aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractInsertSectionTabDialog* aTabDlg = pFact->CreateInsertSectionTabDialog( DLG_INSERT_SECTION,
                                                        &pThis->GetView().GetViewFrame()->GetWindow(),aSet , *pThis);
        DBG_ASSERT(aTabDlg, "Dialogdiet fail!");
        aTabDlg->SetSection(*pSect);
        aTabDlg->Execute();

        delete pSect;
        delete aTabDlg;
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Bereich bearbeiten
 --------------------------------------------------------------------*/

void SwBaseShell::EditRegionDialog(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    USHORT nSlot = rReq.GetSlot();
    const SfxPoolItem* pItem = 0;
    if(pArgs)
        pArgs->GetItemState(nSlot, FALSE, &pItem);
    SwWrtShell& rWrtShell = GetShell();

    switch ( nSlot )
    {
        case FN_EDIT_REGION:
        {
            Window* pParentWin = &GetView().GetViewFrame()->GetWindow();
            BOOL bStart = TRUE;
            if(bStart)
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                AbstractEditRegionDlg* pEditRegionDlg = pFact->CreateEditRegionDlg( MD_EDIT_REGION,
                                                        pParentWin, rWrtShell);
                DBG_ASSERT(pEditRegionDlg, "Dialogdiet fail!");
                if(pItem && pItem->ISA(SfxStringItem))
                {
                    pEditRegionDlg->SelectSection(((const SfxStringItem*)pItem)->GetValue());
                }
                pEditRegionDlg->Execute();
                delete pEditRegionDlg;
            }
            else
                InfoBox(pParentWin, SW_RES(REG_WRONG_PASSWORD)).Execute();
        }
        break;
    }
}
