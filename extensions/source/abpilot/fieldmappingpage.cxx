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

#include "fieldmappingpage.hxx"
#include "fieldmappingimpl.hxx"
#include "addresssettings.hxx"
#include "abspilot.hxx"


namespace abp
{
    FieldMappingPage::FieldMappingPage(weld::Container* pPage, OAddressBookSourcePilot* pController)
        : AddressBookSourcePage(pPage, pController, u"modules/sabpilot/ui/fieldassignpage.ui"_ustr, u"FieldAssignPage"_ustr)
        , m_xInvokeDialog(m_xBuilder->weld_button(u"assign"_ustr))
        , m_xHint(m_xBuilder->weld_label(u"hint"_ustr))
    {
        m_xInvokeDialog->connect_clicked(LINK(this, FieldMappingPage, OnInvokeDialog));
    }

    FieldMappingPage::~FieldMappingPage()
    {
    }

    void FieldMappingPage::Activate()
    {
        AddressBookSourcePage::Activate();
        m_xInvokeDialog->grab_focus();
    }

    void FieldMappingPage::initializePage()
    {
        AddressBookSourcePage::initializePage();
        implUpdateHint();
    }

    void FieldMappingPage::implUpdateHint()
    {
        const AddressSettings& rSettings = getSettings();
        OUString sHint;
        if ( rSettings.aFieldMapping.empty() )
            sHint = compmodule::ModuleRes(RID_STR_NOFIELDSASSIGNED);
        m_xHint->set_label(sHint);
    }

    IMPL_LINK_NOARG( FieldMappingPage, OnInvokeDialog, weld::Button&, void )
    {
        AddressSettings& rSettings = getSettings();

        // invoke the dialog doing the mapping
        if ( fieldmapping::invokeDialog( getORB(), getDialog()->getDialog(), getDialog()->getDataSource().getDataSource(), rSettings ) )
        {
            if ( !rSettings.aFieldMapping.empty() )
                getDialog()->travelNext();
            else
                implUpdateHint();
        }
    }

}   // namespace abp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
