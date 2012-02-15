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




#ifndef _SD_DLGFIELD_HXX
#define _SD_DLGFIELD_HXX


#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <svx/langbox.hxx>
class SvxFieldData;

/*************************************************************************
|*
|* Dialog zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

class SdModifyFieldDlg : public ModalDialog
{
private:
    FixedLine           aGrpType;
    RadioButton         aRbtFix;
    RadioButton         aRbtVar;
    FixedText           maFtLanguage;
    SvxLanguageBox      maLbLanguage;
    FixedText           aFtFormat;
    ListBox             aLbFormat;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    SfxItemSet          maInputSet;

    const SvxFieldData* pField;

    void                FillFormatList();
    void                FillControls();

    DECL_LINK( LanguageChangeHdl, void * );

public:
    SdModifyFieldDlg( Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet );
    ~SdModifyFieldDlg() {}

    SvxFieldData*       GetField();
    SfxItemSet          GetItemSet();
};



#endif      // _SD_DLGFIELD_HXX
