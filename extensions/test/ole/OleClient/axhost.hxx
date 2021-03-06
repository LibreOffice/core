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

#pragma once

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>


class HostWin: public CWindowImpl<HostWin, CWindow,
               CWinTraits< WS_CAPTION|WS_POPUPWINDOW|WS_VISIBLE, 0> >
{
    CComBSTR controlName;
    CComPtr<IUnknown> spControl;
public:
    explicit HostWin(LPWSTR progid);

    ~HostWin();


    BEGIN_MSG_MAP(HostWin)
        MESSAGE_HANDLER( WM_CREATE, OnCreate)
    END_MSG_MAP()

    IUnknown* GetHostedControl(){
        return spControl;
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
