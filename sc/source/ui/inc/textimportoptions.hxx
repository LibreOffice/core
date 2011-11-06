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



#ifndef SC_UI_IMPORTOPTIONS_HXX
#define SC_UI_IMPORTOPTIONS_HXX

#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "i18npool/lang.h"
#include "svx/langbox.hxx"

class ScTextImportOptionsDlg : public ModalDialog
{
public:
    ScTextImportOptionsDlg(Window* pParent);
    virtual ~ScTextImportOptionsDlg();

    virtual short Execute();

    LanguageType getLanguageType() const;
    bool isDateConversionSet() const;

private:
    void init();

private:
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    FixedLine       maFlChooseLang;

    RadioButton     maRbAutomatic;
    RadioButton     maRbCustom;

    SvxLanguageBox  maLbCustomLang;

    FixedLine       maFlOption;

    CheckBox        maBtnConvertDate;

    DECL_LINK( OKHdl, OKButton* );

    DECL_LINK( RadioHdl, RadioButton* );
};


#endif
