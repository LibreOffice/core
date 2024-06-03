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

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <navipi.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <helpids.h>

// class ScScenarioWindow ------------------------------------------------

void ScScenarioWindow::UpdateEntries( const std::vector<OUString> &rNewEntryList )
{
    m_xLbScenario->clear();
    m_aEntries.clear();

    switch( rNewEntryList.size() )
    {
        case 0:
            // no scenarios in current sheet
            SetComment( OUString() );
        break;

        case 1:
            // sheet is a scenario container, comment only
            SetComment( rNewEntryList[0] );
        break;

        default:
        {
            // sheet contains scenarios
            assert(rNewEntryList.size() % 3 == 0 && "ScScenarioListBox::UpdateEntries - wrong list size");
            m_xLbScenario->freeze();

            std::vector<OUString>::const_iterator iter;
            for (iter = rNewEntryList.begin(); iter != rNewEntryList.end(); ++iter)
            {
                ScenarioEntry aEntry;

                // first entry of a triple is the scenario name
                aEntry.maName = *iter;

                // second entry of a triple is the scenario comment
                ++iter;
                aEntry.maComment = *iter;

                // third entry of a triple is the protection ("0" = not protected, "1" = protected)
                ++iter;
                aEntry.mbProtected = !(*iter).isEmpty() && (*iter)[0] != '0';

                m_aEntries.push_back( aEntry );
                m_xLbScenario->append_text(aEntry.maName);
            }
            m_xLbScenario->thaw();
            m_xLbScenario->unselect_all();
            SetComment(OUString());
        }
    }
}

IMPL_LINK_NOARG(ScScenarioWindow, SelectHdl, weld::TreeView&, void)
{
    if (const ScenarioEntry* pEntry = GetSelectedScenarioEntry())
        SetComment(pEntry->maComment);
}

IMPL_LINK_NOARG(ScScenarioWindow, DoubleClickHdl, weld::TreeView&, bool)
{
    SelectScenario();
    return true;
}

IMPL_LINK(ScScenarioWindow, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;

    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    switch( aCode.GetCode() )
    {
        case KEY_RETURN:
            SelectScenario();
            bHandled = true;
        break;
        case KEY_DELETE:
            DeleteScenario();
            bHandled = true;
        break;
    }

    return bHandled;
}

IMPL_LINK(ScScenarioWindow, ContextMenuHdl, const CommandEvent&, rCEvt, bool)
{
    bool bHandled = false;

    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        if (const ScenarioEntry* pEntry = GetSelectedScenarioEntry())
        {
            if (!pEntry->mbProtected)
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xLbScenario.get(), u"modules/scalc/ui/scenariomenu.ui"_ustr));
                std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu(u"menu"_ustr));
                OUString sIdent(xPopup->popup_at_rect(m_xLbScenario.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1))));
                if (sIdent == "delete")
                    DeleteScenario();
                else if (sIdent == "edit")
                    EditScenario();
            }
        }
        bHandled = true;
    }

    return bHandled;
}

const ScScenarioWindow::ScenarioEntry* ScScenarioWindow::GetSelectedScenarioEntry() const
{
    size_t nPos = m_xLbScenario->get_selected_index();
    return (nPos < m_aEntries.size()) ? &m_aEntries[ nPos ] : nullptr;
}

void ScScenarioWindow::ExecuteScenarioSlot(sal_uInt16 nSlotId)
{
    if( SfxViewFrame* pViewFrm = SfxViewFrame::Current() )
    {
        SfxStringItem aStringItem(nSlotId, m_xLbScenario->get_selected_text());
        pViewFrm->GetDispatcher()->ExecuteList(nSlotId,
                SfxCallMode::SLOT | SfxCallMode::RECORD, { &aStringItem } );
    }
}

void ScScenarioWindow::SelectScenario()
{
    if (m_xLbScenario->get_selected_index() != -1)
        ExecuteScenarioSlot(SID_SELECT_SCENARIO);
}

void ScScenarioWindow::EditScenario()
{
    if (m_xLbScenario->get_selected_index() != -1)
        ExecuteScenarioSlot(SID_EDIT_SCENARIO);
}

void ScScenarioWindow::DeleteScenario()
{
    if (m_xLbScenario->get_selected_index() != -1)
    {
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xLbScenario.get(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       ScResId(STR_QUERY_DELSCENARIO)));
        xQueryBox->set_default_response(RET_YES);
        if (xQueryBox->run() == RET_YES)
            ExecuteScenarioSlot(SID_DELETE_SCENARIO);
    }
}

// class ScScenarioWindow ------------------------------------------------

ScScenarioWindow::ScScenarioWindow(weld::Builder& rBuilder, const OUString& aQH_List,
                                   const OUString& aQH_Comment)
    : m_xLbScenario(rBuilder.weld_tree_view(u"scenariolist"_ustr))
    , m_xEdComment(rBuilder.weld_text_view(u"scenariotext"_ustr))
{
    m_xLbScenario->set_help_id(HID_SC_SCENWIN_TOP);
    m_xEdComment->set_help_id(HID_SC_SCENWIN_BOTTOM);

    m_xLbScenario->set_tooltip_text(aQH_List);
    m_xEdComment->set_tooltip_text(aQH_Comment);

    m_xLbScenario->connect_changed(LINK(this, ScScenarioWindow, SelectHdl));
    m_xLbScenario->connect_row_activated(LINK(this, ScScenarioWindow, DoubleClickHdl));
    m_xLbScenario->connect_key_press(LINK(this, ScScenarioWindow, KeyInputHdl));
    m_xLbScenario->connect_popup_menu(LINK(this, ScScenarioWindow, ContextMenuHdl));

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm)
    {
        SfxBindings& rBindings = pViewFrm->GetBindings();
        rBindings.Invalidate( SID_SELECT_SCENARIO );
        rBindings.Update( SID_SELECT_SCENARIO );
    }
}

ScScenarioWindow::~ScScenarioWindow()
{
}

void ScScenarioWindow::NotifyState( const SfxPoolItem* pState )
{
    if( pState )
    {
        m_xLbScenario->set_sensitive(true);

        if ( auto pStringItem = dynamic_cast<const SfxStringItem*>( pState) )
        {
            const OUString& aNewEntry( pStringItem->GetValue() );

            if (!aNewEntry.isEmpty())
                m_xLbScenario->select_text(aNewEntry);
            else
                m_xLbScenario->unselect_all();
        }
        else if ( auto pStringListItem = dynamic_cast<const SfxStringListItem*>( pState) )
        {
            UpdateEntries(pStringListItem->GetList());
        }
    }
    else
    {
        m_xLbScenario->set_sensitive(false);
        m_xLbScenario->unselect_all();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
