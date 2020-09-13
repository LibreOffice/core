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

#pragma once

#include <sfx2/tabdlg.hxx>
#include <dsntypes.hxx>
#include "IItemSetHelper.hxx"
#include <memory>

namespace com::sun::star {
    namespace beans {
        class XPropertySet;
    }
    namespace sdbc {
        class XConnection;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
}

namespace dbaui
{

// ODbAdminDialog
class ODbDataSourceAdministrationHelper;
/** tab dialog for administrating the office wide registered data sources
*/
class ODbAdminDialog final : public SfxTabDialogController, public IItemSetHelper, public IDatabaseSettingsDialog
{
private:
    std::unique_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;

    OString             m_sMainPageID;

public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbAdminDialog(weld::Window* pParent, SfxItemSet const * _pItems,
                   const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
    virtual ~ODbAdminDialog() override;

    /** create and return an item set for use with the dialog.
        @param      _pTypeCollection        pointer to an <type>ODatasourceMap</type>. May be NULL, in this case
                                            the pool will not contain a typecollection default.
    */
    static void createItemSet(std::unique_ptr<SfxItemSet>& _rpSet, SfxItemPool*& _rpPool, std::vector<SfxPoolItem*>*& _rpDefaults, ::dbaccess::ODsnTypeCollection* _pTypeCollection);
    /** destroy and item set / item pool / pool defaults previously created by <method>createItemSet</method>
    */
    static void destroyItemSet(std::unique_ptr<SfxItemSet>& _rpSet, SfxItemPool*& _rpPool, std::vector<SfxPoolItem*>*& _rpDefaults);

    /** selects the DataSource
        @param  _rName
            The name of the data source
    */
    void selectDataSource(const css::uno::Any& _aDataSourceName);

    virtual const SfxItemSet* getOutputSet() const override;
    virtual SfxItemSet* getWriteOutputSet() override;

    // forwards to ODbDataSourceAdministrationHelper
    virtual css::uno::Reference< css::uno::XComponentContext > getORB() const override;
    virtual std::pair< css::uno::Reference< css::sdbc::XConnection >,bool> createConnection() override;
    virtual css::uno::Reference< css::sdbc::XDriver > getDriver() override;
    virtual OUString getDatasourceType(const SfxItemSet& _rSet) const override;
    virtual void clearPassword() override;
    virtual void saveDatasource() override;
    virtual void setTitle(const OUString& _sTitle) override;
    virtual void enableConfirmSettings( bool _bEnable ) override;

private:
    // adds a new detail page and remove all the old ones
    void addDetailPage(const OString& rPageId, const char* pTextId, CreateTabPage pCreateFunc);

    virtual void PageCreated(const OString& rId, SfxTabPage& _rPage) override;
    virtual short Ok() override;

    /// select a datasource with a given name, adjust the item set accordingly, and everything like that ..
    void impl_selectDataSource(const css::uno::Any& _aDataSourceName);
    /// reset the tag pages according to m_sCurrentDatasource and <arg>_rxDatasource</arg>
    void impl_resetPages(const css::uno::Reference< css::beans::XPropertySet >& _rxDatasource);

    enum ApplyResult
    {
        AR_LEAVE_MODIFIED,      // something was modified and has successfully been committed
        AR_KEEP                 // don't leave the page (e.g. because an error occurred)
    };
    /** apply all changes made
    */
    ApplyResult implApplyChanges();
};

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
