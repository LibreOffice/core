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

#include <Qt5AccessibleWidget.hxx>
#include <Qt5AccessibleWidget.moc>

#include <QtGui/QAccessibleInterface>

#include <Qt5Frame.hxx>
#include <Qt5Tools.hxx>
#include <Qt5Widget.hxx>
#include <Qt5XAccessible.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <sal/log.hxx>
#include <vcl/popupmenuwindow.hxx>
#include <comphelper/AccessibleImplementationHelper.hxx>

using namespace css;
using namespace css::accessibility;
using namespace css::uno;

Qt5AccessibleWidget::Qt5AccessibleWidget(const Reference<XAccessible> xAccessible)
    : m_xAccessible(xAccessible)
{
}

QWindow* Qt5AccessibleWidget::window() const { return nullptr; }

int Qt5AccessibleWidget::childCount() const
{
    return m_xAccessible->getAccessibleContext()->getAccessibleChildCount();
}

int Qt5AccessibleWidget::indexOfChild(const QAccessibleInterface* /* child */) const { return 0; }
QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
    Qt5AccessibleWidget::relations(QAccessible::Relation /* match */) const
{
    return QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>();
}

QAccessibleInterface* Qt5AccessibleWidget::focusChild() const
{
    /* if (m_pWindow->HasChildPathFocus())
        return QAccessible::queryAccessibleInterface(
            new Qt5XAccessible(m_xAccessible->getAccessibleContext()->getAccessibleChild(index))); */
    return QAccessible::queryAccessibleInterface(object());
}

QRect Qt5AccessibleWidget::rect() const
{
    Reference<XAccessibleComponent> xAccessibleComponent(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    awt::Point aPoint = xAccessibleComponent->getLocation();
    awt::Size aSize = xAccessibleComponent->getSize();

    return QRect(aPoint.X, aPoint.Y, aSize.Width, aSize.Height);
}

QAccessibleInterface* Qt5AccessibleWidget::parent() const
{
    return QAccessible::queryAccessibleInterface(
        new Qt5XAccessible(m_xAccessible->getAccessibleContext()->getAccessibleParent()));
}
QAccessibleInterface* Qt5AccessibleWidget::child(int index) const
{
    return QAccessible::queryAccessibleInterface(
        new Qt5XAccessible(m_xAccessible->getAccessibleContext()->getAccessibleChild(index)));
}

QString Qt5AccessibleWidget::text(QAccessible::Text text) const
{
    switch (text)
    {
        case QAccessible::Name:
            return toQString(m_xAccessible->getAccessibleContext()->getAccessibleName());
        case QAccessible::Description:
        case QAccessible::DebugDescription:
            return toQString(m_xAccessible->getAccessibleContext()->getAccessibleDescription());
        case QAccessible::Value:
        case QAccessible::Help:
        case QAccessible::Accelerator:
        case QAccessible::UserText:
        default:
            return QString("Unknown");
    }
}
QAccessible::Role Qt5AccessibleWidget::role() const
{
    if (!m_xAccessible.is())
        return QAccessible::NoRole;

    switch (m_xAccessible->getAccessibleContext()->getAccessibleRole())
    {
        case AccessibleRole::UNKNOWN:
            return QAccessible::NoRole;

        case AccessibleRole::ALERT:
            return QAccessible::AlertMessage;

        case AccessibleRole::COLUMN_HEADER:
            return QAccessible::ColumnHeader;

        case AccessibleRole::CANVAS:
            return QAccessible::Canvas;

        case AccessibleRole::CHECK_BOX:
            return QAccessible::CheckBox;

        case AccessibleRole::CHECK_MENU_ITEM:
            return QAccessible::MenuItem;

        case AccessibleRole::COLOR_CHOOSER:
            return QAccessible::ColorChooser;

        case AccessibleRole::COMBO_BOX:
            return QAccessible::ComboBox;

        case AccessibleRole::DATE_EDITOR:
            return QAccessible::EditableText;

        case AccessibleRole::DESKTOP_ICON:
            return QAccessible::Graphic;

        case AccessibleRole::DESKTOP_PANE:
        case AccessibleRole::DIRECTORY_PANE:
            return QAccessible::Pane;

        case AccessibleRole::DIALOG:
            return QAccessible::Dialog;

        case AccessibleRole::DOCUMENT:
            return QAccessible::Document;

        case AccessibleRole::EMBEDDED_OBJECT:
            return QAccessible::UserRole;

        case AccessibleRole::END_NOTE:
            return QAccessible::Note;

        case AccessibleRole::FILLER:
            return QAccessible::Whitespace;

        case AccessibleRole::FONT_CHOOSER:
            return QAccessible::UserRole;

        case AccessibleRole::FOOTER:
            return QAccessible::Footer;

        case AccessibleRole::FOOTNOTE:
            return QAccessible::Note;

        case AccessibleRole::FRAME: // top-level window with title bar
            return QAccessible::Window;

        case AccessibleRole::GLASS_PANE:
            return QAccessible::UserRole;

        case AccessibleRole::GRAPHIC:
            return QAccessible::Graphic;

        case AccessibleRole::GROUP_BOX:
            return QAccessible::Grouping;

        case AccessibleRole::HEADER:
            return QAccessible::UserRole;

        case AccessibleRole::HEADING:
            return QAccessible::Heading;

        case AccessibleRole::HYPER_LINK:
            return QAccessible::Link;

        case AccessibleRole::ICON:
            return QAccessible::Graphic;

        case AccessibleRole::INTERNAL_FRAME:
            return QAccessible::UserRole;

        case AccessibleRole::LABEL:
            return QAccessible::StaticText;

        case AccessibleRole::LAYERED_PANE:
            return QAccessible::Pane;

        case AccessibleRole::LIST:
            return QAccessible::List;

        case AccessibleRole::LIST_ITEM:
            return QAccessible::ListItem;

        case AccessibleRole::MENU:
        case AccessibleRole::MENU_BAR:
            return QAccessible::MenuBar;

        case AccessibleRole::MENU_ITEM:
            return QAccessible::MenuItem;

        case AccessibleRole::OPTION_PANE:
            return QAccessible::Pane;

        case AccessibleRole::PAGE_TAB:
            return QAccessible::PageTab;

        case AccessibleRole::PAGE_TAB_LIST:
            return QAccessible::PageTabList;

        case AccessibleRole::PANEL:
            return QAccessible::Pane;

        case AccessibleRole::PARAGRAPH:
            return QAccessible::Paragraph;

        case AccessibleRole::PASSWORD_TEXT:
            return QAccessible::EditableText;

        case AccessibleRole::POPUP_MENU:
            return QAccessible::PopupMenu;

        case AccessibleRole::PUSH_BUTTON:
            return QAccessible::Button;

        case AccessibleRole::PROGRESS_BAR:
            return QAccessible::ProgressBar;

        case AccessibleRole::RADIO_BUTTON:
            return QAccessible::RadioButton;

        case AccessibleRole::RADIO_MENU_ITEM:
            return QAccessible::MenuItem;

        case AccessibleRole::ROW_HEADER:
            return QAccessible::RowHeader;

        case AccessibleRole::ROOT_PANE:
            return QAccessible::Pane;

        case AccessibleRole::SCROLL_BAR:
            return QAccessible::ScrollBar;

        case AccessibleRole::SCROLL_PANE:
            return QAccessible::Pane;

        case AccessibleRole::SHAPE:
            return QAccessible::Graphic;

        case AccessibleRole::SEPARATOR:
            return QAccessible::Separator;

        case AccessibleRole::SLIDER:
            return QAccessible::Slider;

        case AccessibleRole::SPIN_BOX:
            return QAccessible::SpinBox;

        case AccessibleRole::SPLIT_PANE:
            return QAccessible::Pane;

        case AccessibleRole::STATUS_BAR:
            return QAccessible::StatusBar;

        case AccessibleRole::TABLE:
            return QAccessible::Table;

        case AccessibleRole::TABLE_CELL:
            return QAccessible::Cell;

        case AccessibleRole::TEXT:
            return QAccessible::EditableText;

        case AccessibleRole::TEXT_FRAME:
            return QAccessible::UserRole;

        case AccessibleRole::TOGGLE_BUTTON:
            return QAccessible::Button;

        case AccessibleRole::TOOL_BAR:
            return QAccessible::ToolBar;

        case AccessibleRole::TOOL_TIP:
            return QAccessible::ToolTip;

        case AccessibleRole::TREE:
            return QAccessible::Tree;

        case AccessibleRole::VIEW_PORT:
            return QAccessible::UserRole;

        case AccessibleRole::BUTTON_DROPDOWN:
            return QAccessible::Button;

        case AccessibleRole::BUTTON_MENU:
            return QAccessible::Button;

        case AccessibleRole::CAPTION:
            return QAccessible::StaticText;

        case AccessibleRole::CHART:
            return QAccessible::Chart;

        case AccessibleRole::EDIT_BAR:
            return QAccessible::Equation;

        case AccessibleRole::FORM:
            return QAccessible::Form;

        case AccessibleRole::IMAGE_MAP:
            return QAccessible::Graphic;

        case AccessibleRole::NOTE:
            return QAccessible::Note;

        case AccessibleRole::RULER:
            return QAccessible::UserRole;

        case AccessibleRole::SECTION:
            return QAccessible::Section;

        case AccessibleRole::TREE_ITEM:
            return QAccessible::TreeItem;

        case AccessibleRole::TREE_TABLE:
            return QAccessible::Tree;

        case AccessibleRole::COMMENT:
            return QAccessible::Note;

        case AccessibleRole::COMMENT_END:
            return QAccessible::UserRole;

        case AccessibleRole::DOCUMENT_PRESENTATION:
            return QAccessible::Document;

        case AccessibleRole::DOCUMENT_SPREADSHEET:
            return QAccessible::Document;

        case AccessibleRole::DOCUMENT_TEXT:
            return QAccessible::Document;

        case AccessibleRole::STATIC:
            return QAccessible::StaticText;

        /* Ignore window objects for sub-menus, combo- and list boxes,
         *  which are exposed as children of their parents.
         */
        case AccessibleRole::WINDOW: // top-level window without title bar
        {
            return QAccessible::Window;
        }
    }

    SAL_WARN("vcl.qt5",
             "Unmapped role: " << m_xAccessible->getAccessibleContext()->getAccessibleRole());
    return QAccessible::NoRole;
}

namespace
{
void lcl_addState(QAccessible::State* state, sal_Int16 nState)
{
    switch (nState)
    {
        case AccessibleStateType::INVALID:
            state->invalid = true;
            break;
        case AccessibleStateType::ACTIVE:
            state->active = true;
            break;
        case AccessibleStateType::ARMED:
            // No match
            break;
        case AccessibleStateType::BUSY:
            state->busy = true;
            break;
        case AccessibleStateType::CHECKED:
            state->checked = true;
            break;
        case AccessibleStateType::EDITABLE:
            state->editable = true;
            break;
        case AccessibleStateType::ENABLED:
            state->disabled = false;
            break;
        case AccessibleStateType::EXPANDABLE:
            state->expandable = true;
            break;
        case AccessibleStateType::FOCUSABLE:
            state->focusable = true;
            break;
        case AccessibleStateType::FOCUSED:
            state->focused = true;
            break;
        case AccessibleStateType::HORIZONTAL:
            // No match
            break;
        case AccessibleStateType::ICONIFIED:
            // No match
            break;
        case AccessibleStateType::INDETERMINATE:
            // No match
            break;
        case AccessibleStateType::MANAGES_DESCENDANTS:
            // No match
            break;
        case AccessibleStateType::MODAL:
            state->modal = true;
            break;
        case AccessibleStateType::OPAQUE:
            // No match
            break;
        case AccessibleStateType::PRESSED:
            state->pressed = true;
            break;
        case AccessibleStateType::RESIZABLE:
            state->sizeable = true;
            break;
        case AccessibleStateType::SELECTABLE:
            state->selectable = true;
            break;
        case AccessibleStateType::SELECTED:
            state->selected = true;
            break;
        case AccessibleStateType::SENSITIVE:
            // No match
            break;
        case AccessibleStateType::SHOWING:
            // No match
            break;
        case AccessibleStateType::SINGLE_LINE:
            // No match
            break;
        case AccessibleStateType::STALE:
            // No match
            break;
        case AccessibleStateType::TRANSIENT:
            // No match
            break;
        case AccessibleStateType::VERTICAL:
            // No match
            break;
        case AccessibleStateType::VISIBLE:
            state->invisible = false;
            break;
        case AccessibleStateType::DEFAULT:
            // No match
            break;
        case AccessibleStateType::DEFUNC:
            state->invalid = true;
            break;
        case AccessibleStateType::MULTI_SELECTABLE:
            state->multiSelectable = true;
            break;
        default:
            SAL_WARN("vcl.qt5", "Unmapped state: " << nState);
            break;
    }
}
}

QAccessible::State Qt5AccessibleWidget::state() const
{
    QAccessible::State state;

    if (!m_xAccessible.is())
        return state;
    Reference<XAccessibleStateSet> xStateSet(
        m_xAccessible->getAccessibleContext()->getAccessibleStateSet());

    if (!xStateSet.is())
        return state;

    Sequence<sal_Int16> aStates = xStateSet->getStates();

    for (sal_Int32 n = 0; n < aStates.getLength(); n++)
    {
        lcl_addState(&state, n);
    }

    return state;
}

QColor Qt5AccessibleWidget::foregroundColor() const
{
    Reference<XAccessibleComponent> xAccessibleComponent(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    return toQColor(xAccessibleComponent->getForeground());
}
QColor Qt5AccessibleWidget::backgroundColor() const
{
    Reference<XAccessibleComponent> xAccessibleComponent(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    return toQColor(xAccessibleComponent->getBackground());
}

void* Qt5AccessibleWidget::interface_cast(QAccessible::InterfaceType t)
{
    if (t == QAccessible::ActionInterface)
        return static_cast<QAccessibleActionInterface*>(this);
    return nullptr;
}

bool Qt5AccessibleWidget::isValid() const
{
    return m_xAccessible.is() && m_xAccessible->getAccessibleContext().is();
}

QObject* Qt5AccessibleWidget::object() const { return nullptr; }

void Qt5AccessibleWidget::setText(QAccessible::Text /* t */, const QString& /* text */) {}

QAccessibleInterface* Qt5AccessibleWidget::childAt(int x, int y) const
{
    Reference<XAccessibleComponent> xAccessibleComponent(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    return QAccessible::queryAccessibleInterface(
        new Qt5XAccessible(xAccessibleComponent->getAccessibleAtPoint(awt::Point(x, y))));
}

QAccessibleInterface* Qt5AccessibleWidget::customFactory(const QString& classname, QObject* object)
{
    if (classname == QLatin1String("Qt5Widget") && object && object->isWidgetType())
    {
        Qt5Widget* pWidget = static_cast<Qt5Widget*>(object);
        return new Qt5AccessibleWidget(pWidget->m_pFrame->GetWindow()->GetAccessible());
    }
    if (classname == QLatin1String("Qt5XAccessible") && object)
    {
        if (dynamic_cast<Qt5XAccessible*>(object) != nullptr)
        {
            Qt5XAccessible* pVclWindow = static_cast<Qt5XAccessible*>(object);
            return new Qt5AccessibleWidget(pVclWindow->m_xAccessible);
        }
    }

    return nullptr;
}

// QAccessibleActionInterface
QStringList Qt5AccessibleWidget::actionNames() const
{
    QStringList actionNames;
    Reference<XAccessibleAction> xAccessibleAction(m_xAccessible, UNO_QUERY);
    if (!xAccessibleAction.is())
        return actionNames;

    int count = xAccessibleAction->getAccessibleActionCount();
    for (int i = 0; i < count; i++)
    {
        OUString desc = xAccessibleAction->getAccessibleActionDescription(i);
        actionNames.append(toQString(desc));
    }
    return actionNames;
}

void Qt5AccessibleWidget::doAction(const QString& actionName)
{
    Reference<XAccessibleAction> xAccessibleAction(m_xAccessible, UNO_QUERY);
    if (!xAccessibleAction.is())
        return;

    int index = actionNames().indexOf(actionName);
    if (index == -1)
        return;
    xAccessibleAction->doAccessibleAction(index);
}

QStringList Qt5AccessibleWidget::keyBindingsForAction(const QString& actionName) const
{
    QStringList keyBindings;
    Reference<XAccessibleAction> xAccessibleAction(m_xAccessible, UNO_QUERY);
    if (!xAccessibleAction.is())
        return keyBindings;

    int index = actionNames().indexOf(actionName);
    if (index == -1)
        return keyBindings;

    Reference<XAccessibleKeyBinding> xKeyBinding
        = xAccessibleAction->getAccessibleActionKeyBinding(index);

    int count = xKeyBinding->getAccessibleKeyBindingCount();
    for (int i = 0; i < count; i++)
    {
        Sequence<awt::KeyStroke> keyStroke = xKeyBinding->getAccessibleKeyBinding(i);
        keyBindings.append(toQString(comphelper::GetkeyBindingStrByXkeyBinding(keyStroke)));
    }
    return keyBindings;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
