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


#ifndef _CUSTOMIZEADDRESSLISTDIALOG_HXX
#define _CUSTOMIZEADDRESSLISTDIALOG_HXX
#include <sfx2/basedlgs.hxx>

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

struct SwCSVData;
/*-- 08.04.2004 14:04:39---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwCustomizeAddressListDialog : public SfxModalDialog
{
    FixedText               m_aFieldsFT;
    ListBox                 m_aFieldsLB;

    PushButton              m_aAddPB;
    PushButton              m_aDeletePB;
    PushButton              m_aRenamePB;

    ImageButton             m_aUpPB;
    ImageButton             m_aDownPB;

    FixedLine               m_aSeparatorFL;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    SwCSVData*              m_pNewData;

    DECL_LINK(AddRenameHdl_Impl, PushButton*);
    DECL_LINK(DeleteHdl_Impl, PushButton*);
    DECL_LINK(UpDownHdl_Impl, PushButton*);
    DECL_LINK(ListBoxSelectHdl_Impl, ListBox*);

    void UpdateButtons();
public:
    SwCustomizeAddressListDialog(Window* pParent, const SwCSVData& rOldData);
    ~SwCustomizeAddressListDialog();

    SwCSVData*    GetNewData();
};
/*-- 13.04.2004 13:30:21---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwAddRenameEntryDialog : public SfxModalDialog
{
    FixedText               m_aFieldNameFT;
    Edit                    m_aFieldNameED;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    const ::std::vector< ::rtl::OUString >& m_rCSVHeader;

    DECL_LINK(ModifyHdl_Impl, Edit*);
public:
    SwAddRenameEntryDialog(Window* pParent, bool bRename, const ::std::vector< ::rtl::OUString >& aCSVHeader);
    ~SwAddRenameEntryDialog();

    void                SetFieldName(const String& rName) {m_aFieldNameED.SetText(rName);}
    String              GetFieldName() const {return m_aFieldNameED.GetText();};

};

#endif

