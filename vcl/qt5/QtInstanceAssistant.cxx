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

// Name of QObject property set on QWizardPage objects for the page
// index as used in the weld::Assistant API.
// This is different from the page id as used in the QWizard API
const char* const PROPERTY_PAGE_INDEX = "page-index";

QtInstanceAssistant::QtInstanceAssistant(QWizard* pWizard)
    : QtInstanceDialog(pWizard)
    , m_pWizard(pWizard)
{
    assert(pWizard);
}

int QtInstanceAssistant::get_current_page() const
{
    SolarMutexGuard g;

    int nPageIndex = -1;
    GetQtInstance().RunInMainThread([&] {
        if (QWizardPage* pPage = m_pWizard->page(m_pWizard->currentId()))
            nPageIndex = pageIndex(*pPage);
    });

    return nPageIndex;
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
        if (QWizardPage* pPage = page(nPage))
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
        const QList<int> aPageIds = m_pWizard->pageIds();
        for (int nId : aPageIds)
        {
            QWizardPage* pPage = m_pWizard->page(nId);
            assert(pPage);
            if (pageIndex(*pPage) == nPage)
            {
                m_pWizard->setCurrentId(nId);
                return;
            }
        }
#else
        // QWizard::setCurrentId only available from 6.4 on
        // start with first page and advance until the expected one is the current one
        m_pWizard->restart();
        int nCurrentId = m_pWizard->currentId();
        while (nCurrentId != -1 && pageIndex(*m_pWizard->page(nCurrentId)) != nPage)
            m_pWizard->next();
#endif
    });
}

void QtInstanceAssistant::set_current_page(const OUString& rIdent)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const QList<int> aPageIds = m_pWizard->pageIds();
        for (int nPageId : aPageIds)
        {
            QWizardPage* pPage = m_pWizard->page(nPageId);
            if (pPage && pPage->objectName() == toQString(rIdent))
            {
                set_current_page(nPageId);
                break;
            }
        }
    });
}

void QtInstanceAssistant::set_page_index(const OUString& rIdent, int nIndex)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const QString sIdent = toQString(rIdent);

        // QWizard page IDs are different from weld::Assistant page indices
        // use a vector where items will be sorted by page index for help
        QList<QWizardPage*> aPages;
        int nOldIndex = -1;
        const QList<int> aPageIds = m_pWizard->pageIds();
        for (int nPageId : aPageIds)
        {
            QWizardPage* pPage = m_pWizard->page(nPageId);
            assert(pPage);
            aPages.push_back(pPage);
            if (pPage->objectName() == sIdent)
                nOldIndex = pageIndex(*pPage);
        }

        assert(nOldIndex >= 0 && "no page with the given identifier");

        // sort vector by page index
        std::sort(aPages.begin(), aPages.end(), [](QWizardPage* pFirst, QWizardPage* pSecond) {
            return pageIndex(*pFirst) < pageIndex(*pSecond);
        });
        // remove and reinsert the page at new position
        QWizardPage* pPage = aPages.takeAt(nOldIndex);
        aPages.insert(nIndex, pPage);

        // update index property for all pages
        for (qsizetype i = 0; i < aPages.size(); ++i)
            setPageIndex(*aPages.at(i), i);
    });
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

        const int nPageIndex = m_pWizard->pageIds().size();
        setPageIndex(*pNewPage, nPageIndex);

        m_pWizard->addPage(pNewPage);

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

QWizardPage* QtInstanceAssistant::page(int nPageIndex) const
{
    const QList<int> aPageIds = m_pWizard->pageIds();
    for (int nId : aPageIds)
    {
        QWizardPage* pPage = m_pWizard->page(nId);
        if (pPage && pageIndex(*pPage) == nPageIndex)
            return pPage;
    }

    return nullptr;
}

int QtInstanceAssistant::pageIndex(const QWizardPage& rPage)
{
    const QVariant aPageProperty = rPage.property(PROPERTY_PAGE_INDEX);
    assert(aPageProperty.isValid() && aPageProperty.canConvert<int>());
    return aPageProperty.toInt();
}

void QtInstanceAssistant::setPageIndex(QWizardPage& rPage, int nIndex)
{
    rPage.setProperty(PROPERTY_PAGE_INDEX, nIndex);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
