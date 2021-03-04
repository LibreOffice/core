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

#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sidebar/ControllerFactory.hxx>
#include <sidebar/Tools.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/frame/XDispatch.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

PanelTitleBar::PanelTitleBar(const OUString& rsTitle,
                             vcl::Window* pParentWindow,
                             Panel* pPanel)
    : TitleBar(pParentWindow, "sfx/ui/paneltitlebar.ui", "PanelTitleBar",
               Theme::Color_PanelTitleBarBackground),
      mxExpander(m_xBuilder->weld_expander("expander")),
      mpPanel(pPanel),
      mxFrame(),
      msIdent("button"),
      msMoreOptionsCommand()
{
    mxExpander->set_label(rsTitle);
    mxExpander->connect_expanded(LINK(this, PanelTitleBar, ExpandHdl));

    assert(mpPanel);

    UpdateExpandedState();

#ifdef DEBUG
    SetText(OUString("PanelTitleBar"));
#endif
}

void PanelTitleBar::SetTitle(const OUString& rsTitle)
{
    mxExpander->set_label(rsTitle);
}

OUString PanelTitleBar::GetTitle() const
{
    return mxExpander->get_label();
}

void PanelTitleBar::UpdateExpandedState()
{
    mxExpander->set_expanded(mpPanel->IsExpanded());
}

PanelTitleBar::~PanelTitleBar()
{
    disposeOnce();
}

void PanelTitleBar::dispose()
{
    Reference<lang::XComponent> xComponent(mxController, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    mxController.clear();
    mpPanel.clear();
    mxExpander.reset();
    TitleBar::dispose();
}

void PanelTitleBar::SetMoreOptionsCommand(const OUString& rsCommandName,
                                          const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                          const css::uno::Reference<css::frame::XController>& rxController)
{
    if (rsCommandName == msMoreOptionsCommand)
        return;

    if (!msMoreOptionsCommand.isEmpty())
        mxToolBox->hide();

    msMoreOptionsCommand = rsCommandName;
    mxFrame = rxFrame;

    if (msMoreOptionsCommand.isEmpty())
        return;

    msIdent = msMoreOptionsCommand.toUtf8();
    mxToolBox->set_item_ident(0, msIdent);

    Reference<lang::XComponent> xComponent(mxController, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    mxController =
        ControllerFactory::CreateToolBoxController(
            *mxToolBox, *m_xBuilder, msMoreOptionsCommand, rxFrame, rxController, true);

    mxToolBox->show();
    mxToolBox->set_item_icon_name(msIdent, "sfx2/res/symphony/morebutton.png");
    mxToolBox->set_item_tooltip_text(msIdent, SfxResId(SFX_STR_SIDEBAR_MORE_OPTIONS));
}

void PanelTitleBar::HandleToolBoxItemClick()
{
    if (!mxController)
        return;
    mxController->click();
    mxController->execute(0);
}

IMPL_LINK(PanelTitleBar, ExpandHdl, weld::Expander&, rExpander, void)
{
    if (!mpPanel)
        return;
    mpPanel->SetExpanded(rExpander.get_expanded());
}

void PanelTitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    mxToolBox->set_item_icon_name(msIdent, "sfx2/res/symphony/morebutton.png");
    TitleBar::DataChanged(rEvent);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
