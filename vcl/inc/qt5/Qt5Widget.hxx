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

class Qt5Frame;
class Qt5Object;
class QEvent;
class QFocusEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QMoveEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QWheelEvent;
class QVariant;

class Qt5Widget : public QWidget
{
    Q_OBJECT

    Qt5Frame& m_rFrame;
    bool m_bNonEmptyIMPreeditSeen;
    int m_nDeltaX;
    int m_nDeltaY;

    bool handleKeyEvent(QKeyEvent*, bool);
    void handleMouseButtonEvent(QMouseEvent*, bool);
    void commitText(const QString& aText) const;

    virtual bool event(QEvent*) override;

    virtual void focusInEvent(QFocusEvent*) override;
    virtual void focusOutEvent(QFocusEvent*) override;
    // keyPressEvent(QKeyEvent*) is handled via event(QEvent*); see comment
    virtual void keyReleaseEvent(QKeyEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void dragEnterEvent(QDragEnterEvent*) override;
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

public slots:
    static void showTooltip(const OUString& rTip);

public:
    Qt5Widget(Qt5Frame& rFrame, Qt::WindowFlags f = Qt::WindowFlags());

    Qt5Frame& getFrame() const { return m_rFrame; }
    void startDrag(sal_Int8 nSourceActions);
    void endExtTextInput();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
