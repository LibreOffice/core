/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_extensions.h>

#include "dp_gui.h"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_extensionboxwithbuttons.hxx"

#include <dp_shared.hxx>
#include <strings.hrc>

#include <officecfg/Office/ExtensionManager.hxx>
#include <sal/log.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Menu.hxx>

#include <memory>
#include <utility>

using namespace ::com::sun::star;

namespace dp_gui {

ExtensionBoxWithButtons::ExtensionBoxWithButtons(ExtMgrDialog& rParentDialog,
                                                 std::unique_ptr<weld::ScrolledWindow> xScroll,
                                                 TheExtensionManager& rManager)
    : ExtensionBox(std::move(xScroll), rManager)
    , m_bInterfaceLocked(false)
    , m_rParent(rParentDialog)
{
}

void ExtensionBoxWithButtons::RecalcAll()
{
    const sal_Int32 nActive = getSelIndex();

    if (nActive != ExtensionBox::ENTRY_NOTFOUND)
    {
        SetButtonStatus( GetEntryData( nActive) );
    }
    else
    {
        m_rParent.enableOptionsButton(false);
        m_rParent.enableRemoveButton(false);
        m_rParent.enableEnableButton(false);
    }

    ExtensionBox::RecalcAll();
}


//This function may be called with nPos < 0
void ExtensionBoxWithButtons::selectEntry(const tools::Long nPos)
{
    if ( HasActive() && ( nPos == getSelIndex() ) )
        return;

    ExtensionBox::selectEntry(nPos);
}

void ExtensionBoxWithButtons::SetButtonStatus(const TEntry_Impl& rEntry)
{
    bool bShowOptionBtn = true;

    rEntry->m_bHasButtons = false;
    if ((rEntry->m_eState == PackageState::REGISTERED)
        || (rEntry->m_eState == PackageState::NOT_AVAILABLE))
    {
        m_rParent.enableButtontoEnable(false);
    }
    else
    {
        m_rParent.enableButtontoEnable(true);
        bShowOptionBtn = false;
    }

    if ((!rEntry->m_bUser || (rEntry->m_eState == PackageState::NOT_AVAILABLE)
         || rEntry->m_bMissingDeps)
        && !rEntry->m_bMissingLic)
    {
        m_rParent.enableEnableButton(false);
    }
    else
    {
        m_rParent.enableEnableButton(!rEntry->m_bLocked);
        rEntry->m_bHasButtons = true;
    }

    if ( rEntry->m_bHasOptions && bShowOptionBtn )
    {
        m_rParent.enableOptionsButton(true);
        rEntry->m_bHasButtons = true;
    }
    else
    {
        m_rParent.enableOptionsButton(false);
    }

    if ( rEntry->m_bUser || rEntry->m_bShared )
    {
        m_rParent.enableRemoveButton(!rEntry->m_bLocked);
        rEntry->m_bHasButtons = true;
    }
    else
    {
        m_rParent.enableRemoveButton(false);
    }
}

bool ExtensionBoxWithButtons::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return ExtensionBox::Command(rCEvt);

    tools::Long nIndex = -1;
    Point aPos;
    if (rCEvt.IsMouseEvent())
    {
        aPos = rCEvt.GetMousePosPixel();
        nIndex = PointToPos(aPos);
    }
    else
    {
        nIndex = getSelIndex();
        if (nIndex >= 0)
            aPos = GetEntryRect(nIndex).Center();
    }
    if (nIndex < 0)
        return false;

    const OUString sCommand = ShowPopupMenu(aPos, nIndex);
    if (sCommand == "CMD_ENABLE")
        m_rParent.enablePackage(GetEntryData(nIndex)->m_xPackage, true);
    else if (sCommand == "CMD_DISABLE")
        m_rParent.enablePackage(GetEntryData(nIndex)->m_xPackage, false);
    else if (sCommand == "CMD_UPDATE")
        m_rParent.updatePackage(GetEntryData(nIndex)->m_xPackage);
    else if (sCommand == "CMD_REMOVE")
        m_rParent.removePackage(GetEntryData(nIndex)->m_xPackage);
    else if (sCommand == "CMD_SHOW_LICENSE")
    {
        m_rParent.incBusy();
        ShowLicenseDialog aLicenseDlg(m_rParent.getDialog(), GetEntryData(nIndex)->m_xPackage);
        aLicenseDlg.run();
        m_rParent.decBusy();
    }

    return true;
}

OUString ExtensionBoxWithButtons::ShowPopupMenu(const Point& rPos, const tools::Long nPos)
{
    if (nPos >= GetEntryCount())
        return u"CMD_NONE"_ustr;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, u"desktop/ui/extensionmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu(u"menu"_ustr));

#if ENABLE_EXTENSION_UPDATE
    xPopup->append(u"CMD_UPDATE"_ustr, DpResId( RID_CTX_ITEM_CHECK_UPDATE ) );
#endif

    if ( ! GetEntryData( nPos )->m_bLocked )
    {
        if ( GetEntryData( nPos )->m_bUser )
        {
            if (GetEntryData(nPos)->m_eState == PackageState::REGISTERED)
                xPopup->append(u"CMD_DISABLE"_ustr, DpResId(RID_CTX_ITEM_DISABLE));
            else if (GetEntryData(nPos)->m_eState != PackageState::NOT_AVAILABLE)
                xPopup->append(u"CMD_ENABLE"_ustr, DpResId(RID_CTX_ITEM_ENABLE));
        }
        if (!officecfg::Office::ExtensionManager::ExtensionSecurity::DisableExtensionRemoval::get())
        {
            xPopup->append(u"CMD_REMOVE"_ustr, DpResId(RID_CTX_ITEM_REMOVE));
        }
    }

    if ( !GetEntryData( nPos )->m_sLicenseText.isEmpty() )
        xPopup->append(u"CMD_SHOW_LICENSE"_ustr, DpResId(RID_STR_SHOW_LICENSE_CMD));

    return xPopup->popup_at_rect(GetDrawingArea(), tools::Rectangle(rPos, Size(1, 1)));
}

bool ExtensionBoxWithButtons::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (m_bInterfaceLocked)
        return false;
    return ExtensionBox::MouseButtonDown(rMEvt);
}

void ExtensionBoxWithButtons::enableButtons(bool bEnable)
{
    m_bInterfaceLocked = ! bEnable;

    if ( bEnable )
    {
        sal_Int32 nIndex = getSelIndex();
        if (nIndex != ExtensionBox::ENTRY_NOTFOUND)
            SetButtonStatus( GetEntryData( nIndex ) );
    }
    else
    {
        m_rParent.enableEnableButton(false);
        m_rParent.enableOptionsButton(false);
        m_rParent.enableRemoveButton(false);
    }
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
