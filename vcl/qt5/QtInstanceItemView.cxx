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

bool QtInstanceItemView::get_iter_first(weld::TreeIter& rIter) const
{
    QtInstanceTreeIter& rQtIter = static_cast<QtInstanceTreeIter&>(rIter);
    const QModelIndex aIndex = modelIndex(0);
    rQtIter.setModelIndex(aIndex);
    return aIndex.isValid();
}

bool QtInstanceItemView::iter_next_sibling(weld::TreeIter& rIter) const
{
    QtInstanceTreeIter& rQtIter = static_cast<QtInstanceTreeIter&>(rIter);
    const QModelIndex aIndex = rQtIter.modelIndex();
    const QModelIndex aSiblingIndex = m_rModel.sibling(aIndex.row() + 1, 0, aIndex);
    rQtIter.setModelIndex(aSiblingIndex);

    return aSiblingIndex.isValid();
}

int QtInstanceItemView::get_iter_index_in_parent(const weld::TreeIter& rIter) const
{
    SolarMutexGuard g;

    int nIndex;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aIndex = modelIndex(rIter);
        nIndex = aIndex.row();
    });

    return nIndex;
}

std::unique_ptr<weld::TreeIter> QtInstanceItemView::get_iterator(int nPos) const
{
    const QModelIndex aIndex = modelIndex(nPos);
    if (aIndex.isValid())
        return std::make_unique<QtInstanceTreeIter>(aIndex);

    return {};
}

OUString QtInstanceItemView::get_id(const weld::TreeIter& rIter) const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        QVariant aRoleData = m_rModel.data(modelIndex(rIter), ROLE_ID);
        if (aRoleData.canConvert<QString>())
            sId = toOUString(aRoleData.toString());
    });

    return sId;
}

void QtInstanceItemView::set_id(const weld::TreeIter& rIter, const OUString& rId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread(
        [&] { m_rModel.setData(modelIndex(rIter), toQString(rId), ROLE_ID); });
}

bool QtInstanceItemView::get_selected(weld::TreeIter* pIter) const
{
    SolarMutexGuard g;

    bool bHasSelection = false;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndexList aSelectedIndexes = getSelectionModel().selectedIndexes();
        if (aSelectedIndexes.empty())
            return;

        bHasSelection = true;
        if (pIter)
            static_cast<QtInstanceTreeIter*>(pIter)->setModelIndex(aSelectedIndexes.first());
    });
    return bHasSelection;
}

bool QtInstanceItemView::get_cursor(weld::TreeIter* pIter) const
{
    SolarMutexGuard g;

    bool bRet = false;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aCurrentIndex = getItemView().currentIndex();
        QtInstanceTreeIter* pQtIter = static_cast<QtInstanceTreeIter*>(pIter);
        if (pQtIter)
            pQtIter->setModelIndex(aCurrentIndex);
        bRet = aCurrentIndex.isValid();
    });

    return bRet;
}

void QtInstanceItemView::do_set_cursor(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getItemView().setCurrentIndex(modelIndex(rIter)); });
}

void QtInstanceItemView::do_select(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QItemSelectionModel::SelectionFlags eFlags
            = QItemSelectionModel::Select | QItemSelectionModel::Rows;
        if (getItemView().selectionMode() == QAbstractItemView::SingleSelection)
            eFlags |= QItemSelectionModel::Clear;

        getSelectionModel().select(modelIndex(rIter), eFlags);
    });
}

void QtInstanceItemView::do_unselect(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread(
        [&] { getSelectionModel().select(modelIndex(rIter), QItemSelectionModel::Deselect); });
}

void QtInstanceItemView::do_select_all()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { getItemView().selectAll(); });
}

void QtInstanceItemView::do_unselect_all()
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

QAbstractItemView& QtInstanceItemView::getItemView() const
{
    QAbstractItemView* pView = qobject_cast<QAbstractItemView*>(getQWidget());
    assert(pView);
    return *pView;
}

QItemSelectionModel& QtInstanceItemView::getSelectionModel() const
{
    QItemSelectionModel* pSelectionModel = getItemView().selectionModel();
    assert(pSelectionModel);
    return *pSelectionModel;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
