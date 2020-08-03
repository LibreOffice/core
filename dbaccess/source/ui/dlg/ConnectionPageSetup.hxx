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
#include "ConnectionHelper.hxx"

#include "adminpages.hxx"

namespace dbaui
{

    // OConnectionTabPageSetup

    /** implements the connection page of the data source properties dialog.
    */
    class OConnectionTabPageSetup : public OConnectionHelper
    {
        std::unique_ptr<weld::Label> m_xHelpText;
        std::unique_ptr<weld::Label> m_xHeaderText;

        // called when the test connection button was clicked
        DECL_LINK(OnEditModified, weld::Entry&, void);

    public:
        OConnectionTabPageSetup(weld::Container* pPage, weld::DialogController* pController, const OUString& _rUIXMLDescription, const OString& _rId, const SfxItemSet& _rCoreAttrs, const char* pHelpTextResId, const char* pHeaderResId, const char* pUrlResId);
        virtual ~OConnectionTabPageSetup() override;

        static std::unique_ptr<OGenericAdministrationPage> CreateDbaseTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet);
        static std::unique_ptr<OGenericAdministrationPage> CreateMSAccessTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet);
        static std::unique_ptr<OGenericAdministrationPage> CreateADOTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet);
        static std::unique_ptr<OGenericAdministrationPage> CreateODBCTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet);
        static std::unique_ptr<OGenericAdministrationPage> CreateUserDefinedTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet);

        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) override;

        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual bool        commitPage( ::vcl::WizardTypes::CommitPageReason _eReason ) override;

    protected:
        virtual bool checkTestConnection() override;
    };


}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
