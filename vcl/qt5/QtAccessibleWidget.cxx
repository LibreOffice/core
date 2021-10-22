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

#include <QtAccessibleWidget.hxx>
#include <QtAccessibleWidget.moc>

#include <QtGui/QAccessibleInterface>

#include <QtAccessibleEventListener.hxx>
#include <QtFrame.hxx>
#include <QtTools.hxx>
#include <QtWidget.hxx>
#include <QtXAccessible.hxx>

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
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
#include <o3tl/any.hxx>
#include <sal/log.hxx>

using namespace css;
using namespace css::accessibility;
using namespace css::beans;
using namespace css::uno;

QtAccessibleWidget::QtAccessibleWidget(const Reference<XAccessible> xAccessible, QObject* pObject)
    : m_xAccessible(xAccessible)
    , m_pObject(pObject)
{
    Reference<XAccessibleContext> xContext = xAccessible->getAccessibleContext();
    Reference<XAccessibleEventBroadcaster> xBroadcaster(xContext, UNO_QUERY);
    if (xBroadcaster.is())
    {
        Reference<XAccessibleEventListener> xListener(
            new QtAccessibleEventListener(xAccessible, this));
        xBroadcaster->addAccessibleEventListener(xListener);
    }
}

Reference<XAccessibleContext> QtAccessibleWidget::getAccessibleContextImpl() const
{
    Reference<XAccessibleContext> xAc;

    if (m_xAccessible.is())
    {
        try
        {
            xAc = m_xAccessible->getAccessibleContext();
        }
        catch (css::lang::DisposedException /*ex*/)
        {
            SAL_WARN("vcl.qt", "Accessible context disposed already");
        }
        // sometimes getAccessibleContext throws also RuntimeException if context is no longer alive
        catch (css::uno::RuntimeException /*ex*/)
        {
            // so let's catch it here, cuz otherwise soffice falls flat on its face
            // with FatalError and nothing else
            SAL_WARN("vcl.qt", "Accessible context no longer alive");
        }
    }

    return xAc;
}

css::uno::Reference<css::accessibility::XAccessibleTable>
QtAccessibleWidget::getAccessibleTableForParent() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return nullptr;

    Reference<XAccessible> xParent = xAcc->getAccessibleParent();
    if (!xParent.is())
        return nullptr;

    Reference<XAccessibleContext> xParentContext = xParent->getAccessibleContext();
    if (!xParentContext.is())
        return nullptr;

    return Reference<XAccessibleTable>(xParentContext, UNO_QUERY);
}

QWindow* QtAccessibleWidget::window() const { return nullptr; }

int QtAccessibleWidget::childCount() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return 0;

    return xAc->getAccessibleChildCount();
}

int QtAccessibleWidget::indexOfChild(const QAccessibleInterface* /* child */) const { return 0; }

namespace
{
sal_Int16 lcl_matchQtTextBoundaryType(QAccessible::TextBoundaryType boundaryType)
{
    switch (boundaryType)
    {
        case QAccessible::CharBoundary:
            return com::sun::star::accessibility::AccessibleTextType::CHARACTER;
        case QAccessible::WordBoundary:
            return com::sun::star::accessibility::AccessibleTextType::WORD;
        case QAccessible::SentenceBoundary:
            return com::sun::star::accessibility::AccessibleTextType::SENTENCE;
        case QAccessible::ParagraphBoundary:
            return com::sun::star::accessibility::AccessibleTextType::PARAGRAPH;
        case QAccessible::LineBoundary:
            return com::sun::star::accessibility::AccessibleTextType::LINE;
        case QAccessible::NoBoundary:
            // assert here, better handle it directly at call site
            assert(false
                   && "No match for QAccessible::NoBoundary, handle it separately at call site.");
            break;
        default:
            break;
    }

    SAL_WARN("vcl.qt", "Unmatched text boundary type: " << boundaryType);
    return -1;
}

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
            SAL_WARN("vcl.qt", "Unmatched relation: " << relationType);
            return {};
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
            SAL_WARN("vcl.qt", "Unmatched relation: " << relationType);
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
            { QAccessible::queryAccessibleInterface(new QtXAccessible(xAccessible)), aQRelation });
    }
}
}

QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
QtAccessibleWidget::relations(QAccessible::Relation match) const
{
    QVector<QPair<QAccessibleInterface*, QAccessible::Relation>> relations;

    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return relations;

    Reference<XAccessibleRelationSet> xRelationSet = xAc->getAccessibleRelationSet();
    if (xRelationSet.is())
    {
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
    }

    return relations;
}

QAccessibleInterface* QtAccessibleWidget::focusChild() const
{
    /* if (m_pWindow->HasChildPathFocus())
        return QAccessible::queryAccessibleInterface(
            new QtXAccessible(m_xAccessible->getAccessibleContext()->getAccessibleChild(index))); */
    return QAccessible::queryAccessibleInterface(object());
}

QRect QtAccessibleWidget::rect() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QRect();

    Reference<XAccessibleComponent> xAccessibleComponent(xAc, UNO_QUERY);
    awt::Point aPoint = xAccessibleComponent->getLocation();
    awt::Size aSize = xAccessibleComponent->getSize();

    return QRect(aPoint.X, aPoint.Y, aSize.Width, aSize.Height);
}

QAccessibleInterface* QtAccessibleWidget::parent() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    return QAccessible::queryAccessibleInterface(new QtXAccessible(xAc->getAccessibleParent()));
}
QAccessibleInterface* QtAccessibleWidget::child(int index) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    return QAccessible::queryAccessibleInterface(new QtXAccessible(xAc->getAccessibleChild(index)));
}

QString QtAccessibleWidget::text(QAccessible::Text text) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QString();

    switch (text)
    {
        case QAccessible::Name:
            return toQString(xAc->getAccessibleName());
        case QAccessible::Description:
        case QAccessible::DebugDescription:
            return toQString(xAc->getAccessibleDescription());
        case QAccessible::Value:
        case QAccessible::Help:
        case QAccessible::Accelerator:
        case QAccessible::UserText:
        default:
            return QString("Unknown");
    }
}
QAccessible::Role QtAccessibleWidget::role() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QAccessible::NoRole;

    switch (xAc->getAccessibleRole())
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

    SAL_WARN("vcl.qt", "Unmapped role: " << getAccessibleContextImpl()->getAccessibleRole());
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
        case AccessibleStateType::EXPANDED:
            state->expanded = true;
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
        case AccessibleStateType::MOVEABLE:
            state->movable = true;
            break;
        case AccessibleStateType::MULTI_LINE:
            state->multiLine = true;
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
            SAL_WARN("vcl.qt", "Unmapped state: " << nState);
            break;
    }
}
}

QAccessible::State QtAccessibleWidget::state() const
{
    QAccessible::State state;

    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return state;

    Reference<XAccessibleStateSet> xStateSet(xAc->getAccessibleStateSet());

    if (!xStateSet.is())
        return state;

    const Sequence<sal_Int16> aStates = xStateSet->getStates();

    for (const sal_Int16 nState : aStates)
    {
        lcl_addState(&state, nState);
    }

    return state;
}

QColor QtAccessibleWidget::foregroundColor() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QColor();

    Reference<XAccessibleComponent> xAccessibleComponent(xAc, UNO_QUERY);
    return toQColor(Color(ColorTransparency, xAccessibleComponent->getForeground()));
}

QColor QtAccessibleWidget::backgroundColor() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QColor();

    Reference<XAccessibleComponent> xAccessibleComponent(xAc, UNO_QUERY);
    return toQColor(Color(ColorTransparency, xAccessibleComponent->getBackground()));
}

void* QtAccessibleWidget::interface_cast(QAccessible::InterfaceType t)
{
    if (t == QAccessible::ActionInterface)
        return static_cast<QAccessibleActionInterface*>(this);
    if (t == QAccessible::TextInterface)
        return static_cast<QAccessibleTextInterface*>(this);
    if (t == QAccessible::EditableTextInterface)
        return static_cast<QAccessibleEditableTextInterface*>(this);
    if (t == QAccessible::ValueInterface)
        return static_cast<QAccessibleValueInterface*>(this);
    if (t == QAccessible::TableCellInterface)
        return static_cast<QAccessibleTableCellInterface*>(this);
    if (t == QAccessible::TableInterface)
        return static_cast<QAccessibleTableInterface*>(this);
    return nullptr;
}

bool QtAccessibleWidget::isValid() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    return xAc.is();
}

QObject* QtAccessibleWidget::object() const { return m_pObject; }

void QtAccessibleWidget::setText(QAccessible::Text /* t */, const QString& /* text */) {}

QAccessibleInterface* QtAccessibleWidget::childAt(int x, int y) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    Reference<XAccessibleComponent> xAccessibleComponent(xAc, UNO_QUERY);
    return QAccessible::queryAccessibleInterface(
        new QtXAccessible(xAccessibleComponent->getAccessibleAtPoint(awt::Point(x, y))));
}

QAccessibleInterface* QtAccessibleWidget::customFactory(const QString& classname, QObject* object)
{
    if (classname == QLatin1String("QtWidget") && object && object->isWidgetType())
    {
        QtWidget* pWidget = static_cast<QtWidget*>(object);
        vcl::Window* pWindow = pWidget->frame().GetWindow();

        if (pWindow)
            return new QtAccessibleWidget(pWindow->GetAccessible(), object);
    }
    if (classname == QLatin1String("QtXAccessible") && object)
    {
        QtXAccessible* pXAccessible = dynamic_cast<QtXAccessible*>(object);
        if (pXAccessible && pXAccessible->m_xAccessible.is())
            return new QtAccessibleWidget(pXAccessible->m_xAccessible, object);
    }

    return nullptr;
}

// QAccessibleActionInterface
QStringList QtAccessibleWidget::actionNames() const
{
    QStringList actionNames;
    Reference<XAccessibleAction> xAccessibleAction(getAccessibleContextImpl(), UNO_QUERY);
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

void QtAccessibleWidget::doAction(const QString& actionName)
{
    Reference<XAccessibleAction> xAccessibleAction(getAccessibleContextImpl(), UNO_QUERY);
    if (!xAccessibleAction.is())
        return;

    int index = actionNames().indexOf(actionName);
    if (index == -1)
        return;
    xAccessibleAction->doAccessibleAction(index);
}

QStringList QtAccessibleWidget::keyBindingsForAction(const QString& actionName) const
{
    QStringList keyBindings;
    Reference<XAccessibleAction> xAccessibleAction(getAccessibleContextImpl(), UNO_QUERY);
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

// QAccessibleTextInterface
void QtAccessibleWidget::addSelection(int /* startOffset */, int /* endOffset */)
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::addSelection");
}

namespace
{
OUString lcl_convertFontWeight(double fontWeight)
{
    if (fontWeight == awt::FontWeight::THIN || fontWeight == awt::FontWeight::ULTRALIGHT)
        return "100";
    if (fontWeight == awt::FontWeight::LIGHT)
        return "200";
    if (fontWeight == awt::FontWeight::SEMILIGHT)
        return "300";
    if (fontWeight == awt::FontWeight::NORMAL)
        return "normal";
    if (fontWeight == awt::FontWeight::SEMIBOLD)
        return "500";
    if (fontWeight == awt::FontWeight::BOLD)
        return "bold";
    if (fontWeight == awt::FontWeight::ULTRABOLD)
        return "800";
    if (fontWeight == awt::FontWeight::BLACK)
        return "900";

    // awt::FontWeight::DONTKNOW || fontWeight == awt::FontWeight::NORMAL
    return "normal";
}
}

QString QtAccessibleWidget::attributes(int offset, int* startOffset, int* endOffset) const
{
    if (startOffset == nullptr || endOffset == nullptr)
        return QString();

    *startOffset = -1;
    *endOffset = -1;

    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return QString();

    // handle special values for offset the same way base class's QAccessibleTextWidget::attributes does
    // (as defined in IAccessible 2: -1 -> length, -2 -> cursor position)
    if (offset == -2)
        offset = cursorPosition();

    const int nTextLength = characterCount();
    if (offset == -1 || offset == nTextLength)
        offset = nTextLength - 1;

    if (offset < 0 || offset > nTextLength)
        return QString();

    const Sequence<PropertyValue> attribs
        = xText->getCharacterAttributes(offset, Sequence<OUString>());
    OUString aRet;
    for (PropertyValue const& prop : attribs)
    {
        OUString sAttribute;
        OUString sValue;
        if (prop.Name == "CharFontName")
        {
            sAttribute = "font-family";
            sValue = *o3tl::doAccess<OUString>(prop.Value);
        }
        else if (prop.Name == "CharHeight")
        {
            sAttribute = "font-size";
            sValue = OUString::number(*o3tl::doAccess<double>(prop.Value)) + "pt";
        }
        else if (prop.Name == "CharWeight")
        {
            sAttribute = "font-weight";
            sValue = lcl_convertFontWeight(*o3tl::doAccess<double>(prop.Value));
        }

        if (!sAttribute.isEmpty() && !sValue.isEmpty())
            aRet += sAttribute + ":" + sValue + ";";
    }
    *startOffset = offset;
    *endOffset = offset + 1;
    return toQString(aRet);
}

int QtAccessibleWidget::characterCount() const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        return xText->getCharacterCount();
    return 0;
}
QRect QtAccessibleWidget::characterRect(int /* offset */) const
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::characterRect");
    return QRect();
}

int QtAccessibleWidget::cursorPosition() const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        return xText->getCaretPosition();
    return 0;
}

int QtAccessibleWidget::offsetAtPoint(const QPoint& /* point */) const
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::offsetAtPoint");
    return 0;
}
void QtAccessibleWidget::removeSelection(int /* selectionIndex */)
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::removeSelection");
}
void QtAccessibleWidget::scrollToSubstring(int startIndex, int endIndex)
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        xText->scrollSubstringTo(startIndex, endIndex, AccessibleScrollType_SCROLL_ANYWHERE);
}

void QtAccessibleWidget::selection(int selectionIndex, int* startOffset, int* endOffset) const
{
    if (!startOffset && !endOffset)
        return;

    Reference<XAccessibleText> xText;
    if (selectionIndex == 0)
        xText = Reference<XAccessibleText>(getAccessibleContextImpl(), UNO_QUERY);

    if (startOffset)
        *startOffset = xText.is() ? xText->getSelectionStart() : 0;
    if (endOffset)
        *endOffset = xText.is() ? xText->getSelectionEnd() : 0;
}

int QtAccessibleWidget::selectionCount() const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is() && !xText->getSelectedText().isEmpty())
        return 1; // Only 1 selection supported atm
    return 0;
}
void QtAccessibleWidget::setCursorPosition(int position)
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        xText->setCaretPosition(position);
}
void QtAccessibleWidget::setSelection(int /* selectionIndex */, int startOffset, int endOffset)
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        xText->setSelection(startOffset, endOffset);
}
QString QtAccessibleWidget::text(int startOffset, int endOffset) const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        return toQString(xText->getTextRange(startOffset, endOffset));
    return QString();
}
QString QtAccessibleWidget::textAfterOffset(int /* offset */,
                                            QAccessible::TextBoundaryType /* boundaryType */,
                                            int* /* startOffset */, int* /* endOffset */) const
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::textAfterOffset");
    return QString();
}

QString QtAccessibleWidget::textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType,
                                         int* startOffset, int* endOffset) const
{
    if (startOffset == nullptr || endOffset == nullptr)
        return QString();

    if (boundaryType == QAccessible::NoBoundary)
    {
        const int nCharCount = characterCount();
        *startOffset = 0;
        *endOffset = nCharCount;
        return text(0, nCharCount);
    }

    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return QString();

    sal_Int16 nUnoBoundaryType = lcl_matchQtTextBoundaryType(boundaryType);
    assert(nUnoBoundaryType > 0);

    const TextSegment segment = xText->getTextAtIndex(offset, nUnoBoundaryType);
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;
    return toQString(segment.SegmentText);
}

QString QtAccessibleWidget::textBeforeOffset(int /* offset */,
                                             QAccessible::TextBoundaryType /* boundaryType */,
                                             int* /* startOffset */, int* /* endOffset */) const
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::textBeforeOffset");
    return QString();
}

// QAccessibleEditableTextInterface

void QtAccessibleWidget::deleteText(int startOffset, int endOffset)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return;

    Reference<XAccessibleEditableText> xEditableText(xAc, UNO_QUERY);
    if (!xEditableText.is())
        return;
    xEditableText->deleteText(startOffset, endOffset);
}

void QtAccessibleWidget::insertText(int offset, const QString& text)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return;

    Reference<XAccessibleEditableText> xEditableText(xAc, UNO_QUERY);
    if (!xEditableText.is())
        return;
    xEditableText->insertText(toOUString(text), offset);
}

void QtAccessibleWidget::replaceText(int startOffset, int endOffset, const QString& text)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return;

    Reference<XAccessibleEditableText> xEditableText(xAc, UNO_QUERY);
    if (!xEditableText.is())
        return;
    xEditableText->replaceText(startOffset, endOffset, toOUString(text));
}

// QAccessibleValueInterface
QVariant QtAccessibleWidget::currentValue() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QVariant();

    Reference<XAccessibleValue> xValue(xAc, UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getCurrentValue() >>= aDouble;
    return QVariant(aDouble);
}

QVariant QtAccessibleWidget::maximumValue() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QVariant();

    Reference<XAccessibleValue> xValue(xAc, UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getMaximumValue() >>= aDouble;
    return QVariant(aDouble);
}

QVariant QtAccessibleWidget::minimumStepSize() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QVariant();

    Reference<XAccessibleValue> xValue(xAc, UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double dMinStep = 0;
    xValue->getMinimumIncrement() >>= dMinStep;
    return QVariant(dMinStep);
}

QVariant QtAccessibleWidget::minimumValue() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QVariant();

    Reference<XAccessibleValue> xValue(xAc, UNO_QUERY);
    if (!xValue.is())
        return QVariant();
    double aDouble = 0;
    xValue->getMinimumValue() >>= aDouble;
    return QVariant(aDouble);
}

void QtAccessibleWidget::setCurrentValue(const QVariant& value)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return;

    Reference<XAccessibleValue> xValue(xAc, UNO_QUERY);
    if (!xValue.is())
        return;
    xValue->setCurrentValue(Any(value.toDouble()));
}

// QAccessibleTable
QAccessibleInterface* QtAccessibleWidget::caption() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return nullptr;
    return QAccessible::queryAccessibleInterface(new QtXAccessible(xTable->getAccessibleCaption()));
}

QAccessibleInterface* QtAccessibleWidget::cellAt(int row, int column) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return nullptr;
    return QAccessible::queryAccessibleInterface(
        new QtXAccessible(xTable->getAccessibleCellAt(row, column)));
}

int QtAccessibleWidget::columnCount() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return 0;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getAccessibleColumnCount();
}

QString QtAccessibleWidget::columnDescription(int column) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QString();

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return QString();
    return toQString(xTable->getAccessibleColumnDescription(column));
}

bool QtAccessibleWidget::isColumnSelected(int nColumn) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return false;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return false;

    return xTable->isAccessibleColumnSelected(nColumn);
}

bool QtAccessibleWidget::isRowSelected(int nRow) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return false;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return false;

    return xTable->isAccessibleRowSelected(nRow);
}

void QtAccessibleWidget::modelChange(QAccessibleTableModelChangeEvent*) {}

int QtAccessibleWidget::rowCount() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return 0;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getAccessibleRowCount();
}

QString QtAccessibleWidget::rowDescription(int row) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QString();

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return QString();
    return toQString(xTable->getAccessibleRowDescription(row));
}

bool QtAccessibleWidget::selectColumn(int column)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return false;

    Reference<XAccessibleTableSelection> xTableSelection(xAc, UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->selectColumn(column);
}

bool QtAccessibleWidget::selectRow(int row)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return false;

    Reference<XAccessibleTableSelection> xTableSelection(xAc, UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->selectRow(row);
}

int QtAccessibleWidget::selectedCellCount() const
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTableInterface::selectedCellCount");
    return 0;
}

QList<QAccessibleInterface*> QtAccessibleWidget::selectedCells() const
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTableInterface::selectedCells");
    return QList<QAccessibleInterface*>();
}

int QtAccessibleWidget::selectedColumnCount() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return 0;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getSelectedAccessibleColumns().getLength();
}

QList<int> QtAccessibleWidget::selectedColumns() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QList<int>();

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return QList<int>();
    return toQList(xTable->getSelectedAccessibleColumns());
}

int QtAccessibleWidget::selectedRowCount() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return 0;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return 0;
    return xTable->getSelectedAccessibleRows().getLength();
}

QList<int> QtAccessibleWidget::selectedRows() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return QList<int>();

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return QList<int>();
    return toQList(xTable->getSelectedAccessibleRows());
}

QAccessibleInterface* QtAccessibleWidget::summary() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return nullptr;
    return QAccessible::queryAccessibleInterface(new QtXAccessible(xTable->getAccessibleSummary()));
}

bool QtAccessibleWidget::unselectColumn(int column)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return false;

    Reference<XAccessibleTableSelection> xTableSelection(xAc, UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->unselectColumn(column);
}

bool QtAccessibleWidget::unselectRow(int row)
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return false;

    Reference<XAccessibleTableSelection> xTableSelection(xAc, UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->unselectRow(row);
}

QList<QAccessibleInterface*> QtAccessibleWidget::columnHeaderCells() const
{
    SAL_WARN("vcl.qt", "Unsupported QAccessibleTableCellInterface::columnHeaderCells");
    return QList<QAccessibleInterface*>();
}

int QtAccessibleWidget::columnIndex() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return -1;

    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return -1;

    const sal_Int32 nIndexInParent = xAcc->getAccessibleIndexInParent();
    return xTable->getAccessibleColumn(nIndexInParent);
}

bool QtAccessibleWidget::isSelected() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return false;

    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return false;

    const sal_Int32 nColumn = columnIndex();
    const sal_Int32 nRow = rowIndex();
    return xTable->isAccessibleSelected(nRow, nColumn);
}

int QtAccessibleWidget::columnExtent() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return -1;

    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return -1;

    const sal_Int32 nColumn = columnIndex();
    const sal_Int32 nRow = rowIndex();
    return xTable->getAccessibleColumnExtentAt(nRow, nColumn);
}

QList<QAccessibleInterface*> QtAccessibleWidget::rowHeaderCells() const
{
    SAL_WARN("vcl.qt", "Unsupported QAccessibleTableCellInterface::rowHeaderCells");
    return QList<QAccessibleInterface*>();
}

int QtAccessibleWidget::rowExtent() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return -1;

    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return -1;

    const sal_Int32 nColumn = columnIndex();
    const sal_Int32 nRow = rowIndex();
    return xTable->getAccessibleRowExtentAt(nRow, nColumn);
}

int QtAccessibleWidget::rowIndex() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return -1;

    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return -1;

    const sal_Int32 nIndexInParent = xAcc->getAccessibleIndexInParent();
    return xTable->getAccessibleRow(nIndexInParent);
}

QAccessibleInterface* QtAccessibleWidget::table() const
{
    SAL_WARN("vcl.qt", "Unsupported QAccessibleTableCellInterface::table");
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
