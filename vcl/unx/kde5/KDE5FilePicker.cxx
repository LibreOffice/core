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

#include "KDE5FilePicker.hxx"

#include <KWindowSystem>
#include <KFileWidget>

#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QWindow>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>

// KDE5FilePicker

KDE5FilePicker::~KDE5FilePicker()
{
    delete _extraControls;
    delete _dialog;
}

bool KDE5FilePicker::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Show && o->isWidgetType())
    {
        auto* w = static_cast<QWidget*>(o);
        if (!w->parentWidget() && w->isModal())
        {
            if (auto* fileWidget = w->findChild<KFileWidget*>({}, Qt::FindDirectChildrenOnly))
                fileWidget->setCustomWidget(_extraControls);
        }
    }
    return QObject::eventFilter(o, e);
}

#include <KDE5FilePicker.moc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
