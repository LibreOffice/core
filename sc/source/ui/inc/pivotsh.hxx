/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PIVOTSH_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PIVOTSH_HXX

#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>

#include "shellids.hxx"

class ScTabViewShell;
class ScDPObject;

class ScPivotShell : public SfxShell
{
public:
    SFX_DECL_INTERFACE(SCID_PIVOT_SHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    ScPivotShell(ScTabViewShell* pView);
    virtual ~ScPivotShell();

    void Execute(SfxRequest& rReq);
    void GetState(SfxItemSet& rSet);

private:
    ScTabViewShell* pViewShell;

    ScDPObject*     GetCurrDPObject();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
