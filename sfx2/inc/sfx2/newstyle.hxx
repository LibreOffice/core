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


#ifndef _NEWSTYLE_HXX
#define _NEWSTYLE_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

class SfxStyleSheetBasePool;

class SFX2_DLLPUBLIC SfxNewStyleDlg : public ModalDialog
{
private:
    FixedLine               aColFL;
    ComboBox                aColBox;
    OKButton                aOKBtn;
    CancelButton            aCancelBtn;

    QueryBox                aQueryOverwriteBox;
    SfxStyleSheetBasePool&  rPool;

//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( OKHdl, Control * );
    DECL_DLLPRIVATE_LINK( ModifyHdl, ComboBox * );
//#endif

public:
    SfxNewStyleDlg( Window* pParent, SfxStyleSheetBasePool& );
    ~SfxNewStyleDlg();

    String                  GetName() const { return aColBox.GetText().EraseLeadingChars(); }
};

#endif

