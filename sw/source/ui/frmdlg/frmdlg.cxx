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

#include <svx/dialogs.hrc>
#include <hintids.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/htmlmode.hxx>
#include <fmtfsize.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <frmdlg.hxx>
#include <frmpage.hxx>
#include <wrap.hxx>
#include <column.hxx>
#include <macassgn.hxx>

#include <frmui.hrc>
#include <globals.hrc>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>

/*--------------------------------------------------------------------
    Description:    the dialog's carrier
 --------------------------------------------------------------------*/
SwFrmDlg::SwFrmDlg( SfxViewFrame*       pViewFrame,
                    Window*             pParent,
                    const SfxItemSet&   rCoreSet,
                    sal_Bool                bNewFrm,
                    sal_uInt16              nResType,
                    sal_Bool                bFormat,
                    sal_uInt16              nDefPage,
                    const String*       pStr) :

    SfxTabDialog(pViewFrame, pParent, SW_RES(nResType), &rCoreSet, pStr != 0),
    m_bFormat(bFormat),
    m_bNew(bNewFrm),
    m_rSet(rCoreSet),
    m_nDlgType(nResType),
    m_pWrtShell(((SwView*)pViewFrame->GetViewShell())->GetWrtShellPtr())
{
    FreeResource();
    sal_uInt16 nHtmlMode = ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell());
    m_bHTMLMode = static_cast< sal_Bool >(nHtmlMode & HTMLMODE_ON);

    // example font for both example TabPages
    //
    if(pStr)
    {
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_COLL_HEADER);
        aTmp += *pStr;
        aTmp += ')';
    }

    AddTabPage(TP_FRM_STD,  SwFrmPage::Create, 0);
    AddTabPage(TP_FRM_ADD,  SwFrmAddPage::Create, 0);
    AddTabPage(TP_FRM_WRAP, SwWrapTabPage::Create, 0);
    AddTabPage(TP_FRM_URL,  SwFrmURLPage::Create, 0);
    if(m_nDlgType == DLG_FRM_GRF)
    {
        AddTabPage( TP_GRF_EXT, SwGrfExtPage::Create, 0 );
        AddTabPage( RID_SVXPAGE_GRFCROP );
    }
    if (m_nDlgType == DLG_FRM_STD)
    {
        AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    AddTabPage( TP_MACRO_ASSIGN, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), 0);
    AddTabPage( TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), 0 );

    if(m_bHTMLMode)
    {
        switch( m_nDlgType )
        {
        case DLG_FRM_STD:
                RemoveTabPage(TP_COLUMN);
            // no break
        case DLG_FRM_OLE:
                RemoveTabPage(TP_FRM_URL);
                RemoveTabPage(TP_MACRO_ASSIGN);
            break;
        case DLG_FRM_GRF:
                RemoveTabPage(RID_SVXPAGE_GRFCROP);
            break;
        }
        if( m_nDlgType != DLG_FRM_STD )
            RemoveTabPage(TP_BACKGROUND);
    }

    if (m_bNew)
        SetCurPageId(TP_FRM_STD);

    if (nDefPage)
        SetCurPageId(nDefPage);
}

SwFrmDlg::~SwFrmDlg()
{
}

void SwFrmDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch ( nId )
    {
    case TP_FRM_STD:
        ((SwFrmPage&)rPage).SetNewFrame(m_bNew);
        ((SwFrmPage&)rPage).SetFormatUsed(m_bFormat);
        ((SwFrmPage&)rPage).SetFrmType(m_nDlgType);
        break;

    case TP_FRM_ADD:
        ((SwFrmAddPage&)rPage).SetFormatUsed(m_bFormat);
        ((SwFrmAddPage&)rPage).SetFrmType(m_nDlgType);
        ((SwFrmAddPage&)rPage).SetNewFrame(m_bNew);
        ((SwFrmAddPage&)rPage).SetShell(m_pWrtShell);
        break;

    case TP_FRM_WRAP:
        ((SwWrapTabPage&)rPage).SetNewFrame(m_bNew);
        ((SwWrapTabPage&)rPage).SetFormatUsed(m_bFormat, sal_False);
        ((SwWrapTabPage&)rPage).SetShell(m_pWrtShell);
        break;

    case TP_COLUMN:
        {
            ((SwColumnPage&)rPage).SetFrmMode(sal_True);
            ((SwColumnPage&)rPage).SetFormatUsed(m_bFormat);

            const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)
                                                m_rSet.Get( RES_FRM_SIZE );
            ((SwColumnPage&)rPage).SetPageWidth( rSize.GetWidth() );
        }
        break;

    case TP_MACRO_ASSIGN:
        {
        SfxAllItemSet aNewSet(*aSet.GetPool());
        aNewSet.Put( SwMacroAssignDlg::AddEvents(
            DLG_FRM_GRF == m_nDlgType ? MACASSGN_GRAPHIC : DLG_FRM_OLE == m_nDlgType ? MACASSGN_OLE : MACASSGN_FRMURL ) );
        if ( m_pWrtShell )
            rPage.SetFrame( m_pWrtShell->GetView().GetViewFrame()->GetFrame().GetFrameInterface() );
        rPage.PageCreated(aNewSet);
        break;
        }

    case TP_BACKGROUND:
        if( DLG_FRM_STD == m_nDlgType )
        {
            sal_Int32 nFlagType = SVX_SHOW_SELECTOR;
            if(!m_bHTMLMode)
                nFlagType |= SVX_ENABLE_TRANSPARENCY;
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlagType));

            SvxGradientListItem aGradientListItem(m_pWrtShell->GetDoc()->GetOrCreateDrawModel()->GetGradientList(), SID_GRADIENT_LIST);
            aSet.Put(aGradientListItem);

            XFillStyleItem aFillStyleItem(((const XFillStyleItem&)m_rSet.Get(RES_FILL_STYLE)).GetValue(), SID_SW_ATTR_FILL_STYLE);
            aSet.Put(aFillStyleItem);

            const XFillGradientItem& rFillGradientItem = (const XFillGradientItem&)m_rSet.Get(RES_FILL_GRADIENT);
            XFillGradientItem aFillGradientItem(rFillGradientItem.GetName(), rFillGradientItem.GetGradientValue(), SID_SW_ATTR_FILL_GRADIENT);
            aSet.Put(aFillGradientItem);

            rPage.PageCreated(aSet);
        }
        break;

    case TP_BORDER:
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_FRAME));
            rPage.PageCreated(aSet);
        }
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
