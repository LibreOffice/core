/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceAssistant.hxx>
#include <QtInstanceAssistant.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceAssistant::QtInstanceAssistant(QWizard* pWizard)
    : QtInstanceDialog(pWizard)
    , m_pWizard(pWizard)
{
    assert(pWizard);
}

int QtInstanceAssistant::get_current_page() const
{
    SolarMutexGuard g;

    int nPage = 0;
    GetQtInstance().RunInMainThread([&] { nPage = m_pWizard->currentId(); });

    return nPage;
}

int QtInstanceAssistant::get_n_pages() const
{
    SolarMutexGuard g;

    int nPageCount = 0;
    GetQtInstance().RunInMainThread([&] { nPageCount = m_pWizard->pageIds().size(); });

    return nPageCount;
}

OUString QtInstanceAssistant::get_page_ident(int nPage) const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        if (QWizardPage* pPage = m_pWizard->page(nPage))
            sId = toOUString(pPage->objectName());
    });

    return sId;
}

OUString QtInstanceAssistant::get_current_page_ident() const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        if (QWizardPage* pPage = m_pWizard->currentPage())
            sId = toOUString(pPage->objectName());
    });

    return sId;
}

void QtInstanceAssistant::set_current_page(int nPage)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
        m_pWizard->setCurrentId(nPage);
#else
        int nCurrentPage = m_pWizard->currentId();
        if (nPage > nCurrentPage)
        {
            for (int i = 0; i < nPage - nCurrentPage; ++i)
                m_pWizard->next();
        }
        else if (nPage < nCurrentPage)
        {
            for (int i = 0; i < nPage - nCurrentPage; ++i)
                m_pWizard->back();
        }
#endif
    });
}

void QtInstanceAssistant::set_current_page(const OUString& rIdent)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const QList<int> aPageIds = m_pWizard->pageIds();
        for (int nPage : aPageIds)
        {
            QWizardPage* pPage = m_pWizard->page(nPage);
            if (pPage && pPage->objectName() == toQString(rIdent))
            {
                set_current_page(nPage);
                break;
            }
        }
    });
}

void QtInstanceAssistant::set_page_index(const OUString&, int)
{
    assert(false && "not implemented yet");
}

void QtInstanceAssistant::set_page_title(const OUString& rIdent, const OUString& rTitle)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QWizardPage* pPage = page(rIdent))
            pPage->setTitle(toQString(rTitle));
    });
}

OUString QtInstanceAssistant::get_page_title(const OUString& rIdent) const
{
    SolarMutexGuard g;

    OUString sTitle;
    GetQtInstance().RunInMainThread([&] {
        if (QWizardPage* pPage = page(rIdent))
            sTitle = toOUString(pPage->title());
    });

    return sTitle;
}

void QtInstanceAssistant::set_page_sensitive(const OUString& rIdent, bool bSensitive)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QWizardPage* pPage = page(rIdent))
            pPage->setEnabled(bSensitive);
    });
}

weld::Container* QtInstanceAssistant::append_page(const OUString& rIdent)
{
    SolarMutexGuard g;

    weld::Container* pContainer = nullptr;

    GetQtInstance().RunInMainThread([&] {
        QWizardPage* pNewPage = new QWizardPage;
        pNewPage->setObjectName(toQString(rIdent));
        // ensure that QWizard page ID matches page index
        const int nPageId = m_pWizard->pageIds().size();
        m_pWizard->setPage(nPageId, pNewPage);

        m_aPages.emplace_back(new QtInstanceContainer(pNewPage));
        pContainer = m_aPages.back().get();
    });

    return pContainer;
}

void QtInstanceAssistant::set_page_side_help_id(const OUString&)
{
    assert(false && "not implemented yet");
}

void QtInstanceAssistant::set_page_side_image(const OUString&)
{
    assert(false && "not implemented yet");
}

QWizardPage* QtInstanceAssistant::page(const OUString& rIdent) const
{
    const QList<int> aPageIds = m_pWizard->pageIds();
    for (int nPage : aPageIds)
    {
        QWizardPage* pPage = m_pWizard->page(nPage);
        if (pPage && pPage->objectName() == toQString(rIdent))
            return pPage;
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
