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
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <o3tl/string_view.hxx>

using namespace css;

uno::Reference<accessibility::XAccessibleContext>
AccessibilityTools::getAccessibleObjectForPredicate(
    const uno::Reference<accessibility::XAccessibleContext>& xCtx,
    const std::function<bool(const uno::Reference<accessibility::XAccessibleContext>&)>& cPredicate)
{
    if (cPredicate(xCtx))
    {
        return xCtx;
    }
    else
    {
        sal_Int64 count = xCtx->getAccessibleChildCount();

        for (sal_Int64 i = 0; i < count && i < AccessibilityTools::MAX_CHILDREN; i++)
        {
            uno::Reference<accessibility::XAccessibleContext> xCtx2
                = getAccessibleObjectForPredicate(xCtx->getAccessibleChild(i), cPredicate);
            if (xCtx2.is())
                return xCtx2;
        }
    }
    return nullptr;
}

uno::Reference<accessibility::XAccessibleContext>
AccessibilityTools::getAccessibleObjectForPredicate(
    const uno::Reference<accessibility::XAccessible>& xAcc,
    const std::function<bool(const uno::Reference<accessibility::XAccessibleContext>&)>& cPredicate)
{
    return getAccessibleObjectForPredicate(xAcc->getAccessibleContext(), cPredicate);
}

uno::Reference<accessibility::XAccessibleContext> AccessibilityTools::getAccessibleObjectForRole(
    const uno::Reference<accessibility::XAccessibleContext>& xCtx, sal_Int16 role)
{
    return getAccessibleObjectForPredicate(
        xCtx, [&role](const uno::Reference<accessibility::XAccessibleContext>& xObjCtx) {
            return (xObjCtx->getAccessibleRole() == role
                    && xObjCtx->getAccessibleStateSet()
                           & accessibility::AccessibleStateType::SHOWING);
        });
}

css::uno::Reference<css::accessibility::XAccessibleContext>
AccessibilityTools::getAccessibleObjectForRole(
    const css::uno::Reference<css::accessibility::XAccessible>& xacc, sal_Int16 role)
{
    return getAccessibleObjectForRole(xacc->getAccessibleContext(), role);
}

/* this is basically the same as getAccessibleObjectForPredicate() but specialized for efficiency,
 * and because the template version will not work with getAccessibleObjectForPredicate() anyway */
css::uno::Reference<css::accessibility::XAccessibleContext>
AccessibilityTools::getAccessibleObjectForName(
    const css::uno::Reference<css::accessibility::XAccessibleContext>& xCtx, const sal_Int16 role,
    std::u16string_view name)
{
    if (xCtx->getAccessibleRole() == role && nameEquals(xCtx, name))
        return xCtx;

    auto nChildren = xCtx->getAccessibleChildCount();
    for (decltype(nChildren) i = 0; i < nChildren && i < AccessibilityTools::MAX_CHILDREN; i++)
    {
        if (auto xMatchChild = getAccessibleObjectForName(xCtx->getAccessibleChild(i), role, name))
            return xMatchChild;
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
    else if (relset1.is())
    {
        auto relCount1 = relset1->getRelationCount();
        auto relCount2 = relset2->getRelationCount();
        if (relCount1 != relCount2)
            return false;

        for (sal_Int32 i = 0; i < relCount1; ++i)
        {
            if (relset1->getRelation(i) != relset2->getRelation(i))
                return false;
        }
    }

    return equals(xctx1->getAccessibleParent(), xctx2->getAccessibleParent());
}

bool AccessibilityTools::nameEquals(const uno::Reference<accessibility::XAccessibleContext>& xCtx,
                                    const std::u16string_view name)
{
    auto ctxName = xCtx->getAccessibleName();
    std::u16string_view rest;

    if (!o3tl::starts_with(ctxName, name, &rest))
        return false;
    if (rest == u"")
        return true;

#if defined(_WIN32)
    // see OAccessibleMenuItemComponent::GetAccessibleName():
    // on Win32, ignore a \tSHORTCUT suffix on a menu item
    switch (xCtx->getAccessibleRole())
    {
        case accessibility::AccessibleRole::MENU_ITEM:
        case accessibility::AccessibleRole::RADIO_MENU_ITEM:
        case accessibility::AccessibleRole::CHECK_MENU_ITEM:
            return rest[0] == '\t';

        default:
            break;
    }
#endif

#if OSL_DEBUG_LEVEL > 0
    // see VCLXAccessibleComponent::getAccessibleName()
    static const char* pEnvAppendType = getenv("LIBO_APPEND_WINDOW_TYPE_TO_ACCESSIBLE_NAME");
    if (pEnvAppendType && OUString::createFromAscii(pEnvAppendType) != u"0")
    {
        auto pVCLXAccessibleComponent = dynamic_cast<VCLXAccessibleComponent*>(xCtx.get());
        if (pVCLXAccessibleComponent)
        {
            auto windowType = pVCLXAccessibleComponent->GetWindow()->GetType();
            if (rest
                == Concat2View(u" (Type = " + OUString::number(static_cast<sal_Int32>(windowType))
                               + ")"))
                return true;
        }
    }
#endif
    return false;
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
            return u"UNKNOWN"_ustr;
        case accessibility::AccessibleRole::ALERT:
            return u"ALERT"_ustr;
        case accessibility::AccessibleRole::BLOCK_QUOTE:
            return u"BLOCK_QUOTE"_ustr;
        case accessibility::AccessibleRole::BUTTON_DROPDOWN:
            return u"BUTTON_DROPDOWN"_ustr;
        case accessibility::AccessibleRole::BUTTON_MENU:
            return u"BUTTON_MENU"_ustr;
        case accessibility::AccessibleRole::CANVAS:
            return u"CANVAS"_ustr;
        case accessibility::AccessibleRole::CAPTION:
            return u"CAPTION"_ustr;
        case accessibility::AccessibleRole::CHART:
            return u"CHART"_ustr;
        case accessibility::AccessibleRole::CHECK_BOX:
            return u"CHECK_BOX"_ustr;
        case accessibility::AccessibleRole::CHECK_MENU_ITEM:
            return u"CHECK_MENU_ITEM"_ustr;
        case accessibility::AccessibleRole::COLOR_CHOOSER:
            return u"COLOR_CHOOSER"_ustr;
        case accessibility::AccessibleRole::COLUMN_HEADER:
            return u"COLUMN_HEADER"_ustr;
        case accessibility::AccessibleRole::COMBO_BOX:
            return u"COMBO_BOX"_ustr;
        case accessibility::AccessibleRole::COMMENT:
            return u"COMMENT"_ustr;
        case accessibility::AccessibleRole::COMMENT_END:
            return u"COMMENT_END"_ustr;
        case accessibility::AccessibleRole::DATE_EDITOR:
            return u"DATE_EDITOR"_ustr;
        case accessibility::AccessibleRole::DESKTOP_ICON:
            return u"DESKTOP_ICON"_ustr;
        case accessibility::AccessibleRole::DESKTOP_PANE:
            return u"DESKTOP_PANE"_ustr;
        case accessibility::AccessibleRole::DIALOG:
            return u"DIALOG"_ustr;
        case accessibility::AccessibleRole::DIRECTORY_PANE:
            return u"DIRECTORY_PANE"_ustr;
        case accessibility::AccessibleRole::DOCUMENT:
            return u"DOCUMENT"_ustr;
        case accessibility::AccessibleRole::DOCUMENT_PRESENTATION:
            return u"DOCUMENT_PRESENTATION"_ustr;
        case accessibility::AccessibleRole::DOCUMENT_SPREADSHEET:
            return u"DOCUMENT_SPREADSHEET"_ustr;
        case accessibility::AccessibleRole::DOCUMENT_TEXT:
            return u"DOCUMENT_TEXT"_ustr;
        case accessibility::AccessibleRole::EDIT_BAR:
            return u"EDIT_BAR"_ustr;
        case accessibility::AccessibleRole::EMBEDDED_OBJECT:
            return u"EMBEDDED_OBJECT"_ustr;
        case accessibility::AccessibleRole::END_NOTE:
            return u"END_NOTE"_ustr;
        case accessibility::AccessibleRole::FILE_CHOOSER:
            return u"FILE_CHOOSER"_ustr;
        case accessibility::AccessibleRole::FILLER:
            return u"FILLER"_ustr;
        case accessibility::AccessibleRole::FONT_CHOOSER:
            return u"FONT_CHOOSER"_ustr;
        case accessibility::AccessibleRole::FOOTER:
            return u"FOOTER"_ustr;
        case accessibility::AccessibleRole::FOOTNOTE:
            return u"FOOTNOTE"_ustr;
        case accessibility::AccessibleRole::FORM:
            return u"FORM"_ustr;
        case accessibility::AccessibleRole::FRAME:
            return u"FRAME"_ustr;
        case accessibility::AccessibleRole::GLASS_PANE:
            return u"GLASS_PANE"_ustr;
        case accessibility::AccessibleRole::GRAPHIC:
            return u"GRAPHIC"_ustr;
        case accessibility::AccessibleRole::GROUP_BOX:
            return u"GROUP_BOX"_ustr;
        case accessibility::AccessibleRole::HEADER:
            return u"HEADER"_ustr;
        case accessibility::AccessibleRole::HEADING:
            return u"HEADING"_ustr;
        case accessibility::AccessibleRole::HYPER_LINK:
            return u"HYPER_LINK"_ustr;
        case accessibility::AccessibleRole::ICON:
            return u"ICON"_ustr;
        case accessibility::AccessibleRole::IMAGE_MAP:
            return u"IMAGE_MAP"_ustr;
        case accessibility::AccessibleRole::INTERNAL_FRAME:
            return u"INTERNAL_FRAME"_ustr;
        case accessibility::AccessibleRole::LABEL:
            return u"LABEL"_ustr;
        case accessibility::AccessibleRole::LAYERED_PANE:
            return u"LAYERED_PANE"_ustr;
        case accessibility::AccessibleRole::LIST:
            return u"LIST"_ustr;
        case accessibility::AccessibleRole::LIST_ITEM:
            return u"LIST_ITEM"_ustr;
        case accessibility::AccessibleRole::MENU:
            return u"MENU"_ustr;
        case accessibility::AccessibleRole::MENU_BAR:
            return u"MENU_BAR"_ustr;
        case accessibility::AccessibleRole::MENU_ITEM:
            return u"MENU_ITEM"_ustr;
        case accessibility::AccessibleRole::NOTE:
            return u"NOTE"_ustr;
        case accessibility::AccessibleRole::OPTION_PANE:
            return u"OPTION_PANE"_ustr;
        case accessibility::AccessibleRole::PAGE:
            return u"PAGE"_ustr;
        case accessibility::AccessibleRole::PAGE_TAB:
            return u"PAGE_TAB"_ustr;
        case accessibility::AccessibleRole::PAGE_TAB_LIST:
            return u"PAGE_TAB_LIST"_ustr;
        case accessibility::AccessibleRole::PANEL:
            return u"PANEL"_ustr;
        case accessibility::AccessibleRole::PARAGRAPH:
            return u"PARAGRAPH"_ustr;
        case accessibility::AccessibleRole::PASSWORD_TEXT:
            return u"PASSWORD_TEXT"_ustr;
        case accessibility::AccessibleRole::POPUP_MENU:
            return u"POPUP_MENU"_ustr;
        case accessibility::AccessibleRole::PROGRESS_BAR:
            return u"PROGRESS_BAR"_ustr;
        case accessibility::AccessibleRole::PUSH_BUTTON:
            return u"PUSH_BUTTON"_ustr;
        case accessibility::AccessibleRole::RADIO_BUTTON:
            return u"RADIO_BUTTON"_ustr;
        case accessibility::AccessibleRole::RADIO_MENU_ITEM:
            return u"RADIO_MENU_ITEM"_ustr;
        case accessibility::AccessibleRole::ROOT_PANE:
            return u"ROOT_PANE"_ustr;
        case accessibility::AccessibleRole::ROW_HEADER:
            return u"ROW_HEADER"_ustr;
        case accessibility::AccessibleRole::RULER:
            return u"RULER"_ustr;
        case accessibility::AccessibleRole::SCROLL_BAR:
            return u"SCROLL_BAR"_ustr;
        case accessibility::AccessibleRole::SCROLL_PANE:
            return u"SCROLL_PANE"_ustr;
        case accessibility::AccessibleRole::SECTION:
            return u"SECTION"_ustr;
        case accessibility::AccessibleRole::SEPARATOR:
            return u"SEPARATOR"_ustr;
        case accessibility::AccessibleRole::SHAPE:
            return u"SHAPE"_ustr;
        case accessibility::AccessibleRole::SLIDER:
            return u"SLIDER"_ustr;
        case accessibility::AccessibleRole::SPIN_BOX:
            return u"SPIN_BOX"_ustr;
        case accessibility::AccessibleRole::SPLIT_PANE:
            return u"SPLIT_PANE"_ustr;
        case accessibility::AccessibleRole::STATIC:
            return u"STATIC"_ustr;
        case accessibility::AccessibleRole::STATUS_BAR:
            return u"STATUS_BAR"_ustr;
        case accessibility::AccessibleRole::TABLE:
            return u"TABLE"_ustr;
        case accessibility::AccessibleRole::TABLE_CELL:
            return u"TABLE_CELL"_ustr;
        case accessibility::AccessibleRole::TEXT:
            return u"TEXT"_ustr;
        case accessibility::AccessibleRole::TEXT_FRAME:
            return u"TEXT_FRAME"_ustr;
        case accessibility::AccessibleRole::TOGGLE_BUTTON:
            return u"TOGGLE_BUTTON"_ustr;
        case accessibility::AccessibleRole::TOOL_BAR:
            return u"TOOL_BAR"_ustr;
        case accessibility::AccessibleRole::TOOL_TIP:
            return u"TOOL_TIP"_ustr;
        case accessibility::AccessibleRole::TREE:
            return u"TREE"_ustr;
        case accessibility::AccessibleRole::TREE_ITEM:
            return u"TREE_ITEM"_ustr;
        case accessibility::AccessibleRole::TREE_TABLE:
            return u"TREE_TABLE"_ustr;
        case accessibility::AccessibleRole::VIEW_PORT:
            return u"VIEW_PORT"_ustr;
        case accessibility::AccessibleRole::WINDOW:
            return u"WINDOW"_ustr;
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
            case accessibility::AccessibleStateType::CHECKABLE:
                name = "CHECKABLE";
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
        return u"unknown"_ustr;
    return combinedName;
}

OUString AccessibilityTools::getEventIdName(const sal_Int16 event_id)
{
    switch (event_id)
    {
        case accessibility::AccessibleEventId::ACTION_CHANGED:
            return u"ACTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            return u"ACTIVE_DESCENDANT_CHANGED"_ustr;
        case accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS:
            return u"ACTIVE_DESCENDANT_CHANGED_NOFOCUS"_ustr;
        case accessibility::AccessibleEventId::BOUNDRECT_CHANGED:
            return u"BOUNDRECT_CHANGED"_ustr;
        case accessibility::AccessibleEventId::CARET_CHANGED:
            return u"CARET_CHANGED"_ustr;
        case accessibility::AccessibleEventId::CHILD:
            return u"CHILD"_ustr;
        case accessibility::AccessibleEventId::COLUMN_CHANGED:
            return u"COLUMN_CHANGED"_ustr;
        case accessibility::AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
            return u"CONTENT_FLOWS_FROM_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
            return u"CONTENT_FLOWS_TO_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
            return u"CONTROLLED_BY_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
            return u"CONTROLLER_FOR_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::DESCRIPTION_CHANGED:
            return u"DESCRIPTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::HYPERTEXT_CHANGED:
            return u"HYPERTEXT_CHANGED"_ustr;
        case accessibility::AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            return u"INVALIDATE_ALL_CHILDREN"_ustr;
        case accessibility::AccessibleEventId::LABELED_BY_RELATION_CHANGED:
            return u"LABELED_BY_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
            return u"LABEL_FOR_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::LISTBOX_ENTRY_COLLAPSED:
            return u"LISTBOX_ENTRY_COLLAPSED"_ustr;
        case accessibility::AccessibleEventId::LISTBOX_ENTRY_EXPANDED:
            return u"LISTBOX_ENTRY_EXPANDED"_ustr;
        case accessibility::AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
            return u"MEMBER_OF_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::NAME_CHANGED:
            return u"NAME_CHANGED"_ustr;
        case accessibility::AccessibleEventId::PAGE_CHANGED:
            return u"PAGE_CHANGED"_ustr;
        case accessibility::AccessibleEventId::ROLE_CHANGED:
            return u"ROLE_CHANGED"_ustr;
        case accessibility::AccessibleEventId::SECTION_CHANGED:
            return u"SECTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::SELECTION_CHANGED:
            return u"SELECTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::SELECTION_CHANGED_ADD:
            return u"SELECTION_CHANGED_ADD"_ustr;
        case accessibility::AccessibleEventId::SELECTION_CHANGED_REMOVE:
            return u"SELECTION_CHANGED_REMOVE"_ustr;
        case accessibility::AccessibleEventId::SELECTION_CHANGED_WITHIN:
            return u"SELECTION_CHANGED_WITHIN"_ustr;
        case accessibility::AccessibleEventId::STATE_CHANGED:
            return u"STATE_CHANGED"_ustr;
        case accessibility::AccessibleEventId::SUB_WINDOW_OF_RELATION_CHANGED:
            return u"SUB_WINDOW_OF_RELATION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TABLE_CAPTION_CHANGED:
            return u"TABLE_CAPTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
            return u"TABLE_COLUMN_DESCRIPTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
            return u"TABLE_COLUMN_HEADER_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TABLE_MODEL_CHANGED:
            return u"TABLE_MODEL_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            return u"TABLE_ROW_DESCRIPTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            return u"TABLE_ROW_HEADER_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TABLE_SUMMARY_CHANGED:
            return u"TABLE_SUMMARY_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TEXT_ATTRIBUTE_CHANGED:
            return u"TEXT_ATTRIBUTE_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TEXT_CHANGED:
            return u"TEXT_CHANGED"_ustr;
        case accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED:
            return u"TEXT_SELECTION_CHANGED"_ustr;
        case accessibility::AccessibleEventId::VALUE_CHANGED:
            return u"VALUE_CHANGED"_ustr;
        case accessibility::AccessibleEventId::VISIBLE_DATA_CHANGED:
            return u"VISIBLE_DATA_CHANGED"_ustr;
    }
    return unknownName(event_id);
}

OUString AccessibilityTools::getRelationTypeName(const sal_Int16 rel_type)
{
    switch (rel_type)
    {
        case accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM:
            return u"CONTENT_FLOWS_FROM"_ustr;
        case accessibility::AccessibleRelationType::CONTENT_FLOWS_TO:
            return u"CONTENT_FLOWS_TO"_ustr;
        case accessibility::AccessibleRelationType::CONTROLLED_BY:
            return u"CONTROLLED_BY"_ustr;
        case accessibility::AccessibleRelationType::CONTROLLER_FOR:
            return u"CONTROLLER_FOR"_ustr;
        case accessibility::AccessibleRelationType::DESCRIBED_BY:
            return u"DESCRIBED_BY"_ustr;
        case accessibility::AccessibleRelationType::INVALID:
            return u"INVALID"_ustr;
        case accessibility::AccessibleRelationType::LABELED_BY:
            return u"LABELED_BY"_ustr;
        case accessibility::AccessibleRelationType::LABEL_FOR:
            return u"LABEL_FOR"_ustr;
        case accessibility::AccessibleRelationType::MEMBER_OF:
            return u"MEMBER_OF"_ustr;
        case accessibility::AccessibleRelationType::NODE_CHILD_OF:
            return u"NODE_CHILD_OF"_ustr;
        case accessibility::AccessibleRelationType::SUB_WINDOW_OF:
            return u"SUB_WINDOW_OF"_ustr;
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

OUString AccessibilityTools::debugName(accessibility::XAccessibleAction* xAct)
{
    OUStringBuffer r = "actions=[";

    const sal_Int32 nActions = xAct->getAccessibleActionCount();
    for (sal_Int32 i = 0; i < nActions; i++)
    {
        if (i > 0)
            r.append(", ");

        r.append("description=\"" + xAct->getAccessibleActionDescription(i) + "\"");

        const auto& xKeyBinding = xAct->getAccessibleActionKeyBinding(i);
        if (xKeyBinding)
        {
            r.append(" keybindings=[");
            const sal_Int32 nKeyBindings = xKeyBinding->getAccessibleKeyBindingCount();
            for (sal_Int32 j = 0; j < nKeyBindings; j++)
            {
                if (j > 0)
                    r.append(", ");

                int k = 0;
                for (const auto& keyStroke : xKeyBinding->getAccessibleKeyBinding(j))
                {
                    if (k++ > 0)
                        r.append(", ");

                    r.append('"');
                    if (keyStroke.Modifiers & awt::KeyModifier::MOD1)
                        r.append("<Mod1>");
                    if (keyStroke.Modifiers & awt::KeyModifier::MOD2)
                        r.append("<Mod2>");
                    if (keyStroke.Modifiers & awt::KeyModifier::MOD3)
                        r.append("<Mod3>");
                    if (keyStroke.Modifiers & awt::KeyModifier::SHIFT)
                        r.append("<Shift>");
                    r.append(OUStringChar(keyStroke.KeyChar) + "\"");
                }
            }
            r.append("]");
        }
    }
    r.append("]");
    return r.makeStringAndClear();
}

OUString AccessibilityTools::debugName(accessibility::XAccessibleText* xTxt)
{
    uno::Reference<accessibility::XAccessibleContext> xCtx(xTxt, uno::UNO_QUERY);
    return debugName(xCtx.get());
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
