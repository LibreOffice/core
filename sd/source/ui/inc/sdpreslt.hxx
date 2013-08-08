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



#ifndef SD_PRES_LAYOUT_DLG_HXX
#define SD_PRES_LAYOUT_DLG_HXX

#include <vcl/dialog.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>

class SfxItemSet;

namespace sd {
class DrawDocShell;
class ViewShell;
}



class SdPresLayoutDlg
    : public ModalDialog
{
public:
    SdPresLayoutDlg(
        ::sd::DrawDocShell* pDocShell,
        ::sd::ViewShell* pViewShell,
        ::Window* pWindow,
        const SfxItemSet& rInAttrs);

    virtual ~SdPresLayoutDlg (void);

    void                GetAttr(SfxItemSet& rOutAttrs);

    DECL_LINK(ClickLayoutHdl, void *);
    DECL_LINK(ClickLoadHdl, void *);

private:
    ::sd::DrawDocShell* mpDocSh;
    ::sd::ViewShell*    mpViewSh;
    FixedText           maFtLayout;
    ValueSet            maVS;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    CheckBox            maCbxMasterPage;
    CheckBox            maCbxCheckMasters;
    PushButton          maBtnLoad;

    const SfxItemSet&   mrOutAttrs;

    std::vector< String >   maLayoutNames;
    // List*               mpLayoutNames;

    String              maName;          // Layoutname oder Dateiname
    sal_uInt32          mnLayoutCount;  // Anzahl, der im Dokument vorhandenen MasterPages
    const String        maStrNone;

    void                FillValueSet();
    void                Reset();
};

#endif

