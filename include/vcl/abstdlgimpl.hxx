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

#pragma once

#include <vcl/abstdlg.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <type_traits>

namespace vcl
{
// Templated base class storing the dialog pointer, and implementing Execute, StartExecuteAsync,
// and the shared constructor used by derived classes
template <class Base, class Dialog, template <class...> class Ptr, bool Async>
requires std::is_base_of_v<VclAbstractDialog, Base>&&
    std::is_base_of_v<weld::DialogController, Dialog> class AbstractDialogImpl_BASE : public Base
{
public:
    explicit AbstractDialogImpl_BASE(Ptr<Dialog> p)
        : m_pDlg(std::move(p))
    {
    }
    template <typename... Arg>
    requires std::is_constructible_v<Dialog, Arg...> explicit AbstractDialogImpl_BASE(Arg&&... arg)
        : m_pDlg(make_dialog(std::forward<Arg>(arg)...))
    {
    }

    short Execute() override { return m_pDlg->run(); }

    bool StartExecuteAsync(VclAbstractDialog::AsyncContext& rCtx) override
    {
        if constexpr (Async)
            return Dialog::runAsync(m_pDlg, rCtx.maEndDialogFn);
        else
            return Base::StartExecuteAsync(rCtx); // assert / fail
    }

protected:
    Ptr<Dialog> m_pDlg;

private:
    template <typename... Arg> static auto make_dialog(Arg&&... arg)
    {
        if constexpr (std::is_same_v<Ptr<void>, std::shared_ptr<void>>)
            return std::make_shared<Dialog>(std::forward<Arg>(arg)...);
        else
            return std::make_unique<Dialog>(std::forward<Arg>(arg)...);
    }
};

// Base for synchronously called dialogs, using unique_ptr
template <class Base, class Dialog>
using AbstractDialogImpl_Sync = AbstractDialogImpl_BASE<Base, Dialog, std::unique_ptr, false>;

// Base for synchronously called dialogs, using shared_ptr
template <class Base, class Dialog>
using AbstractDialogImpl_Sync_Shared
    = AbstractDialogImpl_BASE<Base, Dialog, std::shared_ptr, false>;

// Base for asynchronously called dialogs
template <class Base, class Dialog>
using AbstractDialogImpl_Async = AbstractDialogImpl_BASE<Base, Dialog, std::shared_ptr, true>;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
