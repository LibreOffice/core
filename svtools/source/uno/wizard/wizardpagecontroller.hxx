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

#include <vcl/wizardmachine.hxx>
#include <com/sun/star/ui/dialogs/XWizardController.hpp>

namespace svt::uno
{


    //= WizardPageController

    class WizardPageController : public vcl::IWizardPageController
    {
    public:
        WizardPageController(
            weld::Container* pParent,
            const css::uno::Reference< css::ui::dialogs::XWizardController >& i_rController,
            const sal_Int16 i_nPageId
        );
        virtual ~WizardPageController();

        // IWizardPageController overridables
        virtual void        initializePage() override;
        virtual bool        commitPage( vcl::WizardTypes::CommitPageReason _eReason ) override;
        virtual bool        canAdvance() const override;

        const css::uno::Reference< css::ui::dialogs::XWizardPage >&
                            getWizardPage() const { return m_xWizardPage; }

    private:
        const css::uno::Reference< css::ui::dialogs::XWizardController >  m_xController;
        css::uno::Reference< css::ui::dialogs::XWizardPage >              m_xWizardPage;
    };


} // namespace svt::uno



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
