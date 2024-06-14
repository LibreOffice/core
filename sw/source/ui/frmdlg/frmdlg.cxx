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
#include <sfx2/sfxdlg.hxx>
#include <sfx2/htmlmode.hxx>
#include <fmtfsize.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <frmdlg.hxx>
#include <frmpage.hxx>
#include <wrap.hxx>
#include <column.hxx>
#include <macassgn.hxx>
#include <IDocumentSettingAccess.hxx>

#include <strings.hrc>
#include <svl/eitem.hxx>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svx/drawitem.hxx>
#include <comphelper/lok.hxx>

// the dialog's carrier
SwFrameDlg::SwFrameDlg(const SfxViewFrame& rViewFrame,
                       weld::Window*       pParent,
                       const SfxItemSet&   rCoreSet,
                       bool                bNewFrame,
                       const OUString&     sResType,
                       bool                bFormat,
                       const OUString&     sDefPage,
                       const OUString*     pStr)

    : SfxTabDialogController(pParent, "modules/swriter/ui/" + sResType.toAsciiLowerCase() + ".ui",
                             sResType, &rCoreSet, pStr != nullptr)
    , m_bFormat(bFormat)
    , m_bNew(bNewFrame)
    , m_rSet(rCoreSet)
    , m_sDlgType(sResType)
    , m_pWrtShell(static_cast<SwView*>(rViewFrame.GetViewShell())->GetWrtShellPtr())
{
    sal_uInt16 nHtmlMode = ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell());
    bool bHTMLMode = (nHtmlMode & HTMLMODE_ON) != 0;

    // example font for both example TabPages

    if (pStr)
    {
        m_xDialog->set_title(m_xDialog->get_title() + SwResId(STR_FRMUI_COLL_HEADER) + *pStr + ")");
    }

    AddTabPage(u"type"_ustr,  SwFramePage::Create, nullptr);
    AddTabPage(u"options"_ustr,  SwFrameAddPage::Create, nullptr);
    AddTabPage(u"wrap"_ustr, SwWrapTabPage::Create, nullptr);
    AddTabPage(u"hyperlink"_ustr,  SwFrameURLPage::Create, nullptr);
    if (m_sDlgType == "PictureDialog")
    {
        AddTabPage(u"picture"_ustr, SwGrfExtPage::Create, nullptr);
        AddTabPage(u"crop"_ustr, RID_SVXPAGE_GRFCROP);
    }
    if (m_sDlgType == "FrameDialog")
    {
        AddTabPage(u"columns"_ustr, SwColumnPage::Create, nullptr);
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    // add Area and Transparence TabPages
    AddTabPage(u"area"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_AREA ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_AREA ));
    AddTabPage(u"transparence"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_TRANSPARENCE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_TRANSPARENCE ) );

    AddTabPage(u"macro"_ustr, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), nullptr);
    AddTabPage(u"borders"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), nullptr);

    if(bHTMLMode)
    {
        if (m_sDlgType == "FrameDialog" || m_sDlgType == "ObjectDialog")
        {
            if (m_sDlgType == "FrameDialog")
                RemoveTabPage(u"columns"_ustr);
            RemoveTabPage(u"hyperlink"_ustr);
            RemoveTabPage(u"macro"_ustr);
        }
        else if (m_sDlgType == "PictureDialog")
            RemoveTabPage(u"crop"_ustr);
        if( m_sDlgType != "FrameDialog" )
        {
            // RemoveTabPage("background");
            RemoveTabPage(u"area"_ustr);
            RemoveTabPage(u"transparence"_ustr);
        }
    }

    if(comphelper::LibreOfficeKit::isActive())
        RemoveTabPage(u"macro"_ustr);

    if (m_bNew)
        SetCurPageId(u"type"_ustr);

    if (!sDefPage.isEmpty())
        SetCurPageId(sDefPage);
}

SwFrameDlg::~SwFrameDlg()
{
}

void SwFrameDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "type")
    {
        static_cast<SwFramePage&>(rPage).SetNewFrame(m_bNew);
        static_cast<SwFramePage&>(rPage).SetFormatUsed(m_bFormat);
        static_cast<SwFramePage&>(rPage).SetFrameType(m_sDlgType);
    }
    else if (rId == "options")
    {
        static_cast<SwFrameAddPage&>(rPage).SetFormatUsed(m_bFormat);
        static_cast<SwFrameAddPage&>(rPage).SetFrameType(m_sDlgType);
        static_cast<SwFrameAddPage&>(rPage).SetNewFrame(m_bNew);
        static_cast<SwFrameAddPage&>(rPage).SetShell(m_pWrtShell);
    }
    else if (rId == "wrap")
    {
        static_cast<SwWrapTabPage&>(rPage).SetNewFrame(m_bNew);
        static_cast<SwWrapTabPage&>(rPage).SetFormatUsed(m_bFormat, false);
        static_cast<SwWrapTabPage&>(rPage).SetShell(m_pWrtShell);
    }
    else if (rId == "columns")
    {
        static_cast<SwColumnPage&>(rPage).SetFrameMode(true);
        static_cast<SwColumnPage&>(rPage).SetFormatUsed(m_bFormat);

        const SwFormatFrameSize& rSize = m_rSet.Get( RES_FRM_SIZE );
        static_cast<SwColumnPage&>(rPage).SetPageWidth( rSize.GetWidth() );
    }
    else if (rId == "macro")
    {
        SfxAllItemSet aNewSet(*aSet.GetPool());
        aNewSet.Put( SwMacroAssignDlg::AddEvents(
            m_sDlgType == "PictureDialog" ? MACASSGN_GRAPHIC : m_sDlgType == "ObjectDialog" ? MACASSGN_OLE : MACASSGN_FRMURL ) );
        if (m_pWrtShell)
            rPage.SetFrame( m_pWrtShell->GetView().GetViewFrame().GetFrame().GetFrameInterface() );
        rPage.PageCreated(aNewSet);
    }
    else if (rId == "borders")
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,static_cast<sal_uInt16>(SwBorderModes::FRAME)));
        rPage.PageCreated(aSet);
    }
    // inits for Area and Transparency TabPages
    // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
    // the color table) need to be added as items (e.g. SvxColorListItem) to make
    // these pages find the needed attributes for fill style suggestions.
    // These are set in preparation to trigger this dialog (FN_FORMAT_FRAME_DLG and
    // FN_DRAW_WRAP_DLG), but could also be directly added from the DrawModel.
    else if (rId == "area")
    {
        SfxItemSetFixed<SID_COLOR_TABLE, SID_PATTERN_LIST,
                        SID_OFFER_IMPORT, SID_OFFER_IMPORT>
            aNew(*GetInputSetImpl()->GetPool());

        aNew.Put(m_rSet);

        // add flag for direct graphic content selection
        aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

        rPage.PageCreated(aNew);
    }
    else if (rId == "transparence")
    {
        rPage.PageCreated(m_rSet);
    }
    else if (rId == "crop")
    {
        sal_Int32 nPreferredDPI = m_pWrtShell->GetDoc()->getIDocumentSettingAccess().getImagePreferredDPI();
        if (nPreferredDPI)
            rPage.getAdditionalProperties().emplace("PreferredDPI", css::uno::Any(nPreferredDPI));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
