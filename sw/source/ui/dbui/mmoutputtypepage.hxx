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


#ifndef _MAILMERGEOUTPUTTYPEPAGE_HXX
#define _MAILMERGEOUTPUTTYPEPAGE_HXX

#include <svtools/wizardmachine.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <mailmergehelper.hxx>
class SwMailMergeWizard;
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeOutputTypePage : public svt::OWizardPage
{
    SwBoldFixedInfo m_aHeaderFI;
    FixedInfo       m_aTypeFT;
    RadioButton     m_aLetterRB;
    RadioButton     m_aMailRB;

    SwBoldFixedInfo m_aHintHeaderFI;
    FixedInfo       m_aHintFI;

    String          m_sLetterHintHeader;
    String          m_sMailHintHeader;
    String          m_sLetterHint;
    String          m_sMailHint;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(TypeHdl_Impl, RadioButton*);

public:
        SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent);
        ~SwMailMergeOutputTypePage();

};
#endif


