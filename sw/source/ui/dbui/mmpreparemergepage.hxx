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


#ifndef _MAILMERGEPREPAREMERGEPAGE_HXX
#define _MAILMERGEPREPAREMERGEPAGE_HXX

#include <svtools/wizardmachine.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <mailmergehelper.hxx>
#include "actctrl.hxx"

class SwMailMergeWizard;
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergePrepareMergePage : public svt::OWizardPage
{
    SwBoldFixedInfo m_aHeaderFI;
    FixedInfo       m_aPreviewFI;
    FixedText       m_aRecipientFT;
    PushButton      m_aFirstPB;
    PushButton      m_aPrevPB;
    NumEditAction   m_aRecordED;
    PushButton      m_aNextPB;
    PushButton      m_aLastPB;
    CheckBox        m_ExcludeCB;

    FixedLine       m_aNoteHeaderFL;
    FixedInfo       m_aEditFI;
    PushButton      m_aEditPB;


    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(EditDocumentHdl_Impl, PushButton*);
    DECL_LINK(ExcludeHdl_Impl, CheckBox*);
    DECL_LINK(MoveHdl_Impl, void*);

    virtual void        ActivatePage();
    virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

public:
        SwMailMergePrepareMergePage( SwMailMergeWizard* _pParent);
        ~SwMailMergePrepareMergePage();

};

#endif


