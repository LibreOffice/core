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

#include <test/a11y/AccessibilityTools.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <sal/log.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/timer.hxx>

using namespace css;

css::uno::Reference<css::accessibility::XAccessibleContext>
AccessibilityTools::getAccessibleObjectForRole(
    const css::uno::Reference<css::accessibility::XAccessible>& xacc, sal_Int16 role)
{
    css::uno::Reference<css::accessibility::XAccessibleContext> ac = xacc->getAccessibleContext();
    bool isShowing = ac->getAccessibleStateSet() & accessibility::AccessibleStateType::SHOWING;

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

bool AccessibilityTools::equals(const uno::Reference<accessibility::XAccessible>& xacc1,
                                const uno::Reference<accessibility::XAccessible>& xacc2)
{
    if (!xacc1.is() || !xacc2.is())
        return xacc1.is() == xacc2.is();
    return equals(xacc1->getAccessibleContext(), xacc2->getAccessibleContext());
}

bool AccessibilityTools::equals(const uno::Reference<accessibility::XAccessibleContext>& xctx1,
                                const uno::Reference<accessibility::XAccessibleContext>& xctx2)
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

    /* because in Writer at least some children only are referenced by their relations to others
     * objects, we need to account for that as their index in parent is incorrect (so not
     * necessarily unique) */
    auto relset1 = xctx1->getAccessibleRelationSet();
    auto relset2 = xctx2->getAccessibleRelationSet();
    if (relset1.is() != relset2.is())
        return false;

    auto relCount1 = relset1->getRelationCount();
    auto relCount2 = relset2->getRelationCount();
    if (relCount1 != relCount2)
        return false;

    for (sal_Int32 i = 0; i < relCount1; ++i)
    {
        if (relset1->getRelation(i) != relset2->getRelation(i))
            return false;
    }

    return equals(xctx1->getAccessibleParent(), xctx2->getAccessibleParent());
}

static OUString unknownName(const sal_Int64 value)
{
    return "unknown (" + OUString::number(value) + ")";
}

OUString AccessibilityTools::getRoleName(const sal_Int16 role)
{
    switch (role)
    {
        case accessibility::AccessibleRole::UNKNOWN:
            return "UNKNOWN";
        case accessibility::AccessibleRole::ALERT:
            return "ALERT";
        case accessibility::AccessibleRole::BUTTON_DROPDOWN:
            return "BUTTON_DROPDOWN";
        case accessibility::AccessibleRole::BUTTON_MENU:
            return "BUTTON_MENU";
        case accessibility::AccessibleRole::CANVAS:
            return "CANVAS";
        case accessibility::AccessibleRole::CAPTION:
            return "CAPTION";
        case accessibility::AccessibleRole::CHART:
            return "CHART";
        case accessibility::AccessibleRole::CHECK_BOX:
            return "CHECK_BOX";
        case accessibility::AccessibleRole::CHECK_MENU_ITEM:
            return "CHECK_MENU_ITEM";
        case accessibility::AccessibleRole::COLOR_CHOOSER:
            return "COLOR_CHOOSER";
        case accessibility::AccessibleRole::COLUMN_HEADER:
            return "COLUMN_HEADER";
        case accessibility::AccessibleRole::COMBO_BOX:
            return "COMBO_BOX";
        case accessibility::AccessibleRole::COMMENT:
            return "COMMENT";
        case accessibility::AccessibleRole::COMMENT_END:
            return "COMMENT_END";
        case accessibility::AccessibleRole::DATE_EDITOR:
            return "DATE_EDITOR";
        case accessibility::AccessibleRole::DESKTOP_ICON:
            return "DESKTOP_ICON";
        case accessibility::AccessibleRole::DESKTOP_PANE:
            return "DESKTOP_PANE";
        case accessibility::AccessibleRole::DIALOG:
            return "DIALOG";
        case accessibility::AccessibleRole::DIRECTORY_PANE:
            return "DIRECTORY_PANE";
        case accessibility::AccessibleRole::DOCUMENT:
            return "DOCUMENT";
        case accessibility::AccessibleRole::DOCUMENT_PRESENTATION:
            return "DOCUMENT_PRESENTATION";
        case accessibility::AccessibleRole::DOCUMENT_SPREADSHEET:
            return "DOCUMENT_SPREADSHEET";
        case accessibility::AccessibleRole::DOCUMENT_TEXT:
            return "DOCUMENT_TEXT";
        case accessibility::AccessibleRole::EDIT_BAR:
            return "EDIT_BAR";
        case accessibility::AccessibleRole::EMBEDDED_OBJECT:
            return "EMBEDDED_OBJECT";
        case accessibility::AccessibleRole::END_NOTE:
            return "END_NOTE";
        case accessibility::AccessibleRole::FILE_CHOOSER:
            return "FILE_CHOOSER";
        case accessibility::AccessibleRole::FILLER:
            return "FILLER";
        case accessibility::AccessibleRole::FONT_CHOOSER:
            return "FONT_CHOOSER";
        case accessibility::AccessibleRole::FOOTER:
            return "FOOTER";
        case accessibility::AccessibleRole::FOOTNOTE:
            return "FOOTNOTE";
        case accessibility::AccessibleRole::FORM:
            return "FORM";
        case accessibility::AccessibleRole::FRAME:
            return "FRAME";
        case accessibility::AccessibleRole::GLASS_PANE:
            return "GLASS_PANE";
        case accessibility::AccessibleRole::GRAPHIC:
            return "GRAPHIC";
        case accessibility::AccessibleRole::GROUP_BOX:
            return "GROUP_BOX";
        case accessibility::AccessibleRole::HEADER:
            return "HEADER";
        case accessibility::AccessibleRole::HEADING:
            return "HEADING";
        case accessibility::AccessibleRole::HYPER_LINK:
            return "HYPER_LINK";
        case accessibility::AccessibleRole::ICON:
            return "ICON";
        case accessibility::AccessibleRole::IMAGE_MAP:
            return "IMAGE_MAP";
        case accessibility::AccessibleRole::INTERNAL_FRAME:
            return "INTERNAL_FRAME";
        case accessibility::AccessibleRole::LABEL:
            return "LABEL";
        case accessibility::AccessibleRole::LAYERED_PANE:
            return "LAYERED_PANE";
        case accessibility::AccessibleRole::LIST:
            return "LIST";
        case accessibility::AccessibleRole::LIST_ITEM:
            return "LIST_ITEM";
        case accessibility::AccessibleRole::MENU:
            return "MENU";
        case accessibility::AccessibleRole::MENU_BAR:
            return "MENU_BAR";
        case accessibility::AccessibleRole::MENU_ITEM:
            return "MENU_ITEM";
        case accessibility::AccessibleRole::NOTE:
            return "NOTE";
        case accessibility::AccessibleRole::OPTION_PANE:
            return "OPTION_PANE";
        case accessibility::AccessibleRole::PAGE:
            return "PAGE";
        case accessibility::AccessibleRole::PAGE_TAB:
            return "PAGE_TAB";
        case accessibility::AccessibleRole::PAGE_TAB_LIST:
            return "PAGE_TAB_LIST";
        case accessibility::AccessibleRole::PANEL:
            return "PANEL";
        case accessibility::AccessibleRole::PARAGRAPH:
            return "PARAGRAPH";
        case accessibility::AccessibleRole::PASSWORD_TEXT:
            return "PASSWORD_TEXT";
        case accessibility::AccessibleRole::POPUP_MENU:
            return "POPUP_MENU";
        case accessibility::AccessibleRole::PROGRESS_BAR:
            return "PROGRESS_BAR";
        case accessibility::AccessibleRole::PUSH_BUTTON:
            return "PUSH_BUTTON";
        case accessibility::AccessibleRole::RADIO_BUTTON:
            return "RADIO_BUTTON";
        case accessibility::AccessibleRole::RADIO_MENU_ITEM:
            return "RADIO_MENU_ITEM";
        case accessibility::AccessibleRole::ROOT_PANE:
            return "ROOT_PANE";
        case accessibility::AccessibleRole::ROW_HEADER:
            return "ROW_HEADER";
        case accessibility::AccessibleRole::RULER:
            return "RULER";
        case accessibility::AccessibleRole::SCROLL_BAR:
            return "SCROLL_BAR";
        case accessibility::AccessibleRole::SCROLL_PANE:
            return "SCROLL_PANE";
        case accessibility::AccessibleRole::SECTION:
            return "SECTION";
        case accessibility::AccessibleRole::SEPARATOR:
            return "SEPARATOR";
        case accessibility::AccessibleRole::SHAPE:
            return "SHAPE";
        case accessibility::AccessibleRole::SLIDER:
            return "SLIDER";
        case accessibility::AccessibleRole::SPIN_BOX:
            return "SPIN_BOX";
        case accessibility::AccessibleRole::SPLIT_PANE:
            return "SPLIT_PANE";
        case accessibility::AccessibleRole::STATIC:
            return "STATIC";
        case accessibility::AccessibleRole::STATUS_BAR:
            return "STATUS_BAR";
        case accessibility::AccessibleRole::TABLE:
            return "TABLE";
        case accessibility::AccessibleRole::TABLE_CELL:
            return "TABLE_CELL";
        case accessibility::AccessibleRole::TEXT:
            return "TEXT";
        case accessibility::AccessibleRole::TEXT_FRAME:
            return "TEXT_FRAME";
        case accessibility::AccessibleRole::TOGGLE_BUTTON:
            return "TOGGLE_BUTTON";
        case accessibility::AccessibleRole::TOOL_BAR:
            return "TOOL_BAR";
        case accessibility::AccessibleRole::TOOL_TIP:
            return "TOOL_TIP";
        case accessibility::AccessibleRole::TREE:
            return "TREE";
        case accessibility::AccessibleRole::TREE_ITEM:
            return "TREE_ITEM";
        case accessibility::AccessibleRole::TREE_TABLE:
            return "TREE_TABLE";
        case accessibility::AccessibleRole::VIEW_PORT:
            return "VIEW_PORT";
        case accessibility::AccessibleRole::WINDOW:
            return "WINDOW";
    };
    return unknownName(role);
}

OUString AccessibilityTools::debugAccessibleStateSet(const sal_Int64 nCombinedState)
{
    OUString combinedName;

    for (int i = 0; i < 63; i++)
    {
        sal_Int64 state = sal_Int64(1) << i;
        if (!(state & nCombinedState))
            continue;
        OUString name;
        switch (state)
        {
            case accessibility::AccessibleStateType::ACTIVE:
                name = "ACTIVE";
                break;
            case accessibility::AccessibleStateType::ARMED:
                name = "ARMED";
                break;
            case accessibility::AccessibleStateType::BUSY:
                name = "BUSY";
                break;
            case accessibility::AccessibleStateType::CHECKED:
                name = "CHECKED";
                break;
            case accessibility::AccessibleStateType::COLLAPSE:
                name = "COLLAPSE";
                break;
            case accessibility::AccessibleStateType::DEFAULT:
                name = "DEFAULT";
                break;
            case accessibility::AccessibleStateType::DEFUNC:
                name = "DEFUNC";
                break;
            case accessibility::AccessibleStateType::EDITABLE:
                name = "EDITABLE";
                break;
            case accessibility::AccessibleStateType::ENABLED:
                name = "ENABLED";
                break;
            case accessibility::AccessibleStateType::EXPANDABLE:
                name = "EXPANDABLE";
                break;
            case accessibility::AccessibleStateType::EXPANDED:
                name = "EXPANDED";
                break;
            case accessibility::AccessibleStateType::FOCUSABLE:
                name = "FOCUSABLE";
                break;
            case accessibility::AccessibleStateType::FOCUSED:
                name = "FOCUSED";
                break;
            case accessibility::AccessibleStateType::HORIZONTAL:
                name = "HORIZONTAL";
                break;
            case accessibility::AccessibleStateType::ICONIFIED:
                name = "ICONIFIED";
                break;
            case accessibility::AccessibleStateType::INDETERMINATE:
                name = "INDETERMINATE";
                break;
            case accessibility::AccessibleStateType::INVALID:
                name = "INVALID";
                break;
            case accessibility::AccessibleStateType::MANAGES_DESCENDANTS:
                name = "MANAGES_DESCENDANTS";
                break;
            case accessibility::AccessibleStateType::MODAL:
                name = "MODAL";
                break;
            case accessibility::AccessibleStateType::MOVEABLE:
                name = "MOVEABLE";
                break;
            case accessibility::AccessibleStateType::MULTI_LINE:
                name = "MULTI_LINE";
                break;
            case accessibility::AccessibleStateType::MULTI_SELECTABLE:
                name = "MULTI_SELECTABLE";
                break;
            case accessibility::AccessibleStateType::OFFSCREEN:
                name = "OFFSCREEN";
                break;
            case accessibility::AccessibleStateType::OPAQUE:
                name = "OPAQUE";
                break;
            case accessibility::AccessibleStateType::PRESSED:
                name = "PRESSED";
                break;
            case accessibility::AccessibleStateType::RESIZABLE:
                name = "RESIZABLE";
                break;
            case accessibility::AccessibleStateType::SELECTABLE:
                name = "SELECTABLE";
                break;
            case accessibility::AccessibleStateType::SELECTED:
                name = "SELECTED";
                break;
            case accessibility::AccessibleStateType::SENSITIVE:
                name = "SENSITIVE";
                break;
            case accessibility::AccessibleStateType::SHOWING:
                name = "SHOWING";
                break;
            case accessibility::AccessibleStateType::SINGLE_LINE:
                name = "SINGLE_LINE";
                break;
            case accessibility::AccessibleStateType::STALE:
                name = "STALE";
                break;
            case accessibility::AccessibleStateType::TRANSIENT:
                name = "TRANSIENT";
                break;
            case accessibility::AccessibleStateType::VERTICAL:
                name = "VERTICAL";
                break;
            case accessibility::AccessibleStateType::VISIBLE:
                name = "VISIBLE";
                break;
            default:
                name = unknownName(state);
                break;
        }
        if (combinedName.getLength())
            combinedName += " | ";
        combinedName += name;
    }

    if (combinedName.isEmpty())
        return "unknown";
    return combinedName;
}

OUString AccessibilityTools::getEventIdName(const sal_Int16 event_id)
{
    switch (event_id)
    {
        case accessibility::AccessibleEventId::ACTION_CHANGED:
            return "ACTION_CHANGED";
        case accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            return "ACTIVE_DESCENDANT_CHANGED";
        case accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS:
            return "ACTIVE_DESCENDANT_CHANGED_NOFOCUS";
        case accessibility::AccessibleEventId::BOUNDRECT_CHANGED:
            return "BOUNDRECT_CHANGED";
        case accessibility::AccessibleEventId::CARET_CHANGED:
            return "CARET_CHANGED";
        case accessibility::AccessibleEventId::CHILD:
            return "CHILD";
        case accessibility::AccessibleEventId::COLUMN_CHANGED:
            return "COLUMN_CHANGED";
        case accessibility::AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
            return "CONTENT_FLOWS_FROM_RELATION_CHANGED";
        case accessibility::AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
            return "CONTENT_FLOWS_TO_RELATION_CHANGED";
        case accessibility::AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
            return "CONTROLLED_BY_RELATION_CHANGED";
        case accessibility::AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
            return "CONTROLLER_FOR_RELATION_CHANGED";
        case accessibility::AccessibleEventId::DESCRIPTION_CHANGED:
            return "DESCRIPTION_CHANGED";
        case accessibility::AccessibleEventId::HYPERTEXT_CHANGED:
            return "HYPERTEXT_CHANGED";
        case accessibility::AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            return "INVALIDATE_ALL_CHILDREN";
        case accessibility::AccessibleEventId::LABELED_BY_RELATION_CHANGED:
            return "LABELED_BY_RELATION_CHANGED";
        case accessibility::AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
            return "LABEL_FOR_RELATION_CHANGED";
        case accessibility::AccessibleEventId::LISTBOX_ENTRY_COLLAPSED:
            return "LISTBOX_ENTRY_COLLAPSED";
        case accessibility::AccessibleEventId::LISTBOX_ENTRY_EXPANDED:
            return "LISTBOX_ENTRY_EXPANDED";
        case accessibility::AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
            return "MEMBER_OF_RELATION_CHANGED";
        case accessibility::AccessibleEventId::NAME_CHANGED:
            return "NAME_CHANGED";
        case accessibility::AccessibleEventId::PAGE_CHANGED:
            return "PAGE_CHANGED";
        case accessibility::AccessibleEventId::ROLE_CHANGED:
            return "ROLE_CHANGED";
        case accessibility::AccessibleEventId::SECTION_CHANGED:
            return "SECTION_CHANGED";
        case accessibility::AccessibleEventId::SELECTION_CHANGED:
            return "SELECTION_CHANGED";
        case accessibility::AccessibleEventId::SELECTION_CHANGED_ADD:
            return "SELECTION_CHANGED_ADD";
        case accessibility::AccessibleEventId::SELECTION_CHANGED_REMOVE:
            return "SELECTION_CHANGED_REMOVE";
        case accessibility::AccessibleEventId::SELECTION_CHANGED_WITHIN:
            return "SELECTION_CHANGED_WITHIN";
        case accessibility::AccessibleEventId::STATE_CHANGED:
            return "STATE_CHANGED";
        case accessibility::AccessibleEventId::SUB_WINDOW_OF_RELATION_CHANGED:
            return "SUB_WINDOW_OF_RELATION_CHANGED";
        case accessibility::AccessibleEventId::TABLE_CAPTION_CHANGED:
            return "TABLE_CAPTION_CHANGED";
        case accessibility::AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
            return "TABLE_COLUMN_DESCRIPTION_CHANGED";
        case accessibility::AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
            return "TABLE_COLUMN_HEADER_CHANGED";
        case accessibility::AccessibleEventId::TABLE_MODEL_CHANGED:
            return "TABLE_MODEL_CHANGED";
        case accessibility::AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            return "TABLE_ROW_DESCRIPTION_CHANGED";
        case accessibility::AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            return "TABLE_ROW_HEADER_CHANGED";
        case accessibility::AccessibleEventId::TABLE_SUMMARY_CHANGED:
            return "TABLE_SUMMARY_CHANGED";
        case accessibility::AccessibleEventId::TEXT_ATTRIBUTE_CHANGED:
            return "TEXT_ATTRIBUTE_CHANGED";
        case accessibility::AccessibleEventId::TEXT_CHANGED:
            return "TEXT_CHANGED";
        case accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED:
            return "TEXT_SELECTION_CHANGED";
        case accessibility::AccessibleEventId::VALUE_CHANGED:
            return "VALUE_CHANGED";
        case accessibility::AccessibleEventId::VISIBLE_DATA_CHANGED:
            return "VISIBLE_DATA_CHANGED";
    }
    return unknownName(event_id);
}

OUString AccessibilityTools::getRelationTypeName(const sal_Int16 rel_type)
{
    switch (rel_type)
    {
        case accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM:
            return "CONTENT_FLOWS_FROM";
        case accessibility::AccessibleRelationType::CONTENT_FLOWS_TO:
            return "CONTENT_FLOWS_TO";
        case accessibility::AccessibleRelationType::CONTROLLED_BY:
            return "CONTROLLED_BY";
        case accessibility::AccessibleRelationType::CONTROLLER_FOR:
            return "CONTROLLER_FOR";
        case accessibility::AccessibleRelationType::DESCRIBED_BY:
            return "DESCRIBED_BY";
        case accessibility::AccessibleRelationType::INVALID:
            return "INVALID";
        case accessibility::AccessibleRelationType::LABELED_BY:
            return "LABELED_BY";
        case accessibility::AccessibleRelationType::LABEL_FOR:
            return "LABEL_FOR";
        case accessibility::AccessibleRelationType::MEMBER_OF:
            return "MEMBER_OF";
        case accessibility::AccessibleRelationType::NODE_CHILD_OF:
            return "NODE_CHILD_OF";
        case accessibility::AccessibleRelationType::SUB_WINDOW_OF:
            return "SUB_WINDOW_OF";
    }
    return unknownName(rel_type);
}

OUString AccessibilityTools::debugName(accessibility::XAccessibleContext* ctx)
{
    return "role=" + AccessibilityTools::getRoleName(ctx->getAccessibleRole()) + " name=\""
           + ctx->getAccessibleName() + "\" description=\"" + ctx->getAccessibleDescription()
           + "\"";
}

OUString AccessibilityTools::debugName(accessibility::XAccessible* acc)
{
    return debugName(acc->getAccessibleContext().get());
}

OUString AccessibilityTools::debugName(const accessibility::AccessibleEventObject* evobj)
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
