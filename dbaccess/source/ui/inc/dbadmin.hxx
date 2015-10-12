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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DBADMIN_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DBADMIN_HXX

#include <sfx2/tabdlg.hxx>
#include "dsntypes.hxx"
#include "IItemSetHelper.hxx"
#include <comphelper/uno3.hxx>
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

namespace dbaui
{

// ODbAdminDialog
class ODbDataSourceAdministrationHelper;
/** tab dialog for administrating the office wide registered data sources
*/
class ODbAdminDialog : public SfxTabDialog , public IItemSetHelper, public IDatabaseSettingsDialog
{
private:
    typedef ::std::stack< sal_Int32 > PageStack;
    PageStack               m_aCurrentDetailPages;  // ids of all currently enabled (type-dependent) detail pages

    ::std::unique_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;

    bool                m_bApplied : 1;     /// sal_True if any changes have been applied while the dialog was executing
    bool                m_bUIEnabled : 1;   /// <TRUE/> if the UI is enabled, false otherwise. Cannot be switched back to <TRUE/>, once it is <FALSE/>
    sal_uInt16          m_nMainPageID;

public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbAdminDialog(vcl::Window* pParent,
        SfxItemSet* _pItems,
        const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );
    virtual ~ODbAdminDialog();
    virtual void dispose() override;

    /** create and return an item set for use with the dialog.
        @param      _pTypeCollection        pointer to an <type>ODatasourceMap</type>. May be NULL, in this case
                                            the pool will not contain a typecollection default.
    */
    static SfxItemSet*  createItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults, ::dbaccess::ODsnTypeCollection* _pTypeCollection);
    /** destroy and item set / item pool / pool defaults previously created by <method>createItemSet</method>
    */
    static void         destroyItemSet(SfxItemSet*& _rpSet, SfxItemPool*& _rpPool, SfxPoolItem**& _rppDefaults);

    /** selects the DataSource
        @param  _rName
            The name of the data source
    */
    void selectDataSource(const css::uno::Any& _aDataSourceName);

    virtual const SfxItemSet* getOutputSet() const override;
    virtual SfxItemSet* getWriteOutputSet() override;

    // forwards to ODbDataSourceAdministrationHelper
    virtual css::uno::Reference< css::uno::XComponentContext > getORB() const override;
    virtual ::std::pair< css::uno::Reference< css::sdbc::XConnection >,sal_Bool> createConnection() override;
    virtual css::uno::Reference< css::sdbc::XDriver > getDriver() override;
    virtual OUString getDatasourceType(const SfxItemSet& _rSet) const override;
    virtual void clearPassword() override;
    virtual bool saveDatasource() override;
    virtual void setTitle(const OUString& _sTitle) override;
    virtual void enableConfirmSettings( bool _bEnable ) override;

protected:
    // adds a new detail page and remove all the old ones
    void addDetailPage(sal_uInt16 _nPageId,sal_uInt16 _nTextId,CreateTabPage pCreateFunc);

    virtual void PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage) override;
    virtual short Ok() override;

protected:
    inline bool isUIEnabled() const { return m_bUIEnabled; }
    inline void     disabledUI() { m_bUIEnabled = false; }

private:
    /// select a datasource with a given name, adjust the item set accordingly, and everything like that ..
    void impl_selectDataSource(const css::uno::Any& _aDataSourceName);
    /// reset the tag pages according to m_sCurrentDatasource and <arg>_rxDatasource</arg>
    void impl_resetPages(const css::uno::Reference< css::beans::XPropertySet >& _rxDatasource);

    enum ApplyResult
    {
        AR_LEAVE_MODIFIED,      // something was modified and has successfully been committed
        AR_LEAVE_UNCHANGED,     // no changes were made
        AR_KEEP                 // don't leave the page (e.g. because an error occurred)
    };
    /** apply all changes made
    */
    ApplyResult implApplyChanges();
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBADMIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
