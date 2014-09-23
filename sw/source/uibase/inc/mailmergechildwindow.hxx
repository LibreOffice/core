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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_MAILMERGECHILDWINDOW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_MAILMERGECHILDWINDOW_HXX
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/toolbox.hxx>
#include "swdllapi.h"

class SwMailMergeChildWin : public SfxFloatingWindow
{
    ToolBox* m_pBackTB;
    DECL_LINK( BackHdl, void* );

public:
    SwMailMergeChildWin(SfxBindings*, SfxChildWindow*, vcl::Window *pParent);

    virtual void FillInfo(SfxChildWinInfo&) const SAL_OVERRIDE;
};

class SwMailMergeChildWindow : public SfxChildWindow
{
public:
    SwMailMergeChildWindow( vcl::Window* ,
                    sal_uInt16 nId,
                    SfxBindings*,
                    SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW( SwMailMergeChildWindow );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
