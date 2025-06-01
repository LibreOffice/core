/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceEntryTreeView.hxx>
#include <QtInstanceEntryTreeView.moc>

#include <QtWidgets/QCompleter>

QtInstanceEntryTreeView::QtInstanceEntryTreeView(QWidget* pContainer, QLineEdit* pLineEdit,
                                                 QTreeView* pTreeView,
                                                 std::unique_ptr<weld::Entry> xEntry,
                                                 std::unique_ptr<weld::TreeView> xTreeView)
    : weld::EntryTreeView(std::move(xEntry), std::move(xTreeView))
    , QtInstanceContainer(pContainer)
    , m_pLineEdit(pLineEdit)
    , m_pTreeView(pTreeView)
{
    set_entry_completion(true, true);

    connect(m_pLineEdit, &QLineEdit::textChanged, this, &QtInstanceEntryTreeView::editTextChanged);
    m_pLineEdit->installEventFilter(this);
}

void QtInstanceEntryTreeView::grab_focus() { m_xEntry->grab_focus(); }

void QtInstanceEntryTreeView::connect_focus_in(const Link<Widget&, void>& rLink)
{
    m_xEntry->connect_focus_in(rLink);
}

void QtInstanceEntryTreeView::connect_focus_out(const Link<Widget&, void>& rLink)
{
    m_xEntry->connect_focus_out(rLink);
}

void QtInstanceEntryTreeView::make_sorted() { m_xTreeView->make_sorted(); }

bool QtInstanceEntryTreeView::changed_by_direct_pick() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceEntryTreeView::set_entry_completion(bool bEnable, bool bCaseSensitive)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QCompleter* pCompleter = nullptr;
        if (bEnable)
        {
            pCompleter = new QCompleter(m_pTreeView->model(), m_pTreeView);
            pCompleter->setCompletionMode(QCompleter::InlineCompletion);
            pCompleter->setFilterMode(Qt::MatchStartsWith);
            Qt::CaseSensitivity eCaseSensitivity
                = bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
            pCompleter->setCaseSensitivity(eCaseSensitivity);
        }

        m_pLineEdit->setCompleter(pCompleter);
    });
}

VclPtr<VirtualDevice> QtInstanceEntryTreeView::create_render_virtual_device() const
{
    return create_virtual_device();
}

bool QtInstanceEntryTreeView::eventFilter(QObject* pObject, QEvent* pEvent)
{
    if (pObject != m_pLineEdit)
        return false;

    switch (pEvent->type())
    {
        case QEvent::KeyPress:
        {
            QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);

            // don't intercept key event handling if modifiers (other than key pad) are used
            if (pKeyEvent->modifiers() & ~Qt::KeypadModifier)
                return false;

            switch (pKeyEvent->key())
            {
                case Qt::Key_Down:
                case Qt::Key_PageDown:
                case Qt::Key_PageUp:
                case Qt::Key_Up:
                    // forward key events for navigation through entries to the tree view
                    return QCoreApplication::sendEvent(m_pTreeView, pEvent);
                default:
                    return false;
            }
        }
        default:
            return false;
    }
}

void QtInstanceEntryTreeView::editTextChanged(const QString& rText)
{
    const int nIndex = m_xTreeView->find_text(toOUString(rText));
    if (nIndex < 0)
        return;

    const QModelIndex aModelIndex = m_pTreeView->model()->index(nIndex, 0);
    m_pTreeView->selectionModel()->setCurrentIndex(aModelIndex,
                                                   QItemSelectionModel::ClearAndSelect);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
