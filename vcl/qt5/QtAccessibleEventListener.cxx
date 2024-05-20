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
#include <QtAccessibleRegistry.hxx>
#include <QtTools.hxx>

#include <sal/log.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <QtGui/QAccessible>
#include <QtGui/QAccessibleTextSelectionEvent>

using namespace css;
using namespace css::accessibility;
using namespace css::lang;
using namespace css::uno;

QtAccessibleEventListener::QtAccessibleEventListener(QtAccessibleWidget* pAccessibleWidget)
    : m_pAccessibleWidget(pAccessibleWidget)
{
}

void QtAccessibleEventListener::HandleStateChangedEvent(
    QAccessibleInterface* pQAccessibleInterface,
    const css::accessibility::AccessibleEventObject& rEvent)
{
    QAccessible::State aState;

    sal_Int64 nState = 0;
    rEvent.NewValue >>= nState;
    // States in 'QAccessibleStateChangeEvent' indicate what states have changed, so if e.g.
    // an object loses focus (not just if it gains it), 'focus' state needs to be set to 'true',
    // so retrieve the old/previous value from the event if necessary.
    if (nState == AccessibleStateType::INVALID)
        rEvent.OldValue >>= nState;

    switch (nState)
    {
        case AccessibleStateType::ACTIVE:
            aState.active = true;
            break;
        case AccessibleStateType::BUSY:
            aState.busy = true;
            break;
        case AccessibleStateType::CHECKABLE:
            aState.checkable = true;
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
        case AccessibleStateType::SHOWING:
        {
            // Qt does not have an equivalent for the SHOWING state,
            // but has separate event types
            QAccessible::Event eEventType;
            sal_Int64 nNewState = 0;
            if ((rEvent.NewValue >>= nNewState) && nNewState == AccessibleStateType::SHOWING)
                eEventType = QAccessible::ObjectShow;
            else
                eEventType = QAccessible::ObjectHide;
            QAccessibleEvent aEvent(pQAccessibleInterface, eEventType);
            QAccessible::updateAccessibility(&aEvent);
            break;
        }
        // These don't seem to have a matching Qt equivalent
        case AccessibleStateType::ARMED:
        case AccessibleStateType::DEFUNC:
        case AccessibleStateType::HORIZONTAL:
        case AccessibleStateType::ICONIFIED:
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::MANAGES_DESCENDANTS:
        case AccessibleStateType::OPAQUE:
        case AccessibleStateType::SENSITIVE:
        case AccessibleStateType::STALE:
        case AccessibleStateType::TRANSIENT:
        case AccessibleStateType::VERTICAL:
        default:
            return;
    }

    QAccessibleStateChangeEvent aEvent(pQAccessibleInterface, aState);
    QAccessible::updateAccessibility(&aEvent);
}

void QtAccessibleEventListener::notifyEvent(const css::accessibility::AccessibleEventObject& rEvent)
{
    QAccessibleInterface* pQAccessibleInterface = m_pAccessibleWidget;

    switch (rEvent.EventId)
    {
        case AccessibleEventId::NAME_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::NameChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::DESCRIPTION_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::DescriptionChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::ACTION_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::ActionChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
        {
            // Qt has a QAccessible::ActiveDescendantChanged event type, but events of
            // that type are currently just ignored on Qt side and not forwarded to AT-SPI.
            // Send a state change event for the focused state of the newly
            // active descendant instead
            uno::Reference<accessibility::XAccessible> xActiveAccessible;
            rEvent.NewValue >>= xActiveAccessible;
            if (!xActiveAccessible.is())
                return;

            QObject* pQtAcc = QtAccessibleRegistry::getQObject(xActiveAccessible);
            QAccessibleInterface* pInterface = QAccessible::queryAccessibleInterface(pQtAcc);
            QAccessible::State aState;
            aState.focused = true;
            QAccessibleStateChangeEvent aEvent(pInterface, aState);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::CARET_CHANGED:
        {
            sal_Int32 nNewCursorPos = 0;
            rEvent.NewValue >>= nNewCursorPos;
            QAccessibleTextCursorEvent aEvent(pQAccessibleInterface, nNewCursorPos);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::CHILD:
        {
            Reference<XAccessible> xChild;
            if (rEvent.NewValue >>= xChild)
            {
                assert(xChild.is()
                       && "AccessibleEventId::CHILD event NewValue without valid child set");
                // tdf#159213 for now, workaround invalid events being sent and don't crash in release builds
                if (!xChild.is())
                    return;
                QAccessibleEvent aEvent(QtAccessibleRegistry::getQObject(xChild),
                                        QAccessible::ObjectCreated);
                QAccessible::updateAccessibility(&aEvent);
                return;
            }
            if (rEvent.OldValue >>= xChild)
            {
                assert(xChild.is()
                       && "AccessibleEventId::CHILD event OldValue without valid child set");
                // tdf#159213 for now, workaround invalid events being sent and don't crash in release builds
                if (!xChild.is())
                    return;
                QAccessibleEvent aEvent(QtAccessibleRegistry::getQObject(xChild),
                                        QAccessible::ObjectDestroyed);
                QAccessible::updateAccessibility(&aEvent);
                return;
            }
            SAL_WARN("vcl.qt",
                     "Ignoring invalid AccessibleEventId::CHILD event without any child set.");
            return;
        }
        case AccessibleEventId::HYPERTEXT_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::HypertextChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::SELECTION_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::Selection);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::VISIBLE_DATA_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::VisibleDataChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TEXT_SELECTION_CHANGED:
        {
            QAccessibleTextInterface* pTextInterface = pQAccessibleInterface->textInterface();
            if (!pTextInterface)
            {
                SAL_WARN("vcl.qt", "TEXT_SELECTION_CHANGED event received for object not "
                                   "implementing text interface");
                return;
            }
            int nStartOffset = 0;
            int nEndOffset = 0;
            pTextInterface->selection(0, &nStartOffset, &nEndOffset);
            QAccessibleTextSelectionEvent aEvent(pQAccessibleInterface, nStartOffset, nEndOffset);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TEXT_ATTRIBUTE_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::AttributeChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TEXT_CHANGED:
        {
            TextSegment aDeletedText;
            TextSegment aInsertedText;
            if (rEvent.OldValue >>= aDeletedText)
            {
                QAccessibleTextRemoveEvent aEvent(pQAccessibleInterface, aDeletedText.SegmentStart,
                                                  toQString(aDeletedText.SegmentText));
                QAccessible::updateAccessibility(&aEvent);
            }
            if (rEvent.NewValue >>= aInsertedText)
            {
                QAccessibleTextInsertEvent aEvent(pQAccessibleInterface, aInsertedText.SegmentStart,
                                                  toQString(aInsertedText.SegmentText));
                QAccessible::updateAccessibility(&aEvent);
            }
            return;
        }
        case AccessibleEventId::TABLE_CAPTION_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::TableCaptionChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface,
                                    QAccessible::TableColumnDescriptionChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::TableColumnHeaderChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TABLE_MODEL_CHANGED:
        {
            AccessibleTableModelChange aChange;
            rEvent.NewValue >>= aChange;

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
            QAccessibleTableModelChangeEvent aTableEvent(pQAccessibleInterface, nType);
            aTableEvent.setFirstRow(aChange.FirstRow);
            aTableEvent.setLastRow(aChange.LastRow);
            aTableEvent.setFirstColumn(aChange.FirstColumn);
            aTableEvent.setLastColumn(aChange.LastColumn);
            QAccessible::updateAccessibility(&aTableEvent);
            return;
        }
        case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::TableRowDescriptionChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::TableRowHeaderChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::TABLE_SUMMARY_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::TableSummaryChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::SELECTION_CHANGED_ADD:
        case AccessibleEventId::SELECTION_CHANGED_REMOVE:
        {
            QAccessible::Event eEventType;
            if (rEvent.EventId == AccessibleEventId::SELECTION_CHANGED_ADD)
                eEventType = QAccessible::SelectionAdd;
            else
                eEventType = QAccessible::SelectionRemove;

            uno::Reference<accessibility::XAccessible> xChildAcc;
            rEvent.NewValue >>= xChildAcc;
            if (!xChildAcc.is())
            {
                SAL_WARN("vcl.qt",
                         "Selection add/remove event without the (un)selected accessible set");
                return;
            }
            Reference<XAccessibleContext> xContext = xChildAcc->getAccessibleContext();
            if (!xContext.is())
            {
                SAL_WARN("vcl.qt", "No valid XAccessibleContext for (un)selected accessible.");
                return;
            }

            // Qt expects the event to be sent for the (un)selected child
            QObject* pChildObject = QtAccessibleRegistry::getQObject(xChildAcc);
            assert(pChildObject);
            QAccessibleEvent aEvent(pChildObject, eEventType);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::SELECTION_CHANGED_WITHIN:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::SelectionWithin);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::PAGE_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::PageChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::SECTION_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::SectionChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::COLUMN_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::TextColumnChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::BOUNDRECT_CHANGED:
        {
            QAccessibleEvent aEvent(pQAccessibleInterface, QAccessible::LocationChanged);
            QAccessible::updateAccessibility(&aEvent);
            return;
        }
        case AccessibleEventId::STATE_CHANGED:
            HandleStateChangedEvent(pQAccessibleInterface, rEvent);
            return;
        case AccessibleEventId::VALUE_CHANGED:
        {
            QAccessibleValueInterface* pValueInterface = pQAccessibleInterface->valueInterface();
            if (pValueInterface)
            {
                const QVariant aValue = pValueInterface->currentValue();
                QAccessibleValueChangeEvent aEvent(pQAccessibleInterface, aValue);
                QAccessible::updateAccessibility(&aEvent);
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
            SAL_WARN("vcl.qt", "Unmapped AccessibleEventId: " << rEvent.EventId);
            return;
    }
}

void QtAccessibleEventListener::disposing(const EventObject& /* Source */)
{
    assert(m_pAccessibleWidget);
    m_pAccessibleWidget->invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
