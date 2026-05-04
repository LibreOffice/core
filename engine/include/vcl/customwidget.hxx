/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <rtl/ustring.hxx>
#include <vcl/ctrl.hxx>

namespace tools
{
class JsonWriter;
}

/**
 * VCL widget for client-rendered custom widgets.
 *
 * VclCustomWidget produces JSON via DumpAsPropertyTree(). The client
 * receives the JSON and renders the widget natively.
 */
class VCL_DLLPUBLIC VclCustomWidget final : public Control
{
public:
    using DumpCallback = std::function<void(tools::JsonWriter& /*rWriter*/)>;

private:
    OUString m_aCustomType;
    DumpCallback m_aDumpCallback;

public:
    VclCustomWidget(vcl::Window* pParent, WinBits nStyle = WB_TABSTOP);
    virtual ~VclCustomWidget() override;

    virtual void DumpAsPropertyTree(tools::JsonWriter& rJsonWriter) override;

    void SetCustomType(const OUString& rType) { m_aCustomType = rType; }
    const OUString& GetCustomType() const { return m_aCustomType; }

    void SetDumpCallback(const DumpCallback& rCallback) { m_aDumpCallback = rCallback; }

    virtual FactoryFunction GetUITestFactory() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
