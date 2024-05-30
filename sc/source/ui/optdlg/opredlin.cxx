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
#include <officecfg/Office/Calc.hxx>

#include <opredlin.hxx>

ScRedlineOptionsTabPage::ScRedlineOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/scalc/ui/optchangespage.ui"_ustr, u"OptChangesPage"_ustr, &rSet)
    , m_xContentColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"changes"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xContentColorImg(m_xBuilder->weld_widget(u"lockchanges"_ustr))
    , m_xRemoveColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"deletions"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xRemoveColorImg(m_xBuilder->weld_widget(u"lockdeletions"_ustr))
    , m_xInsertColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"entries"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xInsertColorImg(m_xBuilder->weld_widget(u"lockentries"_ustr))
    , m_xMoveColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"insertions"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xMoveColorImg(m_xBuilder->weld_widget(u"lockinsertions"_ustr))
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

OUString ScRedlineOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"label2"_ustr, u"label3"_ustr, u"label4"_ustr, u"label5"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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
    m_xContentColorLB->set_sensitive(!officecfg::Office::Calc::Revision::Color::Change::isReadOnly());
    m_xContentColorImg->set_visible(officecfg::Office::Calc::Revision::Color::Change::isReadOnly());

    nColor = aAppOptions.GetTrackMoveColor();
    m_xMoveColorLB->SelectEntry(nColor);
    m_xMoveColorLB->set_sensitive(!officecfg::Office::Calc::Revision::Color::Insertion::isReadOnly());
    m_xMoveColorImg->set_visible(officecfg::Office::Calc::Revision::Color::Insertion::isReadOnly());

    nColor = aAppOptions.GetTrackInsertColor();
    m_xInsertColorLB->SelectEntry(nColor);
    m_xInsertColorLB->set_sensitive(!officecfg::Office::Calc::Revision::Color::MovedEntry::isReadOnly());
    m_xInsertColorImg->set_visible(officecfg::Office::Calc::Revision::Color::MovedEntry::isReadOnly());

    nColor = aAppOptions.GetTrackDeleteColor();
    m_xRemoveColorLB->SelectEntry(nColor);
    m_xRemoveColorLB->set_sensitive(!officecfg::Office::Calc::Revision::Color::Deletion::isReadOnly());
    m_xRemoveColorImg->set_visible(officecfg::Office::Calc::Revision::Color::Deletion::isReadOnly());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
