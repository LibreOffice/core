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
#include <tools/urlobj.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/filenotation.hxx>
#include <sfx2/docfilt.hxx>
#include <vcl/msgbox.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>


namespace abp
{

    using namespace ::svt;
    using namespace ::utl;

    const SfxFilter* lcl_getBaseFilter()
    {
        const SfxFilter* pFilter = SfxFilter::GetFilterByName(OUString("StarOffice XML (Base)"));
        OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
        return pFilter;
    }

    FinalPage::FinalPage( OAddessBookSourcePilot* _pParent )
        : AddressBookSourcePage(_pParent, "DataSourcePage",
            "modules/sabpilot/ui/datasourcepage.ui")
    {
        get(m_pLocation, "location");
        get(m_pBrowse, "browse");
        get(m_pRegisterName, "available");
        get(m_pEmbed, "embed");
        get(m_pNameLabel, "nameft");
        get(m_pLocationLabel, "locationft");
        get(m_pName, "name");
        get(m_pDuplicateNameError, "warning");
        m_pLocationController = new svx::DatabaseLocationInputController(_pParent->getORB(),
            *m_pLocation, *m_pBrowse);

        m_pName->SetModifyHdl( LINK(this, FinalPage, OnNameModified) );
        m_pLocation->SetModifyHdl( LINK(this, FinalPage, OnNameModified) );
        m_pRegisterName->SetClickHdl( LINK( this, FinalPage, OnRegister ) );
        m_pRegisterName->Check();
        m_pEmbed->SetClickHdl( LINK( this, FinalPage, OnEmbed ) );
        m_pEmbed->Check();
        OnEmbed(m_pEmbed);
    }

    FinalPage::~FinalPage()
    {
        disposeOnce();
    }

    void FinalPage::dispose()
    {
        delete m_pLocationController;
        m_pLocation.clear();
        m_pBrowse.clear();
        m_pRegisterName.clear();
        m_pEmbed.clear();
        m_pNameLabel.clear();
        m_pLocationLabel.clear();
        m_pName.clear();
        m_pDuplicateNameError.clear();
        AddressBookSourcePage::dispose();
    }

    bool FinalPage::isValidName() const
    {
        OUString sCurrentName(m_pName->GetText());

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
            OUString sPath = SvtPathOptions().GetWorkPath();
            sPath += "/";
            sPath += rSettings.sDataSourceName;

            const SfxFilter* pFilter = lcl_getBaseFilter();
            if ( pFilter )
            {
                OUString sExt = pFilter->GetDefaultExtension();
                sPath += sExt.getToken(1,'*');
            }

            aURL.SetURL(sPath);
        }
        OSL_ENSURE( aURL.GetProtocol() != INetProtocol::NotValid ,"No valid file name!");
        rSettings.sDataSourceName = aURL.GetMainURL( INetURLObject::NO_DECODE );
        m_pLocationController->setURL( rSettings.sDataSourceName );
        OUString sName = aURL.getName( );
        sal_Int32 nPos = sName.indexOf(aURL.GetExtension());
        if ( nPos != -1 )
        {
            sName = sName.replaceAt(nPos-1, 4, "");
        }
        m_pName->SetText(sName);

        OnRegister(m_pRegisterName);
    }


    void FinalPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        setFields();
    }


    bool FinalPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return false;

        if  (   ( ::svt::WizardTypes::eTravelBackward != _eReason )
            &&  ( !m_pLocationController->prepareCommit() )
            )
            return false;

        AddressSettings& rSettings = getSettings();
        rSettings.sDataSourceName = m_pLocationController->getURL();
        rSettings.bRegisterDataSource = m_pRegisterName->IsChecked();
        if ( rSettings.bRegisterDataSource )
            rSettings.sRegisteredDataSourceName = m_pName->GetText();
        rSettings.bEmbedDataSource = m_pEmbed->IsChecked();

        return true;
    }


    void FinalPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        // get the names of all data sources
        ODataSourceContext aContext( getORB() );
        aContext.getDataSourceNames( m_aInvalidDataSourceNames );

        // give the name edit the focus
        m_pLocation->GrabFocus();

        // default the finish button
        getDialog()->defaultButton( WizardButtonFlags::FINISH );
    }


    void FinalPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();

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
        bool bEmptyName = m_pName->GetText().isEmpty();
        bool bEmptyLocation = m_pLocation->GetText().isEmpty();

        // enable or disable the finish button
        getDialog()->enableButtons( WizardButtonFlags::FINISH, !bEmptyLocation && (!m_pRegisterName->IsChecked() || bValidName) );

        // show the error message for an invalid name
        m_pDuplicateNameError->Show( !bValidName && !bEmptyName );
    }


    IMPL_LINK_NOARG_TYPED( FinalPage, OnNameModified, Edit&, void )
    {
        implCheckName();
    }


    IMPL_LINK_NOARG_TYPED(FinalPage, OnRegister, Button*, void)
    {
        bool bEnable = m_pRegisterName->IsChecked();
        m_pNameLabel->Enable(bEnable);
        m_pName->Enable(bEnable);
        implCheckName();
    }

    IMPL_LINK_NOARG_TYPED(FinalPage, OnEmbed, Button*, void)
    {
        bool bEmbed = m_pEmbed->IsChecked();
        m_pLocationLabel->Enable(!bEmbed);
        m_pLocation->Enable(!bEmbed);
        m_pBrowse->Enable(!bEmbed);
    }

}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
