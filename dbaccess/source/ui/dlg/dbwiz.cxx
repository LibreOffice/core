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

#include "dbwiz.hxx"
#include "dbu_dlg.hrc"
#include "dsitems.hxx"
#include "dsnItem.hxx"
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/msgbox.hxx>
#include "dbustrings.hrc"
#include "adminpages.hxx"
#include "generalpage.hxx"
#include "localresaccess.hxx"
#include "stringlistitem.hxx"
#include "propertysetitem.hxx"
#include <unotools/confignode.hxx>
#include "ConnectionPage.hxx"
#include "DriverSettings.hxx"
#include "DbAdminImpl.hxx"
#include "dbaccess_helpid.hrc"

namespace dbaui
{
using namespace svt;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
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
ODbTypeWizDialog::ODbTypeWizDialog(vcl::Window* _pParent
                               ,SfxItemSet* _pItems
                               ,const Reference< XComponentContext >& _rxORB
                               ,const css::uno::Any& _aDataSourceName
                               )
    :OWizardMachine(_pParent, WizardButtonFlags::NEXT | WizardButtonFlags::PREVIOUS | WizardButtonFlags::FINISH | WizardButtonFlags::CANCEL | WizardButtonFlags::HELP )
    ,m_pOutSet(nullptr)
{
    m_pImpl.reset(new ODbDataSourceAdministrationHelper(_rxORB,this,this));
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    m_pOutSet = new SfxItemSet( *_pItems->GetPool(), _pItems->GetRanges() );

    m_pImpl->translateProperties(xDatasource, *m_pOutSet);
    m_eType = dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(*m_pOutSet);

    SetPageSizePixel(LogicToPixel(::Size(PAGE_X, PAGE_Y), MAP_APPFONT));
    defaultButton(WizardButtonFlags::NEXT);
    enableButtons(WizardButtonFlags::FINISH, false);
    enableAutomaticNextButtonState();

    m_pPrevPage->SetHelpId(HID_DBWIZ_PREVIOUS);
    m_pNextPage->SetHelpId(HID_DBWIZ_NEXT);
    m_pCancel->SetHelpId(HID_DBWIZ_CANCEL);
    m_pFinish->SetHelpId(HID_DBWIZ_FINISH);
    m_pHelp->SetUniqueId(UID_DBWIZ_HELP);
    // no local resources needed anymore

    const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _pItems->GetItem(DSID_TYPECOLLECTION) );
    m_pCollection = pCollectionItem->getCollection();

    ActivatePage();
    setTitleBase(ModuleRes(STR_DATABASE_TYPE_CHANGE));
}

ODbTypeWizDialog::~ODbTypeWizDialog()
{
    disposeOnce();
}

void ODbTypeWizDialog::dispose()
{
    delete m_pOutSet;
    svt::OWizardMachine::dispose();
}

IMPL_LINK_TYPED(ODbTypeWizDialog, OnTypeSelected, OGeneralPage&, _rTabPage, void)
{
    m_eType = _rTabPage.GetSelectedType();
    const bool bURLRequired = m_pCollection->isConnectionUrlRequired(m_eType);
    enableButtons(WizardButtonFlags::NEXT,bURLRequired);
    enableButtons(WizardButtonFlags::FINISH,!bURLRequired);
}

WizardTypes::WizardState ODbTypeWizDialog::determineNextState( WizardState _nCurrentState ) const
{
    WizardTypes::WizardState nNextState = WZS_INVALID_STATE;
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
    return m_pOutSet;
}

SfxItemSet* ODbTypeWizDialog::getWriteOutputSet()
{
    return m_pOutSet;
}

::std::pair< Reference<XConnection>,sal_Bool> ODbTypeWizDialog::createConnection()
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

VclPtr<TabPage> ODbTypeWizDialog::createPage(WizardState _nState)
{
    sal_uInt16 nStringId = STR_PAGETITLE_ADVANCED;
    VclPtr<TabPage> pPage;
    switch(_nState)
    {
        case START_PAGE: // start state
        {
            pPage = VclPtr<OGeneralPageDialog>::Create(this,*m_pOutSet);
            OGeneralPage* pGeneralPage = static_cast< OGeneralPage* >( pPage.get() );
            pGeneralPage->SetTypeSelectHandler( LINK( this, ODbTypeWizDialog, OnTypeSelected));
            nStringId = STR_PAGETITLE_GENERAL;
        }
        break;
        case CONNECTION_PAGE:
            pPage = OConnectionTabPage::Create(this,m_pOutSet);
            nStringId = STR_PAGETITLE_CONNECTION;
            break;

        case ADDITIONAL_PAGE_DBASE:
            pPage = ODriversSettings::CreateDbase(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_FLAT:
            pPage = ODriversSettings::CreateText(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_LDAP:
            pPage = ODriversSettings::CreateLDAP(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_MYSQL_JDBC:
            pPage = ODriversSettings::CreateMySQLJDBC(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_MYSQL_NATIVE:
            pPage = ODriversSettings::CreateMySQLNATIVE(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_MYSQL_ODBC:
            pPage = ODriversSettings::CreateMySQLODBC(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_ORACLE_JDBC:
            pPage = ODriversSettings::CreateOracleJDBC(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_ADO:
            pPage = ODriversSettings::CreateAdo(this,m_pOutSet);
            break;
        case ADDITIONAL_PAGE_ODBC:
            pPage = ODriversSettings::CreateODBC(this,m_pOutSet);
            break;
        case ADDITIONAL_USERDEFINED:
            pPage = ODriversSettings::CreateUser(this,m_pOutSet);
            break;
        default:
            OSL_FAIL("Wrong state!");
            break;
    }

    // register ourself as modified listener
    if ( pPage )
    {
        static_cast<OGenericAdministrationPage*>(pPage.get())->SetServiceFactory( m_pImpl->getORB() );
        static_cast<OGenericAdministrationPage*>(pPage.get())->SetAdminDialog(this,this);
        pPage->SetText(ModuleRes(nStringId));
        defaultButton( _nState == START_PAGE ? WizardButtonFlags::NEXT : WizardButtonFlags::FINISH );
        enableButtons( WizardButtonFlags::FINISH, _nState != START_PAGE);
        pPage->Show();
    }
    return pPage;
}

bool ODbTypeWizDialog::leaveState(WizardState _nState)
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(_nState));
    if ( pPage )
        pPage->FillItemSet(m_pOutSet);
    return true;
}

void ODbTypeWizDialog::setTitle(const OUString& _sTitle)
{
    SetText(_sTitle);
}

void ODbTypeWizDialog::enableConfirmSettings( bool _bEnable )
{
    enableButtons( WizardButtonFlags::FINISH, _bEnable );
    // TODO:
    // this is hacky. At the moment, this method is used in only one case.
    // As soon as it is to be used more wide-spread, we should find a proper concept
    // for enabling both the Next and Finish buttons, depending on the current page state.
    // Plus, the concept must also care for the case where those pages are embedded into
    // anormal tab dialog.
}

bool ODbTypeWizDialog::saveDatasource()
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(getCurrentState()));
    if ( pPage )
        pPage->FillItemSet(m_pOutSet);

    OUString sOldURL;
    if ( m_pImpl->getCurrentDataSource().is() )
        m_pImpl->getCurrentDataSource()->getPropertyValue(PROPERTY_URL) >>= sOldURL;
    DataSourceInfoConverter::convert( getORB(), m_pCollection,sOldURL,m_eType,m_pImpl->getCurrentDataSource());
    return true;
}

IWizardPageController* ODbTypeWizDialog::getPageController( TabPage* _pCurrentPage ) const
{
    OGenericAdministrationPage* pPage = static_cast<OGenericAdministrationPage*>(_pCurrentPage);
    return pPage;
}

bool ODbTypeWizDialog::onFinish()
{
    saveDatasource();
    return m_pImpl->saveChanges(*m_pOutSet) && OWizardMachine::onFinish();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
