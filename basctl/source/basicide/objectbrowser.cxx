/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iderid.hxx>
#include "idetimer.hxx"
#include <objectbrowser.hxx>

#include <strings.hrc>

namespace basctl
{
ObjectBrowser::ObjectBrowser(vcl::Window* pParent)
    : basctl::DockingWindow(pParent, u""_ustr, u"ObjectBrowser"_ustr)
    , m_pDataProvider(std::make_unique<IdeDataProvider>())
{
}

ObjectBrowser::~ObjectBrowser() {}

void ObjectBrowser::Initialize() { m_pDataProvider = std::make_unique<IdeDataProvider>(); }

void ObjectBrowser::dispose()
{
    m_pDataProvider.reset();
    DockingWindow::dispose();
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
