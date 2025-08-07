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

#undef SC_DLLIMPLEMENTATION

#include <scitems.hxx>
#include <svl/eitem.hxx>

#include <hfedtdlg.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scuitphfedit.hxx>
#include <svx/svxids.hrc>
#include <svx/pageitem.hxx>

ScHFEditDlg::ScHFEditDlg(weld::Window* pParent,
                         const SfxItemSet& rCoreSet,
                         std::u16string_view rPageStyle,
                         const OUString& rUIXMLDescription,
                         const OUString& rID)
    :   SfxTabDialogController(pParent, rUIXMLDescription, rID, &rCoreSet)
{
    eNumType = rCoreSet.Get(ATTR_PAGE).GetNumType();

    OUString aTmp = m_xDialog->get_title() +
        " (" + ScResId( STR_PAGESTYLE ) + ": " + rPageStyle + ")";

    m_xDialog->set_title(aTmp);
}

ScHFEditHeaderDlg::ScHFEditHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/headerdialog.ui"_ustr, u"HeaderDialog"_ustr)
{
    AddTabPage(u"headerfirst"_ustr, ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage(u"headerright"_ustr, ScRightHeaderEditPage::Create, nullptr);
    AddTabPage(u"headerleft"_ustr, ScLeftHeaderEditPage::Create, nullptr);
}

ScHFEditFooterDlg::ScHFEditFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/footerdialog.ui"_ustr, u"FooterDialog"_ustr )
{
    AddTabPage(u"footerfirst"_ustr, ScFirstFooterEditPage::Create, nullptr);
    AddTabPage(u"footerright"_ustr, ScRightFooterEditPage::Create, nullptr);
    AddTabPage(u"footerleft"_ustr, ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditSharedFirstHeaderDlg::ScHFEditSharedFirstHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/sharedfirstheaderdialog.ui"_ustr, u"SharedFirstHeaderDialog"_ustr )
{
    AddTabPage(u"headerright"_ustr, ScRightHeaderEditPage::Create, nullptr);
    AddTabPage(u"headerleft"_ustr, ScLeftHeaderEditPage::Create, nullptr);
}

ScHFEditSharedFirstFooterDlg::ScHFEditSharedFirstFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/sharedfirstfooterdialog.ui"_ustr, u"SharedFirstFooterDialog"_ustr )
{
    AddTabPage(u"footerright"_ustr, ScRightFooterEditPage::Create, nullptr);
    AddTabPage(u"footerleft"_ustr, ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditSharedLeftHeaderDlg::ScHFEditSharedLeftHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/sharedleftheaderdialog.ui"_ustr, u"SharedLeftHeaderDialog"_ustr )
{
    AddTabPage(u"headerfirst"_ustr, ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage(u"headerright"_ustr, ScRightHeaderEditPage::Create, nullptr);
}

ScHFEditSharedLeftFooterDlg::ScHFEditSharedLeftFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/sharedleftfooterdialog.ui"_ustr, u"SharedLeftFooterDialog"_ustr )
{
    AddTabPage(u"footerfirst"_ustr, ScFirstFooterEditPage::Create, nullptr);
    AddTabPage(u"footerright"_ustr, ScRightFooterEditPage::Create, nullptr);
}

ScHFEditFirstHeaderDlg::ScHFEditFirstHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/firstheaderdialog.ui"_ustr, u"FirstHeaderDialog"_ustr )
{
    AddTabPage(u"headerfirst"_ustr, ScFirstHeaderEditPage::Create, nullptr);
}

ScHFEditLeftHeaderDlg::ScHFEditLeftHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/leftheaderdialog.ui"_ustr, u"LeftHeaderDialog"_ustr )
{
    AddTabPage(u"headerleft"_ustr, ScLeftHeaderEditPage::Create, nullptr);
}

ScHFEditRightHeaderDlg::ScHFEditRightHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/rightheaderdialog.ui"_ustr, u"RightHeaderDialog"_ustr )
{
    AddTabPage(u"headerright"_ustr, ScRightHeaderEditPage::Create, nullptr);
}

ScHFEditFirstFooterDlg::ScHFEditFirstFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/firstfooterdialog.ui"_ustr, u"FirstFooterDialog"_ustr )
{
    AddTabPage(u"footerfirst"_ustr, ScFirstFooterEditPage::Create, nullptr);
}

ScHFEditLeftFooterDlg::ScHFEditLeftFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/leftfooterdialog.ui"_ustr, u"LeftFooterDialog"_ustr )
{
    AddTabPage(u"footerleft"_ustr, ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditRightFooterDlg::ScHFEditRightFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/rightfooterdialog.ui"_ustr, u"RightFooterDialog"_ustr )
{
    AddTabPage(u"footerright"_ustr, ScRightFooterEditPage::Create, nullptr);
}

ScHFEditSharedHeaderDlg::ScHFEditSharedHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/sharedheaderdialog.ui"_ustr, u"SharedHeaderDialog"_ustr )
{
    AddTabPage(u"headerfirst"_ustr, ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage(u"header"_ustr, ScRightHeaderEditPage::Create, nullptr);
    AddTabPage(u"footerfirst"_ustr, ScFirstFooterEditPage::Create, nullptr);
    AddTabPage(u"footerright"_ustr, ScRightFooterEditPage::Create, nullptr);
    AddTabPage(u"footerleft"_ustr, ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditSharedFooterDlg::ScHFEditSharedFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/sharedfooterdialog.ui"_ustr, u"SharedFooterDialog"_ustr )
{
    AddTabPage(u"headerfirst"_ustr, ScFirstFooterEditPage::Create, nullptr);
    AddTabPage(u"headerright"_ustr, ScRightHeaderEditPage::Create, nullptr);
    AddTabPage(u"headerleft"_ustr, ScLeftHeaderEditPage::Create, nullptr);
    AddTabPage(u"footerfirst"_ustr, ScFirstFooterEditPage::Create, nullptr);
    AddTabPage(u"footer"_ustr, ScRightFooterEditPage::Create, nullptr);
}

ScHFEditAllDlg::ScHFEditAllDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/allheaderfooterdialog.ui"_ustr, u"AllHeaderFooterDialog"_ustr )
{
    AddTabPage(u"headerfirst"_ustr, ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage(u"headerright"_ustr, ScRightHeaderEditPage::Create, nullptr);
    AddTabPage(u"headerleft"_ustr, ScLeftHeaderEditPage::Create, nullptr);
    AddTabPage(u"footerfirst"_ustr, ScFirstFooterEditPage::Create, nullptr);
    AddTabPage(u"footerright"_ustr, ScRightFooterEditPage::Create, nullptr);
    AddTabPage(u"footerleft"_ustr, ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditActiveDlg::ScHFEditActiveDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        u"modules/scalc/ui/headerfooterdialog.ui"_ustr, u"HeaderFooterDialog"_ustr )
{
    const SvxPageItem&  rPageItem =
                rCoreSet.Get(
                    rCoreSet.GetPool()->GetWhichIDFromSlotID(SID_ATTR_PAGE) );

    bool bRightPage = SvxPageUsage::Left != rPageItem.GetPageUsage();

    if ( bRightPage )
    {
        AddTabPage(u"header"_ustr, ScRightHeaderEditPage::Create, nullptr);
        AddTabPage(u"footer"_ustr, ScRightFooterEditPage::Create, nullptr);
    }
    else
    {
        //  #69193a# respect "shared" setting

        bool bShareHeader = rCoreSet.Get(ATTR_PAGE_HEADERSET).GetItemSet().
                                Get(ATTR_PAGE_SHARED).GetValue();
        if ( bShareHeader )
            AddTabPage(u"header"_ustr, ScRightHeaderEditPage::Create, nullptr);
        else
            AddTabPage(u"header"_ustr, ScLeftHeaderEditPage::Create, nullptr);

        bool bShareFooter = rCoreSet.Get(ATTR_PAGE_FOOTERSET).GetItemSet().
                                Get(ATTR_PAGE_SHARED).GetValue();
        if ( bShareFooter )
            AddTabPage(u"footer"_ustr, ScRightFooterEditPage::Create, nullptr);
        else
            AddTabPage(u"footer"_ustr, ScLeftFooterEditPage::Create, nullptr);
    }
}

void ScHFEditDlg::PageCreated(const OUString& /* rId */, SfxTabPage& rPage)
{
    // Can only be a ScHFEditPage...

    static_cast<ScHFEditPage&>(rPage).SetNumType(eNumType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
