/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "AccessibilityTools.hxx"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <sal/log.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/timer.hxx>

using namespace css;
using namespace css::uno;
using namespace css::accessibility;

/* FIXME: mostly copied from sw/qa/extras/accessibility/accessible_relation_set.cxx */
css::uno::Reference<css::accessibility::XAccessibleContext>
AccessibilityTools::getAccessibleObjectForRole(
    const css::uno::Reference<css::accessibility::XAccessible>& xacc, sal_Int16 role)
{
    css::uno::Reference<css::accessibility::XAccessibleContext> ac = xacc->getAccessibleContext();
    bool isShowing = ac->getAccessibleStateSet()->contains(AccessibleStateType::SHOWING);

    if ((ac->getAccessibleRole() == role) && isShowing)
    {
        return ac;
    }
    else
    {
        int count = ac->getAccessibleChildCount();

        for (int i = 0; i < count && i < AccessibilityTools::MAX_CHILDREN; i++)
        {
            css::uno::Reference<css::accessibility::XAccessibleContext> ac2
                = AccessibilityTools::getAccessibleObjectForRole(ac->getAccessibleChild(i), role);
            if (ac2.is())
                return ac2;
        }
    }
    return nullptr;
}
/* FIXME: end copy */

bool AccessibilityTools::equals(const css::uno::Reference<css::accessibility::XAccessible>& xacc1,
                                const css::uno::Reference<css::accessibility::XAccessible>& xacc2)
{
    if (!xacc1.is() || !xacc2.is())
        return xacc1.is() == xacc2.is();
    return equals(xacc1->getAccessibleContext(), xacc2->getAccessibleContext());
}

bool AccessibilityTools::equals(
    const css::uno::Reference<css::accessibility::XAccessibleContext>& xctx1,
    const css::uno::Reference<css::accessibility::XAccessibleContext>& xctx2)
{
    if (!xctx1.is() || !xctx2.is())
        return xctx1.is() == xctx2.is();

    if (xctx1->getAccessibleRole() != xctx2->getAccessibleRole())
        return false;

    if (xctx1->getAccessibleName() != xctx2->getAccessibleName())
        return false;

    if (xctx1->getAccessibleDescription() != xctx2->getAccessibleDescription())
        return false;

    if (xctx1->getAccessibleChildCount() != xctx2->getAccessibleChildCount())
        return false;

    /* this one was not in the Java version */
    if (xctx1->getAccessibleIndexInParent() != xctx2->getAccessibleIndexInParent())
        return false;

    return equals(xctx1->getAccessibleParent(), xctx2->getAccessibleParent());
}

bool AccessibilityTools::equals(
    const css::uno::Reference<css::accessibility::XAccessibleStateSet>& xsts1,
    const css::uno::Reference<css::accessibility::XAccessibleStateSet>& xsts2)
{
    if (!xsts1.is() || !xsts2.is())
        return xsts1.is() == xsts2.is();
    return xsts1->getStates() == xsts2->getStates();
}

OUString AccessibilityTools::getRoleName(const sal_Int16 role)
{
    switch (role)
    {
        case AccessibleRole::UNKNOWN:
            return "UNKNOWN";
        case AccessibleRole::ALERT:
            return "ALERT";
        case AccessibleRole::BUTTON_DROPDOWN:
            return "BUTTON_DROPDOWN";
        case AccessibleRole::BUTTON_MENU:
            return "BUTTON_MENU";
        case AccessibleRole::CANVAS:
            return "CANVAS";
        case AccessibleRole::CAPTION:
            return "CAPTION";
        case AccessibleRole::CHART:
            return "CHART";
        case AccessibleRole::CHECK_BOX:
            return "CHECK_BOX";
        case AccessibleRole::CHECK_MENU_ITEM:
            return "CHECK_MENU_ITEM";
        case AccessibleRole::COLOR_CHOOSER:
            return "COLOR_CHOOSER";
        case AccessibleRole::COLUMN_HEADER:
            return "COLUMN_HEADER";
        case AccessibleRole::COMBO_BOX:
            return "COMBO_BOX";
        case AccessibleRole::COMMENT:
            return "COMMENT";
        case AccessibleRole::COMMENT_END:
            return "COMMENT_END";
        case AccessibleRole::DATE_EDITOR:
            return "DATE_EDITOR";
        case AccessibleRole::DESKTOP_ICON:
            return "DESKTOP_ICON";
        case AccessibleRole::DESKTOP_PANE:
            return "DESKTOP_PANE";
        case AccessibleRole::DIALOG:
            return "DIALOG";
        case AccessibleRole::DIRECTORY_PANE:
            return "DIRECTORY_PANE";
        case AccessibleRole::DOCUMENT:
            return "DOCUMENT";
        case AccessibleRole::DOCUMENT_PRESENTATION:
            return "DOCUMENT_PRESENTATION";
        case AccessibleRole::DOCUMENT_SPREADSHEET:
            return "DOCUMENT_SPREADSHEET";
        case AccessibleRole::DOCUMENT_TEXT:
            return "DOCUMENT_TEXT";
        case AccessibleRole::EDIT_BAR:
            return "EDIT_BAR";
        case AccessibleRole::EMBEDDED_OBJECT:
            return "EMBEDDED_OBJECT";
        case AccessibleRole::END_NOTE:
            return "END_NOTE";
        case AccessibleRole::FILE_CHOOSER:
            return "FILE_CHOOSER";
        case AccessibleRole::FILLER:
            return "FILLER";
        case AccessibleRole::FONT_CHOOSER:
            return "FONT_CHOOSER";
        case AccessibleRole::FOOTER:
            return "FOOTER";
        case AccessibleRole::FOOTNOTE:
            return "FOOTNOTE";
        case AccessibleRole::FORM:
            return "FORM";
        case AccessibleRole::FRAME:
            return "FRAME";
        case AccessibleRole::GLASS_PANE:
            return "GLASS_PANE";
        case AccessibleRole::GRAPHIC:
            return "GRAPHIC";
        case AccessibleRole::GROUP_BOX:
            return "GROUP_BOX";
        case AccessibleRole::HEADER:
            return "HEADER";
        case AccessibleRole::HEADING:
            return "HEADING";
        case AccessibleRole::HYPER_LINK:
            return "HYPER_LINK";
        case AccessibleRole::ICON:
            return "ICON";
        case AccessibleRole::IMAGE_MAP:
            return "IMAGE_MAP";
        case AccessibleRole::INTERNAL_FRAME:
            return "INTERNAL_FRAME";
        case AccessibleRole::LABEL:
            return "LABEL";
        case AccessibleRole::LAYERED_PANE:
            return "LAYERED_PANE";
        case AccessibleRole::LIST:
            return "LIST";
        case AccessibleRole::LIST_ITEM:
            return "LIST_ITEM";
        case AccessibleRole::MENU:
            return "MENU";
        case AccessibleRole::MENU_BAR:
            return "MENU_BAR";
        case AccessibleRole::MENU_ITEM:
            return "MENU_ITEM";
        case AccessibleRole::NOTE:
            return "NOTE";
        case AccessibleRole::OPTION_PANE:
            return "OPTION_PANE";
        case AccessibleRole::PAGE:
            return "PAGE";
        case AccessibleRole::PAGE_TAB:
            return "PAGE_TAB";
        case AccessibleRole::PAGE_TAB_LIST:
            return "PAGE_TAB_LIST";
        case AccessibleRole::PANEL:
            return "PANEL";
        case AccessibleRole::PARAGRAPH:
            return "PARAGRAPH";
        case AccessibleRole::PASSWORD_TEXT:
            return "PASSWORD_TEXT";
        case AccessibleRole::POPUP_MENU:
            return "POPUP_MENU";
        case AccessibleRole::PROGRESS_BAR:
            return "PROGRESS_BAR";
        case AccessibleRole::PUSH_BUTTON:
            return "PUSH_BUTTON";
        case AccessibleRole::RADIO_BUTTON:
            return "RADIO_BUTTON";
        case AccessibleRole::RADIO_MENU_ITEM:
            return "RADIO_MENU_ITEM";
        case AccessibleRole::ROOT_PANE:
            return "ROOT_PANE";
        case AccessibleRole::ROW_HEADER:
            return "ROW_HEADER";
        case AccessibleRole::RULER:
            return "RULER";
        case AccessibleRole::SCROLL_BAR:
            return "SCROLL_BAR";
        case AccessibleRole::SCROLL_PANE:
            return "SCROLL_PANE";
        case AccessibleRole::SECTION:
            return "SECTION";
        case AccessibleRole::SEPARATOR:
            return "SEPARATOR";
        case AccessibleRole::SHAPE:
            return "SHAPE";
        case AccessibleRole::SLIDER:
            return "SLIDER";
        case AccessibleRole::SPIN_BOX:
            return "SPIN_BOX";
        case AccessibleRole::SPLIT_PANE:
            return "SPLIT_PANE";
        case AccessibleRole::STATIC:
            return "STATIC";
        case AccessibleRole::STATUS_BAR:
            return "STATUS_BAR";
        case AccessibleRole::TABLE:
            return "TABLE";
        case AccessibleRole::TABLE_CELL:
            return "TABLE_CELL";
        case AccessibleRole::TEXT:
            return "TEXT";
        case AccessibleRole::TEXT_FRAME:
            return "TEXT_FRAME";
        case AccessibleRole::TOGGLE_BUTTON:
            return "TOGGLE_BUTTON";
        case AccessibleRole::TOOL_BAR:
            return "TOOL_BAR";
        case AccessibleRole::TOOL_TIP:
            return "TOOL_TIP";
        case AccessibleRole::TREE:
            return "TREE";
        case AccessibleRole::TREE_ITEM:
            return "TREE_ITEM";
        case AccessibleRole::TREE_TABLE:
            return "TREE_TABLE";
        case AccessibleRole::VIEW_PORT:
            return "VIEW_PORT";
        case AccessibleRole::WINDOW:
            return "WINDOW";
    };
    return "unknown";
}

OUString AccessibilityTools::getStateName(const sal_Int16 state)
{
    switch (state)
    {
        case AccessibleStateType::ACTIVE:
            return "ACTIVE";
        case AccessibleStateType::ARMED:
            return "ARMED";
        case AccessibleStateType::BUSY:
            return "BUSY";
        case AccessibleStateType::CHECKED:
            return "CHECKED";
        case AccessibleStateType::COLLAPSE:
            return "COLLAPSE";
        case AccessibleStateType::DEFAULT:
            return "DEFAULT";
        case AccessibleStateType::DEFUNC:
            return "DEFUNC";
        case AccessibleStateType::EDITABLE:
            return "EDITABLE";
        case AccessibleStateType::ENABLED:
            return "ENABLED";
        case AccessibleStateType::EXPANDABLE:
            return "EXPANDABLE";
        case AccessibleStateType::EXPANDED:
            return "EXPANDED";
        case AccessibleStateType::FOCUSABLE:
            return "FOCUSABLE";
        case AccessibleStateType::FOCUSED:
            return "FOCUSED";
        case AccessibleStateType::HORIZONTAL:
            return "HORIZONTAL";
        case AccessibleStateType::ICONIFIED:
            return "ICONIFIED";
        case AccessibleStateType::INDETERMINATE:
            return "INDETERMINATE";
        case AccessibleStateType::INVALID:
            return "INVALID";
        case AccessibleStateType::MANAGES_DESCENDANTS:
            return "MANAGES_DESCENDANTS";
        case AccessibleStateType::MODAL:
            return "MODAL";
        case AccessibleStateType::MOVEABLE:
            return "MOVEABLE";
        case AccessibleStateType::MULTI_LINE:
            return "MULTI_LINE";
        case AccessibleStateType::MULTI_SELECTABLE:
            return "MULTI_SELECTABLE";
        case AccessibleStateType::OFFSCREEN:
            return "OFFSCREEN";
        case AccessibleStateType::OPAQUE:
            return "OPAQUE";
        case AccessibleStateType::PRESSED:
            return "PRESSED";
        case AccessibleStateType::RESIZABLE:
            return "RESIZABLE";
        case AccessibleStateType::SELECTABLE:
            return "SELECTABLE";
        case AccessibleStateType::SELECTED:
            return "SELECTED";
        case AccessibleStateType::SENSITIVE:
            return "SENSITIVE";
        case AccessibleStateType::SHOWING:
            return "SHOWING";
        case AccessibleStateType::SINGLE_LINE:
            return "SINGLE_LINE";
        case AccessibleStateType::STALE:
            return "STALE";
        case AccessibleStateType::TRANSIENT:
            return "TRANSIENT";
        case AccessibleStateType::VERTICAL:
            return "VERTICAL";
        case AccessibleStateType::VISIBLE:
            return "VISIBLE";
    }
    return "unknown";
}

OUString AccessibilityTools::getEventIdName(const sal_Int16 event_id)
{
    switch (event_id)
    {
        case AccessibleEventId::ACTION_CHANGED:
            return "ACTION_CHANGED";
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            return "ACTIVE_DESCENDANT_CHANGED";
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS:
            return "ACTIVE_DESCENDANT_CHANGED_NOFOCUS";
        case AccessibleEventId::BOUNDRECT_CHANGED:
            return "BOUNDRECT_CHANGED";
        case AccessibleEventId::CARET_CHANGED:
            return "CARET_CHANGED";
        case AccessibleEventId::CHILD:
            return "CHILD";
        case AccessibleEventId::COLUMN_CHANGED:
            return "COLUMN_CHANGED";
        case AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
            return "CONTENT_FLOWS_FROM_RELATION_CHANGED";
        case AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
            return "CONTENT_FLOWS_TO_RELATION_CHANGED";
        case AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
            return "CONTROLLED_BY_RELATION_CHANGED";
        case AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
            return "CONTROLLER_FOR_RELATION_CHANGED";
        case AccessibleEventId::DESCRIPTION_CHANGED:
            return "DESCRIPTION_CHANGED";
        case AccessibleEventId::HYPERTEXT_CHANGED:
            return "HYPERTEXT_CHANGED";
        case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            return "INVALIDATE_ALL_CHILDREN";
        case AccessibleEventId::LABELED_BY_RELATION_CHANGED:
            return "LABELED_BY_RELATION_CHANGED";
        case AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
            return "LABEL_FOR_RELATION_CHANGED";
        case AccessibleEventId::LISTBOX_ENTRY_COLLAPSED:
            return "LISTBOX_ENTRY_COLLAPSED";
        case AccessibleEventId::LISTBOX_ENTRY_EXPANDED:
            return "LISTBOX_ENTRY_EXPANDED";
        case AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
            return "MEMBER_OF_RELATION_CHANGED";
        case AccessibleEventId::NAME_CHANGED:
            return "NAME_CHANGED";
        case AccessibleEventId::PAGE_CHANGED:
            return "PAGE_CHANGED";
        case AccessibleEventId::ROLE_CHANGED:
            return "ROLE_CHANGED";
        case AccessibleEventId::SECTION_CHANGED:
            return "SECTION_CHANGED";
        case AccessibleEventId::SELECTION_CHANGED:
            return "SELECTION_CHANGED";
        case AccessibleEventId::SELECTION_CHANGED_ADD:
            return "SELECTION_CHANGED_ADD";
        case AccessibleEventId::SELECTION_CHANGED_REMOVE:
            return "SELECTION_CHANGED_REMOVE";
        case AccessibleEventId::SELECTION_CHANGED_WITHIN:
            return "SELECTION_CHANGED_WITHIN";
        case AccessibleEventId::STATE_CHANGED:
            return "STATE_CHANGED";
        case AccessibleEventId::SUB_WINDOW_OF_RELATION_CHANGED:
            return "SUB_WINDOW_OF_RELATION_CHANGED";
        case AccessibleEventId::TABLE_CAPTION_CHANGED:
            return "TABLE_CAPTION_CHANGED";
        case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
            return "TABLE_COLUMN_DESCRIPTION_CHANGED";
        case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
            return "TABLE_COLUMN_HEADER_CHANGED";
        case AccessibleEventId::TABLE_MODEL_CHANGED:
            return "TABLE_MODEL_CHANGED";
        case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            return "TABLE_ROW_DESCRIPTION_CHANGED";
        case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            return "TABLE_ROW_HEADER_CHANGED";
        case AccessibleEventId::TABLE_SUMMARY_CHANGED:
            return "TABLE_SUMMARY_CHANGED";
        case AccessibleEventId::TEXT_ATTRIBUTE_CHANGED:
            return "TEXT_ATTRIBUTE_CHANGED";
        case AccessibleEventId::TEXT_CHANGED:
            return "TEXT_CHANGED";
        case AccessibleEventId::TEXT_SELECTION_CHANGED:
            return "TEXT_SELECTION_CHANGED";
        case AccessibleEventId::VALUE_CHANGED:
            return "VALUE_CHANGED";
        case AccessibleEventId::VISIBLE_DATA_CHANGED:
            return "VISIBLE_DATA_CHANGED";
    }
    return "unknown";
}

OUString AccessibilityTools::debugName(css::accessibility::XAccessibleContext* ctx)
{
    return "role=" + AccessibilityTools::getRoleName(ctx->getAccessibleRole()) + " name=\""
           + ctx->getAccessibleName() + "\" description=\"" + ctx->getAccessibleDescription()
           + "\"";
}

OUString AccessibilityTools::debugName(css::accessibility::XAccessible* acc)
{
    return debugName(acc->getAccessibleContext().get());
}

OUString AccessibilityTools::debugName(css::accessibility::XAccessibleStateSet* xsts)
{
    OUString name;

    for (auto state : xsts->getStates())
    {
        if (name.getLength())
            name += " | ";
        name += AccessibilityTools::getStateName(state);
    }

    return name;
}

OUString AccessibilityTools::debugName(const css::accessibility::AccessibleEventObject* evobj)
{
    return "(AccessibleEventObject) { id=" + getEventIdName(evobj->EventId)
           + " old=" + evobj->OldValue.getValueTypeName()
           + " new=" + evobj->NewValue.getValueTypeName() + " }";
}

bool AccessibilityTools::Await(const std::function<bool()>& cUntilCallback, sal_uInt64 nTimeoutMs)
{
    bool success = false;
    Timer aTimer("wait for event");
    aTimer.SetTimeout(nTimeoutMs);
    aTimer.Start();
    do
    {
        Scheduler::ProcessEventsToIdle();
        success = cUntilCallback();
    } while (!success && aTimer.IsActive());
    SAL_WARN_IF(!success, "test", "timeout reached");
    return success;
}

void AccessibilityTools::Wait(sal_uInt64 nTimeoutMs)
{
    Timer aTimer("wait for event");
    aTimer.SetTimeout(nTimeoutMs);
    aTimer.Start();
    std::cout << "waiting for " << nTimeoutMs << "ms... ";
    do
    {
        Scheduler::ProcessEventsToIdle();
    } while (aTimer.IsActive());
    std::cout << "ok." << std::endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
