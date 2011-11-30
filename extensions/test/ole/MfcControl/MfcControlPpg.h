/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#if !defined(AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED_)
#define AFX_MFCCONTROLPPG_H__AC221FC5_A0D8_11D4_833B_005004526AB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MfcControlPpg.h : Declaration of the CMfcControlPropPage property page class.

////////////////////////////////////////////////////////////////////////////
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
