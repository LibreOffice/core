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

class Qt5Frame;
class Qt5Object;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QMoveEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QWheelEvent;

class Qt5Widget
    : public QWidget
{
    Q_OBJECT

    Qt5Frame  *m_pFrame;

    bool handleKeyEvent( QKeyEvent*, bool );
    void handleMouseButtonEvent( QMouseEvent*, bool );

    virtual void focusInEvent( QFocusEvent* ) override;
    virtual void focusOutEvent( QFocusEvent* ) override;
    virtual void keyPressEvent( QKeyEvent* ) override;
    virtual void keyReleaseEvent( QKeyEvent* ) override;
    virtual void mouseMoveEvent( QMouseEvent*) override;
    virtual void mousePressEvent( QMouseEvent*) override;
    virtual void mouseReleaseEvent( QMouseEvent*) override;
    virtual void moveEvent( QMoveEvent* ) override;
    virtual void paintEvent( QPaintEvent* ) override;
    virtual void resizeEvent( QResizeEvent* ) override;
    virtual void showEvent( QShowEvent* ) override;
    virtual void wheelEvent( QWheelEvent* ) override;

public:
    Qt5Widget( Qt5Frame &rFrame,
               QWidget *parent = Q_NULLPTR,
               Qt::WindowFlags f = Qt::WindowFlags() );
    virtual ~Qt5Widget() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
