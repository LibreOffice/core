/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <sal/config.h>
#include <svx/svxdllapi.h>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <vcl/customweld.hxx>
#include <vcl/vclptr.hxx>

class VirtualDevice;

/* Client-rendered counterpart of WeldEditView */
class SVX_DLLPUBLIC DummyEditView : public weld::CustomClientWidgetController,
                                    public EditViewCallbacks
{
public:
    DummyEditView();
    virtual ~DummyEditView() override;

protected:
    VclPtr<VirtualDevice> m_xVirDev;
    std::unique_ptr<EditEngine> m_xEditEngine;
    std::unique_ptr<EditView> m_xEditView;

    virtual void EditViewInvalidate(const tools::Rectangle&) override {}
    virtual void EditViewSelectionChange() override {}
    virtual OutputDevice& EditViewOutputDevice() const override;
    virtual Point EditViewPointerPosPixel() const override { return Point(); }
    virtual void EditViewInputContext(const InputContext&) override {}
    virtual void EditViewCursorRect(const tools::Rectangle&, int) override {}
    virtual css::uno::Reference<css::datatransfer::clipboard::XClipboard>
    GetClipboard() const override
    {
        return {};
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
