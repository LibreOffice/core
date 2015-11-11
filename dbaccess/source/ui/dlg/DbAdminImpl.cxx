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

#include "DbAdminImpl.hxx"
#include "dsmeta.hxx"

#include <svl/poolitem.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include "DriverSettings.hxx"
#include "IItemSetHelper.hxx"
#include "UITools.hxx"
#include "dbu_dlg.hrc"
#include "dbustrings.hrc"
#include "dsitems.hxx"
#include "dsnItem.hxx"
#include "moduledbu.hxx"
#include "optionalboolitem.hxx"
#include "propertysetitem.hxx"
#include "stringlistitem.hxx"
#include "OAuthenticationContinuation.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/ConnectionPool.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication2.hpp>
#include <com/sun/star/ucb/AuthenticationRequest.hpp>

#include <comphelper/guarding.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <connectivity/DriversConfig.hxx>
#include <connectivity/dbexception.hxx>
#include <osl/file.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <typelib/typedescription.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/waitobj.hxx>
#include <osl/mutex.hxx>

#include <algorithm>
#include <functional>
#include <o3tl/functional.hxx>

namespace dbaui
{
using namespace ::dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace com::sun::star::ucb;
using namespace com::sun::star::task;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;

namespace
{
    bool implCheckItemType( SfxItemSet& _rSet, const sal_uInt16 _nId, std::function<bool ( const SfxPoolItem* )> isItemType )
    {
        bool bCorrectType = false;

        SfxItemPool* pPool = _rSet.GetPool();
        OSL_ENSURE( pPool, "implCheckItemType: invalid item pool!" );
        if ( pPool )
        {
            const SfxPoolItem& rDefItem = pPool->GetDefaultItem( _nId );
            bCorrectType = isItemType(&rDefItem);
        }
        return bCorrectType;
    }

    void lcl_putProperty(const Reference< XPropertySet >& _rxSet, const OUString& _rName, const Any& _rValue)
    {
        try
        {
            if ( _rxSet.is() )
                _rxSet->setPropertyValue(_rName, _rValue);
        }
        catch(Exception&)
        {
    #if OSL_DEBUG_LEVEL > 0
            OString sMessage("ODbAdminDialog::implTranslateProperty: could not set the property ");
            sMessage += OString(_rName.getStr(), _rName.getLength(), RTL_TEXTENCODING_ASCII_US);
            sMessage += OString("!");
            OSL_FAIL(sMessage.getStr());
    #endif
        }

    }

    OUString lcl_createHostWithPort(const SfxStringItem* _pHostName,const SfxInt32Item* _pPortNumber)
    {
        OUString sNewUrl;

        if ( _pHostName && _pHostName->GetValue().getLength() )
            sNewUrl = _pHostName->GetValue();

        if ( _pPortNumber )
        {
            sNewUrl = sNewUrl + ":" + OUString::number(_pPortNumber->GetValue());
        }

        return sNewUrl;
    }
}

    // ODbDataSourceAdministrationHelper
ODbDataSourceAdministrationHelper::ODbDataSourceAdministrationHelper(const Reference< XComponentContext >& _xORB, vcl::Window* _pParent,IItemSetHelper* _pItemSetHelper)
        : m_xContext(_xORB)
        , m_pParent(_pParent)
        , m_pItemSetHelper(_pItemSetHelper)
{
    /// initialize the property translation map
    // direct properties of a data source
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_CONNECTURL, PROPERTY_URL));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_NAME, PROPERTY_NAME));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_USER, PROPERTY_USER));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_PASSWORD, PROPERTY_PASSWORD));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_PASSWORDREQUIRED, PROPERTY_ISPASSWORDREQUIRED));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_TABLEFILTER, PROPERTY_TABLEFILTER));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_READONLY, PROPERTY_ISREADONLY));
    m_aDirectPropTranslator.insert(MapInt2String::value_type(DSID_SUPPRESSVERSIONCL, PROPERTY_SUPPRESSVERSIONCL));

    // implicit properties, to be found in the direct property "Info"
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_JDBCDRIVERCLASS, INFO_JDBCDRIVERCLASS));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTFILEEXTENSION, INFO_TEXTFILEEXTENSION));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CHARSET, INFO_CHARSET));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTFILEHEADER, INFO_TEXTFILEHEADER));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_FIELDDELIMITER, INFO_FIELDDELIMITER));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_TEXTDELIMITER, INFO_TEXTDELIMITER));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_DECIMALDELIMITER, INFO_DECIMALDELIMITER));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_THOUSANDSDELIMITER, INFO_THOUSANDSDELIMITER));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_SHOWDELETEDROWS, INFO_SHOWDELETEDROWS));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_ALLOWLONGTABLENAMES, INFO_ALLOWLONGTABLENAMES));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_ADDITIONALOPTIONS, INFO_ADDITIONALOPTIONS));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_SQL92CHECK, PROPERTY_ENABLESQL92CHECK));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_AUTOINCREMENTVALUE, PROPERTY_AUTOINCREMENTCREATION));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_AUTORETRIEVEVALUE, INFO_AUTORETRIEVEVALUE));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_AUTORETRIEVEENABLED, INFO_AUTORETRIEVEENABLED));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_APPEND_TABLE_ALIAS, INFO_APPEND_TABLE_ALIAS));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_AS_BEFORE_CORRNAME, INFO_AS_BEFORE_CORRELATION_NAME ) );
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CHECK_REQUIRED_FIELDS, INFO_FORMS_CHECK_REQUIRED_FIELDS ) );
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_ESCAPE_DATETIME, INFO_ESCAPE_DATETIME ) );
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_PRIMARY_KEY_SUPPORT, OUString("PrimaryKeySupport")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_PARAMETERNAMESUBST, INFO_PARAMETERNAMESUBST));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_IGNOREDRIVER_PRIV, INFO_IGNOREDRIVER_PRIV));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_BOOLEANCOMPARISON, PROPERTY_BOOLEANCOMPARISONMODE));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_ENABLEOUTERJOIN, PROPERTY_ENABLEOUTERJOIN));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CATALOG, PROPERTY_USECATALOGINSELECT));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_SCHEMA, PROPERTY_USESCHEMAINSELECT));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_INDEXAPPENDIX, OUString("AddIndexAppendix")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_DOSLINEENDS, OUString("PreferDosLikeLineEnds")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_SOCKET, OUString("LocalSocket")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_NAMED_PIPE, OUString("NamedPipe")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_RESPECTRESULTSETTYPE, OUString("RespectDriverResultSetType")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_MAX_ROW_SCAN, OUString("MaxRowScan")));

    // extra settings for odbc
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_USECATALOG, INFO_USECATALOG));
    // extra settings for a ldap address book
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_LDAP_BASEDN, INFO_CONN_LDAP_BASEDN));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_LDAP_ROWCOUNT, INFO_CONN_LDAP_ROWCOUNT));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_CONN_LDAP_USESSL, OUString("UseSSL")));
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_DOCUMENT_URL, PROPERTY_URL));

    // oracle
    m_aIndirectPropTranslator.insert(MapInt2String::value_type(DSID_IGNORECURRENCY, OUString("IgnoreCurrency")));

    try
    {
        m_xDatabaseContext = DatabaseContext::create(m_xContext);
    }
    catch(const Exception&)
    {
        ShowServiceNotAvailableError(_pParent->GetParent(), "com.sun.star.sdb.DatabaseContext", true);
    }
}

bool ODbDataSourceAdministrationHelper::getCurrentSettings(Sequence< PropertyValue >& _rDriverParam)
{
    OSL_ENSURE(m_pItemSetHelper->getOutputSet(), "ODbDataSourceAdministrationHelper::getCurrentSettings : not to be called without an example set!");
    if (!m_pItemSetHelper->getOutputSet())
        return false;

    ::std::vector< PropertyValue > aReturn;
        // collecting this in a vector because it has a push_back, in opposite to sequences

    // user: DSID_USER -> "user"
    const SfxStringItem* pUser = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_USER);
    if (pUser && pUser->GetValue().getLength())
        aReturn.push_back(
            PropertyValue(  OUString("user"), 0,
                            makeAny(OUString(pUser->GetValue())), PropertyState_DIRECT_VALUE));

    // check if the connection type requires a password
    if (hasAuthentication(*m_pItemSetHelper->getOutputSet()))
    {
        // password: DSID_PASSWORD -> password
        const SfxStringItem* pPassword = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_PASSWORD);
        OUString sPassword = pPassword ? pPassword->GetValue() : OUString();
        const SfxBoolItem* pPasswordRequired = m_pItemSetHelper->getOutputSet()->GetItem<SfxBoolItem>(DSID_PASSWORDREQUIRED);
        // if the set does not contain a password, but the item set says it requires one, ask the user
        if ((!pPassword || !pPassword->GetValue().getLength()) && (pPasswordRequired && pPasswordRequired->GetValue()))
        {
            const SfxStringItem* pName = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_NAME);

            Reference< XModel > xModel( getDataSourceOrModel( m_xDatasource ), UNO_QUERY_THROW );
            ::comphelper::NamedValueCollection aArgs( xModel->getArgs() );
            Reference< XInteractionHandler > xHandler( aArgs.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() ) );

            if ( !xHandler.is() )
            {
                // instantiate the default SDB interaction handler
                xHandler.set( task::InteractionHandler::createWithParent(m_xContext, nullptr), UNO_QUERY );
            }

            OUString sName = pName ? pName->GetValue() : OUString();
            OUString sLoginRequest(ModuleRes(STR_ENTER_CONNECTION_PASSWORD));
            OUString sTemp = sName;
            sName = ::dbaui::getStrippedDatabaseName(nullptr,sTemp);
            if ( !sName.isEmpty() )
                sLoginRequest = sLoginRequest.replaceAll("$name$", sName);
            else
            {
                sLoginRequest = sLoginRequest.replaceAll("\"$name$\"", "");
                // ensure that in other languages the string will be deleted
                sLoginRequest = sLoginRequest.replaceAll("$name$", "");
            }

            // the request
            AuthenticationRequest aRequest;
            aRequest.ServerName = sName;
            aRequest.Diagnostic = sLoginRequest;
            aRequest.HasRealm   = aRequest.HasAccount = sal_False;
            // aRequest.Realm
            aRequest.HasUserName = pUser != nullptr;
            aRequest.UserName    = pUser ? OUString(pUser->GetValue()) : OUString();
            aRequest.HasPassword = sal_True;
            //aRequest.Password
            aRequest.HasAccount  = sal_False;
            // aRequest.Account

            comphelper::OInteractionRequest* pRequest = new comphelper::OInteractionRequest(makeAny(aRequest));
            uno::Reference< XInteractionRequest > xRequest(pRequest);

            // build an interaction request
            // two continuations (Ok and Cancel)
            ::rtl::Reference< comphelper::OInteractionAbort > pAbort = new comphelper::OInteractionAbort;
            ::rtl::Reference< dbaccess::OAuthenticationContinuation > pAuthenticate = new dbaccess::OAuthenticationContinuation;
            pAuthenticate->setCanChangeUserName( false );
            pAuthenticate->setRememberPassword( RememberAuthentication_SESSION );

            // some knittings
            pRequest->addContinuation(pAbort.get());
            pRequest->addContinuation(pAuthenticate.get());

            // handle the request
            try
            {
                SolarMutexGuard aSolarGuard;
                // release the mutex when calling the handler, it may need to lock the SolarMutex
                xHandler->handle(xRequest);
            }
            catch(Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            if (!pAuthenticate->wasSelected())
                return false;

            sPassword = pAuthenticate->getPassword();
            if (pAuthenticate->getRememberPassword())
                m_pItemSetHelper->getWriteOutputSet()->Put(SfxStringItem(DSID_PASSWORD, sPassword));
        }

        if (!sPassword.isEmpty())
            aReturn.push_back(
                PropertyValue(  OUString("password"), 0,
                                makeAny(OUString(sPassword)), PropertyState_DIRECT_VALUE));
    }

    if ( !aReturn.empty() )
        _rDriverParam = Sequence< PropertyValue >(&(*aReturn.begin()), aReturn.size());

    // append all the other stuff (charset etc.)
    fillDatasourceInfo(*m_pItemSetHelper->getOutputSet(), _rDriverParam);

    return true;
}

void ODbDataSourceAdministrationHelper::successfullyConnected()
{
    OSL_ENSURE(m_pItemSetHelper->getOutputSet(), "ODbDataSourceAdministrationHelper::successfullyConnected: not to be called without an example set!");
    if (!m_pItemSetHelper->getOutputSet())
        return;

    if (hasAuthentication(*m_pItemSetHelper->getOutputSet()))
    {
        const SfxStringItem* pPassword = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_PASSWORD);
        if (pPassword && (0 != pPassword->GetValue().getLength()))
        {
            OUString sPassword = pPassword->GetValue();

            Reference< XPropertySet > xCurrentDatasource = getCurrentDataSource();
            lcl_putProperty(xCurrentDatasource,m_aDirectPropTranslator[DSID_PASSWORD], makeAny(sPassword));
        }
    }
}

void ODbDataSourceAdministrationHelper::clearPassword()
{
    if (m_pItemSetHelper->getWriteOutputSet())
        m_pItemSetHelper->getWriteOutputSet()->ClearItem(DSID_PASSWORD);
}

::std::pair< Reference<XConnection>,sal_Bool> ODbDataSourceAdministrationHelper::createConnection()
{
    ::std::pair< Reference<XConnection>,sal_Bool> aRet;
    aRet.second = sal_False;
    Sequence< PropertyValue > aConnectionParams;
    if ( getCurrentSettings(aConnectionParams) )
    {
        // the current DSN
        // fill the table list with this connection information
        SQLExceptionInfo aErrorInfo;
        try
        {
            WaitObject aWaitCursor(m_pParent);
            aRet.first = getDriver()->connect(getConnectionURL(), aConnectionParams);
            aRet.second = sal_True;
        }
        catch (const SQLContext& e) { aErrorInfo = SQLExceptionInfo(e); }
        catch (const SQLWarning& e) { aErrorInfo = SQLExceptionInfo(e); }
        catch (const SQLException& e) { aErrorInfo = SQLExceptionInfo(e); }

        showError(aErrorInfo,m_pParent,getORB());
    }
    if ( aRet.first.is() )
        successfullyConnected();// notify the admindlg to save the password

    return aRet;
}

Reference< XDriver > ODbDataSourceAdministrationHelper::getDriver()
{
    return getDriver(getConnectionURL());
}

Reference< XDriver > ODbDataSourceAdministrationHelper::getDriver(const OUString& _sURL)
{
    // get the global DriverManager
    Reference< XConnectionPool > xDriverManager;

    OUString sCurrentActionError = ModuleRes(STR_COULDNOTCREATE_DRIVERMANAGER);
    sCurrentActionError = sCurrentActionError.replaceFirst("#servicename#", "com.sun.star.sdbc.ConnectionPool");

    try
    {
        xDriverManager.set( ConnectionPool::create( getORB() ) );
    }
    catch (const Exception& e)
    {
        // wrap the exception into an SQLException
        SQLException aSQLWrapper(e.Message, getORB(), OUString("S1000"), 0, Any());
        throw SQLException(sCurrentActionError, getORB(), OUString("S1000"), 0, makeAny(aSQLWrapper));
    }

    Reference< XDriver > xDriver = xDriverManager->getDriverByURL(_sURL);
    if (!xDriver.is())
    {
        sCurrentActionError = ModuleRes(STR_NOREGISTEREDDRIVER);
        sCurrentActionError = sCurrentActionError.replaceFirst("#connurl#", _sURL);
        // will be caught and translated into an SQLContext exception
        throw SQLException(sCurrentActionError, getORB(), OUString("S1000"), 0, Any());
    }
    return xDriver;
}

Reference< XPropertySet > ODbDataSourceAdministrationHelper::getCurrentDataSource()
{
    if ( !m_xDatasource.is() )
    {
        Reference<XInterface> xIn(m_aDataSourceOrName,UNO_QUERY);
        if ( !xIn.is() )
        {
            OUString sCurrentDatasource;
            m_aDataSourceOrName >>= sCurrentDatasource;
            OSL_ENSURE(!sCurrentDatasource.isEmpty(),"No datasource name given!");
            try
            {
                if ( m_xDatabaseContext.is() )
                    m_xDatasource.set(m_xDatabaseContext->getByName(sCurrentDatasource),UNO_QUERY);
                xIn = m_xDatasource;
            }
            catch(const Exception&)
            {
            }
        }
        m_xModel.set(getDataSourceOrModel(xIn),UNO_QUERY);
        if ( m_xModel.is() )
            m_xDatasource.set(xIn,UNO_QUERY);
        else
        {
            m_xDatasource.set(getDataSourceOrModel(xIn),UNO_QUERY);
            m_xModel.set(xIn,UNO_QUERY);
        }
    }

    OSL_ENSURE(m_xDatasource.is(), "ODbDataSourceAdministrationHelper::getCurrentDataSource: no data source!");
    return m_xDatasource;
}

OUString ODbDataSourceAdministrationHelper::getDatasourceType( const SfxItemSet& _rSet )
{
    const SfxStringItem* pConnectURL = _rSet.GetItem<SfxStringItem>(DSID_CONNECTURL);
    OSL_ENSURE( pConnectURL , "ODbDataSourceAdministrationHelper::getDatasourceType: invalid items in the source set!" );
    const DbuTypeCollectionItem* pTypeCollection = _rSet.GetItem<DbuTypeCollectionItem>(DSID_TYPECOLLECTION);
    OSL_ENSURE(pTypeCollection, "ODbDataSourceAdministrationHelper::getDatasourceType: invalid items in the source set!");
    ::dbaccess::ODsnTypeCollection* pCollection = pTypeCollection->getCollection();
    return pCollection->getType(pConnectURL->GetValue());
}

bool ODbDataSourceAdministrationHelper::hasAuthentication(const SfxItemSet& _rSet)
{
    return DataSourceMetaData::getAuthentication( getDatasourceType( _rSet ) ) != AuthNone;
}

OUString ODbDataSourceAdministrationHelper::getConnectionURL() const
{
    OUString sNewUrl;

    OUString eType = getDatasourceType(*m_pItemSetHelper->getOutputSet());

    const SfxStringItem* pUrlItem = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_CONNECTURL);
    const DbuTypeCollectionItem* pTypeCollection = m_pItemSetHelper->getOutputSet()->GetItem<DbuTypeCollectionItem>(DSID_TYPECOLLECTION);

    OSL_ENSURE(pUrlItem,"Connection URL is NULL. -> GPF!");
    OSL_ENSURE(pTypeCollection, "ODbDataSourceAdministrationHelper::getDatasourceType: invalid items in the source set!");
    ::dbaccess::ODsnTypeCollection* pCollection = pTypeCollection->getCollection();
    OSL_ENSURE(pCollection, "ODbDataSourceAdministrationHelper::getDatasourceType: invalid type collection!");

    switch( pCollection->determineType(eType) )
    {
        case  ::dbaccess::DST_DBASE:
        case  ::dbaccess::DST_FLAT:
        case  ::dbaccess::DST_CALC:
            break;
        case  ::dbaccess::DST_MSACCESS:
        case  ::dbaccess::DST_MSACCESS_2007:
            {
                OUString sFileName = pCollection->cutPrefix(pUrlItem->GetValue());
                OUString sNewFileName;
                if ( ::osl::FileBase::getSystemPathFromFileURL( sFileName, sNewFileName ) == ::osl::FileBase::E_None )
                {
                    sNewUrl += sNewFileName;
                }
            }
            break;
        case  ::dbaccess::DST_MYSQL_NATIVE:
        case  ::dbaccess::DST_MYSQL_JDBC:
            {
                const SfxStringItem* pHostName = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_CONN_HOSTNAME);
                const SfxInt32Item* pPortNumber = m_pItemSetHelper->getOutputSet()->GetItem<SfxInt32Item>(DSID_MYSQL_PORTNUMBER);
                const SfxStringItem* pDatabaseName = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_DATABASENAME);
                sNewUrl = lcl_createHostWithPort(pHostName,pPortNumber);
                OUString sDatabaseName = pDatabaseName ? pDatabaseName->GetValue() : OUString();
                if ( !sDatabaseName.getLength() && pUrlItem )
                    sDatabaseName = pCollection->cutPrefix( pUrlItem->GetValue() );
                    // TODO: what's that? Why is the database name transported via the URL Item?
                    // Huh? Anybody there?
                    // OJ: It is needed when the connection properties are changed. There the URL is used for every type.

                if ( !sDatabaseName.isEmpty() )
                {
                    sNewUrl += "/" + sDatabaseName;
                }
            }
            break;
        case  ::dbaccess::DST_ORACLE_JDBC:
            {
                const SfxStringItem* pHostName = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_CONN_HOSTNAME);
                const SfxInt32Item* pPortNumber = m_pItemSetHelper->getOutputSet()->GetItem<SfxInt32Item>(DSID_ORACLE_PORTNUMBER);
                const SfxStringItem* pDatabaseName = m_pItemSetHelper->getOutputSet()->GetItem<SfxStringItem>(DSID_DATABASENAME);
                if ( pHostName && pHostName->GetValue().getLength() )
                {
                    sNewUrl = "@" + lcl_createHostWithPort(pHostName,pPortNumber);
                    OUString sDatabaseName = pDatabaseName ? pDatabaseName->GetValue() : OUString();
                    if ( sDatabaseName.isEmpty() && pUrlItem )
                        sDatabaseName = pCollection->cutPrefix( pUrlItem->GetValue() );
                    if ( !sDatabaseName.isEmpty() )
                    {
                        sNewUrl += ":" + sDatabaseName;
                    }
                }
                else
                { // here someone entered a JDBC url which looks like oracle, so we have to use the url property

                }
            }
            break;
        case  ::dbaccess::DST_LDAP:
            {
                const SfxInt32Item* pPortNumber = m_pItemSetHelper->getOutputSet()->GetItem<SfxInt32Item>(DSID_CONN_LDAP_PORTNUMBER);
                sNewUrl = pCollection->cutPrefix(pUrlItem->GetValue()) + lcl_createHostWithPort(nullptr,pPortNumber);
            }
            break;
        case  ::dbaccess::DST_JDBC:
            // run through
        default:
            break;
    }
    if ( !sNewUrl.isEmpty() )
        sNewUrl = pCollection->getPrefix(eType) + sNewUrl;
    else
        sNewUrl = pUrlItem->GetValue();

    return sNewUrl;
}

struct PropertyValueLess
{
    bool operator() (const PropertyValue& x, const PropertyValue& y) const
        { return x.Name < y.Name; }      // construct prevents a MSVC6 warning
};

typedef std::set<PropertyValue, PropertyValueLess> PropertyValueSet;

void ODbDataSourceAdministrationHelper::translateProperties(const Reference< XPropertySet >& _rxSource, SfxItemSet& _rDest)
{
    if (_rxSource.is())
    {
        for (   MapInt2String::const_iterator aDirect = m_aDirectPropTranslator.begin();
                aDirect != m_aDirectPropTranslator.end();
                ++aDirect
            )
        {
            // get the property value
            Any aValue;
            try
            {
                aValue = _rxSource->getPropertyValue(aDirect->second);
            }
            catch(Exception&)
            {
#if OSL_DEBUG_LEVEL > 0
                OString aMessage("ODbDataSourceAdministrationHelper::translateProperties: could not extract the property ");
                aMessage += OString(aDirect->second.getStr(), aDirect->second.getLength(), RTL_TEXTENCODING_ASCII_US);
                aMessage += OString("!");
                OSL_FAIL(aMessage.getStr());
#endif
            }
            // transfer it into an item
            implTranslateProperty(_rDest, aDirect->first, aValue);
        }

        // get the additional information
        Sequence< PropertyValue > aAdditionalInfo;
        try
        {
            _rxSource->getPropertyValue(PROPERTY_INFO) >>= aAdditionalInfo;
        }
        catch(Exception&) { }

        // collect the names of the additional settings
        const PropertyValue* pAdditionalInfo = aAdditionalInfo.getConstArray();
        PropertyValueSet aInfos;
        for (sal_Int32 i=0; i<aAdditionalInfo.getLength(); ++i, ++pAdditionalInfo)
        {
            if( pAdditionalInfo->Name == "JDBCDRV" )
            {   // compatibility
                PropertyValue aCompatibility(*pAdditionalInfo);
                aCompatibility.Name = "JavaDriverClass";
                aInfos.insert(aCompatibility);
            }
            else
                aInfos.insert(*pAdditionalInfo);
        }

        // go through all known translations and check if we have such a setting
        if ( !aInfos.empty() )
        {
            PropertyValue aSearchFor;
            MapInt2String::const_iterator aEnd = m_aIndirectPropTranslator.end();
            for (   MapInt2String::const_iterator aIndirect = m_aIndirectPropTranslator.begin();
                    aIndirect != aEnd;
                    ++aIndirect)
            {
                aSearchFor.Name = aIndirect->second;
                PropertyValueSet::const_iterator aInfoPos = aInfos.find(aSearchFor);
                if (aInfos.end() != aInfoPos)
                    // the property is contained in the info sequence
                    // -> transfer it into an item
                    implTranslateProperty(_rDest, aIndirect->first, aInfoPos->Value);
            }
        }

        convertUrl(_rDest);
    }

    try
    {
        _rDest.Put(OPropertySetItem(DSID_DATASOURCE_UNO, _rxSource));
        Reference<XStorable> xStore(getDataSourceOrModel(_rxSource),UNO_QUERY);
        _rDest.Put(SfxBoolItem(DSID_READONLY, !xStore.is() || xStore->isReadonly() ));
    }
    catch(Exception&)
    {
        OSL_FAIL("IsReadOnly throws an exception!");
    }
}

void ODbDataSourceAdministrationHelper::translateProperties(const SfxItemSet& _rSource, const Reference< XPropertySet >& _rxDest)
{
    OSL_ENSURE(_rxDest.is(), "ODbDataSourceAdministrationHelper::translateProperties: invalid property set!");
    if (!_rxDest.is())
        return;

    // the property set info
    Reference< XPropertySetInfo > xInfo;
    try { xInfo = _rxDest->getPropertySetInfo(); }
    catch(Exception&) { }

    const OUString sUrlProp("URL");
    // transfer the direct properties
    for (   MapInt2String::const_iterator aDirect = m_aDirectPropTranslator.begin();
            aDirect != m_aDirectPropTranslator.end();
            ++aDirect
        )
    {
        const SfxPoolItem* pCurrentItem = _rSource.GetItem((sal_uInt16)aDirect->first);
        if (pCurrentItem)
        {
            sal_Int16 nAttributes = PropertyAttribute::READONLY;
            if (xInfo.is())
            {
                try { nAttributes = xInfo->getPropertyByName(aDirect->second).Attributes; }
                catch(Exception&) { }
            }
            if ((nAttributes & PropertyAttribute::READONLY) == 0)
            {
                if ( sUrlProp == aDirect->second )
                {
                    Any aValue(makeAny(OUString(getConnectionURL())));
                    //  aValue <<= OUString();
                    lcl_putProperty(_rxDest, aDirect->second,aValue);
                }
                else
                    implTranslateProperty(_rxDest, aDirect->second, pCurrentItem);
            }
        }
    }

    // now for the indirect properties

    Sequence< PropertyValue > aInfo;
    // the original properties
    try
    {
        _rxDest->getPropertyValue(PROPERTY_INFO) >>= aInfo;
    }
    catch(Exception&) { }

    // overwrite and extend them
    fillDatasourceInfo(_rSource, aInfo);
    // and propagate the (newly composed) sequence to the set
    lcl_putProperty(_rxDest,PROPERTY_INFO, makeAny(aInfo));
}

void ODbDataSourceAdministrationHelper::fillDatasourceInfo(const SfxItemSet& _rSource, Sequence< css::beans::PropertyValue >& _rInfo)
{
    // within the current "Info" sequence, replace the ones we can examine from the item set
    // (we don't just fill a completely new sequence with our own items, but we preserve any properties unknown to
    // us)

    // first determine which of all the items are relevant for the data source (depends on the connection url)
    OUString eType = getDatasourceType(_rSource);
    ::std::vector< sal_Int32> aDetailIds;
    ODriversSettings::getSupportedIndirectSettings(eType, getORB(), aDetailIds);

    // collect the translated property values for the relevant items
    PropertyValueSet aRelevantSettings;
    MapInt2String::const_iterator aTranslation;
    ::std::vector< sal_Int32>::iterator aDetailsEnd = aDetailIds.end();
    for (::std::vector< sal_Int32>::iterator aIter = aDetailIds.begin();aIter != aDetailsEnd ; ++aIter)
    {
        const SfxPoolItem* pCurrent = _rSource.GetItem((sal_uInt16)*aIter);
        aTranslation = m_aIndirectPropTranslator.find(*aIter);
        if ( pCurrent && (m_aIndirectPropTranslator.end() != aTranslation) )
        {
            if ( aTranslation->second == INFO_CHARSET )
            {
                OUString sCharSet;
                implTranslateProperty(pCurrent) >>= sCharSet;
                if ( !sCharSet.isEmpty() )
                    aRelevantSettings.insert(PropertyValue(aTranslation->second, 0, makeAny(sCharSet), PropertyState_DIRECT_VALUE));
            }
            else
                aRelevantSettings.insert(PropertyValue(aTranslation->second, 0, implTranslateProperty(pCurrent), PropertyState_DIRECT_VALUE));
        }
    }

    // settings to preserve
    MapInt2String   aPreservedSettings;

    // now aRelevantSettings contains all the property values relevant for the current data source type,
    // check the original sequence if it already contains any of these values (which have to be overwritten, then)
    PropertyValue* pInfo = _rInfo.getArray();
    PropertyValue aSearchFor;
    sal_Int32 nObsoleteSetting = -1;
    sal_Int32 nCount = _rInfo.getLength();
    for (sal_Int32 i = 0; i < nCount; ++i, ++pInfo)
    {
        aSearchFor.Name = pInfo->Name;
        PropertyValueSet::iterator aOverwrittenSetting = aRelevantSettings.find(aSearchFor);
        if (aRelevantSettings.end() != aOverwrittenSetting)
        {   // the setting was present in the original sequence, and it is to be overwritten -> replace it
            if ( pInfo->Value != aOverwrittenSetting->Value )
                *pInfo = *aOverwrittenSetting;
            aRelevantSettings.erase(aOverwrittenSetting);
        }
        else if( pInfo->Name == "JDBCDRV" )
        {   // this is a compatibility setting, remove it from the sequence (it's replaced by JavaDriverClass)
            nObsoleteSetting = i;
        }
        else
            aPreservedSettings[i] = pInfo->Name;
    }
    if (-1 != nObsoleteSetting)
        ::comphelper::removeElementAt(_rInfo, nObsoleteSetting);

    if ( !aPreservedSettings.empty() )
    {   // check if there are settings which
        // * are known as indirect properties
        // * but not relevant for the current data source type
        // These settings have to be removed: If they're not relevant, we have no UI for changing them.

        // for this, we need a string-controlled quick access to m_aIndirectPropTranslator
        StringSet aIndirectProps;
        ::std::transform(m_aIndirectPropTranslator.begin(),
                         m_aIndirectPropTranslator.end(),
                         ::std::insert_iterator<StringSet>(aIndirectProps,aIndirectProps.begin()),
                         ::o3tl::select2nd< MapInt2String::value_type >());

        // now check the to-be-preserved props
        ::std::vector< sal_Int32 > aRemoveIndexes;
        sal_Int32 nPositionCorrector = 0;
        MapInt2String::const_iterator aPreservedEnd = aPreservedSettings.end();
        for (   MapInt2String::const_iterator aPreserved = aPreservedSettings.begin();
                aPreserved != aPreservedEnd;
                ++aPreserved
            )
        {
            if (aIndirectProps.end() != aIndirectProps.find(aPreserved->second))
            {
                aRemoveIndexes.push_back(aPreserved->first - nPositionCorrector);
                ++nPositionCorrector;
            }
        }
        // now finally remove all such props
        ::std::vector< sal_Int32 >::const_iterator aRemoveEnd = aRemoveIndexes.end();
        for (   ::std::vector< sal_Int32 >::const_iterator aRemoveIndex = aRemoveIndexes.begin();
                aRemoveIndex != aRemoveEnd;
                ++aRemoveIndex
            )
            ::comphelper::removeElementAt(_rInfo, *aRemoveIndex);
    }

    ::connectivity::DriversConfig aDriverConfig(getORB());
    const ::comphelper::NamedValueCollection& aProperties = aDriverConfig.getProperties(eType);
    Sequence< Any> aTypeSettings;
    aTypeSettings = aProperties.getOrDefault("TypeInfoSettings",aTypeSettings);
    // here we have a special entry for types from oracle
    if ( aTypeSettings.getLength() )
    {
        aRelevantSettings.insert(PropertyValue(OUString("TypeInfoSettings"), 0, makeAny(aTypeSettings), PropertyState_DIRECT_VALUE));
    }

    // check which values are still left ('cause they were not present in the original sequence, but are to be set)
    if ( !aRelevantSettings.empty() )
    {
        sal_Int32 nOldLength = _rInfo.getLength();
        _rInfo.realloc(nOldLength + aRelevantSettings.size());
        PropertyValue* pAppendValues = _rInfo.getArray() + nOldLength;
        PropertyValueSet::const_iterator aRelevantEnd = aRelevantSettings.end();
        for (   PropertyValueSet::const_iterator aLoop = aRelevantSettings.begin();
                aLoop != aRelevantEnd;
                ++aLoop, ++pAppendValues
            )
        {
            if ( aLoop->Name == INFO_CHARSET )
            {
                OUString sCharSet;
                aLoop->Value >>= sCharSet;
                if ( !sCharSet.isEmpty() )
                    *pAppendValues = *aLoop;
            }
            else
                *pAppendValues = *aLoop;
        }
    }
}

Any ODbDataSourceAdministrationHelper::implTranslateProperty(const SfxPoolItem* _pItem)
{
    // translate the SfxPoolItem
    Any aValue;

    const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>( _pItem  );
    const SfxBoolItem* pBoolItem = dynamic_cast<const SfxBoolItem*>( _pItem  );
    const OptionalBoolItem* pOptBoolItem = dynamic_cast<const OptionalBoolItem*>( _pItem  );
    const SfxInt32Item* pInt32Item = dynamic_cast< const SfxInt32Item* >( _pItem );
    const OStringListItem* pStringListItem = dynamic_cast<const OStringListItem*>( _pItem  );

    if ( pStringItem )
    {
        aValue <<= OUString( pStringItem->GetValue().getStr() );
    }
    else if ( pBoolItem )
    {
        aValue <<= pBoolItem->GetValue();
    }
    else if ( pOptBoolItem )
    {
        if ( !pOptBoolItem->HasValue() )
            aValue.clear();
        else
            aValue <<= pOptBoolItem->GetValue();
    }
    else if ( pInt32Item )
    {
        aValue <<= pInt32Item->GetValue();
    }
    else if ( pStringListItem )
    {
        aValue <<= pStringListItem->getList();
    }
    else
    {
        OSL_FAIL("ODbDataSourceAdministrationHelper::implTranslateProperty: unsupported item type!");
        return aValue;
    }

    return aValue;
}

void ODbDataSourceAdministrationHelper::implTranslateProperty(const Reference< XPropertySet >& _rxSet, const OUString& _rName, const SfxPoolItem* _pItem)
{
    Any aValue = implTranslateProperty(_pItem);
    lcl_putProperty(_rxSet, _rName,aValue);
}

OString ODbDataSourceAdministrationHelper::translatePropertyId( sal_Int32 _nId )
{
    OUString aString;

    MapInt2String::const_iterator aPos = m_aDirectPropTranslator.find( _nId );
    if ( m_aDirectPropTranslator.end() != aPos )
    {
        aString = aPos->second;
    }
    else
    {
        MapInt2String::const_iterator indirectPos = m_aIndirectPropTranslator.find( _nId );
        if ( m_aIndirectPropTranslator.end() != indirectPos )
            aString = indirectPos->second;
    }

    OString aReturn( aString.getStr(), aString.getLength(), RTL_TEXTENCODING_ASCII_US );
    return aReturn;
}
template<class T> bool checkItemType(const SfxPoolItem* pItem){ return dynamic_cast<const T*>(pItem) != nullptr;}

void ODbDataSourceAdministrationHelper::implTranslateProperty( SfxItemSet& _rSet, sal_Int32  _nId, const Any& _rValue )
{
    switch ( _rValue.getValueType().getTypeClass() )
    {
        case TypeClass_STRING:
            if ( implCheckItemType( _rSet, _nId, checkItemType<SfxStringItem> ) )
            {
                OUString sValue;
                _rValue >>= sValue;
                _rSet.Put(SfxStringItem(_nId, sValue));
            }
            else {
                OSL_FAIL(
                    OString(
                        "ODbDataSourceAdministrationHelper::implTranslateProperty: invalid property value ("
                        + translatePropertyId(_nId)
                        + " should be no string)!").getStr());
            }
            break;

        case TypeClass_BOOLEAN:
            if ( implCheckItemType( _rSet, _nId, checkItemType<SfxBoolItem> ) )
            {
                bool bVal = false;
                _rValue >>= bVal;
                _rSet.Put(SfxBoolItem(_nId, bVal));
            }
            else if ( implCheckItemType( _rSet, _nId, checkItemType<OptionalBoolItem> ) )
            {
                OptionalBoolItem aItem( _nId );
                if ( _rValue.hasValue() )
                {
                    bool bValue = false;
                    _rValue >>= bValue;
                    aItem.SetValue( bValue );
                }
                else
                    aItem.ClearValue();
                _rSet.Put( aItem );
            }
            else {
                OSL_FAIL(
                    OString(
                        "ODbDataSourceAdministrationHelper::implTranslateProperty: invalid property value ("
                        + translatePropertyId(_nId)
                        + " should be no boolean)!").getStr());
            }
            break;

        case TypeClass_LONG:
            if ( implCheckItemType( _rSet, _nId, checkItemType<SfxInt32Item> ) )
            {
                sal_Int32 nValue = 0;
                _rValue >>= nValue;
                _rSet.Put( SfxInt32Item( _nId, nValue ) );
            }
            else {
                OSL_FAIL(
                    OString(
                        "ODbDataSourceAdministrationHelper::implTranslateProperty: invalid property value ("
                        + translatePropertyId(_nId)
                        + " should be no int)!").getStr());
            }
            break;

        case TypeClass_SEQUENCE:
            if ( implCheckItemType( _rSet, _nId, checkItemType<OStringListItem> ) )
            {
                // determine the element type
                TypeDescription aTD(_rValue.getValueType());
                typelib_IndirectTypeDescription* pSequenceTD =
                    reinterpret_cast< typelib_IndirectTypeDescription* >(aTD.get());
                OSL_ENSURE(pSequenceTD && pSequenceTD->pType, "ODbDataSourceAdministrationHelper::implTranslateProperty: invalid sequence type!");

                Type aElementType(pSequenceTD->pType);
                switch (aElementType.getTypeClass())
                {
                    case TypeClass_STRING:
                    {
                        Sequence< OUString > aStringList;
                        _rValue >>= aStringList;
                        _rSet.Put(OStringListItem(_nId, aStringList));
                    }
                    break;
                    default:
                        OSL_FAIL("ODbDataSourceAdministrationHelper::implTranslateProperty: unsupported property value type!");
                }
            }
            else {
                OSL_FAIL(
                    OString(
                        "ODbDataSourceAdministrationHelper::implTranslateProperty: invalid property value ("
                        + translatePropertyId(_nId)
                        + " should be no string sequence)!").getStr());
            }
            break;

        case TypeClass_VOID:
            _rSet.ClearItem(_nId);
            break;

        default:
            OSL_FAIL("ODbDataSourceAdministrationHelper::implTranslateProperty: unsupported property value type!");
    }
}

OUString ODbDataSourceAdministrationHelper::getDocumentUrl(SfxItemSet& _rDest)
{
    const SfxStringItem* pUrlItem = _rDest.GetItem<SfxStringItem>(DSID_DOCUMENT_URL);
    OSL_ENSURE(pUrlItem,"Document URL is NULL. -> GPF!");
    return pUrlItem->GetValue();
}

void ODbDataSourceAdministrationHelper::convertUrl(SfxItemSet& _rDest)
{
    OUString eType = getDatasourceType(_rDest);

    const SfxStringItem* pUrlItem = _rDest.GetItem<SfxStringItem>(DSID_CONNECTURL);
    const DbuTypeCollectionItem* pTypeCollection = _rDest.GetItem<DbuTypeCollectionItem>(DSID_TYPECOLLECTION);

    OSL_ENSURE(pUrlItem,"Connection URL is NULL. -> GPF!");
    OSL_ENSURE(pTypeCollection, "ODbAdminDialog::getDatasourceType: invalid items in the source set!");
    ::dbaccess::ODsnTypeCollection* pCollection = pTypeCollection->getCollection();
    OSL_ENSURE(pCollection, "ODbAdminDialog::getDatasourceType: invalid type collection!");

    sal_uInt16 nPortNumberId    = 0;
    sal_Int32 nPortNumber   = -1;
    OUString sNewHostName;
    OUString sUrlPart;

    pCollection->extractHostNamePort(pUrlItem->GetValue(),sUrlPart,sNewHostName,nPortNumber);
    const ::dbaccess::DATASOURCE_TYPE eTy = pCollection->determineType(eType);

    switch( eTy )
    {
        case  ::dbaccess::DST_MYSQL_NATIVE:
        case  ::dbaccess::DST_MYSQL_JDBC:
            nPortNumberId = DSID_MYSQL_PORTNUMBER;
            break;
        case  ::dbaccess::DST_ORACLE_JDBC:
            nPortNumberId = DSID_ORACLE_PORTNUMBER;
            break;
        case  ::dbaccess::DST_LDAP:
            nPortNumberId = DSID_CONN_LDAP_PORTNUMBER;
            break;
        default:
            break;
    }

    if ( !sUrlPart.isEmpty() )
    {
        if ( eTy == ::dbaccess::DST_MYSQL_NATIVE )
        {
            _rDest.Put( SfxStringItem( DSID_DATABASENAME, sUrlPart ) );
        }
        else
        {
            OUString sNewUrl = pCollection->getPrefix(eType);
            sNewUrl += sUrlPart;
            _rDest.Put( SfxStringItem( DSID_CONNECTURL, sNewUrl ) );
        }
    }

    if ( !sNewHostName.isEmpty() )
        _rDest.Put(SfxStringItem(DSID_CONN_HOSTNAME, sNewHostName));

    if ( nPortNumber != -1 && nPortNumberId != 0 )
        _rDest.Put(SfxInt32Item(nPortNumberId, nPortNumber));

}

bool ODbDataSourceAdministrationHelper::saveChanges(const SfxItemSet& _rSource)
{
    // put the remembered settings into the property set
    Reference<XPropertySet> xDatasource = getCurrentDataSource();
    if ( !xDatasource.is() )
        return false;

    translateProperties(_rSource,xDatasource );

    return true;
}

void ODbDataSourceAdministrationHelper::setDataSourceOrName( const Any& _rDataSourceOrName )
{
    OSL_ENSURE( !m_aDataSourceOrName.hasValue(), "ODbDataSourceAdministrationHelper::setDataSourceOrName: already have one!" );
        // hmm. We could reset m_xDatasource/m_xModel, probably, and continue working
    m_aDataSourceOrName = _rDataSourceOrName;
}

// DbuTypeCollectionItem
DbuTypeCollectionItem::DbuTypeCollectionItem(sal_Int16 _nWhich, ::dbaccess::ODsnTypeCollection* _pCollection)
    :SfxPoolItem(_nWhich)
    ,m_pCollection(_pCollection)
{
}

DbuTypeCollectionItem::DbuTypeCollectionItem(const DbuTypeCollectionItem& _rSource)
    :SfxPoolItem(_rSource)
    ,m_pCollection(_rSource.getCollection())
{
}

bool DbuTypeCollectionItem::operator==(const SfxPoolItem& _rItem) const
{
    const DbuTypeCollectionItem* pCompare = dynamic_cast<const DbuTypeCollectionItem*>( &_rItem );
    return pCompare && (pCompare->getCollection() == getCollection());
}

SfxPoolItem* DbuTypeCollectionItem::Clone(SfxItemPool* /*_pPool*/) const
{
    return new DbuTypeCollectionItem(*this);
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
