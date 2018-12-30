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

#include <Qt5AccessibleEventListener.hxx>
#include <Qt5Frame.hxx>
#include <Qt5Tools.hxx>
#include <Qt5Widget.hxx>
#include <Qt5XAccessible.hxx>

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/AccessibleImplementationHelper.hxx>
#include <sal/log.hxx>
#include <vcl/popupmenuwindow.hxx>

using namespace css;
using namespace css::accessibility;
using namespace css::beans;
using namespace css::uno;

Qt5AccessibleWidget::Qt5AccessibleWidget(const Reference<XAccessible> xAccessible)
    : m_xAccessible(xAccessible)
{
    Reference<XAccessibleContext> xContext = xAccessible->getAccessibleContext();
    Reference<XAccessibleEventBroadcaster> xBroadcaster(xContext, UNO_QUERY);
    if (xBroadcaster.is())
    {
        Reference<XAccessibleEventListener> xListener(
            new Qt5AccessibleEventListener(xAccessible, this));
        xBroadcaster->addAccessibleEventListener(xListener);
    }
}

QWindow* Qt5AccessibleWidget::window() const { return nullptr; }

int Qt5AccessibleWidget::childCount() const
{
    return m_xAccessible->getAccessibleContext()->getAccessibleChildCount();
}

int Qt5AccessibleWidget::indexOfChild(const QAccessibleInterface* /* child */) const { return 0; }

namespace
{
QAccessible::Relation lcl_matchUnoRelation(short relationType)
{
    switch (relationType)
    {
        case AccessibleRelationType::CONTROLLER_FOR:
            return QAccessible::Controller;
        case AccessibleRelationType::CONTROLLED_BY:
            return QAccessible::Controlled;
        case AccessibleRelationType::LABEL_FOR:
            return QAccessible::Label;
        case AccessibleRelationType::LABELED_BY:
            return QAccessible::Labelled;
        case AccessibleRelationType::INVALID:
        case AccessibleRelationType::CONTENT_FLOWS_FROM:
        case AccessibleRelationType::CONTENT_FLOWS_TO:
        case AccessibleRelationType::MEMBER_OF:
        case AccessibleRelationType::SUB_WINDOW_OF:
        case AccessibleRelationType::NODE_CHILD_OF:
        case AccessibleRelationType::DESCRIBED_BY:
        default:
            SAL_WARN("vcl.qt5", "Unmatched relation: " << relationType);
            return nullptr;
    }
}

short lcl_matchQtRelation(QAccessible::Relation relationType)
{
    switch (relationType)
    {
        case QAccessible::Controller:
            return AccessibleRelationType::CONTROLLER_FOR;
        case QAccessible::Controlled:
            return AccessibleRelationType::CONTROLLED_BY;
        case QAccessible::Label:
            return AccessibleRelationType::LABEL_FOR;
        case QAccessible::Labelled:
            return AccessibleRelationType::LABELED_BY;
        default:
            SAL_WARN("vcl.qt5", "Unmatched relation: " << relationType);
    }
    return 0;
}

void lcl_appendRelation(QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>* relations,
                        AccessibleRelation aRelation)
{
    QAccessible::Relation aQRelation = lcl_matchUnoRelation(aRelation.RelationType);
    sal_uInt32 nTargetCount = aRelation.TargetSet.getLength();

    for (sal_uInt32 i = 0; i < nTargetCount; i++)
    {
        Reference<XAccessible> xAccessible(aRelation.TargetSet[i], uno::UNO_QUERY);
        relations->append(
            { QAccessible::queryAccessibleInterface(new Qt5XAccessible(xAccessible)), aQRelation });
    }
}
}

QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
Qt5AccessibleWidget::relations(QAccessible::Relation match) const
{
    QVector<QPair<QAccessibleInterface*, QAccessible::Relation>> relations;
    Reference<XAccessibleRelationSet> xRelationSet
        = m_xAccessible->getAccessibleContext()->getAccessibleRelationSet();
    if (!xRelationSet.is())
        return relations;

    if (match == QAccessible::AllRelations)
    {
        int count = xRelationSet->getRelationCount();
        for (int i = 0; i < count; i++)
        {
            AccessibleRelation aRelation = xRelationSet->getRelation(i);
            lcl_appendRelation(&relations, aRelation);
        }
    }
    else
    {
        AccessibleRelation aRelation = xRelationSet->getRelation(lcl_matchQtRelation(match));
        lcl_appendRelation(&relations, aRelation);
    }

    return relations;
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
    if (t == QAccessible::TextInterface)
        return static_cast<QAccessibleTextInterface*>(this);
    if (t == QAccessible::EditableTextInterface)
        return static_cast<QAccessibleEditableTextInterface*>(this);
    if (t == QAccessible::ValueInterface)
        return static_cast<QAccessibleValueInterface*>(this);
    if (t == QAccessible::TableInterface)
        return static_cast<QAccessibleTableInterface*>(this);
    return nullptr;
}

bool Qt5AccessibleWidget::isValid() const
{
    if (m_xAccessible.is())
    {
        try
        {
            // getAccessibleContext throws RuntimeException if context is no longer alive
            Reference<XAccessibleContext> xAc = m_xAccessible->getAccessibleContext();
            return xAc.is();
        }
        // so let's catch it here, cuz otherwise soffice falls flat on its face
        // with FatalError and nothing else
        catch (css::uno::RuntimeException /*ex*/)
        {
            return false;
        }
    }
    return false;
}

QObject* Qt5AccessibleWidget::object() const { return nullptr; }

void Qt5AccessibleWidget::setText(QAccessible::Text /* t */, const QString& /* text */) {}

QAccessibleInterface* Qt5AccessibleWidget::childAt(int x, int y) const
{
    if (!m_xAccessible.is())
        return nullptr;

    Reference<XAccessibleContext> xAc;
    try
    {
        xAc = m_xAccessible->getAccessibleContext();
    }
    catch (css::lang::DisposedException /*ex*/)
    {
        return nullptr;
    }

    Reference<XAccessibleComponent> xAccessibleComponent(xAc, UNO_QUERY);
    return QAccessible::queryAccessibleInterface(
        new Qt5XAccessible(xAccessibleComponent->getAccessibleAtPoint(awt::Point(x, y))));
}

QAccessibleInterface* Qt5AccessibleWidget::customFactory(const QString& classname, QObject* object)
{
    if (classname == QLatin1String("Qt5Widget") && object && object->isWidgetType())
    {
        Qt5Widget* pWidget = static_cast<Qt5Widget*>(object);
        vcl::Window* pWindow = pWidget->m_pFrame->GetWindow();

        if (pWindow)
            return new Qt5AccessibleWidget(pWindow->GetAccessible());
    }
    if (classname == QLatin1String("Qt5XAccessible") && object)
    {
        Qt5XAccessible* pXAccessible = dynamic_cast<Qt5XAccessible*>(object);
        if (pXAccessible && pXAccessible->m_xAccessible.is())
            return new Qt5AccessibleWidget(pXAccessible->m_xAccessible);
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

    if (!xKeyBinding.is())
        return keyBindings;

    int count = xKeyBinding->getAccessibleKeyBindingCount();
    for (int i = 0; i < count; i++)
    {
        Sequence<awt::KeyStroke> keyStroke = xKeyBinding->getAccessibleKeyBinding(i);
        keyBindings.append(toQString(comphelper::GetkeyBindingStrByXkeyBinding(keyStroke)));
    }
    return keyBindings;
}

QAccessibleValueInterface* Qt5AccessibleWidget::valueInterface() { return nullptr; }

QAccessibleTextInterface* Qt5AccessibleWidget::textInterface() { return nullptr; }

// QAccessibleTextInterface
void Qt5AccessibleWidget::addSelection(int /* startOffset */, int /* endOffset */)
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::addSelection");
}

namespace
{
OUString lcl_convertFontWeight(double fontWeight)
{
    if (fontWeight == awt::FontWeight::THIN || fontWeight == awt::FontWeight::ULTRALIGHT)
        return OUString("100");
    if (fontWeight == awt::FontWeight::LIGHT)
        return OUString("200");
    if (fontWeight == awt::FontWeight::SEMILIGHT)
        return OUString("300");
    if (fontWeight == awt::FontWeight::NORMAL)
        return OUString("normal");
    if (fontWeight == awt::FontWeight::SEMIBOLD)
        return OUString("500");
    if (fontWeight == awt::FontWeight::BOLD)
        return OUString("bold");
    if (fontWeight == awt::FontWeight::ULTRABOLD)
        return OUString("800");
    if (fontWeight == awt::FontWeight::BLACK)
        return OUString("900");

    // awt::FontWeight::DONTKNOW || fontWeight == awt::FontWeight::NORMAL
    return OUString("normal");
}
}

QString Qt5AccessibleWidget::attributes(int offset, int* startOffset, int* endOffset) const
{
    Reference<XAccessibleText> xText(m_xAccessible, UNO_QUERY);
    if (!xText.is())
        return QString();

    Sequence<PropertyValue> attribs = xText->getCharacterAttributes(offset, Sequence<OUString>());
    const PropertyValue* pValues = attribs.getConstArray();
    OUString aRet;
    for (sal_Int32 i = 0; i < attribs.getLength(); i++)
    {
        if (pValues[i].Name == "CharFontName")
        {
            OUString aStr;
            pValues[i].Value >>= aStr;
            aRet += "font-family:" + aStr + ";";
            continue;
        }
        if (pValues[i].Name == "CharHeight")
        {
            double fHeight;
            pValues[i].Value >>= fHeight;
            aRet += "font-size:" + OUString::number(fHeight) + "pt;";
            continue;
        }
        if (pValues[i].Name == "CharWeight")
        {
            double fWeight;
            pValues[i].Value >>= fWeight;
            aRet += "font-weight:" + lcl_convertFontWeight(fWeight) + ";";
            continue;
        }
    }
    *startOffset = offset;
    *endOffset = offset + 1;
    return toQString(aRet);
}
int Qt5AccessibleWidget::characterCount() const
{
    Reference<XAccessibleText> xText(m_xAccessible, UNO_QUERY);
    if (xText.is())
        return xText->getCharacterCount();
    return 0;
}
QRect Qt5AccessibleWidget::characterRect(int /* offset */) const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::characterRect");
    return QRect();
}
int Qt5AccessibleWidget::cursorPosition() const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::cursorPosition");
    return 0;
}
int Qt5AccessibleWidget::offsetAtPoint(const QPoint& /* point */) const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::offsetAtPoint");
    return 0;
}
void Qt5AccessibleWidget::removeSelection(int /* selectionIndex */)
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::removeSelection");
}
void Qt5AccessibleWidget::scrollToSubstring(int /* startIndex */, int /* endIndex */)
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::scrollToSubstring");
}

void Qt5AccessibleWidget::selection(int selectionIndex, int* startOffset, int* endOffset) const
{
    if (!startOffset && !endOffset)
        return;

    Reference<XAccessibleText> xText;
    if (selectionIndex == 0)
        xText = Reference<XAccessibleText>(m_xAccessible, UNO_QUERY);

    if (startOffset)
        *startOffset = xText.is() ? xText->getSelectionStart() : 0;
    if (endOffset)
        *endOffset = xText.is() ? xText->getSelectionEnd() : 0;
}

int Qt5AccessibleWidget::selectionCount() const
{
    Reference<XAccessibleText> xText(m_xAccessible, UNO_QUERY);
    if (xText.is() && !xText->getSelectedText().isEmpty())
        return 1; // Only 1 selection supported atm
    return 0;
}
void Qt5AccessibleWidget::setCursorPosition(int position)
{
    Reference<XAccessibleText> xText(m_xAccessible, UNO_QUERY);
    if (xText.is())
        xText->setCaretPosition(position);
}
void Qt5AccessibleWidget::setSelection(int /* selectionIndex */, int startOffset, int endOffset)
{
    Reference<XAccessibleText> xText(m_xAccessible, UNO_QUERY);
    if (xText.is())
        xText->setSelection(startOffset, endOffset);
}
QString Qt5AccessibleWidget::text(int startOffset, int endOffset) const
{
    Reference<XAccessibleText> xText(m_xAccessible, UNO_QUERY);
    if (xText.is())
        return toQString(xText->getTextRange(startOffset, endOffset));
    return QString();
}
QString Qt5AccessibleWidget::textAfterOffset(int /* offset */,
                                             QAccessible::TextBoundaryType /* boundaryType */,
                                             int* /* startOffset */, int* /* endOffset */) const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::textAfterOffset");
    return QString();
}
QString Qt5AccessibleWidget::textAtOffset(int /* offset */,
                                          QAccessible::TextBoundaryType /* boundaryType */,
                                          int* /* startOffset */, int* /* endOffset */) const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::textAtOffset");
    return QString();
}
QString Qt5AccessibleWidget::textBeforeOffset(int /* offset */,
                                              QAccessible::TextBoundaryType /* boundaryType */,
                                              int* /* startOffset */, int* /* endOffset */) const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTextInterface::textBeforeOffset");
    return QString();
}

// QAccessibleEditableTextInterface

void Qt5AccessibleWidget::deleteText(int startOffset, int endOffset)
{
    Reference<XAccessibleEditableText> xEditableText(m_xAccessible->getAccessibleContext(),
                                                     UNO_QUERY);
    if (!xEditableText.is())
        return;
    xEditableText->deleteText(startOffset, endOffset);
}

void Qt5AccessibleWidget::insertText(int offset, const QString& text)
{
    Reference<XAccessibleEditableText> xEditableText(m_xAccessible->getAccessibleContext(),
                                                     UNO_QUERY);
    if (!xEditableText.is())
        return;
    xEditableText->insertText(toOUString(text), offset);
}

void Qt5AccessibleWidget::replaceText(int startOffset, int endOffset, const QString& text)
{
    Reference<XAccessibleEditableText> xEditableText(m_xAccessible->getAccessibleContext(),
                                                     UNO_QUERY);
    if (!xEditableText.is())
        return;
    xEditableText->replaceText(startOffset, endOffset, toOUString(text));
}

// QAccessibleValueInterface
QVariant Qt5AccessibleWidget::currentValue() const
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getCurrentValue() >>= aDouble;
    return QVariant(aDouble);
}
QVariant Qt5AccessibleWidget::maximumValue() const
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getMaximumValue() >>= aDouble;
    return QVariant(aDouble);
}
QVariant Qt5AccessibleWidget::minimumStepSize() const { return QVariant(); }
QVariant Qt5AccessibleWidget::minimumValue() const
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getMinimumValue() >>= aDouble;
    return QVariant(aDouble);
}
void Qt5AccessibleWidget::setCurrentValue(const QVariant& value)
{
    Reference<XAccessibleValue> xValue(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xValue.is())
        return;
    xValue->setCurrentValue(Any(value.toDouble()));
}

// QAccessibleTable
QAccessibleInterface* Qt5AccessibleWidget::caption() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return nullptr;
    return QAccessible::queryAccessibleInterface(
        new Qt5XAccessible(xTable->getAccessibleCaption()));
}

QAccessibleInterface* Qt5AccessibleWidget::cellAt(int row, int column) const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return nullptr;
    return QAccessible::queryAccessibleInterface(
        new Qt5XAccessible(xTable->getAccessibleCellAt(row, column)));
}

int Qt5AccessibleWidget::columnCount() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getAccessibleColumnCount();
}

QString Qt5AccessibleWidget::columnDescription(int column) const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return QString();
    return toQString(xTable->getAccessibleColumnDescription(column));
}

bool Qt5AccessibleWidget::isColumnSelected(int /* column */) const { return true; }

bool Qt5AccessibleWidget::isRowSelected(int /* row */) const { return true; }

void Qt5AccessibleWidget::modelChange(QAccessibleTableModelChangeEvent*) {}

int Qt5AccessibleWidget::rowCount() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getAccessibleRowCount();
}

QString Qt5AccessibleWidget::rowDescription(int row) const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return QString();
    return toQString(xTable->getAccessibleRowDescription(row));
}

bool Qt5AccessibleWidget::selectColumn(int column)
{
    Reference<XAccessibleTableSelection> xTableSelection(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->selectColumn(column);
}

bool Qt5AccessibleWidget::selectRow(int row)
{
    Reference<XAccessibleTableSelection> xTableSelection(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->selectRow(row);
}

int Qt5AccessibleWidget::selectedCellCount() const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTableInterface::selectedCellCount");
    return 0;
}

QList<QAccessibleInterface*> Qt5AccessibleWidget::selectedCells() const
{
    SAL_INFO("vcl.qt5", "Unsupported QAccessibleTableInterface::selectedCells");
    return QList<QAccessibleInterface*>();
}

int Qt5AccessibleWidget::selectedColumnCount() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getSelectedAccessibleColumns().getLength();
}

QList<int> Qt5AccessibleWidget::selectedColumns() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return QList<int>();
    return toQList(xTable->getSelectedAccessibleColumns());
}

int Qt5AccessibleWidget::selectedRowCount() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getSelectedAccessibleRows().getLength();
}

QList<int> Qt5AccessibleWidget::selectedRows() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return QList<int>();
    return toQList(xTable->getSelectedAccessibleRows());
}

QAccessibleInterface* Qt5AccessibleWidget::summary() const
{
    Reference<XAccessibleTable> xTable(m_xAccessible->getAccessibleContext(), UNO_QUERY);
    if (!xTable.is())
        return nullptr;
    return QAccessible::queryAccessibleInterface(
        new Qt5XAccessible(xTable->getAccessibleSummary()));
}

bool Qt5AccessibleWidget::unselectColumn(int column)
{
    Reference<XAccessibleTableSelection> xTableSelection(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->unselectColumn(column);
}

bool Qt5AccessibleWidget::unselectRow(int row)
{
    Reference<XAccessibleTableSelection> xTableSelection(m_xAccessible->getAccessibleContext(),
                                                         UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->unselectRow(row);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
