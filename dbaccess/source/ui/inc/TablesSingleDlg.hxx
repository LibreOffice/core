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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLESSINGLEDLG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLESSINGLEDLG_HXX

#include <sfx2/basedlgs.hxx>
#include "IItemSetHelper.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>

#include <memory>

namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertySet;
    }
    namespace uno {
        class XComponentContext;
    }
}}}

namespace dbaui
{
class ODbDataSourceAdministrationHelper;
    // OTableSubscriptionDialog
    class OTableSubscriptionDialog : public SfxSingleTabDialog, public IItemSetHelper
    {
        std::unique_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
        bool m_bStopExecution; // set when the dialog should not be executed

        std::unique_ptr<SfxItemSet> m_pOutSet;
    public:

        OTableSubscriptionDialog(vcl::Window* pParent
            ,const SfxItemSet* _pItems
            ,const css::uno::Reference< css::uno::XComponentContext >& _rxORB
            ,const css::uno::Any& _aDataSourceName
        );
        virtual ~OTableSubscriptionDialog() override;
        virtual void dispose() override;

        // forwards from ODbDataSourceAdministrationHelper
        void        successfullyConnected();
        bool        getCurrentSettings(css::uno::Sequence< css::beans::PropertyValue >& _rDriverParams);
        void        clearPassword();
        OUString    getConnectionURL() const;
        css::uno::Reference< css::beans::XPropertySet > const & getCurrentDataSource();
        void endExecution() { m_bStopExecution = true; }

        virtual const SfxItemSet* getOutputSet() const override;
        virtual SfxItemSet* getWriteOutputSet() override;

        virtual short   Execute() override;
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLESSINGLEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
