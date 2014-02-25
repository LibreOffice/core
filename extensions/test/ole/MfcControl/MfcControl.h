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
#if !defined(AFX_MFCCONTROL_H__AC221FBC_A0D8_11D4_833B_005004526AB4__INCLUDED_)
#define AFX_MFCCONTROL_H__AC221FBC_A0D8_11D4_833B_005004526AB4__INCLUDED_

#ifdef _MSC_VER
#pragma once
#endif

// MfcControl.h : main header file for MFCCONTROL.DLL

#if !defined( __AFXCTL_H__ )
    #error include 'afxctl.h' before including this file
#endif

#include "resource.h"


// CMfcControlApp : See MfcControl.cpp for implementation.

class CMfcControlApp : public COleControlModule
{
public:
    BOOL InitInstance();
    int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCCONTROL_H__AC221FBC_A0D8_11D4_833B_005004526AB4__INCLUDED)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
