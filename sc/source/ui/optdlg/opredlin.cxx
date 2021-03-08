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

#include <svx/colorbox.hxx>

#include <appoptio.hxx>
#include <scmod.hxx>
#include <docsh.hxx>
#include <svx/svxids.hrc>

#include <opredlin.hxx>

ScRedlineOptionsTabPage::ScRedlineOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/optchangespage.ui", "OptChangesPage", &rSet)
    , m_xContentColorLB(new ColorListBox(m_xBuilder->weld_menu_button("changes"),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xRemoveColorLB(new ColorListBox(m_xBuilder->weld_menu_button("deletions"),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xInsertColorLB(new ColorListBox(m_xBuilder->weld_menu_button("entries"),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xMoveColorLB(new ColorListBox(m_xBuilder->weld_menu_button("insertions"),
                [this]{ return GetDialogController()->getDialog(); }))
{
    m_xContentColorLB->SetSlotId(SID_AUTHOR_COLOR);
    m_xRemoveColorLB->SetSlotId(SID_AUTHOR_COLOR);
    m_xInsertColorLB->SetSlotId(SID_AUTHOR_COLOR);
    m_xMoveColorLB->SetSlotId(SID_AUTHOR_COLOR);
}

ScRedlineOptionsTabPage::~ScRedlineOptionsTabPage()
{
    m_xContentColorLB.reset();
    m_xRemoveColorLB.reset();
    m_xInsertColorLB.reset();
    m_xMoveColorLB.reset();
}

std::unique_ptr<SfxTabPage> ScRedlineOptionsTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<ScRedlineOptionsTabPage>( pPage, pController, *rSet );
}

bool ScRedlineOptionsTabPage::FillItemSet( SfxItemSet* /* rSet */ )
{
    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    Color nNew = m_xContentColorLB->GetSelectEntryColor();
    aAppOptions.SetTrackContentColor(nNew);

    nNew = m_xMoveColorLB->GetSelectEntryColor();
    aAppOptions.SetTrackMoveColor(nNew);

    nNew = m_xInsertColorLB->GetSelectEntryColor();
    aAppOptions.SetTrackInsertColor(nNew);

    nNew = m_xRemoveColorLB->GetSelectEntryColor();
    aAppOptions.SetTrackDeleteColor(nNew);

    SC_MOD()->SetAppOptions(aAppOptions);

    //  repaint (if everything would be done by Items (how it should be),
    //  this wouldn't be necessary)
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current() );
    if (pDocSh)
        pDocSh->PostPaintGridAll();

    return false;
}

void ScRedlineOptionsTabPage::Reset( const SfxItemSet* /* rSet */ )
{
    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    Color nColor = aAppOptions.GetTrackContentColor();
    m_xContentColorLB->SelectEntry(nColor);

    nColor = aAppOptions.GetTrackMoveColor();
    m_xMoveColorLB->SelectEntry(nColor);

    nColor = aAppOptions.GetTrackInsertColor();
    m_xInsertColorLB->SelectEntry(nColor);

    nColor = aAppOptions.GetTrackDeleteColor();
    m_xRemoveColorLB->SelectEntry(nColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
