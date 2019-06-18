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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FRMSH_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FRMSH_HXX

#include "basesh.hxx"

class SwFrameShell: public SwBaseShell
{
public:
    SFX_DECL_INTERFACE(SW_FRAMESHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
            SwFrameShell(SwView &rView);
    virtual ~SwFrameShell() override;

    void    Execute(SfxRequest &);
    void    ExecMove(SfxRequest& rReq);
    void    ExecField(const SfxRequest& rReq);
    void    GetState(SfxItemSet &);
    void    ExecFrameStyle(SfxRequest const & rReq);
    void    GetLineStyleState(SfxItemSet &rSet);
    void    StateInsert(SfxItemSet &rSet);

    void GetDrawAttrStateTextFrame(SfxItemSet &rSet);
    void ExecDrawAttrArgsTextFrame(SfxRequest const & rReq);

    void ExecDrawDlgTextFrame(SfxRequest const & rReq);
    void DisableStateTextFrame(SfxItemSet &rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
