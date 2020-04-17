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

#include "Qt5Tools.hxx"

template <typename BaseFilePicker>
class Qt5FilePickerSalAdaptor : public BaseFilePicker
{
private:
    css::uno::Reference<css::uno::XComponentContext> m_context;

public:
    template <typename Args...>
    Qt5FilePickerSalAdaptor(css::uno::Reference<css::uno::XComponentContext> const& context, Args... args)
        : BaseFilePicker(args...)
        , m_context(context)
    {
    }
    ~Qt5FilePickerSalAdaptor() override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this]() {
            // must delete it in main thread, otherwise
            // QSocketNotifier::setEnabled() will crash us
            BaseFilePicker::m_pFileDialog.reset();
        });
    }

    // XFilePickerNotifier
    void SAL_CALL addFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override
    {
        SolarMutexGuard g;
        BaseFilePicker::addFilePickerListener(xListener);
    }
    void SAL_CALL removeFilePickerListener(
        const css::uno::Reference<css::ui::dialogs::XFilePickerListener>& xListener) override
    {
        SolarMutexGuard g;
        BaseFilePicker::removeFilePickerListener(xListener);
    }

    // XFilterManager functions
    void SAL_CALL appendFilter(const OUString& rTitle, const OUString& rFilter) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread(
            [this, &rTitle, &rFilter]() { BaseFilePicker::appendFilter(rTitle, rFilter); });
    }
    void SAL_CALL setCurrentFilter(const OUString& rTitle) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread(
            [this, &rTitle]() { BaseFilePicker::setCurrentFilter(rTitle); });
    }
    OUString SAL_CALL getCurrentFilter() override
    {
        SolarMutexGuard g;
        OUString ret;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread(
            [&ret, this]() { ret = BaseFilePicker::getCurrentFilter(); });
        return ret;
    }

    // XFilterGroupManager functions
    void SAL_CALL
    appendFilterGroup(const OUString& rGroupTitle,
                      const css::uno::Sequence<css::beans::StringPair>& rFilters) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread(
            [this, &rGroupTitle, &rFilters]() { BaseFilePicker::appendFilterGroup(rGroupTitle, rFilters); });
    }

    // XExecutableDialog functions
    void SAL_CALL setTitle(const OUString& rTitle) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread(
            [this, &rTitle]() { BaseFilePicker::setTitle(rTitle); });
    }
    sal_Int16 SAL_CALL execute() override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        sal_uInt16 ret;
        pSalInst->RunInMainThread([&ret, this]() { ret = BaseFilePicker::execute(); });
        return ret;
    }

    // XFilePicker functions
    void SAL_CALL setMultiSelectionMode(sal_Bool bMode) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this, bMode]() { BaseFilePicker::setMultiSelectionMode(bMode); });
    }
    void SAL_CALL setDefaultName(const OUString& rName) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this, &rName]() { BaseFilePicker::setDefaultName(rName); });
    }
    void SAL_CALL setDisplayDirectory(const OUString& rDirectory) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this, &rDirectory]() { BaseFilePicker::setDisplayDirectory(rDirectory); });
    }
    OUString SAL_CALL getDisplayDirectory() override
    {
        SolarMutexGuard g;
        OUString ret;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread(
            [&ret, this]() { ret = BaseFilePicker::getDisplayDirectory(); });
        return ret;
    }

    // XFilePickerControlAccess functions
    void SAL_CALL setValue(sal_Int16 nControlId, sal_Int16 nControlAction,
                           const css::uno::Any& rValue) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this, nControlId, nControlAction, &rValue]() {
            BaseFilePicker::setValue(nControlId, nControlAction, rValue);
        });
    }
    css::uno::Any SAL_CALL getValue(sal_Int16 nControlId,
                                    sal_Int16 nControlAction) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        css::uno::Any ret;
        pSalInst->RunInMainThread([&ret, this, nControlId, nControlAction]() {
            ret = BaseFilePicker::getValue(nControlId, nControlAction);
        });
        return ret;
    }
    void SAL_CALL enableControl(sal_Int16 nControlId, sal_Bool bEnable) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this, nControlId, bEnable]() {
            BaseFilePicker::enableControl(nControlId, bEnable);
        });
    }
    void SAL_CALL setLabel(sal_Int16 nControlId, const OUString& rLabel) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this, nControlId, &rLabel]() {
            BaseFilePicker::setLabel(nControlId, rLabel);
        });
    }
    OUString SAL_CALL getLabel(sal_Int16 nControlId) override
    {
        SolarMutexGuard g;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        OUString ret;
        pSalInst->RunInMainThread([&ret, this, nControlId]() {
            ret = BaseFilePicker::getLabel(nControlId);
        });
        return ret;
    }

    // XFilePicker2 functions
    css::uno::Sequence<OUString> SAL_CALL getSelectedFiles() override
    {
        SolarMutexGuard g;
        css::uno::Sequence<OUString> ret;
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([&ret, this]() { ret = BaseFilePicker::getSelectedFiles(); });

        auto const trans = css::uri::ExternalUriReferenceTranslator::create(m_context);
        size_t i = 0;
        for (const OUString& extURL : ret)
        {
            // Unlike LO, QFileDialog (<https://doc.qt.io/qt-5/qfiledialog.html>) apparently always
            // treats file-system pathnames as UTF-8--encoded, regardless of LANG/LC_CTYPE locale
            // setting.  And pathnames containing byte sequences that are not valid UTF-8 are apparently
            // filtered out and not even displayed by QFileDialog, so aURL will always have a "payload"
            // that matches the pathname's byte sequence.  So the pathname's byte sequence (which
            // happens to also be aURL's payload) in the LANG/LC_CTYPE encoding needs to be converted
            // into LO's internal UTF-8 file URL encoding via
            // XExternalUriReferenceTranslator::translateToInternal (which looks somewhat paradoxical as
            // extURL nominally already has a UTF-8 payload):
            auto intUrl = trans->translateToInternal(extUrl);
            if (intUrl.isEmpty())
            {
                // If translation failed, fall back to original URL:
                SAL_WARN("vcl.qt5", "cannot convert <" << extUrl << "> from locale encoding to UTF-8");
            }
            else
                seq[i] = intUrl;

            i++;
        }

        return ret;
    }

    QString getResString(const char* pResId) override
    {
        QString aResString;

        if (pResId == nullptr)
            return aResString;

        aResString = toQString(VclResId(pResId));

        return aResString.replace('~', '&');
}

    int execFileDialog(QWidget *pTransientParent) override
    {
        if (!pTransientParent)
        {
            vcl::Window* pWindow = ::Application::GetActiveTopWindow();
            if (pWindow)
            {
                Qt5Frame* pFrame = dynamic_cast<Qt5Frame*>(pWindow->ImplGetFrame());
                assert(pFrame);
                if (pFrame)
                    pTransientParent = pFrame->asChild();
            }
        }

        uno::Reference<css::frame::XDesktop> xDesktop(css::frame::Desktop::create(m_context),
                                                      UNO_QUERY_THROW);

        // will hide the window, so do before show
        m_pFileDialog->setParent(pTransientParent, m_pFileDialog->windowFlags());
        m_pFileDialog->show();
        xDesktop->addTerminateListener(this);
        int result = m_pFileDialog->exec();
        xDesktop->removeTerminateListener(this);
        m_pFileDialog->setParent(nullptr, m_pFileDialog->windowFlags());
        return result;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
