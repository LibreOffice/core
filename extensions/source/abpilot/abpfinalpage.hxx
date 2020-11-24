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

#include "abspage.hxx"
#include "abptypes.hxx"

#include <svx/databaselocationinput.hxx>
#include <vcl/vclptr.hxx>

namespace abp
{

    class FinalPage final : public AddressBookSourcePage
    {
        std::unique_ptr<SvtURLBox> m_xLocation;
        std::unique_ptr<weld::Button> m_xBrowse;
        std::unique_ptr<weld::CheckButton> m_xRegisterName;
        std::unique_ptr<weld::CheckButton> m_xEmbed;
        std::unique_ptr<weld::Label> m_xNameLabel;
        std::unique_ptr<weld::Label> m_xLocationLabel;
        std::unique_ptr<weld::Entry> m_xName;
        std::unique_ptr<weld::Label> m_xDuplicateNameError;

        std::unique_ptr<svx::DatabaseLocationInputController>
                        m_xLocationController;

        StringBag       m_aInvalidDataSourceNames;

    public:
        explicit FinalPage(weld::Container* pPage, OAddressBookSourcePilot* pController);
        virtual ~FinalPage() override;

    private:
        // OWizardPage overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;

        // BuilderPage overridables
        virtual void        Activate() override;
        virtual void        Deactivate() override;

        // OImportPage overridables
        virtual bool        canAdvance() const override;

        DECL_LINK(OnEntryNameModified, weld::Entry&, void);
        DECL_LINK(OnComboNameModified, weld::ComboBox&, void);
        DECL_LINK(OnRegister, weld::Button&, void);
        DECL_LINK(OnEmbed, weld::Button&, void);

        bool isValidName() const;
        void implCheckName();
        void setFields();
    };

}   // namespace abp
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
