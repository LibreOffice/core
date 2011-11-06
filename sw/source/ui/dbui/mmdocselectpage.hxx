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


#ifndef _MAILMERGEDOCSELECTPAGE_HXX
#define _MAILMERGEDOCSELECTPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <mailmergehelper.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <svtools/stdctrl.hxx>
class SwMailMergeWizard;
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeDocSelectPage : public svt::OWizardPage
{
    SwBoldFixedInfo     m_aHeaderFI;
    FixedInfo           m_aHowToFT;
    RadioButton         m_aCurrentDocRB;
    RadioButton         m_aNewDocRB;
    RadioButton         m_aLoadDocRB;
    RadioButton         m_aLoadTemplateRB;
    RadioButton         m_aRecentDocRB;

    PushButton          m_aBrowseDocPB;
    PushButton          m_aBrowseTemplatePB;

    ListBox             m_aRecentDocLB;

    String              m_sLoadFileName;
    String              m_sLoadTemplateName;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(DocSelectHdl, RadioButton*);
    DECL_LINK(FileSelectHdl, PushButton*);

    virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

public:
        SwMailMergeDocSelectPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeDocSelectPage();

};

#endif


