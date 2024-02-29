/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vclpluginapi.h>

#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtGui/QAccessible>
#include <QtGui/QAccessibleActionInterface>
#include <QtGui/QAccessibleInterface>
#include <QtGui/QAccessibleTableCellInterface>
#include <QtGui/QAccessibleTableInterface>
#include <QtGui/QAccessibleTextInterface>
#include <QtGui/QAccessibleValueInterface>
#include <QtGui/QColor>
#include <QtGui/QWindow>

#include <com/sun/star/accessibility/XAccessible.hpp>

namespace com::sun::star::accessibility
{
class XAccessibleTable;
}

class QtFrame;
class QtWidget;

class QtAccessibleWidget final : public QAccessibleInterface,
                                 public QAccessibleActionInterface,
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
                                 public QAccessibleAttributesInterface,
#endif
                                 public QAccessibleTextInterface,
                                 public QAccessibleEditableTextInterface,
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
                                 public QAccessibleSelectionInterface,
#endif
                                 public QAccessibleTableCellInterface,
                                 public QAccessibleTableInterface,
                                 public QAccessibleValueInterface
{
public:
    QtAccessibleWidget(const css::uno::Reference<css::accessibility::XAccessible> xAccessible,
                       QObject* pObject);

    void invalidate();

    // QAccessibleInterface
    QWindow* window() const override;
    int childCount() const override;
    int indexOfChild(const QAccessibleInterface* child) const override;
    QVector<QPair<QAccessibleInterface*, QAccessible::Relation>>
    relations(QAccessible::Relation match = QAccessible::AllRelations) const override;
    QAccessibleInterface* focusChild() const override;

    QRect rect() const override;

    QAccessibleInterface* parent() const override;
    QAccessibleInterface* child(int index) const override;

    QString text(QAccessible::Text t) const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

    QColor foregroundColor() const override;
    QColor backgroundColor() const override;

    bool isValid() const override;
    QObject* object() const override;
    void setText(QAccessible::Text t, const QString& text) override;
    QAccessibleInterface* childAt(int x, int y) const override;

    void* interface_cast(QAccessible::InterfaceType t) override;

    // QAccessibleActionInterface
    QStringList actionNames() const override;
    void doAction(const QString& actionName) override;
    QStringList keyBindingsForAction(const QString& actionName) const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // helper method for QAccessibleAttributesInterface
    QHash<QAccessible::Attribute, QVariant> attributes() const;

    // QAccessibleAttributesInterface
    QList<QAccessible::Attribute> attributeKeys() const override;
    QVariant attributeValue(QAccessible::Attribute key) const override;
#endif

    // QAccessibleTextInterface
    void addSelection(int startOffset, int endOffset) override;
    QString attributes(int offset, int* startOffset, int* endOffset) const override;
    int characterCount() const override;
    QRect characterRect(int offset) const override;
    int cursorPosition() const override;
    int offsetAtPoint(const QPoint& point) const override;
    void removeSelection(int selectionIndex) override;
    void scrollToSubstring(int startIndex, int endIndex) override;
    void selection(int selectionIndex, int* startOffset, int* endOffset) const override;
    int selectionCount() const override;
    void setCursorPosition(int position) override;
    void setSelection(int selectionIndex, int startOffset, int endOffset) override;
    QString text(int startOffset, int endOffset) const override;
    QString textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType,
                            int* startOffset, int* endOffset) const override;
    QString textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType, int* startOffset,
                         int* endOffset) const override;
    QString textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType,
                             int* startOffset, int* endOffset) const override;

    // QAccessibleEditableTextInterface
    virtual void deleteText(int startOffset, int endOffset) override;
    virtual void insertText(int offset, const QString& text) override;
    virtual void replaceText(int startOffset, int endOffset, const QString& text) override;

    // QAccessibleValueInterface
    QVariant currentValue() const override;
    QVariant maximumValue() const override;
    QVariant minimumStepSize() const override;
    QVariant minimumValue() const override;
    void setCurrentValue(const QVariant& value) override;

    // QAccessibleTableInterface
    virtual QAccessibleInterface* caption() const override;
    virtual QAccessibleInterface* cellAt(int row, int column) const override;
    virtual int columnCount() const override;
    virtual QString columnDescription(int column) const override;
    virtual bool isColumnSelected(int column) const override;
    virtual bool isRowSelected(int row) const override;
    virtual void modelChange(QAccessibleTableModelChangeEvent* event) override;
    virtual int rowCount() const override;
    virtual QString rowDescription(int row) const override;
    virtual bool selectColumn(int column) override;
    virtual bool selectRow(int row) override;
    virtual int selectedCellCount() const override;
    virtual QList<QAccessibleInterface*> selectedCells() const override;
    virtual int selectedColumnCount() const override;
    virtual QList<int> selectedColumns() const override;
    virtual int selectedRowCount() const override;
    virtual QList<int> selectedRows() const override;
    virtual QAccessibleInterface* summary() const override;
    virtual bool unselectColumn(int column) override;
    virtual bool unselectRow(int row) override;

    // QAccessibleTableCellInterface
    virtual QList<QAccessibleInterface*> columnHeaderCells() const override;
    virtual int columnIndex() const override;
    virtual bool isSelected() const override;
    virtual int columnExtent() const override;
    virtual QList<QAccessibleInterface*> rowHeaderCells() const override;
    virtual int rowExtent() const override;
    virtual int rowIndex() const override;
    virtual QAccessibleInterface* table() const override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // QAccessibleSelectionInterface
    virtual int selectedItemCount() const override;
    virtual QList<QAccessibleInterface*> selectedItems() const override;
    virtual QAccessibleInterface* selectedItem(int selectionIndex) const override;
    virtual bool isSelected(QAccessibleInterface* item) const override;
    virtual bool select(QAccessibleInterface* item) override;
    virtual bool unselect(QAccessibleInterface* item) override;
    virtual bool selectAll() override;
    virtual bool clear() override;
#else
    // no override, but used in QAccessibleTableInterface methods
    int selectedItemCount() const;
    QList<QAccessibleInterface*> selectedItems() const;
#endif

    // Factory
    static QAccessibleInterface* customFactory(const QString& classname, QObject* object);

private:
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
    css::uno::Reference<css::accessibility::XAccessibleContext> getAccessibleContextImpl() const;
    css::uno::Reference<css::accessibility::XAccessibleTable> getAccessibleTableForParent() const;

    template <class Interface> bool accessibleProvidesInterface() const
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext
            = getAccessibleContextImpl();
        css::uno::Reference<Interface> xInterface(xContext, css::uno::UNO_QUERY);
        return xInterface.is();
    }

    QObject* m_pObject;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
