/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceComboBox.hxx>
#include <QtInstanceComboBox.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceComboBox::QtInstanceComboBox(QComboBox* pComboBox)
    : QtInstanceWidget(pComboBox)
    , m_pComboBox(pComboBox)
    , m_bSorted(false)
{
    assert(pComboBox);

    QObject::connect(m_pComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                     &QtInstanceComboBox::handleCurrentIndexChanged);
}

void QtInstanceComboBox::insert(int nPos, const OUString& rStr, const OUString* pId,
                                const OUString* pIconName, VirtualDevice* pImageSurface)
{
    if (pId || pIconName || pImageSurface)
        assert(false && "Handling for these not implemented yet");

    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        m_pComboBox->insertItem(nPos, toQString(rStr));
        if (m_bSorted)
            sortItems();
    });
}

void QtInstanceComboBox::insert_vector(const std::vector<weld::ComboBoxEntry>&, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceComboBox::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceComboBox::get_count() const
{
    SolarMutexGuard g;
    int nCount;
    GetQtInstance().RunInMainThread([&] { nCount = m_pComboBox->count(); });
    return nCount;
}

void QtInstanceComboBox::make_sorted()
{
    SolarMutexGuard g;
    m_bSorted = true;
    GetQtInstance().RunInMainThread([&] { sortItems(); });
}

void QtInstanceComboBox::clear()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pComboBox->clear(); });
}

int QtInstanceComboBox::get_active() const
{
    SolarMutexGuard g;
    int nCurrentIndex;
    GetQtInstance().RunInMainThread([&] { nCurrentIndex = m_pComboBox->currentIndex(); });
    return nCurrentIndex;
}

void QtInstanceComboBox::set_active(int nPos)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pComboBox->setCurrentIndex(nPos); });
}

void QtInstanceComboBox::remove(int nPos)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pComboBox->removeItem(nPos); });
}

OUString QtInstanceComboBox::get_active_text() const
{
    SolarMutexGuard g;
    OUString sCurrentText;
    GetQtInstance().RunInMainThread([&] { sCurrentText = toOUString(m_pComboBox->currentText()); });
    return sCurrentText;
}

OUString QtInstanceComboBox::get_text(int nPos) const
{
    SolarMutexGuard g;
    OUString sItemText;
    GetQtInstance().RunInMainThread([&] { sItemText = toOUString(m_pComboBox->itemText(nPos)); });
    return sItemText;
}

int QtInstanceComboBox::find_text(const OUString& rStr) const
{
    SolarMutexGuard g;
    int nIndex;
    GetQtInstance().RunInMainThread([&] { nIndex = m_pComboBox->findText(toQString(rStr)); });
    return nIndex;
}

OUString QtInstanceComboBox::get_active_id() const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceComboBox::set_active_id(const OUString&) { assert(false && "Not implemented yet"); }

OUString QtInstanceComboBox::get_id(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceComboBox::set_id(int, const OUString&) { assert(false && "Not implemented yet"); }

int QtInstanceComboBox::find_id(const OUString&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

bool QtInstanceComboBox::changed_by_direct_pick() const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceComboBox::has_entry() const
{
    SolarMutexGuard g;
    bool bEditable;
    GetQtInstance().RunInMainThread([&] { bEditable = m_pComboBox->isEditable(); });
    return bEditable;
}

void QtInstanceComboBox::set_entry_message_type(weld::EntryMessageType)
{
    assert(false && "Not implemented yet");
}

void QtInstanceComboBox::set_entry_text(const OUString& rStr)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pComboBox->setEditText(toQString(rStr)); });
}

void QtInstanceComboBox::set_entry_width_chars(int) { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::set_entry_max_length(int) { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::select_entry_region(int, int) { assert(false && "Not implemented yet"); }

bool QtInstanceComboBox::get_entry_selection_bounds(int&, int&)
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceComboBox::set_entry_completion(bool, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceComboBox::set_entry_placeholder_text(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pComboBox->setEditText(toQString(rText)); });
}

void QtInstanceComboBox::set_entry_editable(bool bEditable)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pComboBox->setEditable(bEditable); });
}

void QtInstanceComboBox::cut_entry_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::copy_entry_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::paste_entry_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::set_font(const vcl::Font&) { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::set_entry_font(const vcl::Font&)
{
    assert(false && "Not implemented yet");
}

vcl::Font QtInstanceComboBox::get_entry_font()
{
    assert(false && "Not implemented yet");
    return vcl::Font();
}

bool QtInstanceComboBox::get_popup_shown() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceComboBox::set_custom_renderer(bool) { assert(false && "Not implemented yet"); }

VclPtr<VirtualDevice> QtInstanceComboBox::create_render_virtual_device() const
{
    assert(false && "Not implemented yet");
    return nullptr;
}

void QtInstanceComboBox::set_item_menu(const OUString&, weld::Menu*)
{
    assert(false && "Not implemented yet");
}

int QtInstanceComboBox::get_menu_button_width() const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceComboBox::get_max_mru_count() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceComboBox::set_max_mru_count(int) { assert(false && "Not implemented yet"); }

OUString QtInstanceComboBox::get_mru_entries() const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceComboBox::set_mru_entries(const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceComboBox::set_max_drop_down_rows(int) { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::sortItems() { m_pComboBox->model()->sort(0, Qt::AscendingOrder); }

void QtInstanceComboBox::handleCurrentIndexChanged() { signal_changed(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
