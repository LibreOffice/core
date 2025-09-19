/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basctl/idecodecompletiontypes.hxx>

#include <memory>
#include <tools/link.hxx>

namespace basctl
{
class IdeDataProviderInterface
{
public:
    virtual ~IdeDataProviderInterface() = default;
    virtual void AsyncInitialize(const Link<void*, void>& rFinishCallback) = 0;
    virtual bool IsInitialized() const = 0;
};

class IdeDataProvider : public IdeDataProviderInterface
{
public:
    IdeDataProvider();
    ~IdeDataProvider() override;
    void AsyncInitialize(const Link<void*, void>& rFinishCallback) override;
    bool IsInitialized() const override;

private:
    bool m_bInitialized = false;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
