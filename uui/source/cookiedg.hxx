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



#ifndef UUI_COOKIEDG_HXX
#define UUI_COOKIEDG_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>

struct CntHTTPCookieRequest;

//============================================================================
class CookiesDialog : public ModalDialog
{
    FixedBitmap             maCookieFB;
    FixedText               maCookieFT;
    FixedLine               maInFutureLine;
    RadioButton             maInFutureSendBtn;
    RadioButton             maInFutureIgnoreBtn;
    RadioButton             maInFutureInteractiveBtn;
    GroupBox                maInFutureGB;
    PushButton              maIgnoreBtn;
    PushButton              maSendBtn;

    CntHTTPCookieRequest*   mpCookieRequest;

    DECL_LINK( ButtonHdl_Impl, PushButton * );

public:
    CookiesDialog( Window* pParent, CntHTTPCookieRequest* pRequest,
                   ResMgr* pResMgr );

    virtual short Execute();
};

#endif // UUI_COOKIEDG_HXX

