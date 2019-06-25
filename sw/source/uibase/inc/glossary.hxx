/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GLOSSARY_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GLOSSARY_HXX

#include <vcl/edit.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/text/XAutoTextContainer2.hpp>

#include <rtl/ustring.hxx>
#include <sfx2/basedlgs.hxx>

struct GroupUserData;
class SwGlossaryHdl;
class SwNewGlosNameDlg;
class SwWrtShell;
class SfxViewFrame;
class PopupMenu;
class Menu;
class SwOneExampleFrame;

const short RET_EDIT = 100;

class SwGlossaryDlg : public SfxDialogController
{
    friend class SwNewGlosNameDlg;

    OUString const        m_sReadonlyPath;

    css::uno::Reference< css::text::XAutoTextContainer2 > m_xAutoText;

    SwGlossaryHdl*  m_pGlossaryHdl;

    OUString        m_sResumeGroup;
    OUString        m_sResumeShortName;
    bool            m_bResume;

    const bool      m_bSelection : 1;
    bool            m_bReadOnly : 1;
    bool            m_bIsOld : 1;
    bool            m_bIsDocReadOnly:1;

    SwWrtShell*     m_pShell;

    std::vector<std::unique_ptr<GroupUserData>> m_xGroupData;

    std::unique_ptr<weld::CheckButton> m_xInsertTipCB;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::Label> m_xShortNameLbl;
    TextFilter      m_aNoSpaceFilter;
    std::unique_ptr<weld::Entry> m_xShortNameEdit;
    std::unique_ptr<weld::TreeView> m_xCategoryBox;
    std::unique_ptr<weld::CheckButton> m_xFileRelCB;
    std::unique_ptr<weld::CheckButton> m_xNetRelCB;
    std::unique_ptr<weld::Button> m_xInsertBtn;
    std::unique_ptr<weld::MenuButton> m_xEditBtn;
    std::unique_ptr<weld::Button> m_xBibBtn;
    std::unique_ptr<weld::Button> m_xPathBtn;
    std::unique_ptr<SwOneExampleFrame> m_xExampleFrame;
    std::unique_ptr<weld::CustomWeld> m_xExampleFrameWin;

    void EnableShortName(bool bOn = true);
    void ShowPreview();

    DECL_LINK( NameModify, weld::Entry&, void );
    DECL_LINK( NameDoubleClick, weld::TreeView&, void );
    DECL_LINK( GrpSelect, weld::TreeView&, void );
    DECL_LINK( MenuHdl, const OString&, void );
    DECL_LINK( EnableHdl, weld::ToggleButton&, void );
    DECL_LINK( BibHdl, weld::Button&, void );
    DECL_LINK( InsertHdl, weld::Button&, void );
    DECL_LINK( PathHdl, weld::Button&, void );
    DECL_LINK( CheckBoxHdl, weld::ToggleButton&, void );
    DECL_LINK( PreviewLoadedHdl, SwOneExampleFrame&, void );
    DECL_LINK( KeyInputHdl, const KeyEvent&, bool );
    DECL_LINK( TextFilterHdl, OUString&, bool );

    void            Apply();
    void            Init();
    std::unique_ptr<weld::TreeIter> DoesBlockExist(const OUString& sBlock, const OUString& rShort);
    void            ShowAutoText(const OUString& rGroup, const OUString& rShortName);
    void            ResumeShowAutoText();

    bool            GetResumeData(OUString& rGroup, OUString& rShortName)
                        {rGroup = m_sResumeGroup; rShortName = m_sResumeShortName; return m_bResume;}
    void            SetResumeData(const OUString& rGroup, const OUString& rShortName)
                        {m_sResumeGroup = rGroup; m_sResumeShortName = rShortName; m_bResume = true;}

    void            DeleteEntry();
public:
    SwGlossaryDlg(SfxViewFrame const * pViewFrame, SwGlossaryHdl* pGlosHdl, SwWrtShell *pWrtShell);
    virtual short run() override;
    virtual ~SwGlossaryDlg() override;
    OUString GetCurrGrpName() const;
    OUString GetCurrShortName() const
    {
        return m_xShortNameEdit->get_text();
    }
    static OUString GetCurrGroup();
    static void     SetActGroup(const OUString& rNewGroup);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
