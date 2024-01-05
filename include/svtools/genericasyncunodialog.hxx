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

#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <svtools/genericunodialog.hxx>
#include <vcl/svapp.hxx>

using namespace css::uno;

namespace svt
{
typedef cppu::ImplInheritanceHelper<::svt::OGenericUnoDialog,
                                    css::ui::dialogs::XAsynchronousExecutableDialog>
    OGenericUnoAsyncDialogBase;

/** abstract base class for implementing UNO objects representing asynchronous dialogs

    Contrary to StartExecuteAsync in VclAbstractDialog from include/vcl/abstdlg.hxx,
    the different methods are used in a special case when an import or export action
    wants to show a dialog, as part of a synchronous filter() API call.

    In this case it's not possible to move the "rest of the code" to an async
    callback, so that needs special handling. Luckily these dialogs are rather rare.
    */
template <typename T> class OGenericUnoAsyncDialog : public OGenericUnoAsyncDialogBase
{
    class UnoAsyncDialogEntryGuard
    {
    public:
        UnoAsyncDialogEntryGuard(OGenericUnoAsyncDialog<T>& _rDialog)
            : m_aGuard(_rDialog.GetMutex())
        {
        }

    private:
        ::osl::MutexGuard m_aGuard;
    };

protected:
    std::shared_ptr<T> m_xAsyncDialog;

protected:
    OGenericUnoAsyncDialog(const css::uno::Reference<css::uno::XComponentContext>& _rxContext)
        : OGenericUnoAsyncDialogBase(_rxContext)
    {
    }

public:
    // XAsynchronousExecutableDialog
    void SAL_CALL setDialogTitle(const OUString& aTitle) override
    {
        OGenericUnoDialog::setTitle(aTitle);
    }

    virtual void SAL_CALL startExecuteModal(
        const css::uno::Reference<css::ui::dialogs::XDialogClosedListener>& xListener) override
    {
        SolarMutexGuard aSolarGuard;

        {
            UnoAsyncDialogEntryGuard aGuard(*this);

            if (m_bExecuting)
                throw RuntimeException("already executing the dialog (recursive call)", *this);

            if (!m_xAsyncDialog)
            {
                m_xAsyncDialog = createAsyncDialog(m_xParent);
                OSL_ENSURE(m_xAsyncDialog, "OGenericUnoAsyncDialog::startExecuteModal: "
                                           "createAsyncDialog returned nonsense!");
                if (!m_xAsyncDialog)
                    return;

                // do some initialisations
                if (!m_bTitleAmbiguous)
                    m_xAsyncDialog->set_title(m_sTitle);
            }

            m_bExecuting = true;
        }

        runAsync(xListener);
    }

protected:
    virtual std::shared_ptr<T>
    createAsyncDialog(const css::uno::Reference<css::awt::XWindow>& /*rParent*/)
    {
        return nullptr;
    }

    void destroyAsyncDialog()
    {
        SolarMutexGuard aSolarGuard;
        if (m_xAsyncDialog)
            m_xAsyncDialog.reset();
    }

    virtual void
    runAsync(const css::uno::Reference<css::ui::dialogs::XDialogClosedListener>& /*xListener*/)
    {
    }

    virtual void executedAsyncDialog(std::shared_ptr<T> /*xAsyncDialog*/,
                                     sal_Int32 /*_nExecutionResult*/)
    {
    }
};

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
