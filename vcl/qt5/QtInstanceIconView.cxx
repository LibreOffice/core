/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceIconView.hxx>
#include <QtInstanceIconView.moc>

#include <QtInstanceTreeIter.hxx>

#include <vcl/qt/QtUtils.hxx>

#include <QtGui/QHelpEvent>
#include <QtWidgets/QToolTip>

QtInstanceIconView::QtInstanceIconView(QListView* pListView)
    : QtInstanceItemView(pListView, *pListView->model())
    , m_pListView(pListView)
{
    assert(m_pListView);

    m_pModel = qobject_cast<QStandardItemModel*>(m_pListView->model());
    assert(m_pModel && "list view doesn't have expected item model set");

    m_pSelectionModel = m_pListView->selectionModel();
    assert(m_pSelectionModel);

    connect(m_pListView, &QListView::activated, this, &QtInstanceIconView::handleActivated);
    connect(m_pSelectionModel, &QItemSelectionModel::selectionChanged, this,
            &QtInstanceIconView::handleSelectionChanged);
}

int QtInstanceIconView::get_item_width() const
{
    assert(false && "Not implemented yet");
    return 0;
}

void QtInstanceIconView::set_item_width(int) { assert(false && "Not implemented yet"); }

void QtInstanceIconView::do_insert(int nPos, const OUString* pStr, const OUString* pId,
                                   const QPixmap* pIcon, weld::TreeIter* pRet)
{
    assert(!pRet && "Support for pRet param not implemented yet");
    (void)pRet;

    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (nPos == -1)
            nPos = m_pModel->rowCount();

        QStandardItem* pItem = new QStandardItem;
        if (pStr)
            pItem->setText(toQString(*pStr));
        if (pId)
            pItem->setData(toQString(*pId), ROLE_ID);
        if (pIcon)
        {
            pItem->setIcon(QIcon(*pIcon));
            // set list view icon size to avoid downscaling
            const QSize aIconSize = m_pListView->iconSize().expandedTo(pIcon->size());
            m_pListView->setIconSize(aIconSize);
        }

        m_pModel->insertRow(nPos, pItem);
    });
}

void QtInstanceIconView::do_insert(int nPos, const OUString* pStr, const OUString* pId,
                                   const OUString* pIconName, weld::TreeIter* pRet)
{
    std::optional<QPixmap> oPixmap;
    if (pIconName)
        oPixmap = loadQPixmapIcon(*pIconName);

    const QPixmap* pPixmapIcon = oPixmap.has_value() ? &oPixmap.value() : nullptr;
    do_insert(nPos, pStr, pId, pPixmapIcon, pRet);
}

void QtInstanceIconView::do_insert(int nPos, const OUString* pStr, const OUString* pId,
                                   const Bitmap* pIcon, weld::TreeIter* pRet)
{
    std::optional<QPixmap> oPixmap;
    if (pIcon)
        oPixmap = toQPixmap(*pIcon);

    const QPixmap* pPixmapIcon = oPixmap.has_value() ? &oPixmap.value() : nullptr;
    do_insert(nPos, pStr, pId, pPixmapIcon, pRet);
}

void QtInstanceIconView::insert_separator(int, const OUString*)
{
    assert(false && "Not implemented yet");
}

int QtInstanceIconView::count_selected_items() const
{
    SolarMutexGuard g;

    int nSelected = 0;
    GetQtInstance().RunInMainThread([&] { nSelected = m_pSelectionModel->selectedRows().count(); });

    return nSelected;
}

void QtInstanceIconView::set_image(int nPos, VirtualDevice& rDevice)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = modelIndex(nPos);
        QIcon aIcon = toQPixmap(rDevice);
        m_pModel->setData(aIndex, aIcon, Qt::DecorationRole);
    });
}

void QtInstanceIconView::set_text(int nPos, const OUString& rText)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = modelIndex(nPos);
        m_pModel->setData(aIndex, toQString(rText));
    });
}

void QtInstanceIconView::set_item_accessible_name(int nPos, const OUString& rName)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = modelIndex(nPos);
        m_pModel->setData(aIndex, toQString(rName), Qt::AccessibleTextRole);
    });
}

void QtInstanceIconView::set_item_tooltip_text(int nPos, const OUString& rToolTip)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = modelIndex(nPos);
        m_pModel->setData(aIndex, toQString(rToolTip), Qt::ToolTipRole);
    });
}

void QtInstanceIconView::do_remove(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

tools::Rectangle QtInstanceIconView::get_rect(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return tools::Rectangle();
}

OUString QtInstanceIconView::get_text(const weld::TreeIter& rIter) const
{
    SolarMutexGuard g;

    OUString sText;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aIndex = modelIndex(rIter);
        const QVariant aData = m_pModel->data(aIndex);
        assert(aData.canConvert<QString>() && "model data not a string");
        sText = toOUString(aData.toString());
    });

    return sText;
}

void QtInstanceIconView::do_scroll_to_item(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pListView->scrollTo(modelIndex(rIter)); });
}

void QtInstanceIconView::selected_foreach(const std::function<bool(weld::TreeIter&)>&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceIconView::n_children() const
{
    SolarMutexGuard g;

    int nChildren = 0;
    GetQtInstance().RunInMainThread([&] { nChildren = m_pModel->rowCount(); });

    return nChildren;
}

bool QtInstanceIconView::handleToolTipEvent(const QHelpEvent& rHelpEvent)
{
    QModelIndex aIndex = m_pListView->indexAt(rHelpEvent.pos());
    if (!aIndex.isValid())
        return QtInstanceWidget::handleToolTipEvent(rHelpEvent);

    SolarMutexGuard g;
    const QtInstanceTreeIter aIter(aIndex);
    const OUString sToolTip = signal_query_tooltip(aIter);
    if (sToolTip.isEmpty())
        return QtInstanceWidget::handleToolTipEvent(rHelpEvent);

    QToolTip::showText(rHelpEvent.globalPos(), toRichTextTooltip(sToolTip), m_pListView,
                       m_pListView->visualRect(aIndex));
    return true;
}

void QtInstanceIconView::handleActivated()
{
    SolarMutexGuard g;
    signal_item_activated();
}

void QtInstanceIconView::handleSelectionChanged()
{
    SolarMutexGuard g;
    signal_selection_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
