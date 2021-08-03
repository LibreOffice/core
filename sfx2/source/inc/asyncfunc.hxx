/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>

class AsyncFunc final : public cppu::WeakImplHelper<css::lang::XUnoTunnel>
{
private:
    std::function<void()> m_pAsyncFunc;

public:
    AsyncFunc(const std::function<void()>&);
    virtual ~AsyncFunc() override;

    void Execute();

    //XUnoTunnel
    UNO3_GETIMPLEMENTATION_DECL(AsyncFunc)
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
