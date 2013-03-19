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

#ifndef EXTENSIONS_ABP_ABSPAGE_HXX
#define EXTENSIONS_ABP_ABSPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "abpresid.hrc"
#include "componentmodule.hxx"
#include <vcl/fixed.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    class OAddessBookSourcePilot;
    struct AddressSettings;

    //=====================================================================
    //= AddressBookSourcePage
    //=====================================================================
    typedef ::svt::OWizardPage AddressBookSourcePage_Base;
    /// the base class for all tab pages in the address book source wizard
    class AddressBookSourcePage : public AddressBookSourcePage_Base
    {
    protected:
        AddressBookSourcePage( OAddessBookSourcePilot* _pParent, const ResId& _rId );

    protected:
        // helper
        OAddessBookSourcePilot* getDialog();
        const OAddessBookSourcePilot*   getDialog() const;
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &
                                getORB();
        AddressSettings&        getSettings();
        const AddressSettings&  getSettings() const;

        // TabDialog overridables
        virtual void        DeactivatePage();
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ABSPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
