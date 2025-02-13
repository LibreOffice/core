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

#include <vcl/qt/QtUtils.hxx>

// role used for the ID in the QStandardItem
constexpr int ROLE_ID = Qt::UserRole + 1000;

QtInstanceIconView::QtInstanceIconView(QListView* pListView)
    : QtInstanceWidget(pListView)
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

void QtInstanceIconView::insert(int, const OUString*, const OUString*, const OUString*,
                                weld::TreeIter*)
{
    assert(false && "Not implemented yet");
}

void QtInstanceIconView::insert(int nPos, const OUString* pStr, const OUString* pId,
                                const VirtualDevice* pIcon, weld::TreeIter* pRet)
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
            pItem->setIcon(QIcon(toQPixmap(*pIcon)));
            // set list view icon size to avoid downscaling
            const QSize aIconSize
                = m_pListView->iconSize().expandedTo(toQSize(pIcon->GetOutputSizePixel()));
            m_pListView->setIconSize(aIconSize);
        }

        m_pModel->insertRow(nPos, pItem);
    });
}

void QtInstanceIconView::insert_separator(int, const OUString*)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceIconView::get_selected_id() const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndexList aSelectedIndexes = m_pSelectionModel->selectedIndexes();
        if (aSelectedIndexes.empty())
            return;

        QVariant aIdData = aSelectedIndexes.first().data(ROLE_ID);
        if (aIdData.canConvert<QString>())
            sId = toOUString(aIdData.toString());
    });

    return sId;
}

void QtInstanceIconView::clear()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pModel->clear(); });
}

int QtInstanceIconView::count_selected_items() const
{
    assert(false && "Not implemented yet");
    return 0;
}

OUString QtInstanceIconView::get_selected_text() const
{
    assert(false && "Not implemented yet");
    return OUString();
}

OUString QtInstanceIconView::get_id(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceIconView::select(int nPos)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pSelectionModel->select(m_pModel->index(nPos, 0), QItemSelectionModel::Select); });
}

void QtInstanceIconView::unselect(int) { assert(false && "Not implemented yet"); }

void QtInstanceIconView::set_image(int, VirtualDevice&) { assert(false && "Not implemented yet"); }

void QtInstanceIconView::set_text(int, const OUString&) { assert(false && "Not implemented yet"); }

void QtInstanceIconView::set_id(int, const OUString&) { assert(false && "Not implemented yet"); }

void QtInstanceIconView::remove(int) { assert(false && "Not implemented yet"); }

tools::Rectangle QtInstanceIconView::get_rect(int) const
{
    assert(false && "Not implemented yet");
    return tools::Rectangle();
}

std::unique_ptr<weld::TreeIter> QtInstanceIconView::make_iterator(const weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return nullptr;
}

bool QtInstanceIconView::get_selected(weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceIconView::get_cursor(weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceIconView::set_cursor(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceIconView::get_iter_first(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

OUString QtInstanceIconView::get_id(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

OUString QtInstanceIconView::get_text(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

bool QtInstanceIconView::iter_next_sibling(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceIconView::scroll_to_item(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
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
