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

#include <svl/whiter.hxx>
#include <svl/style.hxx>

#include <sfx2/styledlg.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

#include "mgetempl.hxx"

/*  [Description]

    Constructor: Add Manage TabPage, set ExampleSet from style.
*/
SfxStyleDialogController::SfxStyleDialogController
(
    weld::Window* pParent,           // Parent
    const OUString& rUIXMLDescription, const OUString& rID,
    SfxStyleSheetBase& rStyle  // stylesheet to be processed
)
    : SfxTabDialogController(pParent, rUIXMLDescription, rID, &rStyle.GetItemSet(), true)
    , m_rStyle(rStyle)
{
    // without ParentSupport suppress the standardButton
    if (!rStyle.HasParentSupport())
        RemoveStandardButton();

    AddTabPage(u"organizer"_ustr, SfxManageStyleSheetPage::Create, nullptr);

    // With new template always set the management page as the current page
    if (rStyle.GetName().isEmpty())
        SetCurPageId(u"organizer"_ustr);
    else
    {
        OUString sTxt = m_xDialog->get_title() + ": " + rStyle.GetName();
        m_xDialog->set_title(sTxt);
    }
    m_xExampleSet.reset(&m_rStyle.GetItemSet()); // in SfxTabDialog::Ctor() already created, reset will delete it

    GetCancelButton().connect_clicked(LINK(this, SfxStyleDialogController, CancelHdl));
}

/*  [Description]

    Destructor: set ExampleSet to NULL, so that SfxTabDialog does not delete
    the Set from Style.
*/
SfxStyleDialogController::~SfxStyleDialogController()
{
    // coverity[leaked_storage] - deliberate, ownership is really with m_rStyle
    m_xExampleSet.release();
}

/*  [Description]

    Override so that always RET_OK is returned.
*/
short SfxStyleDialogController::Ok()
{
    SfxTabDialogController::Ok();
    return RET_OK;
}

/*  [Description]

    If the dialogue was canceled, then all selected attributes must be reset
    again.
*/
IMPL_LINK_NOARG(SfxStyleDialogController, CancelHdl, weld::Button&, void)
{
    SfxTabPage* pPage = GetTabPage(u"organizer");

    const SfxItemSet* pInSet = GetInputSetImpl();
    SfxWhichIter aIter(*pInSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        SfxItemState eState = aIter.GetItemState(false);

        if (SfxItemState::DEFAULT == eState)
            m_xExampleSet->ClearItem(nWhich);
        else
            m_xExampleSet->Put(pInSet->Get(nWhich));
        nWhich = aIter.NextWhich();
    }

    if (pPage)
        pPage->Reset(GetInputSetImpl());

    m_xDialog->response(RET_CANCEL);
}

OUString SfxStyleDialogController::GenerateUnusedName(SfxStyleSheetBasePool &rPool, SfxStyleFamily eFam)
{
    OUString aNo(SfxResId(STR_NONAME));
    sal_uInt16 i = 1;
    OUString aNoName = aNo + OUString::number(i);
    while (rPool.Find(aNoName, eFam))
    {
        ++i;
        aNoName = aNo + OUString::number(i);
    }
    return aNoName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
