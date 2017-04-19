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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_CONNECTIONPAGESETUP_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_CONNECTIONPAGESETUP_HXX
#include "ConnectionHelper.hxx"

#include "adminpages.hxx"
#include <ucbhelper/content.hxx>
#include "curledit.hxx"

#include <vcl/field.hxx>

namespace dbaui
{

    // OConnectionTabPageSetup

    /** implements the connection page of the data source properties dialog.
    */
    class OConnectionTabPageSetup : public OConnectionHelper
    {
        friend class VclPtr<OConnectionTabPageSetup>;
    protected:

        VclPtr<FixedText>           m_pHelpText;
        VclPtr<FixedText>           m_pHeaderText;

        // called when the test connection button was clicked
        DECL_LINK(OnEditModified, Edit&, void);

    public:
        virtual ~OConnectionTabPageSetup() override;
        virtual void dispose() override;
        static  VclPtr<OGenericAdministrationPage> CreateDbaseTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  VclPtr<OGenericAdministrationPage> CreateMSAccessTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  VclPtr<OGenericAdministrationPage> CreateADOTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  VclPtr<OGenericAdministrationPage> CreateODBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        static  VclPtr<OGenericAdministrationPage> CreateUserDefinedTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) override;

        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;

    protected:
        OConnectionTabPageSetup(vcl::Window* pParent, const OString& _rId, const OUString& _rUIXMLDescription, const SfxItemSet& _rCoreAttrs, const char* pHelpTextResId, const char* pHeaderResId, const char* pUrlResId);
        virtual bool checkTestConnection() override;
            // nControlFlags is a combination of the CBTP_xxx-constants
    };

}   // namespace dbaui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
