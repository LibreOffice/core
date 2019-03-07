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

ScRedlineOptionsTabPage::ScRedlineOptionsTabPage( vcl::Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent,"OptChangesPage", "modules/scalc/ui/optchangespage.ui", &rSet)
{
    get(m_pContentColorLB, "changes");
    m_pContentColorLB->SetSlotId(SID_AUTHOR_COLOR);
    get(m_pRemoveColorLB, "deletions");
    m_pRemoveColorLB->SetSlotId(SID_AUTHOR_COLOR);
    get(m_pInsertColorLB, "entries");
    m_pInsertColorLB->SetSlotId(SID_AUTHOR_COLOR);
    get(m_pMoveColorLB, "insertions");
    m_pMoveColorLB->SetSlotId(SID_AUTHOR_COLOR);
}

ScRedlineOptionsTabPage::~ScRedlineOptionsTabPage()
{
    disposeOnce();
}

void ScRedlineOptionsTabPage::dispose()
{
    m_pContentColorLB.clear();
    m_pRemoveColorLB.clear();
    m_pInsertColorLB.clear();
    m_pMoveColorLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> ScRedlineOptionsTabPage::Create( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<ScRedlineOptionsTabPage>::Create( pParent.pParent, *rSet );
}

bool ScRedlineOptionsTabPage::FillItemSet( SfxItemSet* /* rSet */ )
{
    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();

    Color nNew = m_pContentColorLB->GetSelectEntryColor();
    aAppOptions.SetTrackContentColor(nNew);

    nNew = m_pMoveColorLB->GetSelectEntryColor();
    aAppOptions.SetTrackMoveColor(nNew);

    nNew = m_pInsertColorLB->GetSelectEntryColor();
    aAppOptions.SetTrackInsertColor(nNew);

    nNew = m_pRemoveColorLB->GetSelectEntryColor();
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
    m_pContentColorLB->SelectEntry(nColor);

    nColor = aAppOptions.GetTrackMoveColor();
    m_pMoveColorLB->SelectEntry(nColor);

    nColor = aAppOptions.GetTrackInsertColor();
    m_pInsertColorLB->SelectEntry(nColor);

    nColor = aAppOptions.GetTrackDeleteColor();
    m_pRemoveColorLB->SelectEntry(nColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
