/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>

#include "basesh.hxx"
#include <shellid.hxx>

class SwNavigationShell: public SwBaseShell
{
public:
    SFX_DECL_INTERFACE(SW_NAVIGATIONSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SwNavigationShell(SwView &rView);

    void GetState(SfxItemSet &);
    void Execute(SfxRequest const &);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
