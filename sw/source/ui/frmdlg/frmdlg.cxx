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

// the dialog's carrier
SwFrmDlg::SwFrmDlg( SfxViewFrame*       pViewFrame,
                    vcl::Window*             pParent,
                    const SfxItemSet&   rCoreSet,
                    bool                bNewFrm,
                    const OUString&      sResType,
                    bool                bFormat,
                    const OString&      sDefPage,
                    const OUString*     pStr)

    : SfxTabDialog(pViewFrame, pParent, sResType,
        "modules/swriter/ui/" +
        sResType.toAsciiLowerCase() +
        (".ui"), &rCoreSet, pStr != nullptr)
    , m_bFormat(bFormat)
    , m_bNew(bNewFrm)
    , m_rSet(rCoreSet)
    , m_sDlgType(sResType)
    , m_pWrtShell(static_cast<SwView*>(pViewFrame->GetViewShell())->GetWrtShellPtr())
    , m_nStdId(0)
    , m_nAddId(0)
    , m_nWrapId(0)
    , m_nUrlId(0)
    , m_nPictureId(0)
    , m_nCropId(0)
    , m_nColumnId(0)
    //, m_nBackgroundId(0)
    , m_nAreaId(0)
    , m_nTransparenceId(0)
    , m_nMacroId(0)
    , m_nBorderId(0)
{
    sal_uInt16 nHtmlMode = ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell());
    m_bHTMLMode = (nHtmlMode & HTMLMODE_ON) != 0;

    // example font for both example TabPages

    if(pStr)
    {
        SetText(GetText() + SW_RESSTR(STR_COLL_HEADER) + *pStr + ")");
    }

    m_nStdId = AddTabPage("type",  SwFrmPage::Create, nullptr);
    m_nAddId = AddTabPage("options",  SwFrmAddPage::Create, nullptr);
    m_nWrapId = AddTabPage("wrap", SwWrapTabPage::Create, nullptr);
    m_nUrlId = AddTabPage("hyperlink",  SwFrmURLPage::Create, nullptr);
    if (m_sDlgType == "PictureDialog")
    {
        m_nPictureId = AddTabPage("picture", SwGrfExtPage::Create, nullptr);
        m_nCropId = AddTabPage("crop", RID_SVXPAGE_GRFCROP);
    }
    if (m_sDlgType == "FrameDialog")
    {
        m_nColumnId = AddTabPage("columns", SwColumnPage::Create, nullptr);
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");

    //UUUU remove?
    // m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0);

    //UUUU add Area and Transparence TabPages
    m_nAreaId = AddTabPage("area", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
    m_nTransparenceId = AddTabPage("transparence", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

    m_nMacroId = AddTabPage("macro", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), nullptr);
    m_nBorderId = AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), nullptr);

    if(m_bHTMLMode)
    {
        if (m_sDlgType == "FrameDialog" || m_sDlgType == "ObjectDialog")
        {
            if (m_sDlgType == "FrameDialog")
                RemoveTabPage("columns");
            RemoveTabPage("hyperlink");
            RemoveTabPage("macro");
        }
        else if (m_sDlgType == "PictureDialog")
            RemoveTabPage("crop");
        if( m_sDlgType != "FrameDialog" )
        {
            //UUUU RemoveTabPage("background");
            RemoveTabPage("area");
            RemoveTabPage("transparence");
        }
    }

    if (m_bNew)
        SetCurPageId("type");

    if (!sDefPage.isEmpty())
        SetCurPageId(sDefPage);
}

SwFrmDlg::~SwFrmDlg()
{
}

void SwFrmDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nStdId)
    {
        static_cast<SwFrmPage&>(rPage).SetNewFrame(m_bNew);
        static_cast<SwFrmPage&>(rPage).SetFormatUsed(m_bFormat);
        static_cast<SwFrmPage&>(rPage).SetFrmType(m_sDlgType);
    }
    else if (nId == m_nAddId)
    {
        static_cast<SwFrmAddPage&>(rPage).SetFormatUsed(m_bFormat);
        static_cast<SwFrmAddPage&>(rPage).SetFrmType(m_sDlgType);
        static_cast<SwFrmAddPage&>(rPage).SetNewFrame(m_bNew);
        static_cast<SwFrmAddPage&>(rPage).SetShell(m_pWrtShell);
    }
    else if (nId == m_nWrapId)
    {
        static_cast<SwWrapTabPage&>(rPage).SetNewFrame(m_bNew);
        static_cast<SwWrapTabPage&>(rPage).SetFormatUsed(m_bFormat, false);
        static_cast<SwWrapTabPage&>(rPage).SetShell(m_pWrtShell);
    }
    else if (nId == m_nColumnId)
    {
        static_cast<SwColumnPage&>(rPage).SetFrmMode(true);
        static_cast<SwColumnPage&>(rPage).SetFormatUsed(m_bFormat);

        const SwFormatFrmSize& rSize = static_cast<const SwFormatFrmSize&>(
                                            m_rSet.Get( RES_FRM_SIZE ));
        static_cast<SwColumnPage&>(rPage).SetPageWidth( rSize.GetWidth() );
    }
    else if (nId == m_nMacroId)
    {
        SfxAllItemSet aNewSet(*aSet.GetPool());
        aNewSet.Put( SwMacroAssignDlg::AddEvents(
            m_sDlgType == "PictureDialog" ? MACASSGN_GRAPHIC : m_sDlgType == "ObjectDialog" ? MACASSGN_OLE : MACASSGN_FRMURL ) );
        if (m_pWrtShell)
            rPage.SetFrame( m_pWrtShell->GetView().GetViewFrame()->GetFrame().GetFrameInterface() );
        rPage.PageCreated(aNewSet);
    }
    //UUUU
    //else if (nId == m_nBackgroundId && m_sDlgType == "FrameDialog")
    //{
    //    sal_Int32 nFlagType = SVX_SHOW_SELECTOR;
    //    if (!m_bHTMLMode)
    //        nFlagType |= SVX_ENABLE_TRANSPARENCY;
    //    aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlagType));
    //
    //    rPage.PageCreated(aSet);
    //}
    else if (nId == m_nBorderId)
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::FRAME)));
        rPage.PageCreated(aSet);
    }
    //UUUU inits for Area and Transparency TabPages
    // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
    // the color table) need to be added as items (e.g. SvxColorListItem) to make
    // these pages find the needed attributes for fill style suggestions.
    // These are set in preparation to trigger this dialog (FN_FORMAT_FRAME_DLG and
    // FN_DRAW_WRAP_DLG), but could also be directly added from the DrawModel.
    else if (nId == m_nAreaId)
    {
        SfxItemSet aNew(*GetInputSetImpl()->GetPool(),
                        SID_COLOR_TABLE, SID_BITMAP_LIST,
                        SID_OFFER_IMPORT, SID_OFFER_IMPORT, 0, 0);

        aNew.Put(m_rSet);

        // add flag for direct graphic content selection
        aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

        rPage.PageCreated(aNew);
    }
    else if (nId == m_nTransparenceId)
    {
        rPage.PageCreated(m_rSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
