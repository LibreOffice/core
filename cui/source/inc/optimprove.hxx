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


#ifndef _SVX_OPTIMPROVE_HXX
#define _SVX_OPTIMPROVE_HXX

// include ---------------------------------------------------------------

#include <vcl/tabpage.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/fixedhyper.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

// class SvxImprovementPage ----------------------------------------------

class SvxImprovementPage : public TabPage
{
private:
    FixedLine       m_aImproveFL;
    FixedText       m_aInvitationFT;
    RadioButton     m_aYesRB;
    RadioButton     m_aNoRB;
    FixedLine       m_aDataFL;
    FixedText       m_aNumberOfReportsFT;
    FixedText       m_aNumberOfReportsValueFT;
    FixedText       m_aNumberOfActionsFT;
    FixedText       m_aNumberOfActionsValueFT;
    PushButton      m_aShowDataPB;

    String          m_sInfo;
    String          m_sMoreInfo;

public:
    SvxImprovementPage( Window* pParent );
    ~SvxImprovementPage();

    inline bool     IsYesChecked() const { return m_aYesRB.IsChecked() != sal_False; }

    inline String   GetPageText() const { return GetText(); }
    inline String   GetInvitationText() const { return m_aInvitationFT.GetText(); }
    inline String   GetYesButtonText() const { return m_aYesRB.GetText(); }
    inline String   GetNoButtonText() const { return m_aNoRB.GetText(); }
    inline String   GetInfoText() const { return m_sInfo; }
    inline String   GetTitleText() const { return m_aImproveFL.GetText(); }
};

class SvxImprovementOptionsPage : public SfxTabPage
{
private:
    FixedLine                   m_aImproveFL;
    FixedText                   m_aInvitationFT;
    RadioButton                 m_aYesRB;
    RadioButton                 m_aNoRB;
    svt::FixedHyperlinkImage    m_aInfoFI;
    FixedLine                   m_aDataFL;
    FixedText                   m_aNumberOfReportsFT;
    FixedText                   m_aNumberOfReportsValueFT;
    FixedText                   m_aNumberOfActionsFT;
    FixedText                   m_aNumberOfActionsValueFT;
    PushButton                  m_aShowDataPB;

    String                      m_sInfo;
    String                      m_sMoreInfo;
    ::rtl::OUString             m_sLogPath;

    SvxImprovementOptionsPage( Window* pParent, const SfxItemSet& rSet );

    DECL_LINK( HandleHyperlink, svt::FixedHyperlinkImage * );
    DECL_LINK( HandleShowData, PushButton * );

public:
    virtual             ~SvxImprovementOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class SvxImprovementDialog : public SfxSingleTabDialog
{
private:
    SvxImprovementPage*     m_pPage;

    DECL_LINK( HandleOK, OKButton * );
    DECL_LINK( HandleHyperlink, svt::FixedHyperlinkImage * );

public:
    SvxImprovementDialog( Window* pParent, const String& rInfoURL );
};

#endif

