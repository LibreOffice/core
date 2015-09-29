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
    OModuleClient m_aModuleClient;
    ::std::unique_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
    SfxItemSet*             m_pOutSet;
    OUString         m_sURL;
    OUString         m_sOldURL;
    bool                m_bResetting : 1;   /// sal_True while we're resetting the pages
    bool                m_bApplied : 1;     /// sal_True if any changes have been applied while the dialog was executing
    bool                m_bUIEnabled : 1;   /// <TRUE/> if the UI is enabled, false otherwise. Cannot be switched back to <TRUE/>, once it is <FALSE/>
    bool                m_bIsConnectable : 1;
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
    virtual ~ODbTypeWizDialogSetup();
    virtual void dispose() SAL_OVERRIDE;

    virtual const SfxItemSet* getOutputSet() const SAL_OVERRIDE;
    virtual SfxItemSet* getWriteOutputSet() SAL_OVERRIDE;

    // forwards to ODbDataSourceAdministrationHelper
    virtual css::uno::Reference< css::uno::XComponentContext > getORB() const SAL_OVERRIDE;
    virtual ::std::pair< css::uno::Reference< css::sdbc::XConnection >,sal_Bool> createConnection() SAL_OVERRIDE;
    virtual css::uno::Reference< css::sdbc::XDriver > getDriver() SAL_OVERRIDE;
    virtual OUString getDatasourceType(const SfxItemSet& _rSet) const SAL_OVERRIDE;
    virtual void clearPassword() SAL_OVERRIDE;
    virtual void setTitle(const OUString& _sTitle) SAL_OVERRIDE;
    virtual void enableConfirmSettings( bool _bEnable ) SAL_OVERRIDE;
    virtual bool saveDatasource() SAL_OVERRIDE;
    virtual OUString getStateDisplayName( WizardState _nState ) const SAL_OVERRIDE;

    /** returns <TRUE/> if the database should be opened, otherwise <FALSE/>.
    */
    bool IsDatabaseDocumentToBeOpened() const;

    /** returns <TRUE/> if the table wizard should be opened, otherwise <FALSE/>.
    */
    bool IsTableWizardToBeStarted() const;

protected:
    /// to override to create new pages
    virtual VclPtr<TabPage> createPage(WizardState _nState) SAL_OVERRIDE;
    virtual bool        leaveState(WizardState _nState) SAL_OVERRIDE;
    virtual void        enterState(WizardState _nState) SAL_OVERRIDE;
    virtual ::svt::IWizardPageController* getPageController( TabPage* _pCurrentPage ) const SAL_OVERRIDE;
    virtual bool        onFinish() SAL_OVERRIDE;

protected:
    inline void     disabledUI() { m_bUIEnabled = false; }

    /// select a datasource with a given name, adjust the item set accordingly, and everything like that ..
    void implSelectDatasource(const OUString& _rRegisteredName);
    void resetPages(const css::uno::Reference< css::beans::XPropertySet >& _rxDatasource);

    enum ApplyResult
    {
        AR_LEAVE_MODIFIED,      // something was modified and has successfully been committed
        AR_LEAVE_UNCHANGED,     // no changes were made
        AR_KEEP                 // don't leave the page (e.g. because an error occurred)
    };

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
    ::dbaccess::DATASOURCE_TYPE VerifyDataSourceType(const ::dbaccess::DATASOURCE_TYPE _DatabaseType) const;

    void updateTypeDependentStates();
    bool callSaveAsDialog();
    bool IsConnectionUrlRequired();
    DECL_LINK_TYPED(OnTypeSelected, OGeneralPage&, void);
    DECL_LINK_TYPED(OnChangeCreationMode, OGeneralPageWizard&, void);
    DECL_LINK_TYPED(OnRecentDocumentSelected, OGeneralPageWizard&, void);
    DECL_LINK_TYPED(OnSingleDocumentChosen, OGeneralPageWizard&, void);
    DECL_LINK_TYPED(ImplClickHdl, OMySQLIntroPageSetup*, void);
    DECL_LINK_TYPED(ImplModifiedHdl, OGenericAdministrationPage const *, void);
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBWIZSETUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
