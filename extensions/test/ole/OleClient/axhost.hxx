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



#ifndef  __AXHOST_HXX
#define __AXHOST_HXX

#include <atlbase.h>
extern CComModule _Module;
//#include <atlwin.h>
#include <atlcom.h>
#include <atlhost.h>


class HostWin: public CWindowImpl<HostWin, CWindow,
               CWinTraits< WS_CAPTION|WS_POPUPWINDOW|WS_VISIBLE, 0> >
{
    CComBSTR controlName;
    CComPtr<IUnknown> spControl;
public:
    HostWin(LPWSTR progid);

    ~HostWin();


    BEGIN_MSG_MAP(HostWin)
        MESSAGE_HANDLER( WM_CREATE, OnCreate)
    END_MSG_MAP()

    IUnknown* GetHostedControl(){
        return spControl;
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
#endif


