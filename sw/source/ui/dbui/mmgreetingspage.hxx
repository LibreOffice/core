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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_MMGREETINGSPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_MMGREETINGSPAGE_HXX

#include <vcl/wizardmachine.hxx>
#include <sfx2/basedlgs.hxx>
#include <mailmergehelper.hxx>
#include <vcl/weld.hxx>

#include <mailmergewizard.hxx>

class SwMailMergeWizard;

class SwGreetingsHandler
{
protected:
    SwMailMergeWizard* m_pWizard;
    /// The mail merge state, available even when m_pWizard is nullptr.
    SwMailMergeConfigItem& m_rConfigItem;
    bool m_bIsTabPage;

    std::unique_ptr<weld::CheckButton> m_xGreetingLineCB;
    std::unique_ptr<weld::CheckButton> m_xPersonalizedCB;
    std::unique_ptr<weld::Label> m_xFemaleFT;
    std::unique_ptr<weld::ComboBox> m_xFemaleLB;
    std::unique_ptr<weld::Button> m_xFemalePB;
    std::unique_ptr<weld::Label> m_xMaleFT;
    std::unique_ptr<weld::ComboBox> m_xMaleLB;
    std::unique_ptr<weld::Button> m_xMalePB;
    std::unique_ptr<weld::Label> m_xFemaleFI;
    std::unique_ptr<weld::Label> m_xFemaleColumnFT;
    std::unique_ptr<weld::ComboBox> m_xFemaleColumnLB;
    std::unique_ptr<weld::Label> m_xFemaleFieldFT;
    std::unique_ptr<weld::ComboBox> m_xFemaleFieldCB;
    std::unique_ptr<weld::Label> m_xNeutralFT;
    std::unique_ptr<weld::ComboBox> m_xNeutralCB;

    SwGreetingsHandler(SwMailMergeConfigItem& rConfigItem, weld::Builder& rBuilder)
        : m_pWizard(nullptr)
        , m_rConfigItem(rConfigItem)
        , m_bIsTabPage(false)
        , m_xGreetingLineCB(rBuilder.weld_check_button(u"greeting"_ustr))
        , m_xPersonalizedCB(rBuilder.weld_check_button(u"personalized"_ustr))
        , m_xFemaleFT(rBuilder.weld_label(u"femaleft"_ustr))
        , m_xFemaleLB(rBuilder.weld_combo_box(u"female"_ustr))
        , m_xFemalePB(rBuilder.weld_button(u"newfemale"_ustr))
        , m_xMaleFT(rBuilder.weld_label(u"maleft"_ustr))
        , m_xMaleLB(rBuilder.weld_combo_box(u"male"_ustr))
        , m_xMalePB(rBuilder.weld_button(u"newmale"_ustr))
        , m_xFemaleFI(rBuilder.weld_label(u"femalefi"_ustr))
        , m_xFemaleColumnFT(rBuilder.weld_label(u"femalecolft"_ustr))
        , m_xFemaleColumnLB(rBuilder.weld_combo_box(u"femalecol"_ustr))
        , m_xFemaleFieldFT(rBuilder.weld_label(u"femalefieldft"_ustr))
        , m_xFemaleFieldCB(rBuilder.weld_combo_box(u"femalefield"_ustr))
        , m_xNeutralFT(rBuilder.weld_label(u"generalft"_ustr))
        , m_xNeutralCB(rBuilder.weld_combo_box(u"general"_ustr))
    {
    }

    ~SwGreetingsHandler() {}

    DECL_LINK(IndividualHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(GreetingHdl_Impl, weld::Button&, void);

    void Contains(bool bContainsGreeting);
    virtual void UpdatePreview();
};

class SwMailMergeGreetingsPage : public vcl::OWizardPage, public SwGreetingsHandler
{
    std::unique_ptr<SwAddressPreview> m_xPreview;
    std::unique_ptr<weld::Label> m_xPreviewFI;
    std::unique_ptr<weld::Button> m_xAssignPB;
    std::unique_ptr<weld::Label> m_xDocumentIndexFI;
    std::unique_ptr<weld::Button> m_xPrevSetIB;
    std::unique_ptr<weld::Button> m_xNextSetIB;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWIN;

    OUString m_sDocument;

    DECL_LINK(ContainsHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(InsertDataHdl_Impl, weld::Button&, void);
    DECL_LINK(GreetingSelectComboBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(GreetingSelectListBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(AssignHdl_Impl, weld::Button&, void);

    virtual void UpdatePreview() override;
    virtual void Activate() override;
    virtual bool commitPage(::vcl::WizardTypes::CommitPageReason _eReason) override;

public:
    SwMailMergeGreetingsPage(weld::Container* pPage, SwMailMergeWizard* pWizard);
    virtual ~SwMailMergeGreetingsPage() override;
};

class SwMailBodyDialog : public SfxDialogController, public SwGreetingsHandler
{
    std::unique_ptr<weld::TextView> m_xBodyMLE;
    std::unique_ptr<weld::Button> m_xOK;

    DECL_LINK(ContainsHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(OKHdl, weld::Button&, void);

public:
    SwMailBodyDialog(weld::Window* pParent,
                     std::shared_ptr<SwMailMergeConfigItem> const& rConfigItem);
    virtual ~SwMailBodyDialog() override;

    void SetBody(const OUString& rBody) { m_xBodyMLE->set_text(rBody); }
    OUString GetBody() const { return m_xBodyMLE->get_text(); }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
