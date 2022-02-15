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
                         const OString& rID)
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
        "modules/scalc/ui/headerdialog.ui", "HeaderDialog")
{
    AddTabPage("headerfirst", ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage("headerright", ScRightHeaderEditPage::Create, nullptr);
    AddTabPage("headerleft", ScLeftHeaderEditPage::Create, nullptr);
}

ScHFEditFooterDlg::ScHFEditFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/footerdialog.ui", "FooterDialog" )
{
    AddTabPage("footerfirst", ScFirstFooterEditPage::Create, nullptr);
    AddTabPage("footerright", ScRightFooterEditPage::Create, nullptr);
    AddTabPage("footerleft", ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditSharedFirstHeaderDlg::ScHFEditSharedFirstHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/sharedfirstheaderdialog.ui", "SharedFirstHeaderDialog" )
{
    AddTabPage("headerright", ScRightHeaderEditPage::Create, nullptr);
    AddTabPage("headerleft", ScLeftHeaderEditPage::Create, nullptr);
}

ScHFEditSharedFirstFooterDlg::ScHFEditSharedFirstFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/sharedfirstfooterdialog.ui", "SharedFirstFooterDialog" )
{
    AddTabPage("footerright", ScRightFooterEditPage::Create, nullptr);
    AddTabPage("footerleft", ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditSharedLeftHeaderDlg::ScHFEditSharedLeftHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/sharedleftheaderdialog.ui", "SharedLeftHeaderDialog" )
{
    AddTabPage("headerfirst", ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage("headerright", ScRightHeaderEditPage::Create, nullptr);
}

ScHFEditSharedLeftFooterDlg::ScHFEditSharedLeftFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/sharedleftfooterdialog.ui", "SharedLeftFooterDialog" )
{
    AddTabPage("footerfirst", ScFirstFooterEditPage::Create, nullptr);
    AddTabPage("footerright", ScRightFooterEditPage::Create, nullptr);
}

ScHFEditFirstHeaderDlg::ScHFEditFirstHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/firstheaderdialog.ui", "FirstHeaderDialog" )
{
    AddTabPage("headerfirst", ScFirstHeaderEditPage::Create, nullptr);
}

ScHFEditLeftHeaderDlg::ScHFEditLeftHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/leftheaderdialog.ui", "LeftHeaderDialog" )
{
    AddTabPage("headerleft", ScLeftHeaderEditPage::Create, nullptr);
}

ScHFEditRightHeaderDlg::ScHFEditRightHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/rightheaderdialog.ui", "RightHeaderDialog" )
{
    AddTabPage("headerright", ScRightHeaderEditPage::Create, nullptr);
}

ScHFEditFirstFooterDlg::ScHFEditFirstFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/firstfooterdialog.ui", "FirstFooterDialog" )
{
    AddTabPage("footerfirst", ScFirstFooterEditPage::Create, nullptr);
}

ScHFEditLeftFooterDlg::ScHFEditLeftFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/leftfooterdialog.ui", "LeftFooterDialog" )
{
    AddTabPage("footerleft", ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditRightFooterDlg::ScHFEditRightFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/rightfooterdialog.ui", "RightFooterDialog" )
{
    AddTabPage("footerright", ScRightFooterEditPage::Create, nullptr);
}

ScHFEditSharedHeaderDlg::ScHFEditSharedHeaderDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/sharedheaderdialog.ui", "SharedHeaderDialog" )
{
    AddTabPage("headerfirst", ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage("header", ScRightHeaderEditPage::Create, nullptr);
    AddTabPage("footerfirst", ScFirstFooterEditPage::Create, nullptr);
    AddTabPage("footerright", ScRightFooterEditPage::Create, nullptr);
    AddTabPage("footerleft", ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditSharedFooterDlg::ScHFEditSharedFooterDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/sharedfooterdialog.ui", "SharedFooterDialog" )
{
    AddTabPage("headerfirst", ScFirstFooterEditPage::Create, nullptr);
    AddTabPage("headerright", ScRightHeaderEditPage::Create, nullptr);
    AddTabPage("headerleft", ScLeftHeaderEditPage::Create, nullptr);
    AddTabPage("footerfirst", ScFirstFooterEditPage::Create, nullptr);
    AddTabPage("footer", ScRightFooterEditPage::Create, nullptr);
}

ScHFEditAllDlg::ScHFEditAllDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/allheaderfooterdialog.ui", "AllHeaderFooterDialog" )
{
    AddTabPage("headerfirst", ScFirstHeaderEditPage::Create, nullptr);
    AddTabPage("headerright", ScRightHeaderEditPage::Create, nullptr);
    AddTabPage("headerleft", ScLeftHeaderEditPage::Create, nullptr);
    AddTabPage("footerfirst", ScFirstFooterEditPage::Create, nullptr);
    AddTabPage("footerright", ScRightFooterEditPage::Create, nullptr);
    AddTabPage("footerleft", ScLeftFooterEditPage::Create, nullptr);
}

ScHFEditActiveDlg::ScHFEditActiveDlg(
                          weld::Window* pParent,
                          const SfxItemSet& rCoreSet,
                          std::u16string_view rPageStyle)
    :   ScHFEditDlg( pParent, rCoreSet, rPageStyle,
        "modules/scalc/ui/headerfooterdialog.ui", "HeaderFooterDialog" )
{
    const SvxPageItem&  rPageItem =
                rCoreSet.Get(
                    rCoreSet.GetPool()->GetWhich(SID_ATTR_PAGE) );

    bool bRightPage = SvxPageUsage::Left != rPageItem.GetPageUsage();

    if ( bRightPage )
    {
        AddTabPage("header", ScRightHeaderEditPage::Create, nullptr);
        AddTabPage("footer", ScRightFooterEditPage::Create, nullptr);
    }
    else
    {
        //  #69193a# respect "shared" setting

        bool bShareHeader = rCoreSet.Get(ATTR_PAGE_HEADERSET).GetItemSet().
                                Get(ATTR_PAGE_SHARED).GetValue();
        if ( bShareHeader )
            AddTabPage("header", ScRightHeaderEditPage::Create, nullptr);
        else
            AddTabPage("header", ScLeftHeaderEditPage::Create, nullptr);

        bool bShareFooter = rCoreSet.Get(ATTR_PAGE_FOOTERSET).GetItemSet().
                                Get(ATTR_PAGE_SHARED).GetValue();
        if ( bShareFooter )
            AddTabPage("footer", ScRightFooterEditPage::Create, nullptr);
        else
            AddTabPage("footer", ScLeftFooterEditPage::Create, nullptr);
    }
}

void ScHFEditDlg::PageCreated(const OString& /* rId */, SfxTabPage& rPage)
{
    // Can only be a ScHFEditPage...

    static_cast<ScHFEditPage&>(rPage).SetNumType(eNumType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
