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
#include <comphelper/processfactory.hxx>


namespace abp
{
    FieldMappingPage::FieldMappingPage( OAddessBookSourcePilot* _pParent )
        : AddressBookSourcePage(_pParent, "FieldAssignPage",
            "modules/sabpilot/ui/fieldassignpage.ui")
    {
        get(m_pInvokeDialog, "assign");
        get(m_pHint, "hint");

        m_pInvokeDialog->SetClickHdl( LINK( this, FieldMappingPage, OnInvokeDialog ) );
    }

    FieldMappingPage::~FieldMappingPage()
    {
        disposeOnce();
    }

    void FieldMappingPage::dispose()
    {
        m_pInvokeDialog.clear();
        m_pHint.clear();
        AddressBookSourcePage::dispose();
    }

    void FieldMappingPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();
        m_pInvokeDialog->GrabFocus();
    }


    void FieldMappingPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
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
            sHint = ModuleRes(RID_STR_NOFIELDSASSIGNED).toString();
        m_pHint->SetText( sHint );
    }


    IMPL_LINK_NOARG( FieldMappingPage, OnInvokeDialog )
    {
        AddressSettings& rSettings = getSettings();

        // invoke the dialog doing the mapping
        if ( fieldmapping::invokeDialog( getORB(), this, getDialog()->getDataSource().getDataSource(), rSettings ) )
        {
            if ( rSettings.aFieldMapping.size() )
                getDialog()->travelNext();
            else
                implUpdateHint();
        }

        return 0L;
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
