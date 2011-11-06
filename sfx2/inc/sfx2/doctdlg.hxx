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


#ifndef _SFXDOCTDLG_HXX
#define _SFXDOCTDLG_HXX

#include <sfx2/doctempl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

class SfxModalDefParentHelper;

class SfxDocumentTemplateDlg : public ModalDialog
{
private:

    FixedLine       aEditFL;
    Edit            aNameEd;
    FixedLine       aTemplateFL;
    FixedText       aRegionFt;
    ListBox         aRegionLb;
    FixedText       aTemplateFt;
    ListBox         aTemplateLb;

    OKButton        aOkBt;
    CancelButton    aCancelBt;
    HelpButton      aHelpBt;
    PushButton      aEditBt;
    PushButton      aOrganizeBt;

    SfxDocumentTemplates *pTemplates;

    SfxModalDefParentHelper* pHelper;

    void Init();

    DECL_LINK( OrganizeHdl, Button * );
    DECL_LINK( OkHdl, Control * );
    DECL_LINK( RegionSelect, ListBox * );
    DECL_LINK( TemplateSelect, ListBox * );
    DECL_LINK( NameModify, Edit * );
    DECL_LINK( EditHdl, Button * );

public:
    SfxDocumentTemplateDlg(Window * pParent, SfxDocumentTemplates* pTempl );
    ~SfxDocumentTemplateDlg();

    String GetTemplateName() const
    { return aNameEd.GetText().EraseLeadingChars(); }
    String GetTemplatePath();
    void NewTemplate(const String &rPath);
    sal_uInt16 GetRegion() const { return aRegionLb.GetSelectEntryPos(); }
    String GetRegionName() const { return aRegionLb.GetSelectEntry(); }
};

#endif

