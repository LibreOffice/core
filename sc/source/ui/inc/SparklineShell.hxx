/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/shell.hxx>

#include <shellids.hxx>

class ScTabViewShell;
class SfxModule;

namespace sc
{
/** Shell to handle the sparkline context */
class SparklineShell final : public SfxShell
{
public:
    SFX_DECL_INTERFACE(SCID_SPARKLINE_SHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SparklineShell(ScTabViewShell* pView);
    virtual ~SparklineShell() override;

private:
    ScTabViewShell* m_pViewShell;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
