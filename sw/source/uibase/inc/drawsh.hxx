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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DRAWSH_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DRAWSH_HXX

#include "drwbassh.hxx"

class SwDrawBaseShell;

class SwDrawShell: public SwDrawBaseShell
{
public:
    SFX_DECL_INTERFACE(SW_DRAWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                SwDrawShell(SwView &rView);

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        ExecDrawDlg(SfxRequest& rReq);
    void        ExecDrawAttrArgs(SfxRequest& rReq);
    void        GetDrawAttrState(SfxItemSet &rSet);

    void        ExecFormText(SfxRequest& rReq);
    void        GetFormTextState(SfxItemSet& rSet);

    // #i123922# added helper methods to handle applying graphic data to draw objects
    SdrObject* IsSingleFillableNonOLESelected();
    void InsertPictureFromFile(SdrObject& rObject);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
