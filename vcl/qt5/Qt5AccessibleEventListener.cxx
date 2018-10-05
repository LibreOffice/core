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

#include <Qt5AccessibleEventListener.hxx>

#include <sal/log.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>

using namespace css;
using namespace css::accessibility;
using namespace css::lang;
using namespace css::uno;

Qt5AccessibleEventListener::Qt5AccessibleEventListener(const Reference<XAccessible> xAccessible)
    : m_xAccessible(xAccessible)
{
}

void Qt5AccessibleEventListener::notifyEvent(
    const css::accessibility::AccessibleEventObject& aEvent)
{
    SAL_DEBUG("received notify event!!11!!");
    switch (aEvent.EventId)
    {
        case AccessibleEventId::NAME_CHANGED:
            break;
        case AccessibleEventId::DESCRIPTION_CHANGED:
            break;
        case AccessibleEventId::ACTION_CHANGED:
            break;
        case AccessibleEventId::STATE_CHANGED:
            break;
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            break;
        case AccessibleEventId::BOUNDRECT_CHANGED:
            break;
        case AccessibleEventId::CHILD:
            break;
        case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            break;
        case AccessibleEventId::SELECTION_CHANGED:
            break;
        case AccessibleEventId::VISIBLE_DATA_CHANGED:
            break;
        case AccessibleEventId::VALUE_CHANGED:
            break;
        case AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
            break;
        case AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
            break;
        case AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
            break;
        case AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
            break;
        case AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
            break;
        case AccessibleEventId::LABELED_BY_RELATION_CHANGED:
            break;
        case AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
            break;
        case AccessibleEventId::SUB_WINDOW_OF_RELATION_CHANGED:
            break;
        case AccessibleEventId::CARET_CHANGED:
            break;
        case AccessibleEventId::TEXT_SELECTION_CHANGED:
            break;
        case AccessibleEventId::TEXT_CHANGED:
            break;
        case AccessibleEventId::TEXT_ATTRIBUTE_CHANGED:
            break;
        case AccessibleEventId::HYPERTEXT_CHANGED:
            break;
        case AccessibleEventId::TABLE_CAPTION_CHANGED:
            break;
        case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
            break;
        case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
            break;
        case AccessibleEventId::TABLE_MODEL_CHANGED:
            break;
        case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            break;
        case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            break;
        case AccessibleEventId::TABLE_SUMMARY_CHANGED:
            break;
        case AccessibleEventId::LISTBOX_ENTRY_EXPANDED:
            break;
        case AccessibleEventId::LISTBOX_ENTRY_COLLAPSED:
            break;
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS:
            break;
        case AccessibleEventId::SELECTION_CHANGED_ADD:
            break;
        case AccessibleEventId::SELECTION_CHANGED_REMOVE:
            break;
        case AccessibleEventId::SELECTION_CHANGED_WITHIN:
            break;
        case AccessibleEventId::PAGE_CHANGED:
            break;
        case AccessibleEventId::SECTION_CHANGED:
            break;
        case AccessibleEventId::COLUMN_CHANGED:
            break;
        case AccessibleEventId::ROLE_CHANGED:
            break;
        default:
            SAL_WARN("vcl.qt5", "Unmapped AccessibleEventId: " << aEvent.EventId);
            break;
    }
}

void Qt5AccessibleEventListener::disposing(const EventObject& /* Source */) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
