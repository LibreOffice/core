/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/asyncfunc.hxx>

AsyncFunc::AsyncFunc(const std::function<void()>& rAsyncFunc)
    : m_pAsyncFunc(rAsyncFunc)
{
}

AsyncFunc::~AsyncFunc() {}

void AsyncFunc::Execute()
{
    if (m_pAsyncFunc)
        m_pAsyncFunc();
}

//XUnoTunnel
UNO3_GETIMPLEMENTATION_IMPL(AsyncFunc)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
