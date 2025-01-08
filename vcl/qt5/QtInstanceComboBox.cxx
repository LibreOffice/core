/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceComboBox.hxx>
#include <QtInstanceComboBox.moc>

#include <QtInstanceEntry.hxx>

#include <vcl/qt/QtUtils.hxx>

#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>

QtInstanceComboBox::QtInstanceComboBox(QComboBox* pComboBox)
    : QtInstanceWidget(pComboBox)
    , m_pComboBox(pComboBox)
    , m_bSorted(false)
{
    assert(pComboBox);

    QObject::connect(m_pComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                     &QtInstanceComboBox::signalChanged);
    QObject::connect(m_pComboBox, &QComboBox::editTextChanged, this,
                     &QtInstanceComboBox::signalChanged);
}

void QtInstanceComboBox::insert(int nPos, const OUString& rStr, const OUString* pId,
                                const OUString* pIconName, VirtualDevice* pImageSurface)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QVariant aUserData;
        if (pId)
            aUserData = QVariant::fromValue(toQString(*pId));

        m_pComboBox->insertItem(nPos, toQString(rStr), aUserData);

        if (pIconName)
            m_pComboBox->setItemIcon(nPos, loadQPixmapIcon(*pIconName));
        else if (pImageSurface)
            m_pComboBox->setItemIcon(nPos, toQPixmap(*pImageSurface));

        if (m_bSorted)
            sortItems();
    });
}

void QtInstanceComboBox::insert_vector(const std::vector<weld::ComboBoxEntry>& rItems,
                                       bool bKeepExisting)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        if (!bKeepExisting)
            m_pComboBox->clear();

        // if sorted, only sort once at the end
        const bool bSorted = m_bSorted;
        m_bSorted = false;

        for (const weld::ComboBoxEntry& rEntry : rItems)
        {
            const OUString* pId = rEntry.sId.isEmpty() ? nullptr : &rEntry.sId;
            const OUString* pImage = rEntry.sImage.isEmpty() ? nullptr : &rEntry.sImage;
            insert(m_pComboBox->count(), rEntry.sString, pId, pImage, nullptr);
        }

        m_bSorted = bSorted;
        if (m_bSorted)
            sortItems();
    });
}

void QtInstanceComboBox::insert_separator(int nPos, const OUString&)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        if (nPos == -1)
            nPos = m_pComboBox->count();
        m_pComboBox->insertSeparator(nPos);
    });
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
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        QVariant aUserData = m_pComboBox->currentData();
        if (aUserData.canConvert<QString>())
            sId = toOUString(aUserData.toString());
    });

    return sId;
}

void QtInstanceComboBox::set_active_id(const OUString& rId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        int nIndex = find_id(rId);
        set_active(nIndex);
    });
}

OUString QtInstanceComboBox::get_id(int nPos) const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        QVariant aUserData = m_pComboBox->itemData(nPos);
        if (aUserData.canConvert<QString>())
            sId = toOUString(aUserData.toString());
    });

    return sId;
}

void QtInstanceComboBox::set_id(int nRow, const OUString& rId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QVariant aUserData = QVariant::fromValue(toQString(rId));
        m_pComboBox->setItemData(nRow, aUserData);
    });
}

int QtInstanceComboBox::find_id(const OUString& rId) const
{
    SolarMutexGuard g;

    int nIndex;
    GetQtInstance().RunInMainThread([&] {
        QVariant aUserData = toQString(rId);
        nIndex = m_pComboBox->findData(aUserData);
    });

    return nIndex;
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

void QtInstanceComboBox::set_entry_message_type(weld::EntryMessageType eType)
{
    QtInstanceEntry::setMessageType(*m_pComboBox->lineEdit(), eType);
}

void QtInstanceComboBox::set_entry_text(const OUString& rStr)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pComboBox->setEditText(toQString(rStr)); });
}

void QtInstanceComboBox::set_entry_width_chars(int) { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::set_entry_max_length(int) { assert(false && "Not implemented yet"); }

void QtInstanceComboBox::select_entry_region(int nStartPos, int nEndPos)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QLineEdit* pEdit = m_pComboBox->lineEdit();
        if (pEdit)
        {
            if (nEndPos == -1)
                nEndPos = pEdit->text().length();

            const int nLength = nEndPos - nStartPos;
            pEdit->setSelection(nStartPos, nLength);
        }
    });
}

bool QtInstanceComboBox::get_entry_selection_bounds(int& rStartPos, int& rEndPos)
{
    SolarMutexGuard g;

    bool bHasSelection = false;
    GetQtInstance().RunInMainThread([&] {
        QLineEdit* pEdit = m_pComboBox->lineEdit();
        if (pEdit)
        {
            bHasSelection = pEdit->hasSelectedText();
            rStartPos = pEdit->selectionStart();
            rEndPos = pEdit->selectionEnd();
        }
    });

    return bHasSelection;
}

void QtInstanceComboBox::set_entry_completion(bool bEnable, bool bCaseSensitive)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QCompleter* pCompleter = nullptr;
        if (bEnable)
        {
            pCompleter = new QCompleter(m_pComboBox->model(), m_pComboBox);
            pCompleter->setCompletionMode(QCompleter::InlineCompletion);
            Qt::CaseSensitivity eCaseSensitivity
                = bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
            pCompleter->setCaseSensitivity(eCaseSensitivity);
        }

        m_pComboBox->setCompleter(pCompleter);
    });
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

void QtInstanceComboBox::signalChanged()
{
    SolarMutexGuard g;
    signal_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
