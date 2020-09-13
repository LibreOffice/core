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

#include <dsntypes.hxx>
#include "IItemSetHelper.hxx"
#include <vcl/wizardmachine.hxx>
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

using vcl::WizardTypes::WizardState;
using vcl::WizardTypes::CommitPageReason;

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
class ODbTypeWizDialog : public vcl::WizardMachine , public IItemSetHelper, public IDatabaseSettingsDialog
{
private:
    std::unique_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
    std::unique_ptr<SfxItemSet> m_pOutSet;
    ::dbaccess::ODsnTypeCollection*
                            m_pCollection;  /// the DSN type collection instance
    OUString                m_eType;

public:
    /** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
        after the dialog has been destroyed
    */
    ODbTypeWizDialog(weld::Window* pParent
        ,SfxItemSet const * _pItems
        ,const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        ,const css::uno::Any& _aDataSourceName
        );
    virtual ~ODbTypeWizDialog() override;

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

protected:
    /// to override to create new pages
    virtual std::unique_ptr<BuilderPage> createPage(WizardState _nState) override;
    virtual WizardState determineNextState(WizardState _nCurrentState) const override;
    virtual bool        leaveState(WizardState _nState) override;
    virtual ::vcl::IWizardPageController* getPageController(BuilderPage* pCurrentPage) const override;
    virtual bool        onFinish() override;

private:
    DECL_LINK(OnTypeSelected, OGeneralPage&, void);
};

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
