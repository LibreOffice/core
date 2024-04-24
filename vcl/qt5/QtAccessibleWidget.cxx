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

#include <QtGui/QAccessibleInterface>

#include <QtAccessibleEventListener.hxx>
#include <QtAccessibleRegistry.hxx>
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
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/AccessibleImplementationHelper.hxx>
#include <o3tl/any.hxx>
#include <sal/log.hxx>
#include <vcl/accessibility/AccessibleTextAttributeHelper.hxx>

using namespace css;
using namespace css::accessibility;
using namespace css::uno;

QtAccessibleWidget::QtAccessibleWidget(const Reference<XAccessible> xAccessible, QObject* pObject)
    : m_xAccessible(xAccessible)
    , m_pObject(pObject)
{
    Reference<XAccessibleContext> xContext = xAccessible->getAccessibleContext();
    Reference<XAccessibleEventBroadcaster> xBroadcaster(xContext, UNO_QUERY);
    if (xBroadcaster.is())
    {
        Reference<XAccessibleEventListener> xListener(new QtAccessibleEventListener(this));
        xBroadcaster->addAccessibleEventListener(xListener);
    }
}

void QtAccessibleWidget::invalidate()
{
    QtAccessibleRegistry::remove(m_xAccessible);
    m_xAccessible.clear();
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

QWindow* QtAccessibleWidget::window() const
{
    assert(m_pObject);
    if (m_pObject->isWidgetType())
    {
        QWidget* pWidget = static_cast<QWidget*>(m_pObject);
        QWidget* pWindow = pWidget->window();
        if (pWindow)
            return pWindow->windowHandle();
    }

    QAccessibleInterface* pParent = parent();
    if (pParent)
        return pParent->window();

    return nullptr;
}

int QtAccessibleWidget::childCount() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return 0;

    sal_Int64 nChildCount = xAc->getAccessibleChildCount();
    if (nChildCount > std::numeric_limits<int>::max())
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::childCount: Child count exceeds maximum int value, "
                           "returning max int.");
        nChildCount = std::numeric_limits<int>::max();
    }

    return nChildCount;
}

int QtAccessibleWidget::indexOfChild(const QAccessibleInterface* pChild) const
{
    const QtAccessibleWidget* pAccessibleWidget = dynamic_cast<const QtAccessibleWidget*>(pChild);
    if (!pAccessibleWidget)
    {
        SAL_WARN(
            "vcl.qt",
            "QtAccessibleWidget::indexOfChild called with child that is no QtAccessibleWidget");
        return -1;
    }

    Reference<XAccessibleContext> xContext = pAccessibleWidget->getAccessibleContextImpl();
    if (!xContext.is())
        return -1;

    sal_Int64 nChildIndex = xContext->getAccessibleIndexInParent();
    if (nChildIndex > std::numeric_limits<int>::max())
    {
        // use -2 when the child index is too large to fit into 32 bit to neither use the
        // valid index of another child nor -1, which would e.g. make the Orca screen reader
        // interpret the object as being a zombie
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::indexOfChild: Child index exceeds maximum int value, "
                 "returning -2.");
        nChildIndex = -2;
    }
    return nChildIndex;
}

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
    // Qt semantics is the other way around
    switch (relationType)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        case AccessibleRelationType::CONTENT_FLOWS_FROM:
            return QAccessible::FlowsTo;
        case AccessibleRelationType::CONTENT_FLOWS_TO:
            return QAccessible::FlowsFrom;
#endif
        case AccessibleRelationType::CONTROLLED_BY:
            return QAccessible::Controller;
        case AccessibleRelationType::CONTROLLER_FOR:
            return QAccessible::Controlled;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        case AccessibleRelationType::DESCRIBED_BY:
            return QAccessible::DescriptionFor;
#endif
        case AccessibleRelationType::LABELED_BY:
            return QAccessible::Label;
        case AccessibleRelationType::LABEL_FOR:
            return QAccessible::Labelled;
        case AccessibleRelationType::INVALID:
        case AccessibleRelationType::MEMBER_OF:
        case AccessibleRelationType::SUB_WINDOW_OF:
        case AccessibleRelationType::NODE_CHILD_OF:
        default:
            SAL_WARN("vcl.qt", "Unmatched relation: " << relationType);
            return {};
    }
}

void lcl_appendRelation(QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>* relations,
                        AccessibleRelation aRelation, QAccessible::Relation match)
{
    QAccessible::Relation aQRelation = lcl_matchUnoRelation(aRelation.RelationType);
    // skip in case there's no Qt relation matching the filter
    if (!(aQRelation & match))
        return;

    sal_uInt32 nTargetCount = aRelation.TargetSet.getLength();

    for (sal_uInt32 i = 0; i < nTargetCount; i++)
    {
        Reference<XAccessible> xAccessible = aRelation.TargetSet[i];
        relations->append(
            { QAccessible::queryAccessibleInterface(QtAccessibleRegistry::getQObject(xAccessible)),
              aQRelation });
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
        int count = xRelationSet->getRelationCount();
        for (int i = 0; i < count; i++)
        {
            AccessibleRelation aRelation = xRelationSet->getRelation(i);
            lcl_appendRelation(&relations, aRelation, match);
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
    awt::Point aPoint = xAccessibleComponent->getLocationOnScreen();
    awt::Size aSize = xAccessibleComponent->getSize();

    return QRect(aPoint.X, aPoint.Y, aSize.Width, aSize.Height);
}

QAccessibleInterface* QtAccessibleWidget::parent() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    if (xAc->getAccessibleParent().is())
        return QAccessible::queryAccessibleInterface(
            QtAccessibleRegistry::getQObject(xAc->getAccessibleParent()));

    // go via the QObject hierarchy; some a11y objects like the application
    // (at the root of the a11y hierarchy) are handled solely by Qt and have
    // no LO-internal a11y objects associated with them
    QObject* pObj = object();
    if (pObj && pObj->parent())
        return QAccessible::queryAccessibleInterface(pObj->parent());

    // return app as parent for top-level objects
    return QAccessible::queryAccessibleInterface(qApp);
}

QAccessibleInterface* QtAccessibleWidget::child(int index) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    if (index < 0 || index >= xAc->getAccessibleChildCount())
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::child called with invalid index: " << index);
        return nullptr;
    }

    return QAccessible::queryAccessibleInterface(
        QtAccessibleRegistry::getQObject(xAc->getAccessibleChild(index)));
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
        case AccessibleRole::FILE_CHOOSER:
            return QAccessible::Dialog;
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
        case AccessibleRole::NOTIFICATION:
            return QAccessible::Notification;
        case AccessibleRole::OPTION_PANE:
            return QAccessible::Pane;
        case AccessibleRole::PAGE_TAB:
            return QAccessible::PageTab;
        case AccessibleRole::PAGE_TAB_LIST:
            return QAccessible::PageTabList;
        case AccessibleRole::PANEL:
            return QAccessible::Pane;
        case AccessibleRole::PARAGRAPH:
        case AccessibleRole::BLOCK_QUOTE:
            return QAccessible::Paragraph;
        case AccessibleRole::PASSWORD_TEXT:
            // Qt API doesn't have a separate role to distinguish password edits,
            // but a 'passwordEdit' state
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
            return QAccessible::ButtonDropDown;
        case AccessibleRole::BUTTON_MENU:
            return QAccessible::ButtonMenu;
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
        case AccessibleRole::WINDOW: // top-level window without title bar
            return QAccessible::Window;
    }

    SAL_WARN("vcl.qt", "Unmapped role: " << getAccessibleContextImpl()->getAccessibleRole());
    return QAccessible::NoRole;
}

namespace
{
void lcl_addState(QAccessible::State* state, sal_Int64 nState)
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
        case AccessibleStateType::CHECKABLE:
            state->checkable = true;
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
            state->checkStateMixed = true;
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

    sal_Int64 nStateSet(xAc->getAccessibleStateSet());

    for (int i = 0; i < 63; ++i)
    {
        sal_Int64 nState = sal_Int64(1) << i;
        if (nStateSet & nState)
            lcl_addState(&state, nState);
    }

    if (xAc->getAccessibleRole() == AccessibleRole::PASSWORD_TEXT)
        state.passwordEdit = true;

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
    if (t == QAccessible::ActionInterface && accessibleProvidesInterface<XAccessibleAction>())
        return static_cast<QAccessibleActionInterface*>(this);
    if (t == QAccessible::TextInterface && accessibleProvidesInterface<XAccessibleText>())
        return static_cast<QAccessibleTextInterface*>(this);
    if (t == QAccessible::EditableTextInterface
        && accessibleProvidesInterface<XAccessibleEditableText>())
        return static_cast<QAccessibleEditableTextInterface*>(this);
    if (t == QAccessible::ValueInterface && accessibleProvidesInterface<XAccessibleValue>())
        return static_cast<QAccessibleValueInterface*>(this);
    if (t == QAccessible::TableCellInterface)
    {
        // parent must be a table
        Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
        if (xTable.is())
            return static_cast<QAccessibleTableCellInterface*>(this);
    }
    if (t == QAccessible::TableInterface && accessibleProvidesInterface<XAccessibleTable>())
        return static_cast<QAccessibleTableInterface*>(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (t == QAccessible::SelectionInterface && accessibleProvidesInterface<XAccessibleSelection>())
        return static_cast<QAccessibleSelectionInterface*>(this);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    if (t == QAccessible::AttributesInterface)
        return static_cast<QAccessibleAttributesInterface*>(this);
#endif
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
    // convert from screen to local coordinates
    QPoint aLocalCoords = QPoint(x, y) - rect().topLeft();
    return QAccessible::queryAccessibleInterface(
        QtAccessibleRegistry::getQObject(xAccessibleComponent->getAccessibleAtPoint(
            awt::Point(aLocalCoords.x(), aLocalCoords.y()))));
}

QAccessibleInterface* QtAccessibleWidget::customFactory(const QString& classname, QObject* object)
{
    if (classname == QLatin1String("QtWidget") && object && object->isWidgetType())
    {
        QtWidget* pWidget = static_cast<QtWidget*>(object);
        vcl::Window* pWindow = pWidget->frame().GetWindow();

        if (pWindow)
        {
            css::uno::Reference<XAccessible> xAcc = pWindow->GetAccessible();
            // insert into registry so the association between the XAccessible and the QtWidget
            // is remembered rather than creating a different QtXAccessible when a QObject is needed later
            QtAccessibleRegistry::insert(xAcc, object);
            return new QtAccessibleWidget(xAcc, object);
        }
    }
    if (classname == QLatin1String("QtXAccessible") && object)
    {
        QtXAccessible* pXAccessible = static_cast<QtXAccessible*>(object);
        if (pXAccessible->m_xAccessible.is())
        {
            QtAccessibleWidget* pRet = new QtAccessibleWidget(pXAccessible->m_xAccessible, object);
            // clear the reference in the QtXAccessible, no longer needed now that the QtAccessibleWidget holds one
            pXAccessible->m_xAccessible.clear();
            return pRet;
        }
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)

// QAccessibleAttributesInterface helpers
namespace
{
void lcl_insertAttribute(QHash<QAccessible::Attribute, QVariant>& rQtAttrs, const OUString& rName,
                         const OUString& rValue)
{
    if (rName == u"level"_ustr)
    {
        rQtAttrs.insert(QAccessible::Attribute::Level,
                        QVariant::fromValue(static_cast<int>(rValue.toInt32())));
    }
    else
    {
        // for now, leave not explicitly handled attributes as they are and report
        // via QAccessible::Attribute::Custom, but should consider suggesting to
        // add more specific attributes on Qt side and use those instead
        const QVariant aVariant = rQtAttrs.value(QAccessible::Attribute::Custom,
                                                 QVariant::fromValue(QHash<QString, QString>()));
        assert((aVariant.canConvert<QHash<QString, QString>>()));
        QHash<QString, QString> aAttrs = aVariant.value<QHash<QString, QString>>();
        aAttrs.insert(toQString(rName), toQString(rValue));
        rQtAttrs.insert(QAccessible::Attribute::Custom, QVariant::fromValue(aAttrs));
    }
}
}

QHash<QAccessible::Attribute, QVariant> QtAccessibleWidget::attributes() const
{
    Reference<XAccessibleContext> xContext = getAccessibleContextImpl();
    if (!xContext.is())
        return {};

    Reference<XAccessibleExtendedAttributes> xAttributes(xContext, UNO_QUERY);
    if (!xAttributes.is())
        return {};

    OUString sAttrs;
    xAttributes->getExtendedAttributes() >>= sAttrs;

    QHash<QAccessible::Attribute, QVariant> aQtAttrs;
    sal_Int32 nIndex = 0;
    do
    {
        const OUString sAttribute = sAttrs.getToken(0, ';', nIndex);
        sal_Int32 nColonPos = 0;
        const OUString sName = sAttribute.getToken(0, ':', nColonPos);
        const OUString sValue = sAttribute.getToken(0, ':', nColonPos);
        assert(nColonPos == -1
               && "Too many colons in attribute that should have \"name:value\" syntax");
        if (!sName.isEmpty())
            lcl_insertAttribute(aQtAttrs, sName, sValue);
    } while (nIndex >= 0);

    return aQtAttrs;
}

// QAccessibleAttributesInterface
QList<QAccessible::Attribute> QtAccessibleWidget::attributeKeys() const
{
    const QHash<QAccessible::Attribute, QVariant> aAttributes = attributes();
    return aAttributes.keys();
}

QVariant QtAccessibleWidget::attributeValue(QAccessible::Attribute eAttribute) const
{
    const QHash<QAccessible::Attribute, QVariant> aAllAttributes = attributes();
    return aAllAttributes.value(eAttribute);
}
#endif

// QAccessibleTextInterface
void QtAccessibleWidget::addSelection(int /* startOffset */, int /* endOffset */)
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::addSelection");
}

// Text attributes are returned in format specified in IAccessible2 spec, since that
// is what Qt handles:
// https://wiki.linuxfoundation.org/accessibility/iaccessible2/textattributes
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
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::attributes called with invalid offset: " << offset);
        return QString();
    }

    // Qt doesn't have the strict separation into text and object attributes, but also
    // supports text-specific attributes that are object attributes according to the
    // IAccessible2 spec.
    sal_Int32 nStart = 0;
    sal_Int32 nEnd = 0;
    const OUString aRet = AccessibleTextAttributeHelper::GetIAccessible2TextAttributes(
        xText, IA2AttributeType::TextAttributes | IA2AttributeType::ObjectAttributes,
        static_cast<sal_Int32>(offset), nStart, nEnd);
    *startOffset = static_cast<int>(nStart);
    *endOffset = static_cast<int>(nEnd);
    return toQString(aRet);
}

int QtAccessibleWidget::characterCount() const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        return xText->getCharacterCount();
    return 0;
}

QRect QtAccessibleWidget::characterRect(int nOffset) const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return QRect();

    if (nOffset < 0 || nOffset > xText->getCharacterCount())
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::characterRect called with invalid offset: " << nOffset);
        return QRect();
    }

    const awt::Rectangle aBounds = xText->getCharacterBounds(nOffset);
    const QRect aRect(aBounds.X, aBounds.Y, aBounds.Width, aBounds.Height);
    // convert to screen coordinates
    const QRect aScreenPos = rect();
    return aRect.translated(aScreenPos.x(), aScreenPos.y());
}

int QtAccessibleWidget::cursorPosition() const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (xText.is())
        return xText->getCaretPosition();
    return 0;
}

int QtAccessibleWidget::offsetAtPoint(const QPoint& rPoint) const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return -1;

    // convert from screen to local coordinates
    QPoint aLocalCoords = rPoint - rect().topLeft();
    awt::Point aPoint(aLocalCoords.x(), aLocalCoords.y());
    return xText->getIndexAtPoint(aPoint);
}

void QtAccessibleWidget::removeSelection(int /* selectionIndex */)
{
    SAL_INFO("vcl.qt", "Unsupported QAccessibleTextInterface::removeSelection");
}

void QtAccessibleWidget::scrollToSubstring(int startIndex, int endIndex)
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return;

    sal_Int32 nTextLength = xText->getCharacterCount();
    if (startIndex < 0 || startIndex > nTextLength || endIndex < 0 || endIndex > nTextLength)
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::scrollToSubstring called with invalid offset.");
        return;
    }

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
    if (!xText.is())
        return;

    if (position < 0 || position > xText->getCharacterCount())
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::setCursorPosition called with invalid offset: " << position);
        return;
    }

    xText->setCaretPosition(position);
}

void QtAccessibleWidget::setSelection(int /* selectionIndex */, int startOffset, int endOffset)
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return;

    sal_Int32 nTextLength = xText->getCharacterCount();
    if (startOffset < 0 || startOffset > nTextLength || endOffset < 0 || endOffset > nTextLength)
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::setSelection called with invalid offset.");
        return;
    }

    xText->setSelection(startOffset, endOffset);
}

QString QtAccessibleWidget::text(int startOffset, int endOffset) const
{
    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return QString();

    sal_Int32 nTextLength = xText->getCharacterCount();
    if (startOffset < 0 || startOffset > nTextLength || endOffset < 0 || endOffset > nTextLength)
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::text called with invalid offset.");
        return QString();
    }

    return toQString(xText->getTextRange(startOffset, endOffset));
}

QString QtAccessibleWidget::textAfterOffset(int nOffset,
                                            QAccessible::TextBoundaryType eBoundaryType,
                                            int* pStartOffset, int* pEndOffset) const
{
    if (pStartOffset == nullptr || pEndOffset == nullptr)
        return QString();

    *pStartOffset = -1;
    *pEndOffset = -1;

    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return QString();

    const int nCharCount = characterCount();
    // -1 is special value for text length
    if (nOffset == -1)
        nOffset = nCharCount;
    else if (nOffset < -1 || nOffset > nCharCount)
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::textAfterOffset called with invalid offset: " << nOffset);
        return QString();
    }

    if (eBoundaryType == QAccessible::NoBoundary)
    {
        if (nOffset == nCharCount)
            return QString();
        *pStartOffset = nOffset + 1;
        *pEndOffset = nCharCount;
        return text(nOffset + 1, nCharCount);
    }

    sal_Int16 nUnoBoundaryType = lcl_matchQtTextBoundaryType(eBoundaryType);
    assert(nUnoBoundaryType > 0);
    const TextSegment aSegment = xText->getTextBehindIndex(nOffset, nUnoBoundaryType);
    *pStartOffset = aSegment.SegmentStart;
    *pEndOffset = aSegment.SegmentEnd;
    return toQString(aSegment.SegmentText);
}

QString QtAccessibleWidget::textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType,
                                         int* startOffset, int* endOffset) const
{
    if (startOffset == nullptr || endOffset == nullptr)
        return QString();

    const int nCharCount = characterCount();
    if (boundaryType == QAccessible::NoBoundary)
    {
        *startOffset = 0;
        *endOffset = nCharCount;
        return text(0, nCharCount);
    }

    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return QString();

    sal_Int16 nUnoBoundaryType = lcl_matchQtTextBoundaryType(boundaryType);
    assert(nUnoBoundaryType > 0);

    // special value of -1 for offset means text length
    if (offset == -1)
        offset = nCharCount;

    if (offset < 0 || offset > nCharCount)
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::textAtOffset called with invalid offset: " << offset);
        return QString();
    }

    const TextSegment segment = xText->getTextAtIndex(offset, nUnoBoundaryType);
    *startOffset = segment.SegmentStart;
    *endOffset = segment.SegmentEnd;
    return toQString(segment.SegmentText);
}

QString QtAccessibleWidget::textBeforeOffset(int nOffset,
                                             QAccessible::TextBoundaryType eBoundaryType,
                                             int* pStartOffset, int* pEndOffset) const
{
    if (pStartOffset == nullptr || pEndOffset == nullptr)
        return QString();

    *pStartOffset = -1;
    *pEndOffset = -1;

    Reference<XAccessibleText> xText(getAccessibleContextImpl(), UNO_QUERY);
    if (!xText.is())
        return QString();

    const int nCharCount = characterCount();
    // -1 is special value for text length
    if (nOffset == -1)
        nOffset = nCharCount;
    else if (nOffset < -1 || nOffset > nCharCount)
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::textBeforeOffset called with invalid offset: " << nOffset);
        return QString();
    }

    if (eBoundaryType == QAccessible::NoBoundary)
    {
        *pStartOffset = 0;
        *pEndOffset = nOffset;
        return text(0, nOffset);
    }

    sal_Int16 nUnoBoundaryType = lcl_matchQtTextBoundaryType(eBoundaryType);
    assert(nUnoBoundaryType > 0);
    const TextSegment aSegment = xText->getTextBeforeIndex(nOffset, nUnoBoundaryType);
    *pStartOffset = aSegment.SegmentStart;
    *pEndOffset = aSegment.SegmentEnd;
    return toQString(aSegment.SegmentText);
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

    sal_Int32 nTextLength = xEditableText->getCharacterCount();
    if (startOffset < 0 || startOffset > nTextLength || endOffset < 0 || endOffset > nTextLength)
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::deleteText called with invalid offset.");
        return;
    }

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

    if (offset < 0 || offset > xEditableText->getCharacterCount())
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::insertText called with invalid offset: " << offset);
        return;
    }

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

    sal_Int32 nTextLength = xEditableText->getCharacterCount();
    if (startOffset < 0 || startOffset > nTextLength || endOffset < 0 || endOffset > nTextLength)
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::replaceText called with invalid offset.");
        return;
    }

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

    // Different types of numerical values for XAccessibleValue are possible.
    // If current value has an integer type, also use that for the new value, to make
    // sure underlying implementations expecting that can handle the value properly.
    const Any aCurrentValue = xValue->getCurrentValue();
    if (aCurrentValue.getValueTypeClass() == css::uno::TypeClass::TypeClass_LONG)
        xValue->setCurrentValue(Any(static_cast<sal_Int32>(value.toInt())));
    else if (aCurrentValue.getValueTypeClass() == css::uno::TypeClass::TypeClass_HYPER)
        xValue->setCurrentValue(Any(static_cast<sal_Int64>(value.toLongLong())));
    else
        xValue->setCurrentValue(Any(value.toDouble()));
}

// QAccessibleTableInterface
QAccessibleInterface* QtAccessibleWidget::caption() const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return nullptr;
    return QAccessible::queryAccessibleInterface(
        QtAccessibleRegistry::getQObject(xTable->getAccessibleCaption()));
}

QAccessibleInterface* QtAccessibleWidget::cellAt(int row, int column) const
{
    Reference<XAccessibleContext> xAc = getAccessibleContextImpl();
    if (!xAc.is())
        return nullptr;

    Reference<XAccessibleTable> xTable(xAc, UNO_QUERY);
    if (!xTable.is())
        return nullptr;

    if (row < 0 || row >= xTable->getAccessibleRowCount() || column < 0
        || column >= xTable->getAccessibleColumnCount())
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::cellAt called with invalid row/column index ("
                               << row << ", " << column << ")");
        return nullptr;
    }

    return QAccessible::queryAccessibleInterface(
        QtAccessibleRegistry::getQObject(xTable->getAccessibleCellAt(row, column)));
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

    if (nColumn < 0 || nColumn >= xTable->getAccessibleColumnCount())
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::isColumnSelected called with invalid column index "
                               << nColumn);
        return false;
    }

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

    if (nRow < 0 || nRow >= xTable->getAccessibleRowCount())
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::isRowSelected called with invalid row index " << nRow);
        return false;
    }

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

    if (column < 0 || column >= columnCount())
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::selectColumn called with invalid column index " << column);
        return false;
    }

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

    if (row < 0 || row >= rowCount())
    {
        SAL_WARN("vcl.qt", "QtAccessibleWidget::selectRow called with invalid row index " << row);
        return false;
    }

    Reference<XAccessibleTableSelection> xTableSelection(xAc, UNO_QUERY);
    if (!xTableSelection.is())
        return false;
    return xTableSelection->selectRow(row);
}

int QtAccessibleWidget::selectedCellCount() const { return selectedItemCount(); }

QList<QAccessibleInterface*> QtAccessibleWidget::selectedCells() const { return selectedItems(); }

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
    return QAccessible::queryAccessibleInterface(
        QtAccessibleRegistry::getQObject(xTable->getAccessibleSummary()));
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

// QAccessibleTableCellInterface
QList<QAccessibleInterface*> QtAccessibleWidget::columnHeaderCells() const
{
    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return QList<QAccessibleInterface*>();

    Reference<XAccessibleTable> xHeaders = xTable->getAccessibleColumnHeaders();
    if (!xHeaders.is())
        return QList<QAccessibleInterface*>();

    const sal_Int32 nCol = columnIndex();
    QList<QAccessibleInterface*> aHeaderCells;
    for (sal_Int32 nRow = 0; nRow < xHeaders->getAccessibleRowCount(); nRow++)
    {
        Reference<XAccessible> xCell = xHeaders->getAccessibleCellAt(nRow, nCol);
        QAccessibleInterface* pInterface
            = QAccessible::queryAccessibleInterface(QtAccessibleRegistry::getQObject(xCell));
        aHeaderCells.push_back(pInterface);
    }
    return aHeaderCells;
}

int QtAccessibleWidget::columnIndex() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return -1;

    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return -1;

    const sal_Int64 nIndexInParent = xAcc->getAccessibleIndexInParent();
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
    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return QList<QAccessibleInterface*>();

    Reference<XAccessibleTable> xHeaders = xTable->getAccessibleRowHeaders();
    if (!xHeaders.is())
        return QList<QAccessibleInterface*>();

    const sal_Int32 nRow = rowIndex();
    QList<QAccessibleInterface*> aHeaderCells;
    for (sal_Int32 nCol = 0; nCol < xHeaders->getAccessibleColumnCount(); nCol++)
    {
        Reference<XAccessible> xCell = xHeaders->getAccessibleCellAt(nRow, nCol);
        QAccessibleInterface* pInterface
            = QAccessible::queryAccessibleInterface(QtAccessibleRegistry::getQObject(xCell));
        aHeaderCells.push_back(pInterface);
    }
    return aHeaderCells;
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

    const sal_Int64 nIndexInParent = xAcc->getAccessibleIndexInParent();
    return xTable->getAccessibleRow(nIndexInParent);
}

QAccessibleInterface* QtAccessibleWidget::table() const
{
    Reference<XAccessibleTable> xTable = getAccessibleTableForParent();
    if (!xTable.is())
        return nullptr;

    Reference<XAccessible> xTableAcc(xTable, UNO_QUERY);
    if (!xTableAcc.is())
        return nullptr;

    return QAccessible::queryAccessibleInterface(QtAccessibleRegistry::getQObject(xTableAcc));
}

// QAccessibleSelectionInterface
int QtAccessibleWidget::selectedItemCount() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return 0;

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return 0;

    sal_Int64 nSelected = xSelection->getSelectedAccessibleChildCount();
    if (nSelected > std::numeric_limits<int>::max())
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::selectedItemCount: Cell count exceeds maximum int value, "
                 "using max int.");
        nSelected = std::numeric_limits<int>::max();
    }
    return nSelected;
}

QList<QAccessibleInterface*> QtAccessibleWidget::selectedItems() const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return QList<QAccessibleInterface*>();

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return QList<QAccessibleInterface*>();

    QList<QAccessibleInterface*> aSelectedItems;
    sal_Int64 nSelected = xSelection->getSelectedAccessibleChildCount();
    if (nSelected > std::numeric_limits<int>::max())
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::selectedItems: Cell count exceeds maximum int value, "
                 "using max int.");
        nSelected = std::numeric_limits<int>::max();
    }
    for (sal_Int64 i = 0; i < nSelected; i++)
    {
        Reference<XAccessible> xChild = xSelection->getSelectedAccessibleChild(i);
        QAccessibleInterface* pInterface
            = QAccessible::queryAccessibleInterface(QtAccessibleRegistry::getQObject(xChild));
        aSelectedItems.push_back(pInterface);
    }
    return aSelectedItems;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
QAccessibleInterface* QtAccessibleWidget::selectedItem(int nSelectionIndex) const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return nullptr;

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return nullptr;

    if (nSelectionIndex < 0 || nSelectionIndex >= xSelection->getSelectedAccessibleChildCount())
    {
        SAL_WARN("vcl.qt",
                 "QtAccessibleWidget::selectedItem called with invalid index: " << nSelectionIndex);
        return nullptr;
    }

    Reference<XAccessible> xChild = xSelection->getSelectedAccessibleChild(nSelectionIndex);
    if (!xChild)
        return nullptr;

    return QAccessible::queryAccessibleInterface(QtAccessibleRegistry::getQObject(xChild));
}

bool QtAccessibleWidget::isSelected(QAccessibleInterface* pItem) const
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return false;

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return false;

    int nChildIndex = indexOfChild(pItem);
    if (nChildIndex < 0)
        return false;

    return xSelection->isAccessibleChildSelected(nChildIndex);
}

bool QtAccessibleWidget::select(QAccessibleInterface* pItem)
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return false;

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return false;

    int nChildIndex = indexOfChild(pItem);
    if (nChildIndex < 0)
        return false;

    xSelection->selectAccessibleChild(nChildIndex);
    return true;
}

bool QtAccessibleWidget::unselect(QAccessibleInterface* pItem)
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return false;

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return false;

    int nChildIndex = indexOfChild(pItem);
    if (nChildIndex < 0)
        return false;

    xSelection->deselectAccessibleChild(nChildIndex);
    return true;
}

bool QtAccessibleWidget::selectAll()
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return false;

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return false;

    xSelection->selectAllAccessibleChildren();
    return true;
}

bool QtAccessibleWidget::clear()
{
    Reference<XAccessibleContext> xAcc = getAccessibleContextImpl();
    if (!xAcc.is())
        return false;

    Reference<XAccessibleSelection> xSelection(xAcc, UNO_QUERY);
    if (!xSelection.is())
        return false;

    xSelection->clearAccessibleSelection();
    return true;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
