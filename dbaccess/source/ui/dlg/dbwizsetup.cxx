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
#include <dbwizsetup.hxx>
#include <dsmeta.hxx>
#include "DBSetupConnectionPages.hxx"
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <dsitems.hxx>
#include "dsnItem.hxx"

#include <unotools/pathoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <stringconstants.hxx>
#include "adminpages.hxx"
#include <sfx2/docfilt.hxx>
#include <unotools/ucbhelper.hxx>
#include "generalpage.hxx"
#include <stringlistitem.hxx>
#include <unotools/confignode.hxx>
#include "DbAdminImpl.hxx"
#include <helpids.h>
#include "ConnectionPageSetup.hxx"
#include <UITools.hxx>
#include <dbaccess/AsynchronousLink.hxx>
#include <sfx2/filedlghelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <svl/filenotation.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <connectivity/DriversConfig.hxx>

namespace dbaui
{
using namespace dbtools;
using namespace svt;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::task;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;
using namespace com::sun::star::ucb;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::document;
using namespace ::comphelper;
using namespace ::cppu;

// ODbTypeWizDialogSetup
ODbTypeWizDialogSetup::ODbTypeWizDialogSetup(vcl::Window* _pParent
                               ,SfxItemSet const * _pItems
                               ,const Reference< XComponentContext >& _rxORB
                               ,const css::uno::Any& _aDataSourceName
                               )
    :svt::RoadmapWizard( _pParent )

    , m_bIsConnectable( false)
    , m_sRM_IntroText( DBA_RES( STR_PAGETITLE_INTROPAGE ) )
    , m_sRM_dBaseText( DBA_RES( STR_PAGETITLE_DBASE ) )
    , m_sRM_TextText( DBA_RES( STR_PAGETITLE_TEXT ) )
    , m_sRM_MSAccessText( DBA_RES( STR_PAGETITLE_MSACCESS ) )
    , m_sRM_LDAPText( DBA_RES( STR_PAGETITLE_LDAP ) )
    , m_sRM_ADOText( DBA_RES( STR_PAGETITLE_ADO ) )
    , m_sRM_JDBCText( DBA_RES( STR_PAGETITLE_JDBC ) )
    , m_sRM_MySQLNativePageTitle( DBA_RES( STR_PAGETITLE_MYSQL_NATIVE ) )
    , m_sRM_OracleText( DBA_RES( STR_PAGETITLE_ORACLE ) )
    , m_sRM_MySQLText( DBA_RES( STR_PAGETITLE_MYSQL ) )
    , m_sRM_ODBCText( DBA_RES( STR_PAGETITLE_ODBC ) )
    , m_sRM_DocumentOrSpreadSheetText( DBA_RES( STR_PAGETITLE_DOCUMENT_OR_SPREADSHEET ) )
    , m_sRM_AuthentificationText( DBA_RES( STR_PAGETITLE_AUTHENTIFICATION ) )
    , m_sRM_FinalText( DBA_RES( STR_PAGETITLE_FINAL ) )
    , m_sWorkPath( SvtPathOptions().GetWorkPath() )
    , m_pGeneralPage( nullptr )
    , m_pMySQLIntroPage( nullptr )
    , m_pFinalPage( nullptr )
    , m_pCollection( nullptr )
{
    // no local resources needed anymore
    // extract the datasource type collection from the item set
    const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _pItems->GetItem(DSID_TYPECOLLECTION) );
    if (pCollectionItem)
        m_pCollection = pCollectionItem->getCollection();

    OSL_ENSURE(m_pCollection, "ODbTypeWizDialogSetup::ODbTypeWizDialogSetup : really need a DSN type collection !");

    m_pImpl.reset(new ODbDataSourceAdministrationHelper(_rxORB,GetFrameWeld(),_pParent ? _pParent->GetFrameWeld() : nullptr, this));
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    m_pOutSet.reset( new SfxItemSet( *_pItems->GetPool(), _pItems->GetRanges() ) );

    m_pImpl->translateProperties(xDatasource, *m_pOutSet);

    SetPageSizePixel(LogicToPixel(::Size(WIZARD_PAGE_X, WIZARD_PAGE_Y), MapMode(MapUnit::MapAppFont)));
    defaultButton(WizardButtonFlags::NEXT);
    enableButtons(WizardButtonFlags::FINISH, true);
    enableAutomaticNextButtonState();

    ::dbaccess::ODsnTypeCollection::TypeIterator aIter = m_pCollection->begin();
    ::dbaccess::ODsnTypeCollection::TypeIterator aEnd = m_pCollection->end();
    for(PathId i = 1;aIter != aEnd;++aIter,++i)
    {
        const OUString& sURLPrefix = aIter.getURLPrefix();
        svt::RoadmapWizardTypes::WizardPath aPath;
        aPath.push_back(PAGE_DBSETUPWIZARD_INTRO);
        m_pCollection->fillPageIds(sURLPrefix,aPath);
        aPath.push_back(PAGE_DBSETUPWIZARD_AUTHENTIFICATION);
        aPath.push_back(PAGE_DBSETUPWIZARD_FINAL);

        declareAuthDepPath(sURLPrefix,i,aPath);
    }

    svt::RoadmapWizardTypes::WizardPath aPath;
    aPath.push_back(PAGE_DBSETUPWIZARD_INTRO);
    declarePath( static_cast<PathId>(m_pCollection->size()+1), aPath);

    m_pPrevPage->SetHelpId(HID_DBWIZ_PREVIOUS);
    m_pNextPage->SetHelpId(HID_DBWIZ_NEXT);
    m_pCancel->SetHelpId(HID_DBWIZ_CANCEL);
    m_pFinish->SetHelpId(HID_DBWIZ_FINISH);
    SetRoadmapInteractive( true );
    ActivatePage();
    setTitleBase(DBA_RES(STR_DBWIZARDTITLE));
}

void ODbTypeWizDialogSetup::declareAuthDepPath( const OUString& _sURL, PathId _nPathId, const svt::RoadmapWizardTypes::WizardPath& _rPaths)
{
    bool bHasAuthentication = DataSourceMetaData::getAuthentication( _sURL ) != AuthNone;

    // collect the elements of the path
    WizardPath aPath;

    for (auto const& path : _rPaths)
    {
        if ( bHasAuthentication || ( path != PAGE_DBSETUPWIZARD_AUTHENTIFICATION ) )
            aPath.push_back(path);
    }

    // call base method
    ::svt::RoadmapWizard::declarePath( _nPathId, aPath );
}

OUString ODbTypeWizDialogSetup::getStateDisplayName( WizardState _nState ) const
{
    OUString sRoadmapItem;
    switch( _nState )
    {
        case PAGE_DBSETUPWIZARD_INTRO:
            sRoadmapItem = m_sRM_IntroText;
            break;

        case PAGE_DBSETUPWIZARD_DBASE:
            sRoadmapItem = m_sRM_dBaseText;
            break;
        case PAGE_DBSETUPWIZARD_ADO:
            sRoadmapItem = m_sRM_ADOText;
            break;
        case PAGE_DBSETUPWIZARD_TEXT:
            sRoadmapItem = m_sRM_TextText;
            break;
        case PAGE_DBSETUPWIZARD_MSACCESS:
            sRoadmapItem = m_sRM_MSAccessText;
            break;
        case PAGE_DBSETUPWIZARD_LDAP:
            sRoadmapItem = m_sRM_LDAPText;
            break;
        case PAGE_DBSETUPWIZARD_JDBC:
            sRoadmapItem = m_sRM_JDBCText;
            break;
        case PAGE_DBSETUPWIZARD_ORACLE:
            sRoadmapItem = m_sRM_OracleText;
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_INTRO:
            sRoadmapItem = m_sRM_MySQLText;
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_JDBC:
            sRoadmapItem = m_sRM_JDBCText;
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_NATIVE:
            sRoadmapItem = m_sRM_MySQLNativePageTitle;
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_ODBC:
            sRoadmapItem = m_sRM_ODBCText;
            break;
        case PAGE_DBSETUPWIZARD_ODBC:
            sRoadmapItem = m_sRM_ODBCText;
            break;
        case PAGE_DBSETUPWIZARD_DOCUMENT_OR_SPREADSHEET:
            sRoadmapItem = m_sRM_DocumentOrSpreadSheetText;
            break;
        case PAGE_DBSETUPWIZARD_AUTHENTIFICATION:
            sRoadmapItem = m_sRM_AuthentificationText;
            break;
        case PAGE_DBSETUPWIZARD_USERDEFINED:
            sRoadmapItem = DBA_RES(STR_PAGETITLE_CONNECTION);
            break;
        case PAGE_DBSETUPWIZARD_FINAL:
            sRoadmapItem = m_sRM_FinalText;
            break;
        default:
            break;
    }
    return sRoadmapItem;
}

ODbTypeWizDialogSetup::~ODbTypeWizDialogSetup()
{
    disposeOnce();
}

void ODbTypeWizDialogSetup::dispose()
{
    m_pOutSet.reset();
    m_pGeneralPage.clear();
    m_pMySQLIntroPage.clear();
    m_pFinalPage.clear();
    svt::RoadmapWizard::dispose();
}

IMPL_LINK_NOARG(ODbTypeWizDialogSetup, OnTypeSelected, OGeneralPage&, void)
{
    activateDatabasePath();
}

static void lcl_removeUnused(const ::comphelper::NamedValueCollection& _aOld,const ::comphelper::NamedValueCollection& _aNew,::comphelper::NamedValueCollection& _rDSInfo)
{
    _rDSInfo.merge(_aNew,true);
    uno::Sequence< beans::NamedValue > aOldValues = _aOld.getNamedValues();
    const beans::NamedValue* pIter = aOldValues.getConstArray();
    const beans::NamedValue* pEnd  = pIter + aOldValues.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if ( !_aNew.has(pIter->Name) )
        {
            _rDSInfo.remove(pIter->Name);
        }
    }
}

void DataSourceInfoConverter::convert(const Reference<XComponentContext> & xContext, const ::dbaccess::ODsnTypeCollection* _pCollection,const OUString& _sOldURLPrefix,const OUString& _sNewURLPrefix,const css::uno::Reference< css::beans::XPropertySet >& _xDatasource)
{
    if ( _pCollection->getPrefix(_sOldURLPrefix) == _pCollection->getPrefix(_sNewURLPrefix) )
        return ;
    uno::Sequence< beans::PropertyValue> aInfo;
    _xDatasource->getPropertyValue(PROPERTY_INFO) >>= aInfo;
    ::comphelper::NamedValueCollection aDS(aInfo);

    ::connectivity::DriversConfig aDriverConfig(xContext);

    const ::comphelper::NamedValueCollection&  aOldProperties   = aDriverConfig.getProperties(_sOldURLPrefix);
    const ::comphelper::NamedValueCollection&  aNewProperties   = aDriverConfig.getProperties(_sNewURLPrefix);
    lcl_removeUnused(aOldProperties,aNewProperties,aDS);

    aDS >>= aInfo;
    _xDatasource->setPropertyValue(PROPERTY_INFO,uno::makeAny(aInfo));
}

void ODbTypeWizDialogSetup::activateDatabasePath()
{
    switch ( m_pGeneralPage->GetDatabaseCreationMode() )
    {
    case OGeneralPageWizard::eCreateNew:
    {
        sal_Int32 nCreateNewDBIndex = m_pCollection->getIndexOf( m_pGeneralPage->GetSelectedType() );
        if ( nCreateNewDBIndex == -1 )
            nCreateNewDBIndex = m_pCollection->getIndexOf( "sdbc:dbase:" );
        OSL_ENSURE( nCreateNewDBIndex != -1, "ODbTypeWizDialogSetup::activateDatabasePath: the GeneralPage should have prevented this!" );
        activatePath( static_cast< PathId >( nCreateNewDBIndex + 1 ), true );

        enableState(PAGE_DBSETUPWIZARD_FINAL );
        enableButtons( WizardButtonFlags::FINISH, true);
    }
    break;
    case OGeneralPageWizard::eConnectExternal:
    {
        OUString sOld = m_sURL;
        m_sURL = m_pGeneralPage->GetSelectedType();
        DataSourceInfoConverter::convert(getORB(), m_pCollection,sOld,m_sURL,m_pImpl->getCurrentDataSource());
        ::dbaccess::DATASOURCE_TYPE eType = VerifyDataSourceType(m_pCollection->determineType(m_sURL));
        if (eType ==  ::dbaccess::DST_UNKNOWN)
            m_pCollection->determineType(m_sOldURL);

        activatePath( static_cast<PathId>(m_pCollection->getIndexOf(m_sURL) + 1), true);
        updateTypeDependentStates();
    }
    break;
    case OGeneralPageWizard::eOpenExisting:
    {
        activatePath( static_cast<PathId>(m_pCollection->size() + 1), true );
        enableButtons( WizardButtonFlags::FINISH, !m_pGeneralPage->GetSelectedDocumentURL().isEmpty() );
    }
    break;
    default:
        OSL_FAIL( "ODbTypeWizDialogSetup::activateDatabasePath: unknown creation mode!" );
    }

    enableButtons( WizardButtonFlags::NEXT, m_pGeneralPage->GetDatabaseCreationMode() != OGeneralPageWizard::eOpenExisting );
        // TODO: this should go into the base class. Point is, we activate a path whose *last*
        // step is also the current one. The base class should automatically disable
        // the Next button in such a case. However, not for this patch ...
}

void ODbTypeWizDialogSetup::updateTypeDependentStates()
{
    bool bDoEnable = false;
    bool bIsConnectionRequired = m_pCollection->isConnectionUrlRequired(m_sURL);
    if (!bIsConnectionRequired)
    {
        bDoEnable = true;
    }
    else if ( m_sURL == m_sOldURL )
    {
        bDoEnable = m_bIsConnectable;
    }
    enableState(PAGE_DBSETUPWIZARD_AUTHENTIFICATION, bDoEnable);
    enableState(PAGE_DBSETUPWIZARD_FINAL, bDoEnable );
    enableButtons( WizardButtonFlags::FINISH, bDoEnable);
}

void ODbTypeWizDialogSetup::resetPages(const Reference< XPropertySet >& _rxDatasource)
{
    // remove all items which relate to indirect properties from the input set
    // (without this, the following may happen: select an arbitrary data source where some indirect properties
    // are set. Select another data source of the same type, where the indirect props are not set (yet). Then,
    // the indirect property values of the first ds are shown in the second ds ...)
    const ODbDataSourceAdministrationHelper::MapInt2String& rMap = m_pImpl->getIndirectProperties();
    for (auto const& elem : rMap)
        getWriteOutputSet()->ClearItem( static_cast<sal_uInt16>(elem.first) );

    // extract all relevant data from the property set of the data source
    m_pImpl->translateProperties(_rxDatasource, *getWriteOutputSet());
}

const SfxItemSet* ODbTypeWizDialogSetup::getOutputSet() const
{
    return m_pOutSet.get();
}

SfxItemSet* ODbTypeWizDialogSetup::getWriteOutputSet()
{
    return m_pOutSet.get();
}

std::pair< Reference<XConnection>,bool> ODbTypeWizDialogSetup::createConnection()
{
    return m_pImpl->createConnection();
}

Reference< XComponentContext > ODbTypeWizDialogSetup::getORB() const
{
    return m_pImpl->getORB();
}

Reference< XDriver > ODbTypeWizDialogSetup::getDriver()
{
    return m_pImpl->getDriver();
}

::dbaccess::DATASOURCE_TYPE ODbTypeWizDialogSetup::VerifyDataSourceType(const ::dbaccess::DATASOURCE_TYPE DatabaseType) const
{
    ::dbaccess::DATASOURCE_TYPE LocDatabaseType = DatabaseType;
    if ((LocDatabaseType ==  ::dbaccess::DST_MYSQL_JDBC) || (LocDatabaseType ==  ::dbaccess::DST_MYSQL_ODBC) || (LocDatabaseType ==  ::dbaccess::DST_MYSQL_NATIVE))
    {
        if (m_pMySQLIntroPage != nullptr)
        {
            switch( m_pMySQLIntroPage->getMySQLMode() )
            {
                case OMySQLIntroPageSetup::VIA_JDBC:
                    return  ::dbaccess::DST_MYSQL_JDBC;
                case OMySQLIntroPageSetup::VIA_NATIVE:
                    return  ::dbaccess::DST_MYSQL_NATIVE;
                case OMySQLIntroPageSetup::VIA_ODBC:
                    return  ::dbaccess::DST_MYSQL_ODBC;
            }
        }
    }
    return LocDatabaseType;
}

OUString ODbTypeWizDialogSetup::getDatasourceType(const SfxItemSet& _rSet) const
{
    OUString sRet = dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(_rSet);
    if (m_pMySQLIntroPage != nullptr && m_pMySQLIntroPage->IsVisible() )
    {
        switch( m_pMySQLIntroPage->getMySQLMode() )
        {
            case OMySQLIntroPageSetup::VIA_JDBC:
                sRet = "sdbc:mysql:jdbc:";
                break;
            case OMySQLIntroPageSetup::VIA_NATIVE:
                sRet = "sdbc:mysql:mysqlc:";
                break;
            case OMySQLIntroPageSetup::VIA_ODBC:
                sRet = "sdbc:mysql:odbc:";
                break;
        }
    }
    return sRet;
}

void ODbTypeWizDialogSetup::clearPassword()
{
    m_pImpl->clearPassword();
}

VclPtr<TabPage> ODbTypeWizDialogSetup::createPage(WizardState _nState)
{
    VclPtr<SfxTabPage> pFirstPage;
    VclPtr<OGenericAdministrationPage> pPage;
    switch(_nState)
    {
        case PAGE_DBSETUPWIZARD_INTRO:
            pFirstPage = VclPtr<OGeneralPageWizard>::Create(this,*m_pOutSet);
            pPage = static_cast<OGenericAdministrationPage*> (pFirstPage.get());
            m_pGeneralPage = static_cast<OGeneralPageWizard*>(pFirstPage.get());
            m_pGeneralPage->SetTypeSelectHandler(LINK(this, ODbTypeWizDialogSetup, OnTypeSelected));
            m_pGeneralPage->SetCreationModeHandler(LINK( this, ODbTypeWizDialogSetup, OnChangeCreationMode ) );
            m_pGeneralPage->SetDocumentSelectionHandler(LINK( this, ODbTypeWizDialogSetup, OnRecentDocumentSelected ) );
            m_pGeneralPage->SetChooseDocumentHandler(LINK( this, ODbTypeWizDialogSetup, OnSingleDocumentChosen ) );
            break;

        case PAGE_DBSETUPWIZARD_DBASE:
            pPage = OConnectionTabPageSetup::CreateDbaseTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_ADO:
            pPage = OConnectionTabPageSetup::CreateADOTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_TEXT:
            pPage = OTextConnectionPageSetup::CreateTextTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_ODBC:
            pPage = OConnectionTabPageSetup::CreateODBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_JDBC:
            pPage = OJDBCConnectionPageSetup::CreateJDBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_MYSQL_ODBC:
            m_pOutSet->Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getPrefix("sdbc:mysql:odbc:")));
            pPage = OConnectionTabPageSetup::CreateODBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_MYSQL_JDBC:
            m_pOutSet->Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getPrefix("sdbc:mysql:jdbc:")));
            pPage = OGeneralSpecialJDBCConnectionPageSetup::CreateMySQLJDBCTabPage( this, *m_pOutSet);
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_NATIVE:
            m_pOutSet->Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getPrefix("sdbc:mysql:mysqlc:")));
            pPage = MySQLNativeSetupPage::Create( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_ORACLE:
            pPage = OGeneralSpecialJDBCConnectionPageSetup::CreateOracleJDBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_LDAP:
            pPage = OLDAPConnectionPageSetup::CreateLDAPTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_DOCUMENT_OR_SPREADSHEET:
            pPage = OSpreadSheetConnectionPageSetup::CreateDocumentOrSpreadSheetTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_MSACCESS:
            pPage  = OConnectionTabPageSetup::CreateMSAccessTabPage(this,*m_pOutSet);
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_INTRO:
            m_pMySQLIntroPage = OMySQLIntroPageSetup::CreateMySQLIntroTabPage(this,*m_pOutSet);
            m_pMySQLIntroPage->SetClickHdl(LINK( this, ODbTypeWizDialogSetup, ImplClickHdl ) );
            pPage = m_pMySQLIntroPage;
            break;

        case PAGE_DBSETUPWIZARD_AUTHENTIFICATION:
            pPage = OAuthentificationPageSetup::CreateAuthentificationTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_USERDEFINED:
            pPage = OConnectionTabPageSetup::CreateUserDefinedTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_FINAL:
            pPage = OFinalDBPageSetup::CreateFinalDBTabPageSetup(this,*m_pOutSet);
            m_pFinalPage = static_cast<OFinalDBPageSetup*> (pPage.get());
            break;
    }

    if ( pPage )
    {
        if ((_nState != PAGE_DBSETUPWIZARD_INTRO) && (_nState != PAGE_DBSETUPWIZARD_AUTHENTIFICATION))
        {
            pPage->SetModifiedHandler(LINK( this, ODbTypeWizDialogSetup, ImplModifiedHdl ) );
        }

        pPage->SetServiceFactory( m_pImpl->getORB() );
        pPage->SetAdminDialog(this, this);

        defaultButton( _nState == PAGE_DBSETUPWIZARD_FINAL ? WizardButtonFlags::FINISH : WizardButtonFlags::NEXT );
        enableButtons( WizardButtonFlags::FINISH, _nState == PAGE_DBSETUPWIZARD_FINAL );
        enableButtons( WizardButtonFlags::NEXT, _nState != PAGE_DBSETUPWIZARD_FINAL );
        pPage->Show();
    }
    return pPage;
}

IMPL_LINK(ODbTypeWizDialogSetup, ImplModifiedHdl, OGenericAdministrationPage const *, _pConnectionPageSetup, void)
{
    m_bIsConnectable = _pConnectionPageSetup->GetRoadmapStateValue( );
    enableState(PAGE_DBSETUPWIZARD_FINAL, m_bIsConnectable);
    enableState(PAGE_DBSETUPWIZARD_AUTHENTIFICATION, m_bIsConnectable);
    if (getCurrentState() == PAGE_DBSETUPWIZARD_FINAL)
        enableButtons( WizardButtonFlags::FINISH, true);
    else
        enableButtons( WizardButtonFlags::FINISH, m_bIsConnectable);
    enableButtons( WizardButtonFlags::NEXT, m_bIsConnectable  && (getCurrentState() != PAGE_DBSETUPWIZARD_FINAL));
}

IMPL_LINK(ODbTypeWizDialogSetup, ImplClickHdl, OMySQLIntroPageSetup*, _pMySQLIntroPageSetup, void)
{
    OUString sURLPrefix;
    switch( _pMySQLIntroPageSetup->getMySQLMode() )
    {
        case  OMySQLIntroPageSetup::VIA_ODBC:
            sURLPrefix = "sdbc:mysql:odbc:";
            break;
        case  OMySQLIntroPageSetup::VIA_JDBC:
            sURLPrefix = "sdbc:mysql:jdbc:";
            break;
        case  OMySQLIntroPageSetup::VIA_NATIVE:
            sURLPrefix = "sdbc:mysql:mysqlc:";
            break;
    }
    activatePath( static_cast<PathId>(m_pCollection->getIndexOf(sURLPrefix) + 1), true);
}

IMPL_LINK_NOARG(ODbTypeWizDialogSetup, OnChangeCreationMode, OGeneralPageWizard&, void)
{
    activateDatabasePath();
}

IMPL_LINK_NOARG(ODbTypeWizDialogSetup, OnRecentDocumentSelected, OGeneralPageWizard&, void)
{
    enableButtons( WizardButtonFlags::FINISH, !m_pGeneralPage->GetSelectedDocumentURL().isEmpty() );
}

IMPL_LINK_NOARG(ODbTypeWizDialogSetup, OnSingleDocumentChosen, OGeneralPageWizard&, void)
{
    if ( prepareLeaveCurrentState( eFinish ) )
        onFinish();
}

void ODbTypeWizDialogSetup::enterState(WizardState _nState)
{
    m_sURL = dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(*m_pOutSet);
    RoadmapWizard::enterState(_nState);
    switch(_nState)
    {
        case PAGE_DBSETUPWIZARD_INTRO:
            m_sOldURL = m_sURL;
            break;
        case PAGE_DBSETUPWIZARD_FINAL:
            enableButtons( WizardButtonFlags::FINISH, true);
            if ( m_pFinalPage )
                m_pFinalPage->enableTableWizardCheckBox(m_pCollection->supportsTableCreation(m_sURL));
            break;
    }
}

void ODbTypeWizDialogSetup::saveDatasource()
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(getCurrentState()));
    if ( pPage )
        pPage->FillItemSet(m_pOutSet.get());
}

bool ODbTypeWizDialogSetup::leaveState(WizardState _nState)
{
    if (_nState == PAGE_DBSETUPWIZARD_MYSQL_INTRO)
        return true;
    if ( _nState == PAGE_DBSETUPWIZARD_INTRO && m_sURL != m_sOldURL )
    {
        resetPages(m_pImpl->getCurrentDataSource());
    }
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(_nState));
    return pPage && pPage->DeactivatePage(m_pOutSet.get()) != DeactivateRC::KeepPage;
}

void ODbTypeWizDialogSetup::setTitle(const OUString& /*_sTitle*/)
{
    OSL_FAIL( "ODbTypeWizDialogSetup::setTitle: not implemented!" );
        // why?
}

void ODbTypeWizDialogSetup::enableConfirmSettings( bool /*_bEnable*/ )
{
}

namespace
{
    bool lcl_handle( const Reference< XInteractionHandler2 >& _rxHandler, const Any& _rRequest )
    {
        OInteractionRequest* pRequest = new OInteractionRequest( _rRequest );
        Reference < XInteractionRequest > xRequest( pRequest );
        OInteractionAbort* pAbort = new OInteractionAbort;
        pRequest->addContinuation( pAbort );

        return _rxHandler->handleInteractionRequest( xRequest );
    }
}

bool ODbTypeWizDialogSetup::SaveDatabaseDocument()
{
    Reference< XInteractionHandler2 > xHandler( InteractionHandler::createWithParent(getORB(), nullptr) );
    try
    {
        if (callSaveAsDialog())
        {
            m_pImpl->saveChanges(*m_pOutSet);
            Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
            Reference< XModel > xModel( getDataSourceOrModel( xDatasource ), UNO_QUERY_THROW );
            Reference< XStorable > xStore( xModel, UNO_QUERY_THROW );

            if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPageWizard::eCreateNew )
                CreateDatabase();

            ::comphelper::NamedValueCollection aArgs( xModel->getArgs() );
            aArgs.put( "Overwrite", true );
            aArgs.put( "InteractionHandler", xHandler );
            aArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );

            OUString sPath = ODbDataSourceAdministrationHelper::getDocumentUrl( *m_pOutSet );
            xStore->storeAsURL( sPath, aArgs.getPropertyValues() );

            if ( !m_pFinalPage || m_pFinalPage->IsDatabaseDocumentToBeRegistered() )
                RegisterDataSourceByLocation( sPath );

            return true;
        }
    }
    catch ( const Exception& e )
    {
        Any aError = ::cppu::getCaughtException();
        if ( xHandler.is() )
        {
            if ( !lcl_handle( xHandler, aError ) )
            {
                InteractiveIOException aRequest;
                aRequest.Classification = InteractionClassification_ERROR;
                if ( aError.isExtractableTo( ::cppu::UnoType< IOException >::get() ) )
                    // assume saving the document failed
                    aRequest.Code = IOErrorCode_CANT_WRITE;
                else
                    aRequest.Code = IOErrorCode_GENERAL;
                aRequest.Message = e.Message;
                aRequest.Context = e.Context;
                lcl_handle( xHandler, makeAny( aRequest ) );
            }
        }
    }
    return false;
}

    bool ODbTypeWizDialogSetup::IsDatabaseDocumentToBeOpened() const
    {
        if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPageWizard::eOpenExisting )
            return true;

        if ( m_pFinalPage != nullptr )
            return m_pFinalPage->IsDatabaseDocumentToBeOpened();

        return true;
    }

    bool ODbTypeWizDialogSetup::IsTableWizardToBeStarted() const
    {
        if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPageWizard::eOpenExisting )
            return false;

        if ( m_pFinalPage != nullptr )
            return m_pFinalPage->IsTableWizardToBeStarted();

        return false;
    }

    void ODbTypeWizDialogSetup::CreateDatabase()
    {
        OUString sUrl;
        const OUString eType = m_pGeneralPage->GetSelectedType();
        if ( dbaccess::ODsnTypeCollection::isEmbeddedDatabase(eType) )
        {
            sUrl = eType;
            Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
            OSL_ENSURE(xDatasource.is(),"DataSource is null!");
            if ( xDatasource.is() )
                xDatasource->setPropertyValue( PROPERTY_INFO, makeAny( m_pCollection->getDefaultDBSettings( eType ) ) );
            m_pImpl->translateProperties(xDatasource,*m_pOutSet);
        }
        else if ( m_pCollection->isFileSystemBased(eType) )
        {
            Reference< XSimpleFileAccess3 > xSimpleFileAccess(ucb::SimpleFileAccess::create(getORB()));
            INetURLObject aDBPathURL(m_sWorkPath);
            aDBPathURL.Append(m_aDocURL.getBase());
            createUniqueFolderName(&aDBPathURL);
            sUrl = aDBPathURL.GetMainURL( INetURLObject::DecodeMechanism::NONE);
            xSimpleFileAccess->createFolder(sUrl);
            sUrl = eType.concat(sUrl);
        }
        m_pOutSet->Put(SfxStringItem(DSID_CONNECTURL, sUrl));
        m_pImpl->saveChanges(*m_pOutSet);
    }

    void ODbTypeWizDialogSetup::RegisterDataSourceByLocation(const OUString& _sPath)
    {
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        Reference< XDatabaseContext > xDatabaseContext( DatabaseContext::create(getORB()) );
        INetURLObject aURL( _sPath );
        OUString sFilename = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
        OUString sDatabaseName = ::dbtools::createUniqueName(xDatabaseContext, sFilename, false);
        xDatabaseContext->registerObject(sDatabaseName, xDatasource);
    }

    bool ODbTypeWizDialogSetup::callSaveAsDialog()
    {
        bool bRet = false;
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
                FileDialogFlags::NONE, GetFrameWeld());
        std::shared_ptr<const SfxFilter> pFilter = getStandardDatabaseFilter();
        if ( pFilter )
        {
            INetURLObject aWorkURL( m_sWorkPath );
            aFileDlg.SetDisplayFolder( aWorkURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ));

            OUString sDefaultName = DBA_RES( STR_DATABASEDEFAULTNAME );
            OUString sExtension = pFilter->GetDefaultExtension();
            sDefaultName += sExtension.replaceAt( 0, 1, OUString() );
            aWorkURL.Append( sDefaultName );
            sDefaultName = createUniqueFileName( aWorkURL );
            aFileDlg.SetFileName( sDefaultName );

            aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
            aFileDlg.SetCurrentFilter(pFilter->GetUIName());
        }
        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            m_aDocURL = INetURLObject(aFileDlg.GetPath());

            if( m_aDocURL.GetProtocol() != INetProtocol::NotValid )
            {
                OUString sFileName = m_aDocURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                if ( ::utl::UCBContentHelper::IsDocument(sFileName) )
                    ::utl::UCBContentHelper::Kill(sFileName);
                m_pOutSet->Put(SfxStringItem(DSID_DOCUMENT_URL, sFileName));
                bRet = true;
            }
        }
        return bRet;
    }

    void ODbTypeWizDialogSetup::createUniqueFolderName(INetURLObject* pURL)
    {
        Reference< XSimpleFileAccess3 > xSimpleFileAccess(ucb::SimpleFileAccess::create(getORB()));
        OUString sLastSegmentName = pURL->getName();
        bool bFolderExists = true;
        sal_Int32 i = 1;
        while (bFolderExists)
        {
            bFolderExists = xSimpleFileAccess->isFolder(pURL->GetMainURL( INetURLObject::DecodeMechanism::NONE ));
            if (bFolderExists)
            {
                i++;
                pURL->setName(sLastSegmentName.concat(OUString::number(i)));
            }
        }
    }

    OUString ODbTypeWizDialogSetup::createUniqueFileName(const INetURLObject& _rURL)
    {
        Reference< XSimpleFileAccess3 > xSimpleFileAccess(ucb::SimpleFileAccess::create(getORB()));
        OUString BaseName = _rURL.getBase();

        bool bElementExists = true;

        INetURLObject aExistenceCheck( _rURL );
        for ( sal_Int32 i = 1; bElementExists; )
        {
            bElementExists = xSimpleFileAccess->exists( aExistenceCheck.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            if ( bElementExists )
            {
                aExistenceCheck.setBase( BaseName.concat( OUString::number( i ) ) );
                ++i;
            }
        }
        return aExistenceCheck.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
    }
    IWizardPageController* ODbTypeWizDialogSetup::getPageController( TabPage* _pCurrentPage ) const
    {
        OGenericAdministrationPage* pPage = static_cast<OGenericAdministrationPage*>(_pCurrentPage);
        return pPage;
    }

    namespace
    {
        typedef ::cppu::WeakImplHelper<   XTerminateListener
                                      >   AsyncLoader_Base;
        class AsyncLoader : public AsyncLoader_Base
        {
        private:
            Reference< XComponentLoader >       m_xFrameLoader;
            Reference< XDesktop2 >              m_xDesktop;
            Reference< XInteractionHandler2 >   m_xInteractionHandler;
            OUString                     m_sURL;
            OAsynchronousLink                    m_aAsyncCaller;

        public:
            AsyncLoader( const Reference< XComponentContext >& _rxORB, const OUString& _rURL );

            void doLoadAsync();

            // XTerminateListener
            virtual void SAL_CALL queryTermination( const css::lang::EventObject& Event ) override;
            virtual void SAL_CALL notifyTermination( const css::lang::EventObject& Event ) override;
            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        private:
            DECL_LINK( OnOpenDocument, void*, void );
        };

        AsyncLoader::AsyncLoader( const Reference< XComponentContext >& _rxORB, const OUString& _rURL )
            :m_sURL( _rURL )
            ,m_aAsyncCaller( LINK( this, AsyncLoader, OnOpenDocument ) )
        {
            try
            {
                m_xDesktop.set( Desktop::create(_rxORB) );
                m_xFrameLoader.set( m_xDesktop, UNO_QUERY_THROW );
                m_xInteractionHandler = InteractionHandler::createWithParent(_rxORB, nullptr);
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }

        void AsyncLoader::doLoadAsync()
        {
            OSL_ENSURE( !m_aAsyncCaller.IsRunning(), "AsyncLoader:doLoadAsync: already running!" );

            acquire();
            try
            {
                if ( m_xDesktop.is() )
                    m_xDesktop->addTerminateListener( this );
            }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION("dbaccess"); }

            m_aAsyncCaller.Call();
        }

        IMPL_LINK_NOARG( AsyncLoader, OnOpenDocument, void*, void )
        {
            try
            {
                if ( m_xFrameLoader.is() )
                {
                    ::comphelper::NamedValueCollection aLoadArgs;
                    aLoadArgs.put( "InteractionHandler", m_xInteractionHandler );
                    aLoadArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );

                    Sequence< PropertyValue > aLoadArgPV;
                    aLoadArgs >>= aLoadArgPV;

                    m_xFrameLoader->loadComponentFromURL( m_sURL,
                        "_default",
                        FrameSearchFlag::ALL,
                        aLoadArgPV
                    );
                }
            }
            catch( const Exception& )
            {
                // do not assert.
                // Such an exception happens for instance of the to-be-loaded document does not exist anymore.
            }

            try
            {
                if ( m_xDesktop.is() )
                    m_xDesktop->removeTerminateListener( this );
            }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION("dbaccess"); }

            release();
        }

        void SAL_CALL AsyncLoader::queryTermination( const css::lang::EventObject& /*Event*/ )
        {
            throw TerminationVetoException();
        }

        void SAL_CALL AsyncLoader::notifyTermination( const css::lang::EventObject& /*Event*/ )
        {
        }
        void SAL_CALL AsyncLoader::disposing( const css::lang::EventObject& /*Source*/ )
        {
        }
    }

    bool ODbTypeWizDialogSetup::onFinish()
    {
        if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPageWizard::eOpenExisting )
        {
            // we're not going to re-use the XModel we have - since the document the user
            // wants us to load could be a non-database document. Instead, we asynchronously
            // open the selected document. Thus, the wizard's return value is RET_CANCEL,
            // which means to not continue loading the database document
            if ( !OWizardMachine::Finish() )
                return false;

            try
            {
                AsyncLoader* pAsyncLoader = new AsyncLoader( getORB(), m_pGeneralPage->GetSelectedDocumentURL() );
                ::rtl::Reference< AsyncLoader > xKeepAlive( pAsyncLoader );
                pAsyncLoader->doLoadAsync();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }

            return true;
        }

        if (getCurrentState() != PAGE_DBSETUPWIZARD_FINAL)
        {
            skipUntil(PAGE_DBSETUPWIZARD_FINAL);
        }
        if (getCurrentState() == PAGE_DBSETUPWIZARD_FINAL)
            return SaveDatabaseDocument() && OWizardMachine::onFinish();
        else
        {
            enableButtons( WizardButtonFlags::FINISH, false );
            return false;
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
