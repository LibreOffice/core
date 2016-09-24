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

#include "abspage.hxx"
#include "abspilot.hxx"


namespace abp
{


    using namespace ::svt;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    AddressBookSourcePage::AddressBookSourcePage(OAddressBookSourcePilot* _pParent, const OString& rID, const OUString& rUIXMLDescription)
        :AddressBookSourcePage_Base(_pParent, rID, rUIXMLDescription)
    {
    }

    void AddressBookSourcePage::DeactivatePage()
    {
        AddressBookSourcePage_Base::DeactivatePage();
        getDialog()->enableButtons(WizardButtonFlags::NEXT, true);
    }


    OAddressBookSourcePilot* AddressBookSourcePage::getDialog()
    {
        return static_cast<OAddressBookSourcePilot*>(GetParent());
    }


    const OAddressBookSourcePilot* AddressBookSourcePage::getDialog() const
    {
        return static_cast<const OAddressBookSourcePilot*>(GetParent());
    }


    AddressSettings& AddressBookSourcePage::getSettings()
    {
        return getDialog()->getSettings();
    }


    const AddressSettings&  AddressBookSourcePage::getSettings() const
    {
        return getDialog()->getSettings();
    }


    const Reference< XComponentContext > & AddressBookSourcePage::getORB()
    {
        return getDialog()->getORB();
    }


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
