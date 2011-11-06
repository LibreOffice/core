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



#ifndef _SV_TABDLG_HXX
#define _SV_TABDLG_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/dialog.hxx>

class FixedLine;
class TabControl;

// ----------------------
// - TabDialog -
// ----------------------

class VCL_DLLPUBLIC TabDialog : public Dialog
{
private:
    FixedLine*          mpFixedLine;
    Window*             mpViewWindow;
    WindowAlign         meViewAlign;
    sal_Bool                mbPosControls;

    SAL_DLLPRIVATE void ImplInitTabDialogData();
    SAL_DLLPRIVATE void ImplPosControls();

public:
                        TabDialog( Window* pParent,
                                   WinBits nStyle = WB_STDTABDIALOG );
                        TabDialog( Window* pParent, const ResId& rResId );
                        ~TabDialog();

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );

    void                AdjustLayout();

    void                SetViewWindow( Window* pWindow ) { mpViewWindow = pWindow; }
    Window*             GetViewWindow() const { return mpViewWindow; }
    void                SetViewAlign( WindowAlign eAlign ) { meViewAlign = eAlign; }
    WindowAlign         GetViewAlign() const { return meViewAlign; }
};

#endif  // _SV_TABDLG_HXX
