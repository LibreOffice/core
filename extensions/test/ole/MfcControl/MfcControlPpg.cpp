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
// MfcControlPpg.cpp : Implementation of the CMfcControlPropPage property page class.

#include "stdafx.h"
#include "MfcControl.h"
#include "MfcControlPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMfcControlPropPage, COlePropertyPage)



// Message map

BEGIN_MESSAGE_MAP(CMfcControlPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CMfcControlPropPage)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMfcControlPropPage, "MFCCONTROL.MfcControlPropPage.1",
    0xac221fb7, 0xa0d8, 0x11d4, 0x83, 0x3b, 0, 0x50, 0x4, 0x52, 0x6a, 0xb4)



// CMfcControlPropPage::CMfcControlPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CMfcControlPropPage

BOOL CMfcControlPropPage::CMfcControlPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_MFCCONTROL_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}



// CMfcControlPropPage::CMfcControlPropPage - Constructor

CMfcControlPropPage::CMfcControlPropPage() :
    COlePropertyPage(IDD, IDS_MFCCONTROL_PPG_CAPTION)
{
    //{{AFX_DATA_INIT(CMfcControlPropPage)
    // NOTE: ClassWizard will add member initialization here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DATA_INIT
}



// CMfcControlPropPage::DoDataExchange - Moves data between page and properties

void CMfcControlPropPage::DoDataExchange(CDataExchange* pDX)
{
    //{{AFX_DATA_MAP(CMfcControlPropPage)
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}



// CMfcControlPropPage message handlers

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
