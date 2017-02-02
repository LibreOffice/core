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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DBWIZSETUP_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DBWIZSETUP_HXX

#include <sfx2/tabdlg.hxx>
#include "dsntypes.hxx"
#include "IItemSetHelper.hxx"
#include <comphelper/uno3.hxx>
#include <tools/urlobj.hxx>
#include <memory>
#include <svtools/roadmapwizard.hxx>
#include <connectivity/dbtools.hxx>
#include "moduledbu.hxx"

namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertySet;
    }
    namespace sdbc {
        class XConnection;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

namespace dbaui
{

class OGenericAdministrationPage;

// ODbTypeWizDialogSetup
class OGeneralPage;
class OGeneralPageWizard;
class ODbDataSourceAdministrationHelper;
/** tab dialog for administrating the office wide registered data sources
*/
class OMySQLIntroPageSetup;
class OFinalDBPageSetup;

class ODbTypeWizDialogSetup : public svt::RoadmapWizard , public IItemSetHelper, public IDatabaseSettingsDialog,public dbaui::OModuleClient
{

private:
    OModuleClient           m_aModuleClient;
    ::std::unique_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
    SfxItemSet*             m_pOutSet;
    OUString                m_sURL;
    OUString                m_sOldURL;
    bool                    m_bIsConnectable : 1;
    OUString                m_sRM_IntroText;
    OUString                m_sRM_dBaseText;
    OUString                m_sRM_TextText;
    OUString                m_sRM_MSAccessText;
    OUString                m_sRM_LDAPText;
    OUString                m_sRM_ADOText;
    OUString                m_sRM_JDBCText;
    OUString                m_sRM_MySQLNativePageTitle;
    OUString                m_sRM_OracleText;
    OUString                m_sRM_MySQLText;
    OUString                m_sRM_ODBCText;
    OUString                m_sRM_SpreadSheetText;
    OUString                m_sRM_AuthentificationText;
    OUString                m_sRM_FinalText;
    INetURLObject           m_aDocURL;
    OUString                m_sWorkPath;
    VclPtr<OGeneralPageWizard>     m_pGeneralPage;
    VclPtr<OMySQLIntroPageSetup>   m_pMySQLIntroPage;
    VclPtr<OFinalDBPageSetup>      m_pFinalPage;

    ::dbaccess::ODsnTypeCollection*
                            m_pCollection;  /// the DSN type collection instance

public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbTypeWizDialogSetup(vcl::Window* pParent
        ,SfxItemSet* _pItems
        ,const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        ,const css::uno::Any& _aDataSourceName
        );
    virtual ~ODbTypeWizDialogSetup() override;
    virtual void dispose() override;

    virtual const SfxItemSet* getOutputSet() const override;
    virtual SfxItemSet* getWriteOutputSet() override;

    // forwards to ODbDataSourceAdministrationHelper
    virtual css::uno::Reference< css::uno::XComponentContext > getORB() const override;
    virtual ::std::pair< css::uno::Reference< css::sdbc::XConnection >,sal_Bool> createConnection() override;
    virtual css::uno::Reference< css::sdbc::XDriver > getDriver() override;
    virtual OUString getDatasourceType(const SfxItemSet& _rSet) const override;
    virtual void clearPassword() override;
    virtual void setTitle(const OUString& _sTitle) override;
    virtual void enableConfirmSettings( bool _bEnable ) override;
    virtual void saveDatasource() override;
    virtual OUString getStateDisplayName( WizardState _nState ) const override;

    /** returns <TRUE/> if the database should be opened, otherwise <FALSE/>.
    */
    bool IsDatabaseDocumentToBeOpened() const;

    /** returns <TRUE/> if the table wizard should be opened, otherwise <FALSE/>.
    */
    bool IsTableWizardToBeStarted() const;

protected:
    /// to override to create new pages
    virtual VclPtr<TabPage> createPage(WizardState _nState) override;
    virtual bool        leaveState(WizardState _nState) override;
    virtual void        enterState(WizardState _nState) override;
    virtual ::svt::IWizardPageController* getPageController( TabPage* _pCurrentPage ) const override;
    virtual bool        onFinish() override;

    void resetPages(const css::uno::Reference< css::beans::XPropertySet >& _rxDatasource);

private:
    /** declares a path with or without authentication, as indicated by the database type

        @param _sURL
            the data source type for which the path is declared. If this
            data source type does not support authentication, the PAGE_DBSETUPWIZARD_AUTHENTIFICATION
            state will be stripped from the sequence of states.
        @param _nPathId
            the ID of the path
        @path
            the first state in this path, following by an arbitrary number of others, as in
            RoadmapWizard::declarePath.
    */
    void declareAuthDepPath( const OUString& _sURL, PathId _nPathId, const svt::RoadmapWizardTypes::WizardPath& _rPaths);

    void RegisterDataSourceByLocation(const OUString& sPath);
    bool SaveDatabaseDocument();
    void activateDatabasePath();
    OUString createUniqueFileName(const INetURLObject& rURL);
    void CreateDatabase();
    void createUniqueFolderName(INetURLObject* pURL);
    ::dbaccess::DATASOURCE_TYPE VerifyDataSourceType(const ::dbaccess::DATASOURCE_TYPE DatabaseType) const;

    void updateTypeDependentStates();
    bool callSaveAsDialog();
    DECL_LINK(OnTypeSelected, OGeneralPage&, void);
    DECL_LINK(OnChangeCreationMode, OGeneralPageWizard&, void);
    DECL_LINK(OnRecentDocumentSelected, OGeneralPageWizard&, void);
    DECL_LINK(OnSingleDocumentChosen, OGeneralPageWizard&, void);
    DECL_LINK(ImplClickHdl, OMySQLIntroPageSetup*, void);
    DECL_LINK(ImplModifiedHdl, OGenericAdministrationPage const *, void);
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBWIZSETUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
