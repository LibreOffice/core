/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceItemView.hxx>
#include <QtInstanceItemView.moc>

QtInstanceItemView::QtInstanceItemView(QAbstractItemView* pItemView, QAbstractItemModel& rModel)
    : QtInstanceWidget(pItemView)
    , m_rModel(rModel)
{
}

std::unique_ptr<weld::TreeIter> QtInstanceItemView::make_iterator(const weld::TreeIter* pOrig) const
{
    const QModelIndex aIndex = pOrig ? modelIndex(*pOrig) : QModelIndex();
    return std::make_unique<QtInstanceTreeIter>(aIndex);
}

std::unique_ptr<weld::TreeIter> QtInstanceItemView::get_iterator(int nPos) const
{
    const QModelIndex aIndex = modelIndex(nPos);
    if (aIndex.isValid())
        return std::make_unique<QtInstanceTreeIter>(aIndex);

    return {};
}

void QtInstanceItemView::select_all()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { getItemView().selectAll(); });
}

void QtInstanceItemView::unselect_all()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { getItemView().clearSelection(); });
}

void QtInstanceItemView::do_clear()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_rModel.removeRows(0, m_rModel.rowCount()); });
}

QModelIndex QtInstanceItemView::modelIndex(int nRow, int nCol,
                                           const QModelIndex& rParentIndex) const
{
    return modelIndex(treeIter(nRow, rParentIndex), nCol);
}

QModelIndex QtInstanceItemView::modelIndex(const weld::TreeIter& rIter, int nCol) const
{
    QModelIndex aModelIndex = static_cast<const QtInstanceTreeIter&>(rIter).modelIndex();
    return m_rModel.index(aModelIndex.row(), nCol, aModelIndex.parent());
}

QtInstanceTreeIter QtInstanceItemView::treeIter(int nRow, const QModelIndex& rParentIndex) const
{
    return QtInstanceTreeIter(m_rModel.index(nRow, 0, rParentIndex));
}

QAbstractItemView& QtInstanceItemView::getItemView()
{
    QAbstractItemView* pView = qobject_cast<QAbstractItemView*>(getQWidget());
    assert(pView);
    return *pView;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
