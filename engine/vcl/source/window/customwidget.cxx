/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/customwidget.hxx>
#include <tools/json_writer.hxx>

VclCustomWidget::VclCustomWidget(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle)
{
}

VclCustomWidget::~VclCustomWidget() {}

void VclCustomWidget::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    Control::DumpAsPropertyTree(rJsonWriter);

    if (m_aCustomType.isEmpty() || !m_aDumpCallback)
        return;

    rJsonWriter.put("type", "customwidget");
    rJsonWriter.put("customType", m_aCustomType);

    auto aDataNode = rJsonWriter.startNode("data");
    m_aDumpCallback(rJsonWriter);
}

FactoryFunction VclCustomWidget::GetUITestFactory() const { return nullptr; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
