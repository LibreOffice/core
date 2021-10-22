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

#include <sal/log.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>

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
        case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            QAccessible::updateAccessibility(new QAccessibleEvent(
                pQAccessibleInterface, QAccessible::TableRowDescriptionChanged));
            return;
        case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::TableRowHeaderChanged));
            return;
        case AccessibleEventId::TABLE_SUMMARY_CHANGED:
        case AccessibleEventId::CARET_CHANGED:
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
        case AccessibleEventId::TEXT_CHANGED:
        case AccessibleEventId::COLUMN_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::TextColumnChanged));
            return;
        case AccessibleEventId::BOUNDRECT_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::LocationChanged));
            return;
        case AccessibleEventId::STATE_CHANGED:
            QAccessible::updateAccessibility(
                new QAccessibleEvent(pQAccessibleInterface, QAccessible::ForegroundChanged));
            return;
        case AccessibleEventId::ROLE_CHANGED:
        case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        case AccessibleEventId::VALUE_CHANGED:
        case AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
        case AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
        case AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
        case AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
        case AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
        case AccessibleEventId::LABELED_BY_RELATION_CHANGED:
        case AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
        case AccessibleEventId::SUB_WINDOW_OF_RELATION_CHANGED:
        case AccessibleEventId::HYPERTEXT_CHANGED:
        case AccessibleEventId::TABLE_MODEL_CHANGED:
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
