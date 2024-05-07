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
#pragma once

#include <vcl/formatter.hxx>
#include <vcl/weld.hxx>
#include <sal/log.hxx>

/// Dialog for editing a name
class SvxNameDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdtName;
    std::unique_ptr<weld::Label> m_xFtDescription;
    std::unique_ptr<weld::Button> m_xBtnOK;

    Link<SvxNameDialog&, bool> m_aCheckNameHdl;
    Link<SvxNameDialog&, OUString> m_aCheckNameTooltipHdl;

    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    SvxNameDialog(weld::Window* pWindow, const OUString& rName, const OUString& rDesc,
                  const OUString& rTitle = u""_ustr);

    OUString GetName() const { return m_xEdtName->get_text(); }

    /** add a callback Link that is called whenever the content of the edit
        field is changed.  The Link result determines whether the OK
        Button is enabled (> 0) or disabled (== 0).

        @param rLink a Callback declared with DECL_DLLPRIVATE_LINK and implemented with
               IMPL_LINK, that is executed on modification.
     */
    void SetCheckNameHdl(const Link<SvxNameDialog&, bool>& rLink)
    {
        m_aCheckNameHdl = rLink;
        ModifyHdl(*m_xEdtName);
    }

    void SetCheckNameTooltipHdl(const Link<SvxNameDialog&, OUString>& rLink)
    {
        m_aCheckNameTooltipHdl = rLink;
        m_xBtnOK->set_tooltip_text(rLink.Call(*this));
    }

    void SetEditHelpId(const OUString& aHelpId) { m_xEdtName->set_help_id(aHelpId); }
};

/// Dialog for editing a number
class SvxNumberDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::SpinButton> m_xEdtNumber;
    std::unique_ptr<weld::Label> m_xFtDescription;

public:
    SvxNumberDialog(weld::Window* pWindow, const OUString& rDesc, sal_Int64 nValue, sal_Int64 nMin,
                    sal_Int64 nMax);

    sal_Int64 GetNumber() const { return m_xEdtNumber->get_value(); }
};

class SvxDecimalNumberDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::FormattedSpinButton> m_xEdtNumber;
    std::unique_ptr<weld::Label> m_xFtDescription;

public:
    SvxDecimalNumberDialog(weld::Window* pWindow, const OUString& rDesc, double fValue);

    double GetNumber() const { return m_xEdtNumber->GetFormatter().GetValue(); }
};

/** #i68101#
    Dialog for editing Object name
    plus uniqueness-callback-linkHandler */
class SvxObjectNameDialog final : public weld::GenericDialogController
{
private:
    // name
    std::unique_ptr<weld::Entry> m_xEdtName;

    // buttons
    std::unique_ptr<weld::Button> m_xBtnOK;

    // callback link for name uniqueness
    Link<SvxObjectNameDialog&, bool> aCheckNameHdl;

    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    // constructor
    SvxObjectNameDialog(weld::Window* pWindow, const OUString& rName);

    // data access
    OUString GetName() const { return m_xEdtName->get_text(); }

    // set handler
    void SetCheckNameHdl(const Link<SvxObjectNameDialog&, bool>& rLink) { aCheckNameHdl = rLink; }
};

/** #i68101#
    Dialog for editing Object Title and Description */
class SvxObjectTitleDescDialog final : public weld::GenericDialogController
{
private:
    // title
    std::unique_ptr<weld::Label> m_xTitleFT;
    std::unique_ptr<weld::Entry> m_xEdtTitle;

    // description
    std::unique_ptr<weld::Label> m_xDescriptionFT;
    std::unique_ptr<weld::TextView> m_xEdtDescription;

    std::unique_ptr<weld::CheckButton> m_xDecorativeCB;

    DECL_LINK(DecorativeHdl, weld::Toggleable&, void);

public:
    // constructor
    SvxObjectTitleDescDialog(weld::Window* pWindow, const OUString& rTitle, const OUString& rDesc,
                             bool isDecorative);
    // data access
    OUString GetTitle() const { return m_xEdtTitle->get_text(); }
    OUString GetDescription() const { return m_xEdtDescription->get_text(); }
    bool IsDecorative() const { return m_xDecorativeCB->get_active(); }
};

enum class ListMode
{
    String,
    Int64,
    Int32,
    Int16,
    Double
};

/** Generic dialog to edit lists */
class SvxListDialog : public weld::GenericDialogController
{
private:
    ListMode m_aMode;
    std::unique_ptr<weld::TreeView> m_xList;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xRemoveBtn;
    std::unique_ptr<weld::Button> m_xEditBtn;

    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);
    DECL_LINK(DblClickHdl_Impl, weld::TreeView&, bool);
    DECL_LINK(AddHdl_Impl, weld::Button&, void);
    DECL_LINK(RemoveHdl_Impl, weld::Button&, void);
    DECL_LINK(EditHdl_Impl, weld::Button&, void);

    void SelectionChanged();

public:
    explicit SvxListDialog(weld::Window* pParent);
    virtual ~SvxListDialog() override;

    std::vector<OUString> GetEntries();
    void SetEntries(std::vector<OUString> const& rParams);
    void EditEntry();
    void SetMode(ListMode aMode);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
