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

#pragma once

#include <QtWidgets/QWidget>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>

class QInputEvent;
class Qt5Frame;
class Qt5Object;
struct SalAbstractMouseEvent;

class Qt5Widget : public QWidget
{
    Q_OBJECT

    Qt5Frame& m_rFrame;
    bool m_bNonEmptyIMPreeditSeen;
    int m_nDeltaX;
    int m_nDeltaY;

    enum class ButtonKeyState
    {
        Pressed,
        Released
    };

    static void commitText(Qt5Frame&, const QString& aText);
    static bool handleKeyEvent(Qt5Frame&, const QWidget&, QKeyEvent*, const ButtonKeyState);
    static void handleMouseButtonEvent(const Qt5Frame&, const QMouseEvent*, const ButtonKeyState);
    static void fillSalAbstractMouseEvent(const Qt5Frame& rFrame, const QInputEvent* pQEvent,
                                          const QPoint& rPos, Qt::MouseButtons eButtons, int nWidth,
                                          SalAbstractMouseEvent& aSalEvent);

    virtual bool event(QEvent*) override;

    virtual void focusInEvent(QFocusEvent*) override;
    virtual void focusOutEvent(QFocusEvent*) override;
    // keyPressEvent(QKeyEvent*) is handled via event(QEvent*); see comment
    virtual void keyReleaseEvent(QKeyEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void dragEnterEvent(QDragEnterEvent*) override;
    virtual void dragLeaveEvent(QDragLeaveEvent*) override;
    virtual void dragMoveEvent(QDragMoveEvent*) override;
    virtual void dropEvent(QDropEvent*) override;
    virtual void moveEvent(QMoveEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void showEvent(QShowEvent*) override;
    virtual void wheelEvent(QWheelEvent*) override;
    virtual void closeEvent(QCloseEvent*) override;
    virtual void changeEvent(QEvent*) override;

    void inputMethodEvent(QInputMethodEvent*) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery) const override;

public:
    Qt5Widget(Qt5Frame& rFrame, Qt::WindowFlags f = Qt::WindowFlags());

    Qt5Frame& frame() const { return m_rFrame; }
    void endExtTextInput();

    static bool handleEvent(Qt5Frame&, const QWidget&, QEvent*);
    // key events might be propagated further down => call base on false
    static inline bool handleKeyReleaseEvent(Qt5Frame&, const QWidget&, QKeyEvent*);
    // mouse events are always accepted
    static inline void handleMousePressEvent(const Qt5Frame&, const QMouseEvent*);
    static inline void handleMouseReleaseEvent(const Qt5Frame&, const QMouseEvent*);
};

bool Qt5Widget::handleKeyReleaseEvent(Qt5Frame& rFrame, const QWidget& rWidget, QKeyEvent* pEvent)
{
    return handleKeyEvent(rFrame, rWidget, pEvent, ButtonKeyState::Released);
}

void Qt5Widget::handleMousePressEvent(const Qt5Frame& rFrame, const QMouseEvent* pEvent)
{
    handleMouseButtonEvent(rFrame, pEvent, ButtonKeyState::Pressed);
}

void Qt5Widget::handleMouseReleaseEvent(const Qt5Frame& rFrame, const QMouseEvent* pEvent)
{
    handleMouseButtonEvent(rFrame, pEvent, ButtonKeyState::Released);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
