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

#ifndef _CUI_IMAPWND_HXX
#define _CUI_IMAPWND_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svl/itempool.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/transfer.hxx>
#include <svtools/imap.hxx>
#include <sfx2/frame.hxx>
#include <svtools/svmedit.hxx>


/*************************************************************************
|*
|*
|*
\************************************************************************/

class URLDlg : public ModalDialog
{
    FixedText           maFtURL;
    Edit                maEdtURL;
    FixedText           maFtTarget;
    ComboBox            maCbbTargets;
    FixedText           maFtName;
    Edit                maEdtName;
    FixedText           maFtAlternativeText;
    Edit                maEdtAlternativeText;
    FixedText           maFtDescription;
    MultiLineEdit       maEdtDescription;
    FixedLine           maFlURL;
    HelpButton          maBtnHelp;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;

public:

                        URLDlg( Window* pWindow,
                                const String& rURL, const String& rAlternativeText, const String& rDescription,
                                const String& rTarget, const String& rName,
                                TargetList& rTargetList );

    String              GetURL() const { return maEdtURL.GetText(); }
    String              GetAltText() const { return maEdtAlternativeText.GetText(); }
    String              GetDesc() const { return maEdtDescription.GetText(); }
    String              GetTarget() const { return maCbbTargets.GetText(); }
    String              GetName() const { return maEdtName.GetText(); }
};

#endif
