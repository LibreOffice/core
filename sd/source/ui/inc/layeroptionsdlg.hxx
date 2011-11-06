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




#ifndef _SD_LAYER_DLG_HXX_
#define _SD_LAYER_DLG_HXX_

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/svmedit.hxx>
#include "sdresid.hxx"
#include "strings.hrc"

class SfxItemSet;

class SdInsertLayerDlg : public ModalDialog
{
private:
    FixedText           maFtName;
    Edit                maEdtName;
    FixedText           maFtTitle;
    Edit                maEdtTitle;
    FixedText           maFtDesc;
    MultiLineEdit       maEdtDesc;
    CheckBox            maCbxVisible;
    CheckBox            maCbxPrintable;
    CheckBox            maCbxLocked;
    FixedLine           maFixedLine;
    HelpButton          maBtnHelp;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;

    const SfxItemSet&   mrOutAttrs;

public:

    SdInsertLayerDlg( Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, String aStr );
    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // _SD_LAYER_DLG_HXX_
