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

#include <core_resource.hxx>
#include <dbwiz.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <dsitems.hxx>
#include "dsnItem.hxx"
#include "adminpages.hxx"
#include "generalpage.hxx"
#include <unotools/confignode.hxx>
#include "ConnectionPage.hxx"
#include "DriverSettings.hxx"
#include "DbAdminImpl.hxx"
#include <helpids.h>

namespace dbaui
{
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

#define START_PAGE                      0
#define CONNECTION_PAGE                 1
#define ADDITIONAL_PAGE_DBASE           2
#define ADDITIONAL_PAGE_FLAT            3
#define ADDITIONAL_PAGE_LDAP            4
//5 was ADDITIONAL_PAGE_ADABAS
#define ADDITIONAL_PAGE_MYSQL_JDBC      6
#define ADDITIONAL_PAGE_MYSQL_ODBC      7
#define ADDITIONAL_PAGE_ORACLE_JDBC     8
#define ADDITIONAL_PAGE_ADO             9
#define ADDITIONAL_PAGE_ODBC           10
#define ADDITIONAL_USERDEFINED         11
#define ADDITIONAL_PAGE_MYSQL_NATIVE   12

// ODbTypeWizDialog
ODbTypeWizDialog::ODbTypeWizDialog(weld::Window* _pParent, SfxItemSet const * _pItems,
                                   const Reference< XComponentContext >& _rxORB, const css::uno::Any& _aDataSourceName)
    : WizardMachine(_pParent, WizardButtonFlags::NEXT | WizardButtonFlags::PREVIOUS | WizardButtonFlags::FINISH | WizardButtonFlags::CANCEL | WizardButtonFlags::HELP )
{
    m_pImpl.reset(new ODbDataSourceAdministrationHelper(_rxORB, m_xAssistant.get(), _pParent, this));
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    m_pOutSet.reset(new SfxItemSet( *_pItems->GetPool(), _pItems->GetRanges() ));

    m_pImpl->translateProperties(xDatasource, *m_pOutSet);
    m_eType = dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(*m_pOutSet);

    defaultButton(WizardButtonFlags::NEXT);
    enableButtons(WizardButtonFlags::FINISH, false);
    enableAutomaticNextButtonState();

    m_xPrevPage->set_help_id(HID_DBWIZ_PREVIOUS);
    m_xNextPage->set_help_id(HID_DBWIZ_NEXT);
    m_xCancel->set_help_id(HID_DBWIZ_CANCEL);
    m_xFinish->set_help_id(HID_DBWIZ_FINISH);
    // no local resources needed anymore

    const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>(_pItems->GetItem(DSID_TYPECOLLECTION));
    assert(pCollectionItem && "must exist");
    m_pCollection = pCollectionItem->getCollection();

    ActivatePage();
    setTitleBase(DBA_RES(STR_DATABASE_TYPE_CHANGE));

    m_xAssistant->set_current_page(0);
}

ODbTypeWizDialog::~ODbTypeWizDialog()
{
}

IMPL_LINK(ODbTypeWizDialog, OnTypeSelected, OGeneralPage&, _rTabPage, void)
{
    m_eType = _rTabPage.GetSelectedType();
    const bool bURLRequired = m_pCollection->isConnectionUrlRequired(m_eType);
    enableButtons(WizardButtonFlags::NEXT,bURLRequired);
    enableButtons(WizardButtonFlags::FINISH,!bURLRequired);
}

WizardState ODbTypeWizDialog::determineNextState( WizardState _nCurrentState ) const
{
    WizardState nNextState = WZS_INVALID_STATE;
    switch(_nCurrentState)
    {
        case START_PAGE:
            switch(m_pCollection->determineType(m_eType))
            {
                case  ::dbaccess::DST_MOZILLA:
                case  ::dbaccess::DST_OUTLOOK:
                case  ::dbaccess::DST_OUTLOOKEXP:
                case  ::dbaccess::DST_EVOLUTION:
                case  ::dbaccess::DST_EVOLUTION_GROUPWISE:
                case  ::dbaccess::DST_EVOLUTION_LDAP:
                case  ::dbaccess::DST_KAB:
                case  ::dbaccess::DST_MACAB:
                    nNextState = WZS_INVALID_STATE;
                    break;
                case  ::dbaccess::DST_MYSQL_NATIVE:
                    nNextState = ADDITIONAL_PAGE_MYSQL_NATIVE;
                    break;
                default:
                    nNextState = CONNECTION_PAGE;
                    break;
            }
            break;
        case CONNECTION_PAGE:
            switch(m_pCollection->determineType(m_eType))
            {
                case  ::dbaccess::DST_MOZILLA:
                case  ::dbaccess::DST_THUNDERBIRD:
                case  ::dbaccess::DST_OUTLOOK:
                case  ::dbaccess::DST_OUTLOOKEXP:
                case  ::dbaccess::DST_EVOLUTION:
                case  ::dbaccess::DST_EVOLUTION_GROUPWISE:
                case  ::dbaccess::DST_EVOLUTION_LDAP:
                case  ::dbaccess::DST_KAB:
                case  ::dbaccess::DST_MACAB:
                case  ::dbaccess::DST_MSACCESS:
                case  ::dbaccess::DST_MSACCESS_2007:
                case  ::dbaccess::DST_JDBC:
                case  ::dbaccess::DST_CALC:
                case  ::dbaccess::DST_WRITER:
                    nNextState = WZS_INVALID_STATE;
                    break;
                case  ::dbaccess::DST_DBASE:
                    nNextState = ADDITIONAL_PAGE_DBASE;
                    break;
                case  ::dbaccess::DST_FLAT:
                    nNextState = ADDITIONAL_PAGE_FLAT;
                    break;
                case  ::dbaccess::DST_LDAP:
                    nNextState = ADDITIONAL_PAGE_LDAP;
                    break;
                case  ::dbaccess::DST_MYSQL_JDBC:
                    nNextState = ADDITIONAL_PAGE_MYSQL_JDBC;
                    break;
                case  ::dbaccess::DST_MYSQL_ODBC:
                    nNextState = ADDITIONAL_PAGE_MYSQL_ODBC;
                    break;
                case  ::dbaccess::DST_ORACLE_JDBC:
                    nNextState = ADDITIONAL_PAGE_ORACLE_JDBC;
                    break;
                case  ::dbaccess::DST_ADO:
                    nNextState = ADDITIONAL_PAGE_ADO;
                    break;
                case  ::dbaccess::DST_ODBC:
                    nNextState = ADDITIONAL_PAGE_ODBC;
                    break;
                default:
                    nNextState = WZS_INVALID_STATE;
                    break;
            }
            break;
    }

    return nNextState;
}

const SfxItemSet* ODbTypeWizDialog::getOutputSet() const
{
    return m_pOutSet.get();
}

SfxItemSet* ODbTypeWizDialog::getWriteOutputSet()
{
    return m_pOutSet.get();
}

std::pair< Reference<XConnection>,bool> ODbTypeWizDialog::createConnection()
{
    return m_pImpl->createConnection();
}

Reference< XComponentContext > ODbTypeWizDialog::getORB() const
{
    return m_pImpl->getORB();
}

Reference< XDriver > ODbTypeWizDialog::getDriver()
{
    return m_pImpl->getDriver();
}

OUString ODbTypeWizDialog::getDatasourceType(const SfxItemSet& _rSet) const
{
    return dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(_rSet);
}

void ODbTypeWizDialog::clearPassword()
{
    m_pImpl->clearPassword();
}

std::unique_ptr<BuilderPage> ODbTypeWizDialog::createPage(WizardState _nState)
{
    TranslateId pStringId = STR_PAGETITLE_ADVANCED;
    std::unique_ptr<BuilderPage> xPage;

    OUString sIdent(OUString::number(_nState));
    weld::Container* pPageContainer = m_xAssistant->append_page(sIdent);

    switch(_nState)
    {
        case START_PAGE: // start state
        {
            xPage = std::make_unique<OGeneralPageDialog>(pPageContainer, this, *m_pOutSet);
            OGeneralPage* pGeneralPage = static_cast<OGeneralPage*>(xPage.get());
            pGeneralPage->SetTypeSelectHandler( LINK( this, ODbTypeWizDialog, OnTypeSelected));
            pStringId = STR_PAGETITLE_GENERAL;
        }
        break;
        case CONNECTION_PAGE:
            xPage = OConnectionTabPage::Create(pPageContainer, this, m_pOutSet.get());
            pStringId = STR_PAGETITLE_CONNECTION;
            break;

        case ADDITIONAL_PAGE_DBASE:
            xPage = ODriversSettings::CreateDbase(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_FLAT:
            xPage = ODriversSettings::CreateText(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_LDAP:
            xPage = ODriversSettings::CreateLDAP(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_MYSQL_JDBC:
            xPage = ODriversSettings::CreateMySQLJDBC(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_MYSQL_NATIVE:
            xPage = ODriversSettings::CreateMySQLNATIVE(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_MYSQL_ODBC:
            xPage = ODriversSettings::CreateMySQLODBC(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_ORACLE_JDBC:
            xPage = ODriversSettings::CreateOracleJDBC(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_ADO:
            xPage = ODriversSettings::CreateAdo(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_PAGE_ODBC:
            xPage = ODriversSettings::CreateODBC(pPageContainer, this, m_pOutSet.get());
            break;
        case ADDITIONAL_USERDEFINED:
            xPage = ODriversSettings::CreateUser(pPageContainer, this, m_pOutSet.get());
            break;
        default:
            OSL_FAIL("Wrong state!");
            break;
    }

    // register ourself as modified listener
    if ( xPage )
    {
        static_cast<OGenericAdministrationPage*>(xPage.get())->SetServiceFactory( m_pImpl->getORB() );
        static_cast<OGenericAdministrationPage*>(xPage.get())->SetAdminDialog(this,this);
        m_xAssistant->set_page_title(sIdent, DBA_RES(pStringId));
        defaultButton( _nState == START_PAGE ? WizardButtonFlags::NEXT : WizardButtonFlags::FINISH );
        enableButtons( WizardButtonFlags::FINISH, _nState != START_PAGE);
    }
    return xPage;
}

bool ODbTypeWizDialog::leaveState(WizardState _nState)
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardMachine::GetPage(_nState));
    if ( pPage )
        pPage->FillItemSet(m_pOutSet.get());
    return true;
}

void ODbTypeWizDialog::setTitle(const OUString& _sTitle)
{
    m_xAssistant->set_title(_sTitle);
}

void ODbTypeWizDialog::enableConfirmSettings( bool _bEnable )
{
    enableButtons( WizardButtonFlags::FINISH, _bEnable );
    // TODO:
    // this is hacky. At the moment, this method is used in only one case.
    // As soon as it is to be used more wide-spread, we should find a proper concept
    // for enabling both the Next and Finish buttons, depending on the current page state.
    // Plus, the concept must also care for the case where those pages are embedded into
    // a normal tab dialog.
}

void ODbTypeWizDialog::saveDatasource()
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardMachine::GetPage(getCurrentState()));
    if ( pPage )
        pPage->FillItemSet(m_pOutSet.get());

    OUString sOldURL;
    if ( m_pImpl->getCurrentDataSource().is() )
        m_pImpl->getCurrentDataSource()->getPropertyValue(PROPERTY_URL) >>= sOldURL;
    DataSourceInfoConverter::convert( getORB(), m_pCollection,sOldURL,m_eType,m_pImpl->getCurrentDataSource());
}

vcl::IWizardPageController* ODbTypeWizDialog::getPageController(BuilderPage* pCurrentPage) const
{
    OGenericAdministrationPage* pPage = static_cast<OGenericAdministrationPage*>(pCurrentPage);
    return pPage;
}

bool ODbTypeWizDialog::onFinish()
{
    saveDatasource();
    return m_pImpl->saveChanges(*m_pOutSet) && WizardMachine::onFinish();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
