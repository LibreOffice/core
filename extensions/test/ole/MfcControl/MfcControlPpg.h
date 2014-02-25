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
#if !defined(AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED_)
#define AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED_

#ifdef _MSC_VER
#pragma once
#endif

// MfcControlPpg.h : Declaration of the CMfcControlPropPage property page class.


// CMfcControlPropPage : See MfcControlPpg.cpp.cpp for implementation.

class CMfcControlPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CMfcControlPropPage)
    DECLARE_OLECREATE_EX(CMfcControlPropPage)

// Constructor
public:
    CMfcControlPropPage();

// Dialog Data
    //{{AFX_DATA(CMfcControlPropPage)
    enum { IDD = IDD_PROPPAGE_MFCCONTROL };
        // NOTE - ClassWizard will add data members here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CMfcControlPropPage)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
