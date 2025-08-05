/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceTreeView.hxx>
#include <QtInstanceTreeView.moc>

#include <vcl/qt/QtUtils.hxx>

#include <QtWidgets/QHeaderView>

// role used for the ID in the QStandardItem
constexpr int ROLE_ID = Qt::UserRole + 1000;

// Property used to store the supported roles for each of the columns
const char* const PROPERTY_COLUMN_ROLES = "column-roles";

QtInstanceTreeView::QtInstanceTreeView(QTreeView* pTreeView)
    : QtInstanceWidget(pTreeView)
    , m_pTreeView(pTreeView)
{
    assert(m_pTreeView);

    m_pModel = qobject_cast<QSortFilterProxyModel*>(m_pTreeView->model());
    assert(m_pModel && "tree view doesn't have expected QSortFilterProxyModel set");

    m_pSourceModel = qobject_cast<QStandardItemModel*>(m_pModel->sourceModel());
    assert(m_pSourceModel && "proxy model doesn't have expected source model");

    m_pSelectionModel = m_pTreeView->selectionModel();
    assert(m_pSelectionModel);

    m_pColumnRoles = columnRoles(*pTreeView);
    assert(m_pColumnRoles.size() == m_pModel->columnCount() && "column count doesn't match");

    connect(m_pTreeView, &QTreeView::activated, this, &QtInstanceTreeView::handleActivated);
    connect(m_pSelectionModel, &QItemSelectionModel::selectionChanged, this,
            &QtInstanceTreeView::handleSelectionChanged);
    connect(m_pModel, &QSortFilterProxyModel::dataChanged, this,
            &QtInstanceTreeView::handleDataChanged);
}

void QtInstanceTreeView::insert(const weld::TreeIter* pParent, int nPos, const OUString* pStr,
                                const OUString* pId, const OUString* pIconName,
                                VirtualDevice* pImageSurface, bool bChildrenOnDemand,
                                weld::TreeIter* pRet)
{
    assert(!bChildrenOnDemand && "Not implemented yet");
    // avoid -Werror=unused-parameter for release build
    (void)bChildrenOnDemand;

    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aParentIndex
            = pParent ? static_cast<const QtInstanceTreeIter*>(pParent)->modelIndex()
                      : QModelIndex();

        if (nPos == -1)
            nPos = m_pModel->rowCount(aParentIndex);
        m_pModel->insertRow(nPos, aParentIndex);

        // ensure parent has same column count as top-level
        if (aParentIndex.isValid() && m_pModel->columnCount(aParentIndex) == 0)
            m_pModel->insertColumns(0, m_pModel->columnCount(), aParentIndex);

        const QModelIndex aIndex = modelIndex(nPos, 0, aParentIndex);
        QStandardItem* pItem = itemFromIndex(aIndex);
        if (pStr)
            set_text(treeIter(nPos), *pStr);
        if (pId)
            pItem->setData(toQString(*pId), ROLE_ID);

        if (pIconName && !pIconName->isEmpty())
            pItem->setIcon(loadQPixmapIcon(*pIconName));
        else if (pImageSurface)
            pItem->setIcon(toQPixmap(*pImageSurface));

        if (m_bExtraToggleButtonsEnabled)
            itemFromIndex(toggleButtonModelIndex(QtInstanceTreeIter(aIndex)))->setCheckable(true);

        if (pRet)
            static_cast<QtInstanceTreeIter*>(pRet)->setModelIndex(aIndex);
    });
}

void QtInstanceTreeView::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceTreeView::get_selected_text() const
{
    SolarMutexGuard g;

    OUString sText;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndexList aSelectedIndexes = m_pSelectionModel->selectedIndexes();
        if (aSelectedIndexes.empty())
            return;

        sText = toOUString(itemFromIndex(aSelectedIndexes.first())->text());
    });

    return sText;
}

OUString QtInstanceTreeView::get_selected_id() const
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

void QtInstanceTreeView::enable_toggle_buttons(weld::ColumnToggleType)
{
    assert(m_pModel->rowCount() == 0 && "Must be called before inserting any data");

    m_bExtraToggleButtonsEnabled = true;

    m_pTreeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}

void QtInstanceTreeView::set_clicks_to_toggle(int) { assert(false && "Not implemented yet"); }

int QtInstanceTreeView::get_selected_index() const
{
    SolarMutexGuard g;

    int nIndex = -1;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndexList aSelectedIndexes = m_pSelectionModel->selectedIndexes();
        if (aSelectedIndexes.empty())
            return;

        nIndex = aSelectedIndexes.first().row();
    });

    return nIndex;
}

void QtInstanceTreeView::select(int nPos) { select(treeIter(nPos)); }

void QtInstanceTreeView::unselect(int nPos) { unselect(treeIter(nPos)); }

void QtInstanceTreeView::remove(int nPos) { remove(treeIter(nPos)); }

OUString QtInstanceTreeView::get_text(int nRow, int nCol) const
{
    return get_text(treeIter(nRow), nCol);
}

void QtInstanceTreeView::set_text(int nRow, const OUString& rText, int nCol)
{
    set_text(treeIter(nRow), rText, nCol);
}

void QtInstanceTreeView::set_sensitive(int nRow, bool bSensitive, int nCol)
{
    set_sensitive(treeIter(nRow), bSensitive, nCol);
}

bool QtInstanceTreeView::get_sensitive(int nRow, int nCol) const
{
    return get_sensitive(treeIter(nRow), nCol);
}

void QtInstanceTreeView::set_id(int nRow, const OUString& rId) { set_id(treeIter(nRow), rId); }

void QtInstanceTreeView::set_toggle(int nRow, TriState eState, int nCol)
{
    set_toggle(treeIter(nRow), eState, nCol);
}

TriState QtInstanceTreeView::get_toggle(int nRow, int nCol) const
{
    return get_toggle(treeIter(nRow), nCol);
}

void QtInstanceTreeView::set_image(int nRow, const OUString& rImage, int nCol)
{
    set_image(treeIter(nRow), rImage, nCol);
}

void QtInstanceTreeView::set_image(int nRow, VirtualDevice& rImage, int nCol)
{
    set_image(treeIter(nRow), rImage, nCol);
}

void QtInstanceTreeView::set_image(int nRow,
                                   const css::uno::Reference<css::graphic::XGraphic>& rImage,
                                   int nCol)
{
    set_image(treeIter(nRow), rImage, nCol);
}

void QtInstanceTreeView::set_text_emphasis(int nRow, bool bOn, int nCol)
{
    return set_text_emphasis(treeIter(nRow), bOn, nCol);
}

bool QtInstanceTreeView::get_text_emphasis(int nRow, int nCol) const
{
    return get_text_emphasis(treeIter(nRow), nCol);
}

void QtInstanceTreeView::set_text_align(int nRow, double fAlign, int nCol)
{
    return set_text_align(treeIter(nRow), fAlign, nCol);
}

void QtInstanceTreeView::swap(int nPos1, int nPos2)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const bool bPos1Selected = m_pSelectionModel->isRowSelected(nPos1);
        const bool bPos2Selected = m_pSelectionModel->isRowSelected(nPos2);

        const int nSourceModelPos1 = m_pModel->mapToSource(modelIndex(nPos1)).row();
        const int nSourceModelPos2 = m_pModel->mapToSource(modelIndex(nPos2)).row();

        const int nMin = std::min(nSourceModelPos1, nSourceModelPos2);
        const int nMax = std::max(nSourceModelPos1, nSourceModelPos2);
        QList<QStandardItem*> aMaxRow = m_pSourceModel->takeRow(nMax);
        QList<QStandardItem*> aMinRow = m_pSourceModel->takeRow(nMin);
        m_pSourceModel->insertRow(nMin, aMaxRow);
        m_pSourceModel->insertRow(nMax, aMinRow);

        // restore selection
        if (bPos1Selected)
            select(m_pModel->mapFromSource(m_pSourceModel->index(nSourceModelPos2, 0)).row());
        if (bPos2Selected)
            select(m_pModel->mapFromSource(m_pSourceModel->index(nSourceModelPos1, 0)).row());
    });
}

std::vector<int> QtInstanceTreeView::get_selected_rows() const
{
    SolarMutexGuard g;

    std::vector<int> aSelectedRows;

    GetQtInstance().RunInMainThread([&] {
        const QModelIndexList aSelectionIndexes = m_pSelectionModel->selectedRows();
        for (const QModelIndex& aIndex : aSelectionIndexes)
            aSelectedRows.push_back(aIndex.row());
    });

    return aSelectedRows;
}

void QtInstanceTreeView::set_font_color(int nPos, const Color& rColor)
{
    set_font_color(treeIter(nPos), rColor);
}

void QtInstanceTreeView::scroll_to_row(int nRow) { scroll_to_row(treeIter(nRow)); }

bool QtInstanceTreeView::is_selected(int nPos) const { return is_selected(treeIter(nPos)); }

int QtInstanceTreeView::get_cursor_index() const
{
    SolarMutexGuard g;

    int nIndex = -1;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aCurrentIndex = m_pSelectionModel->currentIndex();
        if (aCurrentIndex.isValid())
            nIndex = aCurrentIndex.row();

    });

    return nIndex;
}

void QtInstanceTreeView::set_cursor(int nPos) { set_cursor(treeIter(nPos)); }

int QtInstanceTreeView::find_text(const OUString& rText) const
{
    SolarMutexGuard g;

    int nIndex = -1;
    GetQtInstance().RunInMainThread([&] {
        // search in underlying QStandardItemModel and map index
        const QList<QStandardItem*> aItems = m_pSourceModel->findItems(toQString(rText));
        if (!aItems.empty())
            nIndex = m_pModel->mapFromSource(aItems.at(0)->index()).row();
    });

    return nIndex;
}

OUString QtInstanceTreeView::get_id(int nPos) const { return get_id(treeIter(nPos)); }

int QtInstanceTreeView::find_id(const OUString& rId) const
{
    SolarMutexGuard g;

    int nIndex = -1;
    GetQtInstance().RunInMainThread([&] {
        for (int i = 0; i < m_pModel->rowCount(); i++)
        {
            if (get_id(i) == rId)
            {
                nIndex = i;
                return;
            }
        }
    });

    return nIndex;
}

std::unique_ptr<weld::TreeIter> QtInstanceTreeView::make_iterator(const weld::TreeIter* pOrig) const
{
    const QModelIndex aIndex = pOrig ? modelIndex(*pOrig) : QModelIndex();
    return std::make_unique<QtInstanceTreeIter>(aIndex);
}

void QtInstanceTreeView::copy_iterator(const weld::TreeIter& rSource, weld::TreeIter& rDest) const
{
    const QModelIndex aModelIndex = static_cast<const QtInstanceTreeIter&>(rSource).modelIndex();
    static_cast<QtInstanceTreeIter&>(rDest).setModelIndex(aModelIndex);
}

bool QtInstanceTreeView::get_selected(weld::TreeIter* pIter) const
{
    SolarMutexGuard g;

    bool bHasSelection = false;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndexList aSelectedIndexes = m_pSelectionModel->selectedIndexes();
        if (aSelectedIndexes.empty())
            return;

        bHasSelection = true;
        if (pIter)
            static_cast<QtInstanceTreeIter*>(pIter)->setModelIndex(aSelectedIndexes.first());
    });
    return bHasSelection;
}

bool QtInstanceTreeView::get_cursor(weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_cursor(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pSelectionModel->setCurrentIndex(modelIndex(rIter), QItemSelectionModel::Select);
    });
}

bool QtInstanceTreeView::get_iter_first(weld::TreeIter& rIter) const
{
    QtInstanceTreeIter& rQtIter = static_cast<QtInstanceTreeIter&>(rIter);
    const QModelIndex aIndex = modelIndex(0);
    rQtIter.setModelIndex(aIndex);
    return aIndex.isValid();
}

bool QtInstanceTreeView::iter_next_sibling(weld::TreeIter& rIter) const
{
    QtInstanceTreeIter& rQtIter = static_cast<QtInstanceTreeIter&>(rIter);
    const QModelIndex aIndex = rQtIter.modelIndex();
    const QModelIndex aSiblingIndex = m_pModel->sibling(aIndex.row() + 1, 0, aIndex);
    rQtIter.setModelIndex(aSiblingIndex);

    return aSiblingIndex.isValid();
}

bool QtInstanceTreeView::iter_previous_sibling(weld::TreeIter& rIter) const
{
    QtInstanceTreeIter& rQtIter = static_cast<QtInstanceTreeIter&>(rIter);
    const QModelIndex aIndex = rQtIter.modelIndex();
    const QModelIndex aSiblingIndex = m_pModel->sibling(aIndex.row() - 1, 0, aIndex);
    rQtIter.setModelIndex(aSiblingIndex);

    return aSiblingIndex.isValid();
}

bool QtInstanceTreeView::iter_next(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_previous(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_children(weld::TreeIter& rIter) const
{
    QtInstanceTreeIter& rQtIter = static_cast<QtInstanceTreeIter&>(rIter);
    const QModelIndex aChildIndex = m_pModel->index(0, 0, rQtIter.modelIndex());
    rQtIter.setModelIndex(aChildIndex);

    return aChildIndex.isValid();
}

bool QtInstanceTreeView::iter_parent(weld::TreeIter& rIter) const
{
    QtInstanceTreeIter& rQtIter = static_cast<QtInstanceTreeIter&>(rIter);
    const QModelIndex aParentIndex = rQtIter.modelIndex().parent();
    rQtIter.setModelIndex(aParentIndex);

    return aParentIndex.isValid();
}

int QtInstanceTreeView::get_iter_depth(const weld::TreeIter& rIter) const
{
    const QtInstanceTreeIter& rQtIter = static_cast<const QtInstanceTreeIter&>(rIter);
    int nDepth = 0;
    QModelIndex aParentIndex = rQtIter.modelIndex().parent();
    while (aParentIndex.isValid())
    {
        nDepth++;
        aParentIndex = aParentIndex.parent();
    }

    return nDepth;
}

int QtInstanceTreeView::get_iter_index_in_parent(const weld::TreeIter& rIter) const
{
    SolarMutexGuard g;

    int nIndex;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aIndex = modelIndex(rIter);
        nIndex = aIndex.row();
    });

    return nIndex;
}

int QtInstanceTreeView::iter_compare(const weld::TreeIter&, const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return 0;
}

bool QtInstanceTreeView::iter_has_child(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

int QtInstanceTreeView::iter_n_children(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::remove(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aIndex = modelIndex(rIter);
        m_pModel->removeRow(aIndex.row(), aIndex.parent());
    });
}

void QtInstanceTreeView::select(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QItemSelectionModel::SelectionFlags eFlags
            = QItemSelectionModel::Select | QItemSelectionModel::Rows;
        if (m_pTreeView->selectionMode() == QAbstractItemView::SingleSelection)
            eFlags |= QItemSelectionModel::Clear;

        m_pSelectionModel->select(modelIndex(rIter), eFlags);
    });
}

void QtInstanceTreeView::unselect(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread(
        [&] { m_pSelectionModel->select(modelIndex(rIter), QItemSelectionModel::Deselect); });
}

void QtInstanceTreeView::set_extra_row_indent(const weld::TreeIter&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_text(const weld::TreeIter& rIter, const OUString& rStr, int nCol)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aIndex
            = nCol == -1 ? firstTextColumnModelIndex(rIter) : modelIndex(rIter, nCol);
        m_pModel->setData(aIndex, toQString(rStr));
    });
}

void QtInstanceTreeView::set_sensitive(const weld::TreeIter& rIter, bool bSensitive, int nCol)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        // column index -1 means "all columns"
        if (nCol == -1)
        {
            for (int i = 0; i < m_pModel->columnCount(); ++i)
                set_sensitive(rIter, bSensitive, i);
            return;
        }

        QStandardItem* pItem = itemFromIndex(modelIndex(rIter, nCol));
        if (pItem)
        {
            if (bSensitive)
                pItem->setFlags(pItem->flags() | Qt::ItemIsEnabled);
            else
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEnabled);
        }
    });
}

bool QtInstanceTreeView::get_sensitive(const weld::TreeIter& rIter, int nCol) const
{
    SolarMutexGuard g;

    bool bSensitive = false;
    GetQtInstance().RunInMainThread([&] {
        QStandardItem* pItem = itemFromIndex(modelIndex(rIter, nCol));
        if (pItem)
            bSensitive = pItem->flags() & Qt::ItemIsEnabled;
    });

    return bSensitive;
}

void QtInstanceTreeView::set_text_emphasis(const weld::TreeIter&, bool, int)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_text_emphasis(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_text_align(const weld::TreeIter&, double, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_toggle(const weld::TreeIter& rIter, TriState eState, int nCol)
{
    SolarMutexGuard g;

    assert((nCol != 0 || !m_bExtraToggleButtonsEnabled)
           && "Column 0 is already used by \"expander toggle\" using special index -1");

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = nCol == -1 ? toggleButtonModelIndex(rIter) : modelIndex(rIter, nCol);
        itemFromIndex(aIndex)->setCheckState(toQtCheckState(eState));
    });
}

TriState QtInstanceTreeView::get_toggle(const weld::TreeIter& rIter, int nCol) const
{
    SolarMutexGuard g;

    assert((nCol != 0 || !m_bExtraToggleButtonsEnabled)
           && "Column 0 is already used by \"expander toggle\" using special index -1");

    TriState eState = TRISTATE_INDET;
    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = nCol == -1 ? toggleButtonModelIndex(rIter) : modelIndex(rIter, nCol);
        eState = toVclTriState(itemFromIndex(aIndex)->checkState());
    });

    return eState;
}

OUString QtInstanceTreeView::get_text(const weld::TreeIter& rIter, int nCol) const
{
    SolarMutexGuard g;

    OUString sText;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aIndex
            = nCol == -1 ? firstTextColumnModelIndex(rIter) : modelIndex(rIter, nCol);
        const QVariant aData = m_pModel->data(aIndex);
        if (aData.canConvert<QString>())
            sText = toOUString(aData.toString());
    });

    return sText;
}

void QtInstanceTreeView::set_id(const weld::TreeIter& rIter, const OUString& rId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread(
        [&] { m_pModel->setData(modelIndex(rIter), toQString(rId), ROLE_ID); });
}

OUString QtInstanceTreeView::get_id(const weld::TreeIter& rIter) const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        QVariant aRoleData = m_pModel->data(modelIndex(rIter), ROLE_ID);
        if (aRoleData.canConvert<QString>())
            sId = toOUString(aRoleData.toString());
    });

    return sId;
}

void QtInstanceTreeView::set_image(const weld::TreeIter& rIter, const OUString& rImage, int nCol)
{
    if (rImage.isEmpty())
        return;

    setImage(rIter, loadQPixmapIcon(rImage), nCol);
}

void QtInstanceTreeView::set_image(const weld::TreeIter& rIter, VirtualDevice& rImage, int nCol)
{
    setImage(rIter, toQPixmap(rImage), nCol);
}

void QtInstanceTreeView::set_image(const weld::TreeIter& rIter,
                                   const css::uno::Reference<css::graphic::XGraphic>& rImage,
                                   int nCol)
{
    setImage(rIter, toQPixmap(rImage), nCol);
}

void QtInstanceTreeView::set_font_color(const weld::TreeIter& rIter, const Color& rColor)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        for (int nCol = 0; nCol < m_pModel->columnCount(); nCol++)
        {
            QModelIndex aIndex = modelIndex(rIter, nCol);
            m_pModel->setData(aIndex, QBrush(toQColor(rColor)), Qt::ForegroundRole);
        }
    });
}

void QtInstanceTreeView::scroll_to_row(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pTreeView->scrollTo(modelIndex(rIter)); });
}

bool QtInstanceTreeView::is_selected(const weld::TreeIter& rIter) const
{
    SolarMutexGuard g;

    bool bSelected = false;
    GetQtInstance().RunInMainThread(
        [&] { bSelected = m_pSelectionModel->isSelected(modelIndex(rIter)); });

    return bSelected;
}

void QtInstanceTreeView::move_subtree(weld::TreeIter&, const weld::TreeIter*, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::all_foreach(const std::function<bool(weld::TreeIter&)>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::selected_foreach(const std::function<bool(weld::TreeIter&)>& func)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndexList aSelectionIndexes = m_pSelectionModel->selectedRows();
        for (QModelIndex& aIndex : aSelectionIndexes)
        {
            QtInstanceTreeIter aIter(aIndex);
            if (func(aIter))
                return;
        }
    });
}

void QtInstanceTreeView::visible_foreach(const std::function<bool(weld::TreeIter&)>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::bulk_insert_for_each(
    int nSourceCount, const std::function<void(weld::TreeIter&, int nSourceIndex)>& func,
    const weld::TreeIter* pParent, const std::vector<int>* pFixedWidths, bool)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        // clear existing children
        const QModelIndex aParentIndex = pParent ? modelIndex(*pParent) : QModelIndex();
        const int nRows = m_pModel->rowCount(aParentIndex);
        if (nRows > 0)
            m_pModel->removeRows(0, nRows, aParentIndex);

        // insert new rows
        m_pModel->insertRows(0, nSourceCount, aParentIndex);

        // call function for each row
        for (int nRow = 0; nRow < nSourceCount; nRow++)
        {
            QtInstanceTreeIter aIter = treeIter(nRow, aParentIndex);
            func(aIter, nRow);
        }

        // set column widths
        if (pFixedWidths)
            set_column_fixed_widths(*pFixedWidths);
    });
}

bool QtInstanceTreeView::get_row_expanded(const weld::TreeIter& rIter) const
{
    SolarMutexGuard g;

    bool bExpanded = false;
    GetQtInstance().RunInMainThread(
        [&] { bExpanded = m_pTreeView->isExpanded(modelIndex(rIter)); });

    return bExpanded;
}

void QtInstanceTreeView::expand_row(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pTreeView->expand(modelIndex(rIter)); });
}

void QtInstanceTreeView::collapse_row(const weld::TreeIter& rIter)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pTreeView->collapse(modelIndex(rIter)); });
}

void QtInstanceTreeView::set_children_on_demand(const weld::TreeIter&, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_children_on_demand(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_show_expanders(bool) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::start_editing(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::end_editing() { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::enable_drag_source(rtl::Reference<TransferDataContainer>&, sal_uInt8)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::select_all()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTreeView->selectAll(); });
}

void QtInstanceTreeView::unselect_all()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTreeView->clearSelection(); });
}

int QtInstanceTreeView::n_children() const
{
    SolarMutexGuard g;

    int nChildCount;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aRootIndex
            = m_pModel->mapFromSource(m_pSourceModel->invisibleRootItem()->index());
        nChildCount = m_pModel->rowCount(aRootIndex);
    });
    return nChildCount;
}

void QtInstanceTreeView::make_sorted()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pTreeView->setSortingEnabled(true);
        // sort by first column
        m_pModel->sort(0);
    });
}

void QtInstanceTreeView::make_unsorted()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pTreeView->setSortingEnabled(false);
        m_pModel->sort(-1);
    });
}

bool QtInstanceTreeView::get_sort_order() const
{
    SolarMutexGuard g;

    bool bAscending = true;
    GetQtInstance().RunInMainThread(
        [&] { bAscending = m_pModel->sortOrder() == Qt::AscendingOrder; });

    return bAscending;
}

void QtInstanceTreeView::set_sort_order(bool bAscending)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const Qt::SortOrder eOrder = bAscending ? Qt::AscendingOrder : Qt::DescendingOrder;
        m_pModel->sort(m_pModel->sortColumn(), eOrder);
    });
}

void QtInstanceTreeView::set_sort_indicator(TriState, int)
{
    assert(false && "Not implemented yet");
}

TriState QtInstanceTreeView::get_sort_indicator(int) const
{
    assert(false && "Not implemented yet");
    return TRISTATE_INDET;
}

int QtInstanceTreeView::get_sort_column() const
{
    SolarMutexGuard g;

    int nSortColumn = 0;
    GetQtInstance().RunInMainThread([&] { nSortColumn = m_pModel->sortColumn(); });

    return nSortColumn;
}

void QtInstanceTreeView::set_sort_column(int nColumn)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pModel->sort(nColumn); });
}

void QtInstanceTreeView::clear()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        // don't use QStandardItemModel::clear, as that would remove header data as well
        m_pModel->removeRows(0, m_pModel->rowCount());
    });
}

int QtInstanceTreeView::get_height_rows(int) const
{
    SAL_WARN("vcl.qt", "QtInstanceTreeView::get_height_rows just returns 0 for now");
    return 0;
}

void QtInstanceTreeView::columns_autosize()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        for (int i = 0; i < m_pModel->columnCount(); i++)
            m_pTreeView->resizeColumnToContents(i);
    });
}

void QtInstanceTreeView::set_column_fixed_widths(const std::vector<int>& rWidths)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        assert(rWidths.size() <= o3tl::make_unsigned(m_pModel->columnCount()));
        for (size_t i = 0; i < rWidths.size(); ++i)
            m_pTreeView->setColumnWidth(i, rWidths.at(i));
    });
}

void QtInstanceTreeView::set_column_editables(const std::vector<bool>&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceTreeView::get_column_width(int nCol) const
{
    SolarMutexGuard g;

    int nWidth = 0;
    GetQtInstance().RunInMainThread([&] { nWidth = m_pTreeView->columnWidth(nCol); });

    return nWidth;
}

void QtInstanceTreeView::set_centered_column(int) { assert(false && "Not implemented yet"); }

OUString QtInstanceTreeView::get_column_title(int nColumn) const
{
    SolarMutexGuard g;

    OUString sTitle;
    GetQtInstance().RunInMainThread([&] {
        const QVariant aVariant = m_pModel->headerData(nColumn, Qt::Horizontal);
        assert(aVariant.isValid() && aVariant.canConvert<QString>());
        sTitle = toOUString(aVariant.toString());
    });

    return sTitle;
}

void QtInstanceTreeView::set_column_title(int nColumn, const OUString& rTitle)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread(
        [&] { m_pModel->setHeaderData(nColumn, Qt::Horizontal, toQString(rTitle)); });
}

void QtInstanceTreeView::set_selection_mode(SelectionMode eMode)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pTreeView->setSelectionMode(mapSelectionMode(eMode)); });
}

int QtInstanceTreeView::count_selected_rows() const { return get_selected_rows().size(); }

void QtInstanceTreeView::remove_selection() { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::changed_by_hover() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::vadjustment_set_value(int) { assert(false && "Not implemented yet"); }

int QtInstanceTreeView::vadjustment_get_value() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::set_column_custom_renderer(int, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::queue_draw() { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::get_dest_row_at_pos(const Point&, weld::TreeIter*, bool, bool)
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::unset_drag_dest_row() { assert(false && "Not implemented yet"); }

tools::Rectangle QtInstanceTreeView::get_row_area(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return tools::Rectangle();
}

weld::TreeView* QtInstanceTreeView::get_drag_source() const
{
    assert(false && "Not implemented yet");
    return nullptr;
}

QAbstractItemView::SelectionMode QtInstanceTreeView::mapSelectionMode(SelectionMode eMode)
{
    switch (eMode)
    {
        case SelectionMode::NONE:
            return QAbstractItemView::NoSelection;
        case SelectionMode::Single:
            return QAbstractItemView::SingleSelection;
        case SelectionMode::Range:
            return QAbstractItemView::ContiguousSelection;
        case SelectionMode::Multiple:
            return QAbstractItemView::ExtendedSelection;
        default:
            assert(false && "unhandled selection mode");
            return QAbstractItemView::SingleSelection;
    }
}

QList<QList<Qt::ItemDataRole>> QtInstanceTreeView::columnRoles(QTreeView& rTreeView)
{
    QVariant aVariant = rTreeView.property(PROPERTY_COLUMN_ROLES);
    if (aVariant.isValid())
    {
        assert(aVariant.canConvert<QList<QList<Qt::ItemDataRole>>>());
        return aVariant.value<QList<QList<Qt::ItemDataRole>>>();
    }

    return {};
}

void QtInstanceTreeView::setColumnRoles(QTreeView& rTreeView,
                                        const QList<QList<Qt::ItemDataRole>>& rDataRoles)
{
    rTreeView.setProperty(PROPERTY_COLUMN_ROLES, QVariant::fromValue(rDataRoles));
}

QModelIndex QtInstanceTreeView::modelIndex(int nRow, int nCol,
                                           const QModelIndex& rParentIndex) const
{
    return modelIndex(treeIter(nRow, rParentIndex), nCol);
}

QModelIndex QtInstanceTreeView::modelIndex(const weld::TreeIter& rIter, int nCol) const
{
    QModelIndex aModelIndex = static_cast<const QtInstanceTreeIter&>(rIter).modelIndex();
    return m_pModel->index(aModelIndex.row(), nCol, aModelIndex.parent());
}

QtInstanceTreeIter QtInstanceTreeView::treeIter(int nRow, const QModelIndex& rParentIndex) const
{
    return QtInstanceTreeIter(m_pModel->index(nRow, 0, rParentIndex));
}

QStandardItem* QtInstanceTreeView::itemFromIndex(const QModelIndex& rIndex) const
{
    const QModelIndex aSourceIndex = m_pModel->mapToSource(rIndex);
    return m_pSourceModel->itemFromIndex(aSourceIndex);
}

QModelIndex QtInstanceTreeView::toggleButtonModelIndex(const weld::TreeIter& rIter) const
{
    assert(m_bExtraToggleButtonsEnabled && "Special toggle buttons are not enabled");

    const QModelIndex aIndex = modelIndex(rIter);
    // Special toggle buttons are always in the first column
    return m_pModel->index(aIndex.row(), 0, aIndex.parent());
}

QModelIndex QtInstanceTreeView::firstTextColumnModelIndex(const weld::TreeIter& rIter) const
{
    for (qsizetype i = 0; i < m_pColumnRoles.size(); i++)
    {
        if (m_pColumnRoles.at(i).contains(Qt::DisplayRole))
            return modelIndex(rIter, i);
    }

    assert(false && "No text column found");
    return QModelIndex();
}

void QtInstanceTreeView::setImage(const weld::TreeIter& rIter, const QPixmap& rPixmap, int nCol)
{
    // set the "expander image" for the Qt::DecorationRole in the first column
    // It can still contain additional data (like text) for other roles, but not
    // another image. If that turns out to be problematic, another solution needs
    // to be found.
    if (nCol == -1)
        nCol = 0;

    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = modelIndex(rIter, nCol);
        m_pModel->setData(aIndex, rPixmap, Qt::DecorationRole);
    });
}

void QtInstanceTreeView::handleActivated()
{
    SolarMutexGuard g;
    signal_row_activated();
}

void QtInstanceTreeView::handleDataChanged(const QModelIndex& rTopLeft,
                                           const QModelIndex& rBottomRight,
                                           const QVector<int>& rRoles)
{
    SolarMutexGuard g;

    // only notify about check state changes
    if (!rRoles.contains(Qt::CheckStateRole))
        return;

    assert(rTopLeft == rBottomRight && "Case of multiple changes not implemented yet");
    (void)rBottomRight;

    int nColIndex = rTopLeft.column();
    if (m_bExtraToggleButtonsEnabled && nColIndex == 0)
        // use special index of -1 for the "expander toggle"
        nColIndex = -1;

    signal_toggled(iter_col(QtInstanceTreeIter(rTopLeft), nColIndex));
}

void QtInstanceTreeView::handleSelectionChanged()
{
    SolarMutexGuard g;
    signal_selection_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
