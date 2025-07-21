/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceNotebook.hxx>
#include <QtInstanceNotebook.moc>

#include <vcl/qt/QtUtils.hxx>

#include <QtWidgets/QTabBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

// property on each tab's widget holding the accessible identifier of that tab
const char* const PROPERTY_TAB_PAGE_ID = "tab-page-id";

QtInstanceNotebook::QtInstanceNotebook(QTabWidget* pTabWidget)
    : QtInstanceWidget(pTabWidget)
    , m_pTabWidget(pTabWidget)
{
    assert(m_pTabWidget);

    if (m_pTabWidget->count())
        m_sCurrentTabId = get_current_page_ident();

    connect(m_pTabWidget, &QTabWidget::currentChanged, this,
            &QtInstanceNotebook::currentTabChanged);
}

int QtInstanceNotebook::get_current_page() const
{
    SolarMutexGuard g;
    int nCurrentIndex = 0;
    GetQtInstance().RunInMainThread([&] { nCurrentIndex = m_pTabWidget->currentIndex(); });
    return nCurrentIndex;
}

int QtInstanceNotebook::get_page_index(const OUString& rIdent) const
{
    SolarMutexGuard g;

    const QString sId = toQString(rIdent);
    int nIndex = -1;
    GetQtInstance().RunInMainThread([&] {
        for (int i = 0; i < m_pTabWidget->count(); ++i)
        {
            if (get_page_ident(i) == rIdent)
            {
                nIndex = i;
                return;
            }
        }
    });
    return nIndex;
}

OUString QtInstanceNotebook::get_page_ident(int nPage) const
{
    SolarMutexGuard g;

    OUString sIdent;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pPage = m_pTabWidget->widget(nPage);
        assert(pPage);
        QVariant aIdVariant = pPage->property(PROPERTY_TAB_PAGE_ID);
        if (aIdVariant.canConvert<QString>())
            sIdent = toOUString(aIdVariant.toString());
    });
    return sIdent;
}

OUString QtInstanceNotebook::get_current_page_ident() const
{
    SolarMutexGuard g;

    OUString sIdent;
    GetQtInstance().RunInMainThread([&] {
        const int nIndex = m_pTabWidget->currentIndex();
        if (nIndex >= 0)
            sIdent = get_page_ident(nIndex);
    });
    return sIdent;
}

void QtInstanceNotebook::set_current_page(int nPage)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTabWidget->setCurrentIndex(nPage); });
}

void QtInstanceNotebook::set_current_page(const OUString& rIdent)
{
    set_current_page(get_page_index(rIdent));
}

void QtInstanceNotebook::remove_page(const OUString& rIdent)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTabWidget->removeTab(get_page_index(rIdent)); });
}

void QtInstanceNotebook::insert_page(const OUString& rIdent, const OUString& rLabel, int nPos,
                                     const OUString* /* pIconName */)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pPage = new QWidget;
        pPage->setLayout(new QVBoxLayout);
        pPage->setProperty(PROPERTY_TAB_PAGE_ID, toQString(rIdent));
        m_pTabWidget->insertTab(nPos, pPage, toQString(rLabel));
    });
}

void QtInstanceNotebook::set_tab_label_text(const OUString& rIdent, const OUString& rLabel)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        const int nIndex = get_page_index(rIdent);
        m_pTabWidget->setTabText(nIndex, toQString(rLabel));
    });
}

OUString QtInstanceNotebook::get_tab_label_text(const OUString& rIdent) const
{
    SolarMutexGuard g;
    OUString sText;
    GetQtInstance().RunInMainThread([&] {
        const int nIndex = get_page_index(rIdent);
        sText = toOUString(m_pTabWidget->tabText(nIndex));
    });
    return sText;
}

void QtInstanceNotebook::set_show_tabs(bool bShow)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTabWidget->tabBar()->setVisible(bShow); });
}

int QtInstanceNotebook::get_n_pages() const
{
    SolarMutexGuard g;
    int nCount = 0;
    GetQtInstance().RunInMainThread([&] { nCount = m_pTabWidget->count(); });
    return nCount;
}

weld::Container* QtInstanceNotebook::get_page(const OUString& rIdent) const
{
    SolarMutexGuard g;

    QWidget* pWidget = nullptr;
    GetQtInstance().RunInMainThread([&] {
        const int nIndex = get_page_index(rIdent);
        pWidget = m_pTabWidget->widget(nIndex);
    });

    if (!pWidget)
        return nullptr;

    if (!m_aPageContainerInstances.contains(pWidget))
        m_aPageContainerInstances.emplace(pWidget, std::make_unique<QtInstanceContainer>(pWidget));

    return m_aPageContainerInstances.at(pWidget).get();
}

void QtInstanceNotebook::setTabIdAndLabel(QTabWidget& rTabWidget, int nPage, const OUString& rIdent,
                                          const OUString& rLabel)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pPage = rTabWidget.widget(nPage);
        assert(pPage);
        pPage->setProperty(PROPERTY_TAB_PAGE_ID, toQString(rIdent));
        rTabWidget.setTabText(nPage, toQString(rLabel));
    });
}

void QtInstanceNotebook::currentTabChanged()
{
    SolarMutexGuard g;

    if (!m_sCurrentTabId.isEmpty())
        m_aLeavePageHdl.Call(m_sCurrentTabId);

    m_sCurrentTabId = get_current_page_ident();

    if (!m_sCurrentTabId.isEmpty())
        m_aEnterPageHdl.Call(m_sCurrentTabId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
