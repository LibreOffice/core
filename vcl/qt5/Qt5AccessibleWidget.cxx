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
#include <Qt5VclWindow.hxx>
#include <Qt5Widget.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>

#include <sal/log.hxx>
#include <vcl/popupmenuwindow.hxx>

using namespace css::accessibility;
using namespace css::uno;

Qt5AccessibleWidget::Qt5AccessibleWidget(Qt5Widget* pFrame, vcl::Window* pWindow)
    : m_pFrame(pFrame)
    , m_pWindow(pWindow)
{
}

Qt5AccessibleWidget::Qt5AccessibleWidget(vcl::Window* pWindow)
    : m_pWindow(pWindow)
{
}

QWindow* Qt5AccessibleWidget::window() const { return nullptr; }

int Qt5AccessibleWidget::childCount() const
{
    if (!m_pWindow.get())
        return 0;

    return m_pWindow->GetAccessibleChildWindowCount();
}

int Qt5AccessibleWidget::indexOfChild(const QAccessibleInterface* /* child */) const { return 0; }
QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
    Qt5AccessibleWidget::relations(QAccessible::Relation /* match */) const
{
    return QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>();
}

QAccessibleInterface* Qt5AccessibleWidget::focusChild() const
{
    if (m_pWindow->HasChildPathFocus())
        return QAccessible::queryAccessibleInterface(
            new Qt5VclWindow(Application::GetFocusWindow()));
    return QAccessible::queryAccessibleInterface(object());
}

QRect Qt5AccessibleWidget::rect() const
{
    if (!m_pWindow.get())
        return QRect();

    SolarMutexGuard aSolarGuard;

    // TODO: This seems to return a relative position (to the parent window).
    // Needs to be absolute instead.
    Point aPoint(m_pWindow->GetPosPixel());
    Size aSize(m_pWindow->GetSizePixel());

    return QRect(aPoint.X(), aPoint.Y(), aSize.Width(), aSize.Height());
}

QAccessibleInterface* Qt5AccessibleWidget::parent() const
{
    if (!m_pWindow)
        return QAccessible::queryAccessibleInterface(nullptr);

    return QAccessible::queryAccessibleInterface(
        new Qt5VclWindow(m_pWindow->GetAccessibleParentWindow()));
}
QAccessibleInterface* Qt5AccessibleWidget::child(int index) const
{
    if (!m_pWindow)
        return QAccessible::queryAccessibleInterface(nullptr);

    return QAccessible::queryAccessibleInterface(
        new Qt5VclWindow(m_pWindow->GetAccessibleChildWindow(index)));
}

QString Qt5AccessibleWidget::text(QAccessible::Text text) const
{
    if (!m_pWindow.get())
        return QString();

    SolarMutexGuard aSolarGuard;

    switch (text)
    {
        case QAccessible::Name:
            return toQString(m_pWindow->GetAccessibleName());
        case QAccessible::Description:
        case QAccessible::DebugDescription:
            return toQString(m_pWindow->GetAccessibleDescription());
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
    if (!m_pWindow.get())
        return QAccessible::NoRole;

    switch (m_pWindow->GetAccessibleRole())
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
            SolarMutexGuard aSolarGuard;
            WindowType type = WindowType::WINDOW;
            bool parentIsMenuFloatingWindow = false;

            vcl::Window* pParent = m_pWindow->GetParent();
            if (pParent)
            {
                type = pParent->GetType();
                parentIsMenuFloatingWindow = pParent->IsMenuFloatingWindow();
            }

            if ((WindowType::LISTBOX != type) && (WindowType::COMBOBOX != type)
                && (WindowType::MENUBARWINDOW != type) && !parentIsMenuFloatingWindow)
            {
                return QAccessible::Window;
            }
        }
            SAL_FALLTHROUGH;

        default:
        {
            SolarMutexGuard aSolarGuard;
            vcl::Window* pChild = m_pWindow->GetWindow(GetWindowType::FirstChild);
            if (pChild)
            {
                if (WindowType::HELPTEXTWINDOW == pChild->GetType())
                {
                    return QAccessible::HelpBalloon;
                }
                else if (m_pWindow->GetType() == WindowType::BORDERWINDOW
                         && pChild->GetType() == WindowType::FLOATINGWINDOW)
                {
                    PopupMenuFloatingWindow* p = dynamic_cast<PopupMenuFloatingWindow*>(pChild);
                    if (p && p->IsPopupMenu() && p->GetMenuStackLevel() == 0)
                    {
                        return QAccessible::PopupMenu;
                    }
                }
            }
            break;
        }
    }
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
            //state->horizontal = true;
            break;
        case AccessibleStateType::ICONIFIED:
            //state->iconified = true;
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

    Reference<XAccessible> xAccessible(m_pWindow->GetAccessible());
    if (!xAccessible.is())
        return state;
    Reference<XAccessibleStateSet> xStateSet(
        xAccessible->getAccessibleContext()->getAccessibleStateSet());

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
    return toQColor(m_pWindow->GetControlForeground());
}
QColor Qt5AccessibleWidget::backgroundColor() const
{
    return toQColor(m_pWindow->GetControlBackground());
}

void* Qt5AccessibleWidget::interface_cast(QAccessible::InterfaceType /* t */)
{
    /* if (t == QAccessible::ActionInterface)
        return static_cast<QAccessibleActionInterface*>(this); */
    return nullptr;
}

// QAccessibleActionInterface
/* QStringList Qt5AccessibleWidget::actionNames() const
{
    qDebug("Qt5AccessibleWidget::actionNames");
    QStringList actionNames;
    return actionNames;
}
void Qt5AccessibleWidget::doAction(const QString& actionName)
{
    qDebug("Qt5AccessibleWidget::doAction");
}
QStringList Qt5AccessibleWidget::keyBindingsForAction(const QString& actionName) const
{
    qDebug("Qt5AccessibleWidget::keyBindingsForAction");
    return QStringList();
} */

bool Qt5AccessibleWidget::isValid() const { return m_pWindow.get() != nullptr; }

QObject* Qt5AccessibleWidget::object() const { return nullptr; }

void Qt5AccessibleWidget::setText(QAccessible::Text t, const QString& text)
{
    if (!m_pWindow)
        return;

    switch (t)
    {
        case QAccessible::Name:
            m_pWindow->SetAccessibleName(toOUString(text));
            break;
        case QAccessible::Description:
        case QAccessible::DebugDescription:
            m_pWindow->SetAccessibleDescription(toOUString(text));
            break;
        case QAccessible::Value:
        case QAccessible::Help:
        case QAccessible::Accelerator:
        case QAccessible::UserText:
            break;
    }
}

QAccessibleInterface* Qt5AccessibleWidget::childAt(int /* x */, int /* y */) const
{
    return nullptr;
}

QAccessibleInterface* Qt5AccessibleWidget::customFactory(const QString& classname, QObject* object)
{
    if (classname == QLatin1String("Qt5Widget") && object && object->isWidgetType())
    {
        return new Qt5AccessibleWidget(static_cast<Qt5Widget*>(object),
                                       (static_cast<Qt5Widget*>(object))->m_pFrame->GetWindow());
    }
    if (classname == QLatin1String("Qt5VclWindow") && object)
    {
        if (dynamic_cast<Qt5VclWindow*>(object) != nullptr)
            return new Qt5AccessibleWidget((static_cast<Qt5VclWindow*>(object))->m_pWindow);
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
