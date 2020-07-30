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

#ifndef INCLUDED_EXTENSIONS_SOURCE_ABPILOT_ABSPAGE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_ABPILOT_ABSPAGE_HXX

#include <vcl/wizardmachine.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <strings.hrc>
#include <componentmodule.hxx>

namespace abp
{
    class OAddressBookSourcePilot;
    struct AddressSettings;

    typedef ::vcl::OWizardPage AddressBookSourcePage_Base;
    /// the base class for all tab pages in the address book source wizard
    class AddressBookSourcePage : public AddressBookSourcePage_Base
    {
        OAddressBookSourcePilot* m_pDialog;

    protected:
        AddressBookSourcePage(weld::Container* pPage, OAddressBookSourcePilot* pController, const OUString& rUIXMLDescription, const OString& rID);

    protected:
        // helper
        OAddressBookSourcePilot* getDialog();
        const OAddressBookSourcePilot* getDialog() const;
        const css::uno::Reference< css::uno::XComponentContext > &
                                getORB() const;
        AddressSettings&        getSettings();
        const AddressSettings&  getSettings() const;

        // BuilderPage overridables
        virtual void        Activate() override;
        virtual void        Deactivate() override;
    };
}   // namespace abp

#endif // INCLUDED_EXTENSIONS_SOURCE_ABPILOT_ABSPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
