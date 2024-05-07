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

#include <core_resource.hxx>
#include <vcl/svapp.hxx>
#include "AppTitleWindow.hxx"

namespace dbaui
{
OTitleWindow::OTitleWindow(weld::Container* pParent, TranslateId pTitleId)
    : m_xBuilder(Application::CreateBuilder(pParent, u"dbaccess/ui/titlewindow.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"TitleWindow"_ustr))
    , m_xTitleFrame(m_xBuilder->weld_container(u"titleparent"_ustr))
    , m_xTitle(m_xBuilder->weld_label(u"title"_ustr))
    , m_xChildContainer(m_xBuilder->weld_container(u"box"_ustr))
{
    setTitle(pTitleId);

    m_xContainer->set_stack_background();
    m_xTitleFrame->set_title_background();
    m_xTitle->set_label_type(weld::LabelType::Title);
}

OTitleWindow::~OTitleWindow() {}

weld::Container* OTitleWindow::getChildContainer() { return m_xChildContainer.get(); }

void OTitleWindow::setChildWindow(const std::shared_ptr<OChildWindow>& rChild)
{
    m_xChild = rChild;
}

void OTitleWindow::setTitle(TranslateId pTitleId)
{
    if (!pTitleId)
        return;
    m_xTitle->set_label(DBA_RES(pTitleId));
}

void OTitleWindow::GrabFocus()
{
    if (m_xChild)
        m_xChild->GrabFocus();
}

bool OTitleWindow::HasChildPathFocus() const { return m_xChild && m_xChild->HasChildPathFocus(); }

} // namespace dbaui
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
