/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbwizsetup.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:26:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef DBAUI_DBWIZ2_HXX
#define DBAUI_DBWIZ2_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef DBAUI_ITEMSETHELPER_HXX
#include "IItemSetHelper.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <memory>
#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#include <svtools/roadmapwizard.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

//.........................................................................
namespace dbaui
{
//.........................................................................

class ODsnTypeCollection;
class OGenericAdministrationPage;

//=========================================================================
//= ODbTypeWizDialogSetup
//=========================================================================
class OGeneralPage;
class ODbDataSourceAdministrationHelper;
/** tab dialog for administrating the office wide registered data sources
*/
class OMySQLIntroPageSetup;

class ODbTypeWizDialogSetup : public svt::RoadmapWizard , public IItemSetHelper, public IDatabaseSettingsDialog,public dbaui::OModuleClient
{

private:
    OModuleClient m_aModuleClient;
    ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
    SfxItemSet*             m_pOutSet;
    DATASOURCE_TYPE         m_eType;
    DATASOURCE_TYPE         m_eOldType;
    sal_Bool                m_bResetting : 1;   /// sal_True while we're resetting the pages
    sal_Bool                m_bApplied : 1;     /// sal_True if any changes have been applied while the dialog was executing
    sal_Bool                m_bUIEnabled : 1;   /// <TRUE/> if the UI is enabled, false otherwise. Cannot be switched back to <TRUE/>, once it is <FALSE/>
    sal_Bool                m_bIsConnectable : 1;
    String                  m_sRM_IntroText;
    String                  m_sRM_dBaseText;
    String                  m_sRM_TextText;
    String                  m_sRM_MSAccessText;
    String                  m_sRM_LDAPText;
    String                  m_sRM_ADABASText;
    String                  m_sRM_ADOText;
    String                  m_sRM_JDBCText;
    String                  m_sRM_OracleText;
    String                  m_sRM_MySQLText;
    String                  m_sRM_ODBCText;
    String                  m_sRM_SpreadSheetText;
    String                  m_sRM_AuthentificationText;
    String                  m_sRM_FinalText;
    INetURLObject           m_aDocURL;
    String                  m_sWorkPath;
    OGeneralPage*           m_pGeneralPage;
    OMySQLIntroPageSetup*   m_pMySQLIntroPage;
    ODsnTypeCollection*     m_pCollection;  /// the DSN type collection instance



public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbTypeWizDialogSetup(Window* pParent
        ,SfxItemSet* _pItems
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        ,const ::com::sun::star::uno::Any& _aDataSourceName
        );
    virtual ~ODbTypeWizDialogSetup();

    virtual const SfxItemSet* getOutputSet() const;
    virtual SfxItemSet* getWriteOutputSet();

    // forwards to ODbDataSourceAdministrationHelper
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const;
    virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver();
    virtual DATASOURCE_TYPE     getDatasourceType(const SfxItemSet& _rSet) const;
    virtual void clearPassword();
    virtual void setTitle(const ::rtl::OUString& _sTitle);
    virtual void enableConfirmSettings( bool _bEnable );
    virtual sal_Bool saveDatasource();
    virtual String  getStateDisplayName( WizardState _nState ) const;

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
    virtual ::svt::IWizardPage* getWizardPage(TabPage* _pCurrentPage) const;
    virtual sal_Bool onFinish(sal_Int32 _nResult);

protected:
    inline sal_Bool isUIEnabled() const { return m_bUIEnabled; }
    inline void     disabledUI() { m_bUIEnabled = sal_False; }

    /// select a datasource with a given name, adjust the item set accordingly, and everything like that ..
    void implSelectDatasource(const ::rtl::OUString& _rRegisteredName);
    void resetPages(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDatasource);

    enum ApplyResult
    {
        AR_LEAVE_MODIFIED,      // somthing was modified and has successfully been committed
        AR_LEAVE_UNCHANGED,     // no changes were made
        AR_KEEP                 // don't leave the page (e.g. because an error occured)
    };

private:
    /** declares a path with or without authentication, as indicated by the database type

        @param _eType
            the data source type for which the path is declared. If this
            data source type does not support authentication, the PAGE_DBSETUPWIZARD_AUTHENTIFICATION
            state will be stripped from the sequence of states.
        @param _nPathId
            the ID of the path
        @path
            the first state in this path, following by an arbitrary number of others, as in
            RoadmapWizard::declarePath.
    */
    void declareAuthDepPath( DATASOURCE_TYPE _eType, PathId _nPathId, WizardState _nFirstState, ... );

    void RegisterDataSourceByLocation(const ::rtl::OUString& sPath);
    sal_Bool SaveDatabaseDocument();
    void activateDatabasePath();
    String createUniqueFileName(const INetURLObject& rURL);
    void CreateDatabase();
    void createUniqueFolderName(INetURLObject* pURL);
    DATASOURCE_TYPE VerifyDataSourceType(const DATASOURCE_TYPE _DatabaseType) const;

    DATASOURCE_TYPE getDefaultDatabaseType() const;

    void updateTypeDependentStates();
    sal_Bool callSaveAsDialog();
    sal_Bool IsConnectionUrlRequired();
    DECL_LINK(OnTypeSelected, OGeneralPage*);
    DECL_LINK(OnChangeCreationMode, OGeneralPage*);
    DECL_LINK(OnRecentDocumentSelected, OGeneralPage*);
    DECL_LINK(OnSingleDocumentChosen, OGeneralPage*);
    DECL_LINK(ImplClickHdl, OMySQLIntroPageSetup*);
    DECL_LINK(ImplModifiedHdl, OGenericAdministrationPage*);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_DBWIZ2_HXX

