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

#ifndef DBAUI_DBWIZ2_HXX
#define DBAUI_DBWIZ2_HXX

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

//.........................................................................
namespace dbaui
{
//.........................................................................

class OGenericAdministrationPage;

//=========================================================================
//= ODbTypeWizDialogSetup
//=========================================================================
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
    ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
    SfxItemSet*             m_pOutSet;
    OUString         m_sURL;
    OUString         m_sOldURL;
    sal_Bool                m_bResetting : 1;   /// sal_True while we're resetting the pages
    sal_Bool                m_bApplied : 1;     /// sal_True if any changes have been applied while the dialog was executing
    sal_Bool                m_bUIEnabled : 1;   /// <TRUE/> if the UI is enabled, false otherwise. Cannot be switched back to <TRUE/>, once it is <FALSE/>
    sal_Bool                m_bIsConnectable : 1;
    String                  m_sRM_IntroText;
    String                  m_sRM_dBaseText;
    String                  m_sRM_TextText;
    String                  m_sRM_MSAccessText;
    String                  m_sRM_LDAPText;
    String                  m_sRM_ADOText;
    String                  m_sRM_JDBCText;
    String                  m_sRM_MySQLNativePageTitle;
    String                  m_sRM_OracleText;
    String                  m_sRM_MySQLText;
    String                  m_sRM_ODBCText;
    String                  m_sRM_SpreadSheetText;
    String                  m_sRM_AuthentificationText;
    String                  m_sRM_FinalText;
    INetURLObject           m_aDocURL;
    String                  m_sWorkPath;
    OGeneralPageWizard*     m_pGeneralPage;
    OMySQLIntroPageSetup*   m_pMySQLIntroPage;
    OFinalDBPageSetup*      m_pFinalPage;

    ::dbaccess::ODsnTypeCollection*
                            m_pCollection;  /// the DSN type collection instance



public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbTypeWizDialogSetup(Window* pParent
        ,SfxItemSet* _pItems
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        ,const ::com::sun::star::uno::Any& _aDataSourceName
        );
    virtual ~ODbTypeWizDialogSetup();

    virtual const SfxItemSet* getOutputSet() const;
    virtual SfxItemSet* getWriteOutputSet();

    // forwards to ODbDataSourceAdministrationHelper
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getORB() const;
    virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver();
    virtual OUString getDatasourceType(const SfxItemSet& _rSet) const;
    virtual void clearPassword();
    virtual void setTitle(const OUString& _sTitle);
    virtual void enableConfirmSettings( bool _bEnable );
    virtual sal_Bool saveDatasource();
    virtual OUString getStateDisplayName( WizardState _nState ) const;

    /** returns <TRUE/> if the database should be opened, otherwise <FALSE/>.
    */
    sal_Bool IsDatabaseDocumentToBeOpened() const;

    /** returns <TRUE/> if the table wizard should be opened, otherwise <FALSE/>.
    */
    sal_Bool IsTableWizardToBeStarted() const;

protected:
    /// to override to create new pages
    virtual TabPage*    createPage(WizardState _nState);
    virtual sal_Bool    leaveState(WizardState _nState);
    virtual void enterState(WizardState _nState);
    virtual ::svt::IWizardPageController* getPageController( TabPage* _pCurrentPage ) const;
    virtual sal_Bool onFinish();

protected:
    inline sal_Bool isUIEnabled() const { return m_bUIEnabled; }
    inline void     disabledUI() { m_bUIEnabled = sal_False; }

    /// select a datasource with a given name, adjust the item set accordingly, and everything like that ..
    void implSelectDatasource(const OUString& _rRegisteredName);
    void resetPages(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDatasource);

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
    sal_Bool SaveDatabaseDocument();
    void activateDatabasePath();
    String createUniqueFileName(const INetURLObject& rURL);
    void CreateDatabase();
    void createUniqueFolderName(INetURLObject* pURL);
    ::dbaccess::DATASOURCE_TYPE VerifyDataSourceType(const ::dbaccess::DATASOURCE_TYPE _DatabaseType) const;

    void updateTypeDependentStates();
    sal_Bool callSaveAsDialog();
    sal_Bool IsConnectionUrlRequired();
    DECL_LINK(OnTypeSelected, OGeneralPage*);
    DECL_LINK(OnChangeCreationMode, OGeneralPageWizard*);
    DECL_LINK(OnRecentDocumentSelected, OGeneralPageWizard*);
    DECL_LINK(OnSingleDocumentChosen, OGeneralPageWizard*);
    DECL_LINK(ImplClickHdl, OMySQLIntroPageSetup*);
    DECL_LINK(ImplModifiedHdl, OGenericAdministrationPage*);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_DBWIZ2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
