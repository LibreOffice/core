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


#ifndef _MAILCONFIGPAGE_HXX
#define _MAILCONFIGPAGE_HXX

#include <sfx2/tabdlg.hxx>
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#include <sfx2/basedlgs.hxx>

class SwTestAccountSettingsDialog;
class SwMailMergeConfigItem;
/*-- 05.05.2004 16:45:45---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailConfigPage : public SfxTabPage
{
    friend class SwTestAccountSettingsDialog;

    FixedLine       m_aIdentityFL;

    FixedText       m_aDisplayNameFT;
    Edit            m_aDisplayNameED;
    FixedText       m_aAddressFT;
    Edit            m_aAddressED;

    CheckBox        m_aReplyToCB;
    FixedText       m_aReplyToFT;
    Edit            m_aReplyToED;

    FixedLine       m_aSMTPFL;

    FixedText       m_aServerFT;
    Edit            m_aServerED;
    FixedText       m_aPortFT;
    NumericField    m_aPortNF;

    CheckBox        m_aSecureCB;

    PushButton      m_aServerAuthenticationPB;

    FixedLine       m_aSeparatorFL;
    PushButton      m_aTestPB;

    SwMailMergeConfigItem*  m_pConfigItem;

    DECL_LINK(ReplyToHdl, CheckBox*);
    DECL_LINK(AuthenticationHdl, PushButton*);
    DECL_LINK(TestHdl, PushButton*);


public:
    SwMailConfigPage( Window* pParent, const SfxItemSet& rSet );
    ~SwMailConfigPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

/*-- 18.08.2004 12:02:02---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailConfigDlg : public SfxSingleTabDialog
{
public:

    SwMailConfigDlg( Window* pParent, SfxItemSet& rSet );
    ~SwMailConfigDlg();
};

#endif

