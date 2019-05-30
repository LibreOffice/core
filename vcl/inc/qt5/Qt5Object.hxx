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

#include <salobj.hxx>
#include <vcl/sysdata.hxx>

#include <QtCore/QObject>
#include <QtGui/QRegion>
#include <QtGui/QWindow>

class Qt5Frame;
class QWidget;

class Qt5Object : public QObject, public SalObject
{
    Q_OBJECT

    friend class Qt5OpenGLContext;

    SystemEnvData m_aSystemData;
    Qt5Frame* m_pParent;
    QWidget* m_pQWidget; // main widget, container
    QWindow* m_pQWindow; // contained window, used for opengl rendering
    QRegion m_pRegion;

public:
    Qt5Object(Qt5Frame* pParent, bool bShow);

    Qt5Frame* frame() const { return m_pParent; }
    QWidget* widget() const { return m_pQWidget; }
    QWindow* windowHandle() const { return m_pQWindow; }

    virtual void ResetClipRegion() override;
    virtual void BeginSetClipRegion(sal_uInt32 nRects) override;
    virtual void UnionClipRegion(long nX, long nY, long nWidth, long nHeight) override;
    virtual void EndSetClipRegion() override;

    virtual void SetPosSize(long nX, long nY, long nWidth, long nHeight) override;
    virtual void Show(bool bVisible) override;

    virtual void SetForwardKey(bool bEnable) override;

    virtual const SystemEnvData* GetSystemData() const override { return &m_aSystemData; }
};

class Qt5ObjectWindow : public QWindow
{
    Qt5Object& m_rParent;

    bool event(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    // keyPressEvent(QKeyEvent*) is handled via event(QEvent*); see comment in Qt5Widget::event
    void keyReleaseEvent(QKeyEvent*) override;

public:
    explicit Qt5ObjectWindow(Qt5Object& rParent);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
