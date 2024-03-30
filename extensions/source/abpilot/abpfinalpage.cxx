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

#include "abpfinalpage.hxx"
#include "addresssettings.hxx"
#include "abspilot.hxx"
#include <osl/diagnose.h>
#include <tools/urlobj.hxx>
#include <svtools/inettbc.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/filenotation.hxx>
#include <sfx2/docfilt.hxx>
#include <o3tl/string_view.hxx>

namespace abp
{
    static std::shared_ptr<const SfxFilter> lcl_getBaseFilter()
    {
        std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetFilterByName("StarOffice XML (Base)");
        OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
        return pFilter;
    }

    FinalPage::FinalPage(weld::Container* pPage, OAddressBookSourcePilot* pWizard)
        : AddressBookSourcePage(pPage, pWizard, "modules/sabpilot/ui/datasourcepage.ui",
                                "DataSourcePage")
        , m_xLocation(new SvtURLBox(m_xBuilder->weld_combo_box("location")))
        , m_xBrowse(m_xBuilder->weld_button("browse"))
        , m_xRegisterName(m_xBuilder->weld_check_button("available"))
        , m_xEmbed(m_xBuilder->weld_check_button("embed"))
        , m_xNameLabel(m_xBuilder->weld_label("nameft"))
        , m_xLocationLabel(m_xBuilder->weld_label("locationft"))
        , m_xName(m_xBuilder->weld_entry("name"))
        , m_xDuplicateNameError(m_xBuilder->weld_label("warning"))
    {
        m_xLocation->SetSmartProtocol(INetProtocol::File);
        m_xLocation->DisableHistory();

        m_xLocationController.reset( new svx::DatabaseLocationInputController(pWizard->getORB(),
            *m_xLocation, *m_xBrowse, *pWizard->getDialog()) );

        m_xName->connect_changed( LINK(this, FinalPage, OnEntryNameModified) );
        m_xLocation->connect_changed( LINK(this, FinalPage, OnComboNameModified) );
        m_xRegisterName->connect_toggled( LINK( this, FinalPage, OnRegister ) );
        m_xRegisterName->set_active(true);
        m_xEmbed->connect_toggled( LINK( this, FinalPage, OnEmbed ) );
        m_xEmbed->set_active(true);
    }

    FinalPage::~FinalPage()
    {
        m_xLocationController.reset();
    }

    bool FinalPage::isValidName() const
    {
        OUString sCurrentName(m_xName->get_text());

        if (sCurrentName.isEmpty())
            // the name must not be empty
            return false;

        if ( m_aInvalidDataSourceNames.find( sCurrentName ) != m_aInvalidDataSourceNames.end() )
            // there already is a data source with this name
            return false;

        return true;
    }

    void FinalPage::setFields()
    {
        AddressSettings& rSettings = getSettings();

        INetURLObject aURL( rSettings.sDataSourceName );
        if( aURL.GetProtocol() == INetProtocol::NotValid )
        {
            OUString sPath = SvtPathOptions().GetWorkPath() +
                "/" + rSettings.sDataSourceName;

            std::shared_ptr<const SfxFilter> pFilter = lcl_getBaseFilter();
            if ( pFilter )
            {
                OUString sExt = pFilter->GetDefaultExtension();
                sPath += o3tl::getToken(sExt,1,'*');
            }

            aURL.SetURL(sPath);
        }
        OSL_ENSURE( aURL.GetProtocol() != INetProtocol::NotValid ,"No valid file name!");
        rSettings.sDataSourceName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        m_xLocationController->setURL( rSettings.sDataSourceName );
        OUString sName = aURL.getName( );
        sal_Int32 nPos = sName.indexOf(aURL.GetFileExtension());
        if ( nPos != -1 )
        {
            sName = sName.replaceAt(nPos-1, 4, u"");
        }
        m_xName->set_text(sName);

        OnRegister(*m_xRegisterName);
    }


    void FinalPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        setFields();
    }

    bool FinalPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return false;

        if  (   ( ::vcl::WizardTypes::eTravelBackward != _eReason )
            &&  ( !m_xLocationController->prepareCommit() )
            )
            return false;

        AddressSettings& rSettings = getSettings();
        rSettings.sDataSourceName = m_xLocationController->getURL();
        rSettings.bRegisterDataSource = m_xRegisterName->get_active();
        if ( rSettings.bRegisterDataSource )
            rSettings.sRegisteredDataSourceName = m_xName->get_text();
        rSettings.bEmbedDataSource = m_xEmbed->get_active();

        return true;
    }

    void FinalPage::Activate()
    {
        AddressBookSourcePage::Activate();

        // get the names of all data sources
        ODataSourceContext aContext( getORB() );
        aContext.getDataSourceNames( m_aInvalidDataSourceNames );

        // give the name edit the focus
        m_xLocation->grab_focus();

        // default the finish button
        getDialog()->defaultButton( WizardButtonFlags::FINISH );

        OnEmbed(*m_xEmbed);
    }

    void FinalPage::Deactivate()
    {
        AddressBookSourcePage::Deactivate();

        // default the "next" button, again
        getDialog()->defaultButton( WizardButtonFlags::NEXT );
        // disable the finish button
        getDialog()->enableButtons( WizardButtonFlags::FINISH, false );
    }


    bool FinalPage::canAdvance() const
    {
        return false;
    }

    void FinalPage::implCheckName()
    {
        bool bValidName = isValidName();
        bool bEmptyName = m_xName->get_text().isEmpty();
        bool bEmptyLocation = m_xLocation->get_active_text().isEmpty();

        // enable or disable the finish button
        getDialog()->enableButtons( WizardButtonFlags::FINISH, !bEmptyLocation && (!m_xRegisterName->get_active() || bValidName) );

        // show the error message for an invalid name
        m_xDuplicateNameError->set_visible(!bValidName && !bEmptyName);
    }

    IMPL_LINK_NOARG( FinalPage, OnEntryNameModified, weld::Entry&, void )
    {
        implCheckName();
    }

    IMPL_LINK_NOARG( FinalPage, OnComboNameModified, weld::ComboBox&, void )
    {
        implCheckName();
    }

    IMPL_LINK_NOARG(FinalPage, OnRegister, weld::Toggleable&, void)
    {
        bool bEnable = m_xRegisterName->get_active();
        m_xNameLabel->set_sensitive(bEnable);
        m_xName->set_sensitive(bEnable);
        implCheckName();
    }

    IMPL_LINK_NOARG(FinalPage, OnEmbed, weld::Toggleable&, void)
    {
        bool bEmbed = m_xEmbed->get_active();
        m_xLocationLabel->set_sensitive(!bEmbed);
        m_xLocation->set_sensitive(!bEmbed);
        m_xBrowse->set_sensitive(!bEmbed);
    }

}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
