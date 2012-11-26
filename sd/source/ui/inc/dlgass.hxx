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



#ifndef INC_DLGASS
#define INC_DLGASS

#ifndef _GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#include <svtools/svmedit.hxx>
#include "assclass.hxx"
#include <vcl/lstbox.hxx>
#include <sfx2/app.hxx>

#include "sdenumdef.hxx"

class AssistentDlgImpl;

class AssistentDlg:public ModalDialog
{
private:
    AssistentDlgImpl* mpImpl;

public:
    AssistentDlg(Window* pParent, bool bAutoPilot);
    ~AssistentDlg();

    DECL_LINK( FinishHdl, OKButton * );

    SfxObjectShellLock GetDocument();
    OutputType GetOutputMedium() const;
    bool IsSummary() const;
    StartType GetStartType() const;
    String GetDocPath() const;
    bool GetStartWithFlag() const;

    bool IsDocEmpty() const;
    String GetPassword();
};

#endif
