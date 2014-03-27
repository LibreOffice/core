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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DBWIZ_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DBWIZ_HXX

#include <sfx2/tabdlg.hxx>
#include "dsntypes.hxx"
#include "IItemSetHelper.hxx"
#include <comphelper/uno3.hxx>
#include <svtools/wizardmachine.hxx>
#include "moduledbu.hxx"
#include <memory>

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

namespace dbaccess
{
    class ODsnTypeCollection;
}
namespace dbaui
{

// ODbTypeWizDialog
class OGeneralPage;
class ODbDataSourceAdministrationHelper;
/** tab dialog for administrating the office wide registered data sources
*/
class ODbTypeWizDialog : public svt::OWizardMachine , public IItemSetHelper, public IDatabaseSettingsDialog,public dbaui::OModuleClient
{
private:
    OModuleClient m_aModuleClient;
    ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
    SfxItemSet*             m_pOutSet;
    ::dbaccess::ODsnTypeCollection*
                            m_pCollection;  /// the DSN type collection instance
    OUString         m_eType;

    sal_Bool                m_bResetting : 1;   /// sal_True while we're resetting the pages
    sal_Bool                m_bApplied : 1;     /// sal_True if any changes have been applied while the dialog was executing
    sal_Bool                m_bUIEnabled : 1;   /// <TRUE/> if the UI is enabled, false otherwise. Cannot be switched back to <TRUE/>, once it is <FALSE/>

public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbTypeWizDialog(Window* pParent
        ,SfxItemSet* _pItems
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        ,const ::com::sun::star::uno::Any& _aDataSourceName
        );
    virtual ~ODbTypeWizDialog();

    virtual const SfxItemSet* getOutputSet() const SAL_OVERRIDE;
    virtual SfxItemSet* getWriteOutputSet() SAL_OVERRIDE;

    // forwards to ODbDataSourceAdministrationHelper
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getORB() const SAL_OVERRIDE;
    virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver() SAL_OVERRIDE;
    virtual OUString getDatasourceType(const SfxItemSet& _rSet) const SAL_OVERRIDE;
    virtual void clearPassword() SAL_OVERRIDE;
    virtual sal_Bool saveDatasource() SAL_OVERRIDE;
    virtual void setTitle(const OUString& _sTitle) SAL_OVERRIDE;
    virtual void enableConfirmSettings( bool _bEnable ) SAL_OVERRIDE;

protected:
    /// to override to create new pages
    virtual TabPage*    createPage(WizardState _nState) SAL_OVERRIDE;
    virtual WizardState determineNextState(WizardState _nCurrentState) const SAL_OVERRIDE;
    virtual bool        leaveState(WizardState _nState) SAL_OVERRIDE;
    virtual ::svt::IWizardPageController*
                        getPageController( TabPage* _pCurrentPage ) const SAL_OVERRIDE;
    virtual bool        onFinish() SAL_OVERRIDE;

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
    /** apply all changes made
    */
    ApplyResult implApplyChanges();

private:
    DECL_LINK(OnTypeSelected, OGeneralPage*);
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBWIZ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
