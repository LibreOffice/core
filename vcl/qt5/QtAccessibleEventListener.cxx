/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <QtAccessibleEventListener.hxx>
#include <QtTools.hxx>

#include <sal/log.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <QtGui/QAccessible>

using namespace css;
using namespace css::accessibility;
using namespace css::lang;
using namespace css::uno;

QtAccessibleEventListener::QtAccessibleEventListener(const Reference<XAccessible> xAccessible,
                                                     QtAccessibleWidget* pAccessibleWidget)
    : m_xAccessible(xAccessible)
    , m_pAccessibleWidget(pAccessibleWidget)
{
}

void QtAccessibleEventListener::HandleStateChangedEvent(
    QAccessibleInterface* pQAccessibleInterface,
    const css::accessibility::AccessibleEventObject& rEvent)
{
    QAccessible::State aState;

    short nState = 0;
    rEvent.NewValue >>= nState;
    // States in 'QAccessibleStateChangeEvent' indicate what states have changed, so if e.g.
    // an object loses focus (not just if it gains it), 'focus' state needs to be set to 'true',
    // so retrieve the old/previous value from the event if necessary.
    if (nState == AccessibleStateType::INVALID)
        rEvent.OldValue >>= nState;

    switch (nState)
    {
        case AccessibleStateType::ACTIVE:
            // ignore for now, since it somehow causes Orca to become unresponsive quite quickly
            // TODO: analyze further and fix root cause
            /*
            aState.active = true;
            break;
            */
            return;
        case AccessibleStateType::BUSY:
            aState.busy = true;
            break;
        case AccessibleStateType::CHECKED:
            aState.checked = true;
            break;
        case AccessibleStateType::COLLAPSE:
            aState.collapsed = true;
            break;
        case AccessibleStateType::DEFAULT:
            aState.defaultButton = true;
            break;
        case AccessibleStateType::ENABLED:
            aState.disabled = true;
            break;
        case AccessibleStateType::EDITABLE:
            aState.editable = true;
            break;
        case AccessibleStateType::EXPANDABLE:
            aState.expandable = true;
            break;
        case AccessibleStateType::EXPANDED:
            aState.expanded = true;
            break;
        case AccessibleStateType::FOCUSABLE:
            aState.focusable = true;
            break;
        case AccessibleStateType::FOCUSED:
            aState.focused = true;
            break;
        case AccessibleStateType::INVALID:
            aState.invalid = true;
            break;
        case AccessibleStateType::VISIBLE:
            aState.invisible = true;
            break;
        case AccessibleStateType::MODAL:
            aState.modal = true;
            break;
        case AccessibleStateType::MOVEABLE:
            aState.movable = true;
            break;
        case AccessibleStateType::MULTI_LINE:
        // comment in Qt's qaccessible.h has this:
        // "// quint64 singleLine : 1; // we have multi line, this is redundant."
        case AccessibleStateType::SINGLE_LINE:
            aState.multiLine = true;
            break;
        case AccessibleStateType::MULTI_SELECTABLE:
            aState.multiSelectable = true;
            break;
        case AccessibleStateType::OFFSCREEN:
            aState.offscreen = true;
            break;
        case AccessibleStateType::PRESSED:
            aState.pressed = true;
            break;
        case AccessibleStateType::RESIZABLE:
            aState.sizeable = true;
            break;
        case AccessibleStateType::SELECTABLE:
            aState.selectable = true;
            break;
        case AccessibleStateType::SELECTED:
            aState.selected = true;
            break;
        // These don't seem to have a matching Qt equivalent
        case AccessibleStateType::ARMED:
        case AccessibleStateType::DEFUNC:
        case AccessibleStateType::HORIZONTAL:
        case AccessibleStateType::ICONIFIED:
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::MANAGES_DESCENDANTS:
        case AccessibleStateType::OPAQUE:
        case AccessibleStateType::SENSITIVE:
        case AccessibleStateType::SHOWING:
        case AccessibleStateType::STALE:
        case AccessibleStateType::TRANSIENT:
        case AccessibleStateType::VERTICAL:
        default:
            return;
    }

    QAccessible::updateAccessibility(
        new QAccessibleStateChangeEvent(pQAccessibleInterface, aState));
}

void QtAccessibleEventListener::notifyEvent(const css::accessibility::AccessibleEventObject& aEvent)
{
    QAccessibleInterface* pQAccessibleInterface = m_pAccessibleWidget;

    Reference<XAccessible> xChild;
    switch (aEvent.EventId)
    {
        case AccessibleEventId::NAME_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::NameChanged));
            return;
        case AccessibleEventId::DESCRIPTION_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::DescriptionChanged));
            return;
        case AccessibleEventId::ACTION_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::ActionChanged));
            return;
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::ActiveDescendantChanged));
            return;
        case AccessibleEventId::CARET_CHANGED:
        {
            sal_Int32 nNewCursorPos = 0;
            aEvent.NewValue >>= nNewCursorPos;
            QAccessible::updateAccessibility(
                new QAccessibleTextCursorEvent(pQAccessibleInterface, nNewCursorPos));
            return;
        }
        case AccessibleEventId::CHILD:
        {
            QAccessible::Event event = QAccessible::InvalidEvent;
            if (aEvent.OldValue >>= xChild)
                event = QAccessible::ObjectDestroyed;
            if (aEvent.NewValue >>= xChild)
                event = QAccessible::ObjectCreated;
            if (event != QAccessible::InvalidEvent)
                QAccessible::updateAccessibility(
                    new QAccessibleEvent(pQAccessibleInterface, event));
            return;
        }
        case AccessibleEventId::HYPERTEXT_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::HypertextChanged));
            return;
        case AccessibleEventId::SELECTION_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::Selection));
            return;
        case AccessibleEventId::VISIBLE_DATA_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::VisibleDataChanged));
            return;
        case AccessibleEventId::TEXT_SELECTION_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::Selection));
            return;
        case AccessibleEventId::TEXT_ATTRIBUTE_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::AttributeChanged));
            return;
        case AccessibleEventId::TEXT_CHANGED:
        {
            TextSegment aDeletedText;
            TextSegment aInsertedText;
            if (aEvent.OldValue >>= aDeletedText)
            {
                QAccessible::updateAccessibility(
                    new QAccessibleTextRemoveEvent(pQAccessibleInterface, aDeletedText.SegmentStart,
                                                   toQString(aDeletedText.SegmentText)));
            }
            if (aEvent.NewValue >>= aInsertedText)
            {
                QAccessible::updateAccessibility(new QAccessibleTextInsertEvent(
                    pQAccessibleInterface, aInsertedText.SegmentStart,
                    toQString(aInsertedText.SegmentText)));
            }
            return;
        }
        case AccessibleEventId::TABLE_CAPTION_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::TableCaptionChanged));
            return;
        case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
            QAccessible::updateAccessibility(new QAccessibleEvent(
                pQAccessibleInterface, QAccessible::TableColumnDescriptionChanged));
            return;
        case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::TableColumnHeaderChanged));
            return;
        case AccessibleEventId::TABLE_MODEL_CHANGED:
        {
            AccessibleTableModelChange aChange;
            aEvent.NewValue >>= aChange;

            QAccessibleTableModelChangeEvent::ModelChangeType nType;
            switch (aChange.Type)
            {
                case AccessibleTableModelChangeType::COLUMNS_INSERTED:
                    nType = QAccessibleTableModelChangeEvent::ColumnsInserted;
                    break;
                case AccessibleTableModelChangeType::COLUMNS_REMOVED:
                    nType = QAccessibleTableModelChangeEvent::ColumnsRemoved;
                    break;
                case AccessibleTableModelChangeType::ROWS_INSERTED:
                    nType = QAccessibleTableModelChangeEvent::RowsInserted;
                    break;
                case AccessibleTableModelChangeType::ROWS_REMOVED:
                    nType = QAccessibleTableModelChangeEvent::RowsRemoved;
                    break;
                case AccessibleTableModelChangeType::UPDATE:
                    nType = QAccessibleTableModelChangeEvent::DataChanged;
                    break;
                default:
                    assert(false && "Unhandled AccessibleTableModelChangeType");
                    return;
            }
            QAccessibleTableModelChangeEvent* pTableEvent
                = new QAccessibleTableModelChangeEvent(pQAccessibleInterface, nType);
            pTableEvent->setFirstRow(aChange.FirstRow);
            pTableEvent->setLastRow(aChange.LastRow);
            pTableEvent->setFirstColumn(aChange.FirstColumn);
            pTableEvent->setLastColumn(aChange.LastColumn);
            QAccessible::updateAccessibility(pTableEvent);
            return;
        }
        case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            QAccessible::updateAccessibility(new QAccessibleEvent(
                pQAccessibleInterface, QAccessible::TableRowDescriptionChanged));
            return;
        case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::TableRowHeaderChanged));
            return;
        case AccessibleEventId::TABLE_SUMMARY_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::TableSummaryChanged));
            return;
        case AccessibleEventId::SELECTION_CHANGED_ADD:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::SelectionAdd));
            return;
        case AccessibleEventId::SELECTION_CHANGED_REMOVE:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::SelectionRemove));
            return;
        case AccessibleEventId::SELECTION_CHANGED_WITHIN:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::SelectionWithin));
            return;
        case AccessibleEventId::PAGE_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::PageChanged));
            return;
        case AccessibleEventId::SECTION_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::SectionChanged));
            return;
        case AccessibleEventId::COLUMN_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::TextColumnChanged));
            return;
        case AccessibleEventId::BOUNDRECT_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::LocationChanged));
            return;
        case AccessibleEventId::STATE_CHANGED:
            HandleStateChangedEvent(pQAccessibleInterface, aEvent);
            return;
        case AccessibleEventId::VALUE_CHANGED:
        {
            QAccessibleValueInterface* pValueInterface = pQAccessibleInterface->valueInterface();
            if (pValueInterface)
            {
                const QVariant aValue = pValueInterface->currentValue();
                QAccessible::updateAccessibility(
                    new QAccessibleValueChangeEvent(pQAccessibleInterface, aValue));
            }
            return;
        }
        case AccessibleEventId::ROLE_CHANGED:
        case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        case AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
        case AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
        case AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
        case AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
        case AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
        case AccessibleEventId::LABELED_BY_RELATION_CHANGED:
        case AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
        case AccessibleEventId::SUB_WINDOW_OF_RELATION_CHANGED:
        case AccessibleEventId::LISTBOX_ENTRY_EXPANDED:
        case AccessibleEventId::LISTBOX_ENTRY_COLLAPSED:
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS:
        default:
            SAL_WARN("vcl.qt", "Unmapped AccessibleEventId: " << aEvent.EventId);
            return;
    }
}

void QtAccessibleEventListener::disposing(const EventObject& /* Source */) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
