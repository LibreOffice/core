/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

