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

// role used for the ID in the QStandardItem
constexpr int ROLE_ID = Qt::UserRole + 1000;

namespace
{
struct QtInstanceTreeIter final : public weld::TreeIter
{
    QModelIndex m_aModelIndex;

    explicit QtInstanceTreeIter(QModelIndex aModelIndex)
        : m_aModelIndex(aModelIndex)
    {
    }

    virtual bool equal(const TreeIter& rOther) const override
    {
        return m_aModelIndex == static_cast<const QtInstanceTreeIter&>(rOther).m_aModelIndex;
    }
};
};

QtInstanceTreeView::QtInstanceTreeView(QTreeView* pTreeView)
    : QtInstanceWidget(pTreeView)
    , m_pTreeView(pTreeView)
{
    assert(m_pTreeView);

    m_pModel = qobject_cast<QStandardItemModel*>(m_pTreeView->model());
    assert(m_pModel && "tree view doesn't have expected item model set");

    m_pSelectionModel = m_pTreeView->selectionModel();
    assert(m_pSelectionModel);

    connect(m_pTreeView, &QTreeView::activated, this, &QtInstanceTreeView::handleActivated);
    connect(m_pSelectionModel, &QItemSelectionModel::selectionChanged, this,
            &QtInstanceTreeView::handleSelectionChanged);
}

void QtInstanceTreeView::insert(const weld::TreeIter* pParent, int nPos, const OUString* pStr,
                                const OUString* pId, const OUString* pIconName,
                                VirtualDevice* pImageSurface, bool bChildrenOnDemand,
                                weld::TreeIter* pRet)
{
    // Only specific subset of parameters handled so far;
    // assert only these are used at the moment and implement remaining cases
    // when needed to support more dialogs, then adjust/remove asserts below
    assert(!pParent && "Not implemented yet");
    assert(!pIconName && "Not implemented yet");
    assert(!pImageSurface && "Not implemented yet");
    assert(!bChildrenOnDemand && "Not implemented yet");
    // avoid -Werror=unused-parameter for release build
    (void)pParent;
    (void)pIconName;
    (void)pImageSurface;
    (void)bChildrenOnDemand;

    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QStandardItem* pItem = new QStandardItem;
        if (pStr)
            pItem->setText(toQString(*pStr));
        if (pId)
            pItem->setData(toQString(*pId), ROLE_ID);

        if (nPos == -1)
            nPos = m_pModel->rowCount();
        m_pModel->insertRow(nPos, pItem);

        if (pRet)
            static_cast<QtInstanceTreeIter*>(pRet)->m_aModelIndex = modelIndex(nPos);
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

        sText = toOUString(m_pModel->itemFromIndex(aSelectedIndexes.first())->text());
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
    assert(false && "Not implemented yet");
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

void QtInstanceTreeView::select(int nPos)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pSelectionModel->select(m_pModel->index(nPos, 0), QItemSelectionModel::Select); });
}

void QtInstanceTreeView::unselect(int nPos)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        m_pSelectionModel->select(m_pModel->index(nPos, 0), QItemSelectionModel::Deselect);
    });
}

void QtInstanceTreeView::remove(int nPos)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pModel->removeRow(nPos); });
}

OUString QtInstanceTreeView::get_text(int nRow, int nCol) const
{
    assert(nCol == -1 && "Column support not implemented yet");
    (void)nCol; // for release build

    SolarMutexGuard g;

    OUString sText;
    GetQtInstance().RunInMainThread([&] {
        const QModelIndex aIndex = m_pModel->index(nRow, 0);
        const QVariant aData = m_pModel->data(aIndex);
        assert(aData.canConvert<QString>() && "model data not a string");
        sText = toOUString(aData.toString());
    });

    return sText;
}

void QtInstanceTreeView::set_text(int nRow, const OUString& rText, int nCol)
{
    assert(nCol != -1 && "Support for special index -1 (first text column) not implemented yet");

    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QStandardItem* pItem = m_pModel->item(nRow, nCol);
        if (!pItem)
        {
            pItem = new QStandardItem;
            m_pModel->setItem(nRow, nCol, pItem);
        }
        pItem->setText(toQString(rText));
    });
}

void QtInstanceTreeView::set_sensitive(int, bool, int) { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::get_sensitive(int, int) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_id(int, const OUString&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::set_toggle(int, TriState, int) { assert(false && "Not implemented yet"); }

TriState QtInstanceTreeView::get_toggle(int, int) const
{
    assert(false && "Not implemented yet");
    return TRISTATE_INDET;
}

void QtInstanceTreeView::set_image(int, const OUString&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_image(int, VirtualDevice&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_image(int, const css::uno::Reference<css::graphic::XGraphic>&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_text_emphasis(int, bool, int)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_text_emphasis(int, int) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_text_align(int, double, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::swap(int, int) { assert(false && "Not implemented yet"); }

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

void QtInstanceTreeView::set_font_color(int, const Color&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::scroll_to_row(int) { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::is_selected(int) const
{
    assert(false && "Not implemented yet");
    return false;
}

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

void QtInstanceTreeView::set_cursor(int nPos)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        m_pSelectionModel->setCurrentIndex(m_pModel->index(nPos, 0), QItemSelectionModel::NoUpdate);
    });
}

int QtInstanceTreeView::find_text(const OUString& rText) const
{
    SolarMutexGuard g;

    int nIndex = -1;
    GetQtInstance().RunInMainThread([&] {
        const QList<QStandardItem*> aItems = m_pModel->findItems(toQString(rText));
        if (!aItems.empty())
            nIndex = aItems.at(0)->index().row();
    });

    return nIndex;
}

OUString QtInstanceTreeView::get_id(int nPos) const { return get_id(modelIndex(nPos)); }

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
    static_cast<QtInstanceTreeIter&>(rDest).m_aModelIndex = modelIndex(rSource);
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
            static_cast<QtInstanceTreeIter*>(pIter)->m_aModelIndex = aSelectedIndexes.first();
    });
    return bHasSelection;
}

bool QtInstanceTreeView::get_cursor(weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_cursor(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_iter_first(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_next_sibling(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_previous_sibling(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
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

bool QtInstanceTreeView::iter_children(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_parent(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

int QtInstanceTreeView::get_iter_depth(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceTreeView::get_iter_index_in_parent(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return -1;
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

void QtInstanceTreeView::remove(const weld::TreeIter&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::select(const weld::TreeIter&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::unselect(const weld::TreeIter&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::set_extra_row_indent(const weld::TreeIter&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_text(const weld::TreeIter& rIter, const OUString& rStr, int nCol)
{
    assert(nCol != -1 && "Special column -1 not handled yet");

    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = modelIndex(rIter, nCol);
        m_pModel->setData(aIndex, toQString(rStr));
    });
}

void QtInstanceTreeView::set_sensitive(const weld::TreeIter&, bool, int)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_sensitive(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return false;
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

void QtInstanceTreeView::set_toggle(const weld::TreeIter&, TriState, int)
{
    assert(false && "Not implemented yet");
}

TriState QtInstanceTreeView::get_toggle(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return TRISTATE_INDET;
}

OUString QtInstanceTreeView::get_text(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceTreeView::set_id(const weld::TreeIter& rIter, const OUString& rId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QModelIndex aIndex = modelIndex(rIter);
        m_pModel->setData(aIndex, toQString(rId), ROLE_ID);
    });
}

OUString QtInstanceTreeView::get_id(const weld::TreeIter& rIter) const
{
    return get_id(modelIndex(rIter));
}

void QtInstanceTreeView::set_image(const weld::TreeIter& rIter, const OUString& rImage, int nCol)
{
    assert(nCol != -1 && "Special column -1 not handled yet");

    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (rImage.isEmpty())
            return;
        QModelIndex aIndex = modelIndex(rIter, nCol);
        QIcon aIcon = loadQPixmapIcon(rImage);
        m_pModel->setData(aIndex, aIcon, Qt::DecorationRole);
    });
}

void QtInstanceTreeView::set_image(const weld::TreeIter&, VirtualDevice&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_image(const weld::TreeIter&,
                                   const css::uno::Reference<css::graphic::XGraphic>&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_font_color(const weld::TreeIter&, const Color&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::scroll_to_row(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::is_selected(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
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
    int, const std::function<void(weld::TreeIter&, int nSourceIndex)>&, const weld::TreeIter*,
    const std::vector<int>*, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_row_expanded(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::expand_row(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::collapse_row(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
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

int QtInstanceTreeView::n_children() const
{
    SolarMutexGuard g;

    int nChildCount;
    GetQtInstance().RunInMainThread(
        [&] { nChildCount = m_pModel->rowCount(m_pModel->invisibleRootItem()->index()); });
    return nChildCount;
}

void QtInstanceTreeView::make_sorted()
{
    GetQtInstance().RunInMainThread([&] { m_pTreeView->setSortingEnabled(true); });
}

void QtInstanceTreeView::make_unsorted()
{
    GetQtInstance().RunInMainThread([&] { m_pTreeView->setSortingEnabled(false); });
}

bool QtInstanceTreeView::get_sort_order() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_sort_order(bool) { assert(false && "Not implemented yet"); }

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
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::set_sort_column(int) { assert(false && "Not implemented yet"); }

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

OUString QtInstanceTreeView::get_column_title(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceTreeView::set_column_title(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_selection_mode(SelectionMode eMode)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pTreeView->setSelectionMode(mapSelectionMode(eMode)); });
}

int QtInstanceTreeView::count_selected_rows() const
{
    assert(false && "Not implemented yet");
    return 0;
}

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

QModelIndex QtInstanceTreeView::modelIndex(int nRow, int nCol) const
{
    return m_pModel->index(nRow, nCol);
}

QModelIndex QtInstanceTreeView::modelIndex(const weld::TreeIter& rIter, int nCol) const
{
    QModelIndex aModelIndex = static_cast<const QtInstanceTreeIter&>(rIter).m_aModelIndex;
    return modelIndex(aModelIndex.row(), nCol);
}

OUString QtInstanceTreeView::get_id(const QModelIndex& rModelIndex) const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        QVariant aRoleData = m_pModel->data(rModelIndex, ROLE_ID);
        if (aRoleData.canConvert<QString>())
            sId = toOUString(aRoleData.toString());
    });

    return sId;
}

void QtInstanceTreeView::handleActivated()
{
    SolarMutexGuard g;
    signal_row_activated();
}

void QtInstanceTreeView::handleSelectionChanged()
{
    SolarMutexGuard g;
    signal_selection_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
